# Week 11 실습: Ceres BA 예제 분석

> 🎯 **목표**: Ceres examples/bal_problem.cc 분석 및 BAL 데이터셋 실행
> 💻 **언어**: C++ (Ceres, Eigen)
> ⏰ **예상 시간**: 5시간

---

## 📋 실습 개요

Week 11은 **Ceres Solver의 BAL(Bundle Adjustment in the Large) 예제**를 분석하고, 실제 데이터셋에서 BA를 실행하여 수렴 과정을 관찰합니다. Week 8에서 간단한 Ceres BA를 구현했다면, 이번에는 실제 대규모 데이터에서 동작하는 것을 확인합니다.

---

## 🔧 환경 설정

### 사전 준비 (Week 8에서 완료)

```bash
# Ceres가 이미 설치되어 있어야 합니다
pkg-config --modversion ceres

# 없다면 설치
sudo apt install cmake libgoogle-glog-dev libgflags-dev
sudo apt install libatlas-base-dev libeigen3-dev

git clone https://ceres-solver.googlesource.com/ceres-solver
cd ceres-solver
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

### BAL 데이터셋 다운로드

```bash
# 작은 데이터셋부터 시작 (추천)
mkdir -p data && cd data

# Ladybug 데이터셋 (49카메라, 7776점)
wget https://grail.cs.washington.edu/projects/bal/data/ladybug/problem-49-7776-pre.txt.bz2
bunzip2 problem-49-7776-pre.txt.bz2

# 더 작은 것도 있음 (16카메라, 22106점)
wget https://grail.cs.washington.edu/projects/bal/data/ladybug/problem-16-22106-pre.txt.bz2
bunzip2 problem-16-22106-pre.txt.bz2
```

---

## 프로젝트 구조

```
week11/
├── CMakeLists.txt
├── README.md
├── PRACTICE.md
├── quiz_easy.cpp
├── quiz_medium.cpp
├── quiz_solutions/
│   ├── easy_sol.cpp
│   └── medium_sol.cpp
├── data/                          ← BAL 데이터셋 (직접 다운로드)
│   └── problem-49-7776-pre.txt
└── src/                           ← 실습 코드 (직접 작성)
    ├── simple_ba.cpp              ← Step 1: 간단한 BA
    └── bal_ba.cpp                 ← Step 2: BAL 데이터 BA
```

---

## Step 1: Ceres BAL 예제 분석

### Ceres 공식 예제 위치

```bash
# Ceres 소스코드에서 찾기
ls ceres-solver/examples/
# → bundle_adjuster.cc     ← BAL 데이터 로더 + BA
# → bal_problem.h          ← BAL 파일 파싱
# → bal_problem.cc
```

### bal_problem.h 핵심 구조

```cpp
// BAL 데이터를 읽고 관리하는 클래스
class BALProblem {
public:
    // BAL 파일 로드
    explicit BALProblem(const std::string& filename);

    int num_cameras() const;        // 카메라 수
    int num_points() const;         // 3D 점 수
    int num_observations() const;   // 관측 수

    // 관측 데이터 접근
    const double* observations() const;

    // 카메라/점 파라미터 접근 (최적화 대상)
    double* mutable_cameras();
    double* mutable_points();

    // 특정 관측에 대한 카메라/점 파라미터 포인터
    double* mutable_camera_for_observation(int i);
    double* mutable_point_for_observation(int i);

private:
    int num_cameras_;
    int num_points_;
    int num_observations_;
    int num_parameters_;

    int* camera_index_;     // 각 관측의 카메라 ID
    int* point_index_;      // 각 관측의 점 ID
    double* observations_;  // 관측값 (u, v 쌍)
    double* parameters_;    // 카메라 + 점 파라미터 (연속 배열)
};
```

### bundle_adjuster.cc 핵심 흐름

```cpp
// 1. BAL 데이터 로드
BALProblem bal_problem("problem-49-7776-pre.txt");

