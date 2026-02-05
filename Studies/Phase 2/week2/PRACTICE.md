# Week 2: 카메라 캘리브레이션 실습 (C++)

> 🎯 **목표**: ELP 스테레오 카메라의 내부/외부 파라미터를 C++로 측정  
> 💻 **언어**: C++ (OpenCV 4.x)  
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP 800P Stereo Monochrome  
> ⏰ **예상 시간**: 6-8시간

---

## 📋 준비사항

### 하드웨어
- [ ] Jetson Orin Nano 세팅 완료
- [ ] ELP 스테레오 카메라 연결 (USB)
- [ ] 체커보드 패턴 출력 (8×6, 30mm 정사각형, A3 용지 권장)

### 소프트웨어
```bash
# OpenCV 4.x 확인
pkg-config --modversion opencv4

# Eigen3 설치
sudo apt install libeigen3-dev

# v4l2 (카메라 인터페이스)
sudo apt install v4l-utils libv4l-dev

# 빌드 도구
sudo apt install cmake build-essential
```

---

## 📂 프로젝트 구조

```bash
cd ~/workspace
mkdir -p stereo_calib_practice
cd stereo_calib_practice
```

```
stereo_calib_practice/
├── CMakeLists.txt
├── include/
│   └── camera_calibration.hpp
├── src/
│   ├── camera_calibration.cpp
│   ├── mono_calib.cpp
│   └── stereo_calib.cpp
└── data/
    ├── left/
    ├── right/
    └── calib_results/
```

---

## 🔧 실습 1: 단일 카메라 캘리브레이션

### Step 1: 헤더 파일 작성

**include/camera_calibration.hpp**:
```cpp
#ifndef CAMERA_CALIBRATION_HPP
#define CAMERA_CALIBRATION_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class CameraCalibration {
public:
    CameraCalibration(cv::Size boardSize, float squareSize);
    
    bool findChessboardCorners(const cv::Mat& image, 
                               std::vector<cv::Point2f>& corners);
    
    double calibrate(const std::vector<std::vector<cv::Point2f>>& imagePoints,
                     cv::Size imageSize,
                     cv::Mat& cameraMatrix,
                     cv::Mat& distCoeffs);
    
    void saveCalibration(const std::string& filename,
                        const cv::Mat& K,
                        const cv::Mat& dist);
    
private:
    cv::Size boardSize_;
    float squareSize_;
    std::vector<std::vector<cv::Point3f>> objectPoints_;
    
    void generateObjectPoints();
};

#endif
```

### Step 2: 구현 파일 작성

**src/camera_calibration.cpp**:
```cpp
#include "../include/camera_calibration.hpp"
#include <opencv2/calib3d.hpp>
#include <iostream>

CameraCalibration::CameraCalibration(cv::Size boardSize, float squareSize)
    : boardSize_(boardSize), squareSize_(squareSize) {}

bool CameraCalibration::findChessboardCorners(
    const cv::Mat& image, 
    std::vector<cv::Point2f>& corners) {
    
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    bool found = cv::findChessboardCorners(
        gray, boardSize_, corners,
        cv::CALIB_CB_ADAPTIVE_THRESH | 
        cv::CALIB_CB_NORMALIZE_IMAGE |
        cv::CALIB_CB_FAST_CHECK);
    
    if (found) {
        cv::cornerSubPix(
            gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
            cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 
                           30, 0.1));
    }
    
    return found;
}

void CameraCalibration::generateObjectPoints() {
    std::vector<cv::Point3f> obj;
    for (int i = 0; i < boardSize_.height; i++) {
        for (int j = 0; j < boardSize_.width; j++) {
            obj.push_back(cv::Point3f(j * squareSize_, i * squareSize_, 0));
        }
    }
    objectPoints_.clear();
    objectPoints_.push_back(obj);
}

double CameraCalibration::calibrate(
    const std::vector<std::vector<cv::Point2f>>& imagePoints,
    cv::Size imageSize,
    cv::Mat& cameraMatrix,
    cv::Mat& distCoeffs) {
    
    generateObjectPoints();
    
    // 모든 프레임에 동일한 objectPoints 적용
    std::vector<std::vector<cv::Point3f>> allObjectPoints;
    for (size_t i = 0; i < imagePoints.size(); i++) {
        allObjectPoints.push_back(objectPoints_[0]);
    }
    
    std::vector<cv::Mat> rvecs, tvecs;
    double rms = cv::calibrateCamera(
        allObjectPoints, imagePoints, imageSize,
        cameraMatrix, distCoeffs, rvecs, tvecs);
    
    std::cout << "✅ RMS re-projection error: " << rms << " pixels" << std::endl;
    
    return rms;
}

void CameraCalibration::saveCalibration(
    const std::string& filename,
    const cv::Mat& K,
    const cv::Mat& dist) {
    
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);
    fs << "camera_matrix" << K;
    fs << "distortion_coefficients" << dist;
    fs << "image_width" << 800;
    fs << "image_height" << 600;
    fs.release();
    
    std::cout << "💾 Saved calibration to " << filename << std::endl;
}
```

