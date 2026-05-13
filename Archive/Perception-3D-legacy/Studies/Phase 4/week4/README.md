# Week 4: Monocular 3D Detection 모델 - SMOKE, FCOS3D, Depth 추정

> **이번 주 목표**: 대표적인 Monocular 3D Detection 모델(SMOKE, FCOS3D)의 원리를 이해하고, Depth 추정 및 3D Box 인코딩 방법을 학습
> **예상 시간**: 12-15시간
> **핵심 질문**: "단안 카메라 이미지 한 장에서 어떻게 3D bounding box를 예측할 수 있는가?"

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | SMOKE 핵심 아이디어, FCOS3D 구조, Depth 추정 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | sin/cos 인코딩/디코딩, Multi-task Loss 코드 실습 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | Monocular 3D Detection 모델 분석 및 구현 실습 |

---

## 시작하기 전에

### Week 1-3 복습

```
Week 1: 3D Detection이 필요한 이유, 7개 파라미터 [x,y,z,l,w,h,theta]
Week 2: 좌표계 변환 (Camera/LiDAR/BEV), KITTI 규약
Week 3: KITTI 데이터셋 구조, 레이블 파싱, 시각화

이번 주는 이 모든 지식을 '모델'로 연결합니다!
```

### 이번 주의 핵심 질문

```
Monocular 3D Detection의 핵심 도전:

  카메라 이미지 1장 → [x, y, z, l, w, h, theta] 예측

  특히 z (깊이)를 어떻게 추정하는가?
  → 이것이 Monocular 3D Detection의 최대 난관입니다!

  각 모델이 이 문제를 어떻게 풀었는지 비교합니다:
    SMOKE:  Keypoint 기반 (CenterNet 확장)
    FCOS3D: Anchor-free Multi-task (FCOS 확장)
```

---

## 핵심 개념 자세히 알아보기

### 1. Monocular 3D Detection 개요

#### 1.1 문제 정의

```
입력: RGB 이미지 1장 (H x W x 3)
출력: 각 객체의 3D bounding box
     [x, y, z, l, w, h, theta, class, confidence]

핵심 과제:
  1. 어디에 물체가 있는가? (2D 위치)
  2. 물체의 깊이(z)는 얼마인가? (가장 어려움!)
  3. 물체의 3D 크기(l, w, h)는? (카테고리 평균 활용)
  4. 물체의 방향(theta)은? (sin/cos 인코딩)
```

#### 1.2 주요 접근 방식

```
Monocular 3D Detection 방법론:

1. Direct Regression (직접 회귀)
   - 이미지 feature에서 직접 [x,y,z,l,w,h,theta] 회귀
   - 예: FCOS3D, PGD
   - 장점: 단순한 파이프라인
   - 단점: 깊이 추정 정확도 한계

2. Keypoint 기반
   - 객체 중심/꼭짓점 등 keypoint를 먼저 검출
   - 2D-3D 기하학적 관계로 3D 복원
   - 예: SMOKE, MonoGRNet
   - 장점: 기하학적 제약 활용
   - 단점: keypoint 검출 정확도에 의존

3. Depth Map 기반
   - 먼저 pixel-wise depth map 추정
   - depth + 2D bbox로 3D 복원
   - 예: Pseudo-LiDAR, D4LCN
   - 장점: depth 정보 명시적 사용
   - 단점: depth 추정 오차 전파

4. Transformer 기반
   - Attention으로 3D 정보 추출
   - 예: MonoDETR, DETR3D
   - 장점: 글로벌 컨텍스트 활용
   - 단점: 연산량 큼
```

### 2. SMOKE: Single-Stage Monocular 3D Object Detection via Keypoint Estimation

#### 2.1 SMOKE 개요

```
SMOKE (CVPRW 2020):
  - CenterNet의 3D 확장
  - Anchor-free, Single-stage
  - 객체 중심점(keypoint)을 heatmap으로 검출
  - 중심점에서 3D 속성을 직접 회귀

핵심 아이디어:
  "2D 이미지에서 객체의 중심점을 찾고,
   그 중심점에서 3D 정보(depth, size, rotation)를 예측한다"
```

#### 2.2 SMOKE 구조

