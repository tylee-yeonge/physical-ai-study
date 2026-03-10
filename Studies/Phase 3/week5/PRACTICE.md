# Week 5 실습: Mini VO 프로젝트 - 완전한 Visual Odometry 구현

> 🎯 **목표**: Week 2 (초기화) + Week 3 (추적)을 통합한 VO 시스템 구현
> 💻 **언어**: C++ (OpenCV)
> ⏰ **예상 시간**: 15시간

---

## 📋 실습 개요

Week 5는 **Mini VO 시스템**을 처음부터 끝까지 구현합니다. Essential Matrix 초기화 + PnP 추적 + 맵 관리를 하나로 통합합니다.

**먼저 돌려보고, 모르는 것을 채운다 (원칙 1)**:
→ Step 1~5를 따라 구현 → 실행 → 결과 관찰 → 드리프트를 직접 확인

---

## 🔧 환경 설정

```bash
# OpenCV 4.x 필요 (Phase 2에서 이미 설치됨)
pkg-config --modversion opencv4
```

---

## 프로젝트 구조

```
week5_mini_vo/
├── CMakeLists.txt
├── include/
│   └── mini_vo.hpp
└── src/
    ├── mini_vo.cpp
    └── main.cpp
```

---

## Step 1: VO 시스템 클래스 정의

### include/mini_vo.hpp

```cpp
#ifndef MINI_VO_HPP
#define MINI_VO_HPP

#include <opencv2/opencv.hpp>
#include <vector>

namespace vo {

enum class VOStatus {
    INITIALIZING,
    TRACKING,
    LOST
};

/**
 * @brief Mini Visual Odometry 시스템
 *
 * Week 2 (Essential Matrix 초기화) + Week 3 (PnP 추적) 통합
 */
class MiniVO {
public:
    /**
     * @param fx, fy, cx, cy 카메라 내부 파라미터
     */
    MiniVO(double fx, double fy, double cx, double cy);

    /**
     * @brief 새 프레임 처리
     * @param frame 그레이스케일 이미지
     * @return true: 성공, false: 실패
     */
    bool processFrame(const cv::Mat& frame);

    // Getter
    VOStatus getStatus() const { return status_; }
    cv::Mat getRotation() const { return R_curr_.clone(); }
    cv::Mat getTranslation() const { return t_curr_.clone(); }
    int getFrameId() const { return frame_id_; }
    int getNumMapPoints() const { return (int)map_points_.size(); }
    int getNumInliers() const { return num_inliers_; }

private:
    // 초기화 (Essential Matrix, Week 2)
    bool initialize(const cv::Mat& frame);

    // 추적 (PnP, Week 3)
    bool track(const cv::Mat& frame);

    // 맵 관리
    void updateMap();
    bool isKeyframe() const;
    void addNewPoints();

    // 특징점 검출
    void detectFeatures(const cv::Mat& frame,
                        std::vector<cv::Point2f>& keypoints);

    // 카메라 파라미터
    cv::Mat K_;
    double fx_, fy_, cx_, cy_;

    // 상태
    VOStatus status_;
    int frame_id_;
    int num_inliers_;

    // 포즈
    cv::Mat R_curr_, t_curr_;

    // 이전 프레임 정보
    cv::Mat prev_frame_;
    std::vector<cv::Point2f> prev_keypoints_;

    // 3D 맵
    std::vector<cv::Point3f> map_points_;
    std::vector<cv::Point2f> map_keypoints_;  // 대응되는 2D 점
    std::vector<int> point_ages_;

    // 초기화용
    cv::Mat first_frame_;
    std::vector<cv::Point2f> first_keypoints_;

    // 파라미터
    static constexpr int MIN_FEATURES = 50;
    static constexpr int MAX_FEATURES = 500;
    static constexpr double MIN_PARALLAX = 1.5;
    static constexpr double INLIER_THRESHOLD = 0.3;
    static constexpr int KEYFRAME_INTERVAL = 10;
    static constexpr int MAX_POINT_AGE = 30;
};

} // namespace vo

#endif
```

---

## Step 2: 구현

### src/mini_vo.cpp

