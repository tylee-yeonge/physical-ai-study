# Week 6: Keyframe Management - 중요한 프레임만 선택하기

> 🎯 **이번 주 목표**: Keyframe 선택 전략을 이해하기
> ⏰ **예상 시간**: 5시간 (개념 중심)
> 💡 **핵심 질문**: "모든 프레임이 아니라 왜 일부만 선택할까?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 데모 실행 | `basic.cpp` | 빌드 후 `./basic` 실행, 결과 먼저 관찰 |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | Keyframe 목적, 선택 조건, Culling 기준 확인 |
| 4 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | ORB-SLAM vs VINS 비교, Covisibility 분석 |
| 5 | 직접 구현 (Step 1-2) | `my_basic.cpp` | addCovisibility, getCovisibleKeyframes 구현 |
| 6 | 직접 구현 (Step 3-5) | `my_basic.cpp` | needNewKeyframe, isRedundant, cullRedundant 구현 |
| 7 | 실습 | [PRACTICE.md](./PRACTICE.md) | Keyframe 선택/제거 로직 구현 및 VO 통합 |

### 퀴즈 ↔ basic.cpp 매핑

| 퀴즈 | 문제 | basic.cpp 교육 블록 |
|:----:|------|-------------------|
| easy Q1 | Keyframe 사용 이유 3가지 | `demoKeyframeSelection()` — Keyframe 정의 |
| easy Q2 | ORB-SLAM 기본 조건 (20프레임) | `demoKeyframeSelection()` — 조건별 판단 시연 |
| easy Q3 | Culling 목적 | `demoCullingRule()` — 90% Rule 소개 |
| easy Q4 | 90% rule 설명 | `demoCullingRule()` — 수치 예시 |
| easy Q5 | Covisibility graph 용도 | `demoCovisibility()` — 3가지 용도 |
| medium Q1 | ORB-SLAM vs VINS 차이 | `demoKeyframeSelection()` — 두 방식 비교 |
| medium Q3 | Covisibility가 높은 KF | `demoCovisibility()` — 그래프 시연 |
| medium Q4 | 첫 KF 보호 이유 | `demoCullingRule()` — 좌표계 원점 |

---

## 🌟 시작하기 전에

### Week 5의 문제점

**Week 5 Mini VO:**
```
매 프레임마다:
  - 새 특징점 검출
  - 맵에 점 추가
  - 포즈 추정
  
결과:
  프레임 100: 맵 크기 5,000개 😱
  프레임 500: 맵 크기 25,000개 😱😱
  프레임 1000: 맵 크기 50,000개 💥 (메모리 부족!)
```

**비유:**
```
상황: 여행 사진 정리
  
나쁜 방법: 모든 사진 저장 (10,000장)
         → 저장공간 부족, 찾기 어려움
         
좋은 방법: 중요한 사진만 (100장)
         → 관리 가능, 핵심 순간만
         
→ 이게 바로 Keyframe!
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. Keyframe이 뭐죠?

**정의:**
```
Keyframe = 맵 구축/최적화에 사용되는 중요한 프레임

일반 프레임: 추적만 (PnP)
Keyframe:   추적 + 맵 추가 + 최적화
```

**SLAM에서의 역할:**
```
┌─────────────────────────────────────┐
│         SLAM 시스템                   │
├─────────────────────────────────────┤
│                                     │
│  모든 프레임 (30 FPS)                  │
│  ────────────────────               │
│  Frame 0, 1, 2, 3, 4, 5, 6, 7, ...  │
│                                     │
│  ↓ (필터링)                           │
│                                     │
│  Keyframes (3-5 FPS)                │
│  ────────────────                   │
│  Frame 0,    3,    7,    12, ...    │
│                                     │
│  → 이것들만 맵에 저장!                  │
│  → 이것들만 Bundle Adjustment!        │
│                                     │
└─────────────────────────────────────┘
```

**왜 필요한가요?**
```
✅ 계산량 감소
   - BA: O(n³) → Keyframe만 최적화
   
✅ 메모리 절약
   - 5000 프레임 → 500 Keyframe (10배 감소)
   
