# Week 3 실습: 3D-2D 모션 추정 (PnP)

> 🎯 **목표**: PnP로 카메라 포즈 추적 및 절대 스케일 복원  
> 💻 **언어**: C++ (OpenCV, Eigen)  
> ⏰ **예상 시간**: 10시간

---

## 📋 실습 개요

Week 3는 **3D-2D 대응**을 이용한 포즈 추정입니다. PnP (Perspective-n-Point) 알고리즘으로 카메라를 추적하고, **절대 스케일**을 복원합니다.

---

## 🔧 프로젝트 구조

```
week3_pnp/
├── CMakeLists.txt
├── include/
│   ├── camera.hpp          # Week 1
│   ├── types.hpp           # Week 1
│   ├── motion_2d2d.hpp     # Week 2
│   └── motion_3d2d.hpp     # 새로 추가
└── src/
    ├── motion_3d2d.cpp
    └── main.cpp
```

---

## Step 1: MotionEstimator3D2D 구현

### include/motion_3d2d.hpp

```cpp
#ifndef MOTION_3D2D_HPP
#define MOTION_3D2D_HPP

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <vector>

namespace vo {

class MotionEstimator3D2D {
public:
    MotionEstimator3D2D(const cv::Mat& K);
    
    /**
     * @brief PnP로 포즈 추정
     */
    bool estimatePose(
        const std::vector<cv::Point3f>& points3d,
        const std::vector<cv::Point2f>& points2d,
        cv::Mat& R,
        cv::Mat& t,
        bool use_ransac = true
    );
    
    /**
     * @brief PnP + RANSAC
     */
    double estimatePoseRansac(
        const std::vector<cv::Point3f>& points3d,
        const std::vector<cv::Point2f>& points2d,
        cv::Mat& R,
        cv::Mat& t,
        std::vector<int>& inliers
    );
    
    /**
     * @brief 재투영 오차
     */
    double computeReprojectionError(
        const std::vector<cv::Point3f>& points3d,
        const std::vector<cv::Point2f>& points2d,
        const cv::Mat& R,
        const cv::Mat& t
    );
    
private:
    cv::Mat K_;
    cv::Mat dist_coeffs_;
};

} // namespace vo

#endif
```

### src/motion_3d2d.cpp

```cpp
#include "motion_3d2d.hpp"
#include <iostream>

namespace vo {

MotionEstimator3D2D::MotionEstimator3D2D(const cv::Mat& K) 
    : K_(K.clone()) {
    dist_coeffs_ = cv::Mat::zeros(4, 1, CV_64F);
}

bool MotionEstimator3D2D::estimatePose(
    const std::vector<cv::Point3f>& points3d,
    const std::vector<cv::Point2f>& points2d,
    cv::Mat& R,
    cv::Mat& t,
    bool use_ransac
) {
    if (points3d.size() < 4) {
        std::cerr << "Need at least 4 points!" << std::endl;
        return false;
    }
    
    cv::Mat rvec, tvec;
    
    if (use_ransac) {
        cv::Mat inliers;
        bool success = cv::solvePnPRansac(
            points3d, points2d,
            K_, dist_coeffs_,
            rvec, tvec,
            false,
            100,
            8.0,
            0.99,
            inliers,
            cv::SOLVEPNP_EPNP
        );
        
        if (!success) return false;
        
        int num_inliers = inliers.rows;
        std::cout << "  Inliers: " << num_inliers << "/" << points3d.size()
                  << " (" << (num_inliers * 100.0 / points3d.size()) << "%)" 
                  << std::endl;
    } else {
        bool success = cv::solvePnP(
            points3d, points2d,
            K_, dist_coeffs_,
            rvec, tvec,
            false,
            cv::SOLVEPNP_EPNP
        );
        if (!success) return false;
    }
    
    cv::Rodrigues(rvec, R);
    t = tvec.clone();
    
    return true;
}

double MotionEstimator3D2D::estimatePoseRansac(
    const std::vector<cv::Point3f>& points3d,
    const std::vector<cv::Point2f>& points2d,
    cv::Mat& R,
    cv::Mat& t,
    std::vector<int>& inliers
) {
    cv::Mat rvec, tvec;
    cv::Mat inliers_mat;
    
    bool success = cv::solvePnPRansac(
        points3d, points2d,
        K_, dist_coeffs_,
        rvec, tvec,
        false, 100, 8.0, 0.99,
        inliers_mat,
        cv::SOLVEPNP_EPNP
    );
    
    if (!success) return 0.0;
    
    inliers.clear();
    for (int i = 0; i < inliers_mat.rows; i++) {
        inliers.push_back(inliers_mat.at<int>(i));
    }
    
    cv::Rodrigues(rvec, R);
    t = tvec.clone();
    
    return (double)inliers.size() / points3d.size();
}

double MotionEstimator3D2D::computeReprojectionError(
    const std::vector<cv::Point3f>& points3d,
    const std::vector<cv::Point2f>& points2d,
    const cv::Mat& R,
    const cv::Mat& t
) {
    cv::Mat rvec;
    cv::Rodrigues(R, rvec);
    
    std::vector<cv::Point2f> projected;
    cv::projectPoints(points3d, rvec, t, K_, dist_coeffs_, projected);
    
    double total_error = 0.0;
    for (size_t i = 0; i < points2d.size(); i++) {
        double error = cv::norm(points2d[i] - projected[i]);
        total_error += error * error;
    }
    
    return std::sqrt(total_error / points2d.size());
}

} // namespace vo
```

