# Week 7: Depth Map 계산 및 실시간 데모 (C++)

> 🎯 **목표**: Disparity → Depth 변환 및 실시간 깊이 추정 시스템 구현
> 💻 **언어**: C++ (OpenCV 4.x)
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo (캘리브레이션 완료)
> ⏰ **예상 시간**: 6-8시간

---

## 📋 전제 조건

- [ ] Week 5 Stereo Rectification 완료
- [ ] Week 6 Disparity Map 생성 완료

---

## 🔧 실습 1: DepthEstimator 클래스 구현

### 헤더 파일

**include/depth_estimator.hpp**:
```cpp
#ifndef DEPTH_ESTIMATOR_HPP
#define DEPTH_ESTIMATOR_HPP

#include <opencv2/opencv.hpp>

class DepthEstimator {
public:
    DepthEstimator(float baseline, float focal_length);

    void computeDepth(const cv::Mat& disparity, cv::Mat& depth);

    void reprojectTo3D(const cv::Mat& disparity,
                      const cv::Mat& Q,
                      cv::Mat& points3D);

    float getDepthAt(const cv::Mat& depth, int x, int y);

private:
    float baseline_;       // mm
    float focal_length_;   // pixels
};

#endif
```

### 구현 파일

**src/depth_estimator.cpp**:
```cpp
#include "../include/depth_estimator.hpp"
#include <iostream>

DepthEstimator::DepthEstimator(float baseline, float focal_length)
    : baseline_(baseline), focal_length_(focal_length) {}

void DepthEstimator::computeDepth(const cv::Mat& disparity, cv::Mat& depth) {
    depth = cv::Mat::zeros(disparity.size(), CV_32F);

    for (int i = 0; i < disparity.rows; i++) {
        for (int j = 0; j < disparity.cols; j++) {
            float disp = disparity.at<float>(i, j);

            if (disp > 0) {
                // depth = (baseline * focal_length) / disparity
                depth.at<float>(i, j) = (baseline_ * focal_length_) / disp;
            }
        }
    }
}

void DepthEstimator::reprojectTo3D(
    const cv::Mat& disparity,
    const cv::Mat& Q,
    cv::Mat& points3D) {

    cv::reprojectImageTo3D(disparity, points3D, Q, true);
}

float DepthEstimator::getDepthAt(const cv::Mat& depth, int x, int y) {
    if (x < 0 || x >= depth.cols || y < 0 || y >= depth.rows) {
        return -1.0f;
    }

    return depth.at<float>(y, x);
}
```

---

## 🔧 실습 2: 실시간 Depth Map 데모

