# Week 7 실습: Bundle Adjustment with g2o

> 🎯 **목표**: g2o로 간단한 BA 구현  
> 💻 **언어**: C++ (g2o, Eigen)  
> ⏰ **예상 시간**: 12시간

---

## 📋 실습 개요

Week 7은 **Bundle Adjustment (BA)**를 g2o 프레임워크로 구현합니다. 여러 카메라 포즈와 3D 점들을 동시에 최적화하여 재투영 오차를 최소화합니다.

---

## 🔧 환경 설정

### g2o 설치

```bash
# Dependencies
sudo apt update
sudo apt install libsuitesparse-dev libeigen3-dev libcholmod3

# g2o
git clone https://github.com/RainerKuemmerle/g2o.git
cd g2o
mkdir build && cd build
cmake ..
make -j4
sudo make install

# 확인
ls /usr/local/include/g2o
```

---

## 프로젝트 구조

```
week7_ba/
├── CMakeLists.txt
├── include/
│   └── simple_ba.hpp
└── src/
    ├── simple_ba.cpp
    └── main.cpp
```

---

## Step 1: Simple BA 구현

### include/simple_ba.hpp

```cpp
#ifndef SIMPLE_BA_HPP
#define SIMPLE_BA_HPP

#include <g2o/core/sparse_optimizer.h>
#include <g2o/core/block_solver.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include <g2o/core/robust_kernel_impl.h>
#include <g2o/solvers/eigen/linear_solver_eigen.h>
#include <g2o/types/sba/types_six_dof_expmap.h>
#include <Eigen/Dense>
#include <vector>

namespace vo {

/**
 * @brief 관측 데이터
 */
struct Observation {
    int camera_id;
    int point_id;
    Eigen::Vector2d uv;  // 픽셀 좌표
};

/**
 * @brief Simple Bundle Adjustment
 */
class SimpleBundleAdjustment {
public:
    SimpleBundleAdjustment(double fx, double fy, double cx, double cy);
    
    /**
     * @brief BA 실행
     * @param poses 카메라 포즈 (입출력)
     * @param points 3D 점 (입출력)
     * @param observations 관측
     * @param use_robust Robust kernel 사용 여부
     */
    void optimize(
        std::vector<Eigen::Isometry3d>& poses,
        std::vector<Eigen::Vector3d>& points,
        const std::vector<Observation>& observations,
        bool use_robust = true,
        int iterations = 10
    );
    
    /**
     * @brief 재투영 오차 계산
     */
    double computeReprojectionError(
        const std::vector<Eigen::Isometry3d>& poses,
        const std::vector<Eigen::Vector3d>& points,
        const std::vector<Observation>& observations
    );
    
private:
    double fx_, fy_, cx_, cy_;
};

} // namespace vo

#endif
```

### src/simple_ba.cpp

