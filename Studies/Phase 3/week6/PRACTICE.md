# Week 6 실습: Keyframe Management 구현

> 🎯 **목표**: Keyframe 선택/제거 전략 구현  
> 💻 **언어**: C++ (OpenCV, Eigen)  
> ⏰ **예상 시간**: 10시간

---

## 📋 실습 개요

Week 6는 **Keyframe 관리**를 구현합니다. 모든 프레임을 저장하면 메모리가 부족하므로, 중요한 프레임만 선택하고 중복된 Keyframe은 제거합니다.

---

## 🔧 프로젝트 구조

```
week6_keyframe/
├── CMakeLists.txt
├── include/
│   ├── types.hpp           # Week 1
│   └── keyframe_manager.hpp
└── src/
    ├── keyframe_manager.cpp
    └── main.cpp
```

---

## Step 1: Keyframe 클래스

### include/keyframe_manager.hpp

```cpp
#ifndef KEYFRAME_MANAGER_HPP
#define KEYFRAME_MANAGER_HPP

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <vector>
#include <map>

namespace vo {

/**
 * @brief Keyframe 클래스
 */
class Keyframe {
public:
    int id;
    cv::Mat image;
    
    // 포즈
    Eigen::Matrix3d R;
    Eigen::Vector3d t;
    
    // 특징점
    std::vector<cv::Point2f> keypoints;
    std::vector<int> map_point_indices;
    
    // Covisibility
    std::map<Keyframe*, int> covisible_keyframes;
    
    Keyframe(int id, const cv::Mat& img) 
        : id(id), image(img.clone()) {
        R = Eigen::Matrix3d::Identity();
        t = Eigen::Vector3d::Zero();
    }
    
    void addCovisibility(Keyframe* other, int num_shared) {
        covisible_keyframes[other] = num_shared;
    }
};

/**
 * @brief Keyframe 선택기 (ORB-SLAM 방식)
 */
class KeyframeSelector {
public:
    KeyframeSelector(int min_frames = 20,
                    int max_frames = 30,
                    double min_tracked_ratio = 0.5,
                    double min_parallax = 10.0);
    
    /**
     * @brief 새 Keyframe 필요 여부
     */
    bool needNewKeyframe(
        int num_frames_since_last,
        int num_tracked,
        int total_map_points,
        double avg_parallax
    );
    
private:
    int min_frames_;
    int max_frames_;
    double min_tracked_ratio_;
    double min_parallax_;
};

/**
 * @brief Keyframe Culler (90% rule)
 */
class KeyframeCuller {
public:
    KeyframeCuller(double redundancy_threshold = 0.9);
    
    /**
     * @brief 중복 Keyframe 제거
     */
    void cullRedundantKeyframes(
        std::vector<Keyframe*>& keyframes,
        const std::vector<int>& map_point_obs
    );
    
private:
    double redundancy_threshold_;
    
    bool isRedundant(
        const Keyframe* kf,
        const std::vector<int>& map_point_obs
    );
};

} // namespace vo

#endif
```

---

## Step 2: KeyframeSelector 구현

### src/keyframe_manager.cpp

