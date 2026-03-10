# Week 1 실습: VO 기본 타입 구현 (C++)

> 🎯 **목표**: Pose, Camera 등 VO의 기본 타입 구현  
> 💻 **언어**: C++ (Eigen)  
> ⏰ **예상 시간**: 10시간

---

## 📋 실습 개요

Week 1은 **VO의 기초 타입**을 C++로 구현합니다. SE(3) 포즈, Pinhole 카메라 모델 등 SLAM에서 사용되는 핵심 데이터 구조를 만듭니다.

### 환경 설정

```bash
# Ubuntu/Jetson
sudo apt update
sudo apt install build-essential cmake git

# Eigen3
sudo apt install libeigen3-dev

# OpenCV (선택)
sudo apt install libopencv-dev
```

---

## 🔧 프로젝트 구조

```
week1_vo_basics/
├── CMakeLists.txt
├── include/
│   ├── camera.hpp
│   └── types.hpp
└── src/
    ├── types.cpp
    └── main.cpp
```

---

## Step 1: SE(3) Pose 클래스

### include/types.hpp

```cpp
#ifndef VO_TYPES_HPP
#define VO_TYPES_HPP

#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace vo {

/**
 * @brief SE(3) 포즈 (회전 + 이동)
 */
class Pose {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    Pose() : R_(Eigen::Matrix3d::Identity()), 
             t_(Eigen::Vector3d::Zero()) {}
    
    Pose(const Eigen::Matrix3d& R, const Eigen::Vector3d& t) 
        : R_(R), t_(t) {}
    
    // SE(3) 행렬로 변환
    Eigen::Matrix4d toMatrix() const {
        Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
        T.block<3,3>(0,0) = R_;
        T.block<3,1>(0,3) = t_;
        return T;
    }
    
    // 역변환
    Pose inverse() const {
        return Pose(R_.transpose(), -R_.transpose() * t_);
    }
    
    // 포즈 합성
    Pose operator*(const Pose& other) const {
        return Pose(R_ * other.R_, R_ * other.t_ + t_);
    }
    
    // 점 변환
    Eigen::Vector3d transformPoint(const Eigen::Vector3d& p) const {
        return R_ * p + t_;
    }
    
    const Eigen::Matrix3d& R() const { return R_; }
    const Eigen::Vector3d& t() const { return t_; }
    
    Eigen::Matrix3d& R() { return R_; }
    Eigen::Vector3d& t() { return t_; }
    
private:
    Eigen::Matrix3d R_;  // 회전
    Eigen::Vector3d t_;  // 이동
};

/**
 * @brief 3D 맵 포인트
 */
struct MapPoint {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    Eigen::Vector3d position;  // 3D 위치
    int num_observations;      // 관측 횟수
    bool is_outlier;           // Outlier 여부
    
    MapPoint() : position(Eigen::Vector3d::Zero()),
                 num_observations(0),
                 is_outlier(false) {}
    
    MapPoint(const Eigen::Vector3d& p) 
        : position(p), num_observations(1), is_outlier(false) {}
};

} // namespace vo

#endif
```

### src/types.cpp

```cpp
#include "types.hpp"
#include <iostream>

using namespace vo;

// Pose 테스트 함수
void testPoseOperations() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Pose 연산 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // Pose 생성
    Eigen::Matrix3d R1 = Eigen::Matrix3d::Identity();
    Eigen::Vector3d t1(1.0, 0.0, 0.0);
    Pose pose1(R1, t1);
    
    std::cout << "Pose 1:" << std::endl;
    std::cout << pose1.toMatrix() << "\n" << std::endl;
    
    // 역변환 테스트
    Pose pose1_inv = pose1.inverse();
    Pose identity = pose1 * pose1_inv;
    
    std::cout << "Pose1 * Pose1_inv (should be I):" << std::endl;
    std::cout << identity.toMatrix() << "\n" << std::endl;
    
    // 점 변환
    Eigen::Vector3d p(0.0, 0.0, 1.0);
    Eigen::Vector3d p_transformed = pose1.transformPoint(p);
    
    std::cout << "Original point: " << p.transpose() << std::endl;
    std::cout << "Transformed: " << p_transformed.transpose() << "\n" << std::endl;
    
    // 포즈 합성
    Eigen::Matrix3d R2;
    R2 = Eigen::AngleAxisd(M_PI/4, Eigen::Vector3d::UnitZ());
    Eigen::Vector3d t2(0.0, 1.0, 0.0);
    Pose pose2(R2, t2);
    
    Pose pose_combined = pose1 * pose2;
    std::cout << "Pose1 * Pose2:" << std::endl;
    std::cout << pose_combined.toMatrix() << std::endl;
}

void testMapPoint() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "MapPoint 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    MapPoint mp1;
    mp1.position = Eigen::Vector3d(1.0, 2.0, 5.0);
    mp1.num_observations = 3;
    
    std::cout << "MapPoint position: " << mp1.position.transpose() << std::endl;
    std::cout << "Observations: " << mp1.num_observations << std::endl;
    std::cout << "Is outlier: " << (mp1.is_outlier ? "Yes" : "No") << std::endl;
}
```