```
입력 이미지
    ↓
Backbone (DLA-34)
    ↓
Feature Map (H/4 x W/4)
    ↓
+----------------------------------+
|          SMOKE Head              |
+-------------+--------------------+
|  Heatmap    |   Regression       |
|  (cls별     |   - 3D offset      |
|   중심점)   |   - depth (z)      |
|             |   - size (h,w,l)   |
|             |   - rotation       |
|             |     (sin, cos)     |
+-------------+--------------------+

Heatmap:
  - 크기: (C x H/4 x W/4)  (C = 클래스 수)
  - 각 픽셀의 값 = 해당 위치에 객체 중심이 있을 확률
  - Gaussian 분포로 GT 생성

Regression:
  - 각 중심점에서 3D 속성을 예측
  - offset: 다운샘플링에 의한 정수화 오차 보정
  - depth: log(depth)로 예측 (스케일 문제 완화)
  - size: 카테고리 평균으로부터의 잔차(residual) 예측
  - rotation: sin(theta), cos(theta)로 인코딩
```

#### 2.3 SMOKE의 Depth 추정

```
SMOKE에서의 깊이 추정 방법:

1. 직접 회귀 + 기하학적 제약

   중심점의 이미지 좌표 (u, v)와 3D 좌표 (X, Y, Z)의 관계:
     u = fx * X/Z + cx
     v = fy * Y/Z + cy

   네트워크가 Z를 예측하면:
     X = (u - cx) * Z / fx
     Y = (v - cy) * Z / fy

   → 깊이 Z 하나만 정확히 추정하면 X, Y는 자동 결정!

2. Log-space 예측:
   z_pred = sigmoid(output) * z_max
   또는 z_pred = exp(output) (log-space)

   이유: 깊이의 범위가 크고 (1m ~ 80m),
         가까운 것과 먼 것의 오차 비율을 균일하게 하기 위해
```

### 3. FCOS3D: Fully Convolutional One-Stage Monocular 3D Object Detection

#### 3.1 FCOS3D 개요

```
FCOS3D (ICCVW 2021):
  - FCOS (2D Anchor-free detector)의 3D 확장
  - Feature Pyramid Network (FPN) 사용
  - Multi-scale 검출
  - Multi-task Head: 분류 + 여러 3D 속성 동시 예측

핵심 아이디어:
  "FPN의 각 위치에서 분류, 2D offset, depth, size, rotation을
   동시에 예측하는 Multi-task Learning"
```

#### 3.2 FCOS3D 구조

```
입력 이미지
    ↓
Backbone (ResNet-101)
    ↓
FPN (Feature Pyramid Network)
    ↓ Multi-scale features (P3, P4, P5, P6, P7)
    ↓
+--------------------------------------+
|           FCOS3D Head                |
+--------------------------------------+
| 각 FPN 레벨에서:                      |
|                                      |
| [분류 분기]        [회귀 분기]         |
|  - cls score       - 2D center offset|
|  - centerness      - depth (z)       |
|                    - size (l, w, h)  |
|                    - rotation        |
|                      (sin θ, cos θ)  |
|                    - velocity (opt)  |
|                    - attribute (opt) |
+--------------------------------------+

FCOS vs FCOS3D 차이:
+------------+--------------+------------------+
|            | FCOS (2D)    | FCOS3D (3D)      |
+------------+--------------+------------------+
| 회귀 대상  | l,t,r,b      | offset, z, size, |
|            | (2D bbox)    | rotation         |
| Center     | centerness   | 3D centerness    |
| 좌표계     | 이미지       | Camera 3D        |
| FPN 할당   | 크기 기반    | 깊이 기반        |
+------------+--------------+------------------+
```

#### 3.3 FCOS3D의 FPN 레벨 할당

```
FCOS (2D): 객체 크기로 FPN 레벨 할당
  작은 객체 → P3 (고해상도)
  큰 객체 → P7 (저해상도)

FCOS3D: 깊이(depth)로 FPN 레벨 할당
  가까운 객체 → P3 (이미지에서 큼)
  먼 객체 → P7 (이미지에서 작음)

  이유:
    멀리 있는 객체는 이미지에서 작게 보이므로
    저해상도 feature에서 검출하는 것이 적합
```