✅ 맵 품질 향상
   - 중복 정보 제거
   - 다양한 시점만 선택
```

---

### 2. Keyframe 선택 기준

#### ORB-SLAM2 방식

**기본 조건 (AND):**
```
1. 최소 프레임 간격
   last_kf_id - curr_id >= 20 frames
   
2. Local Mapping idle
   백엔드가 바쁘지 않을 때
```

**주 조건 (OR):**
```
조건 A: 충분한 프레임 경과
   curr_id - last_kf_id >= max_frames (보통 30)
   
조건 B: 추적 품질 저하
   matched_inliers < 100
   OR
   matched_inliers < 0.9 * reference_matches
   
조건 C: 시각적 변화 큼
   tracked_map_points < 50%
   (이전 Keyframe 대비)
```

**의사코드:**
```cpp
bool needNewKeyframe() {
    // 기본 조건
    if (num_frames_since_last_kf < 20) return false;
    if (local_mapping_busy) return false;
    
    // 주 조건
    bool cond_A = num_frames_since_last_kf >= 30;
    bool cond_B = matched_inliers < 100 || 
                  matched_inliers < 0.9 * ref_matches;
    bool cond_C = tracked_ratio < 0.5;
    
    return cond_A || cond_B || cond_C;
}
```

#### VINS-Mono 방식

**시차(Parallax) 기반:**
```cpp
bool checkParallax(Frame& curr, Frame& last_kf) {
    // 평균 시차 계산
    double sum_parallax = 0.0;
    int count = 0;
    
    for (auto& match : matches) {
        cv::Point2f p1 = last_kf.keypoints[match.first];
        cv::Point2f p2 = curr.keypoints[match.second];
        
        double parallax = cv::norm(p2 - p1);
        sum_parallax += parallax;
        count++;
    }
    
    double avg_parallax = sum_parallax / count;
    
    // 평균 시차 > 10 픽셀이면 Keyframe
    return avg_parallax > 10.0;
}
```

**의미:**
```
시차 크다 = 카메라가 많이 움직임
          = 새로운 시점
          = 맵에 추가할 가치 있음!
```

---

### 3. Keyframe 제거 (Culling)

**왜 제거?**
```
문제: Keyframe도 계속 쌓이면...
      → 1000개 Keyframe → 여전히 많음!
      
해결: 덜 중요한 Keyframe 제거
```

#### ORB-SLAM2 Culling 전략

**기준: 중복도(Redundancy)**
```
90% Rule:
  - Keyframe A의 맵 포인트
  - 다른 3개 이상 Keyframe에서도 관측
  - 90% 이상이면
  → Keyframe A 제거!
```

**의사코드:**
```cpp
void cullKeyframes() {
    for (auto* kf : keyframes) {
        if (kf == first_kf) continue;  // 첫 KF는 유지
        
        int redundant_obs = 0;
        int total_obs = 0;
        
        for (auto* mp : kf->map_points) {
            total_obs++;
            
            // 다른 Keyframe에서 관측 횟수
            int obs_count = mp->num_observations();
            
            if (obs_count >= 3) {
                redundant_obs++;
            }
        }
        
        double redundancy = (double)redundant_obs / total_obs;
        
        if (redundancy > 0.9) {
            removeKeyframe(kf);
        }
    }
}
```

**시각화:**
```
Before Culling:
  KF0 ──┐
  KF1 ──┼── Point A (4 observations)
  KF2 ──┤
  KF3 ──┘
  
KF2의 모든 점이 다른 KF들에서도 보임
→ KF2 제거!

After Culling:
  KF0 ──┐
  KF1 ──┼── Point A (3 observations, 여전히 충분)
  KF3 ──┘
```

---

### 4. Covisibility Graph

**정의:**
```
Covisibility = 같은 맵 포인트를 공유하는 Keyframe들

예:
  KF1: Point A, B, C 관측
  KF2: Point B, C, D 관측
  
  → KF1-KF2 covisibility = 2 (B, C)
```

**그래프 구조:**
```
        [KF0]
         / \
      15/   \12
       /     \
    [KF1]--8--[KF2]
       \     /
      10\   /9
         \ /
        [KF3]