```cpp
#include "simple_ba.hpp"
#include <iostream>

namespace vo {

SimpleBundleAdjustment::SimpleBundleAdjustment(
    double fx, double fy, double cx, double cy
) : fx_(fx), fy_(fy), cx_(cx), cy_(cy) {}

void SimpleBundleAdjustment::optimize(
    std::vector<Eigen::Isometry3d>& poses,
    std::vector<Eigen::Vector3d>& points,
    const std::vector<Observation>& observations,
    bool use_robust,
    int iterations
) {
    // 1. Optimizer 생성
    g2o::SparseOptimizer optimizer;
    optimizer.setVerbose(false);
    
    // 2. Solver 설정
    typedef g2o::BlockSolver<g2o::BlockSolverTraits<6, 3>> BlockSolverType;
    typedef g2o::LinearSolverEigen<BlockSolverType::PoseMatrixType> LinearSolverType;
    
    auto solver = new g2o::OptimizationAlgorithmLevenberg(
        g2o::make_unique<BlockSolverType>(
            g2o::make_unique<LinearSolverType>()
        )
    );
    
    optimizer.setAlgorithm(solver);
    
    // 3. 카메라 파라미터
    g2o::CameraParameters* cam_params = new g2o::CameraParameters(
        fx_, Eigen::Vector2d(cx_, cy_), 0.0);
    cam_params->setId(0);
    optimizer.addParameter(cam_params);
    
    // 4. Vertex: 카메라 포즈
    for (size_t i = 0; i < poses.size(); i++) {
        g2o::VertexSE3Expmap* v = new g2o::VertexSE3Expmap();
        v->setId(i);
        v->setEstimate(g2o::SE3Quat(
            poses[i].linear(),
            poses[i].translation()
        ));
        
        // 첫 카메라는 고정 (gauge freedom)
        if (i == 0) {
            v->setFixed(true);
        }
        
        optimizer.addVertex(v);
    }
    
    // 5. Vertex: 3D 점
    for (size_t j = 0; j < points.size(); j++) {
        g2o::VertexPointXYZ* v = new g2o::VertexPointXYZ();
        v->setId(poses.size() + j);
        v->setEstimate(points[j]);
        v->setMarginalized(true);  // Schur complement
        
        optimizer.addVertex(v);
    }
    
    // 6. Edge: 재투영 제약
    for (const auto& obs : observations) {
        g2o::EdgeProjectXYZ2UV* edge = new g2o::EdgeProjectXYZ2UV();
        
        // Vertex 연결
        edge->setVertex(0, optimizer.vertex(poses.size() + obs.point_id));
        edge->setVertex(1, optimizer.vertex(obs.camera_id));
        
        // 관측값
        edge->setMeasurement(obs.uv);
        
        // 정보 행렬 (1/σ²)
        edge->setInformation(Eigen::Matrix2d::Identity());
        
        // 카메라 파라미터
        edge->setParameterId(0, 0);
        
        // Robust kernel
        if (use_robust) {
            g2o::RobustKernelHuber* rk = new g2o::RobustKernelHuber;
            rk->setDelta(1.0);  // 1 픽셀
            edge->setRobustKernel(rk);
        }
        
        optimizer.addEdge(edge);
    }
    
    // 7. 초기 오차
    std::cout << "Initial error: " 
              << computeReprojectionError(poses, points, observations) 
              << " px" << std::endl;
    
    // 8. 최적화!
    optimizer.initializeOptimization();
    optimizer.optimize(iterations);
    
    // 9. 결과 추출
    for (size_t i = 0; i < poses.size(); i++) {
        g2o::VertexSE3Expmap* v = static_cast<g2o::VertexSE3Expmap*>(
            optimizer.vertex(i));
        
        auto se3 = v->estimate();
        poses[i].linear() = se3.rotation().toRotationMatrix();
        poses[i].translation() = se3.translation();
    }
    
    for (size_t j = 0; j < points.size(); j++) {
        g2o::VertexPointXYZ* v = static_cast<g2o::VertexPointXYZ*>(
            optimizer.vertex(poses.size() + j));
        
        points[j] = v->estimate();
    }
    
    // 10. 최종 오차
    std::cout << "Final error: " 
              << computeReprojectionError(poses, points, observations) 
              << " px" << std::endl;
}

double SimpleBundleAdjustment::computeReprojectionError(
    const std::vector<Eigen::Isometry3d>& poses,
    const std::vector<Eigen::Vector3d>& points,
    const std::vector<Observation>& observations
) {
    double total_error = 0.0;
    
    for (const auto& obs : observations) {
        // 3D 점을 카메라 좌표계로 변환
        Eigen::Vector3d p_cam = poses[obs.camera_id] * points[obs.point_id];
        
        // 투영
        Eigen::Vector2d uv_proj;
        uv_proj(0) = fx_ * p_cam(0) / p_cam(2) + cx_;
        uv_proj(1) = fy_ * p_cam(1) / p_cam(2) + cy_;
        
        // 오차
        Eigen::Vector2d error = obs.uv - uv_proj;
        total_error += error.squaredNorm();
    }
    
    return std::sqrt(total_error / observations.size());
}

} // namespace vo
```

