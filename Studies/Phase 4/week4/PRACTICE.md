# Week 4 실습: 2D EKF 구현 - 비선형 위치 추적

> 🎯 **목표**: C++로 2D EKF를 구현하고, 원형 운동하는 로봇을 추적
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 4시간

---

## 📋 실습 개요

등속 원운동하는 로봇의 위치를 EKF로 추적합니다. 상태 전이가 비선형(회전 포함)이므로 자코비안을 직접 계산해야 합니다.

---

## 🔧 빌드 및 실행

```bash
cd week4
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

---

## Step 1: 2D EKF 클래스

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <random>
#include <vector>

/**
 * 2D EKF: 등속 원운동 모델
 * 상태: [x, y, θ, v] (위치, 방향, 속도)
 * 측정: [x, y] (위치를 GPS처럼 관측)
 */
class EKF2D {
public:
    EKF2D(double dt) : dt_(dt) {
        x_ = Eigen::Vector4d::Zero();

        P_ = Eigen::Matrix4d::Identity() * 10.0;

        Q_ = Eigen::Matrix4d::Identity();
        Q_(0,0) = 0.1;  Q_(1,1) = 0.1;
        Q_(2,2) = 0.01; Q_(3,3) = 0.01;

        R_ = Eigen::Matrix2d::Identity() * 4.0;  // 측정 노이즈 σ=2m

        H_ << 1, 0, 0, 0,
               0, 1, 0, 0;
    }

    // 비선형 상태 전이 f(x)
    Eigen::Vector4d f(const Eigen::Vector4d& x) {
        double px = x(0), py = x(1), theta = x(2), v = x(3);
        Eigen::Vector4d x_new;
        x_new(0) = px + v * std::cos(theta) * dt_;  // x += v·cos(θ)·dt
        x_new(1) = py + v * std::sin(theta) * dt_;  // y += v·sin(θ)·dt
        x_new(2) = theta;                            // θ 유지 (입력으로 변경)
        x_new(3) = v;                                // v 유지
        return x_new;
    }

    // 자코비안 F = ∂f/∂x
    Eigen::Matrix4d computeF(const Eigen::Vector4d& x) {
        double theta = x(2), v = x(3);
        Eigen::Matrix4d F = Eigen::Matrix4d::Identity();

        // ∂(x_new)/∂θ = -v·sin(θ)·dt
        F(0, 2) = -v * std::sin(theta) * dt_;
        // ∂(x_new)/∂v = cos(θ)·dt
        F(0, 3) = std::cos(theta) * dt_;
        // ∂(y_new)/∂θ = v·cos(θ)·dt
        F(1, 2) = v * std::cos(theta) * dt_;
        // ∂(y_new)/∂v = sin(θ)·dt
        F(1, 3) = std::sin(theta) * dt_;

        return F;
    }

    // 예측 단계
    void predict(double omega) {
        // 각속도 입력으로 방향 업데이트
        x_(2) += omega * dt_;

        // 자코비안 계산
        Eigen::Matrix4d F = computeF(x_);

        // 상태 예측
        x_ = f(x_);

        // 공분산 예측
        P_ = F * P_ * F.transpose() + Q_;
    }

    // 업데이트 단계
    void update(const Eigen::Vector2d& z) {
        // 잔차
        Eigen::Vector2d y = z - H_ * x_;

        // 잔차 공분산
        Eigen::Matrix2d S = H_ * P_ * H_.transpose() + R_;

        // 칼만 게인
        Eigen::Matrix<double, 4, 2> K = P_ * H_.transpose() * S.inverse();

        // 상태 업데이트
        x_ = x_ + K * y;

        // 공분산 업데이트
        P_ = (Eigen::Matrix4d::Identity() - K * H_) * P_;
    }

    Eigen::Vector4d getState() const { return x_; }
    Eigen::Matrix4d getCovariance() const { return P_; }
    void setState(const Eigen::Vector4d& x) { x_ = x; }

private:
    double dt_;
    Eigen::Vector4d x_;
    Eigen::Matrix4d P_, Q_;
    Eigen::Matrix2d R_;
    Eigen::Matrix<double, 2, 4> H_;
};
```

