# Week 12 실습: VIO 초기화 구현

## 실습 목표
- Visual-Inertial Alignment의 핵심인 **선형 시스템 구성 및 풀기** 구현
- 자이로 바이어스 추정 → 스케일/중력/속도 동시 추정 파이프라인 체험

---

## 빌드 방법

```bash
cd Studies/Phase\ 4/week12
mkdir -p build && cd build
cmake .. && make
./quiz_easy      # 개념 퀴즈
./quiz_medium    # 계산 퀴즈
```

---

## 핵심 코드: Visual-Inertial Alignment

### 1. 자이로 바이어스 추정

```cpp
#include <Eigen/Dense>
#include <cmath>

using namespace Eigen;

// SO(3) 유틸리티
Matrix3d skew(const Vector3d& v) {
    Matrix3d m;
    m <<    0, -v.z(),  v.y(),
         v.z(),     0, -v.x(),
        -v.y(),  v.x(),     0;
    return m;
}

Matrix3d expSO3(const Vector3d& omega) {
    double angle = omega.norm();
    if (angle < 1e-10) return Matrix3d::Identity();
    Vector3d axis = omega / angle;
    Matrix3d K = skew(axis);
    return Matrix3d::Identity()
         + std::sin(angle) * K + (1.0 - std::cos(angle)) * K * K;
}

Vector3d logSO3(const Matrix3d& R) {
    double cos_angle = (R.trace() - 1.0) / 2.0;
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle));
    double angle = std::acos(cos_angle);
    if (angle < 1e-10) return Vector3d::Zero();
    Matrix3d log_R = (angle / (2.0 * std::sin(angle))) * (R - R.transpose());
    return Vector3d(log_R(2,1), log_R(0,2), log_R(1,0));
}

/**
 * 자이로 바이어스 추정
 *
 * Vision에서 얻은 상대 회전 R_ij와
 * IMU Pre-integration으로 얻은 ΔR_ij를 비교하여 b_g 추정
 *
 * 비용함수: min_{b_g} Σ ||Log(ΔR_ij^T · R_i^T · R_j)||²
 *
 * 1차 근사:
 *   ΔR_ij(b_g) ≈ ΔR_ij(0) · Exp(J_ΔR · δb_g)
 *   → 선형 시스템으로 변환
 */
Vector3d estimateGyroBias(
    const std::vector<Matrix3d>& R_vision,    // Vision 회전 (월드 기준)
    const std::vector<Matrix3d>& DR_preint,   // Pre-integrated 상대 회전
    const std::vector<Matrix3d>& J_DR_bg      // ΔR의 b_g에 대한 자코비안
) {
    int n = R_vision.size() - 1;  // 키프레임 쌍 수

    // Ax = b 구성 (3n × 3)
    MatrixXd A(3 * n, 3);
    VectorXd b_vec(3 * n);

    for (int k = 0; k < n; k++) {
        // Vision에서의 상대 회전
        Matrix3d R_ij_vision = R_vision[k].transpose() * R_vision[k+1];

        // 잔차: Log(ΔR_ij^T · R_ij_vision)
        Matrix3d residual_R = DR_preint[k].transpose() * R_ij_vision;
        Vector3d residual = logSO3(residual_R);

        // 자코비안 (ΔR의 b_g에 대한)
        // J_DR_bg[k]는 미리 Pre-integration에서 계산됨
        A.block<3,3>(3*k, 0) = J_DR_bg[k];
        b_vec.segment<3>(3*k) = residual;
    }

    // 최소자승 해: b_g = (A^T A)^{-1} A^T b
    Vector3d b_g = (A.transpose() * A).ldlt().solve(A.transpose() * b_vec);

    return b_g;
}
```

### 2. 스케일, 중력, 속도 동시 추정

