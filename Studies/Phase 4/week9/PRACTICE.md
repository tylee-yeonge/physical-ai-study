# Week 9 실습: Pre-integration 공분산 전파 구현

> 🎯 **목표**: C++로 Pre-integration 공분산을 전파하고 시간에 따른 변화 관찰
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 5시간

---

## 📋 실습 개요

Week 7의 Pre-integration 클래스에 공분산 전파를 추가합니다. 적분 시간에 따른 불확실성 증가를 관찰하고, Factor 가중치에 미치는 영향을 확인합니다.

---

## 🔧 빌드 및 실행

```bash
cd week9
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

---

## Step 1: 공분산 전파 포함 Pre-integration

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <cmath>

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

/**
 * Pre-integration with Covariance Propagation
 *
 * 상태: [δφ(3), δv(3), δp(3)] = 9D
 * 공분산: Σ (9×9)
 */
class PreIntegrationWithCov {
public:
    PreIntegrationWithCov(double sigma_acc, double sigma_gyro)
        : sigma_acc_(sigma_acc), sigma_gyro_(sigma_gyro), dt_sum_(0.0)
    {
        delta_R_ = Eigen::Matrix3d::Identity();
        delta_v_ = Eigen::Vector3d::Zero();
        delta_p_ = Eigen::Vector3d::Zero();
        Sigma_ = Eigen::Matrix<double, 9, 9>::Zero();
    }

    void integrate(const Eigen::Vector3d& acc, const Eigen::Vector3d& gyro,
                   double dt)
    {
        // === 1. 공분산 전파 (먼저! 현재 delta_R 사용) ===
        propagateCovariance(acc, gyro, dt);

        // === 2. Pre-integrated measurement 업데이트 ===
        delta_p_ += delta_v_ * dt + 0.5 * delta_R_ * acc * dt * dt;
        delta_v_ += delta_R_ * acc * dt;
        delta_R_ = delta_R_ * expSO3(gyro * dt);

        dt_sum_ += dt;
    }

    // 대각 공분산 출력 (불확실성 요약)
    void printUncertainty() const {
        std::cout << "    σ_rotation: ["
                  << std::sqrt(Sigma_(0,0)) << ", "
                  << std::sqrt(Sigma_(1,1)) << ", "
                  << std::sqrt(Sigma_(2,2)) << "] rad" << std::endl;
        std::cout << "    σ_velocity: ["
                  << std::sqrt(Sigma_(3,3)) << ", "
                  << std::sqrt(Sigma_(4,4)) << ", "
                  << std::sqrt(Sigma_(5,5)) << "] m/s" << std::endl;
        std::cout << "    σ_position: ["
                  << std::sqrt(Sigma_(6,6)) << ", "
                  << std::sqrt(Sigma_(7,7)) << ", "
                  << std::sqrt(Sigma_(8,8)) << "] m" << std::endl;
    }

    Eigen::Matrix<double, 9, 9> getCovariance() const { return Sigma_; }
    double getDeltaT() const { return dt_sum_; }

private:
    void propagateCovariance(const Eigen::Vector3d& acc,
                              const Eigen::Vector3d& gyro,
                              double dt)
    {
        // A 행렬 (9×9)
        Eigen::Matrix<double, 9, 9> A = Eigen::Matrix<double, 9, 9>::Identity();
        A.block<3,3>(0,0) += -skew(gyro) * dt;
        A.block<3,3>(3,0) = -delta_R_ * skew(acc) * dt;
        A.block<3,3>(6,3) = Eigen::Matrix3d::Identity() * dt;

        // B 행렬 (9×6)
        Eigen::Matrix<double, 9, 6> B = Eigen::Matrix<double, 9, 6>::Zero();
        B.block<3,3>(0,0) = -Eigen::Matrix3d::Identity() * dt;
        B.block<3,3>(3,3) = -delta_R_ * dt;

        // Q 행렬 (6×6): IMU 노이즈
        Eigen::Matrix<double, 6, 6> Q = Eigen::Matrix<double, 6, 6>::Zero();
        Q.block<3,3>(0,0) = Eigen::Matrix3d::Identity()
                           * sigma_gyro_ * sigma_gyro_ / dt;
        Q.block<3,3>(3,3) = Eigen::Matrix3d::Identity()
                           * sigma_acc_ * sigma_acc_ / dt;

        // 전파: Σ' = A·Σ·A^T + B·Q·B^T
        Sigma_ = A * Sigma_ * A.transpose() + B * Q * B.transpose();
    }

    Eigen::Matrix3d delta_R_;
    Eigen::Vector3d delta_v_, delta_p_;
    Eigen::Matrix<double, 9, 9> Sigma_;
    double sigma_acc_, sigma_gyro_;
    double dt_sum_;
};
```

