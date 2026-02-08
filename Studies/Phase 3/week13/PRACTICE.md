# Week 13 실습: 스케일 복구 방법 비교 분석

> 🎯 **목표**: 스케일 복구 방법들의 특성을 코드로 비교
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 3시간

---

## 📋 실습 개요

Week 13 실습은 시뮬레이션 기반으로 3가지 스케일 복구 방법을 비교합니다:

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | Monocular: 스케일 드리프트 관찰 | 필수 | 1시간 |
| 2 | Stereo: baseline으로 스케일 유지 | 필수 | 1시간 |
| 3 | IMU 융합: 가속도 적분으로 스케일 복구 | 필수 | 1시간 |

---

## 🔧 환경 설정

```bash
# Eigen (필수)
# macOS
brew install eigen

# Ubuntu
sudo apt install libeigen3-dev
```

---

## 프로젝트 구조

```
week13/
├── CMakeLists.txt
├── README.md
├── PRACTICE.md          ← 지금 읽고 있는 파일
├── scale_recovery_sim.cpp  ← 이 실습의 메인 코드
├── quiz_easy.cpp
├── quiz_medium.cpp
└── quiz_solutions/
    ├── easy_sol.cpp
    └── medium_sol.cpp
```

---

## 전체 시뮬레이션 코드: scale_recovery_sim.cpp

아래 코드를 `scale_recovery_sim.cpp`로 저장하세요.

