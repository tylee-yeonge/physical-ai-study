# Week 4: 특징점 매칭 실습 (C++)


> **목표**: 스테레오 매칭 및 RANSAC Outlier 제거 구현
> **언어**: C++ (OpenCV 4.x)
> **하드웨어**: Jetson Orin Nano + ELP Stereo Camera
> **예상 시간**: 6-8시간


---


## 전제 조건


- [ ] Week 3 특징점 검출 실습 완료
- [ ] ORB 디스크립터 개념 이해


---


## 실습 1: 스테레오 매칭 (Brute-Force)


### StereoFeatureMatcher 클래스 구현


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


---


## 실습 2: RANSAC Outlier 제거


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
                                       3.0, // distance threshold
                                       0.99, // confidence
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


## 체크리스트


- [ ] 스테레오 매칭 구현
- [ ] Lowe's Ratio Test 적용
- [ ] 에피폴라 제약 필터링
- [ ] RANSAC outlier 제거
- [ ] Inlier ratio > 50% 달성


---


## mini_vo 구현 (이번 주 핵심)


**구현 파일**: `Studies/Phase 2/mini_vo/src/feature_matcher.cpp`


### 구현해야 할 내용


| 함수 | 내용 |
|------|------|
| `match()` | BF 매칭 → Ratio Test(0.75) → RANSAC으로 outlier 제거 |


### 완성 기준


```bash
./mini_vo


# W4 출력 예시
# [매칭 결과]
# 원본 매칭: 300개
# Ratio Test 후: 180개
# RANSAC 후: 140개 ← inlier만 생존
```


---


**다음 단계**: Week 5 - 에피폴라 기하학 + 스테레오 Rectification
