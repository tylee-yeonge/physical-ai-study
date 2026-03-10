# Week 12 실습: 스케일 드리프트 시뮬레이션

> 🎯 **목표**: 간단한 코드로 스케일 드리프트 현상 확인
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 3시간

---

## 📋 실습 개요

스케일 드리프트가 실제로 어떻게 누적되는지, 합성 궤적을 만들어 직접 확인합니다.

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | 원형 궤적 생성 + 스케일 노이즈 | 필수 | 1.5시간 |
| 2 | 드리프트 누적 분석 | 필수 | 1시간 |
| 3 | 노이즈 비율별 비교 | 선택 | 0.5시간 |

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

## Step 1: 스케일 드리프트 시뮬레이션

### 1.1 목표

원형 궤적을 생성한 뒤, 매 프레임 스케일에 1% 노이즈를 추가하여 드리프트가 얼마나 심해지는지 관찰합니다.

### 1.2 프로젝트 구조

```
week12/
├── CMakeLists.txt
├── README.md
├── PRACTICE.md
├── scale_drift_sim.cpp    ← 이 파일을 만듭니다
├── quiz_easy.cpp
├── quiz_medium.cpp
└── quiz_solutions/
    ├── easy_sol.cpp
    └── medium_sol.cpp
```

### 1.3 코드: scale_drift_sim.cpp

