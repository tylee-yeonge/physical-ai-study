# Week 10 실습: g2o Solver 조합 비교 실험

> 🎯 **목표**: BlockSolver, LinearSolver 조합 변경 후 성능 비교
> 💻 **언어**: C++ (g2o, Eigen)
> ⏰ **예상 시간**: 8시간

---

## 📋 실습 개요

Week 7에서 g2o BA를 "돌려봤다면", 이번 주는 Solver 내부 설정을 바꿔가며 **"왜 이렇게 설정하는지"** 체감합니다.

**먼저 돌려보고, 모르는 것을 채운다 (원칙 1)**:
→ Solver 조합을 바꿔보고 → 속도/정확도 차이 관찰 → README 이론으로 돌아가기

---

## 🔧 환경 설정

```bash
# g2o 설치 확인 (Week 7에서 설치)
ls /usr/local/lib/libg2o*

# Eigen3
pkg-config --modversion eigen3
```

---

## 프로젝트 구조

```
week10_g2o_solver/
├── CMakeLists.txt
└── src/
    └── main.cpp
```

---

## Step 1: Solver 조합 비교 실험

### src/main.cpp

```cpp
#include <g2o/core/sparse_optimizer.h>
#include <g2o/core/block_solver.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include <g2o/core/optimization_algorithm_gauss_newton.h>
#include <g2o/solvers/eigen/linear_solver_eigen.h>
#include <g2o/solvers/dense/linear_solver_dense.h>
#include <g2o/types/sba/types_six_dof_expmap.h>

#include <Eigen/Dense>
#include <iostream>
#include <random>
#include <chrono>

/**
 * 합성 BA 데이터 생성
 */
struct BAData {
    std::vector<g2o::SE3Quat> gt_poses;
    std::vector<Eigen::Vector3d> gt_points;

    struct Obs {
        int cam_id, pt_id;
        Eigen::Vector2d uv;
    };
    std::vector<Obs> observations;

    double fx, fy, cx, cy;
};

BAData generateData(int num_cameras, int num_points) {
    BAData data;
    data.fx = 500; data.fy = 500;
    data.cx = 320; data.cy = 240;

    std::default_random_engine gen(42);
    std::normal_distribution<double> noise(0.0, 1.0);
    std::uniform_real_distribution<double> pt_dist(-3.0, 3.0);

    // 카메라 (일직선 이동)
    for (int i = 0; i < num_cameras; i++) {
        Eigen::Quaterniond q(1, 0, 0, 0);  // identity
        Eigen::Vector3d t(i * 0.5, 0, 0);
        data.gt_poses.push_back(g2o::SE3Quat(q, t));
    }

    // 3D 점
    for (int i = 0; i < num_points; i++) {
        Eigen::Vector3d p(pt_dist(gen), pt_dist(gen), 5.0 + pt_dist(gen));
        data.gt_points.push_back(p);
    }

    // 관측 생성
    for (int cam = 0; cam < num_cameras; cam++) {
        for (int pt = 0; pt < num_points; pt++) {
            // 카메라 좌표로 변환
            Eigen::Vector3d p_cam = data.gt_poses[cam].map(data.gt_points[pt]);

            if (p_cam.z() > 0.1) {
                double u = data.fx * p_cam.x() / p_cam.z() + data.cx + noise(gen);
                double v = data.fy * p_cam.y() / p_cam.z() + data.cy + noise(gen);

                if (u > 0 && u < 640 && v > 0 && v < 480) {
                    data.observations.push_back(
                        {cam, pt, Eigen::Vector2d(u, v)});
                }
            }
        }
    }

    return data;
}

/**
 * g2o BA 구성 및 최적화 실행
 *
 * use_marginalized: Schur Complement 사용 여부
 * use_levenberg: LM vs GN 선택
 */
double runBA(
    const BAData& data,
    bool use_marginalized,
    bool use_levenberg,
    int max_iterations
) {
    // ─────── Solver 구성 ───────
    // Layer 1: LinearSolver
    typedef g2o::BlockSolver_6_3 BlockSolverType;
    auto linearSolver = g2o::make_unique<
        g2o::LinearSolverEigen<BlockSolverType::PoseMatrixType>>();

    // Layer 2: BlockSolver
    auto blockSolver = g2o::make_unique<BlockSolverType>(
        std::move(linearSolver));

    // Layer 3: Algorithm
    g2o::OptimizationAlgorithm* algorithm;
    if (use_levenberg) {
        algorithm = new g2o::OptimizationAlgorithmLevenberg(
            std::move(blockSolver));
    } else {
        algorithm = new g2o::OptimizationAlgorithmGaussNewton(
            std::move(blockSolver));
    }

    // Optimizer
    g2o::SparseOptimizer optimizer;
    optimizer.setAlgorithm(algorithm);
    optimizer.setVerbose(false);

    // ─────── 카메라 내부 파라미터 ───────
    g2o::CameraParameters* cam_params =
        new g2o::CameraParameters(data.fx, Eigen::Vector2d(data.cx, data.cy), 0);
    cam_params->setId(0);
    optimizer.addParameter(cam_params);

    // ─────── Vertex 추가 ───────
    std::default_random_engine gen(123);
    std::normal_distribution<double> pose_noise(0.0, 0.05);
    std::normal_distribution<double> pt_noise(0.0, 0.3);

    // 카메라 포즈
    for (size_t i = 0; i < data.gt_poses.size(); i++) {
        auto* v = new g2o::VertexSE3Expmap();
        v->setId((int)i);

        // 첫 카메라 고정, 나머지는 노이즈 추가
        if (i == 0) {
            v->setEstimate(data.gt_poses[i]);
            v->setFixed(true);
        } else {
            g2o::SE3Quat noisy = data.gt_poses[i];
            Eigen::Vector3d t_noise(pose_noise(gen), pose_noise(gen), pose_noise(gen));
            noisy.setTranslation(noisy.translation() + t_noise);
            v->setEstimate(noisy);
        }
        optimizer.addVertex(v);
    }

    // 3D 점
    int cam_count = (int)data.gt_poses.size();
    for (size_t i = 0; i < data.gt_points.size(); i++) {
        auto* v = new g2o::VertexPointXYZ();
        v->setId(cam_count + (int)i);

        Eigen::Vector3d noisy = data.gt_points[i] +
            Eigen::Vector3d(pt_noise(gen), pt_noise(gen), pt_noise(gen));
        v->setEstimate(noisy);
        v->setMarginalized(use_marginalized);  // 핵심!
        optimizer.addVertex(v);
    }

    // ─────── Edge 추가 ───────
    for (const auto& obs : data.observations) {
        auto* edge = new g2o::EdgeProjectXYZ2UV();
        edge->setVertex(0,
            dynamic_cast<g2o::OptimizableGraph::Vertex*>(
                optimizer.vertex(cam_count + obs.pt_id)));
        edge->setVertex(1,
            dynamic_cast<g2o::OptimizableGraph::Vertex*>(
                optimizer.vertex(obs.cam_id)));
        edge->setMeasurement(obs.uv);
        edge->setInformation(Eigen::Matrix2d::Identity());
        edge->setParameterId(0, 0);

        // Robust kernel
        auto* rk = new g2o::RobustKernelHuber;
        rk->setDelta(1.0);
        edge->setRobustKernel(rk);

        optimizer.addEdge(edge);
    }

    // ─────── 최적화 실행 ───────
    optimizer.initializeOptimization();

    auto start = std::chrono::high_resolution_clock::now();
    optimizer.optimize(max_iterations);
    auto end = std::chrono::high_resolution_clock::now();

    double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    return elapsed_ms;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 10: g2o Solver 조합 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ═══════════════════════════════════
    // 실험 1: 규모별 비교
    // ═══════════════════════════════════

    std::cout << "실험 1: Marginalize 효과\n" << std::endl;
    std::cout << "  규모    | Marginalized | No-Marginalize" << std::endl;
    std::cout << "  --------|--------------|---------------" << std::endl;

    int sizes[][2] = {{5, 50}, {10, 200}, {20, 500}};
    for (auto& sz : sizes) {
        BAData data = generateData(sz[0], sz[1]);

        double t_marg = runBA(data, true, true, 10);
        double t_nomarg = runBA(data, false, true, 10);

        printf("  %2d cam, %3d pts | %7.1f ms   | %7.1f ms\n",
               sz[0], sz[1], t_marg, t_nomarg);
    }

    // ═══════════════════════════════════
    // 실험 2: LM vs GN
    // ═══════════════════════════════════

    std::cout << "\n실험 2: Levenberg-Marquardt vs Gauss-Newton\n" << std::endl;

    BAData data = generateData(10, 200);

    double t_lm = runBA(data, true, true, 10);
    double t_gn = runBA(data, true, false, 10);

    std::cout << "  LM: " << t_lm << " ms" << std::endl;
    std::cout << "  GN: " << t_gn << " ms" << std::endl;

    // ═══════════════════════════════════
    // 결과 요약
    // ═══════════════════════════════════

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "핵심 관찰:" << std::endl;
    std::cout << "  1. Marginalize(Schur)가 큰 문제에서 확연히 빠름" << std::endl;
    std::cout << "  2. LM이 GN보다 안정적 (발산하지 않음)" << std::endl;
    std::cout << "  3. 문제 규모가 커질수록 차이 벌어짐" << std::endl;
    std::cout << "\nSLAM에서의 의미:" << std::endl;
    std::cout << "  - ORB-SLAM: BlockSolver_6_3 + Marginalize + LM" << std::endl;
    std::cout << "  - VINS: Ceres + Schur (DENSE_SCHUR)" << std::endl;
    std::cout << "  - 둘 다 Schur Complement가 핵심!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week10_g2o_Solver)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# g2o
find_package(g2o REQUIRED)
include_directories(${G2O_INCLUDE_DIRS})

# Executable
add_executable(solver_compare src/main.cpp)
target_link_libraries(solver_compare
    g2o_core
    g2o_stuff
    g2o_types_sba
    g2o_solver_eigen
    g2o_solver_dense
    Eigen3::Eigen
)
```

