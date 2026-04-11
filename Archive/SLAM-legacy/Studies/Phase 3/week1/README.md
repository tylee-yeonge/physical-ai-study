# Week 1: VO 파이프라인 + 모션 추정 방법

> [goal] **목표**: VO 전체 흐름 이해, 2D-2D / 3D-2D / ICP 방법론 비교
> [time] **예상 시간**: 7-10시간
> [tip] **핵심 질문**: "카메라만으로 움직임을 어떻게 추정하는가?"

---

## [list] 학습 순서

| 순서 | 단계 | 설명 |
|:----:|------|------|
| 1 | README.md | 아래 핵심 개념을 읽고 이해 |
| 2 | PRACTICE.md | VINS 코드에서 해당 부분 찾기 |
| 3 | 자체 점검 | 질문에 자기 말로 답 써보기 |

---

## [ref] 핵심 개념

### 1. Visual Odometry란?

연속적인 카메라 이미지만으로 카메라의 움직임(R, t)을 추정하는 기술.

**쉬운 비유:** 차를 운전할 때 앞 유리창(카메라)만 보면서 "100m 정도 앞으로 갔고, 왼쪽으로 5도 돌았다"를 파악하는 것.

**왜 필요한가?**
- GPS 안 되는 곳 (실내, 터널, 지하)
- 정밀한 위치 추정 (로봇, 드론)
- 저렴한 센서로 위치 파악 (카메라 하나면 됨)

### VO vs SLAM

| 구분 | Visual Odometry | SLAM |
|------|----------------|------|
| 하는 일 | "지금 얼마나 움직였어?" | "내가 어디 있고, 주변 지도는?" |
| 범위 | 로컬 (최근 몇 프레임) | 전역 (전체 경로) |
| Loop Closure | 없음 | 있음 ("여기 전에 왔었네!") |
| 오차 | 계속 누적 (드리프트) | Loop로 보정 |
| 계산량 | 가벼움 | 무거움 |

**둘의 관계:** SLAM = VO(프론트엔드) + Loop Closure + 최적화(백엔드)

---

### 2. VO 파이프라인 6단계

```
+----------+   +----------+   +----------+
| 1. 이미지 | → | 2. 특징점 | → | 3. 매칭/  |
|    입력   |   |    검출   |   |    추적   |
+----------+   +----------+   +----------+
                                     ↓
+----------+   +----------+   +----------+
| 6. 포즈   | ← | 5. 최적화 | ← | 4. 모션   |
|    출력   |   |  (선택)   |   |    추정   |
+----------+   +----------+   +----------+
```

**1단계 - 이미지 입력:** 카메라에서 연속 프레임 수신 (보통 30 FPS)

**2단계 - 특징점 검출:** 이미지에서 "특별한 점" 찾기
- FAST: 속도 빠름, VINS에서 사용
- ORB: FAST + BRIEF 디스크립터, ORB-SLAM에서 사용
- 보통 200-500개 특징점 검출

**3단계 - 매칭/추적:** 이전 프레임의 점과 현재 프레임의 점 연결
- **Optical Flow (KLT)**: 밝기 패턴으로 추적, 빠름 → VINS 사용
- **Descriptor 매칭**: ORB 등 디스크립터 비교, 정확함 → ORB-SLAM 사용

**4단계 - 모션 추정:** 대응점으로 카메라가 얼마나 움직였는지 계산
- 2D-2D (초기화), 3D-2D (추적) → 아래에서 자세히

**5단계 - Local 최적화:** Bundle Adjustment로 정확도 향상 (Week 2)

**6단계 - 포즈 출력:** 최종 카메라 위치 & 방향 [R, t]

**병목은 어디?** 특징점 검출 & 매칭이 가장 시간 소요. 실시간을 위해 FAST 같은 빠른 검출기 + KLT 추적 사용.

---

### 3. 모션 추정 3가지 방법

#### 방법 1: 2D-2D (Essential Matrix) - VO 초기화용

**상황:** 3D 정보가 전혀 없는 시작 시점. 두 이미지의 2D 특징점 대응만 있음.

**수학적 배경:**
```
에피폴라 제약: x'^T E x = 0

E = [t]_x R   (t의 skew-symmetric matrix × 회전 행렬)
```

**과정:**
1. 두 이미지에서 특징점 매칭 (ORB 또는 KLT)
2. Essential Matrix 추정 (5-point algorithm + RANSAC)
3. SVD로 E 분해 → R, t 복원 (4가지 해 발생)
4. **Cheirality check**: 3D 점이 두 카메라 앞에 있는 해 선택
5. 삼각측량으로 초기 3D 맵 생성

**한계:**
- **스케일 모호성**: t의 방향은 알지만 크기는 모름 (||t||=1로 정규화)
- **Pure rotation**: 평행이동 없으면 E 추정 불안정
- **평면 장면**: Homography와 혼동 (degenerate case)

**OpenCV:**
```cpp
cv::Mat E = cv::findEssentialMat(pts1, pts2, K, cv::RANSAC, 0.999, 1.0);
cv::recoverPose(E, pts1, pts2, K, R, t);
```

---

#### 방법 2: 3D-2D (PnP) - VO 추적의 핵심

**상황:** 이전에 삼각측량한 3D 맵포인트가 있고, 현재 프레임에서 2D 관측이 있음.

**왜 핵심인가?** 초기화 후 **매 프레임** 이 방법으로 포즈를 추정함.

