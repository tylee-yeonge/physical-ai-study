# Week 7 실습: Pre-integration 수식 구현 - IMU 사전 적분

> 🎯 **목표**: C++로 Pre-integration 핵심 수식을 단계별 구현
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 5시간

---

## 📋 실습 개요

IMU 데이터로 Pre-integrated measurement (ΔR, Δv, Δp)를 계산하고, 바이어스 보정까지 구현합니다. 등속 직선 운동 + 등속 회전 시나리오에서 적분 결과를 검증합니다.

---

## 🔧 빌드 및 실행

```bash
cd week7
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

---

## Step 1: SO(3) 유틸리티 함수

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <vector>

// Skew-symmetric 행렬 (hat 연산)
Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
    Eigen::Matrix3d m;
    m <<    0, -v.z(),  v.y(),
         v.z(),     0, -v.x(),
        -v.y(),  v.x(),     0;
    return m;
}

// SO(3) 지수 사상: 각속도 벡터 → 회전행렬
Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
    double angle = omega.norm();
    if (angle < 1e-10) {
        return Eigen::Matrix3d::Identity();
    }
    Eigen::Vector3d axis = omega / angle;
    Eigen::Matrix3d K = skew(axis);
    // Rodrigues 공식
    return Eigen::Matrix3d::Identity()
         + std::sin(angle) * K
         + (1.0 - std::cos(angle)) * K * K;
}

// SO(3) 로그 사상: 회전행렬 → 각속도 벡터
Eigen::Vector3d logSO3(const Eigen::Matrix3d& R) {
    double cos_angle = (R.trace() - 1.0) / 2.0;
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle));
    double angle = std::acos(cos_angle);
    if (angle < 1e-10) {
        return Eigen::Vector3d::Zero();
    }
    Eigen::Matrix3d log_R = angle / (2.0 * std::sin(angle)) * (R - R.transpose());
    return Eigen::Vector3d(log_R(2,1), log_R(0,2), log_R(1,0));
}
```

---

## Step 2: Pre-integration 클래스

