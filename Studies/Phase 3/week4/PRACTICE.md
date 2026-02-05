# Week 4 실습: 3D-3D 모션 추정 (ICP)

> 🎯 **목표**: ICP 알고리즘 이해 및 SVD 구현  
> 💻 **언어**: C++ (Eigen)  
> ⏰ **예상 시간**: 10시간

---

## 📋 실습 개요

Week 4는 **3D-3D 대응**을 이용한 정합입니다. ICP (Iterative Closest Point)를 구현하여 두 점군 간의 변환을 추정합니다. **주의**: ICP는 VIO에서는 거의 사용되지 않습니다 (RGB-D, LiDAR SLAM용).

---

## 🔧 프로젝트 구조

```
week4_icp/
├── CMakeLists.txt
├── include/
│   └── icp.hpp
└── src/
    ├── icp.cpp
    └── main.cpp
```

---

## Step 1: ICP 구현

### include/icp.hpp

```cpp
#ifndef ICP_HPP
#define ICP_HPP

#include <Eigen/Dense>
#include <vector>

namespace vo {

class ICP {
public:
    ICP(int max_iterations = 50, double tolerance = 1e-6);
    
    /**
     * @brief ICP 실행
     * @return 최종 오차
     */
    double align(
        const std::vector<Eigen::Vector3d>& source,
        const std::vector<Eigen::Vector3d>& target,
        Eigen::Matrix3d& R,
        Eigen::Vector3d& t
    );
    
private:
    int max_iterations_;
    double tolerance_;
    
    void computeTransform(
        const std::vector<Eigen::Vector3d>& source,
        const std::vector<Eigen::Vector3d>& target,
        Eigen::Matrix3d& R,
        Eigen::Vector3d& t
    );
    
    double computeError(
        const std::vector<Eigen::Vector3d>& source,
        const std::vector<Eigen::Vector3d>& target,
        const Eigen::Matrix3d& R,
        const Eigen::Vector3d& t
    );
};

} // namespace vo

#endif
```

### src/icp.cpp

```cpp
#include "icp.hpp"
#include <iostream>

namespace vo {

ICP::ICP(int max_iterations, double tolerance)
    : max_iterations_(max_iterations), tolerance_(tolerance) {}

double ICP::align(
    const std::vector<Eigen::Vector3d>& source,
    const std::vector<Eigen::Vector3d>& target,
    Eigen::Matrix3d& R,
    Eigen::Vector3d& t
) {
    if (source.size() != target.size()) {
        std::cerr << "Size mismatch!" << std::endl;
        return -1.0;
    }
    
    R = Eigen::Matrix3d::Identity();
    t = Eigen::Vector3d::Zero();
    
    double prev_error = 1e10;
    
    for (int iter = 0; iter < max_iterations_; iter++) {
        // 1. 변환
        std::vector<Eigen::Vector3d> source_transformed;
        for (const auto& p : source) {
            source_transformed.push_back(R * p + t);
        }
        
        // 2. SVD로 변환 계산
        Eigen::Matrix3d R_iter;
        Eigen::Vector3d t_iter;
        computeTransform(source_transformed, target, R_iter, t_iter);
        
        // 3. 누적 업데이트
        t = R_iter * t + t_iter;
        R = R_iter * R;
        
        // 4. 오차
        double error = computeError(source, target, R, t);
        
        if (iter % 10 == 0) {
            std::cout << "Iteration " << iter << ": Error = " 
                      << error << std::endl;
        }
        
        // 5. 수렴
        if (std::abs(prev_error - error) < tolerance_) {
            std::cout << "Converged at iteration " << iter << std::endl;
            break;
        }
        
        prev_error = error;
    }
    
    return prev_error;
}

void ICP::computeTransform(
    const std::vector<Eigen::Vector3d>& source,
    const std::vector<Eigen::Vector3d>& target,
    Eigen::Matrix3d& R,
    Eigen::Vector3d& t
) {
    // 중심
    Eigen::Vector3d centroid_src = Eigen::Vector3d::Zero();
    Eigen::Vector3d centroid_tgt = Eigen::Vector3d::Zero();
    
    for (size_t i = 0; i < source.size(); i++) {
        centroid_src += source[i];
        centroid_tgt += target[i];
    }
    
    centroid_src /= source.size();
    centroid_tgt /= target.size();
    
    // 중심 제거
    std::vector<Eigen::Vector3d> src_centered, tgt_centered;
    for (size_t i = 0; i < source.size(); i++) {
        src_centered.push_back(source[i] - centroid_src);
        tgt_centered.push_back(target[i] - centroid_tgt);
    }
    
    // Covariance H
    Eigen::Matrix3d H = Eigen::Matrix3d::Zero();
    for (size_t i = 0; i < source.size(); i++) {
        H += src_centered[i] * tgt_centered[i].transpose();
    }
    
    // SVD: H = U * Σ * V^T
    Eigen::JacobiSVD<Eigen::Matrix3d> svd(
        H, Eigen::ComputeFullU | Eigen::ComputeFullV);
    
    Eigen::Matrix3d U = svd.matrixU();
    Eigen::Matrix3d V = svd.matrixV();
    
    // R = V * U^T
    R = V * U.transpose();
    
    // Reflection 체크
    if (R.determinant() < 0) {
        V.col(2) *= -1;
        R = V * U.transpose();
    }
    
    // t
    t = centroid_tgt - R * centroid_src;
}

double ICP::computeError(
    const std::vector<Eigen::Vector3d>& source,
    const std::vector<Eigen::Vector3d>& target,
    const Eigen::Matrix3d& R,
    const Eigen::Vector3d& t
) {
    double error = 0.0;
    
    for (size_t i = 0; i < source.size(); i++) {
        Eigen::Vector3d transformed = R * source[i] + t;
        error += (transformed - target[i]).squaredNorm();
    }
    
    return std::sqrt(error / source.size());
}

} // namespace vo
```