---

## Step 2: Pinhole Camera 모델

### include/camera.hpp

```cpp
#ifndef VO_CAMERA_HPP
#define VO_CAMERA_HPP

#include <Eigen/Dense>
#include <opencv2/opencv.hpp>

namespace vo {

/**
 * @brief Pinhole 카메라 모델
 */
class PinholeCamera {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    PinholeCamera(double fx, double fy, double cx, double cy)
        : fx_(fx), fy_(fy), cx_(cx), cy_(cy) {
        K_ = (Eigen::Matrix3d() << fx, 0, cx,
                                    0, fy, cy,
                                    0, 0, 1).finished();
    }
    
    // OpenCV Mat에서 생성
    PinholeCamera(const cv::Mat& K_cv) {
        fx_ = K_cv.at<double>(0,0);
        fy_ = K_cv.at<double>(1,1);
        cx_ = K_cv.at<double>(0,2);
        cy_ = K_cv.at<double>(1,2);
        
        K_ = (Eigen::Matrix3d() << fx_, 0, cx_,
                                    0, fy_, cy_,
                                    0, 0, 1).finished();
    }
    
    // 3D 점 → 2D 투영
    Eigen::Vector2d project(const Eigen::Vector3d& p3d) const {
        Eigen::Vector2d p2d;
        p2d(0) = fx_ * p3d(0) / p3d(2) + cx_;
        p2d(1) = fy_ * p3d(1) / p3d(2) + cy_;
        return p2d;
    }
    
    // 2D 점 → 정규화 좌표 (depth = 1)
    Eigen::Vector3d normalize(const Eigen::Vector2d& p2d) const {
        Eigen::Vector3d p_norm;
        p_norm(0) = (p2d(0) - cx_) / fx_;
        p_norm(1) = (p2d(1) - cy_) / fy_;
        p_norm(2) = 1.0;
        return p_norm;
    }
    
    // 역투영 (depth 지정)
    Eigen::Vector3d backproject(const Eigen::Vector2d& p2d, double depth) const {
        Eigen::Vector3d p_norm = normalize(p2d);
        return p_norm * depth;
    }
    
    // OpenCV Mat으로 변환
    cv::Mat toOpenCV() const {
        cv::Mat K_cv = (cv::Mat_<double>(3,3) << 
            fx_, 0, cx_,
            0, fy_, cy_,
            0, 0, 1);
        return K_cv;
    }
    
    const Eigen::Matrix3d& K() const { return K_; }
    double fx() const { return fx_; }
    double fy() const { return fy_; }
    double cx() const { return cx_; }
    double cy() const { return cy_; }
    
private:
    double fx_, fy_;  // 초점 거리
    double cx_, cy_;  // 주점
    Eigen::Matrix3d K_;  // 내부 파라미터 행렬
};

} // namespace vo

#endif
```

---

## Step 3: 메인 테스트

### src/main.cpp