### Step 3: 메인 프로그램 작성

**src/mono_calib.cpp**:
```cpp
#include "../include/camera_calibration.hpp"
#include <opencv2/videoio.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <camera_id>" << std::endl;
        std::cout << "Example: " << argv[0] << " 0" << std::endl;
        return -1;
    }
    
    int camera_id = std::stoi(argv[1]);
    
    // 체커보드 설정 (8x6, 30mm)
    CameraCalibration calib(cv::Size(8, 6), 30.0);
    
    cv::VideoCapture cap(camera_id);
    if (!cap.isOpened()) {
        std::cerr << "❌ Cannot open camera " << camera_id << std::endl;
        return -1;
    }
    
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 800);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 600);
    
    std::vector<std::vector<cv::Point2f>> imagePoints;
    cv::Mat frame;
    int capturedFrames = 0;
    const int targetFrames = 20;
    
    std::cout << "\n📸 카메라 캘리브레이션 시작" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "SPACE: 캡처 (" << targetFrames << "장 필요)" << std::endl;
    std::cout << "ESC: 캡처 종료" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    while (capturedFrames < targetFrames) {
        cap >> frame;
        if (frame.empty()) break;
        
        cv::Mat display = frame.clone();
        std::vector<cv::Point2f> corners;
        
        bool found = calib.findChessboardCorners(frame, corners);
        
        if (found) {
            cv::drawChessboardCorners(display, cv::Size(8, 6), corners, found);
            cv::putText(display, "Chessboard Found! (Press SPACE)", 
                       cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 
                       0.7, cv::Scalar(0, 255, 0), 2);
        } else {
            cv::putText(display, "Searching for chessboard...", 
                       cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 
                       0.7, cv::Scalar(0, 0, 255), 2);
        }
        
        cv::putText(display, 
                   "Captured: " + std::to_string(capturedFrames) + "/" + 
                   std::to_string(targetFrames),
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 
                   1, cv::Scalar(255, 255, 0), 2);
        
        cv::imshow("Calibration", display);
        
        int key = cv::waitKey(30);
        if (key == 27) break;  // ESC
        if (key == 32 && found) {  // SPACE
            imagePoints.push_back(corners);
            capturedFrames++;
            std::cout << "📷 Frame " << capturedFrames << " captured" << std::endl;
        }
    }
    
    cv::destroyAllWindows();
    
    if (capturedFrames < 10) {
        std::cerr << "❌ Not enough frames! (minimum 10)" << std::endl;
        return -1;
    }
    
    std::cout << "\n⚙️  캘리브레이션 수행 중..." << std::endl;
    
    cv::Mat K, dist;
    double rms = calib.calibrate(imagePoints, cv::Size(800, 600), K, dist);
    
    std::cout << "\n📊 결과:" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Camera Matrix K:\n" << K << std::endl;
    std::cout << "\nDistortion Coefficients:\n" << dist << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::string filename = (camera_id == 0) ? 
        "camera_left_calib.yaml" : "camera_right_calib.yaml";
    calib.saveCalibration(filename, K, dist);
    
    if (rms < 0.5) {
        std::cout << "✅ 우수한 캘리브레이션! (RMS < 0.5)" << std::endl;
    } else if (rms < 1.0) {
        std::cout << "⚠️  양호한 캘리브레이션 (RMS < 1.0)" << std::endl;
    } else {
        std::cout << "❌ 재캘리브레이션 권장 (RMS >= 1.0)" << std::endl;
    }
    
    return 0;
}
```