**src/realtime_depth.cpp**:
```cpp
#include "../include/stereo_rectifier.hpp"
#include "../include/stereo_matcher.hpp"
#include "../include/depth_estimator.hpp"
#include <opencv2/videoio.hpp>
#include <iostream>
#include <chrono>

int main() {
    // 캘리브레이션 로드
    StereoRectifier rectifier("stereo_calib.yaml");
    rectifier.computeRectification(cv::Size(800, 600));

    // Baseline과 focal length 추출
    cv::FileStorage fs("stereo_calib.yaml", cv::FileStorage::READ);
    float baseline;
    fs["baseline"] >> baseline;

    cv::Mat K_left;
    fs["K_left"] >> K_left;
    float focal_length = K_left.at<double>(0, 0);
    fs.release();

    StereoMatcher matcher;
    DepthEstimator depth_est(baseline, focal_length);

    // 카메라 열기
    cv::VideoCapture cap_left(0), cap_right(1);

    cv::Mat frame_left, frame_right;
    cv::Mat rect_left, rect_right;
    cv::Mat disparity, depth;

    // 마우스 콜백용 전역 변수
    cv::Point mouse_pt(-1, -1);

    auto mouse_callback = [](int event, int x, int y, int, void* userdata) {
        if (event == cv::EVENT_MOUSEMOVE) {
            cv::Point* pt = static_cast<cv::Point*>(userdata);
            pt->x = x;
            pt->y = y;
        }
    };

    cv::namedWindow("Depth Map");
    cv::setMouseCallback("Depth Map", mouse_callback, &mouse_pt);

    std::cout << "\n🎥 실시간 Depth 추정 시작" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "마우스를 Depth Map 위로 이동하면 거리 표시" << std::endl;
    std::cout << "Q/W: NumDisparities 조정" << std::endl;
    std::cout << "A/S: BlockSize 조정" << std::endl;
    std::cout << "ESC: 종료\n" << std::endl;

    int numDisp = 64, blockSize = 5;

    while (true) {
        auto start = std::chrono::high_resolution_clock::now();

        cap_left >> frame_left;
        cap_right >> frame_right;

        if (frame_left.empty()) break;

        // Rectify
        rectifier.rectify(frame_left, frame_right, rect_left, rect_right);

        // Disparity
        matcher.computeDisparity(rect_left, rect_right, disparity);

        // Depth
        depth_est.computeDepth(disparity, depth);

        // 시각화
        cv::Mat disp_vis, depth_vis;
        matcher.visualizeDisparity(disparity, disp_vis);

        // Depth는 거리에 따라 색상 매핑 (0-5000mm)
        cv::Mat depth_normalized;
        cv::normalize(depth, depth_normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
        cv::applyColorMap(depth_normalized, depth_vis, cv::COLORMAP_TURBO);

        // 마우스 위치의 거리 표시
        if (mouse_pt.x >= 0 && mouse_pt.x < depth.cols &&
            mouse_pt.y >= 0 && mouse_pt.y < depth.rows) {

            float distance = depth.at<float>(mouse_pt.y, mouse_pt.x);

            if (distance > 0) {
                cv::circle(depth_vis, mouse_pt, 5, cv::Scalar(0, 255, 255), 2);

                std::string dist_text = cv::format("%.1f mm (%.2f m)",
                                                   distance, distance / 1000.0);
                cv::putText(depth_vis, dist_text,
                           cv::Point(mouse_pt.x + 10, mouse_pt.y - 10),
                           cv::FONT_HERSHEY_SIMPLEX, 0.6,
                           cv::Scalar(0, 255, 255), 2);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        double fps = 1000.0 / std::chrono::duration<double, std::milli>(end - start).count();

        cv::putText(depth_vis, cv::format("FPS: %.1f", fps),
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                   0.8, cv::Scalar(0, 255, 0), 2);

        cv::putText(depth_vis,
                   cv::format("NumDisp: %d | BlockSize: %d", numDisp, blockSize),
                   cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX,
                   0.6, cv::Scalar(255, 255, 255), 2);

        cv::imshow("Rectified Left", rect_left);
        cv::imshow("Disparity", disp_vis);
        cv::imshow("Depth Map", depth_vis);

        int key = cv::waitKey(1);
        if (key == 27) break;  // ESC

        // 파라미터 조정
        if (key == 'q' && numDisp > 16) {
            numDisp -= 16;
            matcher.setParameters(numDisp, blockSize);
        }
        if (key == 'w' && numDisp < 256) {
            numDisp += 16;
            matcher.setParameters(numDisp, blockSize);
        }
        if (key == 'a' && blockSize > 3) {
            blockSize -= 2;
            matcher.setParameters(numDisp, blockSize);
        }
        if (key == 's' && blockSize < 21) {
            blockSize += 2;
            matcher.setParameters(numDisp, blockSize);
        }
    }

    return 0;
}
```

---

## ✅ 체크리스트

### Depth Map
- [ ] `DepthEstimator` 클래스 구현
- [ ] depth = (baseline * focal_length) / disparity 공식 이해
- [ ] 거리 측정 정확도 검증 (실측 대비)

### 실시간 데모
- [ ] Rectification + Disparity + Depth 통합 파이프라인
- [ ] 실시간 30fps 달성 (Jetson)
- [ ] 마우스 클릭 거리 표시
- [ ] 파라미터 인터랙티브 조정

### 3D 점군 (선택)
- [ ] `reprojectTo3D`로 3D 좌표 생성
- [ ] Open3D로 점군 시각화

---

## 🏗️ mini_vo 구현 (이번 주 핵심)

**구현 파일**: `Studies/Phase 2/mini_vo/src/triangulator.cpp`

### 구현해야 할 내용

| 함수 | 내용 |
|------|------|
| `triangulate()` | DLT 직접 구현: 크로스곱 → 4×4 A 행렬 구성 → SVD → 동차→유클리드 변환 |

### 완성 기준

```bash
./mini_vo

# W7 출력 예시
# [삼각측량]
#   복원된 3D 점: 142개
#   평균 재투영 오차: 0.8 픽셀  ← 1.0 이하면 양호
#   cv::triangulatePoints 오차: 0.7 픽셀
```

재투영 오차가 `cv::triangulatePoints`와 비슷하면 성공이다.

---

**다음 단계**: Week 8 - KLT Optical Flow 추적