```cpp
/**
 * IMU Pre-integration
 *
 * IMU 데이터를 순차적으로 받아서:
 * 1. Pre-integrated measurement (ΔR, Δv, Δp) 계산
 * 2. 바이어스 보정용 자코비안 계산
 */
class PreIntegration {
public:
    PreIntegration(const Eigen::Vector3d& bias_acc,
                   const Eigen::Vector3d& bias_gyro)
        : b_a_(bias_acc), b_g_(bias_gyro), dt_sum_(0.0)
    {
        // Pre-integrated measurements 초기화
        delta_R_ = Eigen::Matrix3d::Identity();
        delta_v_ = Eigen::Vector3d::Zero();
        delta_p_ = Eigen::Vector3d::Zero();

        // 바이어스 보정 자코비안 초기화
        J_R_bg_ = Eigen::Matrix3d::Zero();
        J_v_ba_ = Eigen::Matrix3d::Zero();
        J_v_bg_ = Eigen::Matrix3d::Zero();
        J_p_ba_ = Eigen::Matrix3d::Zero();
        J_p_bg_ = Eigen::Matrix3d::Zero();
    }

    // IMU 데이터 하나를 적분
    void integrate(const Eigen::Vector3d& acc_meas,
                   const Eigen::Vector3d& gyro_meas,
                   double dt)
    {
        // 바이어스 제거
        Eigen::Vector3d acc  = acc_meas - b_a_;
        Eigen::Vector3d gyro = gyro_meas - b_g_;

        // === 자코비안 업데이트 (현재 값 사용, 순서 중요!) ===
        Eigen::Matrix3d dR_inv = expSO3(-gyro * dt);

        // 위치 자코비안 (먼저! 현재 J_v 사용)
        J_p_ba_ += J_v_ba_ * dt - 0.5 * delta_R_ * dt * dt;
        J_p_bg_ += J_v_bg_ * dt
                 - 0.5 * delta_R_ * skew(acc) * J_R_bg_ * dt * dt;

        // 속도 자코비안
        J_v_ba_ -= delta_R_ * dt;
        J_v_bg_ -= delta_R_ * skew(acc) * J_R_bg_ * dt;

        // 회전 자코비안
        J_R_bg_ = dR_inv * J_R_bg_
                 - Eigen::Matrix3d::Identity() * dt;  // Jr ≈ I (작은 각도)

        // === Pre-integrated measurement 업데이트 ===
        // 위치 (현재 delta_v, delta_R 사용)
        delta_p_ += delta_v_ * dt + 0.5 * delta_R_ * acc * dt * dt;

        // 속도 (현재 delta_R 사용)
        delta_v_ += delta_R_ * acc * dt;

        // 회전
        delta_R_ = delta_R_ * expSO3(gyro * dt);

        dt_sum_ += dt;
    }

    // 바이어스 변화에 따른 보정
    void correctBias(const Eigen::Vector3d& db_a,
                     const Eigen::Vector3d& db_g)
    {
        delta_p_ += J_p_ba_ * db_a + J_p_bg_ * db_g;
        delta_v_ += J_v_ba_ * db_a + J_v_bg_ * db_g;
        delta_R_ = delta_R_ * expSO3(J_R_bg_ * db_g);
    }

    // Getters
    Eigen::Matrix3d getDeltaR() const { return delta_R_; }
    Eigen::Vector3d getDeltaV() const { return delta_v_; }
    Eigen::Vector3d getDeltaP() const { return delta_p_; }
    double getDeltaT() const { return dt_sum_; }

    // 상태 복원: 절대 포즈 계산
    void recoverState(
        const Eigen::Vector3d& p_i,
        const Eigen::Vector3d& v_i,
        const Eigen::Matrix3d& R_i,
        const Eigen::Vector3d& gravity,
        Eigen::Vector3d& p_j,
        Eigen::Vector3d& v_j,
        Eigen::Matrix3d& R_j) const
    {
        double dt = dt_sum_;
        R_j = R_i * delta_R_;
        v_j = v_i + gravity * dt + R_i * delta_v_;
        p_j = p_i + v_i * dt + 0.5 * gravity * dt * dt + R_i * delta_p_;
    }

private:
    Eigen::Matrix3d delta_R_;
    Eigen::Vector3d delta_v_, delta_p_;
    Eigen::Vector3d b_a_, b_g_;
    double dt_sum_;

    // 바이어스 보정 자코비안
    Eigen::Matrix3d J_R_bg_;
    Eigen::Matrix3d J_v_ba_, J_v_bg_;
    Eigen::Matrix3d J_p_ba_, J_p_bg_;
};
```

---

## Step 3: 검증 시뮬레이션

