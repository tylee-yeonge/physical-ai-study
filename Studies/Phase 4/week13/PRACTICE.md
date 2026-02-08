# Week 13 실습: Camera-IMU 외부 캘리브레이션

## 실습 목표
- Extrinsic 오차가 VIO에 미치는 영향을 수치적으로 체험
- 핸드-아이 캘리브레이션 (AX=XB) 구현

---

## 빌드 방법

```bash
cd Studies/Phase\ 4/week13
mkdir -p build && cd build
cmake .. && make
./quiz_easy      # 개념 퀴즈
./quiz_medium    # 계산 퀴즈
```

---

## 핵심 코드: Extrinsic 영향 분석

### 1. 회전 오차에 의한 중력 누출

```cpp
#include <Eigen/Dense>
#include <cmath>
#include <iostream>

using namespace Eigen;

Matrix3d expSO3(const Vector3d& omega) {
    double angle = omega.norm();
    if (angle < 1e-10) return Matrix3d::Identity();
    Vector3d axis = omega / angle;
    Matrix3d K;
    K <<    0, -axis.z(),  axis.y(),
         axis.z(),     0, -axis.x(),
        -axis.y(),  axis.x(),     0;
    return Matrix3d::Identity()
         + std::sin(angle) * K + (1.0 - std::cos(angle)) * K * K;
}

/**
 * Extrinsic 회전 오차에 의한 중력 누출 시뮬레이션
 *
 * 정확한 Extrinsic: R_ci_true
 * 오차가 있는 Extrinsic: R_ci_est = R_ci_true * ExpSO3(delta_theta)
 *
 * 이 오차가 가속도 변환에 미치는 영향을 분석
 */
void analyzeExtrinsicError() {
    // 실제 Extrinsic (예: 카메라가 IMU에서 x축으로 45도 회전)
    Matrix3d R_ci_true = expSO3(Vector3d(0.785, 0.0, 0.0)); // 45도

    // 중력 (월드 프레임)
    Vector3d g_world(0, 0, -9.81);

    // IMU 프레임의 중력 (정지, R_wi = I 가정)
    Vector3d g_imu = g_world;  // R_wi = I이면 같음

    // 다양한 회전 오차에 대해
    std::vector<double> errors_deg = {0.1, 0.5, 1.0, 2.0, 5.0};

    std::cout << "Extrinsic 회전 오차의 영향:\n" << std::endl;
    std::cout << "  오차(도) │ 가속도 오차(m/s²) │ 5초 위치 드리프트(m)" << std::endl;
    std::cout << "  ─────────┼──────────────────┼────────────────────" << std::endl;

    for (double err_deg : errors_deg) {
        double err_rad = err_deg * M_PI / 180.0;
        Vector3d delta_theta(err_rad, 0, 0); // x축 오차

        Matrix3d R_ci_est = R_ci_true * expSO3(delta_theta);

        // 올바른 변환: a_cam = R_ci_true * a_imu
        Vector3d a_cam_true = R_ci_true * g_imu;

        // 오차가 있는 변환: a_cam = R_ci_est * a_imu
        Vector3d a_cam_est = R_ci_est * g_imu;

        // 가속도 오차
        double accel_error = (a_cam_est - a_cam_true).norm();

        // 5초간 위치 드리프트
        double pos_drift = 0.5 * accel_error * 25.0;

        printf("   %4.1f°     │     %7.4f       │       %7.3f\n",
               err_deg, accel_error, pos_drift);
    }
}
```

### 2. 시간 오프셋의 영향

```cpp
/**
 * 시간 오프셋 td에 의한 오차 분석
 *
 * Camera timestamp: t_cam
 * IMU timestamp:    t_imu = t_cam + td
 *
 * td를 무시하면 IMU 데이터를 잘못된 시점에 사용
 */
void analyzeTimeOffset() {
    // 시뮬레이션: 일정 각속도로 회전
    double omega = 2.0;  // rad/s (약 114 deg/s)

    std::cout << "\n시간 오프셋의 영향 (각속도 = " << omega << " rad/s):\n" << std::endl;

    std::vector<double> td_values_ms = {1.0, 5.0, 10.0, 20.0, 50.0};

    std::cout << "  td (ms) │ 회전 오차(도) │ 중력 누출(m/s²)" << std::endl;
    std::cout << "  ────────┼──────────────┼─────────────────" << std::endl;

    for (double td_ms : td_values_ms) {
        double td = td_ms / 1000.0;  // seconds
        double rotation_error_rad = omega * td;
        double rotation_error_deg = rotation_error_rad * 180.0 / M_PI;
        double gravity_leak = std::sin(rotation_error_rad) * 9.81;

        printf("   %5.1f   │    %6.3f     │      %6.4f\n",
               td_ms, rotation_error_deg, gravity_leak);
    }

    std::cout << "\n  → td = 10ms + 각속도 2 rad/s → 1.15도 회전 오차!" << std::endl;
    std::cout << "  → 이것이 VINS에서 td를 추정하는 이유" << std::endl;
}
```

### 3. 핸드-아이 캘리브레이션 (AX = XB)

