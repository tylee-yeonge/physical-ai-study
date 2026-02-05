# Week 3-4: 특징점 검출과 매칭 실습 (C++)

> 🎯 **목표**: FAST, ORB 특징점 검출 및 스테레오 매칭 구현  
> 💻 **언어**: C++ (OpenCV 4.x)  
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo Camera  
> ⏰ **예상 시간**: 12-16시간 (2주)

---

## 📋 Week 3: 특징점 검출

### 실습 1: FAST 코너 검출

**src/fast_detector.cpp**:
```cpp
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera" << std::endl;
        return -1;
    }
    
    // FAST 검출기 생성
    cv::Ptr<cv::FastFeatureDetector> fast = cv::FastFeatureDetector::create();
    fast->setThreshold(30);
    fast->setNonmaxSuppression(true);
    
    int max_features = 500;
    
    cv::Mat frame;
    while (true) {
        auto start = std::chrono::high_resolution_clock::now();
        
        cap >> frame;
        if (frame.empty()) break;
        
        // FAST 검출
        std::vector<cv::KeyPoint> keypoints;
        fast->detect(frame, keypoints);
        
        // 응답값 기준 정렬 후 상위 N개만 선택
        if (keypoints.size() > max_features) {
            std::sort(keypoints.begin(), keypoints.end(),
                     [](const cv::KeyPoint& a, const cv::KeyPoint& b) {
                         return a.response > b.response;
                     });
            keypoints.resize(max_features);
        }
        
        // 시각화
        cv::Mat display;
        cv::drawKeypoints(frame, keypoints, display, 
                         cv::Scalar(0, 255, 0), 
                         cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        
        auto end = std::chrono::high_resolution_clock::now();
        double fps = 1000.0 / std::chrono::duration<double, std::milli>(end - start).count();
        
        cv::putText(display, 
                   "FPS: " + std::to_string(int(fps)) + 
                   " | Features: " + std::to_string(keypoints.size()),
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 
                   0.8, cv::Scalar(0, 255, 0), 2);
        
        cv::imshow("FAST Feature Detection", display);
        
        int key = cv::waitKey(1);
        if (key == 27) break;  // ESC
        
        // 파라미터 조정 (키보드 인터랙션)
        if (key == 'q') {
            int threshold = fast->getThreshold();
            fast->setThreshold(std::max(5, threshold - 5));
            std::cout << "Threshold: " << fast->getThreshold() << std::endl;
        }
        if (key == 'w') {
            int threshold = fast->getThreshold();
            fast->setThreshold(std::min(100, threshold + 5));
            std::cout << "Threshold: " << fast->getThreshold() << std::endl;
        }
    }
    
    return 0;
}
```

### 실습 2: ORB 특징점 + 디스크립터

**src/orb_detector.cpp**:
```cpp
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

int main() {
    // ORB 검출기 생성
    cv::Ptr<cv::ORB> orb = cv::ORB::create(
        500,                    // nfeatures
        1.2f,                   // scaleFactor
        8,                      // nlevels
        31,                     // edgeThreshold
        0,                      // firstLevel
        2,                      // WTA_K
        cv::ORB::HARRIS_SCORE,  // scoreType
        31,                     // patchSize
        20                      // fastThreshold
    );
    
    cv::VideoCapture cap(0);
    
    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;
        
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        
        // ORB 검출 및 디스크립터 계산
        orb->detectAndCompute(frame, cv::noArray(), keypoints, descriptors);
        
        // 시각화
        cv::Mat display;
        cv::drawKeypoints(frame, keypoints, display,
                         cv::Scalar(255, 0, 0),
                         cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        
        cv::putText(display, 
                   "ORB Features: " + std::to_string(keypoints.size()) +
                   " | Descriptor size: " + std::to_string(descriptors.rows) + "x" +
                   std::to_string(descriptors.cols),
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                   0.7, cv::Scalar(255, 0, 0), 2);
        
        cv::imshow("ORB Feature Detection", display);
        
        if (cv::waitKey(1) == 27) break;
    }
    
    return 0;
}
```