// 2. Ceres Problem 구성
ceres::Problem problem;

for (int i = 0; i < bal_problem.num_observations(); ++i) {
    // Cost Function 생성
    ceres::CostFunction* cost_function =
        SnavelyReprojectionError::Create(
            observations[2 * i + 0],   // observed_x
            observations[2 * i + 1]);  // observed_y

    // Loss Function (Outlier 처리)
    ceres::LossFunction* loss_function = new ceres::HuberLoss(1.0);

    // Residual Block 추가
    problem.AddResidualBlock(
        cost_function,
        loss_function,
        bal_problem.mutable_camera_for_observation(i),  // 카메라 [9]
        bal_problem.mutable_point_for_observation(i));   // 점 [3]
}

// 3. Solver 옵션
ceres::Solver::Options options;
options.linear_solver_type = ceres::DENSE_SCHUR;
options.minimizer_progress_to_stdout = true;
options.max_num_iterations = 100;
options.gradient_tolerance = 1e-16;
options.function_tolerance = 1e-16;

// 4. 최적화 실행
ceres::Solver::Summary summary;
ceres::Solve(options, &problem, &summary);

// 5. 결과 출력
std::cout << summary.FullReport() << std::endl;
```

---

## Step 2: 간단한 Standalone Ceres BA 구현

Week 8 PRACTICE.md의 코드를 발전시켜, BAL 스타일의 카메라 모델(9파라미터)을 사용합니다.

### src/simple_ba.cpp

```cpp
#include <ceres/ceres.h>
#include <ceres/rotation.h>
#include <Eigen/Dense>
#include <iostream>
#include <random>
#include <vector>

// ============================================================
// Snavely 카메라 모델 (BAL 스타일, 9파라미터)
// ============================================================
struct SnavelyReprojectionError {
    SnavelyReprojectionError(double observed_x, double observed_y)
        : observed_x_(observed_x), observed_y_(observed_y) {}

    template <typename T>
    bool operator()(const T* const camera,
                    const T* const point,
                    T* residuals) const {
        // 1. 3D 점을 카메라 좌표계로 변환
        T p[3];
        ceres::AngleAxisRotatePoint(camera, point, p);
        p[0] += camera[3];
        p[1] += camera[4];
        p[2] += camera[5];

        // 2. 정규화 좌표 (BAL 관례: -z 방향)
        T xp = -p[0] / p[2];
        T yp = -p[1] / p[2];

        // 3. 방사 왜곡
        const T& l1 = camera[7];
        const T& l2 = camera[8];
        T r2 = xp * xp + yp * yp;
        T distortion = T(1.0) + r2 * (l1 + l2 * r2);

        // 4. focal length 적용
        const T& focal = camera[6];
        T predicted_x = focal * distortion * xp;
        T predicted_y = focal * distortion * yp;

        // 5. 잔차 계산
        residuals[0] = predicted_x - T(observed_x_);
        residuals[1] = predicted_y - T(observed_y_);

        return true;
    }

    static ceres::CostFunction* Create(double observed_x, double observed_y) {
        return new ceres::AutoDiffCostFunction<SnavelyReprojectionError, 2, 9, 3>(
            new SnavelyReprojectionError(observed_x, observed_y));
    }

private:
    double observed_x_;
    double observed_y_;
};

// ============================================================
// 합성 데이터 생성
// ============================================================
struct Observation {
    int camera_id;
    int point_id;
    double x, y;  // 관측된 픽셀 좌표
};