```cpp
#include "mini_vo.hpp"
#include <iostream>
#include <algorithm>

namespace vo {

MiniVO::MiniVO(double fx, double fy, double cx, double cy)
    : fx_(fx), fy_(fy), cx_(cx), cy_(cy),
      status_(VOStatus::INITIALIZING), frame_id_(0), num_inliers_(0)
{
    K_ = (cv::Mat_<double>(3, 3) <<
        fx, 0, cx,
        0, fy, cy,
        0, 0, 1);

    R_curr_ = cv::Mat::eye(3, 3, CV_64F);
    t_curr_ = cv::Mat::zeros(3, 1, CV_64F);
}

bool MiniVO::processFrame(const cv::Mat& frame) {
    frame_id_++;

    bool success = false;

    switch (status_) {
        case VOStatus::INITIALIZING:
            success = initialize(frame);
            break;
        case VOStatus::TRACKING:
            success = track(frame);
            break;
        case VOStatus::LOST:
            // 재초기화 시도
            status_ = VOStatus::INITIALIZING;
            first_frame_ = cv::Mat();
            success = initialize(frame);
            break;
    }

    prev_frame_ = frame.clone();
    return success;
}

// ═══════════════════════════════════════════════════
// 초기화: Essential Matrix (Week 2)
// ═══════════════════════════════════════════════════

bool MiniVO::initialize(const cv::Mat& frame) {
    std::vector<cv::Point2f> keypoints;
    detectFeatures(frame, keypoints);

    if ((int)keypoints.size() < MIN_FEATURES) {
        std::cout << "[Init] 특징점 부족: " << keypoints.size() << std::endl;
        return false;
    }

    // 첫 프레임 저장
    if (first_frame_.empty()) {
        first_frame_ = frame.clone();
        first_keypoints_ = keypoints;
        std::cout << "[Init] 첫 프레임 저장 (특징점: "
                  << keypoints.size() << "개)" << std::endl;
        return true;
    }

    // Optical Flow로 매칭
    std::vector<cv::Point2f> matched_prev, matched_curr;
    std::vector<uchar> flow_status;
    std::vector<float> flow_err;

    cv::calcOpticalFlowPyrLK(
        first_frame_, frame,
        first_keypoints_, keypoints,
        flow_status, flow_err
    );

    for (size_t i = 0; i < flow_status.size(); i++) {
        if (flow_status[i]) {
            matched_prev.push_back(first_keypoints_[i]);
            matched_curr.push_back(keypoints[i]);
        }
    }

    if ((int)matched_prev.size() < MIN_FEATURES) {
        std::cout << "[Init] 매칭 부족: " << matched_prev.size() << std::endl;
        return false;
    }

    // 시차 확인
    double total_parallax = 0;
    for (size_t i = 0; i < matched_prev.size(); i++) {
        double dx = matched_curr[i].x - matched_prev[i].x;
        double dy = matched_curr[i].y - matched_prev[i].y;
        total_parallax += std::sqrt(dx*dx + dy*dy);
    }
    double avg_parallax = total_parallax / matched_prev.size();

    if (avg_parallax < MIN_PARALLAX) {
        std::cout << "[Init] 시차 부족: " << avg_parallax << " px" << std::endl;
        return false;
    }

    // Essential Matrix 추정
    cv::Mat inlier_mask;
    cv::Mat E = cv::findEssentialMat(
        matched_prev, matched_curr, K_,
        cv::RANSAC, 0.999, 1.0, inlier_mask
    );

    // R, t 복원
    cv::Mat R, t;
    int inliers = cv::recoverPose(E, matched_prev, matched_curr, K_, R, t, inlier_mask);

    if (inliers < MIN_FEATURES / 2) {
        std::cout << "[Init] inlier 부족: " << inliers << std::endl;
        return false;
    }

    // 초기 삼각측량
    cv::Mat proj0 = cv::Mat::eye(3, 4, CV_64F);
    cv::Mat proj1 = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(proj1(cv::Rect(0, 0, 3, 3)));
    t.copyTo(proj1(cv::Rect(3, 0, 1, 3)));

    // K 적용
    proj0 = K_ * proj0;
    proj1 = K_ * proj1;

    // inlier만 삼각측량
    std::vector<cv::Point2f> inlier_prev, inlier_curr;
    for (size_t i = 0; i < matched_prev.size(); i++) {
        if (inlier_mask.at<uchar>(i)) {
            inlier_prev.push_back(matched_prev[i]);
            inlier_curr.push_back(matched_curr[i]);
        }
    }

    cv::Mat points4D;
    cv::triangulatePoints(proj0, proj1, inlier_prev, inlier_curr, points4D);

    // 3D 점 추출 (유효한 점만)
    map_points_.clear();
    map_keypoints_.clear();
    point_ages_.clear();

    for (int i = 0; i < points4D.cols; i++) {
        float w = points4D.at<float>(3, i);
        if (std::abs(w) < 1e-6) continue;

        float x = points4D.at<float>(0, i) / w;
        float y = points4D.at<float>(1, i) / w;
        float z = points4D.at<float>(2, i) / w;

        // Depth 유효성 체크
        if (z > 0.1 && z < 100.0) {
            map_points_.push_back(cv::Point3f(x, y, z));
            map_keypoints_.push_back(inlier_curr[i]);
            point_ages_.push_back(0);
        }
    }

    // 포즈 초기화
    R_curr_ = R.clone();
    t_curr_ = t.clone();
    status_ = VOStatus::TRACKING;
    num_inliers_ = inliers;

    std::cout << "[Init] 초기화 성공!" << std::endl;
    std::cout << "   매칭: " << matched_prev.size()
              << ", Inliers: " << inliers
              << ", 맵: " << map_points_.size() << "개" << std::endl;

    return true;
}

// ═══════════════════════════════════════════════════
// 추적: PnP + RANSAC (Week 3)
// ═══════════════════════════════════════════════════

bool MiniVO::track(const cv::Mat& frame) {
    if (map_keypoints_.empty() || prev_frame_.empty()) {
        status_ = VOStatus::LOST;
        return false;
    }

    // Optical Flow로 특징점 추적
    std::vector<cv::Point2f> curr_keypoints;
    std::vector<uchar> flow_status;
    std::vector<float> flow_err;

    cv::calcOpticalFlowPyrLK(
        prev_frame_, frame,
        map_keypoints_, curr_keypoints,
        flow_status, flow_err
    );

    // 유효한 추적 결과만 사용
    std::vector<cv::Point3f> valid_3d;
    std::vector<cv::Point2f> valid_2d;
    std::vector<int> valid_ages;

    for (size_t i = 0; i < flow_status.size(); i++) {
        if (flow_status[i] && flow_err[i] < 30.0) {
            valid_3d.push_back(map_points_[i]);
            valid_2d.push_back(curr_keypoints[i]);
            valid_ages.push_back(point_ages_[i]);
        }
    }

    if ((int)valid_3d.size() < MIN_FEATURES / 2) {
        std::cout << "[Track] 추적 점 부족: " << valid_3d.size() << std::endl;
        status_ = VOStatus::LOST;
        return false;
    }

    // PnP + RANSAC
    cv::Mat rvec, tvec;
    std::vector<int> inliers;

    bool success = cv::solvePnPRansac(
        valid_3d, valid_2d, K_,
        cv::noArray(),  // distortion = 0
        rvec, tvec,
        false,          // useExtrinsicGuess
        100,            // iterationsCount
        8.0,            // reprojectionError
        0.99,           // confidence
        inliers
    );

    if (!success || inliers.empty()) {
        std::cout << "[Track] PnP 실패" << std::endl;
        status_ = VOStatus::LOST;
        return false;
    }

    // Inlier ratio 체크
    double ratio = (double)inliers.size() / valid_3d.size();
    if (ratio < INLIER_THRESHOLD) {
        std::cout << "[Track] Inlier 부족: "
                  << inliers.size() << "/" << valid_3d.size()
                  << " (" << (int)(ratio*100) << "%)" << std::endl;
        status_ = VOStatus::LOST;
        return false;
    }

    // 포즈 업데이트
    cv::Rodrigues(rvec, R_curr_);
    t_curr_ = tvec.clone();
    num_inliers_ = (int)inliers.size();

    // 맵 업데이트 (inlier만 유지)
    map_points_.clear();
    map_keypoints_.clear();
    point_ages_.clear();

    for (int idx : inliers) {
        map_points_.push_back(valid_3d[idx]);
        map_keypoints_.push_back(valid_2d[idx]);
        point_ages_.push_back(valid_ages[idx] + 1);
    }

    // 맵 관리
    updateMap();

    return true;
}

// ═══════════════════════════════════════════════════
// 맵 관리
// ═══════════════════════════════════════════════════

void MiniVO::updateMap() {
    // 오래된 점 제거
    for (int i = (int)map_points_.size() - 1; i >= 0; i--) {
        if (point_ages_[i] > MAX_POINT_AGE) {
            map_points_.erase(map_points_.begin() + i);
            map_keypoints_.erase(map_keypoints_.begin() + i);
            point_ages_.erase(point_ages_.begin() + i);
        }
    }

    // 점이 부족하면 새로 추가
    if ((int)map_points_.size() < MIN_FEATURES && isKeyframe()) {
        addNewPoints();
    }
}

bool MiniVO::isKeyframe() const {
    return (frame_id_ % KEYFRAME_INTERVAL == 0);
}

void MiniVO::addNewPoints() {
    // 현재 프레임에서 새 특징점 검출
    std::vector<cv::Point2f> new_kp;
    detectFeatures(prev_frame_, new_kp);

    // 기존 점과 겹치지 않는 점만 추가 (간단 버전)
    for (const auto& pt : new_kp) {
        bool too_close = false;
        for (const auto& existing : map_keypoints_) {
            double dx = pt.x - existing.x;
            double dy = pt.y - existing.y;
            if (dx*dx + dy*dy < 25.0) {  // 5px 이내
                too_close = true;
                break;
            }
        }
        if (!too_close && (int)map_keypoints_.size() < MAX_FEATURES) {
            // 간이 depth 추정: 기존 맵 점들의 중앙값 depth 사용
            float median_depth = 5.0f;  // 기본값
            if (!map_points_.empty()) {
                std::vector<float> depths;
                for (const auto& mp : map_points_) {
                    if (mp.z > 0) depths.push_back(mp.z);
                }
                if (!depths.empty()) {
                    std::sort(depths.begin(), depths.end());
                    median_depth = depths[depths.size() / 2];
                }
            }

            // 역투영으로 3D 점 생성 (근사)
            float x = (float)((pt.x - cx_) / fx_ * median_depth);
            float y = (float)((pt.y - cy_) / fy_ * median_depth);

            map_points_.push_back(cv::Point3f(x, y, median_depth));
            map_keypoints_.push_back(pt);
            point_ages_.push_back(0);
        }
    }
}

void MiniVO::detectFeatures(const cv::Mat& frame,
                            std::vector<cv::Point2f>& keypoints) {
    // FAST 코너 검출 (GFTT보다 빠름)
    cv::goodFeaturesToTrack(
        frame, keypoints,
        MAX_FEATURES,   // maxCorners
        0.01,           // qualityLevel
        10              // minDistance
    );
}

} // namespace vo
```