### 4. Depth 추정 방법 비교

```
방법 1: Direct Regression
  z = sigmoid(output) * z_max
  - 단순하지만 정확도 낮음
  - FCOS3D가 사용

방법 2: Log-space Regression
  z = exp(output)  또는  z = 1 / sigmoid(output)
  - 깊이의 분포가 long-tail이므로 log-space가 유리
  - SMOKE 등이 사용

방법 3: 기하학적 제약 활용
  z = f * H_real / h_pixel
  (f: 초점 거리, H_real: 실제 높이, h_pixel: 이미지에서의 높이)
  - 물체 크기 사전 지식 활용
  - MonoGRNet 등이 보조적으로 사용

방법 4: Depth Map 활용
  - 별도 네트워크로 pixel-wise depth 추정
  - 추정된 depth를 3D 검출에 활용
  - Pseudo-LiDAR 계열

비교:
+--------------+----------+----------+----------+
| 방법          | 정확도   | 복잡도   | 대표 모델 |
+--------------+----------+----------+----------+
| Direct       | 낮음     | 낮음     | FCOS3D   |
| Log-space    | 중간     | 낮음     | SMOKE    |
| 기하학적 제약 | 중간     | 낮음     | MonoGRNet|
| Depth Map    | 높음     | 높음     | PseudoLi |
+--------------+----------+----------+----------+
```

### 5. 3D Box 인코딩: sin/cos Rotation

#### 5.1 왜 sin/cos 인코딩을 사용하는가?

```
문제: yaw 각도(theta)의 회전 특성

  theta = 0도와 theta = 360도는 같은 방향인데,
  직접 회귀하면 0과 360(또는 0과 2*pi)을 다르게 취급!

  예: GT theta = 0.01 라디안
      Pred theta = 6.27 라디안 (2*pi - 0.01)
      → L1 loss = 6.26 (매우 큼!)
      → 실제로는 거의 같은 방향인데 손실이 큼

해결: sin/cos 인코딩

  theta 대신 [sin(theta), cos(theta)]를 예측
  → 각도의 연속성 문제 해결

  예: theta = 0.01
      sin(0.01) = 0.01, cos(0.01) = 1.0

      theta = 6.27 (= 2*pi - 0.01)
      sin(6.27) = -0.01, cos(6.27) = 1.0

      → sin 값만 약간 다르고, cos 값은 거의 같음
      → 손실이 적절하게 작음!
```

#### 5.2 sin/cos에서 theta 복원

```python
import numpy as np

def encode_rotation(theta):
    """회전각을 sin/cos로 인코딩"""
    return np.sin(theta), np.cos(theta)

def decode_rotation(sin_val, cos_val):
    """sin/cos에서 회전각 복원"""
    theta = np.arctan2(sin_val, cos_val)
    return theta

# 테스트
theta_original = 1.5  # 라디안
sin_val, cos_val = encode_rotation(theta_original)
theta_recovered = decode_rotation(sin_val, cos_val)

print(f"원래 theta: {theta_original:.4f}")
print(f"인코딩: sin={sin_val:.4f}, cos={cos_val:.4f}")
print(f"복원 theta: {theta_recovered:.4f}")
```

#### 5.3 Multi-bin 방법

```
일부 모델은 Multi-bin 방법을 사용합니다:

1. 회전 범위를 여러 bin으로 나눔 (예: 2개 bin)
   bin 0: [-pi, 0)
   bin 1: [0, pi)

2. 어떤 bin에 속하는지 분류 (Classification)
3. bin 내에서의 세부 각도 회귀 (Regression)

장점: 분류로 대략적 방향을 먼저 결정하여 정확도 향상
단점: 구현이 복잡
```

### 6. Loss Function

```
Monocular 3D Detection의 Multi-task Loss:

L_total = w_cls * L_cls + w_reg * L_reg

L_cls: 분류 손실
  - SMOKE: Focal Loss (heatmap)
  - FCOS3D: Focal Loss

L_reg: 회귀 손실 (여러 항의 합)
  L_reg = L_offset + L_depth + L_size + L_rotation

  - L_offset: 2D offset → L1 Loss
  - L_depth:  깊이      → L1 Loss (log-space)
  - L_size:   크기       → L1 Loss
  - L_rotation: 회전     → sin/cos에 대한 L1 Loss

각 항의 가중치 조절이 성능에 큰 영향:
  depth loss 가중치를 높이면 → 깊이 정확도 향상
  하지만 다른 task 성능이 저하될 수 있음
  → 적절한 밸런싱이 핵심!
```

