# Week 5: Mini VO 프로젝트 - 완전한 Visual Odometry 구현!

> 🎯 **이번 주 목표**: Week 2 + Week 3를 통합해서 완전한 VO 시스템 만들기  
> ⏰ **예상 시간**: 15시간  
> 💡 **핵심 질문**: "실제로 작동하는 VO를 어떻게 만들까?"

---

## 🌟 드디어 통합!

### 지금까지 배운 것

```
Week 2: 초기화 (Essential Matrix)
  입력: 첫 두 프레임
  출력: 초기 R, t + 3D 맵
  문제: ||t|| = 1 (스케일 모호)

Week 3: 추적 (PnP)
  입력: 3D 맵 + 현재 프레임
  출력: 현재 포즈
  해결: 스케일 유지!

Week 5: VO 시스템
  = Week 2 + Week 3 + α
  = 완전한 Visual Odometry! 🎉
```

**비유:**
```
Week 2-3: 재료 준비 (밀가루, 물, 이스트)
Week 5:   빵 굽기 (실제 조립해서 완성!)
```

---

## 📚 Mini VO 시스템 구조

### 전체 파이프라인

```
┌────────────────────────────────────────────────────────┐
│                    Mini VO 시스템                        │
├────────────────────────────────────────────────────────┤
│                                                        │
│  프레임 0, 1                                             │
│  ─────────────────                                     │
│  📷 → 특징점 검출                                         │
│  📷 → 특징점 매칭                                         │
│       ↓                                                │
│  ┌─────────────────┐                                   │
│  │ 초기화 (Week 2) │                                     │
│  │ Essential Matrix│                                   │
│  └────────┬────────┘                                   │
│           ↓                                            │
│  초기 포즈 + 3D 맵                                        │
│  {R₀, t₀}, {R₁, t₁}, {X₁, X₂, ...}                     │
│                                                        │
│  ════════════════════════════════════                  │
│                                                        │
│  프레임 2, 3, 4, ...                                     │
│  ────────────────────                                  │
│  📷 → 특징점 추적 (Optical Flow)                          │
│       ↓                                                │
│  3D-2D 대응                                             │
│       ↓                                                │
│  ┌─────────────────┐                                   │
│  │  추적 (Week 3)  │                                    │
│  │  PnP + RANSAC   │                                   │
│  └────────┬────────┘                                   │
│           ↓                                            │
│  현재 포즈 {Rₜ, tₜ}                                       │
│           ↓                                            │
│  새 3D 점 추가 (Triangulation)                           │
│  오래된 점 제거                                           │
│                                                        │
│  반복! →                                                │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### 핵심 구성 요소

#### 1. 상태 (State)

```cpp
struct VOState {
    // 현재 상태
    enum { INITIALIZING, TRACKING, LOST } status;
    
    // 포즈
    cv::Mat R_curr, t_curr;  // 현재 포즈
    
    // 3D 맵
    std::vector<cv::Point3f> map_points;      // 3D 점들
    std::vector<int> point_ages;              // 각 점의 나이
    
    // 2D-3D 대응
    std::vector<cv::Point2f> tracked_2d;      // 추적된 2D 점
    std::vector<int> tracked_to_map;          // 맵 인덱스
    
    // 통계
    int frame_id;
    int num_inliers;
    double reprojection_error;
};
```

#### 2. 초기화 모듈

```cpp
bool initialize(
    const cv::Mat& frame0,
    const cv::Mat& frame1,
    VOState& state
) {
    // 1. 특징점 검출
    detectFeatures(frame0, kp0, desc0);
    detectFeatures(frame1, kp1, desc1);
    
    // 2. 매칭
    matchFeatures(desc0, desc1, matches);
    
    // 3. Essential Matrix (Week 2)
    cv::Mat E = cv::findEssentialMat(...);
    cv::recoverPose(E, ..., R, t);
    
    // 4. 초기 3D 맵 생성
    triangulateInitialMap(kp0, kp1, R, t, map_points);
    
    // 5. 상태 초기화
    state.status = TRACKING;
    state.R_curr = R;
    state.t_curr = t;
    state.map_points = map_points;
    
    return true;
}
```

#### 3. 추적 모듈

```cpp
bool track(
    const cv::Mat& prev_frame,
    const cv::Mat& curr_frame,
    VOState& state
) {
    // 1. Optical Flow로 특징점 추적
    cv::calcOpticalFlowPyrLK(
        prev_frame, curr_frame,
        prev_kp, curr_kp, status
    );
    
    // 2. 3D-2D 대응 구성
    build3D2DCorrespondences(state, curr_kp, points3d, points2d);
    
    // 3. PnP (Week 3)
    std::vector<int> inliers;
    cv::solvePnPRansac(
        points3d, points2d, K,
        rvec, tvec, inliers
    );
    
    // 4. Inlier ratio 체크
    double ratio = inliers.size() / (double)points3d.size();
    if (ratio < 0.3) {
        state.status = LOST;
        return false;
    }
    
    // 5. 포즈 업데이트
    cv::Rodrigues(rvec, state.R_curr);
    state.t_curr = tvec;
    
    return true;
}
```

#### 4. 맵 관리

```cpp
void updateMap(VOState& state) {
    // 1. 오래된 점 제거
    for (int i = map_points.size()-1; i >= 0; i--) {
        if (point_ages[i] > 30) {  // 30 프레임 이상
            map_points.erase(map_points.begin() + i);
            point_ages.erase(point_ages.begin() + i);
        }
    }
    
    // 2. 새 점 추가 (keyframe일 때)
    if (isKeyframe()) {
        addNewPoints(state);
    }
    
    // 3. 나이 증가
    for (auto& age : point_ages) {
        age++;
    }
}
```

---

## 💡 구현 디테일

### 1. 언제 초기화할까?

**조건:**
```
✅ 충분한 특징점 (> 100개)
✅ 충분한 베이스라인
   - 평균 시차 > 2 픽셀
   - 중앙값 시차 > 1 픽셀
