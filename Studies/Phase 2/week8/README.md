# Week 8: 광류 (Optical Flow)

## 📌 개요

> 🎯 **목표**: 픽셀 단위 움직임 추정, 특징점 추적 이해하기
> ⏱️ **예상 시간**: 이론 2시간 + 실습 3시간

**광류(Optical Flow)**는 연속된 프레임 사이에서 **픽셀의 움직임**을 추정하는 기술입니다. 특징점 매칭 없이 **추적**을 가능하게 하여 실시간 SLAM에 핵심적입니다.

### 🤔 왜 이걸 배워야 할까요?

**일상 비유**: 달리는 차에서 창밖 풍경

```
정지                        주행 시
┌─────────────┐           ┌─────────────┐
│  🌲   🏠   │           │ 🌲→  🏠→   │
│      🚗    │  ──────▶  │      🚗    │
│  🌳       │           │ 🌳→       │
└─────────────┘           └─────────────┘

물체들이 움직여 보임 = 광류!
→ 내 움직임 정보를 담고 있음
```

**SLAM에서의 중요성**:
- **VINS-Fusion**: Lucas-Kanade로 특징점 추적
- **매칭보다 빠름**: 디스크립터 계산 불필요
- **연속성 활용**: 프레임 간 작은 움직임 가정

---

## 📖 핵심 개념

### 1. 광류란?

#### 정의

인접 프레임 사이 각 픽셀의 **2D 속도 벡터**

```
시간 t                    시간 t+Δt
┌─────────────┐           ┌─────────────┐
│      ●(x,y) │           │     ●(x+Δx, │
│             │  ──────▶  │       y+Δy) │
│             │           │             │
└─────────────┘           └─────────────┘

광류 = (Δx/Δt, Δy/Δt) = (u, v)
```

#### 광류 vs 움직임 장(Motion Field)

| | 광류 | 움직임 장 |
|---|------|----------|
| 정의 | 이미지 밝기 변화 | 실제 3D 움직임의 투영 |
| 측정 | 이미지에서 계산 | 3D 정보 필요 |
| 관계 | 근사치 | 진짜 값 |

**주의**: 밝기 변화 없으면 광류 = 0 (실제 움직임 있어도!)

---

### 2. 밝기 항상성 가정 (Brightness Constancy)

#### 핵심 가정

**같은 점은 시간이 지나도 밝기가 같다**

```
I(x, y, t) = I(x + Δx, y + Δy, t + Δt)
```

#### 광류 방정식 유도

테일러 전개:
```
I(x + Δx, y + Δy, t + Δt) ≈ I(x,y,t) + Iₓ·Δx + Iᵧ·Δy + Iₜ·Δt
```

밝기 항상성에서:
```
Iₓ·Δx + Iᵧ·Δy + Iₜ·Δt = 0

양변을 Δt로 나누면:
Iₓ·u + Iᵧ·v + Iₜ = 0

벡터 형태:
∇I · [u, v]ᵀ = -Iₜ
```

여기서:
- Iₓ, Iᵧ: 공간 그래디언트
- Iₜ: 시간 그래디언트
- u, v: 광류 (구하고자 하는 값)

#### 조리개 문제 (Aperture Problem)

**1개 방정식, 2개 미지수 (u, v)** → 해 무한!

```
에지에서:                  코너에서:
    │                         ┌──
    │ ← 어느 방향?           │
    │                         │
                             명확!
```

**해결**: 추가 가정 필요
- Lucas-Kanade: 지역 일정 가정
- Horn-Schunck: 전역 부드러움 가정

---

### 3. Lucas-Kanade 방법

#### 핵심 아이디어

**작은 윈도우 내 모든 픽셀의 광류가 같다고 가정**

```
윈도우 (예: 21×21)
┌─────────────┐
│ ● ● ● ● ●  │
│ ● ● ● ● ●  │ → 모든 (u, v) 동일
│ ● ● ● ● ●  │
│ ● ● ● ● ●  │
└─────────────┘
```

#### 과잉결정 시스템

윈도우 내 N개 픽셀에서:
```
Iₓ₁·u + Iᵧ₁·v = -Iₜ₁
Iₓ₂·u + Iᵧ₂·v = -Iₜ₂
...
IₓN·u + IᵧN·v = -IₜN
```