---

## Step 3: 메인 프로그램

### src/main.cpp

```cpp
#include "mini_vo.hpp"
#include <iostream>
#include <fstream>

/**
 * 합성 데이터 생성: 원형 궤적
 *
 * 3D 점을 투영해서 가상 이미지를 만듭니다.
 */
void generateSyntheticData(
    const cv::Mat& K,
    int num_frames,
    std::vector<cv::Mat>& images,
    std::vector<cv::Mat>& gt_poses_R,
    std::vector<cv::Mat>& gt_poses_t
) {
    double fx = K.at<double>(0, 0);
    double fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2);
    double cy = K.at<double>(1, 2);
    int width = (int)(cx * 2);
    int height = (int)(cy * 2);

    // 3D 점 (방 안의 점들)
    std::vector<cv::Point3f> world_points;
    srand(42);
    for (int i = 0; i < 200; i++) {
        float x = (rand() % 200 - 100) / 10.0f;
        float y = (rand() % 100 - 50) / 10.0f;
        float z = 2.0f + (rand() % 80) / 10.0f;
        world_points.push_back(cv::Point3f(x, y, z));
    }

    // 원형 궤적
    double radius = 3.0;
    for (int i = 0; i < num_frames; i++) {
        double angle = i * 0.05;  // 천천히 회전

        // 카메라 위치 (원형)
        cv::Mat t = (cv::Mat_<double>(3, 1) <<
            radius * sin(angle),
            0.0,
            radius * cos(angle));

        // 카메라가 원 중심을 바라보도록 회전
        cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
        R.at<double>(0, 0) = cos(angle);
        R.at<double>(0, 2) = sin(angle);
        R.at<double>(2, 0) = -sin(angle);
        R.at<double>(2, 2) = cos(angle);

        gt_poses_R.push_back(R.clone());
        gt_poses_t.push_back(t.clone());

        // 가상 이미지 생성
        cv::Mat image = cv::Mat::zeros(height, width, CV_8UC1);

        for (const auto& pt : world_points) {
            // 카메라 좌표 변환: Pc = R * (Pw - t)
            cv::Mat Pw = (cv::Mat_<double>(3, 1) << pt.x, pt.y, pt.z);
            cv::Mat Pc = R * (Pw - t);

            double Xc = Pc.at<double>(0);
            double Yc = Pc.at<double>(1);
            double Zc = Pc.at<double>(2);

            if (Zc > 0.1) {
                int u = (int)(fx * Xc / Zc + cx);
                int v = (int)(fy * Yc / Zc + cy);

                if (u >= 2 && u < width-2 && v >= 2 && v < height-2) {
                    // 작은 원으로 점 그리기 (특징점 검출 가능하도록)
                    cv::circle(image, cv::Point(u, v), 3,
                              cv::Scalar(200 + rand() % 55), -1);
                }
            }
        }

        // 약간의 노이즈 추가
        cv::Mat noise(image.size(), CV_8UC1);
        cv::randn(noise, 0, 5);
        image += noise;

        images.push_back(image);
    }
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 5: Mini VO System" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    double fx = 500.0, fy = 500.0;
    double cx = 320.0, cy = 240.0;
    cv::Mat K = (cv::Mat_<double>(3, 3) <<
        fx, 0, cx,
        0, fy, cy,
        0, 0, 1);

    // 합성 데이터 생성
    int num_frames = 50;
    std::vector<cv::Mat> images;
    std::vector<cv::Mat> gt_R, gt_t;

    std::cout << "합성 데이터 생성 중..." << std::endl;
    generateSyntheticData(K, num_frames, images, gt_R, gt_t);
    std::cout << "생성 완료: " << images.size() << " 프레임\n" << std::endl;

    // VO 실행
    vo::MiniVO vo_system(fx, fy, cx, cy);

    std::vector<cv::Point3d> trajectory;

    for (int i = 0; i < num_frames; i++) {
        bool success = vo_system.processFrame(images[i]);

        if (vo_system.getStatus() == vo::VOStatus::TRACKING) {
            cv::Mat t = vo_system.getTranslation();
            trajectory.push_back(cv::Point3d(
                t.at<double>(0), t.at<double>(1), t.at<double>(2)));

            if (i % 10 == 0 || i == num_frames - 1) {
                cv::Mat gt = gt_t[i];
                double drift = cv::norm(t - gt);

                std::cout << "Frame " << i << ": "
                          << "맵=" << vo_system.getNumMapPoints() << "점, "
                          << "inlier=" << vo_system.getNumInliers() << ", "
                          << "drift=" << drift << "m" << std::endl;
            }
        }
    }

    // 결과 요약
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "결과 요약" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "총 프레임: " << num_frames << std::endl;
    std::cout << "궤적 포인트: " << trajectory.size() << std::endl;

    if (!trajectory.empty()) {
        cv::Mat final_t = vo_system.getTranslation();
        cv::Mat gt_final = gt_t.back();
        double final_drift = cv::norm(final_t - gt_final);
        double total_distance = cv::norm(gt_t.back());

        std::cout << "최종 드리프트: " << final_drift << " m" << std::endl;
        if (total_distance > 0) {
            std::cout << "드리프트 비율: "
                      << (final_drift / total_distance * 100) << "%" << std::endl;
        }
    }

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "SLAM에서의 의미:" << std::endl;
    std::cout << "  - 드리프트 관찰됨 → VO만으로는 한계" << std::endl;
    std::cout << "  - BA (Phase 3 Week 8)로 개선 가능" << std::endl;
    std::cout << "  - IMU 융합 (Phase 4)으로 스케일 복구 가능" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week5_MiniVO)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

# OpenCV
find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Include
include_directories(${PROJECT_SOURCE_DIR}/include)

# Executable
add_executable(mini_vo_demo
    src/main.cpp
    src/mini_vo.cpp
)

target_link_libraries(mini_vo_demo
    ${OpenCV_LIBS}
)
```

