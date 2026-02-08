# Week 3: 특징점 검출 (Feature Detection)

## 📌 개요

> 🎯 **목표**: 이미지에서 추적/매칭 가능한 점 검출하기
> ⏱️ **예상 시간**: 이론 2시간 + 실습 3시간

**특징점(Feature)**은 이미지에서 독특하게 식별 가능한 점입니다. SLAM에서는 이 특징점을 프레임 간에 추적하거나 매칭하여 카메라 움직임을 추정합니다.

### 🤔 왜 이걸 배워야 할까요?

**일상 비유**: 낯선 도시에서 길을 찾을 때 뭘 보나요?

- ❌ 비슷하게 생긴 건물들 → 헷갈림
- ✅ 눈에 띄는 랜드마크 (타워, 특이한 조각상) → 위치 파악 가능!

**SLAM에서도 마찬가지**:
- 평평한 벽 → 추적 불가 (어디가 어딘지 구분 안 됨)
- 모서리, 패턴 → 추적 가능 (고유하게 식별)

**VINS-Fusion에서의 활용**:
- `feature_tracker` 노드가 FAST 코너 검출
- 검출된 특징점을 프레임 간 추적
- 이 정보로 카메라 포즈 추정

---

## 🚀 먼저 돌려보기 (원칙 1: 역순 학습)

> **이론을 읽기 전에 먼저 코드를 돌려보세요!**

```bash
cd week3 && mkdir build && cd build
cmake .. && make
./basic
```

실행 결과를 관찰한 후, 아래 이론을 읽으며 "아, 이게 이 뜻이었구나" 하고 채워갑니다.

- 📂 [PRACTICE.md](./PRACTICE.md) - 실습 가이드
- 📝 `quiz_easy.cpp` → `quiz_medium.cpp` 순서로 풀기

---

## 📖 핵심 개념

### 1. 좋은 특징점의 조건

#### 특징점이 되려면?

```
❌ 나쁜 특징점 (추적 어려움)           ✅ 좋은 특징점 (추적 가능)

┌──────────────┐                   ┌──────────────┐
│              │                   │    ●──       │
│   (빈 영역)    │                   │   /   \      │
│              │                   │  ●     ●     │
│              │                   │   \   /      │
│              │                   │    ●──       │
└──────────────┘                   └──────────────┘
  균일한 영역                         코너/교차점
```

**좋은 특징점의 3가지 조건**:

| 조건 | 설명 | 예시 |
|------|------|------|
| **반복성** (Repeatability) | 다른 뷰에서도 검출 | 같은 코너가 계속 보임 |
| **구별성** (Distinctiveness) | 주변과 구별됨 | 고유한 패턴 |
| **위치 정확도** | 정확한 위치 검출 | 서브픽셀 정밀도 |

---

### 2. Harris Corner Detector

#### 아이디어

윈도우를 이동시킬 때 **모든 방향으로 밝기 변화**가 있으면 코너!

```
    ← → ↑ ↓ 모든 방향으로 이동

  에지 (한 방향만 변화)    코너 (모든 방향 변화)    플랫 (변화 없음)
       ←→                    ↗↘                    
  ┌─────────┐              ┌──┴──┐              ┌─────────┐
  │ │ │ │ │ │              │  ╲  │              │         │
  │ │ │ │ │ │              │───● │              │         │
  │ │ │ │ │ │              │  ╱  │              │         │
  └─────────┘              └─────┘              └─────────┘
  
  한 방향만 변화            모든 방향 변화         변화 없음
```

#### 수학적 정의

**자기 상관 함수** (Sum of Squared Differences):

```
E(u,v) = Σ w(x,y) [I(x+u, y+v) - I(x, y)]²
           (x,y)
```

- w(x,y): 윈도우 함수 (가우시안)
- I(x,y): 이미지 밝기
- (u,v): 윈도우 이동량

**테일러 전개로 근사**:

```
E(u,v) ≈ [u, v] · M · [u]
                      [v]

    ⎡ Σ Ix²     Σ IxIy ⎤
M = ⎢                  ⎥  (Structure Tensor)
    ⎣ Σ IxIy   Σ Iy²  ⎦
```

- Ix, Iy: 이미지 그래디언트 (미분)

#### Harris 응답 함수

```
R = det(M) - k · trace(M)²
  = λ₁λ₂ - k(λ₁ + λ₂)²

여기서:
- λ₁, λ₂: M의 고유값
- k: 파라미터 (보통 0.04~0.06)
```

**고유값으로 해석**:

| λ₁, λ₂ | 해석 | R 값 |
|--------|------|------|
| 둘 다 작음 | 플랫 영역 | ≈ 0 |
| 하나만 큼 | 에지 | < 0 |
| 둘 다 큼 | **코너** | **>> 0** |

#### Python 구현 (개념 이해용)