✅ 충분한 inliers (> 70%)
```

**코드:**
```cpp
bool shouldInitialize(
    const std::vector<cv::Point2f>& pts0,
    const std::vector<cv::Point2f>& pts1
) {
    if (pts0.size() < 100) return false;
    
    // 시차 계산
    std::vector<double> parallax;
    for (int i = 0; i < pts0.size(); i++) {
        double dx = pts1[i].x - pts0[i].x;
        double dy = pts1[i].y - pts0[i].y;
        parallax.push_back(std::sqrt(dx*dx + dy*dy));
    }
    
    // 중앙값
    std::sort(parallax.begin(), parallax.end());
    double median = parallax[parallax.size()/2];
    
    return median > 1.0;
}
```

### 2. Keyframe 선택

**언제 Keyframe?**
```
새 keyframe 조건 (하나라도 만족):
  1. 이전 keyframe과의 시차 > 10%
  2. 추적되는 점 < 70%
  3. 일정 시간 경과 (10 프레임)
```

**왜 필요?**
```
매 프레임 새 점 추가:
  → 계산량 폭발! 😱
  
Keyframe만 추가:
  → 적절한 개수 유지 😊
  → 중요한 위치만 선택
```

### 3. 맵 점 품질 관리

**좋은 점 vs 나쁜 점**
```
✅ 좋은 점:
   - 여러 프레임에서 관측
   - 재투영 오차 작음
   - Depth 적절 (0.1m ~ 100m)

❌ 나쁜 점:
   - 한 번만 관측
   - 재투영 오차 큼
   - Depth 이상 (음수, 너무 멀리)
```

**제거 전략:**
```cpp
void cullBadPoints(VOState& state) {
    for (int i = map_points.size()-1; i >= 0; i--) {
        bool should_remove = false;
        
        // 나이가 너무 많음
        if (point_ages[i] > 30) {
            should_remove = true;
        }
        
        // 관측 횟수 적음
        if (observation_counts[i] < 3) {
            should_remove = true;
        }
        
        if (should_remove) {
            map_points.erase(map_points.begin() + i);
            point_ages.erase(point_ages.begin() + i);
            observation_counts.erase(observation_counts.begin() + i);
        }
    }
}
```

### 4. 추적 실패 처리

**추적 실패 = Inlier ratio < 30%**

**대처 방법:**
```
1차: 재초기화 시도
   - 현재 + 다음 프레임으로 Essential

2차: Relocalization
   - 이전 keyframe들과 매칭
   - 충분한 대응점 있으면 PnP

3차: 포기
   - "LOST" 상태로 변경
   - 사용자에게 알림
```

---

## 🔬 실습 내용

### 구현 항목

1. **VO 클래스 구조**
   - State 관리
   - 초기화/추적 분리

2. **초기화 모듈**
   - Essential Matrix
   - 초기 맵 생성
   - 품질 체크

3. **추적 모듈**
   - Optical Flow
   - PnP + RANSAC
   - 포즈 업데이트

4. **맵 관리**
   - 점 추가/제거
   - Keyframe 선택
   - 나이 관리

5. **시각화**
   - 궤적 플롯
   - 맵 포인트 시각화
   - 통계 출력

### 테스트 데이터

**옵션 1: 합성 데이터**
```cpp
// 원형 궤적 생성
for (int i = 0; i < 100; i++) {
    double angle = i * 0.1;
    Pose p;
    p.R = rotation_matrix(0, angle, 0);
    p.t = cv::Vec3d(5*cos(angle), 0, 5*sin(angle));
    
    // 가상 이미지 생성
    renderVirtualImage(p, world_points, image);
}
```

**옵션 2: 실제 데이터셋**
```
KITTI:
  - 다운로드: http://www.cvlibs.net/datasets/kitti/
  - 시퀀스 00~10 (도시 주행)
  - Ground truth 포즈 제공