---

## 꼭 이해해야 할 핵심 개념

### SMOKE vs FCOS3D 비교

```
+--------------+------------------+------------------+
|              | SMOKE            | FCOS3D           |
+--------------+------------------+------------------+
| 기반 모델    | CenterNet        | FCOS             |
| Backbone     | DLA-34           | ResNet-101       |
| Feature      | 단일 스케일      | FPN (다중 스케일)|
| 검출 방식    | Heatmap 중심점   | Dense prediction |
| FPN          | 없음             | 있음             |
| Depth 방법   | Log-space +      | Direct + sigmoid |
|              | 기하 제약        |                  |
| Rotation     | sin/cos          | sin/cos          |
| KITTI AP3D   | ~12%             | ~12%             |
| 복잡도       | 낮음             | 중간             |
| 학습 속도    | 빠름             | 보통             |
+--------------+------------------+------------------+
```

### Depth 추정이 왜 그렇게 어려운가

```
Monocular 3D Detection에서 depth가 어려운 이유:

1. 근본적 모호성 (Depth Ambiguity)
   - 하나의 이미지 → 무한한 3D 해석 가능
   - 작은 가까운 물체 vs 큰 먼 물체가 같아 보임

2. 깊이의 비선형성
   - 가까울 때: 1m 차이가 이미지에서 큰 변화
   - 멀 때: 1m 차이가 이미지에서 거의 무변화
   - u = fx * X/Z → Z에 대해 비선형

3. 학습 데이터 분포
   - KITTI: 대부분의 차가 10-50m 범위
   - 매우 가깝거나 먼 경우 데이터 부족
   - Long-tail 분포

4. 시각적 단서의 한계
   - 크기 단서: 같은 클래스도 크기 변동 있음
   - 원근 단서: 직선이 없는 경우 약함
   - 그림자/반사: 조명에 따라 불안정
```

### 포트폴리오에서의 가치

```
면접에서 자주 묻는 질문:

Q: "Monocular 3D Detection이 왜 어렵나요?"
A: "Depth Ambiguity가 핵심입니다. 단안 이미지에서
    깊이는 근본적으로 모호하며, 네트워크는 크기 단서,
    원근법, 카테고리별 크기 분포 등 간접적 단서로
    깊이를 '추정'해야 합니다."

Q: "SMOKE와 FCOS3D의 차이는?"
A: "SMOKE는 CenterNet 기반으로 heatmap 중심점을 먼저
    검출하고, FCOS3D는 FCOS 기반으로 모든 위치에서
    dense prediction합니다. SMOKE는 단순하고 빠르며,
    FCOS3D는 FPN으로 multi-scale 검출이 가능합니다."

Q: "sin/cos 인코딩을 왜 사용하나요?"
A: "각도의 주기성 문제를 해결하기 위해서입니다.
    0도와 360도는 같은 방향인데 직접 회귀하면
    큰 손실이 발생합니다. sin/cos 인코딩으로
    연속적인 표현이 가능합니다."
```

---

## 자체 점검 - 이해했는지 확인!

### Q1: SMOKE의 핵심 아이디어
**Q:** SMOKE에서 객체의 3D 위치 (X, Y, Z)를 어떻게 결정하는가?

**A:**
```
1. Heatmap에서 객체 중심의 이미지 좌표 (u, v) 검출
2. Regression head에서 깊이 Z를 예측
3. 카메라 모델로 X, Y 계산:
   X = (u - cx) * Z / fx
   Y = (v - cy) * Z / fy

즉, 핵심은 Z(깊이)를 정확히 예측하는 것이며,
X, Y는 이미지 좌표와 Z로부터 기하학적으로 결정됩니다.
```

### Q2: FPN의 역할
**Q:** FCOS3D가 FPN을 사용하는 이유와 이점은?