```cpp
/**
 * Week 13 실습: 스케일 복구 방법 비교 시뮬레이션
 *
 * 3가지 방법을 비교합니다:
 *   1. Monocular VO: 스케일 드리프트 발생
 *   2. Stereo VO: baseline으로 절대 스케일 유지
 *   3. VIO (간소화): IMU 가속도 적분으로 스케일 복구
 *
 * 목적: 왜 IMU가 필요한지 직관적으로 이해하기
 */

#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip>

using namespace Eigen;
using namespace std;

// ============================================
// 시뮬레이션 설정
// ============================================
struct SimConfig {
    int num_steps = 50;          // 시뮬레이션 스텝 수
    double dt = 0.1;             // 시간 간격 (초)
    double true_velocity = 1.0;  // 실제 속도 (m/s)

    // 노이즈 파라미터
    double mono_scale_noise = 0.03;   // Monocular 스케일 노이즈 (3%)
    double stereo_depth_noise = 0.01; // Stereo 깊이 노이즈 (1%)
    double imu_accel_bias = 0.01;     // IMU 가속도 바이어스 (m/s²)
    double imu_accel_noise = 0.05;    // IMU 가속도 노이즈 (m/s²)

    // Stereo 파라미터
    double focal_length = 500.0;  // 초점 거리 (pixel)
    double baseline = 0.12;       // 베이스라인 (m)
};

// ============================================
// 1. Monocular VO 시뮬레이션
// ============================================
struct MonoResult {
    vector<Vector3d> positions;
    vector<double> scale_errors;
};

MonoResult simulateMonocular(const SimConfig& config) {
    MonoResult result;
    mt19937 gen(42);
    normal_distribution<double> noise(0, config.mono_scale_noise);

    Vector3d position(0, 0, 0);
    double accumulated_scale = 1.0;

    result.positions.push_back(position);

    for (int i = 0; i < config.num_steps; i++) {
        // 실제 이동량
        double true_displacement = config.true_velocity * config.dt;

        // Monocular: 매 프레임 스케일에 노이즈 추가
        // 스케일을 정확히 알 수 없으므로 상대 스케일만 추정
        double scale_factor = 1.0 + noise(gen);
        accumulated_scale *= scale_factor;

        // 추정된 이동량 (스케일 드리프트 포함)
        double estimated_displacement = true_displacement * accumulated_scale;

        // x 방향으로 직선 이동 (단순화)
        position(0) += estimated_displacement;
        result.positions.push_back(position);

        // 스케일 오차 기록
        double scale_error = abs(accumulated_scale - 1.0) / 1.0 * 100.0;
        result.scale_errors.push_back(scale_error);
    }

    return result;
}

// ============================================
// 2. Stereo VO 시뮬레이션
// ============================================
struct StereoResult {
    vector<Vector3d> positions;
    vector<double> depth_errors;
};

StereoResult simulateStereo(const SimConfig& config) {
    StereoResult result;
    mt19937 gen(123);
    normal_distribution<double> noise(0, config.stereo_depth_noise);

    Vector3d position(0, 0, 0);

    result.positions.push_back(position);

    for (int i = 0; i < config.num_steps; i++) {
        double true_displacement = config.true_velocity * config.dt;

        // Stereo: depth = f * b / disparity
        // disparity에 약간의 노이즈가 있어도 스케일은 유지됨
        double depth_scale = 1.0 + noise(gen);
        double estimated_displacement = true_displacement * depth_scale;

        position(0) += estimated_displacement;
        result.positions.push_back(position);

        // 깊이 오차 기록 (스케일 누적 없음, 매번 독립적)
        double depth_error = abs(depth_scale - 1.0) / 1.0 * 100.0;
        result.depth_errors.push_back(depth_error);
    }

    return result;
}

// ============================================
// 3. VIO (간소화) 시뮬레이션
// ============================================
struct VIOResult {
    vector<Vector3d> positions;
    vector<double> scale_errors;

    // IMU만 사용 시 결과 (비교용)
    vector<Vector3d> imu_only_positions;
};

VIOResult simulateVIO(const SimConfig& config) {
    VIOResult result;
    mt19937 gen(456);
    normal_distribution<double> accel_noise(0, config.imu_accel_noise);
    normal_distribution<double> vision_noise(0, config.mono_scale_noise);

    // VIO 상태
    Vector3d vio_position(0, 0, 0);
    Vector3d vio_velocity(config.true_velocity, 0, 0);

    // IMU만 사용 시 상태
    Vector3d imu_position(0, 0, 0);
    Vector3d imu_velocity(config.true_velocity, 0, 0);

    result.positions.push_back(vio_position);
    result.imu_only_positions.push_back(imu_position);

    for (int i = 0; i < config.num_steps; i++) {
        // 실제 가속도 (등속 운동 → 가속도 = 0)
        Vector3d true_accel(0, 0, 0);

        // IMU 측정: 바이어스 + 노이즈
        Vector3d measured_accel = true_accel;
        measured_accel(0) += config.imu_accel_bias + accel_noise(gen);
        measured_accel(1) += accel_noise(gen) * 0.1;
        measured_accel(2) += accel_noise(gen) * 0.1;

        // --- IMU만 사용 (비교용) ---
        imu_velocity += measured_accel * config.dt;
        imu_position += imu_velocity * config.dt;
        result.imu_only_positions.push_back(imu_position);

        // --- VIO: Vision + IMU 융합 (간소화) ---
        // Vision에서 방향과 상대 위치 추정
        double vision_scale = 1.0 + vision_noise(gen);
        double vision_displacement = config.true_velocity * config.dt * vision_scale;

        // IMU에서 스케일 정보 추출
        // 간소화: IMU로 추정한 이동량과 Vision 이동량을 가중 평균
        double imu_displacement = (imu_velocity.norm()) * config.dt;

        // 융합 (간소화된 가중 평균)
        // Vision 비중 0.7, IMU 비중 0.3 (단순 예시)
        double fused_displacement = 0.7 * vision_displacement + 0.3 * imu_displacement;

        // 실제에서는 EKF 또는 최적화 기반 융합 사용
        vio_position(0) += fused_displacement;
        result.positions.push_back(vio_position);

        // 스케일 오차 계산
        double true_position = (i + 1) * config.true_velocity * config.dt;
        double scale_error = abs(vio_position(0) - true_position) / true_position * 100.0;
        result.scale_errors.push_back(scale_error);
    }

    return result;
}

// ============================================
// 결과 출력
// ============================================
void printResults(const SimConfig& config,
                  const MonoResult& mono,
                  const StereoResult& stereo,
                  const VIOResult& vio) {

    double true_final = config.num_steps * config.true_velocity * config.dt;

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  스케일 복구 방법 비교 시뮬레이션 결과" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "시뮬레이션 설정:" << endl;
    cout << "  스텝 수: " << config.num_steps << endl;
    cout << "  시간 간격: " << config.dt << " s" << endl;
    cout << "  실제 속도: " << config.true_velocity << " m/s" << endl;
    cout << "  실제 최종 위치: " << true_final << " m\n" << endl;

    // 최종 위치 비교
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  최종 위치 비교" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double mono_final = mono.positions.back()(0);
    double stereo_final = stereo.positions.back()(0);
    double vio_final = vio.positions.back()(0);
    double imu_final = vio.imu_only_positions.back()(0);

    cout << fixed << setprecision(3);

    cout << "  방법              │ 최종 위치 (m) │ 오차 (m) │ 오차 (%)" << endl;
    cout << "  ──────────────────┼──────────────┼──────────┼─────────" << endl;
    printf("  실제 (Ground Truth)│    %8.3f   │   %6.3f  │  %5.1f%%\n",
           true_final, 0.0, 0.0);
    printf("  Monocular VO      │    %8.3f   │   %6.3f  │  %5.1f%%\n",
           mono_final, abs(mono_final - true_final),
           abs(mono_final - true_final) / true_final * 100);
    printf("  Stereo VO         │    %8.3f   │   %6.3f  │  %5.1f%%\n",
           stereo_final, abs(stereo_final - true_final),
           abs(stereo_final - true_final) / true_final * 100);
    printf("  VIO (Vision+IMU)  │    %8.3f   │   %6.3f  │  %5.1f%%\n",
           vio_final, abs(vio_final - true_final),
           abs(vio_final - true_final) / true_final * 100);
    printf("  IMU만 (참고)      │    %8.3f   │   %6.3f  │  %5.1f%%\n",
           imu_final, abs(imu_final - true_final),
           abs(imu_final - true_final) / true_final * 100);

    cout << endl;

    // 시간에 따른 오차 변화
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  시간에 따른 위치 오차 변화 (매 10스텝)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "  스텝 │ Mono 오차(%) │ Stereo 오차(%) │ VIO 오차(%) │ IMU만 오차(%)" << endl;
    cout << "  ─────┼─────────────┼────────────────┼─────────────┼──────────────" << endl;

    for (int i = 9; i < config.num_steps; i += 10) {
        double true_pos = (i + 1) * config.true_velocity * config.dt;

        double mono_err = abs(mono.positions[i + 1](0) - true_pos) / true_pos * 100;
        double stereo_err = abs(stereo.positions[i + 1](0) - true_pos) / true_pos * 100;
        double vio_err = abs(vio.positions[i + 1](0) - true_pos) / true_pos * 100;
        double imu_err = abs(vio.imu_only_positions[i + 1](0) - true_pos) / true_pos * 100;

        printf("   %3d  │   %7.2f%%   │     %7.2f%%    │   %7.2f%%   │    %7.2f%%\n",
               i + 1, mono_err, stereo_err, vio_err, imu_err);
    }

    cout << endl;

    // 핵심 관찰
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  핵심 관찰" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "  1. Monocular VO:" << endl;
    cout << "     - 스케일 오차가 시간에 따라 누적 (드리프트)" << endl;
    cout << "     - 매 프레임 3% 노이즈 → 50 스텝 후 큰 오차\n" << endl;

    cout << "  2. Stereo VO:" << endl;
    cout << "     - baseline 덕분에 스케일 안정적" << endl;
    cout << "     - 오차가 누적되지 않고 일정 범위 유지\n" << endl;

    cout << "  3. VIO (Vision + IMU):" << endl;
    cout << "     - IMU가 스케일 정보 제공" << endl;
    cout << "     - Vision이 드리프트 보정" << endl;
    cout << "     - 단독보다 융합이 우수\n" << endl;

    cout << "  4. IMU만:" << endl;
    cout << "     - 시간의 제곱에 비례하여 오차 증가" << endl;
    cout << "     - 장시간 사용 불가 → Vision 보정 필수\n" << endl;

    // Stereo depth 계산 예시
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  Stereo Depth 계산 예시" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "  공식: depth = f * b / disparity\n" << endl;
    cout << "  f = " << config.focal_length << " pixel" << endl;
    cout << "  b = " << config.baseline << " m\n" << endl;

    cout << "  Disparity (pixel) │ Depth (m) │ 비고" << endl;
    cout << "  ──────────────────┼───────────┼──────────────" << endl;

    for (double disp : {60.0, 30.0, 12.0, 6.0, 3.0, 1.0}) {
        double depth = config.focal_length * config.baseline / disp;
        string note = "";
        if (disp >= 30) note = "근거리 (정확)";
        else if (disp >= 6) note = "중거리";
        else if (disp >= 2) note = "원거리 (주의)";
        else note = "매우 먼 거리 (부정확)";

        printf("       %5.1f         │  %7.2f  │ %s\n", disp, depth, note.c_str());
    }

    cout << "\n  핵심: disparity가 작아지면 depth 오차 급증!" << endl;
    cout << "  → 원거리에서 Stereo 정확도 저하\n" << endl;

    // IMU 드리프트 시뮬레이션
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  IMU 바이어스에 의한 위치 오차 (이론값)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "  바이어스: " << config.imu_accel_bias << " m/s²" << endl;
    cout << "  위치 오차 = 0.5 * bias * t²\n" << endl;

    cout << "  시간 (초) │ 위치 오차 (m) │ 평가" << endl;
    cout << "  ──────────┼──────────────┼────────────" << endl;

    for (double t : {1.0, 5.0, 10.0, 30.0, 60.0, 120.0, 300.0}) {
        double error = 0.5 * config.imu_accel_bias * t * t;
        string eval = "";
        if (error < 0.01) eval = "무시 가능";
        else if (error < 0.1) eval = "양호";
        else if (error < 1.0) eval = "주의";
        else if (error < 10.0) eval = "문제";
        else eval = "사용 불가!";

        printf("     %5.0f   │    %8.3f   │ %s\n", t, error, eval.c_str());
    }

    cout << "\n  핵심: IMU만으로는 수십 초 후부터 심각한 드리프트 발생!" << endl;
    cout << "  → Vision으로 주기적 보정 필수 (VIO)\n" << endl;
}

// ============================================
// 메인 함수
// ============================================
int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  Week 13: 스케일 복구 방법 비교 시뮬레이션" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    SimConfig config;

    cout << "시뮬레이션 실행 중...\n" << endl;

    // 1. Monocular VO
    cout << "[1/3] Monocular VO 시뮬레이션..." << endl;
    MonoResult mono = simulateMonocular(config);

    // 2. Stereo VO
    cout << "[2/3] Stereo VO 시뮬레이션..." << endl;
    StereoResult stereo = simulateStereo(config);

    // 3. VIO
    cout << "[3/3] VIO (Vision + IMU) 시뮬레이션..." << endl;
    VIOResult vio = simulateVIO(config);

    cout << endl;

    // 결과 출력
    printResults(config, mono, stereo, vio);

    // Phase 3 완료 메시지
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  Phase 3 Week 13 실습 완료!" << endl;
    cout << "  → 다음: Phase 4 - VIO에서 실제 IMU 융합을 학습합니다" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
```

