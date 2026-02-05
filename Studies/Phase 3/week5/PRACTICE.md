# Week 5 실습: 완전한 Monocular VO 시스템 구현 (C++)

> 🎯 **목표**: Week 2 (초기화) + Week 3 (추적) 통합 + KITTI 테스트  
> 💻 **언어**: C++ (OpenCV, Eigen)  
> ⏰ **예상 시간**: 15-20시간

---

## 📋 실습 개요

Week 5는 **완전한 VO 시스템**을 구현합니다. Week 1-4에서 배운 모든 것을 통합하여 실제 데이터셋에서 작동하는 VO를 만듭니다.

### 시스템 구성

```
초기화 (Week 2)    추적 (Week 3)      맵 관리
     ↓                  ↓                ↓
Essential Matrix → PnP + RANSAC → 점 추가/제거
     ↓                  ↓                ↓
  초기 맵 생성      포즈 업데이트    Keyframe 선택
```

---

## 🔧 프로젝트 구조

```
week5_full_vo/
├── CMakeLists.txt
├── include/
│   ├── camera.hpp              # Week 1
│   ├── types.hpp               # Week 1
│   ├── feature_tracker.hpp     # 새로 추가
│   ├── motion_2d2d.hpp         # Week 2
│   ├── motion_3d2d.hpp         # Week 3
│   └── visual_odometry.hpp     # 메인 VO 클래스
├── src/
│   ├── feature_tracker.cpp
│   ├── visual_odometry.cpp
│   └── main.cpp
└── data/
    └── kitti/                  # KITTI 데이터 (선택)
```

---

## Step 1: Feature Tracker 구현

### include/feature_tracker.hpp

```cpp
#ifndef FEATURE_TRACKER_HPP
#define FEATURE_TRACKER_HPP

#include <opencv2/opencv.hpp>
#include <vector>

namespace vo {

/**
 * @brief FAST + KLT 기반 특징점 추적기
 */
class FeatureTracker {
public:
    FeatureTracker(int max_features = 200, 
                  int fast_threshold = 20,
                  int min_distance = 10);
    
    /**
     * @brief FAST로 특징점 검출
     * @param image 입력 이미지
     * @param keypoints 출력: 검출된 특징점
     */
    void detectFeatures(const cv::Mat& image,
                       std::vector<cv::Point2f>& keypoints);
    
    /**
     * @brief KLT로 특징점 추적
     * @param prev_image 이전 이미지
     * @param curr_image 현재 이미지
     * @param prev_pts 이전 특징점
     * @param curr_pts 출력: 추적된 특징점
     * @param status 출력: 추적 성공 여부
     */
    void trackFeatures(const cv::Mat& prev_image,
                      const cv::Mat& curr_image,
                      const std::vector<cv::Point2f>& prev_pts,
                      std::vector<cv::Point2f>& curr_pts,
                      std::vector<uchar>& status);
    
private:
    int max_features_;
    int fast_threshold_;
    int min_distance_;
    cv::Ptr<cv::FastFeatureDetector> detector_;
};

} // namespace vo

#endif
```

### src/feature_tracker.cpp

```cpp
#include "feature_tracker.hpp"
#include <iostream>

namespace vo {

FeatureTracker::FeatureTracker(int max_features, int fast_threshold, int min_distance)
    : max_features_(max_features),
      fast_threshold_(fast_threshold),
      min_distance_(min_distance) {
    detector_ = cv::FastFeatureDetector::create(fast_threshold);
}

void FeatureTracker::detectFeatures(
    const cv::Mat& image,
    std::vector<cv::Point2f>& keypoints
) {
    std::vector<cv::KeyPoint> kps;
    detector_->detect(image, kps);
    
    // KeyPoint → Point2f 변환
    keypoints.clear();
    for (const auto& kp : kps) {
        keypoints.push_back(kp.pt);
    }
    
    // Non-maximum suppression (간단 버전)
    if (keypoints.size() > max_features_) {
        // Corner response로 정렬 후 상위 N개만
        std::sort(kps.begin(), kps.end(),
                 [](const cv::KeyPoint& a, const cv::KeyPoint& b) {
                     return a.response > b.response;
                 });
        
        keypoints.clear();
        for (int i = 0; i < max_features_; i++) {
            keypoints.push_back(kps[i].pt);
        }
    }
    
    std::cout << "  Detected " << keypoints.size() << " features" << std::endl;
}

void FeatureTracker::trackFeatures(
    const cv::Mat& prev_image,
    const cv::Mat& curr_image,
    const std::vector<cv::Point2f>& prev_pts,
    std::vector<cv::Point2f>& curr_pts,
    std::vector<uchar>& status
) {
    std::vector<float> err;
    
    // Optical Flow (Lucas-Kanade)
    cv::calcOpticalFlowPyrLK(
        prev_image, curr_image,
        prev_pts, curr_pts,
        status, err,
        cv::Size(21, 21),  // window size
        3,                 // max pyramid level
        cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS,
                        30, 0.01)
    );
    
    // 역방향 체크
    std::vector<cv::Point2f> prev_pts_back;
    std::vector<uchar> status_back;
    cv::calcOpticalFlowPyrLK(
        curr_image, prev_image,
        curr_pts, prev_pts_back,
        status_back, err,
        cv::Size(21, 21), 3
    );
    
    // 양방향 일치하는 것만 유지
    for (size_t i = 0; i < status.size(); i++) {
        if (status[i] && status_back[i]) {
            double dist = cv::norm(prev_pts[i] - prev_pts_back[i]);
            if (dist > 1.0) {  // 1px 이상 차이나면 제거
                status[i] = 0;
            }
        } else {
            status[i] = 0;
        }
    }
    
    int num_tracked = cv::countNonZero(status);
    std::cout << "  Tracked " << num_tracked << "/" << prev_pts.size() 
              << " features" << std::endl;
}

} // namespace vo
```