```cpp
/**
 * Visual-Inertial Alignment
 *
 * 선형 시스템으로 s, g, v를 동시에 추정
 *
 * 공식 (키프레임 i→j):
 *   s·(p_j - p_i) = v_i·Δt - 0.5·g·Δt² + R_i·Δv_ij·Δt + R_i·Δp_ij
 *
 * 미지수 벡터:
 *   x = [v_0(3), v_1(3), ..., v_n(3), g(3), s(1)]
 *   크기: 3(n+1) + 3 + 1 = 3n + 7
 */
struct AlignmentResult {
    double scale;
    Vector3d gravity;
    std::vector<Vector3d> velocities;
};

AlignmentResult linearAlignment(
    const std::vector<Vector3d>& p_vision,   // Vision 위치 (up-to-scale)
    const std::vector<Matrix3d>& R_vision,   // Vision 회전
    const std::vector<Vector3d>& Dp_preint,  // Pre-integrated Δp
    const std::vector<Vector3d>& Dv_preint,  // Pre-integrated Δv
    const std::vector<double>& dt_intervals  // 시간 간격
) {
    int n = p_vision.size();       // 키프레임 수
    int pairs = n - 1;             // 키프레임 쌍 수
    int dim = 3 * n + 3 + 1;      // 미지수 차원: v(3n) + g(3) + s(1)

    MatrixXd H = MatrixXd::Zero(6 * pairs, dim);
    VectorXd z = VectorXd::Zero(6 * pairs);

    for (int k = 0; k < pairs; k++) {
        double dt = dt_intervals[k];
        Matrix3d Ri = R_vision[k];
        Vector3d dp_v = p_vision[k+1] - p_vision[k];  // Vision 변위

        // 위치 방정식: s·dp_v = v_i·dt - 0.5·g·dt² + Ri·Δp_ij
        // → v_i·dt - 0.5·g·dt² - s·dp_v = -Ri·Δp_ij

        int row = 6 * k;

        // v_i 계수 (위치)
        H.block<3,3>(row, 3*k) = Matrix3d::Identity() * dt;

        // g 계수 (위치)
        H.block<3,3>(row, 3*n) = -0.5 * dt * dt * Matrix3d::Identity();

        // s 계수 (위치)
        H.block<3,1>(row, 3*n + 3) = -dp_v;

        // 우변 (위치)
        z.segment<3>(row) = -Ri * Dp_preint[k];

        // 속도 방정식: v_j = v_i - g·dt + Ri·Δv_ij
        // → v_i - v_j - g·dt = -Ri·Δv_ij

        // v_i 계수 (속도)
        H.block<3,3>(row+3, 3*k) = Matrix3d::Identity();

        // v_j 계수 (속도)
        if (k + 1 < n) {
            H.block<3,3>(row+3, 3*(k+1)) = -Matrix3d::Identity();
        }

        // g 계수 (속도)
        H.block<3,3>(row+3, 3*n) = -dt * Matrix3d::Identity();

        // 우변 (속도)
        z.segment<3>(row+3) = -Ri * Dv_preint[k];
    }

    // 최소자승 해
    VectorXd x = (H.transpose() * H).ldlt().solve(H.transpose() * z);

    // 결과 추출
    AlignmentResult result;
    result.velocities.resize(n);
    for (int i = 0; i < n; i++) {
        result.velocities[i] = x.segment<3>(3*i);
    }
    result.gravity = x.segment<3>(3*n);
    result.scale = x(3*n + 3);

    return result;
}
```

### 3. 중력 정제 (Gravity Refinement)

```cpp
/**
 * 중력 벡터 정제
 *
 * 선형 추정한 g의 크기가 정확히 9.81이 되도록 제약
 *
 * 아이디어: g = ||g|| · g_hat + w1·b1 + w2·b2
 *   g_hat: 현재 중력 방향 (단위 벡터)
 *   b1, b2: g_hat에 수직인 기저 벡터
 *   w1, w2: 보정할 파라미터 (2D)
 *
 * → 미지수가 3D에서 2D로 줄어듦
 */
Vector3d refineGravity(
    const Vector3d& g_initial,
    double g_norm = 9.81
) {
    Vector3d g_hat = g_initial.normalized();

    // g_hat에 수직인 기저 벡터 2개 생성
    Vector3d tmp = (std::abs(g_hat.x()) < 0.9)
                 ? Vector3d(1,0,0) : Vector3d(0,1,0);
    Vector3d b1 = (tmp - g_hat * g_hat.dot(tmp)).normalized();
    Vector3d b2 = g_hat.cross(b1);

    // 이제 g = g_norm * g_hat + w1*b1 + w2*b2로 파라미터화
    // w1, w2를 반복적으로 최적화

    // (실제 VINS에서는 LinearAlignment을
    //  g 파라미터화를 바꿔서 여러 번 반복)

    // 최종적으로 ||g|| = g_norm으로 정규화
    Vector3d g_refined = g_hat * g_norm;

    return g_refined;
}
```

---

## 실험: 시뮬레이션 초기화

### 시나리오

```
시뮬레이션:
  - 5개 키프레임, 직선 운동 + 회전
  - 실제 스케일 = 3.0
  - 실제 중력 = [0, 0, -9.81]
  - Vision에서 up-to-scale 궤적 제공
  - IMU에서 Pre-integrated 값 제공

테스트:
  1. 선형 시스템으로 s, g, v 추정
  2. 추정값과 실제값 비교
  3. 노이즈 레벨에 따른 정확도 변화
```

---

## VINS-Mono 코드 분석 가이드

### `initial_alignment.cpp` 핵심 흐름

```
solveGyroscopeBias()
  → 모든 키프레임 쌍에서 Vision R vs IMU ΔR 비교
  → 선형 시스템으로 b_g 추정
  → Pre-integration 보정 (repropagate)

LinearAlignment()
  → H, b 행렬 구성 (위치 + 속도 방정식)
  → SVD로 풀기 (LDLT가 아닌 SVD 사용)
  → s, g, v 추출

RefineGravity()
  → 4회 반복
  → g를 2D 파라미터로 재파라미터화
  → ||g|| = G (중력 상수) 제약 적용
  → 수렴 후 최종 g, s, v 반환
```

---

## 참고 자료

- VINS-Mono 논문 Section IV-A,B (Initialization)
- Qin & Shen, "Robust Initialization of Monocular Visual-Inertial Estimation..."
- `initial_alignment.cpp` 소스 코드

---

이전: [Week 11 PRACTICE](../week11/PRACTICE.md)
다음: [Week 13 PRACTICE](../week13/PRACTICE.md)
