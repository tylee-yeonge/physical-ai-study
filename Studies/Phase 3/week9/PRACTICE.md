# Week 9 실습: Schur Complement 이해하기

> 🎯 **목표**: Schur Complement의 원리를 직접 체험
> 💻 **언어**: C++ (Eigen)
> ⏰ **예상 시간**: 7시간

---

## 📋 실습 개요

Week 9 실습은 3단계로 구성됩니다:

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | 간단한 블록 예제로 Schur 직접 계산 | 필수 | 3시간 |
| 2 | g2o/Ceres에서 옵션 비교 | 필수 | 2시간 |
| 3 | VINS 코드 분석 | 선택 | 2시간 |

---

## 🔧 환경 설정

```bash
# Eigen (필수)
sudo apt install libeigen3-dev

# g2o, Ceres (Week 7, 8에서 설치했다면 생략)
```

---

## Step 1: Schur Complement 직접 계산

### 1.1 목표

작은 BA 문제에서 Schur Complement를 **직접** 계산해보고, 일반 해법과 결과가 같음을 확인합니다.

### 1.2 문제 설정

```
2개 카메라 + 3개 점의 간단한 BA

변수:
- c1, c2: 카메라 (각 6차원) → 총 12차원
- p1, p2, p3: 점 (각 3차원) → 총 9차원
- 전체: 21차원

실습에서는 단순화:
- 카메라: 각 2차원 (tx, ty만)
- 점: 각 2차원 (x, y만)
- 전체: 10차원
```

### 1.3 프로젝트 구조

```
week9_schur/
├── CMakeLists.txt
└── schur_demo.cpp
```

### 1.4 코드: schur_demo.cpp

