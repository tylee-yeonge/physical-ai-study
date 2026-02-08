# Week 3 실습: 1D 칼만 필터 구현

> 🎯 **목표**: C++로 1D 칼만 필터를 구현하고, 노이즈 측정값을 필터링
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 4시간

---

## 📋 실습 개요

가속도계의 노이즈 섞인 위치 측정값을 칼만 필터로 필터링합니다. 예측-업데이트 사이클을 직접 구현하고, 칼만 게인과 공분산의 변화를 관찰합니다.

---

## 🔧 빌드 및 실행

```bash
cd week3
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

---

## Step 1: 1D 칼만 필터 클래스

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include <random>
#include <cmath>

/**
 * 1D 칼만 필터
 * 상태: [위치, 속도]
 * 측정: [위치]
 */
class KalmanFilter1D {
public:
    KalmanFilter1D(double dt, double process_noise, double meas_noise) {
        dt_ = dt;

        // 상태: [위치, 속도]
        x_ = Eigen::Vector2d(0.0, 0.0);

        // 상태 전이 행렬 F (등속 운동 모델)
        // x_k = x_{k-1} + v_{k-1} * dt
        // v_k = v_{k-1}
        F_ << 1, dt,
              0, 1;

        // 측정 행렬 H (위치만 관측)
        H_ << 1, 0;

        // 프로세스 노이즈 Q
        Q_ << dt*dt*dt*dt/4, dt*dt*dt/2,
              dt*dt*dt/2,    dt*dt;
        Q_ *= process_noise * process_noise;

        // 측정 노이즈 R
        R_ << meas_noise * meas_noise;

        // 초기 공분산 (불확실)
        P_ << 100, 0,
              0,   100;
    }

    // 예측 단계
    void predict() {
        x_ = F_ * x_;                      // 상태 예측
        P_ = F_ * P_ * F_.transpose() + Q_; // 공분산 예측
    }

    // 업데이트 단계
    void update(double z) {
        // 잔차 (innovation)
        double y = z - H_ * x_;

        // 잔차 공분산
        Eigen::Matrix<double, 1, 1> S = H_ * P_ * H_.transpose() + R_;

        // 칼만 게인
        K_ = P_ * H_.transpose() * S.inverse();

        // 상태 업데이트
        x_ = x_ + K_ * y;

        // 공분산 업데이트
        Eigen::Matrix2d I = Eigen::Matrix2d::Identity();
        P_ = (I - K_ * H_) * P_;
    }

    double getPosition() const { return x_(0); }
    double getVelocity() const { return x_(1); }
    double getPositionVariance() const { return P_(0, 0); }
    double getKalmanGain() const { return K_(0); }

private:
    double dt_;
    Eigen::Vector2d x_;          // 상태 [위치, 속도]
    Eigen::Matrix2d F_;          // 상태 전이
    Eigen::Matrix<double, 1, 2> H_;  // 측정 행렬
    Eigen::Matrix2d Q_;          // 프로세스 노이즈
    Eigen::Matrix<double, 1, 1> R_;  // 측정 노이즈
    Eigen::Matrix2d P_;          // 공분산
    Eigen::Vector2d K_;          // 칼만 게인
};
```

---

## Step 2: 시뮬레이션 실행

```cpp
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3: 1D 칼만 필터 실습" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.1;          // 10Hz
    double total_time = 10.0;
    int steps = total_time / dt;

    // 실제 움직임: 등속 운동 (속도 = 2.0 m/s)
    double true_velocity = 2.0;
    double true_position = 0.0;

    // 노이즈 생성기
    std::default_random_engine gen(42);
    std::normal_distribution<double> meas_noise(0.0, 3.0);  // 측정 노이즈 σ=3m

    // 칼만 필터 생성
    KalmanFilter1D kf(dt, 0.5, 3.0);  // 프로세스 노이즈=0.5, 측정 노이즈=3.0

    std::cout << "  t(s) | 실제위치 | 측정값 | KF추정 | 오차 | σ | K" << std::endl;
    std::cout << "  -----|--------|-------|-------|------|---|---" << std::endl;

    double sum_raw_error = 0;
    double sum_kf_error = 0;

    for (int i = 0; i < steps; i++) {
        // 실제 위치 업데이트
        true_position += true_velocity * dt;

        // 노이즈 섞인 측정값
        double z = true_position + meas_noise(gen);

        // 칼만 필터: 예측 → 업데이트
        kf.predict();
        kf.update(z);

        double kf_pos = kf.getPosition();
        double kf_err = std::abs(kf_pos - true_position);
        double raw_err = std::abs(z - true_position);
        double sigma = std::sqrt(kf.getPositionVariance());

        sum_raw_error += raw_err;
        sum_kf_error += kf_err;

        double t = (i + 1) * dt;
        if (i % 10 == 0 || i == steps - 1) {
            printf("  %4.1f | %6.2f | %6.2f | %6.2f | %4.2f | %.2f | %.3f\n",
                   t, true_position, z, kf_pos, kf_err, sigma,
                   kf.getKalmanGain());
        }
    }

    std::cout << "\n  평균 오차 비교:" << std::endl;
    std::cout << "    측정값 그대로: " << sum_raw_error / steps << " m" << std::endl;
    std::cout << "    칼만 필터:     " << sum_kf_error / steps << " m" << std::endl;
    std::cout << "    → 칼만 필터가 " << (sum_raw_error / sum_kf_error)
              << "배 정확!\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 칼만 필터 실습 완료!" << std::endl;
    std::cout << "💡 노이즈 측정값을 칼만 필터가 크게 개선합니다." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 3: 1D 칼만 필터 실습
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  t(s) | 실제위치 | 측정값 | KF추정 | 오차 | σ | K
  -----|--------|-------|-------|------|---|---
   0.1 |   0.20 |   2.34 |   1.18 | 0.98 | 2.84 | 0.917
   1.1 |   2.20 |   4.81 |   2.35 | 0.15 | 0.72 | 0.163
   2.1 |   4.20 |   6.93 |   4.24 | 0.04 | 0.68 | 0.146
   ...
  10.0 |  20.00 |  18.42 |  19.87 | 0.13 | 0.67 | 0.144

  평균 오차 비교:
    측정값 그대로: 2.41 m
    칼만 필터:     0.52 m
    → 칼만 필터가 4.6배 정확!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ 칼만 필터 실습 완료!
💡 노이즈 측정값을 칼만 필터가 크게 개선합니다.
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] 칼만 필터 클래스 코드 이해
- [ ] F, H, Q, R 행렬의 의미 파악
- [ ] 칼만 게인(K) 수렴 확인 (초기 높다가 안정)
- [ ] 공분산(P) 수렴 확인 (초기 크다가 감소)
- [ ] 측정 노이즈 변경하여 결과 비교

---

## 💡 추가 실험

1. **측정 노이즈 변경**: σ_meas를 1, 5, 10으로 바꿔서 K와 P 변화 관찰
2. **프로세스 노이즈 변경**: σ_process를 0.1, 1.0, 5.0으로 바꿔보기
3. **측정 누락**: 일부 업데이트를 건너뛰고 P 증가 관찰 (Vision 가림 시뮬레이션)
4. **2D로 확장**: x-y 평면 위치 추적 시도

---

**다음**: Week 4에서 비선형 시스템을 위한 EKF를 배웁니다!