---

## 📋 Week 4: 특징점 매칭

### 실습 3: 스테레오 매칭 (Brute-Force)

**include/stereo_feature_matcher.hpp**:
```cpp
#ifndef STEREO_FEATURE_MATCHER_HPP
#define STEREO_FEATURE_MATCHER_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <vector>

class StereoFeatureMatcher {
public:
    StereoFeatureMatcher();
    
    void detectAndMatch(const cv::Mat& img_left, 
                       const cv::Mat& img_right,
                       std::vector<cv::Point2f>& pts_left,
                       std::vector<cv::Point2f>& pts_right);
    
    void visualizeMatches(const cv::Mat& img_left,
                         const cv::Mat& img_right,
                         const std::vector<cv::KeyPoint>& kp_left,
                         const std::vector<cv::KeyPoint>& kp_right,
                         const std::vector<cv::DMatch>& matches,
                         cv::Mat& output);
    
private:
    cv::Ptr<cv::ORB> orb_;
    cv::Ptr<cv::BFMatcher> matcher_;
    
    float ratio_threshold_;
};

#endif
```

**src/stereo_feature_matcher.cpp**:
```cpp
#include "../include/stereo_feature_matcher.hpp"

StereoFeatureMatcher::StereoFeatureMatcher() 
    : ratio_threshold_(0.7f) {
    
    orb_ = cv::ORB::create(1000);
    matcher_ = cv::BFMatcher::create(cv::NORM_HAMMING, false);
}

void StereoFeatureMatcher::detectAndMatch(
    const cv::Mat& img_left, 
    const cv::Mat& img_right,
    std::vector<cv::Point2f>& pts_left,
    std::vector<cv::Point2f>& pts_right) {
    
    // ORB 검출
    std::vector<cv::KeyPoint> kp_left, kp_right;
    cv::Mat desc_left, desc_right;
    
    orb_->detectAndCompute(img_left, cv::noArray(), kp_left, desc_left);
    orb_->detectAndCompute(img_right, cv::noArray(), kp_right, desc_right);
    
    // KNN 매칭
    std::vector<std::vector<cv::DMatch>> knn_matches;
    matcher_->knnMatch(desc_left, desc_right, knn_matches, 2);
    
    // Lowe's Ratio Test
    std::vector<cv::DMatch> good_matches;
    for (const auto& match : knn_matches) {
        if (match.size() < 2) continue;
        if (match[0].distance < ratio_threshold_ * match[1].distance) {
            good_matches.push_back(match[0]);
        }
    }
    
    // 에피폴라 제약 필터링 (rectified 이미지에서 y 좌표가 비슷해야 함)
    pts_left.clear();
    pts_right.clear();
    
    for (const auto& m : good_matches) {
        cv::Point2f pt_l = kp_left[m.queryIdx].pt;
        cv::Point2f pt_r = kp_right[m.trainIdx].pt;
        
        // Y좌표 차이가 2픽셀 이내 (rectified 이미지 가정)
        if (std::abs(pt_l.y - pt_r.y) < 2.0f) {
            pts_left.push_back(pt_l);
            pts_right.push_back(pt_r);
        }
    }
}

void StereoFeatureMatcher::visualizeMatches(
    const cv::Mat& img_left,
    const cv::Mat& img_right,
    const std::vector<cv::KeyPoint>& kp_left,
    const std::vector<cv::KeyPoint>& kp_right,
    const std::vector<cv::DMatch>& matches,
    cv::Mat& output) {
    
    cv::drawMatches(img_left, kp_left, img_right, kp_right,
                   matches, output,
                   cv::Scalar(0, 255, 0),
                   cv::Scalar(0, 0, 255),
                   std::vector<char>(),
                   cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
}
```

### 실습 4: RANSAC Outlier 제거