```cpp
/**
 * Week 9 실습: Schur Complement 직접 이해
 *
 * 목표: Schur Complement가 왜 빠른지 직접 확인
 */

#include <iostream>
#include <Eigen/Dense>
#include <chrono>

using namespace Eigen;
using namespace std;

/**
 * 간단한 BA 문제에서 Schur Complement 시연
 *
 * 설정:
 * - 2개 카메라 (각 2차원: tx, ty)
 * - 3개 점 (각 2차원: x, y)
 * - Hessian은 10x10
 */
void schurComplementDemo() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Step 1: Schur Complement 직접 계산" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    // 문제 크기
    const int n_cameras = 2;
    const int n_points = 3;
    const int cam_dim = 2;   // 간소화: tx, ty만
    const int point_dim = 2; // 간소화: x, y만

    const int Nc = n_cameras * cam_dim;  // 4
    const int Np = n_points * point_dim; // 6
    const int N = Nc + Np;               // 10

    cout << "문제 크기:" << endl;
    cout << "  카메라: " << n_cameras << " x " << cam_dim << " = " << Nc << " 변수" << endl;
    cout << "  점: " << n_points << " x " << point_dim << " = " << Np << " 변수" << endl;
    cout << "  전체: " << N << " 변수\n" << endl;

    // ============================================
    // Hessian 행렬 생성 (실제로는 J^T·J로 계산)
    // ============================================

    // BA의 Hessian 구조:
    // H = [ Hcc  Hcp ]
    //     [ Hpc  Hpp ]

    // Hcc: 카메라-카메라 (밀집)
    MatrixXd Hcc = MatrixXd::Random(Nc, Nc);
    Hcc = Hcc * Hcc.transpose();  // 양정치 대칭으로
    Hcc += 5.0 * MatrixXd::Identity(Nc, Nc);  // 대각 강화

    // Hpp: 점-점 (블록 대각!)
    // 핵심: 각 점은 독립적 → 블록 대각 구조
    MatrixXd Hpp = MatrixXd::Zero(Np, Np);
    for (int i = 0; i < n_points; i++) {
        MatrixXd block = MatrixXd::Random(point_dim, point_dim);
        block = block * block.transpose() + 3.0 * MatrixXd::Identity(point_dim, point_dim);
        Hpp.block(i * point_dim, i * point_dim, point_dim, point_dim) = block;
    }

    // Hcp: 카메라-점 (희소)
    MatrixXd Hcp = MatrixXd::Random(Nc, Np) * 0.5;

    // Hpc = Hcp^T
    MatrixXd Hpc = Hcp.transpose();

    // 전체 Hessian 조립
    MatrixXd H(N, N);
    H.block(0, 0, Nc, Nc) = Hcc;
    H.block(0, Nc, Nc, Np) = Hcp;
    H.block(Nc, 0, Np, Nc) = Hpc;
    H.block(Nc, Nc, Np, Np) = Hpp;

    // b 벡터 생성
    VectorXd b = VectorXd::Random(N);
    VectorXd bc = b.head(Nc);
    VectorXd bp = b.tail(Np);

    cout << "Hessian 구조 시각화:" << endl;
    cout << "         카메라(" << Nc << ")  점(" << Np << ")" << endl;
    cout << "        ┌────────┬────────┐" << endl;
    cout << "카메라  │  Hcc   │  Hcp   │" << endl;
    cout << "        │ (밀집) │ (희소) │" << endl;
    cout << "        ├────────┼────────┤" << endl;
    cout << "점      │  Hpc   │  Hpp   │" << endl;
    cout << "        │ (희소) │(블록대각)│" << endl;
    cout << "        └────────┴────────┘\n" << endl;

    // ============================================
    // 방법 1: 일반적인 해법 (전체 역행렬)
    // ============================================
    cout << "방법 1: 일반 해법 (전체 H 역행렬)" << endl;

    auto t1_start = chrono::high_resolution_clock::now();

    VectorXd x_direct = H.ldlt().solve(b);

    auto t1_end = chrono::high_resolution_clock::now();
    auto t1_duration = chrono::duration_cast<chrono::microseconds>(t1_end - t1_start);

    VectorXd dc_direct = x_direct.head(Nc);
    VectorXd dp_direct = x_direct.tail(Np);

    cout << "  Δc (카메라): " << dc_direct.transpose() << endl;
    cout << "  Δp (점): " << dp_direct.transpose() << endl;
    cout << "  시간: " << t1_duration.count() << " μs\n" << endl;

    // ============================================
    // 방법 2: Schur Complement
    // ============================================
    cout << "방법 2: Schur Complement" << endl;

    auto t2_start = chrono::high_resolution_clock::now();

    // Step 2a: Hpp 역행렬 (블록 대각이라 쉬움!)
    MatrixXd Hpp_inv = MatrixXd::Zero(Np, Np);
    for (int i = 0; i < n_points; i++) {
        MatrixXd block = Hpp.block(i * point_dim, i * point_dim, point_dim, point_dim);
        Hpp_inv.block(i * point_dim, i * point_dim, point_dim, point_dim) = block.inverse();
    }

    // Step 2b: Schur complement 계산
    // S = Hcc - Hcp * Hpp^(-1) * Hpc
    MatrixXd S = Hcc - Hcp * Hpp_inv * Hpc;

    // Step 2c: 수정된 b
    // b' = bc - Hcp * Hpp^(-1) * bp
    VectorXd b_prime = bc - Hcp * Hpp_inv * bp;

    // Step 2d: 카메라만 풀기 (작은 시스템!)
    VectorXd dc_schur = S.ldlt().solve(b_prime);

    // Step 2e: 점은 back-substitution
    // Δp = Hpp^(-1) * (bp - Hpc * Δc)
    VectorXd dp_schur = Hpp_inv * (bp - Hpc * dc_schur);

    auto t2_end = chrono::high_resolution_clock::now();
    auto t2_duration = chrono::duration_cast<chrono::microseconds>(t2_end - t2_start);

    cout << "  Δc (카메라): " << dc_schur.transpose() << endl;
    cout << "  Δp (점): " << dp_schur.transpose() << endl;
    cout << "  시간: " << t2_duration.count() << " μs\n" << endl;

    // ============================================
    // 결과 비교
    // ============================================
    cout << "결과 비교:" << endl;
    double error_c = (dc_direct - dc_schur).norm();
    double error_p = (dp_direct - dp_schur).norm();

    cout << "  카메라 차이: " << error_c << endl;
    cout << "  점 차이: " << error_p << endl;

    if (error_c < 1e-10 && error_p < 1e-10) {
        cout << "\n✅ 두 방법의 결과가 동일합니다!" << endl;
    } else {
        cout << "\n⚠️ 수치 오차가 있습니다 (정상)" << endl;
    }

    // ============================================
    // 핵심 포인트
    // ============================================
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "💡 핵심 포인트" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "1. Schur Complement로 문제 크기 감소:" << endl;
    cout << "   원래: " << N << " x " << N << " 시스템" << endl;
    cout << "   Schur 후: " << Nc << " x " << Nc << " 시스템\n" << endl;

    cout << "2. Hpp가 블록 대각이라 역행렬 쉬움:" << endl;
    cout << "   전체 역행렬: O(" << Np << "³)" << endl;
    cout << "   블록별 역행렬: O(" << n_points << " x " << point_dim << "³)\n" << endl;

    cout << "3. 실제 BA에서는:" << endl;
    cout << "   카메라: ~100개 (600 변수)" << endl;
    cout << "   점: ~10,000개 (30,000 변수)" << endl;
    cout << "   → Schur로 30,600 → 600 감소!\n" << endl;
}

/**
 * 규모에 따른 성능 비교
 */
void scalingDemo() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "성능 스케일링 테스트" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "점 개수 | 전체 크기 | Direct (μs) | Schur (μs) | 비율" << endl;
    cout << "--------|----------|-------------|------------|-----" << endl;

    for (int n_points : {5, 10, 20, 50, 100}) {
        const int n_cameras = 3;
        const int cam_dim = 2;
        const int point_dim = 2;

        const int Nc = n_cameras * cam_dim;
        const int Np = n_points * point_dim;
        const int N = Nc + Np;

        // Hessian 생성
        MatrixXd Hcc = MatrixXd::Random(Nc, Nc);
        Hcc = Hcc * Hcc.transpose() + 5.0 * MatrixXd::Identity(Nc, Nc);

        MatrixXd Hpp = MatrixXd::Zero(Np, Np);
        for (int i = 0; i < n_points; i++) {
            MatrixXd block = MatrixXd::Random(point_dim, point_dim);
            block = block * block.transpose() + 3.0 * MatrixXd::Identity(point_dim, point_dim);
            Hpp.block(i * point_dim, i * point_dim, point_dim, point_dim) = block;
        }

        MatrixXd Hcp = MatrixXd::Random(Nc, Np) * 0.5;
        MatrixXd Hpc = Hcp.transpose();

        MatrixXd H(N, N);
        H.block(0, 0, Nc, Nc) = Hcc;
        H.block(0, Nc, Nc, Np) = Hcp;
        H.block(Nc, 0, Np, Nc) = Hpc;
        H.block(Nc, Nc, Np, Np) = Hpp;

        VectorXd b = VectorXd::Random(N);
        VectorXd bc = b.head(Nc);
        VectorXd bp = b.tail(Np);

        // Direct solve
        auto t1_start = chrono::high_resolution_clock::now();
        VectorXd x_direct = H.ldlt().solve(b);
        auto t1_end = chrono::high_resolution_clock::now();
        auto t1 = chrono::duration_cast<chrono::microseconds>(t1_end - t1_start).count();

        // Schur solve
        auto t2_start = chrono::high_resolution_clock::now();

        MatrixXd Hpp_inv = MatrixXd::Zero(Np, Np);
        for (int i = 0; i < n_points; i++) {
            MatrixXd block = Hpp.block(i * point_dim, i * point_dim, point_dim, point_dim);
            Hpp_inv.block(i * point_dim, i * point_dim, point_dim, point_dim) = block.inverse();
        }
        MatrixXd S = Hcc - Hcp * Hpp_inv * Hpc;
        VectorXd b_prime = bc - Hcp * Hpp_inv * bp;
        VectorXd dc = S.ldlt().solve(b_prime);
        VectorXd dp = Hpp_inv * (bp - Hpc * dc);

        auto t2_end = chrono::high_resolution_clock::now();
        auto t2 = chrono::duration_cast<chrono::microseconds>(t2_end - t2_start).count();

        double ratio = (double)t1 / t2;

        printf("   %3d  |    %3d   |    %5ld    |    %5ld   | %.1fx\n",
               n_points, N, t1, t2, ratio);
    }

    cout << "\n💡 점이 많아질수록 Schur의 이점이 커집니다!" << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 9: Schur Complement 직접 이해하기" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    schurComplementDemo();
    scalingDemo();

    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "✅ Step 1 완료!" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
```