숫자 = 공유하는 맵 포인트 개수
```

**용도:**
```
1. Local BA 범위 결정
   - 현재 KF의 covisible KF들만 최적화
   
2. Loop Closure 후보
   - Covisibility 높으면 같은 장소
   
3. Relocalization
   - Track lost 시 covisible KF에서 재시도
```

---

## 💡 구현 세부사항

### Keyframe 클래스

```cpp
class Keyframe {
public:
    int id;
    cv::Mat image;
    
    // 포즈
    Eigen::Matrix3d R;
    Eigen::Vector3d t;
    
    // 특징점
    std::vector<cv::Point2f> keypoints;
    cv::Mat descriptors;
    
    // 맵 포인트
    std::vector<MapPoint*> map_points;
    
    // Covisibility
    std::map<Keyframe*, int> covisible_keyframes;  // KF → 공유 포인트 개수
    
    // 통계
    int num_tracked;
    double avg_parallax;
    
    Keyframe(int id, const cv::Mat& img) : id(id), image(img.clone()) {}
    
    void addCovisibility(Keyframe* other, int num_shared) {
        covisible_keyframes[other] = num_shared;
    }
    
    std::vector<Keyframe*> getCovisibleKeyframes(int min_shared = 15) {
        std::vector<Keyframe*> result;
        for (auto& [kf, count] : covisible_keyframes) {
            if (count >= min_shared) {
                result.push_back(kf);
            }
        }
        return result;
    }
};
```

### Keyframe Selection

```cpp
class KeyframeSelector {
public:
    KeyframeSelector() : 
        min_frames_(20),
        max_frames_(30),
        min_tracked_ratio_(0.5),
        min_parallax_(10.0) {}
    
    bool needNewKeyframe(
        const Frame& curr,
        const Keyframe* last_kf,
        int num_frames_since_last
    ) {
        // 기본 조건
        if (num_frames_since_last < min_frames_) {
            return false;
        }
        
        // 조건 A: 최대 간격
        if (num_frames_since_last >= max_frames_) {
            return true;
        }
        
        // 조건 B: 추적 품질
        double tracked_ratio = (double)curr.num_tracked / last_kf->map_points.size();
        if (tracked_ratio < min_tracked_ratio_) {
            return true;
        }
        
        // 조건 C: 시차
        double parallax = computeParallax(curr, *last_kf);
        if (parallax > min_parallax_) {
            return true;
        }
        
        return false;
    }
    
private:
    int min_frames_;
    int max_frames_;
    double min_tracked_ratio_;
    double min_parallax_;
    
    double computeParallax(const Frame& curr, const Keyframe& last_kf) {
        double sum = 0.0;
        int count = 0;
        
        for (int i = 0; i < curr.matches.size(); i++) {
            int idx_curr = curr.matches[i].first;
            int idx_last = curr.matches[i].second;
            
            cv::Point2f p1 = last_kf.keypoints[idx_last];
            cv::Point2f p2 = curr.keypoints[idx_curr];
            
            sum += cv::norm(p2 - p1);
            count++;
        }
        
        return count > 0 ? sum / count : 0.0;
    }
};
```

### Keyframe Culling

```cpp
class KeyframeCuller {
public:
    void cullRedundantKeyframes(
        std::vector<Keyframe*>& keyframes,
        double redundancy_threshold = 0.9
    ) {
        for (int i = keyframes.size()-1; i > 0; i--) {  // 첫 KF 제외
            Keyframe* kf = keyframes[i];
            
            if (isRedundant(kf, redundancy_threshold)) {
                removeKeyframe(keyframes, i);
            }
        }
    }
    
private:
    bool isRedundant(Keyframe* kf, double threshold) {
        int redundant_obs = 0;
        int total_obs = 0;
        
        for (auto* mp : kf->map_points) {
            if (!mp) continue;
            
            total_obs++;
            
            // 다른 Keyframe에서도 관측되는지
            if (mp->num_observations >= 3) {
                redundant_obs++;
            }
        }
        
        if (total_obs == 0) return false;
        
        double redundancy = (double)redundant_obs / total_obs;
        return redundancy > threshold;
    }
    
