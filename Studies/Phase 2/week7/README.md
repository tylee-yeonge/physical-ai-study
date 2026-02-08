# Week 7: 삼각측량과 PnP (Triangulation & PnP)

## 📌 개요

> 🎯 **목표**: 2D 대응점에서 3D 점 복원, 3D-2D 대응에서 카메라 포즈 추정
> ⏱️ **예상 시간**: 이론 3시간 + 실습 2시간

**삼각측량(Triangulation)**은 두 카메라에서 같은 점을 보고 그 점의 3D 위치를 복원하는 기술입니다.
**PnP(Perspective-n-Point)**는 알려진 3D 점과 그 2D 투영에서 카메라 포즈를 추정합니다.

### 🤔 왜 이걸 배워야 할까요?

**일상 비유**: 

```
삼각측량:                    PnP:
  두 눈으로 물체 거리 파악       지도상 랜드마크로 내 위치 파악

     P ●                      🗼 🏛️ 랜드마크 (3D 알려짐)
    ╱ ╲                         ╲ ╱
   ╱   ╲                         ╲╱
  👁️    👁️                       📷
 왼쪽    오른쪽               → 카메라 위치?
```

**SLAM에서의 사용**:
- **맵 구축**: 삼각측량으로 3D 맵 포인트 생성
- **추적**: PnP로 새 프레임의 포즈 추정
- **재위치화**: 맵과 현재 관측으로 위치 복구

---

## 🚀 먼저 돌려보기 (원칙 1: 역순 학습)

> **이론을 읽기 전에 먼저 코드를 돌려보세요!**

```bash
cd week7 && mkdir build && cd build
cmake .. && make
./basic
```

실행 결과를 관찰한 후, 아래 이론을 읽으며 "아, 이게 이 뜻이었구나" 하고 채워갑니다.

- 📂 [PRACTICE.md](./PRACTICE.md) - 실습 가이드
- 📝 `quiz_easy.cpp` → `quiz_medium.cpp` 순서로 풀기

---

## 📖 핵심 개념

### 1. 삼각측량 (Triangulation)

#### 기본 원리

두 카메라에서 같은 3D 점을 관측하면, 광선의 교점이 그 점:

```
        ● P (3D 점)
       ╱│╲
      ╱ │ ╲ 광선 2
 광선1  │  ╲
    ╱   │   ╲
   ●────┼────●
  C₁    │    C₂
        │
    베이스라인
```

**이상적**: 두 광선이 정확히 한 점에서 만남
**실제**: 노이즈로 인해 만나지 않음 → 최소 거리점 추정

#### DLT 삼각측량 (Direct Linear Transform)

각 카메라의 투영 방정식:
```
p₁ = P₁ · X    (카메라 1)
p₂ = P₂ · X    (카메라 2)

여기서:
- p = [u, v, 1]ᵀ (동차 이미지 좌표)
- P = K[R|t] (3×4 투영 행렬)
- X = [X, Y, Z, 1]ᵀ (동차 3D 좌표)
```

**크로스 곱 제거**:
```
p × (P · X) = 0

전개:
u(P₃ᵀX) - (P₁ᵀX) = 0
v(P₃ᵀX) - (P₂ᵀX) = 0
```

**선형 시스템**:
```
A · X = 0

    ⎡ u₁P₁³ᵀ - P₁¹ᵀ ⎤
A = ⎢ v₁P₁³ᵀ - P₁²ᵀ ⎥  (4×4 행렬)
    ⎢ u₂P₂³ᵀ - P₂¹ᵀ ⎥
    ⎣ v₂P₂³ᵀ - P₂²ᵀ ⎦

SVD로 해: A의 null space (가장 작은 특이값)
```

#### OpenCV 삼각측량

```python
import cv2
import numpy as np

# 투영 행렬
P1 = K @ np.hstack([np.eye(3), np.zeros((3,1))])  # [I|0]
P2 = K @ np.hstack([R, t.reshape(3,1)])           # [R|t]

# 삼각측량
pts1 = pts1.T  # (2, N)
pts2 = pts2.T  # (2, N)

points_4d = cv2.triangulatePoints(P1, P2, pts1, pts2)
points_3d = points_4d[:3] / points_4d[3]  # 동차 → 유클리드
```

---

### 2. 삼각측량 정확도

#### 기저선(Baseline) 효과

```
좁은 기저선 (나쁨):           넓은 기저선 (좋음):

      ● P                          ● P
     ╱╲                          ╱   ╲
    ╱  ╲ 작은 각도              ╱     ╲ 큰 각도
   ●────●                      ●───────●
  C₁    C₂                    C₁       C₂

→ 깊이 오차 큼               → 깊이 오차 작음
```

**경험칙**: 
- 삼각측량 각도 > 5° 권장
- 너무 크면 (>60°) 매칭 어려움

#### 깊이 불확실성

```
σ_Z ∝ Z² / (B · f)

여기서:
- Z: 깊이
- B: 베이스라인
- f: 초점 거리

→ 깊이가 멀수록, 베이스라인이 좁을수록 불확실
```

---

### 3. PnP (Perspective-n-Point)

#### 문제 정의

**주어진 것**:
- N개 3D 점 좌표 (맵에서)
- 해당 점들의 2D 투영 (현재 이미지)
- 카메라 내부 파라미터 K

**구하고자 하는 것**:
- 카메라 포즈 (R, t)

```
알려진 3D 점들          현재 카메라 뷰
   ● P₁                    ● p₁
   ● P₂        ────▶       ● p₂
   ● P₃       PnP로 포즈   ● p₃
   ● P₄                    ● p₄

3D-2D 대응 → 카메라 위치/방향
```

