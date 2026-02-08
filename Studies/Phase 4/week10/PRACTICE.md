# Week 10 실습: IMU 적분과 드리프트 관찰

> 🎯 **목표**: C++로 IMU 직접 적분을 구현하고, 노이즈/바이어스에 의한 드리프트 체험
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 5시간

---

## 📋 실습 개요

시뮬레이션된 원형 운동에서 IMU 데이터를 생성하고, 순수 적분으로 궤적을 추정합니다. 노이즈와 바이어스를 추가하며 드리프트가 어떻게 발생하는지 직접 관찰합니다.

---

## 🔧 빌드 및 실행

```bash
cd week10
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

---

## Step 1: SO(3) 유틸리티 + IMU 적분기

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <random>
#include <vector>

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
 * IMU 직접 적분기
 * 상태: R (회전), v (속도), p (위치)
 */
class IMUIntegrator {
public:
    IMUIntegrator() {
        R_ = Eigen::Matrix3d::Identity();
        v_ = Eigen::Vector3d::Zero();
        p_ = Eigen::Vector3d::Zero();
        gravity_ << 0, 0, -9.81;
    }

    void setInitialState(const Eigen::Matrix3d& R,
                          const Eigen::Vector3d& v,
                          const Eigen::Vector3d& p) {
        R_ = R; v_ = v; p_ = p;
    }

    // IMU 데이터 한 스텝 적분
    void integrate(const Eigen::Vector3d& a_meas,
                   const Eigen::Vector3d& w_meas,
                   const Eigen::Vector3d& b_a,
                   const Eigen::Vector3d& b_g,
                   double dt)
    {
        // 바이어스 제거
        Eigen::Vector3d a = a_meas - b_a;
        Eigen::Vector3d w = w_meas - b_g;

        // 월드 프레임 가속도 (중력 보상)
        Eigen::Vector3d a_world = R_ * a + gravity_;

        // 위치 업데이트 (현재 속도 사용)
        p_ += v_ * dt + 0.5 * a_world * dt * dt;

        // 속도 업데이트
        v_ += a_world * dt;

        // 회전 업데이트
        R_ = R_ * expSO3(w * dt);
    }

    Eigen::Matrix3d R() const { return R_; }
    Eigen::Vector3d v() const { return v_; }
    Eigen::Vector3d p() const { return p_; }

private:
    Eigen::Matrix3d R_;
    Eigen::Vector3d v_, p_, gravity_;
};
```

---

## Step 2: 시뮬레이션 (원형 운동 + IMU 생성)

