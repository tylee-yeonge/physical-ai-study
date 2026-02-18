# Week 8: 광류 (Optical Flow) 실습 (C++)

> 🎯 **목표**: Lucas-Kanade Optical Flow 실시간 구현 및 Dense Flow 시각화
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

## 📋 전제 조건

- [ ] Week 3-4 특징점 검출/매칭 이해
- [ ] Lucas-Kanade 원리 이해 (README.md 참조)

---

## 🔧 실습 1: KLT Feature Tracker

### KLTTracker 클래스

**include/klt_tracker.hpp**:
```cpp
#ifndef KLT_TRACKER_HPP
#define KLT_TRACKER_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>
#include <vector>

class KLTTracker {
public:
    KLTTracker(int max_corners = 200, int min_distance = 30);

    void detectFeatures(const cv::Mat& gray);

    bool track(const cv::Mat& prev_gray, const cv::Mat& curr_gray);

    void drawTracks(cv::Mat& display, int trail_length = 10);

    const std::vector<cv::Point2f>& getPrevPoints() const { return prev_pts_; }
    const std::vector<cv::Point2f>& getCurrPoints() const { return curr_pts_; }
    int getTrackedCount() const { return tracked_count_; }

private:
    int max_corners_;
    int min_distance_;
    int tracked_count_;

    std::vector<cv::Point2f> prev_pts_;
    std::vector<cv::Point2f> curr_pts_;
    std::vector<std::vector<cv::Point2f>> track_history_;

    // LK 파라미터
    cv::Size win_size_;
    int max_level_;
    cv::TermCriteria criteria_;

    void refillFeatures(const cv::Mat& gray);
};

#endif
```

### 구현 파일

**src/klt_tracker.cpp**:
```cpp
#include "../include/klt_tracker.hpp"
#include <iostream>

KLTTracker::KLTTracker(int max_corners, int min_distance)
    : max_corners_(max_corners),
      min_distance_(min_distance),
      tracked_count_(0),
      win_size_(21, 21),
      max_level_(3),
      criteria_(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 30, 0.01) {}

void KLTTracker::detectFeatures(const cv::Mat& gray) {
    prev_pts_.clear();
    track_history_.clear();

    cv::goodFeaturesToTrack(
        gray, prev_pts_,
        max_corners_,
        0.01,          // qualityLevel
        min_distance_
    );

    // 각 특징점의 이력 초기화
    track_history_.resize(prev_pts_.size());
    for (size_t i = 0; i < prev_pts_.size(); i++) {
        track_history_[i].push_back(prev_pts_[i]);
    }

    tracked_count_ = prev_pts_.size();
    std::cout << "Detected " << tracked_count_ << " features" << std::endl;
}

bool KLTTracker::track(const cv::Mat& prev_gray, const cv::Mat& curr_gray) {
    if (prev_pts_.empty()) return false;

    std::vector<uchar> status;
    std::vector<float> err;

    // Lucas-Kanade Optical Flow
    cv::calcOpticalFlowPyrLK(
        prev_gray, curr_gray,
        prev_pts_, curr_pts_,
        status, err,
        win_size_, max_level_, criteria_
    );

    // 추적 성공한 점만 유지
    std::vector<cv::Point2f> good_prev, good_curr;
    std::vector<std::vector<cv::Point2f>> good_history;

    for (size_t i = 0; i < status.size(); i++) {
        if (status[i] && err[i] < 30.0f) {
            good_prev.push_back(prev_pts_[i]);
            good_curr.push_back(curr_pts_[i]);

            if (i < track_history_.size()) {
                track_history_[i].push_back(curr_pts_[i]);
                good_history.push_back(track_history_[i]);
            }
        }
    }

    prev_pts_ = good_curr;
    curr_pts_ = good_curr;
    track_history_ = good_history;
    tracked_count_ = good_curr.size();

    // 특징점이 부족하면 보충
    if (tracked_count_ < max_corners_ / 2) {
        refillFeatures(curr_gray);
    }

    return tracked_count_ > 0;
}

void KLTTracker::refillFeatures(const cv::Mat& gray) {
    // 마스크: 기존 특징점 주변 제외
    cv::Mat mask = cv::Mat::ones(gray.size(), CV_8UC1) * 255;
    for (const auto& pt : prev_pts_) {
        cv::circle(mask, pt, min_distance_, 0, -1);
    }

    std::vector<cv::Point2f> new_pts;
    cv::goodFeaturesToTrack(
        gray, new_pts,
        max_corners_ - tracked_count_,
        0.01, min_distance_, mask
    );

    for (const auto& pt : new_pts) {
        prev_pts_.push_back(pt);
        track_history_.push_back({pt});
    }

    tracked_count_ = prev_pts_.size();
}

void KLTTracker::drawTracks(cv::Mat& display, int trail_length) {
    for (size_t i = 0; i < track_history_.size(); i++) {
        const auto& history = track_history_[i];

        // 최근 trail_length개 궤적 그리기
        int start = std::max(0, (int)history.size() - trail_length);
        for (int j = start; j < (int)history.size() - 1; j++) {
            // 색상: 오래된 것 → 빨강, 최근 → 초록
            float ratio = (float)(j - start) / std::max(1, (int)history.size() - start - 1);
            cv::Scalar color(0, 255 * ratio, 255 * (1 - ratio));
            cv::line(display, history[j], history[j + 1], color, 2);
        }

        // 현재 위치 표시
        if (!history.empty()) {
            cv::circle(display, history.back(), 4, cv::Scalar(0, 255, 0), -1);
        }
    }
}
```

