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

## 📋 실습 0: FAST 코너 검출 직접 구현

> 🎯 **목표**: OpenCV의 `cv::FAST`를 사용하지 않고, 알고리즘을 직접 C++로 구현한다.
> ⏰ **예상 시간**: 2-3시간
> 📁 **파일**: `src/my_fast_detector.cpp`

### 왜 직접 구현하는가?

`cv::FAST()`를 호출하면 한 줄이면 끝이다. 하지만 직접 구현해봐야 **왜 빠른지**, **threshold가 뭘 의미하는지**, **NMS가 왜 필요한지** 몸으로 이해할 수 있다.

---

### Step 1: Bresenham 원 위의 16개 픽셀 오프셋 정의

중심 픽셀 `(x, y)` 기준으로 반지름 3인 원 위의 16개 점 좌표를 오프셋으로 미리 정의한다.

```
        1  2  3
     16         4
    15    ●     5
    14          6
    13  12 11 10 9
         8  7
```

- 1번 = `(0, -3)`, 2번 = `(1, -3)`, ... 이런 식으로 `(dx, dy)` 쌍 16개를 배열로 만들어라.
- 이 좌표들은 이론의 Bresenham 원 그림을 보고 직접 채워야 한다.
- **힌트**: 상단 중앙(1번)에서 시계방향으로 돌면서 좌표를 나열하면 된다.

---

### Step 2: 한 픽셀에 대해 코너 여부 판별

함수 `bool is_corner(const cv::Mat& gray, int x, int y, int threshold)` 를 만든다.

**로직**:
1. 중심 밝기값 `Ip = gray.at<uchar>(y, x)` 를 읽는다.
2. 16개 주변 픽셀 각각에 대해 3가지 상태를 분류한다:
   - **밝음**: 주변 픽셀 > `Ip + threshold`
   - **어두움**: 주변 픽셀 < `Ip - threshold`
   - **비슷함**: 그 외
3. 16개의 상태 배열에서 **연속 9개 이상**이 모두 "밝음"이거나 모두 "어두움"이면 코너다.

**주의할 점**:
- 원은 **순환**한다. 15번→16번→1번→2번 이렇게 이어진다. 단순 for문으로는 순환 검사가 안 되므로, 배열을 2배로 늘리거나 모듈로 연산을 활용해라.
- 이미지 경계 픽셀(반지름 3 안쪽)은 건너뛰어야 한다.

---

### Step 3: 고속 사전 검사 (High-speed test)

Step 2를 그대로 쓰면 모든 픽셀에서 16개를 전부 확인해야 한다.
대부분의 픽셀은 코너가 아니므로, **1번(상), 5번(우), 9번(하), 13번(좌)** 4개만 먼저 검사해서 빠르게 제외한다.

**규칙**: 이 4개 중 최소 3개가 "밝음"이거나, 최소 3개가 "어두움"이어야 연속 9개가 가능하다. 3개 미만이면 즉시 `false`를 반환한다.

- Step 2의 함수 앞부분에 이 사전 검사를 추가해라.
- 이것만으로 **~80% 픽셀을 즉시 제외**할 수 있어서 속도가 크게 향상된다.

---

### Step 4: 코너 점수 계산

코너로 판별된 각 픽셀에 **점수**를 매긴다. 점수는 중심 밝기와 16개 주변 픽셀 밝기 차이의 절대값 합이다.

```
score = Σ |I(주변 픽셀 i) - Ip|    (i = 1~16)
```

- 이 점수는 다음 단계(NMS)에서 "누가 더 강한 코너인가"를 비교하는 데 쓰인다.

---

### Step 5: Non-Maximum Suppression (NMS)

인접한 코너 중 가장 강한 것만 남긴다.

**로직**:
1. 코너 점수를 이미지 크기의 `cv::Mat`(float)에 저장한다 (코너 아닌 픽셀은 0).
2. 각 코너 픽셀에 대해 3×3 이웃을 확인한다.
3. 이웃 중 자신보다 점수가 높은 픽셀이 있으면 → 제거.
4. 살아남은 픽셀만 최종 코너로 출력한다.

---

### Step 6: 전체 통합 및 OpenCV 비교

1. 그레이스케일 이미지를 불러온다 (`cv::imread` + `cv::cvtColor`).
2. 모든 픽셀을 순회하며 Step 2~5를 적용해 코너를 검출한다.
3. `cv::FAST()`로도 같은 이미지에서 코너를 검출한다.
4. 두 결과를 나란히 시각화해서 비교한다.
5. 처리 시간도 비교해본다 — 직접 구현이 얼마나 느린지 체감해보라.

**검증 기준**:
- 같은 threshold에서 비슷한 위치에 코너가 검출되는가?
- 검출 개수가 비슷한 수준인가? (완전히 같을 필요는 없다)

---

### 도전 과제 (선택)

- **카메라 실시간 적용**: `cv::VideoCapture`로 웹캠 영상에 직접 구현한 FAST를 돌려보라. 몇 FPS가 나오는가?
- **threshold 슬라이더**: `cv::createTrackbar`로 threshold를 실시간 조절하며 검출 결과 변화를 관찰하라.
- **연속 N 변경**: 9 대신 12로 바꾸면 검출 개수가 어떻게 변하는가?

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

- [ ] FAST 직접 구현: Bresenham 원 16개 오프셋 정의
- [ ] FAST 직접 구현: 연속 9개 판별 + 고속 사전 검사
- [ ] FAST 직접 구현: NMS 구현 및 OpenCV 결과와 비교
- [ ] FAST 검출기 실시간 구현 (OpenCV API)
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

## 🏗️ mini_vo 구현 (이번 주 핵심)

> 이 실습의 최종 목적은 `mini_vo` 프로젝트에 FAST 검출기를 완성하는 것이다.
> 위의 실습을 통해 FAST 알고리즘을 이해했다면, 아래 파일에 직접 구현하라.

**구현 파일 1**: `Studies/Phase 2/mini_vo/src/feature_detector.cpp` — FAST

| 함수 | 내용 |
|------|------|
| `kCircle_[16][2]` | 브레젠험 원 16개 픽셀 오프셋 |
| `highSpeedTest()` | 1, 5, 9, 13번 픽셀 사전 검사 |
| `segmentTest()` | N개 연속 밝거나 어두운지 확인 |
| `cornerScore()` | 16개 픽셀 밝기 차이 합 |
| `nonMaxSuppression()` | 지역 최대값만 생존 |

**구현 파일 2**: `Studies/Phase 2/mini_vo/src/descriptor.cpp` — BRIEF

| 함수 | 내용 |
|------|------|
| `generatePairs()` | 가우시안 분포로 256개 픽셀쌍 오프셋 생성 |
| `compute()` | 각 키포인트에서 픽셀쌍 비교 → 256비트 → 32바이트 Mat |

### 완성 기준

```bash
cd Studies/Phase\ 2/mini_vo/build
./mini_vo

# FAST 완성 기준 (두 숫자가 비슷해야 성공)
# [FAST 비교]
#   직접 구현: 87개
#   cv::FAST:  92개

# BRIEF 완성 기준
# [BRIEF 디스크립터 비교]
#   직접 구현: 85개 키포인트 (32바이트/디스크립터)
#   cv::ORB:   87개 키포인트 (32바이트/디스크립터)
#   자기매칭 비영 해밍거리: 0개 (0이어야 정상)
```

---

**다음 단계**: Week 4 - 특징점 매칭 실습