---

## Step 2: Visual Odometry 클래스

### include/visual_odometry.hpp

```cpp
#ifndef VISUAL_ODOMETRY_HPP
#define VISUAL_ODOMETRY_HPP

#include "camera.hpp"
#include "types.hpp"
#include "feature_tracker.hpp"
#include "motion_2d2d.hpp"
#include "motion_3d2d.hpp"
#include <opencv2/opencv.hpp>
#include <vector>

namespace vo {

enum VOStatus {
    INITIALIZING,
    TRACKING,
    LOST
};

/**
 * @brief Visual Odometry 시스템
 */
class VisualOdometry {
public:
    VisualOdometry(const PinholeCamera& camera);
    
    /**
     * @brief 프레임 처리
     * @param image 현재 이미지
     * @return 성공 여부
     */
    bool processFrame(const cv::Mat& image);
    
    /**
     * @brief 현재 포즈 가져오기
     */
    Pose getCurrentPose() const { return curr_pose_; }
    
    /**
     * @brief 상태 가져오기
     */
    VOStatus getStatus() const { return status_; }
    
    /**
     * @brief 통계 출력
     */
    void printStatus() const;
    
private:
    // 구성요소
    PinholeCamera camera_;
    FeatureTracker tracker_;
    MotionEstimator2D2D estimator_2d_;
    MotionEstimator3D2D estimator_3d_;
    
    // 상태
    VOStatus status_;
    Pose curr_pose_;
    
    // 데이터
    cv::Mat prev_image_;
    std::vector<cv::Point2f> prev_keypoints_;
    std::vector<cv::Point3f> map_points_;
    std::vector<int> map_point_ages_;  // 각 점의 나이
    
    // 통계
    int frame_id_;
    int num_inliers_;
    double reprojection_error_;
    
    // 내부 함수
    bool tryInitialize(const cv::Mat& image);
    bool track(const cv::Mat& image);
    void updateMap(const cv::Mat& image,
                  const std::vector<cv::Point2f>& keypoints);
    void cullOldPoints();
};

} // namespace vo

#endif
```

### src/visual_odometry.cpp