---

## Step 2: 테스트

### src/main.cpp

```cpp
#include "icp.hpp"
#include <iostream>
#include <random>

using namespace vo;

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 4: ICP 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // Ground truth
    Eigen::Matrix3d R_gt;
    R_gt = Eigen::AngleAxisd(0.3, Eigen::Vector3d::UnitZ());
    Eigen::Vector3d t_gt(1.0, 0.5, 0.2);
    
    std::cout << "Ground Truth:" << std::endl;
    std::cout << "R =\n" << R_gt << std::endl;
    std::cout << "t = " << t_gt.transpose() << "\n" << std::endl;
    
    // 소스 점군
    std::vector<Eigen::Vector3d> source;
    std::default_random_engine gen;
    std::uniform_real_distribution<double> dist(-5.0, 5.0);
    
    for (int i = 0; i < 100; i++) {
        source.push_back(Eigen::Vector3d(dist(gen), dist(gen), dist(gen)));
    }
    
    // 타겟 점군
    std::vector<Eigen::Vector3d> target;
    std::normal_distribution<double> noise(0.0, 0.01);
    
    for (const auto& p : source) {
        Eigen::Vector3d p_transformed = R_gt * p + t_gt;
        p_transformed(0) += noise(gen);
        p_transformed(1) += noise(gen);
        p_transformed(2) += noise(gen);
        target.push_back(p_transformed);
    }
    
    // ICP 실행
    std::cout << "Running ICP...\n" << std::endl;
    
    ICP icp(50, 1e-6);
    Eigen::Matrix3d R_est;
    Eigen::Vector3d t_est;
    
    double final_error = icp.align(source, target, R_est, t_est);
    
    std::cout << "\n결과:" << std::endl;
    std::cout << "R_estimated =\n" << R_est << std::endl;
    std::cout << "t_estimated = " << t_est.transpose() << std::endl;
    std::cout << "Final error: " << final_error << std::endl;
    
    // 오차
    Eigen::Matrix3d R_error = R_gt.transpose() * R_est;
    Eigen::Vector3d t_error = t_est - t_gt;
    
    std::cout << "\n오차:" << std::endl;
    std::cout << "R error:\n" << R_error << std::endl;
    std::cout << "t error: " << t_error.transpose() << std::endl;
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ ICP 완료!" << std::endl;
    std::cout << "💡 VIO X, RGB-D/LiDAR O" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week4_ICP)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(icp_demo
    src/main.cpp
    src/icp.cpp
)
```

---

## ✅ 체크리스트

- [ ] ICP SVD 구현
- [ ] 반복 최적화
- [ ] 수렴 (error < 1e-6)
- [ ] Ground truth 비교
- [ ] ICP 한계 이해

---

## 💡 핵심 개념

1. **ICP 단계**
   - 대응 찾기
   - SVD 계산
   - 변환 적용
   - 수렴 체크
   
2. **SVD 정합**
   - H = Σ p'ᵢ qᵢᵀ
   - H = UΣVᵀ
   - R = VUᵀ
   
3. **한계**
   - 초기값 민감
   - Local minimum
   - VIO에서 거의 안 씀!

---

**다음**: Week 5에서 완전한 VO 통합!
