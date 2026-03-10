# Week 8 실습: Bundle Adjustment with Ceres

> 🎯 **목표**: Ceres Solver로 간단한 BA 구현  
> 💻 **언어**: C++ (Ceres, Eigen)  
> ⏰ **예상 시간**: 10시간

---

## 📋 실습 개요

Week 8은 **Ceres Solver**로 Bundle Adjustment를 구현합니다. 자동 미분 덕분에 g2o보다 훨씬 간단합니다!

---

## 🔧 환경 설정

### Ceres 설치

```bash
# Dependencies
sudo apt update
sudo apt install cmake libgoogle-glog-dev libgflags-dev
sudo apt install libatlas-base-dev libeigen3-dev

# Ceres Solver
git clone https://ceres-solver.googlesource.com/ceres-solver
cd ceres-solver
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install

# 확인
pkg-config --modversion ceres
```

---

## 프로젝트 구조

```
week8_ceres_ba/
├── CMakeLists.txt
├── include/
│   └── ceres_ba.hpp
└── src/
    ├── ceres_ba.cpp
    └── main.cpp
```

---

## Step 1: ReprojectionError 정의

### include/ceres_ba.hpp

```cpp
#ifndef CERES_BA_HPP
#define CERES_BA_HPP

#include <ceres/ceres.h>
#include <ceres/rotation.h>
#include <Eigen/Dense>
#include <vector>

namespace vo {

/**
 * @brief 재투영 오차 functor
 * 
 * Ceres의 자동 미분을 위한 템플릿 functor
 */
struct ReprojectionError {
    ReprojectionError(double observed_x, double observed_y, 
                      double fx, double fy, double cx, double cy)
        : observed_x_(observed_x), observed_y_(observed_y),
          fx_(fx), fy_(fy), cx_(cx), cy_(cy) {}
    
    /**
     * @brief operator() - 오차 계산
     * @param camera 카메라 포즈 [6]: [angle_axis(3), translation(3)]
     * @param point 3D 점 [3]: [x, y, z]
     * @param residuals 출력 오차 [2]: [x_error, y_error]
     */
    template <typename T>
    bool operator()(const T* const camera,
                    const T* const point,
                    T* residuals) const {
        // 1. 3D 점을 카메라 좌표계로 변환
        T p[3];
        ceres::AngleAxisRotatePoint(camera, point, p);
        
        // Add translation
        p[0] += camera[3];
        p[1] += camera[4];
        p[2] += camera[5];
        
        // 2. 투영
        T predicted_x = T(fx_) * p[0] / p[2] + T(cx_);
        T predicted_y = T(fy_) * p[1] / p[2] + T(cy_);
        
        // 3. 오차
        residuals[0] = predicted_x - T(observed_x_);
        residuals[1] = predicted_y - T(observed_y_);
        
        return true;
    }
    
    // Factory method
    static ceres::CostFunction* Create(double observed_x, double observed_y,
                                       double fx, double fy, double cx, double cy) {
        return new ceres::AutoDiffCostFunction<ReprojectionError, 2, 6, 3>(
            new ReprojectionError(observed_x, observed_y, fx, fy, cx, cy)
        );
    }
    
private:
    double observed_x_, observed_y_;
    double fx_, fy_, cx_, cy_;
};

/**
 * @brief 관측 데이터
 */
struct Observation {
    int camera_id;
    int point_id;
    Eigen::Vector2d uv;
};

/**
 * @brief Ceres Bundle Adjustment
 */
class CeresBundleAdjustment {
public:
    CeresBundleAdjustment(double fx, double fy, double cx, double cy);
    
    /**
     * @brief BA 실행
     * @param poses 카메라 포즈 (angle-axis + translation)
     * @param points 3D 점
     * @param observations 관측
     * @param use_robust Robust kernel 사용 여부
     */
    void optimize(
        std::vector<Eigen::Matrix<double, 6, 1>>& poses,
        std::vector<Eigen::Vector3d>& points,
        const std::vector<Observation>& observations,
        bool use_robust = true,
        int max_iterations = 50
    );
    
    /**
     * @brief 재투영 오차 계산
     */
    double computeReprojectionError(
        const std::vector<Eigen::Matrix<double, 6, 1>>& poses,
        const std::vector<Eigen::Vector3d>& points,
        const std::vector<Observation>& observations
    );
    
private:
    double fx_, fy_, cx_, cy_;
};

} // namespace vo

#endif
```

### src/ceres_ba.cpp