```cpp
/**
 * 핸드-아이 캘리브레이션: AX = XB
 *
 * A = Camera 상대 회전 (R_cam_ij)
 * B = IMU 상대 회전 (R_imu_ij)
 * X = Extrinsic 회전 (R_ci)
 *
 * 관계식:
 *   R_cam_ij = R_ci * R_imu_ij * R_ci^T
 *   → R_cam_ij * R_ci = R_ci * R_imu_ij
 *   → AX = XB
 *
 * Quaternion 기반 풀이:
 *   (q_A - q_B) * q_X = 0 형태의 방정식
 *   → 여러 쌍 모아서 SVD
 */
struct RotationPair {
    Matrix3d A;  // Camera 상대 회전
    Matrix3d B;  // IMU 상대 회전
};

Matrix3d solveHandEye(const std::vector<RotationPair>& pairs) {
    int n = pairs.size();

    // Angle-axis 기반 풀이 (간단한 버전)
    // (A_i - I) * X = X * (B_i - I)
    // → Σ || logSO3(A_i * X) - logSO3(X * B_i) ||² 최소화

    // 단순 반복법 (실제로는 SVD 기반이 더 정확)
    Matrix3d X = Matrix3d::Identity();

    for (int iter = 0; iter < 100; iter++) {
        Matrix3d delta_sum = Matrix3d::Zero();
        int count = 0;

        for (const auto& pair : pairs) {
            // 잔차: A * X - X * B
            Matrix3d AX = pair.A * X;
            Matrix3d XB = X * pair.B;

            // Procrustes 문제로 근사
            delta_sum += AX.transpose() * XB;
            count++;
        }

        // SVD로 가장 가까운 회전 행렬 찾기
        JacobiSVD<Matrix3d> svd(delta_sum,
                                ComputeFullU | ComputeFullV);
        Matrix3d U = svd.matrixU();
        Matrix3d V = svd.matrixV();

        // det 보정 (proper rotation 보장)
        double det = (V * U.transpose()).determinant();
        Matrix3d D = Matrix3d::Identity();
        D(2,2) = det;

        X = V * D * U.transpose();
    }

    return X;
}

/**
 * 테스트: 시뮬레이션 데이터로 핸드-아이 캘리브레이션
 */
void testHandEyeCalibration() {
    // 실제 Extrinsic
    Matrix3d R_ci_true = expSO3(Vector3d(0.3, -0.1, 0.2));

    // 5개의 회전 쌍 생성
    std::vector<RotationPair> pairs;
    std::vector<Vector3d> imu_rotations = {
        {0.2, 0.1, 0.0},
        {0.0, 0.3, 0.1},
        {-0.1, 0.0, 0.2},
        {0.15, -0.2, 0.05},
        {0.1, 0.1, -0.15}
    };

    for (const auto& omega : imu_rotations) {
        Matrix3d R_imu = expSO3(omega);

        // A = R_ci * R_imu * R_ci^T
        Matrix3d R_cam = R_ci_true * R_imu * R_ci_true.transpose();

        pairs.push_back({R_cam, R_imu});
    }

    Matrix3d R_ci_est = solveHandEye(pairs);

    std::cout << "\n핸드-아이 캘리브레이션 결과:\n" << std::endl;
    std::cout << "  실제 R_ci:" << std::endl;
    std::cout << R_ci_true << "\n" << std::endl;
    std::cout << "  추정 R_ci:" << std::endl;
    std::cout << R_ci_est << "\n" << std::endl;

    // 오차 계산
    Matrix3d error_R = R_ci_true.transpose() * R_ci_est;
    double angle_error = std::acos(
        std::max(-1.0, std::min(1.0, (error_R.trace() - 1.0) / 2.0))
    );
    std::cout << "  회전 오차: " << angle_error * 180.0 / M_PI << " 도" << std::endl;
}
```

---

## 실험 순서

1. **Extrinsic 오차 분석**: 회전 오차 크기별 드리프트 관찰
2. **시간 오프셋 분석**: td 크기별 오차 관찰
3. **핸드-아이 캘리브레이션**: 시뮬레이션 데이터로 Extrinsic 추정

---

## VINS-Mono 코드 분석 가이드

### `initial_ex_rotation.cpp` 핵심 흐름

```
CalibrationExRotation():
  → Camera 상대 회전 수집 (corr_cam)
  → IMU 상대 회전 수집 (corr_imu)
  → Hand-Eye 문제로 변환
  → SVD 기반 풀이
  → 수렴 확인 (eigenvalue ratio)
```

### `parameters.cpp` 설정

```yaml
# VINS config YAML 예시
extrinsicRotation: !!opencv-matrix
   rows: 3
   cols: 3
   data: [1,0,0, 0,1,0, 0,0,1]
extrinsicTranslation: !!opencv-matrix
   rows: 3
   cols: 1
   data: [0.0, 0.0, 0.0]
td: 0.0    # 시간 오프셋 초기값
estimate_td: 1  # 온라인 추정 활성화
```

---

## 참고 자료

- Tsai & Lenz, "A New Technique for Fully Autonomous and Efficient 3D Robotics Hand/Eye Calibration" (1989)
- VINS-Mono 논문 Section V (Online Temporal Calibration)
- Kalibr: https://github.com/ethz-asl/kalibr

---

이전: [Week 12 PRACTICE](../week12/PRACTICE.md)
다음: [Week 14 PRACTICE](../week14/PRACTICE.md)