행렬 형태:
```
A · [u, v]ᵀ = b

    ⎡ Iₓ₁  Iᵧ₁ ⎤       ⎡ -Iₜ₁ ⎤
A = ⎢ Iₓ₂  Iᵧ₂ ⎥,  b = ⎢ -Iₜ₂ ⎥
    ⎣ ...  ... ⎦       ⎣ ...  ⎦
```

**최소제곱 해**:
```
[u, v]ᵀ = (AᵀA)⁻¹ Aᵀb
```

#### AᵀA 행렬

```
        ⎡ ΣIₓ²    ΣIₓIᵧ ⎤
AᵀA =   ⎢               ⎥  = Structure Tensor!
        ⎣ ΣIₓIᵧ  ΣIᵧ²  ⎦
```

**Harris 코너와 같은 행렬!**
- 고유값 둘 다 큼 → 좋은 추적 (코너)
- 하나만 큼 → 에지 (추적 불안정)
- 둘 다 작음 → 플랫 (추적 불가)

#### OpenCV Lucas-Kanade

```python
import cv2
import numpy as np

# 이전 프레임에서 특징점 검출
prev_pts = cv2.goodFeaturesToTrack(
    prev_gray, 
    maxCorners=200, 
    qualityLevel=0.01, 
    minDistance=30
)

# Lucas-Kanade 추적
next_pts, status, error = cv2.calcOpticalFlowPyrLK(
    prev_gray, 
    next_gray, 
    prev_pts, 
    None,
    winSize=(21, 21),           # 윈도우 크기
    maxLevel=3,                  # 피라미드 레벨
    criteria=(cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 30, 0.01)
)

# status[i] = 1: 추적 성공
good_prev = prev_pts[status == 1]
good_next = next_pts[status == 1]
```

---

### 4. 피라미드 LK (Multi-scale)

#### 왜 피라미드가 필요한가?

**큰 움직임 처리**:
- LK는 작은 움직임만 추적 가능
- 피라미드로 큰 움직임도 처리

```
원본 (Level 0)     ────▶  움직임: 20px
    ↓ 축소
Level 1            ────▶  움직임: 10px
    ↓ 축소
Level 2            ────▶  움직임: 5px  (추적 가능!)
```

**과정**:
1. 가장 작은 해상도에서 시작
2. 광류 추정
3. 상위 레벨로 전파 + 정제
4. 원본 해상도까지 반복

---

### 5. 조밀 광류 (Dense Optical Flow)

#### Sparse vs Dense

| | Sparse (LK) | Dense (Farneback 등) |
|---|------------|----------------------|
| 계산 대상 | 특정 점만 | 모든 픽셀 |
| 속도 | 빠름 | 느림 |
| 용도 | 특징점 추적 | 움직임 분석 |
| SLAM | **VINS 사용** | 드물게 사용 |

#### Farneback Dense Flow

```python
# Dense Optical Flow
flow = cv2.calcOpticalFlowFarneback(
    prev_gray, next_gray, None,
    pyr_scale=0.5,    # 피라미드 스케일
    levels=3,          # 피라미드 레벨
    winsize=15,        # 윈도우 크기
    iterations=3,
    poly_n=5,
    poly_sigma=1.2,
    flags=0
)

# flow.shape = (H, W, 2) → (u, v) for each pixel
```

---

### 6. VINS-Fusion에서의 활용

#### feature_tracker 분석

```cpp
// feature_tracker/src/feature_tracker.cpp

void FeatureTracker::trackImage(...)
{
    // 1. FAST로 특징점 검출
    cv::goodFeaturesToTrack(...)
    
    // 2. LK로 추적
    cv::calcOpticalFlowPyrLK(
        prev_img, cur_img, 
        prev_pts, cur_pts,
        status, err,
        cv::Size(21, 21),  // 윈도우
        3                   // 피라미드 레벨
    );
    
    // 3. outlier 제거 (Fundamental Matrix)
    cv::findFundamentalMat(...)
    
    // 4. 새 특징점 추가 (개수 유지)
    if (pts.size() < MAX_CNT)
        cv::goodFeaturesToTrack(...)
}
```

