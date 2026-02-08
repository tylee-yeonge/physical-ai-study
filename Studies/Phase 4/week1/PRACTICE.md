# Week 1 실습: IMU 센서 데이터 이해 및 시뮬레이션

> 🎯 **목표**: IMU 측정 모델을 코드로 구현하고, 적분 드리프트를 체험
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 4시간

---

## 📋 실습 개요

이번 실습에서는 IMU 센서의 측정 모델을 직접 구현하고, 가속도를 적분하여 위치를 추정해 봅니다. 적분 오차가 어떻게 누적되는지 직접 체험합니다.

---

## 🔧 환경 설정

```bash
# Eigen 설치 확인
pkg-config --modversion eigen3

# 빌드
cd week1
mkdir build && cd build
cmake ..
make
```

---

## Step 1: IMU 측정 시뮬레이터

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <random>
#include <vector>
#include <cmath>

/**
 * IMU 측정 시뮬레이터
 * 실제 운동에서 IMU가 출력할 값을 생성합니다.
 */
class IMUSimulator {
public:
    // 노이즈 파라미터
    double accel_noise_std = 0.01;   // m/s² (가속도계 노이즈)
    double gyro_noise_std = 0.001;   // rad/s (자이로 노이즈)
    double accel_bias = 0.02;        // m/s² (가속도계 바이어스)
    double gyro_bias = 0.005;        // rad/s (자이로 바이어스)

    Eigen::Vector3d gravity{0.0, 0.0, -9.81};  // 중력 벡터

    IMUSimulator() : gen_(42), noise_dist_(0.0, 1.0) {}

    /**
     * 가속도계 측정값 생성
     * a_meas = R^T * (a_true - gravity) + bias + noise
     */
    Eigen::Vector3d measureAccel(
        const Eigen::Matrix3d& R_wb,        // World→Body 회전
        const Eigen::Vector3d& accel_world   // 월드 프레임 가속도
    ) {
        // Body 프레임에서의 비중력 가속도 + 중력
        Eigen::Vector3d a_body = R_wb.transpose() * (accel_world - gravity);

        // 바이어스 추가
        Eigen::Vector3d bias(accel_bias, accel_bias * 0.5, accel_bias * 0.8);

        // 노이즈 추가
        Eigen::Vector3d noise(
            noise_dist_(gen_) * accel_noise_std,
            noise_dist_(gen_) * accel_noise_std,
            noise_dist_(gen_) * accel_noise_std
        );

        return a_body + bias + noise;
    }

    /**
     * 자이로스코프 측정값 생성
     * ω_meas = ω_true + bias + noise
     */
    Eigen::Vector3d measureGyro(
        const Eigen::Vector3d& omega_true  // 실제 각속도
    ) {
        Eigen::Vector3d bias(gyro_bias, gyro_bias * 0.3, gyro_bias * 0.6);

        Eigen::Vector3d noise(
            noise_dist_(gen_) * gyro_noise_std,
            noise_dist_(gen_) * gyro_noise_std,
            noise_dist_(gen_) * gyro_noise_std
        );

        return omega_true + bias + noise;
    }

private:
    std::default_random_engine gen_;
    std::normal_distribution<double> noise_dist_;
};
```

---

## Step 2: IMU 적분기

```cpp
/**
 * 단순 IMU 적분기
 * 가속도 → 속도 → 위치 적분
 */
class IMUIntegrator {
public:
    Eigen::Vector3d position = Eigen::Vector3d::Zero();
    Eigen::Vector3d velocity = Eigen::Vector3d::Zero();
    Eigen::Matrix3d rotation = Eigen::Matrix3d::Identity();

    Eigen::Vector3d gravity{0.0, 0.0, -9.81};