void generateSyntheticData(
    std::vector<std::array<double, 9>>& cameras,
    std::vector<std::array<double, 3>>& points,
    std::vector<Observation>& observations)
{
    std::mt19937 gen(42);
    std::normal_distribution<double> noise(0.0, 1.0);

    // 카메라 5대: 원형으로 배치
    int num_cameras = 5;
    double focal = 500.0;  // focal length
    cameras.resize(num_cameras);

    for (int i = 0; i < num_cameras; i++) {
        double angle = 2.0 * M_PI * i / num_cameras;
        double radius = 5.0;

        cameras[i] = {0, 0, 0,                         // angle-axis (정면)
                       radius * cos(angle),              // tx
                       0.0,                              // ty
                       radius * sin(angle),              // tz
                       focal,                            // focal length
                       0.0, 0.0};                        // k1, k2

        // 간단한 회전: 중심을 바라보도록
        double yaw = -angle - M_PI;
        cameras[i][1] = yaw;  // angle-axis y축 회전
    }

    // 3D 점 20개: 원점 주변에 랜덤 배치
    int num_points = 20;
    std::uniform_real_distribution<double> pos(-2.0, 2.0);
    points.resize(num_points);

    for (int i = 0; i < num_points; i++) {
        points[i] = {pos(gen), pos(gen), pos(gen)};
    }

    // 관측 생성
    for (int cam_id = 0; cam_id < num_cameras; cam_id++) {
        for (int pt_id = 0; pt_id < num_points; pt_id++) {
            // 투영 (간단 버전)
            double p[3];
            ceres::AngleAxisRotatePoint(cameras[cam_id].data(),
                                         points[pt_id].data(), p);
            p[0] += cameras[cam_id][3];
            p[1] += cameras[cam_id][4];
            p[2] += cameras[cam_id][5];

            if (p[2] < 0.1) continue;  // 카메라 뒤의 점은 제외

            double xp = -p[0] / p[2];
            double yp = -p[1] / p[2];

            double predicted_x = focal * xp;
            double predicted_y = focal * yp;

            // 이미지 범위 확인 (대략적)
            if (std::abs(predicted_x) > 500 || std::abs(predicted_y) > 500)
                continue;

            // 노이즈 추가
            observations.push_back({
                cam_id, pt_id,
                predicted_x + noise(gen),
                predicted_y + noise(gen)
            });
        }
    }
}

// ============================================================
// 메인
// ============================================================
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 11: Ceres BA - 합성 데이터 실습" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 1. Ground truth 생성
    std::vector<std::array<double, 9>> gt_cameras;
    std::vector<std::array<double, 3>> gt_points;
    std::vector<Observation> observations;
    generateSyntheticData(gt_cameras, gt_points, observations);

    std::cout << "합성 데이터 생성 완료:" << std::endl;
    std::cout << "  카메라: " << gt_cameras.size() << std::endl;
    std::cout << "  3D 점:  " << gt_points.size() << std::endl;
    std::cout << "  관측:   " << observations.size() << "\n" << std::endl;

    // 2. 초기값에 노이즈 추가 (최적화 대상)
    auto cameras = gt_cameras;
    auto points = gt_points;

    std::mt19937 gen(123);
    std::normal_distribution<double> cam_noise(0.0, 0.05);
    std::normal_distribution<double> pt_noise(0.0, 0.3);

    for (size_t i = 1; i < cameras.size(); i++) {  // 첫 카메라는 고정
        for (int j = 0; j < 6; j++) {
            cameras[i][j] += cam_noise(gen);
        }
    }
    for (auto& p : points) {
        for (int j = 0; j < 3; j++) {
            p[j] += pt_noise(gen);
        }
    }

    // 3. Ceres Problem 구성
    ceres::Problem problem;

    for (const auto& obs : observations) {
        ceres::CostFunction* cost_function =
            SnavelyReprojectionError::Create(obs.x, obs.y);

        ceres::LossFunction* loss_function = new ceres::HuberLoss(1.0);

        problem.AddResidualBlock(
            cost_function,
            loss_function,
            cameras[obs.camera_id].data(),
            points[obs.point_id].data());
    }

    // 첫 번째 카메라 고정 (gauge freedom)
    problem.SetParameterBlockConstant(cameras[0].data());

    // 4. Solver 설정
    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_SCHUR;
    options.minimizer_progress_to_stdout = true;
    options.max_num_iterations = 50;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Bundle Adjustment 실행 중..." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 5. 최적화 실행
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    // 6. 결과 출력
    std::cout << "\n" << summary.BriefReport() << "\n" << std::endl;

    // 카메라 오차 분석
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "카메라 평행이동 오차:" << std::endl;
    for (size_t i = 0; i < cameras.size(); i++) {
        double err = 0.0;
        for (int j = 3; j < 6; j++) {
            double d = cameras[i][j] - gt_cameras[i][j];
            err += d * d;
        }
        err = std::sqrt(err);
        std::cout << "  Camera " << i << ": " << err << " m"
                  << (i == 0 ? " (fixed)" : "") << std::endl;
    }

    // 점 오차 분석
    double avg_pt_err = 0.0;
    for (size_t i = 0; i < points.size(); i++) {
        double err = 0.0;
        for (int j = 0; j < 3; j++) {
            double d = points[i][j] - gt_points[i][j];
            err += d * d;
        }
        avg_pt_err += std::sqrt(err);
    }
    avg_pt_err /= points.size();
    std::cout << "  평균 3D 점 오차: " << avg_pt_err << " m" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    if (summary.termination_type == ceres::CONVERGENCE) {
        std::cout << "BA 수렴 성공!" << std::endl;
    } else {
        std::cout << "BA 수렴 실패 (더 많은 iteration 필요)" << std::endl;
    }
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## Step 3: 빌드 및 실행