### Step 4: CMakeLists.txt 작성

```cmake
cmake_minimum_required(VERSION 3.10)
project(stereo_calib_practice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -Wall")

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(include)

# 단일 카메라 캘리브레이션
add_executable(mono_calib 
    src/camera_calibration.cpp
    src/mono_calib.cpp
)
target_link_libraries(mono_calib ${OpenCV_LIBS})

# 스테레오 캘리브레이션 (다음 실습)
# add_executable(stereo_calib 
#     src/camera_calibration.cpp
#     src/stereo_calib.cpp
# )
# target_link_libraries(stereo_calib ${OpenCV_LIBS})
```

### Step 5: 빌드 및 실행

```bash
# 빌드
mkdir build && cd build
cmake ..
make -j4

# Left 카메라 캘리브레이션
./mono_calib 0

# Right 카메라 캘리브레이션
./mono_calib 1
```

---

## 🔧 실습 2: 스테레오 캘리브레이션

### 캘리브레이션 결과 검증

```bash
# Left 카메라 결과 확인
cat camera_left_calib.yaml

# Right 카메라 결과 확인
cat camera_right_calib.yaml
```

**기대 결과**:
```yaml
camera_matrix: !!opencv-matrix
   rows: 3
   cols: 3
   data: [ 6.17306e+02, 0., 3.20885e+02,
           0., 6.17306e+02, 2.44357e+02,
           0., 0., 1. ]
distortion_coefficients: !!opencv-matrix
   rows: 1
   cols: 5
   data: [ -0.283408, 0.073959, 0.000194, 1.76e-05, 0. ]
```

---

## ✅ 체크리스트

### 환경 세팅
- [ ] OpenCV 4.x 설치 확인
- [ ] 체커보드 패턴 출력
- [ ] 카메라 연결 확인 (`v4l2-ctl --list-devices`)

### Left 카메라 캘리브레이션
- [ ] 20장 이상 체커보드 이미지 캡처
- [ ] RMS < 0.5 픽셀 달성
- [ ] `camera_left_calib.yaml` 저장

### Right 카메라 캘리브레이션
- [ ] 20장 이상 체커보드 이미지 캡처
- [ ] RMS < 0.5 픽셀 달성
- [ ] `camera_right_calib.yaml` 저장

### 결과 분석
- [ ] fx, fy 값이 비슷한지 확인 (±5% 이내)
- [ ] cx, cy가 이미지 중심 근처인지 확인
- [ ] 왜곡 계수 (k1, k2) 값 확인

---

## 💡 팁

### 좋은 캘리브레이션을 위한 팁
1. **다양한 각도**: 체커보드를 여러 각도에서 촬영
2. **거리 변화**: 가까이, 멀리, 중간 거리 모두 포함
3. **전체 영역**: 이미지의 모든 영역에 체커보드가 나오도록
4. **선명도**: 흔들림 없이 선명한 이미지만 사용

### 문제 해결
- **체커보드 검출 실패**: 조명 개선, 체커보드 크기 확인
- **RMS 큼 (>1.0)**: 더 많은 이미지 캡처, 흔들린 이미지 제거
- **카메라 열리지 않음**: `ls /dev/video*`로 장치 확인

---

## 📚 참고 자료

- [OpenCV Camera Calibration Tutorial](https://docs.opencv.org/4.x/dc/dbb/tutorial_py_calibration.html)
- [Pinhole Camera Model (README.md)](../week1/README.md)
- Phase 2 Week 1: 핀홀 카메라 모델 이론

---

**다음 단계**: Week 3 - 특징점 검출과 매칭 (FAST, ORB)