**src/ransac_demo.cpp**:
```cpp
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>

int main() {
    // 스테레오 이미지 로드
    cv::Mat img_left = cv::imread("left.jpg", cv::IMREAD_GRAYSCALE);
    cv::Mat img_right = cv::imread("right.jpg", cv::IMREAD_GRAYSCALE);
    
    // ORB 매칭
    cv::Ptr<cv::ORB> orb = cv::ORB::create(1000);
    
    std::vector<cv::KeyPoint> kp_left, kp_right;
    cv::Mat desc_left, desc_right;
    
    orb->detectAndCompute(img_left, cv::noArray(), kp_left, desc_left);
    orb->detectAndCompute(img_right, cv::noArray(), kp_right, desc_right);
    
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<cv::DMatch> matches;
    matcher.match(desc_left, desc_right, matches);
    
    // 좌표 추출
    std::vector<cv::Point2f> pts_left, pts_right;
    for (const auto& m : matches) {
        pts_left.push_back(kp_left[m.queryIdx].pt);
        pts_right.push_back(kp_right[m.trainIdx].pt);
    }
    
    // Fundamental Matrix로 RANSAC
    std::vector<uchar> inlier_mask;
    cv::Mat F = cv::findFundamentalMat(pts_left, pts_right, 
                                       cv::FM_RANSAC, 
                                       3.0,           // distance threshold
                                       0.99,          // confidence
                                       inlier_mask);
    
    // Inlier/Outlier 분류
    std::vector<cv::DMatch> inlier_matches;
    int inlier_count = 0;
    
    for (size_t i = 0; i < inlier_mask.size(); i++) {
        if (inlier_mask[i]) {
            inlier_matches.push_back(matches[i]);
            inlier_count++;
        }
    }
    
    std::cout << "Total matches: " << matches.size() << std::endl;
    std::cout << "Inliers: " << inlier_count << std::endl;
    std::cout << "Outliers: " << matches.size() - inlier_count << std::endl;
    std::cout << "Inlier ratio: " << 
        (float)inlier_count / matches.size() * 100 << "%" << std::endl;
    
    // 시각화
    cv::Mat img_before, img_after;
    cv::drawMatches(img_left, kp_left, img_right, kp_right,
                   matches, img_before,
                   cv::Scalar(0, 255, 0));
    cv::drawMatches(img_left, kp_left, img_right, kp_right,
                   inlier_matches, img_after,
                   cv::Scalar(0, 255, 0));
    
    cv::imshow("Before RANSAC", img_before);
    cv::imshow("After RANSAC (Inliers only)", img_after);
    cv::waitKey(0);
    
    return 0;
}
```

---

## ✅ 체크리스트

### Week 3: 특징점 검출
- [ ] FAST 검출기 실시간 구현
- [ ] 파라미터 조정 (threshold, max_features)
- [ ] 30fps 달성 확인
- [ ] ORB 디스크립터 계산 구현

### Week 4: 매칭
- [ ] 스테레오 매칭 구현
- [ ] Lowe's Ratio Test 적용
- [ ] 에피폴라 제약 필터링
- [ ] RANSAC outlier 제거
- [ ] Inlier ratio > 50% 달성

---

## 💡 성능 최적화 팁 (Jetson)

```cpp
// 1. OpenCV CUDA 가속 (Jetson에서 매우 효과적)
# include <opencv2/cudafeatures2d.hpp>

cv::Ptr<cv::cuda::ORB> orb_cuda = cv::cuda::ORB::create(1000);

// 2. 병렬 처리
#include <omp.h>
#pragma omp parallel for
for (int i = 0; i < num_images; i++) {
    // 병렬 처리
}

// 3. 이미지 다운샘플링
cv::resize(img, img_small, cv::Size(), 0.5, 0.5);
```

---

**다음 단계**: Week 5-7 - 에피폴라 기하학 + 스테레오 Depth