#### PnP 알고리즘들

| 알고리즘 | 최소 점 | 특징 |
|---------|--------|------|
| P3P | 3점 | 최소 해, 최대 4개 해 |
| DLT | 6점+ | 선형, 초기값 불필요 |
| EPnP | 4점+ | 빠름, 정확 |
| **Iterative** | 3점+ | 가장 정확, 초기값 필요 |

#### OpenCV PnP

```python
import cv2

# 3D 점 (맵)
object_points = np.array([
    [0, 0, 5],
    [1, 0, 5],
    [0, 1, 5],
    [1, 1, 5],
    ...
], dtype=np.float32)

# 2D 점 (이미지)
image_points = np.array([
    [320, 240],
    [380, 240],
    [320, 300],
    [380, 300],
    ...
], dtype=np.float32)

# PnP 풀기
success, rvec, tvec = cv2.solvePnP(
    object_points, 
    image_points, 
    K, 
    dist_coeffs,
    flags=cv2.SOLVEPNP_ITERATIVE
)

# rvec → 회전 행렬
R, _ = cv2.Rodrigues(rvec)

print(f"R:\n{R}")
print(f"t: {tvec.flatten()}")
```

#### PnP + RANSAC

outlier 제거를 위해:

```python
success, rvec, tvec, inliers = cv2.solvePnPRansac(
    object_points,
    image_points,
    K,
    dist_coeffs,
    reprojectionError=3.0,
    confidence=0.99
)

print(f"Inliers: {len(inliers)} / {len(object_points)}")
```

---

### 4. SLAM에서의 흐름

#### Visual Odometry 파이프라인

```
프레임 1      프레임 2      프레임 3      ...
    │            │            │
    ▼            ▼            ▼
 특징점 검출   특징점 검출   특징점 검출
    │            │            │
    └─── 매칭 ───┘            │
           │                   │
           ▼                   │
    E → R₁₂, t₁₂             │
           │                   │
           ▼                   │
    삼각측량 → 3D 점들        │
              │               │
              └─── 매칭 ──────┘
                      │
                      ▼
              PnP → R₂₃, t₂₃
```

**단계별**:
1. **초기화**: 프레임 1-2에서 E 분해 + 삼각측량
2. **추적**: 프레임 N에서 맵과 PnP
3. **확장**: 새 점 삼각측량

#### VINS-Fusion에서의 사용

```cpp
// vins_estimator/src/initial/initial_sfm.cpp

// 1. 삼각측량
void GlobalSFM::triangulatePoint(...)

// 2. PnP
bool GlobalSFM::solveFrameByPnP(...)
```

---

## 💻 실습 파일

| 파일 | 내용 | 난이도 |
|------|------|--------|
| `triangulation_basics.py` | DLT 삼각측량, 정확도 분석 | ⭐⭐⭐ |
| `pnp_quiz.py` | PnP 구현, RANSAC | ⭐⭐⭐ |

---

## 📊 핵심 정리

### 삼각측량 vs PnP

| | 삼각측량 | PnP |
|---|---------|-----|
| 입력 | 2D-2D (두 뷰) | 3D-2D (맵-이미지) |
| 출력 | 3D 점 | 카메라 포즈 |
| 용도 | 맵 구축 | 추적/재위치화 |
| 최소 점 | 1점 (2뷰) | 3점 |

### 권장 설정

```python
# 삼각측량
cv2.triangulatePoints(P1, P2, pts1, pts2)

# PnP (RANSAC 권장)
cv2.solvePnPRansac(
    obj_pts, img_pts, K, dist,
    reprojectionError=3.0,
    confidence=0.99,
    flags=cv2.SOLVEPNP_ITERATIVE
)
```

---

## ✅ 학습 완료 체크리스트

### 기초 이해 (필수)
- [ ] 삼각측량 원리 설명 가능
- [ ] PnP가 무엇인지 설명 가능
- [ ] 베이스라인이 정확도에 미치는 영향 이해

### 실용 활용 (권장)
- [ ] cv2.triangulatePoints() 사용 가능
- [ ] cv2.solvePnPRansac() 사용 가능
- [ ] 재투영 오차로 품질 평가 가능

### 심화 (선택)
- [ ] DLT 유도 이해
- [ ] EPnP / P3P 차이 이해
- [ ] VINS SFM 코드 흐름 분석

---

## 🔗 다음 단계

### Week 8: 광류 (Optical Flow)

특징점 없이 움직임 추정:
- Lucas-Kanade Optical Flow
- 특징점 추적
- VINS feature_tracker 분석

---

## 📚 참고 자료

- Multiple View Geometry - Chapter 12 (Triangulation)
- OpenCV solvePnP documentation
- VINS-Fusion initial_sfm.cpp

---

## ❓ FAQ

**Q1: 삼각측량된 점 품질을 어떻게 판단하나요?**
A: 재투영 오차로. 3D → 2D 재투영 후 원래 2D점과 비교.

**Q2: PnP가 실패하는 경우?**
A: 공면 점들(4점 이상), outlier 많음, 시야 밖 점.

**Q3: P3P vs EPnP?**
A: P3P는 정확히 3점, 최대 4해. EPnP는 4점 이상, 빠르고 정확.

**Q4: 삼각측량 각도가 작으면?**
A: 깊이 불확실도 증가. 5° 이상 권장.

---

**🎯 Week 7 핵심 메시지:**

> **삼각측량** = 2D → 3D (맵 구축)
> **PnP** = 3D-2D → 포즈 (추적)
> 
> 둘 다 SLAM의 핵심 연산!