---

## 🔧 실습 2: 실시간 KLT 추적 데모

**src/optical_flow_demo.cpp**:
```cpp
#include "../include/klt_tracker.hpp"
#include <chrono>

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera" << std::endl;
        return -1;
    }

    KLTTracker tracker(200, 30);

    cv::Mat frame, gray, prev_gray;
    bool initialized = false;

    std::cout << "\n🎥 KLT Optical Flow 추적 시작" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "R: 특징점 재검출" << std::endl;
    std::cout << "ESC: 종료\n" << std::endl;

    while (true) {
        auto start = std::chrono::high_resolution_clock::now();

        cap >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        if (!initialized) {
            tracker.detectFeatures(gray);
            initialized = true;
        } else {
            tracker.track(prev_gray, gray);
        }

        // 시각화
        cv::Mat display = frame.clone();
        tracker.drawTracks(display, 15);

        auto end = std::chrono::high_resolution_clock::now();
        double fps = 1000.0 / std::chrono::duration<double, std::milli>(end - start).count();

        cv::putText(display,
                   cv::format("FPS: %.1f | Tracked: %d", fps, tracker.getTrackedCount()),
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                   0.8, cv::Scalar(0, 255, 0), 2);

        cv::imshow("KLT Optical Flow", display);

        int key = cv::waitKey(1);
        if (key == 27) break;
        if (key == 'r') {
            tracker.detectFeatures(gray);
            std::cout << "Re-detected features" << std::endl;
        }

        prev_gray = gray.clone();
    }

    return 0;
}
```

---

## 🔧 실습 3: Dense Optical Flow 시각화

