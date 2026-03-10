# Week 2 실습: 2D-2D 모션 추정 (Essential Matrix)

> 🎯 **목표**: Essential Matrix로 초기 포즈 추정 및 3D 맵 생성  
> 💻 **언어**: C++ (OpenCV, Eigen)  
> ⏰ **예상 시간**: 10시간

---

## 📋 실습 개요

Week 2는 **2D-2D 대응**을 이용한 모션 추정입니다. Essential Matrix를 계산하고, R과 t를 복원한 후, 삼각측량으로 초기 3D 맵을 생성합니다.

---

## 🔧 프로젝트 구조

```
week2_essential/
├── CMakeLists.txt
├── include/
│   ├── camera.hpp          # Week 1에서 재사용
│   ├── types.hpp           # Week 1에서 재사용
│   └── motion_2d2d.hpp     # 새로 추가
└── src/
    ├── motion_2d2d.cpp
    └── main.cpp
```

---

## Step 1: MotionEstimator2D2D 구현

### include/motion_2d2d.hpp

```cpp
#ifndef MOTION_2D2D_HPP
#define MOTION_2D2D_HPP

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <vector>

namespace vo {

class MotionEstimator2D2D {
public:
    MotionEstimator2D2D(const cv::Mat& K);
    
    /**
     * @brief Essential Matrix 추정 (RANSAC)
     */
    bool estimateEssential(
        const std::vector<cv::Point2f>& pts1,
        const std::vector<cv::Point2f>& pts2,
        cv::Mat& E,
        cv::Mat& inliers
    );
    
    /**
     * @brief E에서 R, t 복원
     */
    bool recoverPose(
        const cv::Mat& E,
        const std::vector<cv::Point2f>& pts1,
        const std::vector<cv::Point2f>& pts2,
        cv::Mat& R,
        cv::Mat& t
    );
    
    /**
     * @brief 삼각측량
     */
    void triangulate(
        const cv::Mat& R1, const cv::Mat& t1,
        const cv::Mat& R2, const cv::Mat& t2,
        const std::vector<cv::Point2f>& pts1,
        const std::vector<cv::Point2f>& pts2,
        std::vector<cv::Point3f>& points3d
    );
    
private:
    cv::Mat K_;
};

} // namespace vo

#endif
```

### src/motion_2d2d.cpp

```cpp
#include "motion_2d2d.hpp"
#include <iostream>

namespace vo {

MotionEstimator2D2D::MotionEstimator2D2D(const cv::Mat& K) : K_(K.clone()) {}

bool MotionEstimator2D2D::estimateEssential(
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    cv::Mat& E,
    cv::Mat& inliers
) {
    if (pts1.size() < 5) {
        std::cerr << "Need at least 5 points!" << std::endl;
        return false;
    }
    
    // 정규화 좌표로 변환
    std::vector<cv::Point2f> pts1_norm, pts2_norm;
    cv::Mat K_inv = K_.inv();
    
    for (size_t i = 0; i < pts1.size(); i++) {
        cv::Mat p1 = (cv::Mat_<double>(3,1) << pts1[i].x, pts1[i].y, 1.0);
        cv::Mat p2 = (cv::Mat_<double>(3,1) << pts2[i].x, pts2[i].y, 1.0);
        
        cv::Mat p1_norm = K_inv * p1;
        cv::Mat p2_norm = K_inv * p2;
        
        pts1_norm.push_back(cv::Point2f(p1_norm.at<double>(0), 
                                        p1_norm.at<double>(1)));
        pts2_norm.push_back(cv::Point2f(p2_norm.at<double>(0), 
                                        p2_norm.at<double>(1)));
    }
    
    // Essential Matrix 추정 (5-point + RANSAC)
    E = cv::findEssentialMat(
        pts1_norm, pts2_norm,
        cv::Mat::eye(3, 3, CV_64F),
        cv::RANSAC,
        0.999,
        0.001,
        inliers
    );
    
    if (E.empty()) {
        std::cerr << "Failed to estimate Essential Matrix!" << std::endl;
        return false;
    }
    
    int num_inliers = cv::countNonZero(inliers);
    std::cout << "  Inliers: " << num_inliers << "/" << pts1.size() 
              << " (" << (num_inliers * 100.0 / pts1.size()) << "%)" << std::endl;
    
    return true;
}

bool MotionEstimator2D2D::recoverPose(
    const cv::Mat& E,
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    cv::Mat& R,
    cv::Mat& t
) {
    // 정규화 좌표로 변환
    std::vector<cv::Point2f> pts1_norm, pts2_norm;
    cv::Mat K_inv = K_.inv();
    
    for (size_t i = 0; i < pts1.size(); i++) {
        cv::Mat p1 = (cv::Mat_<double>(3,1) << pts1[i].x, pts1[i].y, 1.0);
        cv::Mat p2 = (cv::Mat_<double>(3,1) << pts2[i].x, pts2[i].y, 1.0);
        
        cv::Mat p1_norm = K_inv * p1;
        cv::Mat p2_norm = K_inv * p2;
        
        pts1_norm.push_back(cv::Point2f(p1_norm.at<double>(0), 
                                        p1_norm.at<double>(1)));
        pts2_norm.push_back(cv::Point2f(p2_norm.at<double>(0), 
                                        p2_norm.at<double>(1)));
    }
    
    // R, t 복원 (Cheirality check 자동)
    cv::Mat inliers;
    int num_good = cv::recoverPose(
        E, pts1_norm, pts2_norm,
        cv::Mat::eye(3, 3, CV_64F),
        R, t, inliers
    );
    
    std::cout << "  Good points (depth > 0): " << num_good << std::endl;
    
    return num_good >= 30;
}

void MotionEstimator2D2D::triangulate(
    const cv::Mat& R1, const cv::Mat& t1,
    const cv::Mat& R2, const cv::Mat& t2,
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    std::vector<cv::Point3f>& points3d
) {
    // 투영 행렬
    cv::Mat P1(3, 4, CV_64F);
    R1.copyTo(P1.rowRange(0,3).colRange(0,3));
    t1.copyTo(P1.rowRange(0,3).col(3));
    P1 = K_ * P1;
    
    cv::Mat P2(3, 4, CV_64F);
    R2.copyTo(P2.rowRange(0,3).colRange(0,3));
    t2.copyTo(P2.rowRange(0,3).col(3));
    P2 = K_ * P2;
    
    // 삼각측량
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);
    
    // 동차 좌표 → 3D
    points3d.clear();
    for (int i = 0; i < points4D.cols; i++) {
        cv::Mat x = points4D.col(i);
        x /= x.at<float>(3);
        
        float depth = x.at<float>(2);
        if (depth > 0 && depth < 100) {
            points3d.push_back(cv::Point3f(
                x.at<float>(0), 
                x.at<float>(1), 
                x.at<float>(2)
            ));
        }
    }
    
    std::cout << "  Triangulated: " << points3d.size() << " points" << std::endl;
}

} // namespace vo
```

