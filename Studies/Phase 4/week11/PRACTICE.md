# Week 11 실습: VIO 초기화 미지수 추정 시뮬레이션

> 🎯 **목표**: VIO 초기화에서 추정해야 할 미지수들의 영향을 코드로 확인
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 4시간

---

## 📋 실습 개요

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | 스케일 오차가 궤적에 미치는 영향 | 필수 | 1시간 |
| 2 | 중력 방향 오차의 영향 | 필수 | 1.5시간 |
| 3 | 바이어스 오차와 초기 속도 오차 | 필수 | 1.5시간 |

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
week11/
├── CMakeLists.txt
├── README.md
├── PRACTICE.md           ← 지금 읽고 있는 파일
├── init_analysis.cpp     ← 이 실습의 메인 코드
├── quiz_easy.cpp
├── quiz_medium.cpp
└── quiz_solutions/
    ├── easy_sol.cpp
    └── medium_sol.cpp
```

---

## 실습 1: 스케일 오차가 궤적에 미치는 영향

```cpp
/**
 * 실습 1: 스케일 오차 분석
 *
 * Vision SfM 궤적에 잘못된 스케일을 적용했을 때
 * IMU 예측과 얼마나 불일치가 발생하는지 확인
 */

#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace Eigen;
using namespace std;

// Vision SfM 궤적 (up-to-scale, 정규화된 단위)
vector<Vector3d> generateSfMTrajectory(int n_frames) {
    vector<Vector3d> traj;
    for (int i = 0; i < n_frames; i++) {
        double t = i * 0.1;  // 0.1초 간격
        // 원호 궤적 (정규화)
        double x = sin(t * 0.5);
        double y = 1.0 - cos(t * 0.5);
        double z = 0.0;
        traj.push_back(Vector3d(x, y, z));
    }
    return traj;
}

// IMU 기반 궤적 (실제 스케일, 미터 단위)
vector<Vector3d> generateIMUTrajectory(int n_frames, double true_scale) {
    vector<Vector3d> traj;
    for (int i = 0; i < n_frames; i++) {
        double t = i * 0.1;
        double x = true_scale * sin(t * 0.5);
        double y = true_scale * (1.0 - cos(t * 0.5));
        double z = 0.0;
        traj.push_back(Vector3d(x, y, z));
    }
    return traj;
}

void analyzeScaleError() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  실습 1: 스케일 오차 분석" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    int n_frames = 20;
    double true_scale = 3.0;  // 실제 스케일 = 3.0m

    auto sfm_traj = generateSfMTrajectory(n_frames);
    auto imu_traj = generateIMUTrajectory(n_frames, true_scale);

    // 다양한 스케일 추정값으로 테스트
    vector<double> test_scales = {1.0, 2.0, 3.0, 4.0, 5.0};

    cout << "  실제 스케일: " << true_scale << " m\n" << endl;
    cout << "  추정 스케일 │ 평균 위치 오차 (m) │ 최대 위치 오차 (m) │ 평가" << endl;
    cout << "  ───────────┼───────────────────┼───────────────────┼──────" << endl;

    for (double s : test_scales) {
        double sum_error = 0.0;
        double max_error = 0.0;

        for (int i = 0; i < n_frames; i++) {
            Vector3d scaled_sfm = s * sfm_traj[i];
            double error = (scaled_sfm - imu_traj[i]).norm();
            sum_error += error;
            max_error = max(max_error, error);
        }

        double avg_error = sum_error / n_frames;
        string eval = (abs(s - true_scale) < 0.1) ? "정확!" :
                       (abs(s - true_scale) < 1.0) ? "주의" : "위험!";

        printf("     %4.1f     │      %8.4f      │      %8.4f      │ %s\n",
               s, avg_error, max_error, eval.c_str());
    }

    cout << "\n  핵심: 스케일이 50%만 틀려도 (3.0 vs 2.0) 큰 위치 오차 발생!" << endl;
    cout << "  → 정확한 스케일 추정이 VIO 초기화의 핵심\n" << endl;
}
```

---

## 실습 2: 중력 방향 오차의 영향

```cpp
/**
 * 실습 2: 중력 방향 오차 분석
 *
 * 중력 방향이 잘못 추정되면 수직 방향 오차가 어떻게 쌓이는지 확인
 */