**src/dense_flow_demo.cpp**:
```cpp
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <chrono>

void drawFlowField(const cv::Mat& flow, cv::Mat& display, int step = 16) {
    for (int y = 0; y < flow.rows; y += step) {
        for (int x = 0; x < flow.cols; x += step) {
            cv::Point2f fxy = flow.at<cv::Point2f>(y, x);

            float magnitude = cv::sqrt(fxy.x * fxy.x + fxy.y * fxy.y);
            if (magnitude > 1.0f) {
                cv::arrowedLine(display,
                               cv::Point(x, y),
                               cv::Point(cvRound(x + fxy.x), cvRound(y + fxy.y)),
                               cv::Scalar(0, 255, 0), 1, cv::LINE_AA, 0, 0.3);
            }
        }
    }
}

void flowToHSV(const cv::Mat& flow, cv::Mat& hsv_display) {
    cv::Mat flow_parts[2];
    cv::split(flow, flow_parts);

    cv::Mat magnitude, angle;
    cv::cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);

    cv::Mat hsv_channels[3];
    hsv_channels[0] = angle;                    // Hue: 방향
    hsv_channels[1] = cv::Mat::ones(flow.size(), CV_32F) * 255;  // Saturation
    cv::normalize(magnitude, hsv_channels[2], 0, 255, cv::NORM_MINMAX);  // Value: 크기

    cv::Mat hsv;
    cv::merge(hsv_channels, 3, hsv);
    hsv.convertTo(hsv, CV_8U);
    cv::cvtColor(hsv, hsv_display, cv::COLOR_HSV2BGR);
}

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) return -1;

    cv::Mat frame, gray, prev_gray;
    cap >> frame;
    cv::cvtColor(frame, prev_gray, cv::COLOR_BGR2GRAY);

    std::cout << "\n🎥 Dense Optical Flow 시작" << std::endl;
    std::cout << "1: Arrow 시각화 / 2: HSV 시각화" << std::endl;
    std::cout << "ESC: 종료\n" << std::endl;

    int vis_mode = 1;

    while (true) {
        auto start = std::chrono::high_resolution_clock::now();

        cap >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Farneback Dense Flow
        cv::Mat flow;
        cv::calcOpticalFlowFarneback(
            prev_gray, gray, flow,
            0.5, 3, 15, 3, 5, 1.2, 0
        );

        cv::Mat display = frame.clone();

        if (vis_mode == 1) {
            drawFlowField(flow, display);
        } else {
            flowToHSV(flow, display);
        }

        auto end = std::chrono::high_resolution_clock::now();
        double fps = 1000.0 / std::chrono::duration<double, std::milli>(end - start).count();

        cv::putText(display, cv::format("Dense Flow FPS: %.1f", fps),
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                   0.8, cv::Scalar(0, 255, 0), 2);

        cv::imshow("Dense Optical Flow", display);

        int key = cv::waitKey(1);
        if (key == 27) break;
        if (key == '1') vis_mode = 1;
        if (key == '2') vis_mode = 2;

        prev_gray = gray.clone();
    }

    return 0;
}
```

---

## ✅ 체크리스트

### Sparse Flow (KLT)
- [ ] `KLTTracker` 클래스 구현
- [ ] 특징점 검출 + LK 추적
- [ ] 추적 실패 감지 및 재검출
- [ ] 궤적 시각화 (trail)
- [ ] 실시간 30fps 달성

### Dense Flow
- [ ] Farneback Dense Flow 구현
- [ ] Arrow 시각화
- [ ] HSV 색상 시각화 (방향 → 색상, 크기 → 밝기)

### VINS 연계
- [ ] VINS feature_tracker 코드 구조 이해
- [ ] `goodFeaturesToTrack` + `calcOpticalFlowPyrLK` 패턴 이해
- [ ] Fundamental Matrix로 outlier 제거 패턴 이해

---

## 💡 VINS-Fusion feature_tracker 비교

```cpp
// VINS-Fusion 패턴 (참고)
// feature_tracker/src/feature_tracker.cpp

void FeatureTracker::trackImage(...) {
    // 1. LK로 추적
    cv::calcOpticalFlowPyrLK(prev_img, cur_img,
        prev_pts, cur_pts, status, err,
        cv::Size(21, 21), 3);

    // 2. Fundamental Matrix로 outlier 제거
    cv::findFundamentalMat(prev_pts, cur_pts,
        cv::FM_RANSAC, F_THRESHOLD, 0.99, status);

    // 3. 부족한 특징점 보충
    if (pts.size() < MAX_CNT)
        cv::goodFeaturesToTrack(cur_img, new_pts, ...);
}
```

---

**Phase 2 완료! 다음: Phase 3 - Visual Odometry & Bundle Adjustment**