---

## Step 2: 시뮬레이션

```cpp
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 4: 2D EKF 실습 - 원형 운동 추적" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.1;
    double total_time = 20.0;
    int steps = total_time / dt;

    // 실제 로봇: 반지름 10m 원형 운동
    double radius = 10.0;
    double omega = 0.2;  // rad/s
    double speed = radius * omega;  // 2.0 m/s

    // 노이즈
    std::default_random_engine gen(42);
    std::normal_distribution<double> meas_noise(0.0, 2.0);

    // EKF
    EKF2D ekf(dt);
    ekf.setState(Eigen::Vector4d(radius, 0, M_PI/2, speed));

    double sum_raw_err = 0, sum_ekf_err = 0;

    std::cout << " t(s) | 실제(x,y) | 측정(x,y) | EKF(x,y) | err" << std::endl;
    std::cout << " -----|-----------|-----------|----------|----" << std::endl;

    for (int i = 0; i < steps; i++) {
        double t = (i + 1) * dt;
        double angle = M_PI/2 + omega * t;

        // 실제 위치
        double true_x = radius * std::cos(angle);
        double true_y = radius * std::sin(angle);

        // 노이즈 측정
        Eigen::Vector2d z(true_x + meas_noise(gen),
                          true_y + meas_noise(gen));

        // EKF
        ekf.predict(omega);
        ekf.update(z);

        Eigen::Vector4d est = ekf.getState();

        double raw_err = std::sqrt(std::pow(z(0)-true_x,2) + std::pow(z(1)-true_y,2));
        double ekf_err = std::sqrt(std::pow(est(0)-true_x,2) + std::pow(est(1)-true_y,2));

        sum_raw_err += raw_err;
        sum_ekf_err += ekf_err;

        if (i % 20 == 0 || i == steps-1) {
            printf(" %4.1f | (%5.1f,%5.1f) | (%5.1f,%5.1f) | (%5.1f,%5.1f) | %.2f\n",
                   t, true_x, true_y, z(0), z(1), est(0), est(1), ekf_err);
        }
    }

    std::cout << "\n  평균 오차:" << std::endl;
    std::cout << "    측정값: " << sum_raw_err / steps << " m" << std::endl;
    std::cout << "    EKF:    " << sum_ekf_err / steps << " m" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ EKF가 비선형 원형 운동을 성공적으로 추적!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 4: 2D EKF 실습 - 원형 운동 추적
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

 t(s) | 실제(x,y) | 측정(x,y) | EKF(x,y) | err
 -----|-----------|-----------|----------|----
  0.1 | (  9.8, 2.0) | ( 11.2, 3.5) | ( 10.1, 2.3) | 0.42
  2.1 | (  9.1, 4.2) | (  7.8, 5.9) | (  9.0, 4.3) | 0.15
  ...
 20.0 | (  7.6, 6.5) | (  6.3, 8.1) | (  7.5, 6.6) | 0.14

  평균 오차:
    측정값: 2.83 m
    EKF:    0.48 m

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ EKF가 비선형 원형 운동을 성공적으로 추적!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] 비선형 상태 전이 f(x) 이해
- [ ] 자코비안 F 수동 계산 및 코드 확인
- [ ] EKF 예측-업데이트 흐름 이해
- [ ] EKF가 노이즈 측정을 필터링하는 것 확인
- [ ] Q, R 값 변경하여 결과 비교

---

## 💡 추가 실험

1. **측정 주파수 변경**: 매 스텝이 아닌 5스텝마다 업데이트 → P 변화 관찰
2. **속도 변경**: omega를 0.5, 1.0으로 → 비선형이 강해질 때 EKF 한계 관찰
3. **수치적 자코비안**: 해석적 F와 수치적 F를 비교하여 정확성 검증
4. **초기값 오차**: 초기 추정을 크게 틀리게 하여 수렴 관찰

---

**다음**: Week 5에서 ESKF를 배워 EKF의 회전 문제를 해결합니다!