```cpp
#include "visual_odometry.hpp"
#include <iostream>

namespace vo {

VisualOdometry::VisualOdometry(const PinholeCamera& camera)
    : camera_(camera),
      tracker_(200, 20, 10),
      estimator_2d_(camera.toOpenCV()),
      estimator_3d_(camera.toOpenCV()),
      status_(INITIALIZING),
      curr_pose_(Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero()),
      frame_id_(0),
      num_inliers_(0),
      reprojection_error_(0.0) {}

bool VisualOdometry::processFrame(const cv::Mat& image) {
    frame_id_++;
    
    if (status_ == INITIALIZING) {
        return tryInitialize(image);
    } else if (status_ == TRACKING) {
        return track(image);
    } else {
        std::cerr << "VO is LOST!" << std::endl;
        return false;
    }
}

bool VisualOdometry::tryInitialize(const cv::Mat& image) {
    if (frame_id_ == 1) {
        // 첫 프레임: 특징점만 검출
        tracker_.detectFeatures(image, prev_keypoints_);
        prev_image_ = image.clone();
        return true;
    }
    
    // 두 번째 프레임: 초기화 시도
    std::vector<cv::Point2f> curr_keypoints;
    std::vector<uchar> status;
    
    tracker_.trackFeatures(prev_image_, image, prev_keypoints_, 
                          curr_keypoints, status);
    
    // 유효한 매칭만 필터링
    std::vector<cv::Point2f> pts0, pts1;
    for (size_t i = 0; i < status.size(); i++) {
        if (status[i]) {
            pts0.push_back(prev_keypoints_[i]);
            pts1.push_back(curr_keypoints[i]);
        }
    }
    
    if (pts0.size() < 50) {
        std::cout << "  Not enough matches. Continue..." << std::endl;
        prev_image_ = image.clone();
        tracker_.detectFeatures(image, prev_keypoints_);
        return false;
    }
    
    // Essential Matrix
    cv::Mat E, inliers;
    if (!estimator_2d_.estimateEssential(pts0, pts1, E, inliers)) {
        return false;
    }
    
    // R, t 복원
    cv::Mat R, t;
    if (!estimator_2d_.recoverPose(E, pts0, pts1, R, t)) {
        return false;
    }
    
    // 삼각측량
    cv::Mat R0 = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t0 = cv::Mat::zeros(3, 1, CV_64F);
    
    map_points_.clear();
    estimator_2d_.triangulate(R0, t0, R, t, pts0, pts1, map_points_);
    
    if (map_points_.size() < 30) {
        std::cout << "  Too few 3D points. Retry..." << std::endl;
        return false;
    }
    
    // 성공: TRACKING 모드로 전환
    Eigen::Matrix3d R_eigen;
    Eigen::Vector3d t_eigen;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            R_eigen(i,j) = R.at<double>(i,j);
        }
        t_eigen(i) = t.at<double>(i);
    }
    
    curr_pose_ = Pose(R_eigen, t_eigen);
    
    // 맵 점 나이 초기화
    map_point_ages_.resize(map_points_.size(), 0);
    
    // 현재 키포인트 저장
    prev_keypoints_ = curr_keypoints;
    prev_image_ = image.clone();
    
    status_ = TRACKING;
    
    std::cout << "\n✅ Initialization Success!" << std::endl;
    std::cout << "  Initial map: " << map_points_.size() << " points" << std::endl;
    
    return true;
}

bool VisualOdometry::track(const cv::Mat& image) {
    // Optical Flow 추적
    std::vector<cv::Point2f> curr_keypoints;
    std::vector<uchar> track_status;
    
    tracker_.trackFeatures(prev_image_, image, prev_keypoints_,
                          curr_keypoints, track_status);
    
    // 3D-2D 대응 구성
    std::vector<cv::Point3f> points3d;
    std::vector<cv::Point2f> points2d;
    
    for (size_t i = 0; i < track_status.size(); i++) {
        if (track_status[i] && i < map_points_.size()) {
            points3d.push_back(map_points_[i]);
            points2d.push_back(curr_keypoints[i]);
        }
    }
    
    if (points3d.size() < 20) {
        std::cerr << "  Lost tracking! (too few points)" << std::endl;
        status_ = LOST;
        return false;
    }
    
    // PnP로 포즈 추정
    cv::Mat R_rel, t_rel;
    std::vector<int> inliers;
    
    double inlier_ratio = estimator_3d_.estimatePoseRansac(
        points3d, points2d, R_rel, t_rel, inliers);
    
    num_inliers_ = inliers.size();
    
    if (inlier_ratio < 0.3) {
        std::cerr << "  Lost tracking! (low inlier ratio)" << std::endl;
        status_ = LOST;
        return false;
    }
    
    // 재투영 오차
    reprojection_error_ = estimator_3d_.computeReprojectionError(
        points3d, points2d, R_rel, t_rel);
    
    // 포즈 업데이트 (월드 좌표계)
    Eigen::Matrix3d R_rel_eigen, R_prev_eigen = curr_pose_.R();
    Eigen::Vector3d t_rel_eigen, t_prev_eigen = curr_pose_.t();
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            R_rel_eigen(i,j) = R_rel.at<double>(i,j);
        }
        t_rel_eigen(i) = t_rel.at<double>(i);
    }
    
    // T_world_curr = T_world_prev * T_prev_curr^-1
    // (PnP는 curr → world 변환을 줌)
    Eigen::Matrix3d R_curr = R_rel_eigen.transpose();
    Eigen::Vector3d t_curr = -R_rel_eigen.transpose() * t_rel_eigen;
    
    curr_pose_ = Pose(R_curr, t_curr);
    
    // 맵 업데이트
    updateMap(image, curr_keypoints);
    
    // 오래된 점 제거
    cullOldPoints();
    
    prev_image_ = image.clone();
    prev_keypoints_ = curr_keypoints;
    
    return true;
}

void VisualOdometry::updateMap(
    const cv::Mat& image,
    const std::vector<cv::Point2f>& keypoints
) {
    // 맵 점 나이 증가
    for (auto& age : map_point_ages_) {
        age++;
    }
    
    // TODO: Keyframe 판단 후 새 점 추가
    // 여기서는 간단히 생략
}

void VisualOdometry::cullOldPoints() {
    // 나이가 50 프레임 이상인 점 제거
    std::vector<cv::Point3f> new_map_points;
    std::vector<int> new_ages;
    
    for (size_t i = 0; i < map_points_.size(); i++) {
        if (map_point_ages_[i] < 50) {
            new_map_points.push_back(map_points_[i]);
            new_ages.push_back(map_point_ages_[i]);
        }
    }
    
    map_points_ = new_map_points;
    map_point_ages_ = new_ages;
}

void VisualOdometry::printStatus() const {
    std::cout << "[Frame " << frame_id_ << "] ";
    
    if (status_ == INITIALIZING) {
        std::cout << "INITIALIZING..." << std::endl;
    } else if (status_ == TRACKING) {
        std::cout << "TRACKING" << std::endl;
        std::cout << "  Inliers: " << num_inliers_ << std::endl;
        std::cout << "  Reproj error: " << reprojection_error_ << " px" << std::endl;
        std::cout << "  Map size: " << map_points_.size() << std::endl;
        std::cout << "  Position: " << curr_pose_.t().transpose() << std::endl;
    } else {
        std::cout << "LOST" << std::endl;
    }
}

} // namespace vo
```