```cpp
/**
 * Week 12 실습: 스케일 드리프트 시뮬레이션
 *
 * 목표: 단안 카메라의 스케일 드리프트가
 *       궤적에 미치는 영향을 직접 확인
 */

#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

/**
 * 원형 궤적 생성 (Ground Truth)
 *
 * @param n_frames   프레임 수
 * @param radius     원 반지름 (미터)
 * @return           각 프레임의 위치 (x, y)
 */
vector<Vector2d> generateCircularTrajectory(int n_frames, double radius) {
    vector<Vector2d> trajectory;
    for (int i = 0; i < n_frames; i++) {
        double theta = 2.0 * M_PI * i / n_frames;
        double x = radius * cos(theta);
        double y = radius * sin(theta);
        trajectory.push_back(Vector2d(x, y));
    }
    return trajectory;
}

/**
 * 스케일 드리프트 시뮬레이션
 *
 * 매 프레임 이동 벡터에 스케일 노이즈를 곱하여
 * 드리프트 누적 과정을 재현
 *
 * @param gt_trajectory   Ground truth 궤적
 * @param scale_noise_std 스케일 노이즈 표준편차 (예: 0.01 = 1%)
 * @param seed            랜덤 시드
 * @return                드리프트가 포함된 추정 궤적
 */
vector<Vector2d> simulateScaleDrift(const vector<Vector2d>& gt_trajectory,
                                     double scale_noise_std,
                                     int seed = 42) {
    default_random_engine gen(seed);
    normal_distribution<double> noise(1.0, scale_noise_std);

    vector<Vector2d> estimated;
    estimated.push_back(gt_trajectory[0]);  // 시작점은 동일

    for (size_t i = 1; i < gt_trajectory.size(); i++) {
        // Ground truth 이동 벡터
        Vector2d delta = gt_trajectory[i] - gt_trajectory[i - 1];

        // 스케일 노이즈 적용
        // 실제 VO에서는 매 프레임 t의 스케일이 조금씩 다름
        double scale_factor = noise(gen);
        Vector2d noisy_delta = scale_factor * delta;

        // 누적
        Vector2d new_pos = estimated.back() + noisy_delta;
        estimated.push_back(new_pos);
    }

    return estimated;
}

/**
 * 궤적 분석: 드리프트 통계 출력
 */
void analyzeTrajectory(const vector<Vector2d>& gt,
                       const vector<Vector2d>& est,
                       const string& label) {
    int n = gt.size();

    // 최종 위치 오차
    double final_error = (gt.back() - est.back()).norm();

    // 평균 위치 오차
    double total_error = 0.0;
    double max_error = 0.0;
    for (int i = 0; i < n; i++) {
        double err = (gt[i] - est[i]).norm();
        total_error += err;
        max_error = max(max_error, err);
    }
    double avg_error = total_error / n;

    // Loop closure 오차 (시작점과 끝점 차이)
    // 원형 궤적이므로 마지막 점은 시작점 근처여야 함
    double loop_error_gt = (gt.back() - gt.front()).norm();
    double loop_error_est = (est.back() - est.front()).norm();

    cout << "  [" << label << "]" << endl;
    cout << "  최종 위치 오차:   " << final_error << " m" << endl;
    cout << "  평균 위치 오차:   " << avg_error << " m" << endl;
    cout << "  최대 위치 오차:   " << max_error << " m" << endl;
    cout << "  Loop closure 오차: " << loop_error_est << " m" << endl;
    cout << endl;
}

/**
 * ASCII 궤적 시각화 (간단 버전)
 */
void visualizeTrajectory(const vector<Vector2d>& gt,
                         const vector<Vector2d>& est,
                         int grid_size = 40) {
    // 전체 범위 계산
    double min_x = 1e9, max_x = -1e9;
    double min_y = 1e9, max_y = -1e9;

    for (const auto& p : gt) {
        min_x = min(min_x, p.x()); max_x = max(max_x, p.x());
        min_y = min(min_y, p.y()); max_y = max(max_y, p.y());
    }
    for (const auto& p : est) {
        min_x = min(min_x, p.x()); max_x = max(max_x, p.x());
        min_y = min(min_y, p.y()); max_y = max(max_y, p.y());
    }

    // 여유 공간
    double margin = 0.5;
    min_x -= margin; max_x += margin;
    min_y -= margin; max_y += margin;

    // 그리드 초기화
    vector<string> grid(grid_size, string(grid_size * 2, ' '));

    // Ground truth 그리기 (.)
    for (const auto& p : gt) {
        int gx = (int)((p.x() - min_x) / (max_x - min_x) * (grid_size * 2 - 1));
        int gy = (int)((p.y() - min_y) / (max_y - min_y) * (grid_size - 1));
        gy = grid_size - 1 - gy;  // y축 반전
        if (gx >= 0 && gx < grid_size * 2 && gy >= 0 && gy < grid_size)
            grid[gy][gx] = '.';
    }

    // 추정 궤적 그리기 (*)
    for (const auto& p : est) {
        int gx = (int)((p.x() - min_x) / (max_x - min_x) * (grid_size * 2 - 1));
        int gy = (int)((p.y() - min_y) / (max_y - min_y) * (grid_size - 1));
        gy = grid_size - 1 - gy;
        if (gx >= 0 && gx < grid_size * 2 && gy >= 0 && gy < grid_size)
            grid[gy][gx] = '*';
    }

    // 시작점 (S)
    {
        int gx = (int)((gt[0].x() - min_x) / (max_x - min_x) * (grid_size * 2 - 1));
        int gy = (int)((gt[0].y() - min_y) / (max_y - min_y) * (grid_size - 1));
        gy = grid_size - 1 - gy;
        if (gx >= 0 && gx < grid_size * 2 && gy >= 0 && gy < grid_size)
            grid[gy][gx] = 'S';
    }

    // 출력
    cout << "  궤적 시각화 ( . = GT,  * = 추정,  S = 시작점 )" << endl;
    cout << "  " << string(grid_size * 2 + 2, '-') << endl;
    for (const auto& row : grid) {
        cout << "  |" << row << "|" << endl;
    }
    cout << "  " << string(grid_size * 2 + 2, '-') << endl;
    cout << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 12: 스케일 드리프트 시뮬레이션" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    // ============================================
    // 파라미터 설정
    // ============================================
    const int n_frames = 100;         // 100 프레임 (원 한 바퀴)
    const double radius = 5.0;        // 반지름 5m

    cout << "설정:" << endl;
    cout << "  프레임 수: " << n_frames << endl;
    cout << "  궤적: 반지름 " << radius << "m 원" << endl;
    cout << "  원 둘레: " << 2 * M_PI * radius << "m\n" << endl;

    // ============================================
    // Ground Truth 궤적 생성
    // ============================================
    auto gt = generateCircularTrajectory(n_frames, radius);

    cout << "Ground Truth 생성 완료" << endl;
    cout << "  시작점: (" << gt.front().x() << ", " << gt.front().y() << ")" << endl;
    cout << "  끝점:   (" << gt.back().x() << ", " << gt.back().y() << ")\n" << endl;

    // ============================================
    // 다양한 노이즈 수준에서 시뮬레이션
    // ============================================
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "스케일 노이즈별 드리프트 비교" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    vector<double> noise_levels = {0.001, 0.005, 0.01, 0.02, 0.05};

    for (double noise_std : noise_levels) {
        auto est = simulateScaleDrift(gt, noise_std);
        char label[64];
        snprintf(label, sizeof(label), "노이즈 %.1f%%", noise_std * 100);
        analyzeTrajectory(gt, est, label);
    }

    // ============================================
    // 1% 노이즈에서 상세 분석
    // ============================================
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "상세 분석: 스케일 노이즈 1% (0.01)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    auto est_1pct = simulateScaleDrift(gt, 0.01);

    // 프레임별 오차 출력 (10프레임 간격)
    cout << "  프레임별 누적 오차:" << endl;
    cout << "  프레임  | GT 위치          | 추정 위치        | 오차(m)" << endl;
    cout << "  --------|-----------------|-----------------|--------" << endl;

    for (int i = 0; i < n_frames; i += 10) {
        double err = (gt[i] - est_1pct[i]).norm();
        printf("    %3d   | (%5.2f, %5.2f)  | (%5.2f, %5.2f)  | %6.3f\n",
               i, gt[i].x(), gt[i].y(),
               est_1pct[i].x(), est_1pct[i].y(), err);
    }
    cout << endl;

    // 시각화
    visualizeTrajectory(gt, est_1pct, 20);

    // ============================================
    // 핵심 포인트
    // ============================================
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "핵심 관찰" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "1. 매 프레임 작은 스케일 오차도 누적되면 큰 드리프트" << endl;
    cout << "2. 원형 궤적에서 시작점과 끝점이 맞지 않음 (Loop closure 필요)" << endl;
    cout << "3. 노이즈 비율이 커질수록 드리프트가 급격히 증가" << endl;
    cout << "4. 이것이 단안 VO의 근본적 한계!" << endl;
    cout << "\n해결책:" << endl;
    cout << "  - IMU 융합 (VIO): 가속도로 스케일 관측" << endl;
    cout << "  - 스테레오 카메라: baseline으로 절대 스케일" << endl;
    cout << "  - Loop Closure + Sim(3): 드리프트 보정" << endl;

    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "실습 완료!" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
```