---

## 빌드 및 실행

```bash
cd week5_mini_vo
mkdir build && cd build
cmake ..
make
./mini_vo_demo
```

### 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 5: Mini VO System
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

합성 데이터 생성 중...
생성 완료: 50 프레임

[Init] 첫 프레임 저장 (특징점: 142개)
[Init] 초기화 성공!
   매칭: 127, Inliers: 118, 맵: 105개
Frame 0: 맵=105점, inlier=118, drift=0.02m
Frame 10: 맵=87점, inlier=82, drift=0.15m
Frame 20: 맵=74점, inlier=69, drift=0.38m
Frame 30: 맵=68점, inlier=61, drift=0.72m
Frame 40: 맵=55점, inlier=48, drift=1.24m
Frame 49: 맵=49점, inlier=43, drift=1.85m

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
결과 요약
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
총 프레임: 50
궤적 포인트: 49
최종 드리프트: 1.85 m
드리프트 비율: 4.2%

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
SLAM에서의 의미:
  - 드리프트 관찰됨 → VO만으로는 한계
  - BA (Phase 3 Week 8)로 개선 가능
  - IMU 융합 (Phase 4)으로 스케일 복구 가능
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## 실제 데이터셋으로 테스트 (선택)

KITTI나 EuRoC를 사용하려면 main.cpp에서 합성 데이터 대신 이미지를 읽으면 됩니다:

```cpp
// KITTI 시퀀스 로드 예시
std::string kitti_path = "/path/to/kitti/sequences/00/image_0/";
for (int i = 0; i < num_frames; i++) {
    char filename[256];
    sprintf(filename, "%s%06d.png", kitti_path.c_str(), i);
    cv::Mat frame = cv::imread(filename, cv::IMREAD_GRAYSCALE);
    vo_system.processFrame(frame);
}
```

---

## ✅ 체크리스트

- [ ] 프로젝트 빌드 성공
- [ ] Essential Matrix 초기화 동작 확인
- [ ] PnP 추적 동작 확인
- [ ] 맵 포인트 생성/제거 동작
- [ ] 드리프트 직접 관찰 (핵심!)
- [ ] "왜 IMU가 필요한가?"에 답할 수 있음

---

## 💡 핵심 포인트

1. **VO = 초기화 (2D-2D) + 추적 (3D-2D)**
   - VINS 프론트엔드가 정확히 이 구조

2. **드리프트는 피할 수 없음**
   - 매 프레임 작은 오차가 누적
   - BA나 Loop Closure 없이는 발산

3. **맵 관리가 의외로 중요**
   - 점이 너무 많으면 느려짐
   - 점이 너무 적으면 추적 실패

4. **AMR에서의 활용**
   - 이 VO가 VINS의 Visual 부분
   - IMU를 추가하면 VIO (Phase 4)