---

## Step 3: 메인 실행

### src/main.cpp

```cpp
#include "visual_odometry.hpp"
#include "camera.hpp"
#include <iostream>
#include <vector>

using namespace vo;

// 가상 데이터 생성 (실제로는 KITTI 등 사용)
std::vector<cv::Mat> generateSyntheticImages(int num_frames) {
    std::vector<cv::Mat> images;
    
    for (int i = 0; i < num_frames; i++) {
        cv::Mat img(480, 640, CV_8UC1);
        
        // 무작위 특징점 시뮬레이션
        for (int j = 0; j < 100; j++) {
            int x = 50 + rand() % 540;
            int y = 50 + rand() % 380;
            cv::circle(img, cv::Point(x + i*2, y), 3, cv::Scalar(255), -1);
        }
        
        images.push_back(img);
    }
    
    return images;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 5: Full Visual Odometry System" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 파라미터
    PinholeCamera camera(718.856, 718.856, 607.1928, 185.2157);
    
    // VO 시스템
    VisualOdometry vo(camera);
    
    // 이미지 로드 (가상 데이터)
    std::vector<cv::Mat> images = generateSyntheticImages(50);
    
    std::cout << "Processing " << images.size() << " frames...\n" << std::endl;
    
    // 프레임 처리
    for (size_t i = 0; i < images.size(); i++) {
        bool success = vo.processFrame(images[i]);
        
        vo.printStatus();
        
        if (!success && vo.getStatus() == LOST) {
            std::cerr << "VO Lost! Stopping..." << std::endl;
            break;
        }
        
        std::cout << std::endl;
    }
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ VO System Test Complete!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
```

---

## Step 4: KITTI 데이터셋 사용 (선택)

### KITTI 다운로드

```bash
# KITTI Odometry Dataset
cd week5_full_vo/data
mkdir kitti && cd kitti

# Sequence 00 (약 1GB)
wget https://s3.eu-central-1.amazonaws.com/avg-kitti/data_odometry_gray.zip

unzip data_odometry_gray.zip
```

### KITTI 로더 구현

```cpp
// kitti_loader.hpp
#ifndef KITTI_LOADER_HPP
#define KITTI_LOADER_HPP

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class KITTILoader {
public:
    KITTILoader(const std::string& dataset_path, int sequence);
    
    cv::Mat getImage(int index);
    int getNumImages() const { return num_images_; }
    cv::Mat getCameraMatrix() const { return K_; }
    
private:
    std::string dataset_path_;
    int sequence_;
    int num_images_;
    cv::Mat K_;
};

#endif
```