```cpp
#include "keyframe_manager.hpp"
#include <iostream>

namespace vo {

KeyframeSelector::KeyframeSelector(
    int min_frames, int max_frames,
    double min_tracked_ratio, double min_parallax
) : min_frames_(min_frames),
    max_frames_(max_frames),
    min_tracked_ratio_(min_tracked_ratio),
    min_parallax_(min_parallax) {}

bool KeyframeSelector::needNewKeyframe(
    int num_frames_since_last,
    int num_tracked,
    int total_map_points,
    double avg_parallax
) {
    // 기본 조건
    if (num_frames_since_last < min_frames_) {
        return false;
    }
    
    // 조건 A: 최대 간격
    if (num_frames_since_last >= max_frames_) {
        std::cout << "  → Keyframe: 최대 간격 (" 
                  << num_frames_since_last << " frames)" << std::endl;
        return true;
    }
    
    // 조건 B: 추적 품질
    if (total_map_points > 0) {
        double tracked_ratio = (double)num_tracked / total_map_points;
        if (tracked_ratio < min_tracked_ratio_) {
            std::cout << "  → Keyframe: 추적 품질 저하 (" 
                      << tracked_ratio * 100 << "%)" << std::endl;
            return true;
        }
    }
    
    // 조건 C: 시차
    if (avg_parallax > min_parallax_) {
        std::cout << "  → Keyframe: 시차 충분 (" 
                  << avg_parallax << " px)" << std::endl;
        return true;
    }
    
    return false;
}

KeyframeCuller::KeyframeCuller(double redundancy_threshold)
    : redundancy_threshold_(redundancy_threshold) {}

void KeyframeCuller::cullRedundantKeyframes(
    std::vector<Keyframe*>& keyframes,
    const std::vector<int>& map_point_obs
) {
    int num_culled = 0;
    
    for (int i = keyframes.size()-1; i > 0; i--) {
        Keyframe* kf = keyframes[i];
        
        if (isRedundant(kf, map_point_obs)) {
            // Covisibility 업데이트
            for (auto& [other, _] : kf->covisible_keyframes) {
                other->covisible_keyframes.erase(kf);
            }
            
            delete kf;
            keyframes.erase(keyframes.begin() + i);
            num_culled++;
        }
    }
    
    if (num_culled > 0) {
        std::cout << "  Culled " << num_culled 
                  << " redundant keyframes" << std::endl;
    }
}

bool KeyframeCuller::isRedundant(
    const Keyframe* kf,
    const std::vector<int>& map_point_obs
) {
    int redundant_obs = 0;
    int total_obs = 0;
    
    for (int mp_idx : kf->map_point_indices) {
        if (mp_idx < 0 || mp_idx >= map_point_obs.size()) continue;
        
        total_obs++;
        
        // 3개 이상 다른 Keyframe에서 관측
        if (map_point_obs[mp_idx] >= 3) {
            redundant_obs++;
        }
    }
    
    if (total_obs == 0) return false;
    
    double redundancy = (double)redundant_obs / total_obs;
    return redundancy > redundancy_threshold_;
}

} // namespace vo
```

---

## Step 3: 테스트

### src/main.cpp

```cpp
#include "keyframe_manager.hpp"
#include <iostream>

using namespace vo;

void simulateKeyframeManagement() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Keyframe Management 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    KeyframeSelector selector(20, 30, 0.5, 10.0);
    KeyframeCuller culler(0.9);
    
    std::vector<Keyframe*> keyframes;
    int last_kf_frame = 0;
    
    for (int frame = 0; frame < 200; frame++) {
        int frames_since_last = frame - last_kf_frame;
        
        // 시뮬레이션 데이터
        int num_tracked = 80 + rand() % 40;
        int total_map_points = 150;
        double avg_parallax = frames_since_last * 0.5;
        
        bool need_kf = selector.needNewKeyframe(
            frames_since_last, num_tracked, 
            total_map_points, avg_parallax);
        
        if (need_kf) {
            cv::Mat dummy_img = cv::Mat::zeros(480, 640, CV_8UC1);
            Keyframe* kf = new Keyframe(frame, dummy_img);
            
            // 임의 맵 포인트
            for (int i = 0; i < 30; i++) {
                kf->map_point_indices.push_back(rand() % 100);
            }
            
            keyframes.push_back(kf);
            last_kf_frame = frame;
            
            std::cout << "Frame " << frame << ": NEW KEYFRAME" << std::endl;
            std::cout << "  총 Keyframes: " << keyframes.size() << std::endl;
        }
        
        // 주기적 Culling
        if (frame % 50 == 0 && frame > 0) {
            std::vector<int> dummy_obs(100, 2); // 각 점 2번 관측
            culler.cullRedundantKeyframes(keyframes, dummy_obs);
            std::cout << "  남은 Keyframes: " << keyframes.size() << "\n" << std::endl;
        }
    }
    
    // 정리
    for (auto* kf : keyframes) {
        delete kf;
    }
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 시뮬레이션 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 6: Keyframe Management" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    simulateKeyframeManagement();
    
    std::cout << "\n💡 핵심 내용:" << std::endl;
    std::cout << "   - Keyframe 선택 (시차, 품질, 간격)" << std::endl;
    std::cout << "   - Culling (90% redundancy)" << std::endl;
    std::cout << "   - 맵 크기 관리\n" << std::endl;
    
    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week6_Keyframe)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(keyframe_demo
    src/main.cpp
    src/keyframe_manager.cpp
)

target_link_libraries(keyframe_demo ${OpenCV_LIBS})
```

