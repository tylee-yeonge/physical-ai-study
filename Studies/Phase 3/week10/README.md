# Week 10: Depth 정확도 검증 (Section 5.3)

> 🎯 **이번 주 목표**: Depth 모델의 출력을 정량적/정성적으로 검증하여 실제 SLAM 적용 가능성을 판단하기
> ⏰ **예상 시간**: 12시간
> 💡 **핵심 질문**: "Depth Anything 모델의 추정 깊이가 실제와 얼마나 차이나는가? 어떤 상황에서 실패하는가?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | Depth 평가 지표, Ground Truth 생성 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | AbsRel 계산, FP16 정밀도 비교 심화 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | Depth 정확도 검증 (Python) |

---

## 🌟 시작하기 전에

### Week 9에서 배운 것

**TensorRT 변환 파이프라인:**
```
PyTorch → ONNX → TensorRT FP16
- Depth Anything Small: Jetson에서 15-20 FPS
- FP16으로 속도 2배, 메모리 40% 절약
```

**하지만 한 가지 의문:**
```
❓ TensorRT로 빠르게 추론은 되는데... 결과가 정확한가?
❓ FP16 변환 후 정밀도가 얼마나 떨어졌나?
❓ 어떤 장면에서 Depth 추정이 실패하나?
❓ SLAM에서 쓸 만한 수준인가?
```

**이번 주에 답합니다!**

---

## 📚 핵심 개념 자세히 알아보기

### 1. Ground Truth 생성: 스테레오 카메라 → Depth

#### 1.1 왜 Ground Truth가 필요한가?

```
Depth Anything 출력 = 상대적 깊이 (Relative Depth)
  → 가까운 것은 크고, 먼 것은 작다
  → 하지만 "실제 몇 미터인지"는 모른다!

Ground Truth = 절대 깊이 (Metric Depth)
  → 실제 거리 정보 (미터 단위)
  → 비교 기준이 되어야 정량 평가 가능
```

#### 1.2 스테레오 매칭으로 Depth 생성

```
왼쪽 카메라        오른쪽 카메라
    ┌───┐              ┌───┐
    │ L │◄─── b(기선) ──►│ R │
    └───┘              └───┘
       \              /
        \   물체 P  /
         \   ●   /
          \   /
           \/

깊이 Z = (f × b) / d
  f: 초점 거리 (focal length, 픽셀)
  b: 기선 거리 (baseline, 미터)
  d: 시차 (disparity, 픽셀) = x_L - x_R
```

#### 1.3 OpenCV 스테레오 매칭 코드

```python
import cv2
import numpy as np

def generate_stereo_depth(img_left, img_right, focal_length, baseline):
    """스테레오 이미지 쌍으로부터 깊이 맵 생성"""

    # 그레이스케일 변환
    gray_l = cv2.cvtColor(img_left, cv2.COLOR_BGR2GRAY)
    gray_r = cv2.cvtColor(img_right, cv2.COLOR_BGR2GRAY)

    # Semi-Global Block Matching (SGBM)
    stereo = cv2.StereoSGBM_create(
        minDisparity=0,
        numDisparities=128,      # 시차 범위 (16의 배수)
        blockSize=11,            # 매칭 블록 크기
        P1=8 * 3 * 11**2,       # 스무딩 파라미터
        P2=32 * 3 * 11**2,
        disp12MaxDiff=1,
        uniquenessRatio=10,
        speckleWindowSize=100,
        speckleRange=32,
        mode=cv2.STEREO_SGBM_MODE_SGBM_3WAY
    )

    # 시차 맵 계산
    disparity = stereo.compute(gray_l, gray_r).astype(np.float32) / 16.0

    # 유효하지 않은 시차 마스킹
    valid_mask = disparity > 0

    # 깊이 계산: Z = f * b / d
    depth = np.zeros_like(disparity)
    depth[valid_mask] = (focal_length * baseline) / disparity[valid_mask]

    return depth, valid_mask
```

---

### 2. Depth 모델 출력 스케일 정렬

#### 2.1 상대 깊이 → 절대 깊이 변환