---

## 빌드 및 실행

```bash
cd week10_g2o_solver
mkdir build && cd build
cmake ..
make
./solver_compare
```

### 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 10: g2o Solver 조합 비교
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

실험 1: Marginalize 효과

  규모    | Marginalized | No-Marginalize
  --------|--------------|---------------
   5 cam,  50 pts |     3.2 ms   |     4.1 ms
  10 cam, 200 pts |    12.5 ms   |    28.7 ms
  20 cam, 500 pts |    45.3 ms   |   185.2 ms

실험 2: Levenberg-Marquardt vs Gauss-Newton

  LM: 12.5 ms
  GN: 10.8 ms

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
핵심 관찰:
  1. Marginalize(Schur)가 큰 문제에서 확연히 빠름
  2. LM이 GN보다 안정적 (발산하지 않음)
  3. 문제 규모가 커질수록 차이 벌어짐

SLAM에서의 의미:
  - ORB-SLAM: BlockSolver_6_3 + Marginalize + LM
  - VINS: Ceres + Schur (DENSE_SCHUR)
  - 둘 다 Schur Complement가 핵심!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] g2o BA 빌드 및 실행 성공
- [ ] Marginalize 유무에 따른 속도 차이 관찰
- [ ] LM vs GN 차이 확인
- [ ] "BlockSolver_6_3에서 6과 3의 의미"를 설명할 수 있음
- [ ] "Schur Complement가 BA를 빠르게 하는 이유"를 설명할 수 있음

