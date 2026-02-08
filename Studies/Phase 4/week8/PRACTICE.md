# Week 8 실습: Factor Graph 구현 - IMU Factor와 Visual Factor

> 🎯 **목표**: C++로 IMU Factor와 Visual Factor의 잔차를 구현하고 최적화
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 5시간

---

## 📋 실습 개요

간단한 2 키프레임 VIO 문제를 구성합니다. IMU Factor와 Visual Factor의 잔차를 직접 계산하고, 가중 최소자승법으로 최적화하는 과정을 체험합니다.

---

## 🔧 빌드 및 실행

```bash
cd week8
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

---

## Step 1: SO(3) 유틸리티 + 잔차 함수

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <cmath>

// SO(3) 유틸리티
Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
    Eigen::Matrix3d m;
    m <<    0, -v.z(),  v.y(),
         v.z(),     0, -v.x(),
        -v.y(),  v.x(),     0;
    return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
    double angle = omega.norm();
    if (angle < 1e-10) return Eigen::Matrix3d::Identity();
    Eigen::Vector3d axis = omega / angle;
    Eigen::Matrix3d K = skew(axis);
    return Eigen::Matrix3d::Identity()
         + std::sin(angle) * K + (1.0 - std::cos(angle)) * K * K;
}

Eigen::Vector3d logSO3(const Eigen::Matrix3d& R) {
    double cos_a = (R.trace() - 1.0) / 2.0;
    cos_a = std::max(-1.0, std::min(1.0, cos_a));
    double angle = std::acos(cos_a);
    if (angle < 1e-10) return Eigen::Vector3d::Zero();
    Eigen::Matrix3d log_R = angle / (2.0 * std::sin(angle)) * (R - R.transpose());
    return Eigen::Vector3d(log_R(2,1), log_R(0,2), log_R(1,0));
}
```

---

## Step 2: IMU Factor 잔차

```cpp
/**
 * IMU Factor 잔차 계산
 *
 * 입력: 상태 i, 상태 j, Pre-integrated measurements
 * 출력: 9D 잔차 [r_rotation(3), r_velocity(3), r_position(3)]
 */
Eigen::VectorXd computeIMUResidual(
    const Eigen::Matrix3d& R_i, const Eigen::Vector3d& p_i,
    const Eigen::Vector3d& v_i,
    const Eigen::Matrix3d& R_j, const Eigen::Vector3d& p_j,
    const Eigen::Vector3d& v_j,
    const Eigen::Matrix3d& delta_R, const Eigen::Vector3d& delta_v,
    const Eigen::Vector3d& delta_p, double delta_t,
    const Eigen::Vector3d& gravity)
{
    Eigen::VectorXd r(9);

    // 회전 잔차: Log(ΔR^T · R_i^T · R_j)
    Eigen::Matrix3d r_rot = delta_R.transpose() * R_i.transpose() * R_j;
    r.segment<3>(0) = logSO3(r_rot);

    // 속도 잔차: R_i^T · (v_j - v_i - g·Δt) - Δv
    r.segment<3>(3) = R_i.transpose() * (v_j - v_i - gravity * delta_t) - delta_v;

    // 위치 잔차: R_i^T · (p_j - p_i - v_i·Δt - 0.5·g·Δt²) - Δp
    r.segment<3>(6) = R_i.transpose() *
        (p_j - p_i - v_i * delta_t - 0.5 * gravity * delta_t * delta_t)
        - delta_p;

    return r;
}
```

---

## Step 3: Visual Factor 잔차

```cpp
/**
 * Visual Factor 잔차 계산
 *
 * 입력: 카메라 포즈, 3D 점, 측정 픽셀, 카메라 내부 파라미터
 * 출력: 2D 재투영 오차
 */
Eigen::Vector2d computeVisualResidual(
    const Eigen::Matrix3d& R_wc, const Eigen::Vector3d& p_wc,
    const Eigen::Vector3d& P_w,
    const Eigen::Vector2d& z_measured,
    double fx, double fy, double cx, double cy)
{
    // 월드 → 카메라 변환
    Eigen::Vector3d P_c = R_wc.transpose() * (P_w - p_wc);

    // 깊이 체크
    if (P_c(2) < 0.01) {
        return Eigen::Vector2d(1e6, 1e6);  // 뒤에 있는 점
    }

    // 투영
    double u_proj = fx * P_c(0) / P_c(2) + cx;
    double v_proj = fy * P_c(1) / P_c(2) + cy;

    // 잔차 = 측정 - 투영
    return z_measured - Eigen::Vector2d(u_proj, v_proj);
}
```

---

## Step 4: 잔차 검증