```
Depth Anything 출력: d_pred (상대적, 0~1 범위)
Ground Truth: d_gt (절대적, 미터 단위)

변환: d_aligned = scale * d_pred + shift

scale과 shift를 구하는 방법:
  → 최소자승법 (Least Squares)
  → d_gt ≈ scale * d_pred + shift
  → 유효한 픽셀에서만 계산
```

#### 2.2 Scale 정렬 코드

```python
def align_depth_scale(pred_depth, gt_depth, valid_mask):
    """상대적 깊이를 절대적 깊이에 맞게 스케일 정렬"""

    pred_valid = pred_depth[valid_mask].flatten()
    gt_valid = gt_depth[valid_mask].flatten()

    # 최소자승법으로 scale, shift 계산
    # gt = scale * pred + shift
    A = np.vstack([pred_valid, np.ones_like(pred_valid)]).T
    result = np.linalg.lstsq(A, gt_valid, rcond=None)
    scale, shift = result[0]

    # 정렬된 깊이
    aligned_depth = scale * pred_depth + shift

    return aligned_depth, scale, shift
```

---

### 3. 정량 평가 지표 (Metrics)

#### 3.1 주요 평가 지표

```
1. AbsRel (Absolute Relative Error)
   AbsRel = (1/N) * Σ |d_pred - d_gt| / d_gt
   → 상대적 오차. 0에 가까울수록 좋음
   → 좋은 모델: AbsRel < 0.1

2. RMSE (Root Mean Squared Error)
   RMSE = sqrt((1/N) * Σ (d_pred - d_gt)^2)
   → 절대적 오차 (미터). 0에 가까울수록 좋음
   → 큰 오차에 더 민감 (제곱 효과)

3. δ < 1.25 (Threshold Accuracy)
   δ_t = (max(d_pred/d_gt, d_gt/d_pred) < t인 비율)
   → t = 1.25, 1.25^2, 1.25^3
   → 1에 가까울수록 좋음
   → δ < 1.25 > 0.95이면 좋은 모델
```

#### 3.2 KITTI 데이터셋 기준값 (참고)

```
┌────────────────────────────────────────────────────┐
│            Depth Estimation 벤치마크                 │
├─────────────────┬──────────┬──────────┬────────────┤
│ 지표            │ 좋은 수준 │ 보통     │ 나쁜 수준   │
├─────────────────┼──────────┼──────────┼────────────┤
│ AbsRel          │ < 0.06   │ 0.06~0.12│ > 0.12     │
│ RMSE (m)        │ < 3.0    │ 3.0~5.0  │ > 5.0      │
│ δ < 1.25        │ > 0.97   │ 0.90~0.97│ < 0.90     │
│ δ < 1.25^2      │ > 0.99   │ 0.97~0.99│ < 0.97     │
└─────────────────┴──────────┴──────────┴────────────┘
```

#### 3.3 평가 코드

```python
def compute_depth_metrics(pred, gt, valid_mask, max_depth=80.0):
    """Depth 추정 평가 지표 계산"""

    # 유효 범위 마스크
    mask = valid_mask & (gt > 0.1) & (gt < max_depth)
    pred_valid = pred[mask]
    gt_valid = gt[mask]

    # AbsRel
    abs_rel = np.mean(np.abs(pred_valid - gt_valid) / gt_valid)

    # RMSE
    rmse = np.sqrt(np.mean((pred_valid - gt_valid) ** 2))

    # RMSE_log
    rmse_log = np.sqrt(np.mean((np.log(pred_valid) - np.log(gt_valid)) ** 2))

    # Threshold accuracy
    ratio = np.maximum(pred_valid / gt_valid, gt_valid / pred_valid)
    delta1 = np.mean(ratio < 1.25)
    delta2 = np.mean(ratio < 1.25 ** 2)
    delta3 = np.mean(ratio < 1.25 ** 3)

    metrics = {
        'AbsRel': abs_rel,
        'RMSE': rmse,
        'RMSE_log': rmse_log,
        'delta_1.25': delta1,
        'delta_1.25^2': delta2,
        'delta_1.25^3': delta3,
    }

    return metrics
```

---

### 4. 정성 평가: Depth 모델의 취약점

#### 4.1 경계선 (Edges) 문제