```python
import cv2
import numpy as np

def harris_corners(image, k=0.04, threshold=0.01):
    # 그래디언트 계산
    Ix = cv2.Sobel(image, cv2.CV_64F, 1, 0, ksize=3)
    Iy = cv2.Sobel(image, cv2.CV_64F, 0, 1, ksize=3)
    
    # Structure Tensor 요소
    Ixx = Ix * Ix
    Iyy = Iy * Iy
    Ixy = Ix * Iy
    
    # 가우시안 스무딩
    Ixx = cv2.GaussianBlur(Ixx, (5, 5), 0)
    Iyy = cv2.GaussianBlur(Iyy, (5, 5), 0)
    Ixy = cv2.GaussianBlur(Ixy, (5, 5), 0)
    
    # Harris 응답
    det = Ixx * Iyy - Ixy * Ixy
    trace = Ixx + Iyy
    R = det - k * trace * trace
    
    # 임계값 적용
    corners = R > threshold * R.max()
    
    return R, corners
```

---

### 3. FAST 코너 검출

#### 왜 FAST인가?

**Features from Accelerated Segment Test**

| 알고리즘 | 속도 | 용도 |
|---------|------|------|
| Harris | 느림 | 정확도 중시 |
| **FAST** | **매우 빠름** | **실시간 SLAM** |

**VINS-Fusion이 FAST를 사용하는 이유**: 실시간 성능!

#### 알고리즘 원리

**중심 픽셀 주위 16개 픽셀 검사**:

```
        1  2  3
     16         4
    15    ●     5
    14          6
    13  12 11 10 9
         8  7

● = 코너 후보 (중심)
1-16 = 주변 픽셀 (Bresenham 원)
```

**코너 조건**:
- 16개 중 **연속 N개** (보통 9~12개)가
- 중심보다 모두 밝거나 (`I_p + t`)
- 중심보다 모두 어두움 (`I_p - t`)

```
if 연속 N개 픽셀이 모두 (center + threshold) 보다 밝음
   OR
   연속 N개 픽셀이 모두 (center - threshold) 보다 어두움:
   → 코너!
```

#### 고속 검사 (High-speed test)

먼저 1, 5, 9, 13번 (상하좌우) 4개만 검사:
- 이 4개 중 3개 이상 만족 안 하면 → 코너 아님 (빠르게 제외)

```python
# OpenCV FAST
fast = cv2.FastFeatureDetector_create(threshold=20)
keypoints = fast.detect(gray_image)
```

#### Non-maximum Suppression (NMS)

검출된 코너가 너무 많으면 → 지역 최대값만 남김

```
전                후 (NMS)
●●●               
●●● ●  ──────▶   ●     ●
●●●
```

---

### 4. 디스크립터 (Descriptor)

#### 왜 디스크립터가 필요한가?

**문제**: 특징점 위치만으로는 **매칭 불가**!

```
이미지 1                    이미지 2
   ●A                          ●?
      ●B                    ●?    ●?
   ●C                          ●?

어떤 점이 어느 점에 대응하는지 모름!
```

**해결**: 각 특징점 주변 **패턴을 숫자로 표현** = 디스크립터

```
특징점 A → [0.2, 0.5, 0.1, ...]  (벡터)
특징점 B → [0.8, 0.1, 0.3, ...]
...

비슷한 벡터 = 같은 점!
```

#### BRIEF 디스크립터

**Binary Robust Independent Elementary Features**

- **이진 디스크립터**: 0과 1로만 구성
- **빠른 계산**: 픽셀 쌍 비교만 필요
- **빠른 매칭**: 해밍 거리 (XOR 연산)

**계산 방법**:

```
패치 내 랜덤하게 선택된 픽셀 쌍 (p1, p2)

τ(p; p1, p2) = { 1  if I(p1) < I(p2)
               { 0  otherwise

256개 쌍 → 256비트 = 32바이트 디스크립터
```

#### ORB = FAST + BRIEF + 회전 불변성

**Oriented FAST and Rotated BRIEF**

ORB는 FAST + BRIEF에 **회전 불변성**을 추가:

1. **Oriented FAST**: 특징점 방향 계산 (Intensity Centroid)
2. **Rotated BRIEF**: 방향에 맞게 BRIEF 패턴 회전

```python
# OpenCV ORB
orb = cv2.ORB_create(nfeatures=500)
keypoints, descriptors = orb.detectAndCompute(gray_image, None)

# keypoints: 특징점 위치
# descriptors: (N, 32) 이진 디스크립터
```

---

### 5. 특징점 비교

| 알고리즘 | 검출 속도 | 디스크립터 | 회전 불변 | 스케일 불변 | SLAM 사용 |
|---------|----------|-----------|----------|-----------|---------|
| Harris | 느림 | 없음 | ❌ | ❌ | 드물게 |
| **FAST** | **매우 빠름** | 없음 | ❌ | ❌ | **VINS** |
| BRIEF | - | 이진(32B) | ❌ | ❌ | - |
| **ORB** | 빠름 | 이진(32B) | ✅ | △ | **ORB-SLAM** |
| SIFT | 매우 느림 | 실수(128D) | ✅ | ✅ | 오프라인 |
| SURF | 느림 | 실수(64D) | ✅ | ✅ | 드물게 |

