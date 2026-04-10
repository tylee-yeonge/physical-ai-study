# Week 1 실습: VINS 코드에서 VO 프론트엔드 분석

> 🎯 **목표**: VO 파이프라인의 각 단계를 VINS 코드에서 찾고 이해
> 💻 **방식**: 코드 분석 (구현 없음)
> ⏰ **예상 시간**: 4-5시간

---

## 준비

VINS-Fusion 소스코드가 필요합니다:
```bash
git clone https://github.com/HKUST-Aerial-Robotics/VINS-Fusion.git
```

---

## 실습 1: feature_tracker 구조 분석 (2시간)

### 분석 대상 파일

```
VINS-Fusion/
└── vins_estimator/src/featureTracker/
    ├── feature_tracker.h
    └── feature_tracker.cpp
```

### Step 1: 특징점 검출 찾기

`feature_tracker.cpp`에서 `cv::goodFeaturesToTrack` 검색:

```cpp
// 이 함수가 FAST 코너 검출에 해당
cv::goodFeaturesToTrack(forw_img, n_pts, MAX_CNT - forw_pts.size(),
                        0.01, MIN_DIST, mask);
```

**확인할 것:**
- [ ] `MAX_CNT`: 최대 특징점 수 (보통 150-200)
- [ ] `MIN_DIST`: 특징점 간 최소 거리 (공간적 분포 보장)
- [ ] `mask`: 기존 특징점 주변은 마스킹 (중복 방지)

### Step 2: KLT 추적 찾기

`cv::calcOpticalFlowPyrLK` 검색:

```cpp
// 이전 프레임 특징점을 현재 프레임에서 추적
cv::calcOpticalFlowPyrLK(cur_img, forw_img, cur_pts, forw_pts,
                         status, err, cv::Size(21, 21), 3);
```

**확인할 것:**
- [ ] `cv::Size(21, 21)`: 윈도우 크기 (큰 움직임 대응)
- [ ] 3: 피라미드 레벨 수 (더 큰 움직임 처리)
- [ ] `status`: 추적 성공 여부 (0/1)

### Step 3: 추적 실패 처리 찾기

추적 실패한 점을 제거하고, 부족하면 새로 검출하는 로직:

```cpp
// status가 0인 점 제거
reduceVector(forw_pts, status);
reduceVector(cur_pts, status);
reduceVector(ids, status);

// 특징점 수가 부족하면 새로 검출
if (n_pts.size() > 0) {
    cv::goodFeaturesToTrack(...);
}
```

**확인할 것:**
- [ ] 특징점에 ID가 부여되는 방식 (`n_id++`)
- [ ] 각 특징점이 몇 프레임 동안 추적되었는지 (`track_cnt`)
- [ ] 오래 추적된 점이 우선순위가 높은 이유

---

## 실습 2: feature_manager 구조 분석 (1시간)

### 분석 대상 파일

```
vins_estimator/src/
└── feature_manager.cpp/.h
```

### 확인 포인트

1. **FeaturePerFrame 구조**: 한 특징점이 한 프레임에서의 관측 정보
   - 2D 좌표, 속도, 깊이 등

2. **FeaturePerId 구조**: 한 특징점의 전체 생명주기
   - 어떤 프레임에서 처음 관측되었는지
   - 몇 개 프레임에서 관측되었는지

3. **addFeatureCheckParallax()**: 새 프레임이 키프레임인지 판단
   - Parallax(시차) 기반 판단
   - 이것이 Week 2의 키프레임 선택과 연결됨

---

## 실습 3: 모션 추정 방법 비교표 채우기 (30분)

아래 표를 직접 채워보세요 (README.md를 보지 않고):

| 항목 | 2D-2D | 3D-2D (PnP) | 3D-3D (ICP) |
|------|-------|-------------|-------------|
| 입력 | | | |
| 최소 점 수 | | | |
| 사용 시점 | | | |
| 스케일 정보 | | | |
| 핵심 알고리즘 | | | |
| VINS에서 사용? | | | |
| OpenCV 함수 | | | |

### 정답 (채운 후 확인)

<details>
<summary>클릭하여 정답 확인</summary>

| 항목 | 2D-2D | 3D-2D (PnP) | 3D-3D (ICP) |
|------|-------|-------------|-------------|
| 입력 | 2D-2D 대응 | 3D점 + 2D관측 | 3D-3D 대응 |
| 최소 점 수 | 5점 (5-point) | 3점 (P3P) | 3점 |
| 사용 시점 | 초기화 | 매 프레임 추적 | RGB-D/LiDAR |
| 스케일 정보 | 없음 (모호) | 있음 (3D점 사용) | 있음 |
| 핵심 알고리즘 | Essential Matrix | P3P, EPnP | SVD, ICP iteration |
| VINS에서 사용? | 초기화 시 | Visual factor | 사용 안 함 |
| OpenCV 함수 | findEssentialMat | solvePnPRansac | - |

</details>

---

## 실습 4: VO 파이프라인과 VINS 코드 매핑 (30분)

VO 파이프라인 6단계를 VINS 코드에 매핑해보세요:

| VO 단계 | VINS 코드 위치 |
|---------|---------------|
| 1. 이미지 입력 | |
| 2. 특징점 검출 | |
| 3. 매칭/추적 | |
| 4. 모션 추정 | |
| 5. 최적화 | |
| 6. 포즈 출력 | |

<details>
<summary>클릭하여 정답 확인</summary>

| VO 단계 | VINS 코드 위치 |
|---------|---------------|
| 1. 이미지 입력 | `img_callback()` in estimator_node.cpp |
| 2. 특징점 검출 | `goodFeaturesToTrack()` in feature_tracker.cpp |
| 3. 매칭/추적 | `calcOpticalFlowPyrLK()` in feature_tracker.cpp |
| 4. 모션 추정 | `optimization()` → Visual factor in estimator.cpp |
| 5. 최적화 | `optimization()` → Ceres Solve in estimator.cpp |
| 6. 포즈 출력 | `pubOdometry()` in visualization.cpp |

</details>

---

## 체크리스트

- [ ] feature_tracker에서 검출-추적-관리 흐름 파악
- [ ] feature_manager의 FeaturePerId 구조 이해
- [ ] 모션 추정 3가지 방법 비교표 작성
- [ ] VO 파이프라인 ↔ VINS 코드 매핑 완료