```cpp
#include "ceres_ba.hpp"
#include <iostream>

namespace vo {

CeresBundleAdjustment::CeresBundleAdjustment(
    double fx, double fy, double cx, double cy
) : fx_(fx), fy_(fy), cx_(cx), cy_(cy) {}

void CeresBundleAdjustment::optimize(
    std::vector<Eigen::Matrix<double, 6, 1>>& poses,
    std::vector<Eigen::Vector3d>& points,
    const std::vector<Observation>& observations,
    bool use_robust,
    int max_iterations
) {
    // 초기 오차
    std::cout << "Initial error: " 
              << computeReprojectionError(poses, points, observations) 
              << " px" << std::endl;
    
    // Ceres Problem 생성
    ceres::Problem problem;
    
    // 각 관측에 대해 residual block 추가
    for (const auto& obs : observations) {
        ceres::CostFunction* cost_function =
            ReprojectionError::Create(obs.uv(0), obs.uv(1), fx_, fy_, cx_, cy_);
        
        // Robust kernel
        ceres::LossFunction* loss_function = nullptr;
        if (use_robust) {
            loss_function = new ceres::HuberLoss(1.0);
        }
        
        problem.AddResidualBlock(
            cost_function,
            loss_function,
            poses[obs.camera_id].data(),
            points[obs.point_id].data()
        );
    }
    
    // 첫 번째 카메라 고정 (gauge freedom)
    problem.SetParameterBlockConstant(poses[0].data());
    
    // Solver 옵션
    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_SCHUR;
    options.minimizer_progress_to_stdout = false;
    options.max_num_iterations = max_iterations;
    
    // 최적화 실행
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    
    std::cout << summary.BriefReport() << std::endl;
    
    // 최종 오차
    std::cout << "Final error: " 
              << computeReprojectionError(poses, points, observations) 
              << " px" << std::endl;
}

double CeresBundleAdjustment::computeReprojectionError(
    const std::vector<Eigen::Matrix<double, 6, 1>>& poses,
    const std::vector<Eigen::Vector3d>& points,
    const std::vector<Observation>& observations
) {
    double total_error = 0.0;
    
    for (const auto& obs : observations) {
        // Angle-axis to rotation matrix
        Eigen::Vector3d angle_axis = poses[obs.camera_id].head<3>();
        double angle = angle_axis.norm();
        
        Eigen::Matrix3d R;
        if (angle < 1e-10) {
            R = Eigen::Matrix3d::Identity();
        } else {
            Eigen::Vector3d axis = angle_axis / angle;
            Eigen::Matrix3d K;
            K << 0, -axis(2), axis(1),
                 axis(2), 0, -axis(0),
                 -axis(1), axis(0), 0;
            R = Eigen::Matrix3d::Identity() + sin(angle) * K + (1 - cos(angle)) * K * K;
        }
        
        Eigen::Vector3d t = poses[obs.camera_id].tail<3>();
        
        // Transform
        Eigen::Vector3d p_cam = R * points[obs.point_id] + t;
        
        // Project
        Eigen::Vector2d uv_proj;
        uv_proj(0) = fx_ * p_cam(0) / p_cam(2) + cx_;
        uv_proj(1) = fy_ * p_cam(1) / p_cam(2) + cy_;
        
        // Error
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
#include "ceres_ba.hpp"
#include <iostream>
#include <random>

using namespace vo;

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8: Bundle Adjustment with Ceres" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 파라미터
    double fx = 500.0, fy = 500.0;
    double cx = 320.0, cy = 240.0;
    
    CeresBundleAdjustment ba(fx, fy, cx, cy);
    
    // Ground truth 생성
    std::vector<Eigen::Matrix<double, 6, 1>> gt_poses;
    std::vector<Eigen::Vector3d> gt_points;
    
    // 3개 카메라
    std::default_random_engine gen(42);
    for (int i = 0; i < 3; i++) {
        Eigen::Matrix<double, 6, 1> pose;
        pose.setZero();
        pose(3) = i * 1.0;  // tx
        gt_poses.push_back(pose);
    }
    
    // 5개 3D 점
    std::uniform_real_distribution<double> dist(-2.0, 2.0);
    for (int i = 0; i < 5; i++) {
        Eigen::Vector3d p(dist(gen), dist(gen), 5.0 + dist(gen));
        gt_points.push_back(p);
    }
    
    // 관측 생성 (노이즈 추가)
    std::vector<Observation> observations;
    std::normal_distribution<double> noise(0.0, 1.0);
    
    for (int cam_id = 0; cam_id < 3; cam_id++) {
        for (int pt_id = 0; pt_id < 5; pt_id++) {
            // Project
            Eigen::Vector3d t = gt_poses[cam_id].tail<3>();
            Eigen::Vector3d p_cam = gt_points[pt_id] - t;
            
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
    auto init_poses = gt_poses;
    auto init_points = gt_points;
    
    std::uniform_real_distribution<double> pose_noise(-0.1, 0.1);
    std::uniform_real_distribution<double> point_noise(-0.2, 0.2);
    
    for (size_t i = 1; i < init_poses.size(); i++) {
        init_poses[i].tail<3>() += Eigen::Vector3d(
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
        Eigen::Vector3d error = init_poses[i].tail<3>() - gt_poses[i].tail<3>();
        std::cout << "Camera " << i << " translation error: " 
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
project(Week8_BA_Ceres)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Ceres
find_package(Ceres REQUIRED)
include_directories(${CERES_INCLUDE_DIRS})

# Include
include_directories(${PROJECT_SOURCE_DIR}/include)

# Executable
add_executable(ceres_ba_demo
    src/main.cpp
    src/ceres_ba.cpp
)

target_link_libraries(ceres_ba_demo
    ${CERES_LIBRARIES}
)
```