### CMakeLists.txt (실습용)

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week11_Ceres_BA_Practice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

find_package(Eigen3 REQUIRED)
find_package(Ceres REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR} ${CERES_INCLUDE_DIRS})

# 간단한 BA 실습
add_executable(simple_ba src/simple_ba.cpp)
target_link_libraries(simple_ba ${CERES_LIBRARIES})
```

### 빌드 및 실행

```bash
cd week11
mkdir build && cd build
cmake ..
make -j$(nproc)

# 실행
./simple_ba
```

### 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 11: Ceres BA - 합성 데이터 실습
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

합성 데이터 생성 완료:
  카메라: 5
  3D 점:  20
  관측:   78

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Bundle Adjustment 실행 중...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

iter      cost      cost_change  |gradient|   |step|    tr_ratio  tr_radius  ls_iter  iter_time  total_time
   0  1.23e+04    0.00e+00    5.67e+04   0.00e+00   0.00e+00  1.00e+04        0    5.00e-03   8.00e-03
   1  4.56e+02    1.19e+04    3.45e+03   2.34e-01   9.87e-01  3.00e+04        1    4.00e-03   1.20e-02
   2  8.90e+01    3.67e+02    4.56e+02   1.23e-01   9.95e-01  9.00e+04        1    3.00e-03   1.50e-02
   3  7.80e+01    1.10e+01    5.67e+01   3.45e-02   9.98e-01  2.70e+05        1    3.00e-03   1.80e-02
   4  7.78e+01    2.00e-01    1.23e+00   4.56e-03   1.00e+00  8.10e+05        1    3.00e-03   2.10e-02

Ceres Solver Report: Iterations: 5, Initial cost: 1.234e+04, Final cost: 7.780e+01, Termination: CONVERGENCE

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
카메라 평행이동 오차:
  Camera 0: 0.000 m (fixed)
  Camera 1: 0.012 m
  Camera 2: 0.008 m
  Camera 3: 0.015 m
  Camera 4: 0.011 m
  평균 3D 점 오차: 0.024 m

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
BA 수렴 성공!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## Step 4: 수렴 과정 관찰 포인트

### 4.1 Iteration별 관찰할 것

```
Solver 출력에서 확인:

1. cost (총 비용)
   → 매 iteration마다 감소해야 함
   → 초반에 급감, 후반에 천천히 감소

