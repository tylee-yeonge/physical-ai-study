# Week 6: Disparity Map 생성 실습 (C++)

> 🎯 **목표**: SGBM으로 Disparity Map 생성 및 시각화
> 💻 **언어**: C++ (OpenCV 4.x)
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo (캘리브레이션 완료)
> ⏰ **예상 시간**: 6-8시간

---

## 📋 전제 조건

- [ ] Week 5 Stereo Rectification 완료
- [ ] Rectified 이미지 생성 가능

---

## 🔧 실습: StereoMatcher 클래스 구현

### 헤더 파일

**include/stereo_matcher.hpp**:
```cpp
#ifndef STEREO_MATCHER_HPP
#define STEREO_MATCHER_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>

class StereoMatcher {
public:
    StereoMatcher();

    void setParameters(int numDisparities, int blockSize);

    void computeDisparity(const cv::Mat& rect_left,
                         const cv::Mat& rect_right,
                         cv::Mat& disparity);

    void visualizeDisparity(const cv::Mat& disparity, cv::Mat& vis);

private:
    cv::Ptr<cv::StereoSGBM> stereo_;
    int numDisparities_;
    int blockSize_;
};

#endif
```

### 구현 파일

**src/stereo_matcher.cpp**:
```cpp
#include "../include/stereo_matcher.hpp"

StereoMatcher::StereoMatcher()
    : numDisparities_(64), blockSize_(5) {

    // SGBM (Semi-Global Block Matching) 설정
    stereo_ = cv::StereoSGBM::create(
        0,                                    // minDisparity
        numDisparities_,
        blockSize_,
        8 * 3 * blockSize_ * blockSize_,     // P1
        32 * 3 * blockSize_ * blockSize_,    // P2
        1,                                    // disp12MaxDiff
        63,                                   // preFilterCap
        10,                                   // uniquenessRatio
        100,                                  // speckleWindowSize
        32,                                   // speckleRange
        cv::StereoSGBM::MODE_SGBM_3WAY
    );
}

void StereoMatcher::setParameters(int numDisparities, int blockSize) {
    numDisparities_ = numDisparities;
    blockSize_ = blockSize;

    stereo_->setNumDisparities(numDisparities_);
    stereo_->setBlockSize(blockSize_);
    stereo_->setP1(8 * 3 * blockSize_ * blockSize_);
    stereo_->setP2(32 * 3 * blockSize_ * blockSize_);
}

void StereoMatcher::computeDisparity(
    const cv::Mat& rect_left,
    const cv::Mat& rect_right,
    cv::Mat& disparity) {

    cv::Mat disp16;
    stereo_->compute(rect_left, rect_right, disp16);

    // 16-bit to 32-bit float (divide by 16)
    disp16.convertTo(disparity, CV_32F, 1.0 / 16.0);
}

void StereoMatcher::visualizeDisparity(const cv::Mat& disparity, cv::Mat& vis) {
    // Normalize to 0-255
    cv::normalize(disparity, vis, 0, 255, cv::NORM_MINMAX, CV_8U);

    // Apply colormap
    cv::applyColorMap(vis, vis, cv::COLORMAP_JET);

    // 유효하지 않은 disparity (0 이하) 검은색 처리
    for (int i = 0; i < disparity.rows; i++) {
        for (int j = 0; j < disparity.cols; j++) {
            if (disparity.at<float>(i, j) <= 0) {
                vis.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
            }
        }
    }
}
```

---

## ✅ 체크리스트

- [ ] `StereoMatcher` SGBM 구현
- [ ] Disparity 시각화 (Colormap)
- [ ] 파라미터 튜닝 (numDisparities, blockSize)
- [ ] 유효하지 않은 disparity 처리

---

## 🏗️ mini_vo 구현 (이번 주 핵심)

**구현 파일**: `Studies/Phase 2/mini_vo/src/pose_recovery.cpp`

### 구현해야 할 내용

| 함수 | 내용 |
|------|------|
| `recover()` | E → SVD → W 행렬 → 4가지 (R, t) → Cheirality Check → 올바른 해 반환 |

### 완성 기준

```bash
./mini_vo

# W6 출력 예시
# [포즈 복원]
#   Cheirality inlier: 142 / 150
#   R (det=1.000):
#     [0.999, -0.012, 0.034]
#     [0.011,  0.999, 0.021]
#     [-0.034, -0.021, 0.999]
#   t: [0.98, 0.03, 0.19]  (단위 벡터)
```

`det(R) = 1.000`이고 Cheirality inlier 비율이 높으면 성공이다.

---

**다음 단계**: Week 7 - Depth Map 계산 + 실시간 데모
