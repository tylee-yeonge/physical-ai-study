# Week 5: 에피폴라 기하학 기초 (Epipolar Geometry)

## 📌 개요

> 🎯 **목표**: 두 뷰 사이의 기하학적 관계 이해하기
> ⏱️ **예상 시간**: 이론 3시간 + 실습 2시간

**에피폴라 기하학**은 두 카메라(또는 같은 카메라의 두 시점)에서 같은 3D 점을 볼 때 성립하는 기하학적 제약입니다. 이 제약을 이용하면 카메라 포즈를 추정할 수 있습니다.

### 🤔 왜 이걸 배워야 할까요?

**일상 비유**: 두 눈으로 보는 원리

```
왼쪽 눈 👁️                 오른쪽 눈 👁️
    │                           │
    │     ──────●──────         │
    │        물체 P              │
    ▼                           ▼
왼쪽에서 본 위치            오른쪽에서 본 위치

→ 두 눈에서 본 위치가 다름 = 깊이 인식의 기초!
```

**SLAM에서의 중요성**:
- **Visual Odometry**: E/F Matrix → 카메라 움직임 (R, t) 추정
- **초기화**: 첫 두 프레임에서 초기 맵 생성
- **루프 클로저 검증**: 매칭의 기하학적 검증

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 데모 실행 | `basic.cpp` | 빌드 후 `./basic` 실행, 결과 먼저 관찰 |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 기초 실습 | `epipolar_basics.py` | E/F 행렬 계산, 에피폴라 선 |
| 4 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | 에피폴라 제약 기본 확인 |
| 5 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | 8-point, 제약 검증 |
| 6 | Python 심화 퀴즈 | `epipolar_quiz.py` | E/F 행렬 심화 분석 |
| 7 | 카메라 실습 | [PRACTICE.md](./PRACTICE.md) | 에피폴라 기하학 실습 |

---

## 🚀 Step 1: 먼저 돌려보기

> **이론을 읽기 전에 먼저 코드를 돌려보세요!**

```bash
cd week5 && mkdir build && cd build
cmake .. && make
./basic
```

실행 결과를 관찰한 후, 아래 이론을 읽으며 "아, 이게 이 뜻이었구나" 하고 채워갑니다.

---

## 📖 핵심 개념

### 1. 에피폴라 기하학 구성요소

#### 두 카메라 시스템

```
                    P (3D 점)
                   ╱│╲
                  ╱ │ ╲
                 ╱  │  ╲
                ╱   │   ╲
    ──────────O₁────│────O₂──────────
              │     │     │
             p₁     │    p₂
           (투영)   │  (투영)
                    │
              에피폴라 평면
```

#### 핵심 용어

| 용어 | 정의 | 그림에서 |
|------|------|---------|
| **에피폴 (Epipole)** | 한 카메라 중심이 다른 이미지에 투영되는 점 | e₁, e₂ |
| **에피폴라 평면** | O₁, O₂, P를 포함하는 평면 | 삼각형 O₁-P-O₂ |
| **에피폴라 선** | 에피폴라 평면과 이미지 평면의 교선 | l₁, l₂ |

```
이미지 1                       이미지 2
┌─────────────────┐           ┌─────────────────┐
│                 │           │                 │
│     ●p₁         │           │       ●p₂       │
│      ╲          │           │      ╱          │
│       ╲ l₁      │           │  l₂ ╱           │
│        ╲        │           │    ╱            │
│         ●e₁     │           │ ●e₂             │
│                 │           │                 │
└─────────────────┘           └─────────────────┘

p₁이 주어지면, p₂는 반드시 l₂ 위에 있음!
→ 이것이 에피폴라 제약
```

---

### 2. 에피폴라 제약 (Epipolar Constraint)

#### 핵심 공식

```
p₂ᵀ · F · p₁ = 0

또는 정규화 좌표로:
x₂ᵀ · E · x₁ = 0
```

- **p**: 픽셀 좌표 [u, v, 1]ᵀ
- **x**: 정규화 좌표 [(u-cx)/fx, (v-cy)/fy, 1]ᵀ
- **F**: Fundamental Matrix (3×3)
- **E**: Essential Matrix (3×3)

**의미**: 두 대응점이 위 조건을 만족해야 올바른 매칭!

---

### 3. Essential Matrix (E)

