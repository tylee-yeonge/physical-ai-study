# Week 3: 특징점 검출 실습 (C++)

> 🎯 **목표**: FAST, ORB 특징점 검출 실시간 구현
> 💻 **언어**: C++ (OpenCV 4.x)
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo Camera 또는 MacBook (내장 카메라)
> ⏰ **예상 시간**: 6-8시간

---

## 📋 준비사항

**Jetson (Linux):**
```bash
sudo apt install libopencv-dev cmake build-essential
```

**MacBook (macOS):**
```bash
brew install opencv
# 카메라 권한: 시스템 설정 → 개인 정보 보호 및 보안 → 카메라 → 터미널 허용
```

> 코드에서 `cv::VideoCapture(0)`은 MacBook 내장 FaceTime 카메라를 자동으로 사용합니다.

---

## 📋 실습 1: FAST 코너 검출

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

---

## 📋 실습 2: ORB 특징점 + 디스크립터

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

## ✅ 체크리스트

- [ ] FAST 검출기 실시간 구현
- [ ] 파라미터 조정 (threshold, max_features)
- [ ] 30fps 달성 확인
- [ ] ORB 디스크립터 계산 구현

---

## 💡 성능 최적화 팁 (Jetson 전용)

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

**다음 단계**: Week 4 - 특징점 매칭 실습