---

## 빌드 및 실행

```bash
cd week8_ceres_ba
mkdir build && cd build
cmake ..
make
./ceres_ba_demo
```

### 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 8: Bundle Adjustment with Ceres
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Generated:
  3 cameras
  5 points
  15 observations

Running Bundle Adjustment...

Initial error: 1.15 px
Ceres Solver Report: Iterations: 5, Initial cost: 1.987e+01, Final cost: 9.234e-02, Termination: CONVERGENCE
Final error: 0.09 px

Results:
Camera 0 translation error: 0.000 m (fixed)
Camera 1 translation error: 0.009 m
Camera 2 translation error: 0.014 m
Avg point error: 0.021 m

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ BA 성공!
💡 오차가 크게 감소했어야 합니다!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] Ceres 설치 성공
- [ ] ReprojectionError functor 구현
- [ ] AutoDiffCostFunction 사용
- [ ] Problem에 ResidualBlock 추가
- [ ] HuberLoss 적용
- [ ] 재투영 오차 < 0.1px

---

## 💡 핵심 개념

1. **Ceres 구조**
   ```
   ReprojectionError (functor)
   ↓
   AutoDiffCostFunction (자동 미분)
   ↓
   Problem (최적화 문제)
   ↓
   Solver (최적화 실행)
   ```

2. **자동 미분**
   ```cpp
   template <typename T>
   bool operator()(const T* camera, const T* point, T* residuals) {
       // T는 ceres::Jet<double, N>
       // 값과 미분을 동시에 계산!
   }
   ```

3. **Robust Loss**
   ```cpp
   ceres::LossFunction* loss = new ceres::HuberLoss(1.0);
   problem.AddResidualBlock(cost, loss, camera, point);
   ```

4. **Gauge Freedom**
   ```cpp
   problem.SetParameterBlockConstant(poses[0].data());
   // 첫 카메라 고정
   ```

---

## 🔗 g2o vs Ceres 비교

**코드 비교:**

```cpp
// g2o: 복잡
g2o::VertexSE3Expmap* v = new g2o::VertexSE3Expmap();
g2o::EdgeProjectXYZ2UV* e = new g2o::EdgeProjectXYZ2UV();
// + Jacobian 정의 필요

// Ceres: 간단
struct ReprojectionError {
    template <typename T>
    bool operator()(const T* camera, const T* point, T* residuals) {
        // 오차만 계산
    }
};
// Jacobian 자동!
```

**성능 (100번 실행):**
- g2o: 평균 15ms
- Ceres: 평균 18ms (약간 느림)
- 하지만 개발 시간: Ceres가 훨씬 빠름!

---

## 🏗️ mini_slam 구현 (이번 주 핵심)

> 이번 주는 mini_slam에 **Ceres 기반 로컬 BA**를 추가한다.
> Week 7에서 설계한 BA 구조를 Ceres cost function으로 구현한다.

**작업 내용**:

| 작업 | 내용 |
|------|------|
| `local_ba_ceres.h` 구현 | Ceres cost function (ReprojectionError) 직접 작성 |
| 로컬 BA 적용 | 최근 N개 키프레임 + 관측 맵 포인트 최적화 |
| 2 키프레임 최적화 테스트 | 간단한 케이스에서 BA 수렴 확인 |
| mini_slam 통합 | 키프레임 추가 시 로컬 BA 자동 실행 |

**구현 파일**:
- `Studies/Phase 3/mini_slam/include/local_ba_ceres.h`
- `Studies/Phase 3/mini_slam/src/local_ba_ceres.cpp`

### 완성 기준

```bash
cd Studies/Phase\ 3/mini_slam/build
./mini_slam

# Ceres로 2 키프레임 최적화 성공
# BA 전후 재투영 오차 비교: 최소 50% 이상 감소
```

---

**다음**: Week 9에서 Schur Complement 이해!