---

## 💡 핵심 포인트

1. **setMarginalized(true)** = Schur Complement 적용
   - 3D 점을 소거 → 카메라만 풀기 → 점 back-substitution
   - 대규모에서 수십 배 빠름

2. **BlockSolver_6_3**: 카메라 6DoF (SE3) + 점 3DoF (XYZ)
   - Hessian을 6x6, 3x3 블록으로 관리

3. **LM vs GN**: LM이 더 안정적 (damping), GN이 약간 더 빠름

---

## 🏗️ mini_slam 구현 (이번 주 핵심)

> 이번 주는 Week 9에서 시작한 g2o 로컬 BA를 **완성**하고, mini_slam에 붙인다.
> g2o 내부 solver 구조를 이해하고, 실제 키프레임 데이터로 LocalBA를 실행한다.

**작업 내용**:

| 작업 | 내용 |
|------|------|
| `local_ba_g2o.h` 완성 | BlockSolver_6_3 + LM + Robust Kernel 구성 |
| mini_slam에 LocalBA 붙이기 | 키프레임 추가 시 최근 N개 키프레임에 대해 g2o BA 실행 |
| BA 전후 비교 | 드리프트 감소량 정량 측정 |

**구현 파일**:
- `Studies/Phase 3/mini_slam/include/local_ba_g2o.h` (완성)
- `Studies/Phase 3/mini_slam/src/local_ba_g2o.cpp` (완성)
- `Studies/Phase 3/mini_slam/main.cpp` (LocalBA 호출 추가)

### 완성 기준

```bash
cd Studies/Phase\ 3/mini_slam/build
./mini_slam

# mini_slam 파이프라인에 LocalBA 적용
# BA 전: 드리프트 X m → BA 후: 드리프트 Y m (Y < X)
# 최근 N개 키프레임 + 관측 맵 포인트 최적화 동작 확인
```

---

**다음**: [Week 11 - Ceres 실습](../week11/README.md)