---

## 빌드 및 실행

```bash
cd week13
mkdir build && cd build
cmake ..
make

# 시뮬레이션 실행
./scale_recovery_sim
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Week 13: 스케일 복구 방법 비교 시뮬레이션
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

시뮬레이션 실행 중...

[1/3] Monocular VO 시뮬레이션...
[2/3] Stereo VO 시뮬레이션...
[3/3] VIO (Vision + IMU) 시뮬레이션...

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  최종 위치 비교
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  방법              │ 최종 위치 (m) │ 오차 (m) │ 오차 (%)
  ──────────────────┼──────────────┼──────────┼─────────
  실제 (Ground Truth)│       5.000  │    0.000 │    0.0%
  Monocular VO      │       6.234  │    1.234 │   24.7%
  Stereo VO         │       5.047  │    0.047 │    0.9%
  VIO (Vision+IMU)  │       5.156  │    0.156 │    3.1%
  IMU만 (참고)      │       5.625  │    0.625 │   12.5%

(나머지 출력 계속...)
```

---

## 코드 이해 포인트

### 1. Monocular의 스케일 드리프트

```cpp
// 핵심: 매 프레임 스케일 노이즈가 곱해짐 (누적!)
double scale_factor = 1.0 + noise(gen);   // 예: 1.03
accumulated_scale *= scale_factor;         // 곱셈 누적 → 드리프트!

// 50프레임 후: 1.03^50 또는 0.97^50 → 크게 벗어남
```