2. cost_change (비용 변화량)
   → 수렴할수록 작아짐
   → function_tolerance 이하면 종료

3. |gradient| (그래디언트 크기)
   → 수렴할수록 0에 가까워짐
   → gradient_tolerance 이하면 종료

4. tr_ratio (Trust Region 비율)
   → 1에 가까우면: 좋은 근사 (Gauss-Newton에 가까움)
   → 0에 가까우면: 나쁜 근사 (더 보수적으로)

5. tr_radius (Trust Region 반지름)
   → 커지면: 공격적 (GN에 가까움)
   → 작아지면: 보수적 (GD에 가까움)
```

### 4.2 실험해볼 것

**실험 1: HuberLoss 유무 비교**
```cpp
// A: HuberLoss 사용
problem.AddResidualBlock(cost, new ceres::HuberLoss(1.0), camera, point);

// B: Loss 없음
problem.AddResidualBlock(cost, nullptr, camera, point);

// 비교: outlier가 있을 때 어떤 차이?
```

**실험 2: Solver 타입 비교**
```cpp
// A: DENSE_SCHUR (소규모)
options.linear_solver_type = ceres::DENSE_SCHUR;

// B: SPARSE_SCHUR (대규모에 유리)
options.linear_solver_type = ceres::SPARSE_SCHUR;

// C: ITERATIVE_SCHUR (초대규모)
options.linear_solver_type = ceres::ITERATIVE_SCHUR;
options.preconditioner_type = ceres::SCHUR_JACOBI;

// 비교: 데이터 규모에 따른 속도 차이
```

**실험 3: 노이즈 크기에 따른 수렴**
```cpp
// 노이즈 작음: 빠르게 수렴
std::normal_distribution<double> noise(0.0, 0.5);

// 노이즈 큼: 느리게 수렴, 더 많은 iteration 필요
std::normal_distribution<double> noise(0.0, 5.0);
```

---

## ✅ 체크리스트

- [ ] Ceres 설치 확인 (pkg-config --modversion ceres)
- [ ] BAL 데이터셋 다운로드
- [ ] simple_ba.cpp 빌드 및 실행
- [ ] 수렴 과정 (cost 감소) 관찰
- [ ] HuberLoss 유무 비교 실험
- [ ] Solver 타입 비교 실험 (선택)
- [ ] BAL 데이터로 대규모 BA 실행 (선택)

---

## 💡 핵심 개념

1. **Snavely 카메라 모델**
   ```
   9파라미터: angle-axis(3) + translation(3) + focal(1) + k1,k2(2)
   → Week 8의 6파라미터보다 현실적
   → 방사 왜곡까지 고려
   ```

2. **BAL 데이터셋**
   ```
   실제 SfM 데이터
   → 합성 데이터가 아닌 진짜 이미지에서 추출
   → 다양한 규모의 문제 제공
   ```

3. **수렴 관찰**
   ```
   Iteration별 cost 변화가 핵심!
   → 초기: 급격한 감소
   → 후반: 완만한 수렴
   → Termination: CONVERGENCE 확인
   ```

4. **Solver 선택**
   ```
   소규모 (카메라 <50):    DENSE_SCHUR
   중규모 (카메라 50~500):  SPARSE_SCHUR
   대규모 (카메라 500+):    ITERATIVE_SCHUR
   ```

---

## 🏗️ mini_slam 구현

> Week 11은 Ceres BAL 실습으로, mini_slam에 직접 코드를 추가하지 않는다.
> 단, BAL 데이터셋의 대규모 BA 실행 경험이 mini_slam의 Ceres BA 구현(Week 8)을 보강한다.
> Solver 타입(DENSE_SCHUR vs SPARSE_SCHUR) 비교 결과를 mini_slam에 반영할 수 있다.

---

**다음**: Week 12에서 Monocular 스케일 모호성을 다룹니다.