    void removeKeyframe(std::vector<Keyframe*>& keyframes, int idx) {
        Keyframe* kf = keyframes[idx];
        
        // Covisibility graph 업데이트
        for (auto& [other, _] : kf->covisible_keyframes) {
            other->covisible_keyframes.erase(kf);
        }
        
        // 맵 포인트에서 관측 제거
        for (auto* mp : kf->map_points) {
            if (mp) {
                mp->removeObservation(kf);
            }
        }
        
        delete kf;
        keyframes.erase(keyframes.begin() + idx);
    }
};
```

---

## 🔍 자체 점검

### 질문 1: Keyframe 목적
**Q:** Keyframe을 사용하는 주요 이유 3가지는?

**A:**
```
1. 계산량 감소
   - BA 복잡도 O(n³) → n을 줄임
   
2. 메모리 절약
   - 전체 프레임 대신 일부만 저장
   
3. 맵 품질
   - 중복 제거, 다양한 시점
```

### 질문 2: 선택 기준
**Q:** ORB-SLAM의 주요 Keyframe 선택 조건은?

**A:**
```
기본: 20 프레임 이상 경과 + Local mapping idle

주 (OR):
  - 30 프레임 이상 경과
  - 추적 품질 저하 (inlier < 100)
  - 시각 변화 큼 (tracked < 50%)
```

### 질문 3: Culling
**Q:** Keyframe을 제거하는 기준은?

**A:**
```
90% Rule:
  - Keyframe의 맵 포인트 중 90% 이상이
  - 다른 3개 이상 Keyframe에서도 관측됨
  
→ 중복! 제거해도 정보 손실 없음
```

### 질문 4: Covisibility
**Q:** Covisibility graph의 용도는?

**A:**
```
1. Local BA 범위 결정
2. Loop Closure 후보 탐색
3. Relocalization 시도
```

---

## 📝 이번 주 실습

### 구현 항목

1. **Keyframe 클래스**
   - 포즈, 특징점, 맵 포인트 저장
   - Covisibility 관리

2. **KeyframeSelector**
   - ORB-SLAM 방식 구현
   - 시차 계산

3. **KeyframeCuller**
   - Redundancy 체크
   - 안전한 제거

4. **Week 5 통합**
   - Mini VO에 Keyframe 추가
   - 맵 크기 관리 확인

### 예상 결과

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Keyframe Management
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Frame 0: NEW KEYFRAME (초기화)
  맵 크기: 150개 점

Frame 25: NEW KEYFRAME (30 프레임 경과)
  맵 크기: 280개 점

Frame 48: NEW KEYFRAME (시차 > 10px)
  맵 크기: 410개 점

Frame 100: Culling performed
  KF3 removed (90% redundancy)
  남은 Keyframes: 3개

Frame 200: 
  Keyframes: 8개
  맵 크기: 650개 점 (안정적!)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ 맵 크기 관리 성공!
```

### 체크리스트

- [ ] Keyframe 클래스 구현
- [ ] 선택 로직 (ORB-SLAM 방식)
- [ ] Culling 로직
- [ ] Week 5 VO에 통합
- [ ] 맵 크기 < 1000개 유지

---

## 🎯 핵심 요약

1. **Keyframe = 중요한 프레임만**
   - 모든 프레임 저장 불가능
   - 계산량/메모리 절약

2. **선택 기준**
   - 시간 경과 (30 프레임)
   - 품질 저하 (inlier < 100)
   - 시각 변화 (tracked < 50%)

3. **Culling**
   - 90% rule
   - 중복 Keyframe 제거

4. **Covisibility Graph**
   - 공유 맵 포인트 관리
   - Local BA, Loop Closure 활용

5. **SLAM의 필수 요소**
   - VO: Keyframe 선택적
   - SLAM: Keyframe 필수!

---

**다음 주**: Week 7 - Local Bundle Adjustment로 Keyframe들을 최적화합니다!

다음: [Week 7 - Local Bundle Adjustment](../week7/README.md)