void analyzeGravityError() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  실습 2: 중력 방향 오차 분석" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double g_magnitude = 9.81;  // m/s²
    double dt = 0.01;           // 100Hz IMU

    // 실제 중력: z축 아래 방향
    Vector3d g_true(0, 0, -g_magnitude);

    // 다양한 중력 방향 오차 테스트
    vector<double> angle_errors_deg = {0.1, 0.5, 1.0, 2.0, 5.0, 10.0};

    cout << "  중력 방향 오차가 10초간 적분에 미치는 영향:\n" << endl;
    cout << "  방향 오차 (°) │ 가속도 오차 (m/s²) │ 10초 후 위치 오차 (m)" << endl;
    cout << "  ─────────────┼───────────────────┼─────────────────────" << endl;

    for (double angle_deg : angle_errors_deg) {
        double angle_rad = angle_deg * M_PI / 180.0;

        // 중력 방향이 x축으로 angle_rad만큼 기울어진 경우
        Vector3d g_wrong(g_magnitude * sin(angle_rad), 0, -g_magnitude * cos(angle_rad));

        // 중력 보상 오차
        Vector3d g_error = g_wrong - g_true;
        double accel_error = g_error.norm();

        // 이 가속도 오차가 10초간 이중 적분되면:
        double t = 10.0;
        double position_error = 0.5 * accel_error * t * t;

        printf("      %5.1f     │       %7.4f      │        %8.3f\n",
               angle_deg, accel_error, position_error);
    }

    cout << "\n  분석:" << endl;
    cout << "  - 1° 오차도 10초 후 약 8.6m 위치 오차!" << endl;
    cout << "  - 5° 오차면 10초 후 약 42.8m → 사용 불가" << endl;
    cout << "  → 중력 방향은 0.1° 이내로 정확해야 함\n" << endl;

    // 시간에 따른 오차 누적 시각화
    cout << "  1° 중력 방향 오차 시 시간별 위치 오차:\n" << endl;
    double angle_1deg = 1.0 * M_PI / 180.0;
    double accel_err_1deg = g_magnitude * sin(angle_1deg);

    cout << "  시간 (초) │ 속도 오차 (m/s) │ 위치 오차 (m) │ 막대 그래프" << endl;
    cout << "  ──────────┼────────────────┼──────────────┼────────────" << endl;

    for (double t = 1.0; t <= 10.0; t += 1.0) {
        double vel_err = accel_err_1deg * t;
        double pos_err = 0.5 * accel_err_1deg * t * t;
        int bars = min((int)(pos_err * 2), 30);
        string bar(bars, '#');

        printf("     %4.1f    │     %7.3f     │    %8.3f    │ %s\n",
               t, vel_err, pos_err, bar.c_str());
    }
}
```

---

## 실습 3: 바이어스 & 초기 속도 오차 분석

```cpp
/**
 * 실습 3: 바이어스와 초기 속도 오차
 *
 * 자이로 바이어스와 초기 속도 오차가 궤적에 미치는 영향
 */

