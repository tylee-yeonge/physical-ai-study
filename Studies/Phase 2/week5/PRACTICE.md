# Week 5-7: 스테레오 비전 및 깊이 추정 실습 (C++ )

> 🎯 **목표**: Stereo Rectification, Disparity, Depth Map 생성  
> 💻 **언어**: C++ (OpenCV 4.x, Eigen3)  
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo (캘리브레이션 완료)  
> ⏰ **예상 시간**: 18-24시간 (3주)

---

## 📋 전제 조건

- [ ] Week 2 캘리브레이션 완료 (`stereo_calib.yaml` 필요)
- [ ] Week 3-4 특징점 검출/매칭 이해

---

## 🔧 실습 1: Stereo Rectification

### StereoRectifier 클래스 구현

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

## 🔧 실습 2: Disparity Map 생성

### StereoMatcher 클래스

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

## 🔧 실습 3: Depth Map 계산

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

## 🔧 실습 4: 실시간 Depth Map 데모

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

### Rectification
- [ ] `StereoRectifier` 클래스 구현
- [ ] Rectified 이미지에서 수평선 정렬 확인

### Disparity/Depth
- [ ] `StereoMatcher` SGBM 구현
- [ ] 실시간 30fps 달성 (Jetson)
- [ ] 파라미터 튜닝 (numDisparities, blockSize)
- [ ] 거리 측정 정확도 검증 (실측 대비)

### 3D 점군 (선택)
- [ ] `reprojectTo3D`로 3D 좌표 생성
- [ ] Open3D로 점군 시각화

---

**다음 단계**: Week 8 - KLT Optical Flow 추적