```cpp
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8: Factor Graph 잔차 계산 실습" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Vector3d gravity(0, 0, -9.81);

    // === 참값 설정 ===
    Eigen::Matrix3d R_i = Eigen::Matrix3d::Identity();
    Eigen::Vector3d p_i(0, 0, 0);
    Eigen::Vector3d v_i(1, 0, 0);

    Eigen::Matrix3d R_j = expSO3(Eigen::Vector3d(0, 0, 0.1));  // z축 0.1rad 회전
    Eigen::Vector3d v_j(1, 0, 0);
    double dt = 1.0;
    Eigen::Vector3d p_j = p_i + v_i * dt + 0.5 * gravity * dt * dt;

    // === Pre-integrated measurements (참값에서 계산) ===
    Eigen::Matrix3d delta_R = R_i.transpose() * R_j;
    Eigen::Vector3d delta_v = R_i.transpose() * (v_j - v_i - gravity * dt);
    Eigen::Vector3d delta_p = R_i.transpose() *
        (p_j - p_i - v_i * dt - 0.5 * gravity * dt * dt);

    // === IMU 잔차 (참값이면 0이어야 함) ===
    auto r_imu = computeIMUResidual(R_i, p_i, v_i, R_j, p_j, v_j,
                                     delta_R, delta_v, delta_p, dt, gravity);

    std::cout << "  [1] IMU Factor 잔차 (참값):" << std::endl;
    std::cout << "    r_rotation: " << r_imu.segment<3>(0).transpose() << std::endl;
    std::cout << "    r_velocity: " << r_imu.segment<3>(3).transpose() << std::endl;
    std::cout << "    r_position: " << r_imu.segment<3>(6).transpose() << std::endl;
    std::cout << "    ||r|| = " << r_imu.norm() << " (0이면 정확)\n" << std::endl;

    // === 상태에 오차를 주면? ===
    Eigen::Vector3d p_j_noisy = p_j + Eigen::Vector3d(0.5, 0.2, -0.1);
    auto r_imu_noisy = computeIMUResidual(R_i, p_i, v_i, R_j, p_j_noisy, v_j,
                                           delta_R, delta_v, delta_p, dt, gravity);

    std::cout << "  [2] IMU Factor 잔차 (위치에 오차 추가):" << std::endl;
    std::cout << "    p_j 오차: [0.5, 0.2, -0.1]" << std::endl;
    std::cout << "    r_position: " << r_imu_noisy.segment<3>(6).transpose() << std::endl;
    std::cout << "    ||r|| = " << r_imu_noisy.norm() << " (0이 아님!)\n" << std::endl;

    // === Visual Factor ===
    double fx = 500, fy = 500, cx = 320, cy = 240;
    Eigen::Vector3d P_w(5, 1, 0);  // 3D 랜드마크

    // 참값으로 투영
    Eigen::Vector3d P_c = R_i.transpose() * (P_w - p_i);
    Eigen::Vector2d z_true(fx * P_c(0)/P_c(2) + cx, fy * P_c(1)/P_c(2) + cy);

    auto r_vis = computeVisualResidual(R_i, p_i, P_w, z_true, fx, fy, cx, cy);

    std::cout << "  [3] Visual Factor 잔차 (참값):" << std::endl;
    std::cout << "    3D 점: " << P_w.transpose() << std::endl;
    std::cout << "    투영: (" << z_true(0) << ", " << z_true(1) << ")" << std::endl;
    std::cout << "    r_visual: " << r_vis.transpose() << std::endl;
    std::cout << "    ||r|| = " << r_vis.norm() << " (0이면 정확)\n" << std::endl;

    // 포즈에 오차를 주면?
    Eigen::Vector3d p_i_noisy = p_i + Eigen::Vector3d(0.1, 0, 0);
    auto r_vis_noisy = computeVisualResidual(R_i, p_i_noisy, P_w, z_true, fx, fy, cx, cy);

    std::cout << "  [4] Visual Factor 잔차 (위치에 0.1m 오차):" << std::endl;
    std::cout << "    r_visual: " << r_vis_noisy.transpose() << " 픽셀" << std::endl;
    std::cout << "    ||r|| = " << r_vis_noisy.norm() << " 픽셀\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "핵심: 잔차 = 측정값 - 추정값" << std::endl;
    std::cout << "  → 참값이면 잔차 = 0" << std::endl;
    std::cout << "  → 오차가 있으면 잔차 ≠ 0 → 최적화 대상" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 8: Factor Graph 잔차 계산 실습
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  [1] IMU Factor 잔차 (참값):
    r_rotation: 0 0 0
    r_velocity: 0 0 0
    r_position: 0 0 0
    ||r|| = 0 (0이면 정확)

  [2] IMU Factor 잔차 (위치에 오차 추가):
    p_j 오차: [0.5, 0.2, -0.1]
    r_position: 0.5 0.2 -0.1
    ||r|| = 0.548 (0이 아님!)

  [3] Visual Factor 잔차 (참값):
    ...
    ||r|| = 0 (0이면 정확)

  [4] Visual Factor 잔차 (위치에 0.1m 오차):
    r_visual: X.X X.X 픽셀
    ||r|| = X.X 픽셀
```

---

## ✅ 체크리스트

- [ ] IMU Factor 잔차 수식을 코드로 이해
- [ ] 참값에서 잔차가 0인 것 확인
- [ ] Visual Factor 재투영 오차 이해
- [ ] 마하라노비스 거리의 의미 이해
- [ ] Factor Graph 그림을 직접 그릴 수 있음

---

## 💡 추가 실험

1. **가중치 비교**: IMU 공분산을 크게/작게 → 최적화 결과 변화 관찰
2. **바이어스 Factor 추가**: b_{i+1} - b_i를 잔차로 추가
3. **Ceres 연결**: Ceres Solver로 실제 최적화 (Advanced)
4. **잔차 자코비안**: 수치적 자코비안 계산으로 검증

---

**다음**: [Week 9 - Pre-integration 심화](../week9/README.md)