### 2. Stereo의 스케일 안정성

```cpp
// 핵심: 매 프레임 독립적으로 depth 계산 (누적 없음!)
double depth_scale = 1.0 + noise(gen);     // 예: 1.01
double estimated = true_displacement * depth_scale;

// 오차가 누적되지 않음 → 항상 1% 내외 유지
```

### 3. VIO의 센서 융합

```cpp
// Vision: 방향은 맞지만 스케일 모호
double vision_displacement = true_velocity * dt * vision_scale;

// IMU: 스케일 정보 있지만 드리프트
double imu_displacement = imu_velocity.norm() * dt;

// 융합: 두 정보를 결합 (간소화된 가중 평균)
double fused = 0.7 * vision_displacement + 0.3 * imu_displacement;

// 실제 VIO에서는:
// - Extended Kalman Filter (EKF) 또는
// - 비선형 최적화 (Ceres/g2o)로 융합
// → Phase 4에서 학습!
```

---

## ✅ 체크리스트

### 코드 실행
- [ ] scale_recovery_sim.cpp 빌드 성공
- [ ] 시뮬레이션 결과 확인
- [ ] 최종 위치 비교 테이블 이해

### 개념 이해
- [ ] Monocular의 스케일 드리프트가 왜 발생하는지 설명 가능
- [ ] Stereo가 왜 스케일 안정적인지 설명 가능
- [ ] IMU 단독 사용의 한계 이해
- [ ] VIO 융합의 장점 이해

### 응용
- [ ] 노이즈 파라미터를 변경해보고 결과 관찰
- [ ] Stereo baseline을 변경하면 depth 정확도가 어떻게 변하는지 확인
- [ ] IMU 바이어스를 변경하면 드리프트가 어떻게 변하는지 확인

---

## 💡 실습 후 생각할 질문

```
1. Monocular에서 스케일 노이즈를 0.01(1%)로 줄이면 충분히 안정적인가?
   → 아니요. 곱셈 누적이므로 시간이 길어지면 여전히 드리프트

2. Stereo의 baseline을 2배로 늘리면?
   → 같은 disparity에서 depth가 2배 → 원거리 정확도 향상
   → 하지만 물리적 크기 증가

3. VIO에서 가중치를 어떻게 최적으로 결정하는가?
   → Extended Kalman Filter의 Kalman Gain
   → 또는 최적화 기반 방법의 정보 행렬
   → Phase 4에서 자세히!
```

---

**다음**: Quiz로 개념 점검!