```
실제 장면:                Depth 출력:
┌──────────────┐         ┌──────────────┐
│   물체 A     │         │   물체 A     │
│         ┌───┐│         │       ~~┌───┐│
│         │ B ││  →      │       ~~│ B ││
│         └───┘│         │       ~~└───┘│
│              │         │              │
└──────────────┘         └──────────────┘

~~ = 경계가 번져 있음 (Depth bleeding)

원인:
- 모델이 경계 주변에서 불확실한 깊이를 출력
- 인코더의 receptive field가 경계를 넘어감
- ViT의 패치 경계와 물체 경계가 일치하지 않음
```

#### 4.2 먼 물체 (Far Objects) 문제

```
깊이별 오차 분포:

거리(m)  |  AbsRel
─────────┼─────────
0~10     |  0.03   (매우 정확)
10~20    |  0.05   (양호)
20~40    |  0.10   (보통)
40~80    |  0.20   (부정확)
80+      |  0.40+  (매우 부정확)

원인:
- 원거리 물체는 이미지에서 작은 영역 차지
- 텍스처 정보 부족
- 단안 카메라의 본질적 한계 (스케일 모호성)
```

#### 4.3 반사 표면 (Reflective Surfaces) 문제

```
반사 표면 유형:
- 거울: 반사된 물체의 깊이를 추정
- 유리: 투과 + 반사가 혼합
- 물웅덩이: 하늘/건물 반사
- 금속 표면: 주변 환경 반사

결과: 실제 표면 깊이 대신 반사된 물체의 깊이를 출력
→ SLAM에서 잘못된 3D 포인트 생성!
```

#### 4.4 정성 평가 시각화 방법

```python
def qualitative_evaluation(pred_depth, gt_depth, rgb_image):
    """정성 평가: 오차 맵 시각화"""

    # 1. 오차 맵
    error_map = np.abs(pred_depth - gt_depth)

    # 2. 엣지 검출 (경계선 평가)
    edges = cv2.Canny(rgb_image, 50, 150)
    edge_mask = edges > 0

    # 3. 경계 주변 오차 vs 비경계 오차
    edge_error = np.mean(error_map[edge_mask])
    non_edge_error = np.mean(error_map[~edge_mask])

    # 4. 거리별 오차
    distance_bins = [(0, 10), (10, 20), (20, 40), (40, 80)]
    for d_min, d_max in distance_bins:
        mask = (gt_depth >= d_min) & (gt_depth < d_max)
        if mask.any():
            bin_error = np.mean(error_map[mask])
            print(f"  거리 {d_min}~{d_max}m: AbsError = {bin_error:.3f}")
```

---

### 5. FP32 vs FP16 정밀도 비교

#### 5.1 비교 방법

```
같은 입력 이미지에 대해:
  1. FP32 TRT 엔진으로 추론 → depth_fp32
  2. FP16 TRT 엔진으로 추론 → depth_fp16
  3. 차이 분석: |depth_fp32 - depth_fp16|

기대 결과:
  - 평균 차이 < 1% (대부분의 경우)
  - 최대 차이 < 5% (극단적인 경우)
  - 깊이 순서(ordering)는 거의 동일
```

---

## 💡 꼭 이해해야 할 핵심 개념

### 개념 1: 상대 깊이 vs 절대 깊이

```
Monocular Depth Estimation의 본질적 한계:
  → 단안 카메라로는 절대 스케일을 알 수 없음
  → "Scale Ambiguity" (스케일 모호성)

해결 방법:
  1. 스테레오 카메라로 GT 생성 후 스케일 정렬
  2. LiDAR sparse point로 스케일 보정
  3. 알려진 물체 크기 (차량 높이 등) 참조
  4. IMU + VO로 스케일 추정
```

### 개념 2: 평가 지표 선택 기준

```
AbsRel: 전체적인 상대 정확도 → 가장 대표적인 지표
RMSE: 큰 오차에 민감 → 안전이 중요한 경우
δ < 1.25: 올바른 비율 → 전반적 신뢰도
RMSE_log: 로그 스케일 오차 → 근거리/원거리 공평한 평가
```

### 개념 3: Depth 실패 모드의 SLAM 영향