EuRoC:
  - 다운로드: https://projects.asl.ethz.ch/datasets/doku.php
  - MH_01~05 (실내 드론)
  - Ground truth 제공
```

### 예상 결과

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Mini VO 실행
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

프레임 0-1: 초기화
   매칭: 215개
   Inliers: 198/215 (92%)
   초기 맵: 185개 점

프레임 2:
   추적: 178/185 (96%)
   PnP inliers: 165/178 (93%)
   포즈: t = [1.02, -0.01, 0.03]
   재투영 오차: 0.65 px

프레임 3:
   추적: 172/185 (93%)
   PnP inliers: 158/172 (92%)
   포즈: t = [2.08, -0.02, 0.05]
   재투영 오차: 0.71 px

...

프레임 100:
   총 이동: 52.3m
   드리프트: 2.1m (4.0%)
   평균 inlier: 88%
   평균 재투영 오차: 0.68 px

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ VO 완료!
```

---

## 📊 성능 평가

### 평가 지표

**1. Absolute Trajectory Error (ATE)**
```
ATE = √(Σ ||t_gt(i) - t_est(i)||² / N)

좋음: < 1% 이동 거리
보통: 1~5%
나쁨: > 5%
```

**2. Relative Pose Error (RPE)**
```
RPE = 연속 프레임 간 오차

좋음: < 0.1m / 100m
```

**3. 드리프트 (Drift)**
```
Drift = 시간에 따른 오차 누적

관찰:
  100m 이동 후 2~5m 오차 (일반적)
  → IMU 없이는 피할 수 없음!
```

### 실험

**Experiment 1: 직선 궤적**
```
예상: 낮은 드리프트 (2~3%)
이유: 일정한 움직임
```

**Experiment 2: 회전 많은 궤적**
```
예상: 높은 드리프트 (5~10%)
이유: 회전 추정 오차 누적
```

**Experiment 3: 텍스처 부족**
```
예상: 추적 실패
이유: 특징점 부족
```

---

## 🔍 자체 점검

### 질문 1: 초기화 조건
**Q:** 왜 베이스라인이 충분해야 하나요?

**A:**
```
베이스라인 작으면:
  - 삼각측량 불안정
  - Depth 오차 큼
  - 스케일 추정 불안정

해결: 시차 > 2 픽셀 확인
```

### 질문 2: Keyframe
**Q:** 왜 매 프레임마다 점을 추가하지 않나요?

**A:**
```
매 프레임 추가:
  - 맵 크기 폭발 (10,000개+)
  - 계산량 증가
  - 중복 점 많음

Keyframe만:
  - 적절한 개수 (500~1000개)
  - 계산 가능
  - 다양한 시점
```

### 질문 3: 드리프트
**Q:** 왜 드리프트가 피할 수 없나요?

**A:**
```
매 프레임 작은 오차:
  프레임 t→t+1: 0.1% 오차
  
100 프레임 후:
  0.1% × 100 = 10% 오차 (복리!)
  
해결:
  ❌ VO만으로는 불가능
  ✅ SLAM (Loop Closure)
  ✅ VIO (IMU 융합)
```

### 질문 4: 추적 vs 초기화
**Q:** 추적이 실패하면 왜 재초기화하나요?

**A:**
```
추적 실패 = Inlier < 30%
  → 포즈 추정 불신
  
재초기화:
  - Essential (2D-2D)로 다시 시작
  - 새로운 맵 생성
  - 스케일은 재설정됨 (단절)
```

---

## 🎯 이번 주 핵심 요약

1. **VO = 초기화 + 추적**
   - Week 2: Essential (첫 2 프레임)
   - Week 3: PnP (나머지 모든 프레임)

2. **상태 관리**
   - INITIALIZING → TRACKING → LOST
   - 포즈 + 맵 + 통계

3. **맵 관리 필수**
   - 점 추가 (Keyframe)
   - 점 제거 (나이, 품질)
   - 적절한 개수 유지

4. **드리프트는 필연**
   - 오차 누적 피할 수 없음
   - 2~5% 일반적
   - SLAM/VIO로 해결

5. **실제 데이터로 테스트**
   - KITTI, EuRoC
   - Ground truth 비교
   - 한계 직접 관찰

---

**축하합니다!** 🎉 드디어 완전한 VO 시스템을 만들었습니다! 이제 왜 IMU가 필요한지, 왜 SLAM이 필요한지 몸으로 느낄 수 있을 거예요!

**다음 단계:**
- Week 6-13: Keyframe, Bundle Adjustment, SLAM 고급 기법
- Phase 4: VIO (Visual-Inertial Odometry) - IMU 융합!

다음: [Week 6 - Keyframe Management](../week6/README.md)