    /**
     * 1 스텝 적분 (Euler method)
     * @param accel_meas 가속도계 측정값 (body frame)
     * @param gyro_meas  자이로 측정값 (body frame)
     * @param dt         시간 간격
     */
    void integrate(
        const Eigen::Vector3d& accel_meas,
        const Eigen::Vector3d& gyro_meas,
        double dt
    ) {
        // 1. 각속도 적분 → 회전 업데이트
        //    R_{k+1} = R_k * exp(ω × dt)
        double angle = gyro_meas.norm() * dt;
        if (angle > 1e-10) {
            Eigen::Vector3d axis = gyro_meas.normalized();
            Eigen::AngleAxisd delta_rot(angle, axis);
            rotation = rotation * delta_rot.toRotationMatrix();
        }

        // 2. 가속도를 월드 프레임으로 변환
        //    a_world = R * a_body + gravity
        Eigen::Vector3d accel_world = rotation * accel_meas + gravity;

        // 3. 속도 적분
        velocity += accel_world * dt;

        // 4. 위치 적분
        position += velocity * dt;
    }
};
```

---

## Step 3: 시뮬레이션 실행

```cpp
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 1: IMU 센서 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    IMUSimulator imu;
    IMUIntegrator integrator;

    // 시뮬레이션 파라미터
    double dt = 0.005;           // 200Hz
    double total_time = 10.0;    // 10초
    int steps = total_time / dt;

    // Ground Truth: 정지 상태 (a=0, ω=0)
    Eigen::Matrix3d R_wb = Eigen::Matrix3d::Identity();
    Eigen::Vector3d a_true = Eigen::Vector3d::Zero();
    Eigen::Vector3d omega_true = Eigen::Vector3d::Zero();

    std::cout << "=== 실험 1: 정지 상태에서 IMU 적분 ===" << std::endl;
    std::cout << "GT 위치: (0, 0, 0)  GT 속도: (0, 0, 0)\n" << std::endl;

    for (int i = 0; i < steps; i++) {
        // IMU 측정값 생성
        Eigen::Vector3d a_meas = imu.measureAccel(R_wb, a_true);
        Eigen::Vector3d w_meas = imu.measureGyro(omega_true);

        // 적분
        integrator.integrate(a_meas, w_meas, dt);

        // 1초마다 출력
        double t = (i + 1) * dt;
        if (std::fmod(t, 1.0) < dt) {
            std::cout << "t=" << t << "s:"
                      << "  pos=(" << integrator.position.transpose() << ")"
                      << "  |pos|=" << integrator.position.norm() << "m"
                      << std::endl;
        }
    }

    std::cout << "\n💡 관찰: 정지 상태인데도 위치가 drift 합니다!" << std::endl;
    std::cout << "   이것이 IMU만으로 추적할 수 없는 이유입니다." << std::endl;

    // 실험 2: 바이어스 영향 확인
    std::cout << "\n=== 실험 2: 바이어스 영향 비교 ===" << std::endl;

    // 바이어스 없는 경우
    IMUSimulator imu_no_bias;
    imu_no_bias.accel_bias = 0.0;
    imu_no_bias.gyro_bias = 0.0;
    IMUIntegrator integrator_no_bias;

    // 바이어스 있는 경우
    IMUSimulator imu_with_bias;
    imu_with_bias.accel_bias = 0.05;
    imu_with_bias.gyro_bias = 0.01;
    IMUIntegrator integrator_with_bias;

    for (int i = 0; i < steps; i++) {
        Eigen::Vector3d a1 = imu_no_bias.measureAccel(R_wb, a_true);
        Eigen::Vector3d w1 = imu_no_bias.measureGyro(omega_true);
        integrator_no_bias.integrate(a1, w1, dt);

        Eigen::Vector3d a2 = imu_with_bias.measureAccel(R_wb, a_true);
        Eigen::Vector3d w2 = imu_with_bias.measureGyro(omega_true);
        integrator_with_bias.integrate(a2, w2, dt);

        double t = (i + 1) * dt;
        if (std::fmod(t, 2.0) < dt) {
            std::cout << "t=" << t << "s:"
                      << "  노이즈만=" << integrator_no_bias.position.norm() << "m"
                      << "  노이즈+바이어스=" << integrator_with_bias.position.norm() << "m"
                      << std::endl;
        }
    }

    std::cout << "\n💡 바이어스가 있으면 drift가 훨씬 빠릅니다!" << std::endl;
    std::cout << "   → 바이어스 추정이 VIO 성능의 핵심입니다." << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ IMU 시뮬레이션 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## 빌드 및 실행

```bash
cd week1
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
Week 1: IMU 센서 시뮬레이션
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

=== 실험 1: 정지 상태에서 IMU 적분 ===
GT 위치: (0, 0, 0)  GT 속도: (0, 0, 0)

t=1s:  pos=(0.01 0.005 0.003)  |pos|=0.011m
t=2s:  pos=(0.04 0.02 0.01)    |pos|=0.047m
t=3s:  pos=(0.09 0.05 0.03)    |pos|=0.11m
...
t=10s: pos=(1.2 0.5 0.3)       |pos|=1.35m

💡 관찰: 정지 상태인데도 위치가 drift 합니다!
   이것이 IMU만으로 추적할 수 없는 이유입니다.

=== 실험 2: 바이어스 영향 비교 ===
t=2s:  노이즈만=0.02m  노이즈+바이어스=0.12m
t=4s:  노이즈만=0.05m  노이즈+바이어스=0.48m
t=6s:  노이즈만=0.08m  노이즈+바이어스=1.1m
t=8s:  노이즈만=0.12m  노이즈+바이어스=1.9m
t=10s: 노이즈만=0.15m  노이즈+바이어스=3.0m

💡 바이어스가 있으면 drift가 훨씬 빠릅니다!
   → 바이어스 추정이 VIO 성능의 핵심입니다.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ IMU 시뮬레이션 완료!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] IMU 측정 모델 코드 이해 (a_meas, ω_meas)
- [ ] 정지 상태에서 가속도계 출력 확인 (~9.81)
- [ ] IMU 적분 drift 관찰
- [ ] 바이어스 유무에 따른 drift 차이 확인
- [ ] 노이즈 파라미터 변경 실험

---

## 💡 추가 실험 아이디어

1. **노이즈 크기 변경**: `accel_noise_std`를 0.001, 0.01, 0.1으로 바꿔보기
2. **주파수 변경**: `dt`를 0.001(1kHz), 0.01(100Hz), 0.05(20Hz)로 바꿔보기
3. **운동 시뮬레이션**: `a_true`를 원운동이나 직선운동으로 설정해보기
4. **중력 보정**: 측정값에서 중력을 빼고 적분하면 어떻게 되는지 확인

---

**다음**: Week 2에서 IMU 노이즈 모델을 상세히 학습합니다!