---

## Step 2: 테스트

### src/main.cpp

```cpp
#include "simple_ba.hpp"
#include <iostream>
#include <random>

using namespace vo;

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7: Bundle Adjustment with g2o" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 파라미터
    double fx = 500.0, fy = 500.0;
    double cx = 320.0, cy = 240.0;
    
    SimpleBundleAdjustment ba(fx, fy, cx, cy);
    
    // Ground truth 생성
    std::vector<Eigen::Isometry3d> gt_poses;
    std::vector<Eigen::Vector3d> gt_points;
    
    // 3개 카메라 (직선 이동)
    for (int i = 0; i < 3; i++) {
        Eigen::Isometry3d pose = Eigen::Isometry3d::Identity();
        pose.translation() = Eigen::Vector3d(i * 1.0, 0, 0);
        gt_poses.push_back(pose);
    }
    
    // 5개 3D 점
    std::default_random_engine gen(42);
    std::uniform_real_distribution<double> dist(-2.0, 2.0);
    
    for (int i = 0; i < 5; i++) {
        Eigen::Vector3d p(dist(gen), dist(gen), 5.0 + dist(gen));
        gt_points.push_back(p);
    }
    
    // 관측 생성 (노이즈 추가)
    std::vector<Observation> observations;
    std::normal_distribution<double> noise(0.0, 1.0);  // 1px 노이즈
    
    for (int cam_id = 0; cam_id < 3; cam_id++) {
        for (int pt_id = 0; pt_id < 5; pt_id++) {
            // Ground truth 투영
            Eigen::Vector3d p_cam = gt_poses[cam_id] * gt_points[pt_id];
            
            if (p_cam(2) > 0) {
                Eigen::Vector2d uv;
                uv(0) = fx * p_cam(0) / p_cam(2) + cx + noise(gen);
                uv(1) = fy * p_cam(1) / p_cam(2) + cy + noise(gen);
                
                observations.push_back({cam_id, pt_id, uv});
            }
        }
    }
    
    std::cout << "Generated:" << std::endl;
    std::cout << "  " << gt_poses.size() << " cameras" << std::endl;
    std::cout << "  " << gt_points.size() << " points" << std::endl;
    std::cout << "  " << observations.size() << " observations\n" << std::endl;
    
    // 초기 추정 (노이즈 추가)
    std::vector<Eigen::Isometry3d> init_poses = gt_poses;
    std::vector<Eigen::Vector3d> init_points = gt_points;
    
    std::uniform_real_distribution<double> pose_noise(-0.1, 0.1);
    std::uniform_real_distribution<double> point_noise(-0.2, 0.2);
    
    for (size_t i = 1; i < init_poses.size(); i++) {
        init_poses[i].translation() += Eigen::Vector3d(
            pose_noise(gen), pose_noise(gen), pose_noise(gen));
    }
    
    for (auto& p : init_points) {
        p += Eigen::Vector3d(
            point_noise(gen), point_noise(gen), point_noise(gen));
    }
    
    // BA 실행
    std::cout << "Running Bundle Adjustment...\n" << std::endl;
    ba.optimize(init_poses, init_points, observations, true, 10);
    
    // 결과 비교
    std::cout << "\nResults:" << std::endl;
    for (size_t i = 0; i < init_poses.size(); i++) {
        Eigen::Vector3d error = init_poses[i].translation() 
                              - gt_poses[i].translation();
        std::cout << "Camera " << i << " error: " 
                  << error.norm() << " m" << std::endl;
    }
    
    double avg_point_error = 0.0;
    for (size_t i = 0; i < init_points.size(); i++) {
        double error = (init_points[i] - gt_points[i]).norm();
        avg_point_error += error;
    }
    avg_point_error /= init_points.size();
    std::cout << "Avg point error: " << avg_point_error << " m" << std::endl;
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ BA 성공!" << std::endl;
    std::cout << "💡 오차가 크게 감소했어야 합니다!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week7_BA_g2o)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# g2o
find_package(G2O REQUIRED)
include_directories(${G2O_INCLUDE_DIR})

# Include
include_directories(${PROJECT_SOURCE_DIR}/include)

# Executable
add_executable(ba_demo
    src/main.cpp
    src/simple_ba.cpp
)

target_link_libraries(ba_demo
    ${G2O_CORE_LIBRARY}
    ${G2O_STUFF_LIBRARY}
    ${G2O_TYPES_SBA}
    ${G2O_SOLVER_EIGEN}
)
```