---

## 빌드 및 실행

```bash
cd week6_keyframe
mkdir build && cd build
cmake ..
make
./keyframe_demo
```

---

## ✅ 체크리스트

- [ ] Keyframe 클래스 구현
- [ ] KeyframeSelector 구현
- [ ] ORB-SLAM 선택 기준 적용
- [ ] KeyframeCuller 구현
- [ ] 90% rule 구현
- [ ] 시뮬레이션 성공

---

## 💡 핵심 개념

1. **선택 기준 (ORB-SLAM)**
   - 기본: 20 프레임 경과
   - 조건 A: 30 프레임
   - 조건 B: 추적 < 50%
   - 조건 C: 시차 > 10px

2. **Culling (90% rule)**
   - 맵 포인트의 90% 이상이
   - 다른 3개 이상 KF에서 관측
   - → 중복, 제거!

3. **Covisibility Graph**
   - 공유 맵 포인트 관리
   - Local BA 범위 결정

---

## 🔗 Week 5 VO에 통합

Week 5의 `VisualOdometry` 클래스에 Keyframe 관리를 추가할 수 있습니다:

```cpp
// VisualOdometry 클래스에 추가
KeyframeSelector kf_selector_;
KeyframeCuller kf_culler_;
std::vector<Keyframe*> keyframes_;

// track() 함수에서
if (kf_selector_.needNewKeyframe(...)) {
    Keyframe* kf = new Keyframe(frame_id_, curr_image);
    kf->R = curr_pose_.R();
    kf->t = curr_pose_.t();
    keyframes_.push_back(kf);
}

// 주기적으로
if (frame_id_ % 50 == 0) {
    kf_culler_.cullRedundantKeyframes(keyframes_, ...);
}
```

---

## 🏗️ mini_slam 구현 (이번 주 핵심)

> 이번 주는 mini_slam에 **Keyframe + Map 관리**를 추가한다.
> 모든 프레임을 저장하지 않고, 키프레임만 선택적으로 저장하여 맵을 관리한다.

**작업 내용**:

| 작업 | 내용 |
|------|------|
| `keyframe.h` 구현 | Keyframe 클래스 (포즈, 특징점, 맵 포인트 인덱스, covisibility) |
| `map.h` 구현 | Map 클래스 (키프레임 저장, 맵 포인트 관리, 키프레임 선택/제거) |
| 키프레임 선택 기준 | parallax > 5°, 추적 특징점 < 80%일 때 새 키프레임 생성 |
| mini_slam 통합 | W5의 VO 파이프라인에 키프레임 관리 연결 |

**구현 파일**:
- `Studies/Phase 3/mini_slam/include/keyframe.h`
- `Studies/Phase 3/mini_slam/include/map.h`
- `Studies/Phase 3/mini_slam/src/keyframe.cpp`
- `Studies/Phase 3/mini_slam/src/map.cpp`

### 완성 기준

```bash
cd Studies/Phase\ 3/mini_slam/build
./mini_slam

# 키프레임만 저장하는 맵 관리 동작 확인
# 50프레임 입력 → 키프레임 5~10개만 선택됨
# Culling으로 중복 키프레임 제거
```

---

**다음**: Week 7에서 Bundle Adjustment로 최적화!