**SLAM에서 선택**:
- **실시간 추적**: FAST (검출만) + KLT (추적) → **VINS**
- **루프 클로저**: ORB (디스크립터 필요) → **ORB-SLAM**

---

### 6. SLAM에서의 활용

#### VINS-Fusion feature_tracker

```
┌────────────────────────────────────────────┐
│           VINS feature_tracker             │
├────────────────────────────────────────────┤
│                                            │
│  새 프레임 ───▶ FAST 검출 ───▶ 특징점들          │
│                    ↓                       │
│             기존 특징점 추적                   │
│             (Lucas-Kanade)                 │
│                    ↓                       │
│             추적 실패한 점 제거                 │
│                    ↓                       │
│             새 특징점 추가                    │
│             (개수 유지)                      │
│                                            │
└────────────────────────────────────────────┘
```

#### ORB-SLAM3

```
┌────────────────────────────────────────────┐
│              ORB-SLAM3                     │
├────────────────────────────────────────────┤
│                                            │
│  새 프레임 ───▶ ORB 검출 ───▶ 디스크립터         │
│                    ↓                       │
│             기존 맵과 매칭                    │
│             (디스크립터 비교)                  │
│                    ↓                       │
│             카메라 포즈 추정                   │
│             (PnP)                          │
│                                            │
└────────────────────────────────────────────┘
```

---

## 💻 실습 파일

| 파일 | 내용 | 난이도 |
|------|------|--------|
| `feature_detection_basics.py` | Harris, FAST, ORB 구현 | ⭐⭐ |
| `feature_detection_quiz.py` | 파라미터 튜닝, 비교 | ⭐⭐⭐ |

---

## 📊 핵심 정리

### 알고리즘 선택 가이드

```
실시간 필요?
    │
    ├── Yes ──▶ 디스크립터 필요?
    │               │
    │               ├── Yes ──▶ ORB
    │               │
    │               └── No ───▶ FAST + KLT 추적
    │
    └── No ───▶ 정확도 우선?
                    │
                    ├── Yes ──▶ SIFT/SURF
                    │
                    └── No ───▶ ORB
```

### 파라미터 가이드

| 파라미터 | 높이면 | 낮추면 |
|---------|--------|--------|
| FAST threshold | 적은 검출, 강한 코너만 | 많은 검출, 약한 코너도 |
| ORB nfeatures | 많은 특징점, 느림 | 적은 특징점, 빠름 |
| NMS 윈도우 | 균일 분포 | 밀집 분포 |

---

## ✅ 학습 완료 체크리스트

### 기초 이해 (필수)
- [ ] 좋은 특징점의 조건 3가지 설명 가능
- [ ] Harris와 FAST의 차이 설명 가능
- [ ] 디스크립터가 왜 필요한지 설명 가능

### 실용 활용 (권장)
- [ ] OpenCV로 FAST, ORB 검출 가능
- [ ] 파라미터 튜닝 효과 이해
- [ ] 검출 결과 시각화 가능

### 심화 (선택)
- [ ] Harris 응답 함수 유도 가능
- [ ] NMS 구현 가능
- [ ] VINS feature_tracker 코드 흐름 이해

---

## 🔗 다음 단계

### Week 4: 특징점 매칭 (Feature Matching)

검출된 특징점을 다른 이미지에서 찾기:
- Brute-Force / FLANN 매칭
- Lowe's Ratio Test
- RANSAC으로 outlier 제거

---

## 📚 참고 자료

- OpenCV Feature Detection Tutorial
- ORB 논문: "ORB: an efficient alternative to SIFT or SURF"
- VINS-Fusion feature_tracker 코드

---

## ❓ FAQ

**Q1: FAST가 Harris보다 왜 빠른가요?**
A: Harris는 윈도우 내 모든 픽셀의 그래디언트를 계산하지만, FAST는 16개 픽셀만 비교합니다.

**Q2: 특징점이 너무 적게 검출되면?**
A: threshold를 낮추거나, 이미지 전처리 (히스토그램 균등화) 적용.

**Q3: ORB vs SIFT, 언제 뭘 쓰나요?**
A: 실시간이면 ORB, 오프라인 정밀 작업이면 SIFT.

**Q4: VINS에서 ORB 안 쓰는 이유?**
A: VINS는 매칭 대신 **추적** (Optical Flow)을 사용해서 디스크립터가 불필요합니다.

---

**🎯 Week 3 핵심 메시지:**

> 특징점 = SLAM의 눈
> 
> **FAST**로 빠르게 검출, **ORB**로 매칭 가능하게!