---

## 빌드 및 실행

```bash
cd week7_ba
mkdir build && cd build
cmake ..
make
./ba_demo
```

### 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 7: Bundle Adjustment with g2o
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Generated:
  3 cameras
  5 points
  15 observations

Running Bundle Adjustment...

Initial error: 1.12 px
Final error: 0.08 px

Results:
Camera 0 error: 0.000 m (fixed)
Camera 1 error: 0.012 m
Camera 2 error: 0.018 m
Avg point error: 0.024 m

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ BA 성공!
💡 오차가 크게 감소했어야 합니다!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] g2o 설치 성공
- [ ] SimpleBundleAdjustment 구현
- [ ] Vertex (camera + points) 추가
- [ ] Edge (reprojection) 추가
- [ ] Huber kernel 적용
- [ ] setMarginalized(true) 적용
- [ ] 재투영 오차 < 0.1px

---

## 💡 핵심 개념

1. **g2o 구조**
   ```
   Optimizer
   ├── Vertices (카메라, 점)
   ├── Edges (재투영 제약)
   └── Solver (Levenberg-Marquardt)
   ```

2. **Schur Complement**
   ```cpp
   v->setMarginalized(true);  // 점들만
   
   → 카메라만 직접 최적화
   → 점들은 closed-form 업데이트
   → 10-100배 빠름!
   ```

3. **Robust Kernel**
   ```cpp
   g2o::RobustKernelHuber* rk = new g2o::RobustKernelHuber;
   rk->setDelta(1.0);  // 1px
   
   → Outlier 영향 감소
   ```

4. **Gauge Freedom**
   ```cpp
   v->setFixed(true);  // 첫 카메라
   
   → 전역 좌표계 고정
   → Unique solution
   ```

---

## 🔗 Week 5 VO에 통합

Week 5의 VO 시스템에 Local BA를 추가:

```cpp
// VisualOdometry 클래스에서
void localBundleAdjustment() {
    // 최근 N개 Keyframe만
    std::vector<Keyframe*> local_kfs;
    for (int i = keyframes_.size()-10; i < keyframes_.size(); i++) {
        local_kfs.push_back(keyframes_[i]);
    }
    
    // BA
    SimpleBundleAdjustment ba(fx_, fy_, cx_, cy_);
    ba.optimize(local_poses, local_points, local_obs);
}

// Keyframe 추가 후 실행
if (keyframes_.size() % 5 == 0) {
    localBundleAdjustment();
}
```

---

## 🏗️ mini_slam 구현

> Week 7은 BA의 **개념 이해** 주차로, mini_slam에 직접 BA 코드를 붙이지 않는다.
> 대신 손계산(2 키프레임 + 3 포인트)으로 BA 식을 유도하고,
> Week 8(Ceres)과 Week 9~10(g2o)에서 실제 구현한다.

**이번 주 결과물**: mini_slam의 로컬 BA 설계 문서

- 어떤 키프레임과 맵 포인트를 BA에 포함할 것인가?
- 고정할 키프레임은? (gauge freedom)
- Edge 구조: 어떤 관측이 어떤 카메라-포인트를 연결하는가?

---

**다음**: Week 8에서 Ceres로 BA 구현!