```cpp
/**
 * 시뮬레이션: 수평면에서 등속 원운동
 * 반지름 5m, 각속도 0.5 rad/s, 10초
 */
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 10: IMU 적분 드리프트 실험" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.005;  // 200Hz
    double total_time = 10.0;
    int steps = total_time / dt;
    double radius = 5.0;
    double omega_z = 0.5;  // z축 회전 rad/s
    double speed = radius * omega_z;  // 2.5 m/s

    Eigen::Vector3d gravity(0, 0, -9.81);

    // 노이즈 생성기
    std::default_random_engine gen(42);
    std::normal_distribution<double> acc_noise(0, 0.1);    // σ=0.1 m/s²
    std::normal_distribution<double> gyro_noise(0, 0.01);  // σ=0.01 rad/s

    // 바이어스 (실험 3에서 사용)
    Eigen::Vector3d b_a_true(0.02, 0.0, 0.0);   // x방향 0.02 m/s²
    Eigen::Vector3d b_g_true(0.0, 0.0, 0.001);  // z축 0.001 rad/s

    // === 실험 1: 노이즈 없이 (이상적) ===
    IMUIntegrator integ1;
    integ1.setInitialState(Eigen::Matrix3d::Identity(),
                           Eigen::Vector3d(0, speed, 0),
                           Eigen::Vector3d(radius, 0, 0));

    // === 실험 2: 노이즈 추가 ===
    IMUIntegrator integ2;
    integ2.setInitialState(Eigen::Matrix3d::Identity(),
                           Eigen::Vector3d(0, speed, 0),
                           Eigen::Vector3d(radius, 0, 0));

    // === 실험 3: 노이즈 + 바이어스 ===
    IMUIntegrator integ3;
    integ3.setInitialState(Eigen::Matrix3d::Identity(),
                           Eigen::Vector3d(0, speed, 0),
                           Eigen::Vector3d(radius, 0, 0));

    double err1_sum = 0, err2_sum = 0, err3_sum = 0;

    for (int i = 0; i < steps; i++) {
        double t = (i + 1) * dt;
        double angle = omega_z * t;

        // Ground Truth
        Eigen::Matrix3d R_true = expSO3(Eigen::Vector3d(0, 0, angle));
        Eigen::Vector3d p_true(radius * std::cos(angle),
                                radius * std::sin(angle), 0);

        // 원운동의 가속도 (구심 가속도: -ω²r 방향)
        Eigen::Vector3d a_world(-radius * omega_z * omega_z * std::cos(angle),
                                 -radius * omega_z * omega_z * std::sin(angle), 0);

        // IMU 측정 생성 (body frame)
        Eigen::Vector3d a_body = R_true.transpose() * (a_world - gravity);
        Eigen::Vector3d w_body(0, 0, omega_z);

        // 노이즈 추가
        Eigen::Vector3d n_a(acc_noise(gen), acc_noise(gen), acc_noise(gen));
        Eigen::Vector3d n_g(gyro_noise(gen), gyro_noise(gen), gyro_noise(gen));

        // 실험 1: 이상적 (노이즈/바이어스 없음)
        integ1.integrate(a_body, w_body,
                         Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), dt);

        // 실험 2: 노이즈만
        integ2.integrate(a_body + n_a, w_body + n_g,
                         Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), dt);

        // 실험 3: 노이즈 + 바이어스 (바이어스 모르는 상태로 적분)
        integ3.integrate(a_body + n_a + b_a_true, w_body + n_g + b_g_true,
                         Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), dt);

        double err1 = (integ1.p() - p_true).norm();
        double err2 = (integ2.p() - p_true).norm();
        double err3 = (integ3.p() - p_true).norm();
        err1_sum += err1; err2_sum += err2; err3_sum += err3;

        if (i % 400 == 399) {
            printf("  t=%4.1fs | 이상적: %6.3fm | 노이즈: %6.3fm | +바이어스: %6.3fm\n",
                   t, err1, err2, err3);
        }
    }

    std::cout << "\n  평균 오차:" << std::endl;
    printf("    이상적:     %6.4f m\n", err1_sum / steps);
    printf("    노이즈:     %6.4f m\n", err2_sum / steps);
    printf("    +바이어스:  %6.4f m\n", err3_sum / steps);

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "결론:" << std::endl;
    std::cout << "  1. 노이즈 없으면 적분이 정확 (수치 오차만)" << std::endl;
    std::cout << "  2. 노이즈 추가 → 시간에 따라 드리프트 증가" << std::endl;
    std::cout << "  3. 바이어스 추가 → 빠르게 발산!" << std::endl;
    std::cout << "  → IMU만으로는 몇 초 내에 쓸모없어짐" << std::endl;
    std::cout << "  → Vision 보정이 반드시 필요!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 10: IMU 적분 드리프트 실험
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  t= 2.0s | 이상적:  0.001m | 노이즈:  0.045m | +바이어스:  0.089m
  t= 4.0s | 이상적:  0.002m | 노이즈:  0.198m | +바이어스:  0.652m
  t= 6.0s | 이상적:  0.003m | 노이즈:  0.531m | +바이어스:  2.134m
  t= 8.0s | 이상적:  0.005m | 노이즈:  0.987m | +바이어스:  5.421m
  t=10.0s | 이상적:  0.007m | 노이즈:  1.654m | +바이어스: 12.387m

  평균 오차:
    이상적:     0.0031 m
    노이즈:     0.4523 m
    +바이어스:  3.2145 m

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
결론:
  1. 노이즈 없으면 적분이 정확 (수치 오차만)
  2. 노이즈 추가 → 시간에 따라 드리프트 증가
  3. 바이어스 추가 → 빠르게 발산!
  → IMU만으로는 몇 초 내에 쓸모없어짐
  → Vision 보정이 반드시 필요!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] IMU 직접 적분 코드 이해 (R, v, p 업데이트)
- [ ] 중력 보상 과정 이해 (a_world = R·a + g)
- [ ] 노이즈 없이 → 정확한 적분 확인
- [ ] 노이즈 추가 → 드리프트 관찰
- [ ] 바이어스 추가 → 이차 발산 관찰
- [ ] 드리프트 크기에 대한 직관 획득

---

## 💡 추가 실험

1. **노이즈 크기 변경**: σ_acc = 0.01, 0.5 → 드리프트 비교
2. **바이어스 크기 변경**: b_g = 0.01 rad/s → 중력 누출 심화 관찰
3. **적분 시간 연장**: 30초, 60초 → 발산 정도 확인
4. **IMU 주파수 변경**: 100Hz, 400Hz → 적분 정확도 비교
5. **자이로 바이어스만**: b_a=0, b_g≠0 → 중력 누출 단독 효과

---

**다음**: [Week 11 - VIO 초기화 문제](../week11/README.md)