```cpp
#include "types.hpp"
#include "camera.hpp"
#include <iostream>

using namespace vo;

void testCameraProjection() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "카메라 투영 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // KITTI 파라미터
    PinholeCamera camera(718.856, 718.856, 607.1928, 185.2157);
    
    std::cout << "Camera matrix K:" << std::endl;
    std::cout << camera.K() << "\n" << std::endl;
    
    // 3D 점
    Eigen::Vector3d p3d(1.0, 0.5, 5.0);  // 5m 앞
    std::cout << "3D point: " << p3d.transpose() << std::endl;
    
    // 투영
    Eigen::Vector2d p2d = camera.project(p3d);
    std::cout << "2D projection: " << p2d.transpose() << std::endl;
    
    // 정규화
    Eigen::Vector3d p_norm = camera.normalize(p2d);
    std::cout << "Normalized (depth=1): " << p_norm.transpose() << std::endl;
    
    // 역투영
    Eigen::Vector3d p3d_back = camera.backproject(p2d, 5.0);
    std::cout << "Backprojected: " << p3d_back.transpose() << std::endl;
    
    // 오차 확인
    Eigen::Vector3d error = p3d - p3d_back;
    std::cout << "Error: " << error.norm() << " (should be ~0)\n" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 1: VO 기본 타입 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // Pose 테스트
    testPoseOperations();
    
    // MapPoint 테스트
    testMapPoint();
    
    // Camera 테스트
    testCameraProjection();
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 모든 테스트 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week1_VO_Basics)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# OpenCV
find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Include
include_directories(${PROJECT_SOURCE_DIR}/include)

# Executable
add_executable(vo_basics
    src/main.cpp
    src/types.cpp
)

target_link_libraries(vo_basics
    ${OpenCV_LIBS}
)
```

---

## 빌드 및 실행

```bash
cd week1_vo_basics
mkdir build && cd build
cmake ..
make
./vo_basics
```

### 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 1: VO 기본 타입 테스트
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Pose 연산 테스트
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Pose 1:
  1   0   0   1
  0   1   0   0
  0   0   1   0
  0   0   0   1

Pose1 * Pose1_inv (should be I):
  1   0   0   0
  0   1   0   0
  0   0   1   0
  0   0   0   1

Original point: [0, 0, 1]
Transformed: [1, 0, 1]

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
카메라 투영 테스트
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

3D point: [1, 0.5, 5]
2D projection: [750.76, 257.00]
Normalized (depth=1): [0.2, 0.1, 1]
Backprojected: [1, 0.5, 5]
Error: 2.3e-15 (should be ~0)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ 모든 테스트 완료!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] Pose 클래스 구현
- [ ] inverse, compose 연산 테스트
- [ ] MapPoint 구조체 정의
- [ ] PinholeCamera 클래스 구현
- [ ] project/backproject 테스트
- [ ] 빌드 성공
- [ ] 모든 테스트 통과

---

## 💡 핵심 개념

1. **SE(3) 변환**
   - 회전 R ∈ SO(3)
   - 이동 t ∈ ℝ³
   - 합성, 역변환

2. **Pinhole 모델**
   - 투영: 3D → 2D
   - 정규화 좌표
   - 역투영: 2D + depth → 3D

3. **좌표계**
   - 카메라 좌표계
   - 정규화 평면
   - 이미지 좌표계

---

## 🏗️ mini_slam 구현 (이번 주 핵심)

> 이 실습의 최종 목적은 `mini_slam` 프로젝트의 뼈대를 만드는 것이다.
> Phase 2의 mini_vo 코드를 검토하고, mini_slam 프로젝트로 이식한다.

**작업 내용**:

| 작업 | 내용 |
|------|------|
| mini_vo 코드 검토 | Phase 2 mini_vo의 각 모듈(camera, feature_detector, tracker 등) 동작 확인 |
| mini_slam 뼈대 생성 | `Studies/Phase 3/mini_slam/` 디렉토리 + CMakeLists.txt 생성 |
| mini_vo 모듈 이식 | Phase 2 코드를 mini_slam/include, mini_slam/src로 복사 및 정리 |
| Pose, MapPoint 타입 통일 | 이번 주 실습의 `Pose`, `MapPoint`를 mini_slam 공통 타입으로 사용 |

**구현 파일**: `Studies/Phase 3/mini_slam/`

```
mini_slam/
├── CMakeLists.txt
├── main.cpp                  ← Phase 3 끝에 전체 파이프라인 실행
├── include/
│   ├── types.h               ← Pose, MapPoint (이번 주)
│   ├── camera.h              ← Phase 2 재사용
│   ├── feature_detector.h    ← Phase 2 재사용
│   ├── feature_matcher.h     ← Phase 2 재사용
│   ├── epipolar.h            ← Phase 2 재사용
│   ├── pose_recovery.h       ← Phase 2 재사용
│   ├── triangulator.h        ← Phase 2 재사용
│   └── tracker.h             ← Phase 2 재사용
└── src/
    └── ...
```

### 완성 기준

```bash
cd Studies/Phase\ 3/mini_slam/build
cmake .. && make

# mini_slam 빌드 성공 + Phase 2 모듈 정상 동작 확인
```

---

**다음**: Week 2에서 Essential Matrix로 모션 추정!
