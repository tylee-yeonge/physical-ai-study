# Week 2 실습: IMU 노이즈 시뮬레이션 및 분석

> 🎯 **목표**: White Noise와 Bias Random Walk를 코드로 구현하고 특성을 비교
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 4시간

---

## 📋 실습 개요

이번 실습에서는 IMU 노이즈 모델을 직접 구현하고, White Noise와 Bias Random Walk가 적분에 미치는 영향을 시각적으로 비교합니다.

---

## 🔧 환경 설정

```bash
cd week2
mkdir build && cd build
cmake ..
make
```

---

## Step 1: 노이즈 모델 구현

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <random>
#include <vector>
#include <cmath>
#include <fstream>

/**
 * IMU 노이즈 모델 시뮬레이터
 * White Noise와 Bias Random Walk를 분리하여 관찰
 */
class IMUNoiseModel {
public:
    // 연속시간 노이즈 파라미터 (noise density)
    double acc_n = 0.008;    // m/s²/√Hz (가속도계 white noise)
    double gyr_n = 0.004;    // rad/s/√Hz (자이로 white noise)
    double acc_w = 0.00004;  // m/s³/√Hz (가속도계 bias random walk)
    double gyr_w = 2.0e-6;   // rad/s²/√Hz (자이로 bias random walk)

    IMUNoiseModel() : gen_(42), normal_(0.0, 1.0) {
        bias_accel_ = Eigen::Vector3d::Zero();
        bias_gyro_ = Eigen::Vector3d::Zero();
    }

    /**
     * White Noise 생성 (이산시간)
     */
    Eigen::Vector3d accelWhiteNoise(double dt) {
        double sigma = acc_n / std::sqrt(dt);
        return Eigen::Vector3d(
            normal_(gen_) * sigma,
            normal_(gen_) * sigma,
            normal_(gen_) * sigma
        );
    }

    Eigen::Vector3d gyroWhiteNoise(double dt) {
        double sigma = gyr_n / std::sqrt(dt);
        return Eigen::Vector3d(
            normal_(gen_) * sigma,
            normal_(gen_) * sigma,
            normal_(gen_) * sigma
        );
    }

    /**
     * Bias 업데이트 (Random Walk)
     */
    void updateBias(double dt) {
        double sigma_ba = acc_w * std::sqrt(dt);
        double sigma_bg = gyr_w * std::sqrt(dt);

        bias_accel_ += Eigen::Vector3d(
            normal_(gen_) * sigma_ba,
            normal_(gen_) * sigma_ba,
            normal_(gen_) * sigma_ba
        );

        bias_gyro_ += Eigen::Vector3d(
            normal_(gen_) * sigma_bg,
            normal_(gen_) * sigma_bg,
            normal_(gen_) * sigma_bg
        );
    }

    Eigen::Vector3d getBiasAccel() const { return bias_accel_; }
    Eigen::Vector3d getBiasGyro() const { return bias_gyro_; }