**과정:**
1. 이전 맵에서 3D 점 확보
2. 현재 프레임에서 대응 2D 점 찾기 (매칭 또는 KLT 추적)
3. PnP + RANSAC으로 현재 포즈 [R, t] 추정

**알고리즘 비교:**
| 알고리즘 | 최소 점 수 | 특징 |
|---------|----------|------|
| P3P | 3점 | 최대 4개 해, RANSAC과 결합 |
| EPnP | 4점 | 효율적 O(n), 많은 점에서 안정 |
| DLS | 3점 | 정확하지만 느림 |

**OpenCV:**
```cpp
cv::solvePnPRansac(
    object_points,   // 3D 점 (이전 맵에서)
    image_points,    // 2D 점 (현재 프레임에서)
    K,               // 카메라 내부 파라미터
    dist_coeffs,     // 왜곡 계수
    rvec, tvec,      // 출력: 회전, 평행이동
    false,           // useExtrinsicGuess
    100,             // iterationsCount
    8.0,             // reprojectionError threshold
    0.99,            // confidence
    inliers          // inlier 인덱스
);
```

**VINS에서:** Visual factor = PnP 기반 재투영 오차를 Ceres로 최적화

---

#### 방법 3: 3D-3D (ICP) - 참고

**상황:** 양쪽 프레임 모두 3D 점이 있음 (RGB-D, LiDAR).

**ICP (Iterative Closest Point):**
1. 가장 가까운 점 쌍 찾기 (correspondence)
2. 변환 행렬 계산 (SVD 기반)
3. 변환 적용 후 1로 돌아가 반복
4. 수렴할 때까지

| ICP 종류 | 최소화 대상 | 수렴 속도 |
|---------|-----------|---------|
| Point-to-Point | 점과 점 거리 | 느림 |
| Point-to-Plane | 점과 평면 거리 | 빠름 |

**AMR에서:** LiDAR SLAM(예: LOAM, LeGO-LOAM)이 ICP 사용

> VIO에서는 주로 **2D-2D(초기화)** 와 **3D-2D(추적)** 를 사용. ICP는 참고.

---

### 4. VO 유형 비교

| 유형 | 입력 | 장점 | 단점 | 사용 예 |
|------|------|------|------|--------|
| **Monocular** | 단안 카메라 | 간단, 저렴, 가벼움 | **스케일 모호성** | 드론, AR |
| **Stereo** | 스테레오 카메라 | 스케일 복원 가능 | 무겁고 비쌈 | 자율주행 |
| **RGB-D** | 카메라 + Depth | 직접 깊이 제공 | 실외/햇빛에 약함 | 실내 로봇 |

**스케일 모호성 (Monocular의 숙명):**
```
카메라가 본 것: "뭔가 이동했는데... 1m? 10m? 100m? 모름!"
이유: 핀홀 카메라는 3D 점 X와 lambda*X를 구분 못함 (같은 2D 점에 투영)
해결: IMU(Phase 4) 또는 Stereo
```

---

### 5. SLAM에서 어디에 쓰이나?

**VINS-Fusion 구조에서 VO의 위치:**
```
VINS-Fusion
+-- 프론트엔드 (= VO)
|   +-- feature_tracker: FAST 검출 + KLT 추적
|   +-- feature_manager: 특징점 생명주기 관리
+-- 백엔드
|   +-- optimization: Ceres BA (Visual + IMU factor)
|   +-- loop_closure: 포즈 그래프 최적화
+-- 초기화
    +-- initial_sfm: Vision-only SfM
    +-- initial_alignment: VI 정렬
```

**AMR 연결:** 
- 휠 오도메트리 ≈ Visual Odometry (둘 다 오차 누적)
- 실무에서는 휠 + Vision + IMU + LiDAR 융합

---

### 6. 드리프트 (Drift)

**비유:** 눈 감고 100걸음 걷기
```
1걸음: 1m (목표) → 실제 0.99m (1% 오차)
100걸음 후: 오차가 수십 cm로 누적!
방향 오차도 있으면 완전히 다른 곳에 도착
```

**VO에서의 드리프트 원인:**
- 특징점 매칭 오차
- 모션 추정 오차 (노이즈, outlier)
- 이미지 노이즈, 모션 블러

**해결:** SLAM의 Loop Closure / IMU 융합(VIO)

---

## [search] 자체 점검

아래 질문에 **자기 말로** 답을 노트에 써보세요. 면접에서 바로 쓸 수 있는 자산이 됩니다.

1. **"VO와 SLAM의 차이를 설명해주세요"**
   - 힌트: 프론트엔드/백엔드, Loop Closure, 드리프트

2. **"VO에서 2D-2D와 3D-2D는 각각 언제 사용하나요?"**
   - 힌트: 초기화 vs 추적, Essential Matrix vs PnP

3. **"Monocular VO의 근본적인 한계는?"**
   - 힌트: 스케일 모호성, 해결 방법

4. **"VO 파이프라인에서 가장 계산량이 많은 단계는?"**
   - 힌트: 특징점 검출 & 매칭, 실시간을 위한 선택

5. **"VINS-Fusion에서 VO는 어떤 역할을 하나요?"**
   - 힌트: feature_tracker, 프론트엔드

---

## [ref] 참고 자료

| 자료 | 용도 |
|------|------|
| Cyrill Stachniss - VO 강의 | VO 전체 흐름 |
| OpenCV `solvePnPRansac` 문서 | PnP 파라미터 이해 |
| VINS-Fusion `feature_tracker.cpp` | 실제 VO 프론트엔드 |

---

다음: [Week 2 - 키프레임 + Bundle Adjustment](../week2/README.md)