---

## Step 2: g2o/Ceres 옵션 비교

### 2.1 목표

`setMarginalized(true)` vs `setMarginalized(false)`의 성능 차이를 직접 확인합니다.

### 2.2 코드: compare_options.cpp

```cpp
/**
 * Week 9 실습 Step 2: g2o 옵션 비교
 *
 * setMarginalized(true) vs setMarginalized(false)
 */

#include <g2o/core/sparse_optimizer.h>
#include <g2o/core/block_solver.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include <g2o/solvers/eigen/linear_solver_eigen.h>
#include <g2o/types/sba/types_six_dof_expmap.h>
#include <iostream>
#include <random>
#include <chrono>

using namespace std;

/**
 * BA 실행 및 시간 측정
 * @param use_schur setMarginalized(true) 사용 여부
 * @return 실행 시간 (ms)
 */
double runBA(int n_cameras, int n_points, bool use_schur) {
    // Optimizer 설정
    g2o::SparseOptimizer optimizer;

    typedef g2o::BlockSolver<g2o::BlockSolverTraits<6, 3>> BlockSolverType;
    typedef g2o::LinearSolverEigen<BlockSolverType::PoseMatrixType> LinearSolverType;

    auto solver = new g2o::OptimizationAlgorithmLevenberg(
        g2o::make_unique<BlockSolverType>(
            g2o::make_unique<LinearSolverType>()
        )
    );
    optimizer.setAlgorithm(solver);
    optimizer.setVerbose(false);

    // 카메라 파라미터
    double fx = 500, fy = 500, cx = 320, cy = 240;
    g2o::CameraParameters* cam = new g2o::CameraParameters(fx, Eigen::Vector2d(cx, cy), 0);
    cam->setId(0);
    optimizer.addParameter(cam);

    // 랜덤 생성기
    default_random_engine gen(42);
    uniform_real_distribution<double> dist(-2.0, 2.0);
    normal_distribution<double> noise(0, 1.0);

    // 카메라 추가
    for (int i = 0; i < n_cameras; i++) {
        g2o::VertexSE3Expmap* v = new g2o::VertexSE3Expmap();
        v->setId(i);

        Eigen::Vector3d t(i * 0.5, 0, 0);
        v->setEstimate(g2o::SE3Quat(Eigen::Matrix3d::Identity(), t));

        if (i == 0) v->setFixed(true);
        optimizer.addVertex(v);
    }

    // 점 추가
    for (int j = 0; j < n_points; j++) {
        g2o::VertexPointXYZ* v = new g2o::VertexPointXYZ();
        v->setId(n_cameras + j);
        v->setEstimate(Eigen::Vector3d(dist(gen), dist(gen), 5.0 + dist(gen)));

        // 핵심: Schur complement 사용 여부!
        v->setMarginalized(use_schur);

        optimizer.addVertex(v);
    }

    // Edge 추가 (모든 카메라-점 조합)
    int edge_id = 0;
    for (int i = 0; i < n_cameras; i++) {
        for (int j = 0; j < n_points; j++) {
            // 랜덤하게 일부만 관측 (80% 확률)
            if (rand() % 10 < 8) {
                g2o::EdgeProjectXYZ2UV* e = new g2o::EdgeProjectXYZ2UV();
                e->setVertex(0, optimizer.vertex(n_cameras + j));
                e->setVertex(1, optimizer.vertex(i));
                e->setMeasurement(Eigen::Vector2d(cx + noise(gen), cy + noise(gen)));
                e->setInformation(Eigen::Matrix2d::Identity());
                e->setParameterId(0, 0);
                optimizer.addEdge(e);
                edge_id++;
            }
        }
    }

    // 최적화 실행 및 시간 측정
    optimizer.initializeOptimization();

    auto start = chrono::high_resolution_clock::now();
    optimizer.optimize(10);
    auto end = chrono::high_resolution_clock::now();

    return chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Step 2: g2o setMarginalized 옵션 비교" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "점 개수 | Schur OFF (ms) | Schur ON (ms) | 속도 향상" << endl;
    cout << "--------|----------------|---------------|----------" << endl;

    for (int n_points : {50, 100, 200, 500, 1000}) {
        int n_cameras = 10;

        double time_off = runBA(n_cameras, n_points, false);
        double time_on = runBA(n_cameras, n_points, true);

        double speedup = time_off / time_on;

        printf("  %4d  |     %7.2f    |     %7.2f   |   %.1fx\n",
               n_points, time_off, time_on, speedup);
    }

    cout << "\n💡 점이 많을수록 setMarginalized(true)의 효과가 큽니다!" << endl;
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "✅ Step 2 완료!" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
```