#### 정의

캘리브레이션된 카메라 간의 기하학적 관계:

```
E = [t]× · R = t̂ · R

여기서:
- R: 회전 행렬 (3×3)
- t: 평행 이동 벡터 (3×1)
- [t]×: t의 반대칭 행렬 (Phase 1 Week 6 복습!)
```

**반대칭 행렬 (skew-symmetric)**:

```
        [  0   -tz   ty ]
[t]× =  [  tz   0   -tx ]
        [ -ty   tx   0  ]
```

#### E의 특성

| 특성 | 값 | 의미 |
|------|-----|------|
| 크기 | 3×3 | - |
| Rank | 2 | 특이 행렬 (det=0) |
| 자유도 | **5 DOF** | R(3) + t 방향(2) |
| 특이값 | σ, σ, 0 | 두 특이값이 같음 |

**왜 5 DOF?**
- R: 3 자유도
- t: 3 자유도이지만, **스케일 모호성**으로 2 자유도

```
💡 스케일 모호성:
   t와 2t는 같은 E를 생성
   → 방향만 알 수 있고, 크기는 모름
   → Monocular SLAM의 근본적 한계!
```

---

### 4. Fundamental Matrix (F)

#### 정의

캘리브레이션되지 않은 카메라 간의 관계:

```
F = K₂⁻ᵀ · E · K₁⁻¹

또는:
E = K₂ᵀ · F · K₁
```

**E vs F 비교**:

| 특성 | E (Essential) | F (Fundamental) |
|------|--------------|-----------------|
| 좌표계 | 정규화 좌표 | 픽셀 좌표 |
| 캘리브레이션 | **필요** | 불필요 |
| 자유도 | 5 DOF | 7 DOF |
| Rank | 2 | 2 |
| SLAM에서 | 주로 사용 | 루프 클로저 검증 |

---

### 5. E 행렬 계산: 8-point 알고리즘

#### 기본 원리

에피폴라 제약을 선형 시스템으로 변환:

```
x₂ᵀ · E · x₁ = 0

→ [x₂·x₁, x₂·y₁, x₂, y₂·x₁, y₂·y₁, y₂, x₁, y₁, 1] · e = 0

e = [E₁₁, E₁₂, E₁₃, E₂₁, E₂₂, E₂₃, E₃₁, E₃₂, E₃₃]ᵀ
```

N개 대응점으로 Ae = 0 형태:
- 8개 이상 필요 (8-point algorithm)
- SVD로 해 구함

#### Normalized 8-point

좌표 정규화로 수치 안정성 향상:

```python
# 1. 점 정규화 (평균=0, 평균 거리=√2)
mean = np.mean(points, axis=0)
scale = np.sqrt(2) / np.mean(np.linalg.norm(points - mean, axis=1))
T = [[scale, 0, -scale*mean[0]],
     [0, scale, -scale*mean[1]],
     [0, 0, 1]]

# 2. 정규화된 점으로 E 계산
# 3. 역정규화: E = T₂ᵀ · E_norm · T₁
```

#### OpenCV 구현

```python
import cv2
import numpy as np

# 대응점
pts1 = np.float32(points_img1)  # (N, 2)
pts2 = np.float32(points_img2)  # (N, 2)

# Essential Matrix (캘리브레이션 필요)
E, mask = cv2.findEssentialMat(
    pts1, pts2, K,
    method=cv2.RANSAC,
    prob=0.999,
    threshold=1.0
)

# Fundamental Matrix (캘리브레이션 불필요)
F, mask = cv2.findFundamentalMat(
    pts1, pts2,
    method=cv2.FM_RANSAC,
    ransacReprojThreshold=3.0
)
```

---

### 6. 5-point 알고리즘

#### E의 제약 활용

E는 5 DOF → 이론상 5점으로 충분!

**장점**:
- 더 적은 점 필요
- RANSAC 반복 감소
- 더 정확 (제약 완전 활용)

**단점**:
- 복잡한 다항식 해
- 여러 해 존재 가능 (최대 10개)

```python
# OpenCV는 5-point도 지원
E, mask = cv2.findEssentialMat(
    pts1, pts2, K,
    method=cv2.RANSAC,
    prob=0.999,
    threshold=1.0
)
# 내부적으로 5-point 또는 8-point 사용
```