---

## Step 2: 시간에 따른 불확실성 관찰

```cpp
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9: Pre-integration 공분산 전파" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // IMU 노이즈 파라미터 (전형적 MEMS IMU)
    double sigma_acc = 0.1;     // m/s²/√Hz
    double sigma_gyro = 0.01;   // rad/s/√Hz
    double dt = 0.005;          // 200Hz

    PreIntegrationWithCov preint(sigma_acc, sigma_gyro);

    // IMU 데이터: 정지 상태 (중력만)
    Eigen::Vector3d acc(0, 0, 9.81);   // 바이어스 제거됨
    Eigen::Vector3d gyro(0, 0, 0);

    // 시간별 불확실성 관찰
    double checkpoints[] = {0.1, 0.5, 1.0, 2.0, 5.0};
    int cp_idx = 0;

    int total_steps = 5.0 / dt;
    for (int i = 0; i < total_steps && cp_idx < 5; i++) {
        preint.integrate(acc, gyro, dt);

        if (std::abs(preint.getDeltaT() - checkpoints[cp_idx]) < dt/2) {
            std::cout << "  t = " << checkpoints[cp_idx] << "s:" << std::endl;
            preint.printUncertainty();
            std::cout << std::endl;
            cp_idx++;
        }
    }

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "관찰:" << std::endl;
    std::cout << "  1. σ_rotation: 시간에 비례 증가 (∝ √t)" << std::endl;
    std::cout << "  2. σ_velocity: 시간에 비례 증가 (∝ √t)" << std::endl;
    std::cout << "  3. σ_position: 시간의 1.5승에 비례 (∝ t^{3/2})" << std::endl;
    std::cout << "  → 위치 불확실성이 가장 빠르게 증가!" << std::endl;
    std::cout << "  → 짧은 키프레임 간격이 중요한 이유" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 9: Pre-integration 공분산 전파
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  t = 0.1s:
    σ_rotation: [0.001, 0.001, 0.001] rad
    σ_velocity: [0.014, 0.014, 0.014] m/s
    σ_position: [0.0005, 0.0005, 0.0005] m

  t = 0.5s:
    σ_rotation: [0.005, 0.005, 0.005] rad
    σ_velocity: [0.032, 0.032, 0.032] m/s
    σ_position: [0.005, 0.005, 0.005] m

  ...

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
관찰:
  1. σ_rotation: 시간에 비례 증가 (∝ √t)
  ...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] A, B 행렬 구성 이해
- [ ] Σ' = A·Σ·A^T + B·Q·B^T 구현
- [ ] 시간에 따른 불확실성 증가 패턴 관찰
- [ ] IMU 노이즈 파라미터 변경 → 공분산 변화 관찰
- [ ] VINS integration_base.h의 covariance 변수 확인

---

## 💡 추가 실험

1. **노이즈 크기 변경**: sigma_acc를 0.01, 0.5로 → 공분산 변화 비교
2. **dt 변경**: 200Hz → 100Hz, 400Hz → 이산화 효과 관찰
3. **회전 추가**: gyro = [0,0,0.5] → 교차 공분산 증가 관찰
4. **VINS 비교**: VINS의 공분산과 이 구현의 공분산 비교

---

**다음**: [Week 10 - 실습: IMU 적분](../week10/README.md)