```cpp
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7: Pre-integration 수식 검증" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 시나리오: 등속 직선 운동 (x 방향, 2m/s)
    // 가속도 = 0, 각속도 = 0
    // 중력 = [0, 0, -9.81]

    Eigen::Vector3d gravity(0, 0, -9.81);
    Eigen::Vector3d b_a = Eigen::Vector3d::Zero();  // 바이어스 없음
    Eigen::Vector3d b_g = Eigen::Vector3d::Zero();

    double dt = 0.005;   // 200Hz
    double total = 1.0;  // 1초
    int steps = total / dt;

    // 초기 상태
    Eigen::Vector3d p_i(0, 0, 0);
    Eigen::Vector3d v_i(2, 0, 0);  // x방향 2m/s
    Eigen::Matrix3d R_i = Eigen::Matrix3d::Identity();

    // Pre-integration
    PreIntegration preint(b_a, b_g);

    for (int k = 0; k < steps; k++) {
        // IMU 측정 시뮬레이션
        // 가속도계: R^T * (a_world - g) + b_a
        // 등속이므로 a_world = 0
        Eigen::Vector3d a_meas = R_i.transpose() * (Eigen::Vector3d::Zero() - gravity) + b_a;
        Eigen::Vector3d g_meas = b_g;  // 정지 회전이므로 0

        preint.integrate(a_meas, g_meas, dt);
    }

    // 상태 복원
    Eigen::Vector3d p_j, v_j;
    Eigen::Matrix3d R_j;
    preint.recoverState(p_i, v_i, R_i, gravity, p_j, v_j, R_j);

    // 실제값 (등속 직선운동)
    Eigen::Vector3d p_true = p_i + v_i * total;  // [2, 0, 0]
    Eigen::Vector3d v_true = v_i;                  // [2, 0, 0]
    Eigen::Matrix3d R_true = R_i;                  // Identity

    std::cout << "  [시나리오] 등속 직선 운동 (x방향 2m/s, 1초)\n" << std::endl;
    std::cout << "  Pre-integrated measurements:" << std::endl;
    std::cout << "    ΔR (logSO3): " << logSO3(preint.getDeltaR()).transpose() << std::endl;
    std::cout << "    Δv: " << preint.getDeltaV().transpose() << std::endl;
    std::cout << "    Δp: " << preint.getDeltaP().transpose() << std::endl;
    std::cout << "    Δt: " << preint.getDeltaT() << "s\n" << std::endl;

    std::cout << "  복원된 상태:" << std::endl;
    std::cout << "    p_j: " << p_j.transpose() << " (실제: " << p_true.transpose() << ")" << std::endl;
    std::cout << "    v_j: " << v_j.transpose() << " (실제: " << v_true.transpose() << ")" << std::endl;

    double p_err = (p_j - p_true).norm();
    double v_err = (v_j - v_true).norm();
    double R_err = logSO3(R_true.transpose() * R_j).norm();

    std::cout << "\n  오차:" << std::endl;
    std::cout << "    위치: " << p_err << " m" << std::endl;
    std::cout << "    속도: " << v_err << " m/s" << std::endl;
    std::cout << "    회전: " << R_err << " rad" << std::endl;

    std::cout << "\n  → " << (p_err < 0.01 ? "위치 정확!" : "위치 오차 있음") << std::endl;
    std::cout << "  → " << (v_err < 0.01 ? "속도 정확!" : "속도 오차 있음") << std::endl;
    std::cout << "  → " << (R_err < 0.001 ? "회전 정확!" : "회전 오차 있음") << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "핵심 관찰:" << std::endl;
    std::cout << "  1. Δp, Δv는 '출발 포즈 기준' 상대 변화량" << std::endl;
    std::cout << "  2. 복원 시 R_i를 곱해서 월드 프레임으로 변환" << std::endl;
    std::cout << "  3. 중력 보상이 정확하면 적분 오차가 매우 작음" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 7: Pre-integration 수식 검증
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  [시나리오] 등속 직선 운동 (x방향 2m/s, 1초)

  Pre-integrated measurements:
    ΔR (logSO3): 0 0 0
    Δv: 0 0 0
    Δp: 2 0 0
    Δt: 1s

  복원된 상태:
    p_j: 2 0 0 (실제: 2 0 0)
    v_j: 2 0 0 (실제: 2 0 0)

  오차:
    위치: 0.000... m
    속도: 0.000... m/s
    회전: 0 rad

  → 위치 정확!
  → 속도 정확!
  → 회전 정확!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
핵심 관찰:
  1. Δp, Δv는 '출발 포즈 기준' 상대 변화량
  2. 복원 시 R_i를 곱해서 월드 프레임으로 변환
  3. 중력 보상이 정확하면 적분 오차가 매우 작음
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] expSO3, logSO3, skew 함수 이해
- [ ] Pre-integration 업데이트 순서 (p → v → R) 이해
- [ ] 가속도계 측정 모델 (R^T(a-g) + b_a) 이해
- [ ] 상태 복원 공식 (p_j = p_i + v_i·Δt + 0.5·g·Δt² + R_i·Δp_ij) 이해
- [ ] 바이어스 보정 자코비안의 역할 이해

---

## 💡 추가 실험

1. **회전 추가**: z축 주위 0.5 rad/s 회전하면서 이동 → ΔR 변화 관찰
2. **바이어스 추가**: b_a = [0.1, 0, 0] → 보정 전/후 오차 비교
3. **노이즈 추가**: IMU에 가우시안 노이즈 → 적분 드리프트 관찰
4. **dt 변경**: 200Hz → 100Hz, 50Hz → 적분 정확도 비교

---

**다음**: [Week 8 - Factor Graph에서의 역할](../week8/README.md)