```
경계 오차 → 포인트 클라우드에서 "비행 포인트" 발생
원거리 오차 → 맵 스케일 왜곡
반사 표면 → 잘못된 3D 좌표 → 루프 클로저 실패

대처:
  → 신뢰도(confidence)가 낮은 영역 필터링
  → 깊이 범위 제한 (예: 0.5m ~ 50m만 사용)
  → 엣지 주변 깊이값 후처리
```

---

## 🔍 자체 점검 - 이해했는지 확인!

**Q1: AbsRel 지표의 의미와 계산 공식은?**

<details>
<summary>정답 보기</summary>

- AbsRel = (1/N) * sum(|pred - gt| / gt)
- "예측값과 실제값의 차이를 실제값으로 나눈 것의 평균"
- 상대적 오차이므로 거리에 관계없이 공정한 평가 가능
- 좋은 모델: AbsRel < 0.1 (10% 이하 오차)

</details>

**Q2: 스테레오 카메라로 깊이를 구하는 공식은?**

<details>
<summary>정답 보기</summary>

- Z = (f * b) / d
- f: 초점 거리(focal length, 픽셀 단위)
- b: 기선 거리(baseline, 미터 단위)
- d: 시차(disparity, 픽셀 단위) = 왼쪽 x 좌표 - 오른쪽 x 좌표
- 시차가 클수록 가까운 물체, 작을수록 먼 물체

</details>

**Q3: Depth 모델이 경계선에서 오차가 큰 이유는?**

<details>
<summary>정답 보기</summary>

- ViT의 패치 경계와 물체 경계가 일치하지 않음
- 인코더의 receptive field가 서로 다른 깊이의 영역을 동시에 포함
- "Depth bleeding" 현상: 깊이 값이 경계를 넘어 번짐
- 디코더 업샘플링 과정에서 경계 정보 손실

</details>

**Q4: 상대적 깊이를 절대적 깊이로 변환할 때 Scale과 Shift가 필요한 이유는?**

<details>
<summary>정답 보기</summary>

- Monocular Depth는 본질적으로 스케일 모호성(Scale Ambiguity)을 가짐
- 모델이 "A가 B보다 가깝다"는 알지만 "A는 3m, B는 5m"는 모름
- Scale: 상대적 깊이 범위를 절대적 깊이 범위로 매핑
- Shift: 깊이 오프셋 보정 (모델의 systematic bias)
- 최소자승법으로 GT에 맞는 scale, shift를 찾아 적용

</details>

---

## 📝 이번 주 실습 & 다음 주 준비

### 실습 구성

| Step | 내용 | 예상 시간 |
|------|------|----------|
| 1 | 스테레오 → GT Depth 생성 | 2시간 |
| 2 | Depth 모델 추론 + Scale 정렬 | 2시간 |
| 3 | 정량 평가: AbsRel, RMSE, delta | 3시간 |
| 4 | 정성 평가: 경계선, 원거리, 반사 | 3시간 |
| 5 | FP32 vs FP16 비교 | 2시간 |

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

### 다음 주 준비

- Week 11에서는 Detection + Depth 융합으로 **3D 위치 추정**을 다룹니다
- YOLO의 2D BBox + Depth 맵 → 3D 좌표 변환
- ROS2 integration 시작

---

## 🎯 이번 주 핵심 요약

### 1. Ground Truth 생성

```
스테레오 카메라 → SGBM → 시차 맵 → Z = f*b/d
절대 깊이 기준으로 모델 출력 검증
```

### 2. 정량 평가 3대 지표

```
AbsRel: 상대적 오차 (< 0.1이면 양호)
RMSE: 절대 오차, 큰 오차에 민감
delta < 1.25: 올바른 비율 (> 0.95이면 양호)
```

### 3. Depth 모델의 취약점

```
경계선: Depth bleeding (번짐)
원거리: 스케일 모호성 심화
반사 표면: 잘못된 깊이 추정
```

### 4. Scale 정렬

```
d_aligned = scale * d_pred + shift
최소자승법으로 GT에 맞춰 스케일 보정
```

---

이전: [Week 9 - ONNX & TensorRT 변환](../week9/README.md)

다음: [Week 11 - Detection + Depth 융합](../week11/README.md)