---

## Step 3: VINS 코드 분석 (선택/고급)

### 3.1 목표

VINS-Mono의 optimization 코드에서 Schur Complement가 어떻게 사용되는지 확인합니다.

### 3.2 VINS 코드 클론

```bash
git clone https://github.com/HKUST-Aerial-Robotics/VINS-Mono.git
cd VINS-Mono
```

### 3.3 분석할 파일

```
vins_estimator/src/estimator.cpp
```

### 3.4 핵심 코드 위치

```cpp
// estimator.cpp: optimization() 함수

void Estimator::optimization()
{
    // ... (생략)

    ceres::Solver::Options options;

    // 👇 여기! Schur Complement 사용
    options.linear_solver_type = ceres::DENSE_SCHUR;

    options.trust_region_strategy_type = ceres::DOGLEG;
    options.max_num_iterations = NUM_ITERATIONS;

    // ... (생략)

    ceres::Solve(options, &problem, &summary);
}
```

### 3.5 분석 포인트

1. **왜 DENSE_SCHUR인가?**
   - Sliding window 크기가 작음 (10~11개 키프레임)
   - 카메라 변수가 적어서 DENSE로 충분

2. **언제 SPARSE_SCHUR를 쓸까?**
   - 키프레임이 100개 이상일 때
   - Global BA 시