---

### 7. 에피폴라 선 시각화

#### 점에서 에피폴라 선 계산

```python
# p₂의 에피폴라 선 (이미지 1에서)
l₁ = F.T @ p₂  # l₁ = Fᵀ·p₂

# p₁의 에피폴라 선 (이미지 2에서)
l₂ = F @ p₁   # l₂ = F·p₁

# 선의 방정식: ax + by + c = 0
# l = [a, b, c]
```

```python
def draw_epipolar_line(img, line, color=(0, 255, 0)):
    """에피폴라 선 그리기"""
    h, w = img.shape[:2]
    a, b, c = line
    
    # 선이 이미지 경계와 만나는 점
    if abs(b) > 1e-6:
        x0, y0 = 0, int(-c / b)
        x1, y1 = w, int(-(c + a*w) / b)
    else:
        x0, y0 = int(-c / a), 0
        x1, y1 = int(-c / a), h
    
    cv2.line(img, (x0, y0), (x1, y1), color, 1)
```

---

## 💻 실습 파일

| 파일 | 내용 | 난이도 |
|------|------|--------|
| `epipolar_basics.py` | E/F 행렬 계산, 에피폴라 선 | ⭐⭐⭐ |
| `epipolar_quiz.py` | 8-point, 제약 검증 | ⭐⭐⭐ |

---

## 📊 핵심 정리

### E vs F 선택 가이드

```
캘리브레이션 됨?
     │
     ├── Yes ──▶ Essential Matrix (E)
     │            - 5 DOF
     │            - R, t 분해 가능
     │            - SLAM 주로 사용
     │
     └── No ───▶ Fundamental Matrix (F)
                  - 7 DOF
                  - 픽셀 좌표 사용
                  - 기하학적 검증용
```

### 공식 요약

| 관계 | 공식 |
|------|------|
| 에피폴라 제약 (E) | **x₂ᵀ E x₁ = 0** |
| 에피폴라 제약 (F) | **p₂ᵀ F p₁ = 0** |
| E 정의 | E = [t]× R |
| E ↔ F 관계 | F = K₂⁻ᵀ E K₁⁻¹ |
| 에피폴라 선 | l₂ = F p₁, l₁ = Fᵀ p₂ |

---

## ✅ 학습 완료 체크리스트

### 기초 이해 (필수)
- [ ] 에피폴, 에피폴라 선/평면 설명 가능
- [ ] E와 F의 차이 설명 가능
- [ ] 에피폴라 제약 의미 설명 가능

### 실용 활용 (권장)
- [ ] OpenCV로 E/F 계산 가능
- [ ] 에피폴라 선 시각화 가능
- [ ] RANSAC + E/F 조합 이해

### 심화 (선택)
- [ ] 8-point 알고리즘 유도 이해
- [ ] E의 특이값 분해 의미 이해
- [ ] 5-point vs 8-point 차이 이해

---

## 🔗 다음 단계

### Week 6: 포즈 추정 (R, t 분해)

E 행렬에서 카메라 움직임 복원:
- SVD로 R, t 분해
- 4가지 해 → Cheirality Check
- 실제 해 선택

---

## 📚 참고 자료

- Multiple View Geometry (Hartley & Zisserman) - Chapter 9
- OpenCV Epipolar Geometry Tutorial
- First Principles of Computer Vision (YouTube)

---

## ❓ FAQ

**Q1: 왜 E는 5 DOF인데 8-point를 쓰나요?**
A: 8-point는 제약 없이 선형 풀이. 5-point는 제약 활용하지만 비선형. RANSAC에서 5-point가 효율적.

**Q2: 스케일 모호성은 어떻게 해결하나요?**
A: Monocular는 해결 불가 (상대 스케일만). Stereo나 IMU 융합으로 해결.

**Q3: E를 알면 F도 알 수 있나요?**
A: Yes! F = K₂⁻ᵀ E K₁⁻¹ (K가 있으면)

**Q4: 매칭이 틀리면 E도 틀리나요?**
A: Yes! RANSAC으로 outlier 제거 필수.

---

**🎯 Week 5 핵심 메시지:**

> 에피폴라 기하학 = 두 뷰의 기하학적 관계
> 
> **x₂ᵀ E x₁ = 0** → 카메라 움직임 추정의 시작!
