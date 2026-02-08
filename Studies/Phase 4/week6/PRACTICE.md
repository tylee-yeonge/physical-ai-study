# Week 6 실습: Pre-integration 필요성 체감하기 (C++)

> 🎯 **목표**: 단순 적분 vs Pre-integration 개념을 코드로 체감
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 5시간

---

## 📋 실습 개요

이번 실습에서는 단순 적분(Naive Integration)을 직접 구현하고, 왜 Pre-integration이 필요한지를 **계산 비용 비교**와 **코드**로 체감합니다.

### 환경 설정

```bash
# macOS
brew install eigen

# Ubuntu
sudo apt install libeigen3-dev
```

---

## 🔧 실습 1: IMU 데이터 구조와 단순 적분

### Step 1: IMU 데이터 구조 정의

```cpp
#include <Eigen/Dense>
#include <vector>
#include <iostream>
#include <cmath>

struct IMUData {
    double timestamp;      // 시간 (초)
    Eigen::Vector3d acc;   // 가속도 (m/s^2)
    Eigen::Vector3d gyro;  // 각속도 (rad/s)
};

// 가상 IMU 데이터 생성 (원운동 시뮬레이션)
std::vector<IMUData> generateCircularMotion(
    double duration,    // 총 시간 (초)
    double imu_freq,    // IMU 주파수 (Hz)
    double radius,      // 원운동 반지름 (m)
    double omega)       // 각속도 (rad/s)
{
    std::vector<IMUData> data;
    double dt = 1.0 / imu_freq;
    int N = static_cast<int>(duration * imu_freq);

    for (int i = 0; i < N; ++i) {
        double t = i * dt;
        IMUData imu;
        imu.timestamp = t;

        // 원운동: 구심 가속도 = omega^2 * radius
        double theta = omega * t;
        imu.acc << -radius * omega * omega * cos(theta),
                   -radius * omega * omega * sin(theta),
                   9.81;  // 중력 포함

        // 일정한 각속도 (z축 회전)
        imu.gyro << 0.0, 0.0, omega;

        data.push_back(imu);
    }

    return data;
}
```

### Step 2: 단순 적분 구현

```cpp
struct State {
    Eigen::Vector3d position;
    Eigen::Vector3d velocity;
    Eigen::Quaterniond rotation;

    State() : position(Eigen::Vector3d::Zero()),
              velocity(Eigen::Vector3d::Zero()),
              rotation(Eigen::Quaterniond::Identity()) {}
};

// 단순 적분: 절대 좌표계에서 적분
State naiveIntegrate(
    const State& initial_state,
    const std::vector<IMUData>& imu_data,
    const Eigen::Vector3d& gravity,
    double dt)
{
    State state = initial_state;

    for (size_t i = 0; i < imu_data.size(); ++i) {
        const auto& imu = imu_data[i];

        // 월드 좌표계에서 가속도 (중력 제거)
        Eigen::Vector3d acc_world =
            state.rotation * (imu.acc - Eigen::Vector3d(0, 0, 9.81))
            + gravity;

        // 위치 업데이트
        state.position += state.velocity * dt
                        + 0.5 * acc_world * dt * dt;

        // 속도 업데이트
        state.velocity += acc_world * dt;

        // 회전 업데이트
        double angle = imu.gyro.norm() * dt;
        if (angle > 1e-10) {
            Eigen::Vector3d axis = imu.gyro.normalized();
            Eigen::Quaterniond dq(Eigen::AngleAxisd(angle, axis));
            state.rotation = (state.rotation * dq).normalized();
        }
    }

    return state;
}
```

---

## 🔧 실습 2: 재적분 비용 측정

### 최적화 시뮬레이션: 포즈 변경 시 재적분 비용

