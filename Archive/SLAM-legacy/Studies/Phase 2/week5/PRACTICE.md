# Week 5: Stereo Rectification 실습 (C++)

> 🎯 **목표**: 스테레오 이미지 정류(Rectification) 구현
> 💻 **언어**: C++ (OpenCV 4.x, Eigen3)
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo (캘리브레이션 완료)
> ⏰ **예상 시간**: 6-8시간

---

## 📋 전제 조건

- [ ] Week 2 캘리브레이션 완료 (`stereo_calib.yaml` 필요)
- [ ] Week 3-4 특징점 검출/매칭 이해

---

## 🔧 실습: StereoRectifier 클래스 구현

### 헤더 파일

**include/stereo_rectifier.hpp**:
```cpp
#ifndef STEREO_RECTIFIER_HPP
#define STEREO_RECTIFIER_HPP

#include <opencv2/opencv.hpp>
#include <string>

class StereoRectifier {
public:
    StereoRectifier(const std::string& calib_file);

    void computeRectification(cv::Size image_size);

    void rectify(const cv::Mat& left, const cv::Mat& right,
                cv::Mat& rect_left, cv::Mat& rect_right);

    const cv::Mat& getQ() const { return Q_; }

private:
    // 캘리브레이션 파라미터
    cv::Mat K_left_, dist_left_;
    cv::Mat K_right_, dist_right_;
    cv::Mat R_, T_;

    // Rectification 결과
    cv::Mat R1_, R2_, P1_, P2_, Q_;
    cv::Mat map1_left_, map2_left_;
    cv::Mat map1_right_, map2_right_;
};

#endif
```

### 구현 파일

**src/stereo_rectifier.cpp**:
```cpp
#include "../include/stereo_rectifier.hpp"
#include <iostream>

StereoRectifier::StereoRectifier(const std::string& calib_file) {
    cv::FileStorage fs(calib_file, cv::FileStorage::READ);

    if (!fs.isOpened()) {
        throw std::runtime_error("Cannot open calibration file");
    }

    fs["K_left"] >> K_left_;
    fs["dist_left"] >> dist_left_;
    fs["K_right"] >> K_right_;
    fs["dist_right"] >> dist_right_;
    fs["R"] >> R_;
    fs["T"] >> T_;

    fs.release();

    std::cout << "✅ Loaded calibration parameters" << std::endl;
}

void StereoRectifier::computeRectification(cv::Size image_size) {
    // Stereo Rectification
    cv::stereoRectify(
        K_left_, dist_left_,
        K_right_, dist_right_,
        image_size, R_, T_,
        R1_, R2_, P1_, P2_, Q_,
        cv::CALIB_ZERO_DISPARITY,  // alpha = 0
        0,                           // newImageSize = 0 (same as original)
        image_size
    );

    // Undistort and Rectify maps
    cv::initUndistortRectifyMap(
        K_left_, dist_left_, R1_, P1_, image_size,
        CV_32FC1, map1_left_, map2_left_);

    cv::initUndistortRectifyMap(
        K_right_, dist_right_, R2_, P2_, image_size,
        CV_32FC1, map1_right_, map2_right_);

    std::cout << "✅ Computed rectification maps" << std::endl;
    std::cout << "Q Matrix:\n" << Q_ << std::endl;
}

void StereoRectifier::rectify(
    const cv::Mat& left, const cv::Mat& right,
    cv::Mat& rect_left, cv::Mat& rect_right) {

    cv::remap(left, rect_left, map1_left_, map2_left_, cv::INTER_LINEAR);
    cv::remap(right, rect_right, map1_right_, map2_right_, cv::INTER_LINEAR);
}
```

---

## ✅ 체크리스트

- [ ] `StereoRectifier` 클래스 구현
- [ ] 캘리브레이션 파일 로드 확인
- [ ] Rectification 맵 계산
- [ ] Rectified 이미지에서 수평선 정렬 확인

---

## 🏗️ mini_vo 구현 (이번 주 핵심)

**구현 파일**: `Studies/Phase 2/mini_vo/src/epipolar.cpp`

### 구현해야 할 내용

| 함수 | 내용 |
|------|------|
| `computeF()` | 8-Point Algorithm 직접 구현 (Hartley 정규화 → A 행렬 → SVD → rank-2 강제 → 역정규화) |
| `computeLines()` | F Matrix로 에피폴라 선 계산 |

### 완성 기준

```bash
./mini_vo

# W5 출력 예시
# [에피폴라 검증]
#   평균 에피폴라 제약 오차: 0.003  ← 0에 가까울수록 정확
#   cv::findFundamentalMat 오차: 0.002
```

직접 구현한 F와 `cv::findFundamentalMat` 결과가 비슷하면 성공이다.

---

**다음 단계**: Week 6 - Disparity Map 생성