#### VINS config 파라미터

```yaml
# config/euroc/euroc_stereo_imu_config.yaml

max_cnt: 150          # 최대 특징점 수
min_dist: 30          # 특징점 간 최소 거리
F_threshold: 1.0      # Fundamental matrix 임계값
```

---

## 💻 실습 파일

| 파일 | 내용 | 난이도 |
|------|------|--------|
| `optical_flow_basics.py` | LK 구현, 시각화 | ⭐⭐⭐ |
| `optical_flow_quiz.py` | 파라미터 분석, Dense Flow | ⭐⭐⭐ |

---

## 📊 핵심 정리

### 광류 방법 비교

| 방법 | 특징 | 장단점 |
|------|------|--------|
| Lucas-Kanade | 지역 일정, Sparse | 빠름, 코너에서 정확 |
| Horn-Schunck | 전역 부드러움, Dense | 느림, 경계 모호 |
| Farneback | 다항식 확장, Dense | 중간 속도 |
| DeepFlow/FlowNet | 딥러닝 | 정확, 느림 |

### LK 파라미터 가이드

| 파라미터 | 높이면 | 낮추면 |
|---------|--------|--------|
| winSize | 더 넓은 영역, 안정적 | 더 정밀, 작은 물체 |
| maxLevel | 큰 움직임 가능 | 빠름, 작은 움직임만 |
| maxCorners | 더 많은 추적 | 빠름 |

---

## ✅ 학습 완료 체크리스트

### 기초 이해 (필수)
- [ ] 밝기 항상성 가정 설명 가능
- [ ] 조리개 문제와 해결 방법 이해
- [ ] Lucas-Kanade 원리 설명 가능

### 실용 활용 (권장)
- [ ] cv2.calcOpticalFlowPyrLK() 사용 가능
- [ ] 추적 실패 감지 (status, error)
- [ ] 파라미터 튜닝 효과 이해

### 심화 (선택)
- [ ] Structure Tensor와 Harris의 관계 이해
- [ ] Dense Flow 사용 가능
- [ ] VINS feature_tracker 코드 분석

---

## 🔗 다음 단계

### Phase 2 완료! 🎉

다음은 **Phase 3: Visual Odometry & Bundle Adjustment**:
- VO 파이프라인 이해
- 번들 조정 (Bundle Adjustment) 개념
- Ceres Solver 맛보기

---

## 📚 참고 자료

- Computer Vision: Algorithms and Applications (Szeliski) - Chapter 8
- OpenCV Optical Flow Tutorial
- VINS-Fusion feature_tracker 코드

---

## ❓ FAQ

**Q1: 추적 실패는 왜 발생하나요?**
A: 폐색, 빠른 움직임, 조명 변화, 플랫 영역.

**Q2: winSize 선택 기준?**
A: 작으면 정밀하나 민감, 크면 안정적이나 뭉뚱그려짐. 보통 15~31.

**Q3: 매칭 대신 추적의 장점?**
A: 디스크립터 계산 불필요 → 더 빠름. 연속 프레임에 적합.

**Q4: 광류로 포즈 추정 가능?**
A: 직접은 불가. 추적된 점들로 E 분해 또는 PnP 필요.

---

**🎯 Week 8 핵심 메시지:**

> 광류 = 픽셀 단위 움직임 추정
> 
> **Lucas-Kanade**로 실시간 특징점 추적!
> VINS-Fusion의 feature_tracker가 바로 이것!

---

# 🎉 Phase 2 완료!

8주간의 컴퓨터 비전 기초를 완료했습니다:

| 주차 | 주제 | 핵심 |
|------|------|------|
| 1 | 핀홀 카메라 | 3D→2D 투영 |
| 2 | 왜곡/캘리브레이션 | K, distortion |
| 3 | 특징점 검출 | Harris, FAST, ORB |
| 4 | 특징점 매칭 | Ratio Test, RANSAC |
| 5 | 에피폴라 기하학 | E, F Matrix |
| 6 | 포즈 추정 | E → R, t |
| 7 | 삼각측량/PnP | 2D↔3D |
| 8 | 광류 | Lucas-Kanade |

**다음: Phase 3 - VO & BA! 🚀**