```cpp
#include <chrono>

void measureReintegrationCost() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "재적분 비용 측정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    Eigen::Vector3d gravity(0, 0, -9.81);
    double dt = 0.005;  // 200Hz

    // IMU 데이터 생성 (1초, 200Hz = 200개 데이터)
    auto imu_data = generateCircularMotion(1.0, 200, 1.0, 1.0);

    int num_keyframes = 20;
    int num_optimization_iters = 30;

    // [방법 1] 단순 적분: 매 최적화 반복마다 재적분
    auto start1 = std::chrono::high_resolution_clock::now();

    int total_integrations_naive = 0;
    for (int opt = 0; opt < num_optimization_iters; ++opt) {
        for (int kf = 0; kf < num_keyframes - 1; ++kf) {
            State init;
            // 최적화 반복마다 초기 포즈가 바뀐다고 가정
            init.position = Eigen::Vector3d::Random() * 0.01;
            naiveIntegrate(init, imu_data, gravity, dt);
            total_integrations_naive += imu_data.size();
        }
    }

    auto end1 = std::chrono::high_resolution_clock::now();
    double time_naive = std::chrono::duration<double, std::milli>(
        end1 - start1).count();

    // [방법 2] Pre-integration: 1회만 적분
    auto start2 = std::chrono::high_resolution_clock::now();

    int total_integrations_preint = 0;
    for (int kf = 0; kf < num_keyframes - 1; ++kf) {
        State init;
        naiveIntegrate(init, imu_data, gravity, dt);
        total_integrations_preint += imu_data.size();
    }
    // 최적화 반복에서는 캐싱된 값 재활용 (행렬 곱만)
    for (int opt = 0; opt < num_optimization_iters; ++opt) {
        for (int kf = 0; kf < num_keyframes - 1; ++kf) {
            // 단순 행렬 연산 (Pre-integration 결과 재활용)
            Eigen::Vector3d p_j = Eigen::Vector3d::Random();  // 시뮬레이션
        }
    }

    auto end2 = std::chrono::high_resolution_clock::now();
    double time_preint = std::chrono::duration<double, std::milli>(
        end2 - start2).count();

    std::cout << "\n[단순 적분]" << std::endl;
    std::cout << "  총 적분 횟수: " << total_integrations_naive << std::endl;
    std::cout << "  소요 시간: " << time_naive << " ms" << std::endl;

    std::cout << "\n[Pre-integration]" << std::endl;
    std::cout << "  총 적분 횟수: " << total_integrations_preint << std::endl;
    std::cout << "  소요 시간: " << time_preint << " ms" << std::endl;

    std::cout << "\n[비교]" << std::endl;
    std::cout << "  적분 횟수 비율: "
              << (double)total_integrations_naive / total_integrations_preint
              << "x" << std::endl;
    std::cout << "  시간 비율: " << time_naive / time_preint
              << "x" << std::endl;
}
```

---

## 🔧 실습 3: 상대 좌표 vs 절대 좌표 비교

### 포즈 변경 시 결과 차이 확인

```cpp
void compareAbsoluteVsRelative() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "절대 좌표 vs 상대 좌표" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    Eigen::Vector3d gravity(0, 0, -9.81);
    double dt = 0.005;

    auto imu_data = generateCircularMotion(0.1, 200, 1.0, 1.0);

    // 초기 포즈 1
    State init1;
    init1.position = Eigen::Vector3d(1.0, 2.0, 0.0);
    init1.velocity = Eigen::Vector3d(0.5, 0.0, 0.0);
    State result1 = naiveIntegrate(init1, imu_data, gravity, dt);

    // 초기 포즈 2 (약간 다름)
    State init2;
    init2.position = Eigen::Vector3d(1.01, 2.005, 0.001);
    init2.velocity = Eigen::Vector3d(0.51, 0.002, 0.0);
    State result2 = naiveIntegrate(init2, imu_data, gravity, dt);

    // 절대 좌표: 결과가 다름
    std::cout << "\n[절대 좌표 결과]" << std::endl;
    std::cout << "  포즈1 → 결과: " << result1.position.transpose() << std::endl;
    std::cout << "  포즈2 → 결과: " << result2.position.transpose() << std::endl;
    std::cout << "  차이: " << (result1.position - result2.position).norm()
              << " m" << std::endl;

    // 상대 좌표: 같아야 함 (Pre-integration 효과)
    Eigen::Vector3d relative1 =
        init1.rotation.inverse() * (result1.position - init1.position
        - init1.velocity * (0.1) - 0.5 * gravity * 0.1 * 0.1);
    Eigen::Vector3d relative2 =
        init2.rotation.inverse() * (result2.position - init2.position
        - init2.velocity * (0.1) - 0.5 * gravity * 0.1 * 0.1);

    std::cout << "\n[상대 좌표 (Pre-integration)]" << std::endl;
    std::cout << "  포즈1 → 상대: " << relative1.transpose() << std::endl;
    std::cout << "  포즈2 → 상대: " << relative2.transpose() << std::endl;
    std::cout << "  차이: " << (relative1 - relative2).norm()
              << " m (거의 0이어야 함)" << std::endl;
}
```

---

## 빌드 및 실행

```bash
cd week6
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

### 예상 출력 (quiz_medium)

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
재적분 비용 측정
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

[단순 적분]
  총 적분 횟수: 114000
  소요 시간: ~150 ms

[Pre-integration]
  총 적분 횟수: 3800
  소요 시간: ~8 ms

[비교]
  적분 횟수 비율: 30x
  시간 비율: ~19x
```

---

## ✅ 체크리스트

- [ ] IMU 데이터 구조 정의
- [ ] 단순 적분 구현 (naiveIntegrate)
- [ ] 재적분 비용 측정
- [ ] 절대/상대 좌표 비교
- [ ] Pre-integration 필요성 체감
- [ ] quiz_easy.cpp 풀기
- [ ] quiz_medium.cpp 풀기

---

## 💡 핵심 정리

1. **단순 적분은 포즈 의존적** - 초기 포즈가 바뀌면 처음부터 다시
2. **Pre-integration은 포즈 독립적** - 상대 변위로 계산하면 재활용 가능
3. **계산 비용 차이는 최적화 반복 횟수에 비례** - 30x 이상 차이 가능
4. **상대 좌표는 초기 포즈에 무관** - 출발점이 바뀌어도 상대 이동은 동일

---

**다음**: Week 7에서 Pre-integration의 수식을 제대로 배웁니다!