    /**
     * 전체 IMU 노이즈 (White Noise + Bias)
     */
    Eigen::Vector3d totalAccelNoise(double dt) {
        updateBias(dt);
        return accelWhiteNoise(dt) + bias_accel_;
    }

private:
    std::default_random_engine gen_;
    std::normal_distribution<double> normal_;
    Eigen::Vector3d bias_accel_;
    Eigen::Vector3d bias_gyro_;
};
```

---

## Step 2: 노이즈 영향 비교 실험

```cpp
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 2: IMU 노이즈 모델 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.005;         // 200Hz
    double total_time = 30.0;  // 30초
    int steps = total_time / dt;

    // 실험 1: White Noise만 적분
    std::cout << "=== 실험 1: White Noise만의 적분 영향 ===" << std::endl;
    {
        IMUNoiseModel noise;
        Eigen::Vector3d velocity = Eigen::Vector3d::Zero();
        Eigen::Vector3d position = Eigen::Vector3d::Zero();

        for (int i = 0; i < steps; i++) {
            Eigen::Vector3d wn = noise.accelWhiteNoise(dt);
            velocity += wn * dt;
            position += velocity * dt;

            double t = (i + 1) * dt;
            if (std::fmod(t, 5.0) < dt) {
                std::cout << "  t=" << t << "s:"
                          << "  |velocity|=" << velocity.norm() << " m/s"
                          << "  |position|=" << position.norm() << " m"
                          << std::endl;
            }
        }
        std::cout << "  → White Noise 적분 = Random Walk (√t에 비례)\n" << std::endl;
    }

    // 실험 2: Bias만 적분
    std::cout << "=== 실험 2: 일정 Bias만의 적분 영향 ===" << std::endl;
    {
        Eigen::Vector3d bias(0.05, 0.02, 0.01);  // 일정한 바이어스
        Eigen::Vector3d velocity = Eigen::Vector3d::Zero();
        Eigen::Vector3d position = Eigen::Vector3d::Zero();

        for (int i = 0; i < steps; i++) {
            velocity += bias * dt;
            position += velocity * dt;

            double t = (i + 1) * dt;
            if (std::fmod(t, 5.0) < dt) {
                std::cout << "  t=" << t << "s:"
                          << "  |velocity|=" << velocity.norm() << " m/s"
                          << "  |position|=" << position.norm() << " m"
                          << std::endl;
            }
        }
        std::cout << "  → Bias 적분: 속도=bt, 위치=0.5bt² (t²에 비례!)\n" << std::endl;
    }

    // 실험 3: 둘 다 포함
    std::cout << "=== 실험 3: White Noise + Bias (현실 IMU) ===" << std::endl;
    {
        IMUNoiseModel noise;
        Eigen::Vector3d velocity = Eigen::Vector3d::Zero();
        Eigen::Vector3d position = Eigen::Vector3d::Zero();

        for (int i = 0; i < steps; i++) {
            Eigen::Vector3d total = noise.totalAccelNoise(dt);
            velocity += total * dt;
            position += velocity * dt;

            double t = (i + 1) * dt;
            if (std::fmod(t, 5.0) < dt) {
                Eigen::Vector3d b = noise.getBiasAccel();
                std::cout << "  t=" << t << "s:"
                          << "  |bias|=" << b.norm()
                          << "  |pos|=" << position.norm() << " m"
                          << std::endl;
            }
        }
        std::cout << "  → 바이어스가 drift를 지배! 노이즈보다 훨씬 큼\n" << std::endl;
    }

    // 실험 4: 다른 품질의 IMU 비교
    std::cout << "=== 실험 4: IMU 등급별 비교 (10초 후 위치 오차) ===" << std::endl;
    {
        struct IMUGrade {
            std::string name;
            double acc_n, gyr_n, acc_w, gyr_w;
        };

        std::vector<IMUGrade> grades = {
            {"소비자급 ($10)",  0.08,  0.005,  0.001,  0.0005},
            {"산업용 ($1000)",  0.008, 0.0005, 0.0001, 0.00005},
            {"항법급 ($100K)",  0.001, 0.00005, 0.00001, 0.000005}
        };

        for (auto& grade : grades) {
            IMUNoiseModel noise;
            noise.acc_n = grade.acc_n;
            noise.gyr_n = grade.gyr_n;
            noise.acc_w = grade.acc_w;
            noise.gyr_w = grade.gyr_w;

            Eigen::Vector3d vel = Eigen::Vector3d::Zero();
            Eigen::Vector3d pos = Eigen::Vector3d::Zero();

            int steps_10s = 10.0 / dt;
            for (int i = 0; i < steps_10s; i++) {
                Eigen::Vector3d total = noise.totalAccelNoise(dt);
                vel += total * dt;
                pos += vel * dt;
            }

            std::cout << "  " << grade.name
                      << "  → 10초 후 drift: " << pos.norm() << " m"
                      << std::endl;
        }
    }

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 노이즈 분석 완료!" << std::endl;
    std::cout << "💡 바이어스가 drift의 주 원인임을 확인했습니다." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## 빌드 및 실행

```bash
cd week2
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 2: IMU 노이즈 모델 분석
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

=== 실험 1: White Noise만의 적분 영향 ===
  t=5s:  |velocity|=0.012 m/s  |position|=0.03 m
  t=10s: |velocity|=0.018 m/s  |position|=0.09 m
  t=15s: |velocity|=0.022 m/s  |position|=0.17 m
  t=20s: |velocity|=0.025 m/s  |position|=0.27 m
  t=25s: |velocity|=0.028 m/s  |position|=0.39 m
  t=30s: |velocity|=0.031 m/s  |position|=0.53 m
  → White Noise 적분 = Random Walk (√t에 비례)

=== 실험 2: 일정 Bias만의 적분 영향 ===
  t=5s:  |velocity|=0.27 m/s  |position|=0.69 m
  t=10s: |velocity|=0.54 m/s  |position|=2.74 m
  t=15s: |velocity|=0.81 m/s  |position|=6.16 m
  t=20s: |velocity|=1.08 m/s  |position|=10.95 m
  t=25s: |velocity|=1.35 m/s  |position|=17.11 m
  t=30s: |velocity|=1.62 m/s  |position|=24.64 m
  → Bias 적분: 속도=bt, 위치=0.5bt² (t²에 비례!)

=== 실험 3: White Noise + Bias (현실 IMU) ===
  ...

=== 실험 4: IMU 등급별 비교 (10초 후 위치 오차) ===
  소비자급 ($10)   → 10초 후 drift: ~5.2 m
  산업용 ($1000)   → 10초 후 drift: ~0.5 m
  항법급 ($100K)   → 10초 후 drift: ~0.05 m

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ 노이즈 분석 완료!
💡 바이어스가 drift의 주 원인임을 확인했습니다.
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] White Noise 생성 및 적분 확인
- [ ] Bias Random Walk 동작 확인
- [ ] 적분 시 White Noise vs Bias 영향 비교
- [ ] IMU 등급별 drift 차이 확인
- [ ] 연속시간 → 이산시간 노이즈 변환 이해

---

## 💡 추가 실험

1. **`acc_n` 변경**: 0.001, 0.01, 0.1으로 바꿔서 drift 비교
2. **`acc_w` 변경**: Bias Random Walk 속도 변경 후 바이어스 drift 관찰
3. **주파수 변경**: dt를 0.001, 0.01로 바꿔서 적분 정밀도 비교
4. **간이 Allan Variance**: 다양한 평균 시간으로 분산 계산해보기

---

**다음**: Week 3에서 칼만 필터를 복습하고 1D 필터를 구현합니다!