---

## 빌드 및 실행

```bash
cd week12
mkdir build && cd build
cmake ..
make

# 시뮬레이션 실행
./scale_drift_sim
```

---

## 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 12: 스케일 드리프트 시뮬레이션
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

설정:
  프레임 수: 100
  궤적: 반지름 5m 원
  원 둘레: 31.4159m

Ground Truth 생성 완료
  시작점: (5, 0)
  끝점:   (4.84, -0.31)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
스케일 노이즈별 드리프트 비교
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  [노이즈 0.1%]
  최종 위치 오차:   0.032 m
  평균 위치 오차:   0.018 m
  최대 위치 오차:   0.041 m
  Loop closure 오차: 0.035 m

  [노이즈 0.5%]
  최종 위치 오차:   0.158 m
  평균 위치 오차:   0.089 m
  최대 위치 오차:   0.203 m
  Loop closure 오차: 0.172 m

  [노이즈 1.0%]
  최종 위치 오차:   0.312 m
  평균 위치 오차:   0.175 m
  최대 위치 오차:   0.408 m
  Loop closure 오차: 0.345 m

  [노이즈 2.0%]
  최종 위치 오차:   0.634 m
  평균 위치 오차:   0.352 m
  최대 위치 오차:   0.815 m
  Loop closure 오차: 0.689 m

  [노이즈 5.0%]
  최종 위치 오차:   1.587 m
  평균 위치 오차:   0.879 m
  최대 위치 오차:   2.038 m
  Loop closure 오차: 1.723 m

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
핵심 관찰
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

1. 매 프레임 작은 스케일 오차도 누적되면 큰 드리프트
2. 원형 궤적에서 시작점과 끝점이 맞지 않음 (Loop closure 필요)
3. 노이즈 비율이 커질수록 드리프트가 급격히 증가
4. 이것이 단안 VO의 근본적 한계!
```

---

## 추가 실험 아이디어

### 실험 1: 직선 궤적에서 드리프트

```cpp
// 원형 대신 직선 궤적으로 변경
vector<Vector2d> generateLinearTrajectory(int n_frames, double step) {
    vector<Vector2d> trajectory;
    for (int i = 0; i < n_frames; i++) {
        trajectory.push_back(Vector2d(i * step, 0));
    }
    return trajectory;
}
```

### 실험 2: 여러 바퀴 돌기

```cpp
// 3바퀴 돌면서 드리프트 관찰
const int n_frames = 300;  // 3 바퀴
// → Loop closure 없이 얼마나 벌어지는지 확인
```

### 실험 3: 바이어스가 있는 스케일 오차

```cpp
// 평균이 1이 아닌 경우 (시스템적 오차)
normal_distribution<double> noise(0.99, 0.01);  // 평균적으로 1% 작게 추정
// → 드리프트가 한 방향으로 편향됨
```

---

## 체크리스트

- [ ] scale_drift_sim.cpp 이해 및 빌드
- [ ] 다양한 노이즈 수준에서 실행
- [ ] 드리프트 패턴 관찰 (노이즈 비례)
- [ ] ASCII 시각화에서 GT와 추정 궤적 차이 확인
- [ ] (선택) 추가 실험 시도

---

## 🏗️ mini_slam 구현 (이번 주 핵심)

> 이번 주는 mini_slam의 출력을 분석하여 **스케일 드리프트를 정량 측정**한다.
> Week 5에서 관찰한 드리프트가 BA(Week 8~10) 적용 후 얼마나 줄었는지 비교한다.

**작업 내용**:

| 작업 | 내용 |
|------|------|
| mini_slam 궤적 출력 | 카메라 경로를 TUM 형식으로 저장 |
| GT 대비 비교 | ATE(Absolute Trajectory Error) 계산 |
| BA 유무 비교 | BA 없는 VO vs BA 있는 mini_slam 드리프트 비교 |
| 스케일 drift 정량 분석 | 거리 대비 드리프트 비율 (%) 측정 |

### 완성 기준

```bash
cd Studies/Phase\ 3/mini_slam/build
./mini_slam

# mini_slam 경로 출력 → GT 비교
# 드리프트 비율: BA 없이 X% → BA 적용 후 Y%
```

---

**다음**: Quiz로 개념 점검!