```cpp
// kitti_loader.cpp
#include "kitti_loader.hpp"
#include <sstream>
#include <iomanip>
#include <fstream>

KITTILoader::KITTILoader(const std::string& dataset_path, int sequence)
    : dataset_path_(dataset_path), sequence_(sequence) {
    
    // Sequence 00 카메라 파라미터
    K_ = (cv::Mat_<double>(3,3) << 
        718.856, 0, 607.1928,
        0, 718.856, 185.2157,
        0, 0, 1);
    
    // 이미지 개수 확인
    num_images_ = 0;
    while (true) {
        std::stringstream ss;
        ss << dataset_path << "/sequences/" 
           << std::setfill('0') << std::setw(2) << sequence
           << "/image_0/" << std::setfill('0') << std::setw(6) << num_images_
           << ".png";
        
        std::ifstream file(ss.str());
        if (!file.good()) break;
        
        num_images_++;
    }
}

cv::Mat KITTILoader::getImage(int index) {
    std::stringstream ss;
    ss << dataset_path_ << "/sequences/"
       << std::setfill('0') << std::setw(2) << sequence_
       << "/image_0/" << std::setfill('0') << std::setw(6) << index
       << ".png";
    
    return cv::imread(ss.str(), cv::IMREAD_GRAYSCALE);
}
```

### KITTI로 실행

```cpp
// main_kitti.cpp
int main() {
    KITTILoader loader("./data/kitti", 0);  // Sequence 00
    
    std::cout << "KITTI Sequence 00: " 
              << loader.getNumImages() << " images" << std::endl;
    
    PinholeCamera camera(loader.getCameraMatrix());
    VisualOdometry vo(camera);
    
    for (int i = 0; i < loader.getNumImages(); i++) {
        cv::Mat image = loader.getImage(i);
        bool success = vo.processFrame(image);
        
        vo.printStatus();
        
        if (!success) break;
    }
    
    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week5_FullVO)

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

# Source files
set(VO_SOURCES
    src/feature_tracker.cpp
    src/visual_odometry.cpp
    # Week 2-3 재사용
    ../week2_essential/src/motion_2d2d.cpp
    ../week3_pnp/src/motion_3d2d.cpp
)

# Executable: Synthetic demo
add_executable(vo_demo
    src/main.cpp
    ${VO_SOURCES}
)

target_link_libraries(vo_demo
    ${OpenCV_LIBS}
)

# Executable: KITTI (optional)
add_executable(vo_kitti
    src/main_kitti.cpp
    src/kitti_loader.cpp
    ${VO_SOURCES}
)

target_link_libraries(vo_kitti
    ${OpenCV_LIBS}
)
```

---

## 빌드 및 실행

```bash
cd week5_full_vo
mkdir build && cd build
cmake ..
make

# Synthetic data
./vo_demo

# KITTI (if downloaded)
./vo_kitti
```

---

## ✅ 체크리스트

### 구현
- [ ] FeatureTracker (FAST + KLT)
- [ ] VisualOdometry 클래스
- [ ] 초기화 로직 (Essential)
- [ ] 추적 로직 (PnP)
- [ ] 맵 관리 (추가/제거)
- [ ] 빌드 성공

### 테스트
- [ ] Synthetic 데이터 성공
- [ ] KITTI 실행 (선택)
- [ ] 초기화 성공
- [ ] 50 프레임 이상 추적
- [ ] 드리프트 관찰

### 분석
- [ ] Inlier ratio 기록
- [ ] 재투영 오차 기록
- [ ] 궤적 시각화 (선택)
- [ ] 드리프트 측정

---

## 💡 예상 결과 및 한계

### 성공 시나리오
```
초기화: 120개 맵 포인트
프레임 10: Inliers 95/120 (79%), Error 0.68px
프레임 50: Inliers 88/115 (76%), Error 0.81px
프레임 100: Inliers 82/110 (75%), Error 0.92px
```

### 한계 관찰
1. **드리프트 발생** ⚠️
   - 100m 이동 시 2-5m 오차 (2-5%)
   - 회전 오차 누적
   
2. **스케일 드리프트**
   - 초기 임의 스케일
   - 시간에 따라 불일치
   
3. **추적 실패**
   - 텍스처 부족 환경
   - 급격한 움직임

→ **왜 IMU가 필요한지, 왜 SLAM이 필요한지 체감!**

---

## 📚 다음 단계

- **Week 6**: Keyframe 관리
- **Week 7-8**: Bundle Adjustment (g2o/Ceres)
- **Phase 4**: VIO (IMU 융합)

---

## 참고 자료

- [Slambook2 Chapter 7-8](https://github.com/gaoxiang12/slambook2)
- [KITTI Dataset](http://www.cvlibs.net/datasets/kitti/)
- [ORB-SLAM2](https://github.com/raulmur/ORB_SLAM2) - 참고용