3. **Marginalization과의 관계**
   - `marginalization_factor.cpp` 참고
   - 오래된 키프레임을 marginalize할 때도 Schur 사용

### 3.6 체크리스트

- [ ] `optimization()` 함수 위치 확인
- [ ] `DENSE_SCHUR` 설정 확인
- [ ] `marginalization_factor.cpp` 구조 파악 (선택)

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week9_Schur)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Step 1: Schur demo (Eigen만 필요)
add_executable(schur_demo schur_demo.cpp)

# Step 2: g2o 비교 (g2o 필요)
find_package(G2O QUIET)
if(G2O_FOUND)
    include_directories(${G2O_INCLUDE_DIR})
    add_executable(compare_options compare_options.cpp)
    target_link_libraries(compare_options
        ${G2O_CORE_LIBRARY}
        ${G2O_STUFF_LIBRARY}
        ${G2O_TYPES_SBA}
        ${G2O_SOLVER_EIGEN}
    )
else()
    message(STATUS "g2o not found, skipping compare_options")
endif()
```

---

## 빌드 및 실행

```bash
cd week9_schur
mkdir build && cd build
cmake ..
make

# Step 1 실행
./schur_demo

# Step 2 실행 (g2o 설치 필요)
./compare_options
```

---

## 예상 출력

### Step 1 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 9: Schur Complement 직접 이해하기
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Step 1: Schur Complement 직접 계산
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

문제 크기:
  카메라: 2 x 2 = 4 변수
  점: 3 x 2 = 6 변수
  전체: 10 변수

방법 1: 일반 해법 (전체 H 역행렬)
  시간: 15 μs

방법 2: Schur Complement
  시간: 8 μs

✅ 두 방법의 결과가 동일합니다!

성능 스케일링 테스트

점 개수 | 전체 크기 | Direct (μs) | Schur (μs) | 비율
--------|----------|-------------|------------|-----
     5  |     16   |       18    |       12   | 1.5x
    10  |     26   |       45    |       20   | 2.3x
    20  |     46   |      180    |       45   | 4.0x
    50  |    106   |     1200    |      150   | 8.0x
   100  |    206   |     8500    |      400   | 21.3x

💡 점이 많아질수록 Schur의 이점이 커집니다!
```

---

## ✅ 체크리스트

### Step 1
- [ ] schur_demo.cpp 이해
- [ ] Direct vs Schur 결과 동일 확인
- [ ] 스케일링 테스트 실행

### Step 2
- [ ] g2o 설치 확인
- [ ] setMarginalized 옵션 비교 실행
- [ ] 속도 향상 확인

### Step 3 (선택)
- [ ] VINS-Mono 클론
- [ ] optimization() 함수 찾기
- [ ] DENSE_SCHUR 설정 확인

---

## 💡 핵심 개념 요약

### Schur Complement 단계

```
1. Hpp 역행렬 계산 (블록 대각이라 쉬움)
2. S = Hcc - Hcp * Hpp^(-1) * Hpc
3. b' = bc - Hcp * Hpp^(-1) * bp
4. Δc = S^(-1) * b' (작은 시스템)
5. Δp = Hpp^(-1) * (bp - Hpc * Δc)
```

### 왜 빠른가?

```
원래: O(N³) where N = 카메라 + 점
Schur: O(Nc³) + O(M) where Nc << N

점이 많을수록 효과 큼!
```

### 코드에서는 한 줄

```cpp
// g2o
v_point->setMarginalized(true);

// Ceres
options.linear_solver_type = ceres::DENSE_SCHUR;
```

---

**다음**: Quiz로 개념 점검!