void analyzeBiasAndVelocityError() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  실습 3: 바이어스 & 초기 속도 오차" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    // 자이로 바이어스 오차 분석
    cout << "  [A] 자이로 바이어스 오차의 영향:\n" << endl;

    vector<double> gyro_biases = {0.0001, 0.0005, 0.001, 0.005, 0.01};

    cout << "  바이어스 (rad/s) │ 1초 후 (°) │ 10초 후 (°) │ 60초 후 (°)" << endl;
    cout << "  ─────────────────┼───────────┼────────────┼────────────" << endl;

    for (double bg : gyro_biases) {
        double err_1s = bg * 1.0 * 180.0 / M_PI;
        double err_10s = bg * 10.0 * 180.0 / M_PI;
        double err_60s = bg * 60.0 * 180.0 / M_PI;

        printf("      %8.4f     │   %6.3f   │   %7.3f   │   %7.3f\n",
               bg, err_1s, err_10s, err_60s);
    }

    cout << "\n  핵심: b_g = 0.001 rad/s → 1분 후 약 3.4° 회전 오차" << endl;
    cout << "  → 방향이 3.4° 틀어지면 10m 이동 시 약 0.6m 옆으로 밀림!\n" << endl;

    // 초기 속도 오차 분석
    cout << "  [B] 초기 속도 오차의 영향:\n" << endl;

    vector<double> vel_errors = {0.01, 0.05, 0.1, 0.5, 1.0};

    cout << "  속도 오차 (m/s) │ 1초 후 (m) │ 5초 후 (m) │ 10초 후 (m)" << endl;
    cout << "  ────────────────┼───────────┼───────────┼────────────" << endl;

    for (double dv : vel_errors) {
        // 초기 속도 오차 → 위치 오차 = dv * t
        printf("      %5.2f       │   %6.3f   │   %6.3f   │   %7.3f\n",
               dv, dv * 1.0, dv * 5.0, dv * 10.0);
    }

    cout << "\n  핵심: 초기 속도가 0.1 m/s만 틀려도 10초 후 1m 오차!" << endl;
    cout << "  → 초기 속도 추정 정확도가 매우 중요\n" << endl;

    // 종합 비교
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  종합: 각 오차의 10초 후 위치 영향\n" << endl;

    cout << "  오차 원인            │ 일반적 크기        │ 10초 후 위치 오차" << endl;
    cout << "  ─────────────────────┼───────────────────┼──────────────────" << endl;
    cout << "  스케일 50% 오차      │ s=1.5 (실제 1.0)  │      ~5.0 m" << endl;
    cout << "  중력 방향 1° 오차    │ 0.017 rad         │      ~8.6 m" << endl;
    cout << "  자이로 바이어스      │ 0.001 rad/s       │      ~0.6 m" << endl;
    cout << "  가속도 바이어스      │ 0.01 m/s²         │      ~0.5 m" << endl;
    cout << "  초기 속도 0.1 m/s    │ 0.1 m/s           │      ~1.0 m" << endl;
    cout << "\n  → 중력 방향과 스케일 오차가 가장 치명적!" << endl;
    cout << "  → 초기화에서 이 둘을 정확히 추정하는 것이 핵심\n" << endl;
}
```

---

## 전체 코드: init_analysis.cpp

위의 3개 실습 함수를 하나의 파일로 결합하세요:

```cpp
#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace Eigen;
using namespace std;

// ... (위의 함수들 포함)

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  Week 11: VIO 초기화 미지수 분석" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    analyzeScaleError();
    analyzeGravityError();
    analyzeBiasAndVelocityError();

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "  Week 11 실습 완료!" << endl;
    cout << "  → 다음: Week 12에서 이 미지수들을 실제로 추정합니다" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
```

---

## 빌드 및 실행

```bash
cd week11
mkdir build && cd build
cmake ..
make

# 실습 코드 실행
./init_analysis
```

---

## ✅ 체크리스트

### 코드 실행
- [ ] init_analysis.cpp 빌드 성공
- [ ] 스케일 오차 분석 결과 확인
- [ ] 중력 방향 오차 분석 결과 확인
- [ ] 바이어스/속도 오차 분석 결과 확인

### 개념 이해
- [ ] 스케일 오차가 왜 위치 오차에 직접 영향을 주는지 설명 가능
- [ ] 중력 방향 1° 오차가 왜 10초 후 ~8.6m 오차가 되는지 이해
- [ ] 자이로 바이어스와 가속도 바이어스의 영향 차이 이해
- [ ] 왜 충분한 움직임이 필요한지 설명 가능

### 응용
- [ ] 각 파라미터를 변경해보고 민감도 분석
- [ ] 어떤 미지수가 가장 먼저 추정되어야 하는지 순서 정하기

---

**다음**: Quiz로 개념 점검!
