# Week 5 실습: 1D ESKF 구현 - Nominal+Error 분리 구조 체험

> 🎯 **목표**: C++로 1D ESKF를 구현하고, EKF와 비교하여 Error-State 분리의 장점 이해
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 4시간

---

## 📋 실습 개요

1D 위치-속도 시스템에서 ESKF를 구현합니다. IMU(가속도계)로 Nominal State를 적분하고, GPS 측정으로 Error State를 보정합니다. 같은 시나리오에서 EKF와 비교하여 ESKF의 구조적 차이를 체험합니다.

---

## 🔧 빌드 및 실행

```bash
cd week5
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

---

## Step 1: ESKF 1D 클래스

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <random>

/**
 * 1D ESKF: 위치-속도 추정
 *
 * Full State: x = [p, v] (위치, 속도)
 * Nominal State: x̄ = [p̄, v̄] → IMU로 적분
 * Error State: δx = [δp, δv] → 칼만 필터로 추정
 *
 * 관계: p = p̄ + δp, v = v̄ + δv
 */
class ESKF1D {
public:
    ESKF1D(double dt) : dt_(dt) {
        // Nominal State
        x_nom_ = Eigen::Vector2d::Zero();  // [p̄, v̄]

        // Error State (항상 0 근처)
        dx_ = Eigen::Vector2d::Zero();     // [δp, δv]

        // Error State 공분산
        P_ = Eigen::Matrix2d::Identity() * 0.01;

        // 프로세스 노이즈 (IMU 노이즈)
        Q_ = Eigen::Matrix2d::Identity();
        Q_(0,0) = 0.01;  // 위치 노이즈
        Q_(1,1) = 0.1;   // 속도 노이즈 (가속도 노이즈에서 유래)

        // 측정 노이즈 (GPS)
        R_ << 4.0;  // σ = 2m

        // 측정 행렬 (위치만 관측)
        H_ << 1.0, 0.0;
    }

    // Stage 1: Nominal 예측 (IMU 적분 - 비선형 가능)
    void predictNominal(double a_meas) {
        // IMU 가속도 적분
        // p̄ += v̄·dt + 0.5·a·dt²
        x_nom_(0) += x_nom_(1) * dt_ + 0.5 * a_meas * dt_ * dt_;
        // v̄ += a·dt
        x_nom_(1) += a_meas * dt_;
    }

    // Stage 2: Error 예측 (공분산 전파)
    void predictError() {
        // Error State 전이 행렬
        // δx' = F_δ · δx + noise
        Eigen::Matrix2d F_delta;
        F_delta << 1.0, dt_,
                   0.0, 1.0;

        // δx는 Reset 후 항상 0이므로 전파할 필요 없음
        // (이론적으로 dx_ = F_delta * dx_ 이지만 0·F = 0)
        // 핵심: 공분산만 전파!
        P_ = F_delta * P_ * F_delta.transpose() + Q_;
    }

    // Stage 3: Error 업데이트 (측정 보정)
    void update(double z_gps) {
        // 잔차: 측정 - Nominal 예측
        // (Error State가 아닌 Nominal에서 계산!)
        double y = z_gps - x_nom_(0);

        // 잔차 공분산
        double S = (H_ * P_ * H_.transpose())(0,0) + R_(0,0);

        // 칼만 게인 (Error State용)
        Eigen::Vector2d K = P_ * H_.transpose() / S;

        // Error State 업데이트
        dx_ = K * y;

        // Error 공분산 업데이트
        P_ = (Eigen::Matrix2d::Identity() - K * H_) * P_;
    }

    // Reset: Error를 Nominal에 반영
    void reset() {
        // Nominal += Error
        x_nom_ += dx_;

        // Error 초기화 → 0
        dx_ = Eigen::Vector2d::Zero();

        // 핵심: 다음 스텝에서 δx ≈ 0 보장!
    }

    // 전체 상태 = Nominal + Error
    Eigen::Vector2d getFullState() const { return x_nom_ + dx_; }
    Eigen::Vector2d getNominal() const { return x_nom_; }
    Eigen::Vector2d getError() const { return dx_; }
    Eigen::Matrix2d getCovariance() const { return P_; }

    void setNominal(const Eigen::Vector2d& x) { x_nom_ = x; }

private:
    double dt_;
    Eigen::Vector2d x_nom_;  // Nominal State
    Eigen::Vector2d dx_;     // Error State
    Eigen::Matrix2d P_, Q_;
    Eigen::Matrix<double, 1, 1> R_;
    Eigen::Matrix<double, 1, 2> H_;
};
```

---

## Step 2: EKF 비교용 클래스

```cpp
/**
 * 비교용 1D EKF (같은 시나리오)
 */
class EKF1D {
public:
    EKF1D(double dt) : dt_(dt) {
        x_ = Eigen::Vector2d::Zero();
        P_ = Eigen::Matrix2d::Identity() * 0.01;
        Q_ = Eigen::Matrix2d::Identity();
        Q_(0,0) = 0.01; Q_(1,1) = 0.1;
        R_ << 4.0;
        H_ << 1.0, 0.0;
    }

    void predict(double a_meas) {
        // 상태 예측 (비선형 적분)
        x_(0) += x_(1) * dt_ + 0.5 * a_meas * dt_ * dt_;
        x_(1) += a_meas * dt_;

        // 자코비안
        Eigen::Matrix2d F;
        F << 1.0, dt_,
             0.0, 1.0;

        // 공분산 예측
        P_ = F * P_ * F.transpose() + Q_;
    }

    void update(double z_gps) {
        double y = z_gps - x_(0);
        double S = (H_ * P_ * H_.transpose())(0,0) + R_(0,0);
        Eigen::Vector2d K = P_ * H_.transpose() / S;
        x_ = x_ + K * y;
        P_ = (Eigen::Matrix2d::Identity() - K * H_) * P_;
    }

    Eigen::Vector2d getState() const { return x_; }
    void setState(const Eigen::Vector2d& x) { x_ = x; }

private:
    double dt_;
    Eigen::Vector2d x_;
    Eigen::Matrix2d P_, Q_;
    Eigen::Matrix<double, 1, 1> R_;
    Eigen::Matrix<double, 1, 2> H_;
};
```