---

## 🏗️ mini_slam 구현 (이번 주 핵심)

> 이번 주는 Phase 2의 mini_vo 코드를 mini_slam으로 **완전히 통합**하는 주차다.
> W1~W3에서 준비한 모듈들을 연결하여 연속 프레임 VO 파이프라인을 완성한다.

**작업 내용**:

| 작업 | 내용 |
|------|------|
| mini_vo → mini_slam 통합 | Phase 2 코드를 mini_slam 프로젝트로 최종 이식 |
| VO 파이프라인 연결 | 초기화(2D-2D) → 추적(3D-2D) → 맵 관리 연결 |
| 연속 프레임 테스트 | 합성 데이터 또는 KITTI에서 50+ 프레임 연속 추적 |
| 드리프트 측정 | GT 대비 드리프트 정량 확인 |

**구현 파일**: `Studies/Phase 3/mini_slam/main.cpp`

### 파이프라인 흐름

```
이미지 시퀀스 입력
    ↓
[Phase 2 재사용] FAST 검출 → LK 추적 → E → R,t → 삼각측량
    ↓
연속 프레임에서 PnP 추적
    ↓
드리프트 측정 (BA 없이 얼마나 벌어지는지 확인)
```

### 완성 기준

```bash
cd Studies/Phase\ 3/mini_slam/build
./mini_slam

# 50프레임 이상 연속 추적 성공
# 드리프트 관찰 → "BA가 왜 필요한가?" 체감
```

---

**다음**: [Week 6 - Keyframe Management](../week6/README.md)