**A:**
```
FPN (Feature Pyramid Network)은 다중 스케일 feature를 제공합니다.

이점:
  - 가까운 (큰) 객체: 고해상도 feature (P3)에서 검출
  - 먼 (작은) 객체: 저해상도 feature (P7)에서 검출
  - 각 스케일에 적합한 feature 사용

FCOS3D 특유의 점:
  기존 FCOS는 '2D 크기'로 FPN 레벨을 할당하지만,
  FCOS3D는 '깊이(depth)'로 FPN 레벨을 할당합니다.
  (멀리 있는 객체 = 작게 보임 → 저해상도 feature)
```

### Q3: Size 예측 방법
**Q:** SMOKE에서 3D bbox의 크기(l, w, h)를 어떻게 예측하는가?

**A:**
```
카테고리 평균 크기 + 잔차(residual) 방식:

각 클래스의 평균 크기:
  Car:        l=3.88, w=1.63, h=1.53
  Pedestrian: l=0.88, w=0.64, h=1.73
  Cyclist:    l=1.78, w=0.60, h=1.73

예측:
  l_pred = l_mean + delta_l  (delta_l을 네트워크가 예측)
  w_pred = w_mean + delta_w
  h_pred = h_mean + delta_h

장점:
  - 대부분의 차는 비슷한 크기 → 잔차가 작음
  - 작은 값을 회귀하므로 학습 안정적
  - 비현실적인 크기 예측 방지 (평균 근처)
```

### Q4: Loss 밸런싱
**Q:** Multi-task Loss에서 depth loss 가중치를 크게 하면 어떻게 되는가?

**A:**
```
장점:
  - Depth 추정 정확도 향상
  - 3D IoU 개선 가능 (depth가 3D IoU에 가장 큰 영향)

단점:
  - 다른 task (분류, 크기, 회전) 성능 저하
  - 전체 밸런스가 깨짐

실제:
  - depth loss 가중치를 높이는 것이 보통 유리
  - 하지만 너무 높이면 분류 성능이 떨어져서 AP 감소
  - Task별 uncertainty로 자동 가중치 조절 방법도 있음
  - 하이퍼파라미터 탐색이 필요
```

---

## 이번 주 실습 & 다음 주 준비

### 이번 주 체크리스트

- [ ] SMOKE 모델 구조 이해 (Heatmap + Regression)
- [ ] FCOS3D 모델 구조 이해 (FPN + Multi-task Head)
- [ ] Depth 추정 방법론 4가지 비교
- [ ] sin/cos rotation 인코딩/디코딩 구현
- [ ] Multi-task Loss 구성 이해
- [ ] SMOKE vs FCOS3D 비교 정리
- [ ] `PRACTICE.md` 실습 완료
- [ ] `quiz_easy.py`, `quiz_medium.py` 풀기

### 다음 주 미리보기: MMDetection3D 실습

```
다음 주에는:
  - MMDetection3D 프레임워크 환경 세팅
  - Config 시스템 이해
  - FCOS3D 모델 학습 (KITTI)
  - Inference 및 3D bbox 시각화
  - AP3D 평가
  → Week 1-4의 이론을 실제 코드로 구현합니다!
```

---

## 이번 주 핵심 요약

1. **SMOKE**: CenterNet 기반 Keypoint 검출. Heatmap으로 중심점 찾고, 해당 위치에서 depth/size/rotation 회귀. 단순하고 빠름

2. **FCOS3D**: FCOS 기반 Anchor-free 검출. FPN으로 Multi-scale, Multi-task Head로 모든 3D 속성 동시 예측. 깊이 기반 FPN 할당

3. **Depth 추정**: Monocular 3D의 최대 난관. Direct/Log-space/기하학적/Depth Map 등 방법. Z를 알면 X, Y는 카메라 모델로 결정

4. **sin/cos 인코딩**: 각도의 주기성 문제 해결. theta 대신 [sin(theta), cos(theta)] 예측. arctan2로 복원

5. **Multi-task Loss**: L_cls + L_offset + L_depth + L_size + L_rotation. 각 가중치 밸런싱이 성능에 큰 영향. Depth 가중치를 높이는 것이 보통 유리

---

이전: [Week 3 - KITTI 데이터셋](../week3/README.md)

다음: [Week 5 - MMDetection3D 실습](../week5/README.md)