---

## Step 2: 메인 테스트

### src/main.cpp

```cpp
#include "motion_2d2d.hpp"
#include <iostream>
#include <vector>

using namespace vo;

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 2: 2D-2D 모션 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 파라미터 (KITTI)
    cv::Mat K = (cv::Mat_<double>(3,3) << 
        718.856, 0, 607.1928,
        0, 718.856, 185.2157,
        0, 0, 1);
    
    MotionEstimator2D2D estimator(K);
    
    // 임의의 대응점 생성
    std::vector<cv::Point2f> pts1, pts2;
    for (int i = 0; i < 100; i++) {
        float x = 200 + rand() % 400;
        float y = 100 + rand() % 300;
        pts1.push_back(cv::Point2f(x, y));
        pts2.push_back(cv::Point2f(x + 10, y + 2));
    }
    
    // 1. Essential Matrix
    std::cout << "1. Essential Matrix 추정" << std::endl;
    cv::Mat E, inliers;
    if (!estimator.estimateEssential(pts1, pts2, E, inliers)) {
        return -1;
    }
    std::cout << "E =\n" << E << "\n" << std::endl;
    
    // 2. R, t 복원
    std::cout << "2. 포즈 복원" << std::endl;
    cv::Mat R, t;
    if (!estimator.recoverPose(E, pts1, pts2, R, t)) {
        return -1;
    }
    std::cout << "R =\n" << R << std::endl;
    std::cout << "t = " << t.t() << " (normalized)\n" << std::endl;
    
    // 3. 삼각측량
    std::cout << "3. 초기 3D 맵 생성" << std::endl;
    cv::Mat R1 = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t1 = cv::Mat::zeros(3, 1, CV_64F);
    
    std::vector<cv::Point3f> points3d;
    estimator.triangulate(R1, t1, R, t, pts1, pts2, points3d);
    
    double avg_depth = 0.0;
    for (const auto& p : points3d) {
        avg_depth += p.z;
    }
    avg_depth /= points3d.size();
    std::cout << "  Average depth: " << avg_depth << std::endl;
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 초기화 완료!" << std::endl;
    std::cout << "⚠️  스케일 모호성: ||t|| = 1" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week2_Essential)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(essential_demo
    src/main.cpp
    src/motion_2d2d.cpp
)

target_link_libraries(essential_demo
    ${OpenCV_LIBS}
)
```

---

## 빌드 및 실행

```bash
cd week2_essential
mkdir build && cd build
cmake ..
make
./essential_demo
```

---

## ✅ 체크리스트

- [ ] Essential Matrix 추정 성공
- [ ] Inlier ratio > 70%
- [ ] R, t 복원 성공
- [ ] 삼각측량 (> 50개 점)
- [ ] **스케일 모호성 이해** ⭐

---

## 💡 핵심 개념

1. **Essential Matrix**
   - 5-point algorithm
   - RANSAC으로 outlier 제거
   
2. **Pose Recovery**
   - SVD 분해
   - 4가지 해 중 Cheirality check
   
3. **삼각측량**
   - DLT (Direct Linear Transform)
   - 깊이 > 0 체크
   
4. **스케일 모호성** ⭐
   - ||t|| = 1로 정규화
   - 절대 스케일 알 수 없음
   - Week 3 PnP로 해결!

---

## 🏗️ mini_slam 구현 (이번 주 핵심)

> 이번 주는 mini_slam의 **2D-2D 초기화 모듈**을 개선한다.
> Phase 2의 Essential Matrix 코드를 기반으로 RANSAC 파라미터를 튜닝하여 정확도를 높인다.

**작업 내용**:

| 작업 | 내용 |
|------|------|
| E Matrix RANSAC 튜닝 | threshold, confidence, iteration 수 최적화 |
| 초기화 품질 검증 | inlier ratio, 재투영 오차 기준 강화 |
| mini_slam 초기화 통합 | `mini_slam/src/` 에서 개선된 2D-2D 초기화 사용 |

**구현 파일**: `Studies/Phase 3/mini_slam/src/epipolar.cpp`

### 완성 기준

```bash
cd Studies/Phase\ 3/mini_slam/build
./mini_slam

# 2프레임 간 포즈 추정 정확도 향상 확인
# Inlier ratio > 80%, 재투영 오차 < 1px
```

---

**다음**: Week 3에서 PnP로 추적 시작!