---

## Step 3: 시뮬레이션 및 비교

```cpp
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 5: ESKF vs EKF 비교 실습" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.1;
    int steps = 100;

    // 실제: 등가속도 운동 (a=1.0 m/s²)
    double true_accel = 1.0;

    // 노이즈
    std::default_random_engine gen(42);
    std::normal_distribution<double> imu_noise(0.0, 0.5);   // IMU 노이즈
    std::normal_distribution<double> gps_noise(0.0, 2.0);   // GPS 노이즈

    // ESKF & EKF 초기화
    ESKF1D eskf(dt);
    EKF1D ekf(dt);

    double sum_eskf_err = 0, sum_ekf_err = 0;

    std::cout << "  t(s) | 실제p | Nominal | ESKF | EKF | ESKF_err | EKF_err" << std::endl;
    std::cout << "  -----|-------|---------|------|-----|----------|--------" << std::endl;

    for (int i = 0; i < steps; i++) {
        double t = (i + 1) * dt;

        // 실제 상태
        double true_p = 0.5 * true_accel * t * t;
        double true_v = true_accel * t;

        // IMU 측정 (가속도 + 노이즈)
        double a_meas = true_accel + imu_noise(gen);

        // ESKF: Stage 1+2 (Nominal 예측 + Error 공분산 전파)
        eskf.predictNominal(a_meas);
        eskf.predictError();

        // EKF: 예측
        ekf.predict(a_meas);

        // GPS 측정 (5스텝마다 = 2Hz)
        if (i % 5 == 4) {
            double z_gps = true_p + gps_noise(gen);

            // ESKF: Stage 3 (업데이트 + Reset)
            eskf.update(z_gps);
            eskf.reset();  // Error → Nominal 반영, δx → 0

            // EKF: 업데이트
            ekf.update(z_gps);
        }

        // 오차 계산
        double eskf_err = std::abs(eskf.getFullState()(0) - true_p);
        double ekf_err = std::abs(ekf.getState()(0) - true_p);
        sum_eskf_err += eskf_err;
        sum_ekf_err += ekf_err;

        if (i % 10 == 9) {
            printf("  %4.1f | %5.1f | %7.1f | %4.1f | %3.1f | %8.3f | %7.3f\n",
                   t, true_p, eskf.getNominal()(0),
                   eskf.getFullState()(0), ekf.getState()(0),
                   eskf_err, ekf_err);
        }
    }

    std::cout << "\n  평균 오차:" << std::endl;
    std::cout << "    ESKF: " << sum_eskf_err / steps << " m" << std::endl;
    std::cout << "    EKF:  " << sum_ekf_err / steps << " m" << std::endl;

    std::cout << "\n  ESKF Error State (Reset 후): " << eskf.getError().transpose() << std::endl;
    std::cout << "  → Reset 덕분에 항상 0 근처!" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "관찰 포인트:" << std::endl;
    std::cout << "  1. ESKF와 EKF 결과가 거의 동일 (1D 선형이므로)" << std::endl;
    std::cout << "  2. ESKF의 Error State는 Reset 후 항상 0" << std::endl;
    std::cout << "  3. 3D 회전에서는 ESKF가 훨씬 유리 (Over-parameterization 해결)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 5: ESKF vs EKF 비교 실습
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  t(s) | 실제p | Nominal | ESKF | EKF | ESKF_err | EKF_err
  -----|-------|---------|------|-----|----------|--------
   1.0 |   0.5 |     0.5 |  0.5 | 0.5 |    0.012 |   0.012
   2.0 |   2.0 |     2.1 |  2.0 | 2.0 |    0.034 |   0.034
   ...
  10.0 |  50.0 |    50.3 | 50.1 |50.1 |    0.089 |   0.089

  평균 오차:
    ESKF: 0.15 m
    EKF:  0.15 m

  ESKF Error State (Reset 후): 0 0
  → Reset 덕분에 항상 0 근처!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
관찰 포인트:
  1. ESKF와 EKF 결과가 거의 동일 (1D 선형이므로)
  2. ESKF의 Error State는 Reset 후 항상 0
  3. 3D 회전에서는 ESKF가 훨씬 유리 (Over-parameterization 해결)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] ESKF의 3단계 흐름 코드에서 확인: Nominal 예측 → Error 전파 → 업데이트+Reset
- [ ] Reset 후 Error State가 0인 것 확인
- [ ] ESKF vs EKF 결과 비교 (1D에서는 거의 동일)
- [ ] GPS 주파수 변경 (5스텝 → 10스텝)하여 드리프트 관찰
- [ ] IMU 노이즈 크기 변경하여 Nominal 정확도 변화 관찰

---

## 💡 추가 실험

1. **Reset 없애보기**: `eskf.reset()` 주석 처리 → Error State가 커지는 것 관찰
2. **IMU 바이어스 추가**: 상태에 `δb_a` 추가하여 3차원 ESKF 구현
3. **측정 주파수 변경**: GPS를 10Hz, 1Hz로 변경하여 Nominal 드리프트 관찰
4. **회전 추가**: 2D로 확장하여 θ 포함 → ESKF의 δθ(1D) vs EKF의 θ(1D) 비교

---

**다음**: [Week 6 - Pre-integration 필요성](../week6/README.md)