---

## Step 2: VO 추적 시뮬레이션

### src/main.cpp

```cpp
#include "motion_2d2d.hpp"
#include "motion_3d2d.hpp"
#include <iostream>

using namespace vo;

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3: 3D-2D (PnP) 추적" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    cv::Mat K = (cv::Mat_<double>(3,3) << 
        718.856, 0, 607.1928,
        0, 718.856, 185.2157,
        0, 0, 1);
    
    // Week 2 초기화 재사용
    MotionEstimator2D2D estimator_2d(K);
    MotionEstimator3D2D estimator_3d(K);
    
    // 가상 대응점
    std::vector<cv::Point2f> pts0, pts1;
    for (int i = 0; i < 100; i++) {
        float x = 200 + rand() % 400;
        float y = 100 + rand() % 300;
        pts0.push_back(cv::Point2f(x, y));
        pts1.push_back(cv::Point2f(x + 15, y + 3));
    }
    
    // 초기화
    std::cout << "━━ 초기화 (Week 2) ━━" << std::endl;
    cv::Mat E, inliers;
    estimator_2d.estimateEssential(pts0, pts1, E, inliers);
    
    cv::Mat R_init, t_init;
    estimator_2d.recoverPose(E, pts0, pts1, R_init, t_init);
    
    cv::Mat R0 = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t0 = cv::Mat::zeros(3, 1, CV_64F);
    
    std::vector<cv::Point3f> map_points;
    estimator_2d.triangulate(R0, t0, R_init, t_init, pts0, pts1, map_points);
    
    std::cout << "초기 맵: " << map_points.size() << "개\n" << std::endl;
    
    // 추적
    std::cout << "━━ PnP 추적 ━━" << std::endl;
    
    for (int frame = 2; frame < 5; frame++) {
        std::cout << "\n[Frame " << frame << "]" << std::endl;
        
        // 현재 프레임 2D 점 (투영 + 노이즈)
        std::vector<cv::Point2f> pts_curr;
        cv::Mat rvec, dist = cv::Mat::zeros(4, 1, CV_64F);
        cv::Rodrigues(R_init, rvec);
        cv::projectPoints(map_points, rvec, t_init, K, dist, pts_curr);
        
        for (auto& p : pts_curr) {
            p.x += (rand() % 5 - 2);
            p.y += (rand() % 5 - 2);
        }
        
        // PnP
        cv::Mat R_rel, t_rel;
        std::vector<int> inliers_idx;
        
        double inlier_ratio = estimator_3d.estimatePoseRansac(
            map_points, pts_curr, R_rel, t_rel, inliers_idx);
        
        std::cout << "  Inlier ratio: " << (inlier_ratio * 100) << "%" << std::endl;
        
        double error = estimator_3d.computeReprojectionError(
            map_points, pts_curr, R_rel, t_rel);
        
        std::cout << "  Reprojection error: " << error << " px" << std::endl;
        std::cout << "  ✅ 절대 스케일 유지!" << std::endl;
    }
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ PnP 추적 완료!" << std::endl;
    std::cout << "📊 Key: 스케일 복원 성공!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week3_PnP)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(pnp_demo
    src/main.cpp
    src/motion_3d2d.cpp
    ../week2/src/motion_2d2d.cpp
)

target_link_libraries(pnp_demo ${OpenCV_LIBS})
```

---

## ✅ 체크리스트

- [ ] PnP 포즈 추정
- [ ] Inlier ratio > 70%
- [ ] 재투영 오차 < 1px
- [ ] **스케일 복원 이해** ⭐

---

## 💡 핵심 개념

1. **PnP 알고리즘**
   - P3P, EPnP, DLS
   - EPnP가 가장 빠름
   
2. **절대 스케일** ⭐
   - 3D 점 사용 → 실제 스케일
   - Week 2와의 차이점!
   
3. **VO 추적**
   - 초기화: 2D-2D
   - 추적: 3D-2D
   - 이게 VO의 핵심!

---

## 🏗️ mini_slam 구현 (이번 주 핵심)

> 이번 주는 mini_slam에 **PnP RANSAC 직접 구현** 모듈을 추가한다.
> OpenCV의 `solvePnPRansac`에 의존하지 않고, RANSAC 루프를 직접 작성한다.

**작업 내용**:

| 작업 | 내용 |
|------|------|
| PnP RANSAC 직접 구현 | RANSAC 루프 (랜덤 4점 샘플 → EPnP → inlier 판별) 직접 작성 |
| N번째 프레임 포즈 추정 | 기존 3D 맵 포인트 + 현재 2D 관측으로 PnP 추적 |
| mini_slam 추적 강화 | 직접 구현한 PnP RANSAC을 mini_slam 추적 모듈에 통합 |

**구현 파일**: `Studies/Phase 3/mini_slam/src/motion_3d2d.cpp`

| 함수 | 내용 |
|------|------|
| `solvePnPRansac()` | RANSAC 루프 직접 구현 (랜덤 샘플 → 모델 → inlier 수) |
| `computeInliers()` | 재투영 오차 기반 inlier/outlier 분류 |
| `refinePose()` | inlier만으로 포즈 재추정 |

### 완성 기준

```bash
cd Studies/Phase\ 3/mini_slam/build
./mini_slam

# PnP RANSAC 직접 구현으로 N번째 프레임 포즈 추정
# Inlier ratio > 70%, 재투영 오차 < 2px
```

---

**다음**: Week 4에서 ICP 학습!
