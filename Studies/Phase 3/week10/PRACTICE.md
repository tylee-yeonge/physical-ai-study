# Week 10 실습: Depth 정확도 검증 (Python)

> 🎯 **목표**: Depth Anything 모델의 출력을 GT와 비교하여 정량/정성 평가 수행
> 💻 **언어**: Python (PyTorch, OpenCV, NumPy)
> ⏰ **예상 시간**: 12시간

---

## 📋 실습 개요

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | 스테레오 매칭으로 GT Depth 생성 | 필수 | 2시간 |
| 2 | Depth 모델 추론 + Scale 정렬 | 필수 | 2시간 |
| 3 | 정량 평가 (AbsRel, RMSE, delta) | 필수 | 3시간 |
| 4 | 정성 평가 (취약점 분석) | 필수 | 3시간 |
| 5 | FP32 vs FP16 비교 분석 | 심화 | 2시간 |

---

## 🔧 환경 설정

```bash
# 가상환경 활성화
conda activate phase5

# 패키지 설치
pip install -r requirements.txt
```

---

## Step 1: 스테레오 매칭으로 GT Depth 생성

### 1.1 목표

KITTI 또는 자체 스테레오 이미지 쌍에서 시차(Disparity) 맵을 계산하고 깊이(Depth) 맵으로 변환합니다.

### 1.2 KITTI 데이터 활용

```python
# download_kitti_sample.py
"""
KITTI 스테레오 데이터 준비
- 공식 사이트: https://www.cvlibs.net/datasets/kitti/eval_depth.php
- 또는 아래 코드로 샘플 시뮬레이션
"""
import numpy as np
import cv2
import os

# KITTI 카메라 파라미터 (예시)
KITTI_PARAMS = {
    'focal_length': 721.5377,   # 픽셀 단위
    'baseline': 0.5372,         # 미터 단위
    'cx': 609.5593,
    'cy': 172.854,
    'width': 1242,
    'height': 375,
}

def create_synthetic_stereo_pair():
    """테스트용 합성 스테레오 쌍 생성"""
    height, width = 375, 1242

    # 간단한 합성 장면 (실제로는 KITTI 데이터 사용)
    img_left = np.random.randint(50, 200, (height, width, 3), dtype=np.uint8)
    img_right = img_left.copy()

    # 인공적인 시차 추가 (실제로는 스테레오 카메라 촬영)
    shift = 20  # 20 픽셀 시차
    img_right[:, shift:] = img_left[:, :-shift]

    return img_left, img_right

print("KITTI 카메라 파라미터:")
for key, val in KITTI_PARAMS.items():
    print(f"  {key}: {val}")
```

### 1.3 스테레오 매칭 구현

```python
# stereo_depth_gt.py
"""스테레오 매칭으로 Ground Truth Depth 맵 생성"""
import cv2
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def compute_stereo_depth(img_left, img_right, focal_length, baseline):
    """SGBM 스테레오 매칭으로 깊이 맵 계산"""

    # 그레이스케일 변환
    gray_l = cv2.cvtColor(img_left, cv2.COLOR_BGR2GRAY)
    gray_r = cv2.cvtColor(img_right, cv2.COLOR_BGR2GRAY)

    # SGBM 파라미터 설정
    min_disp = 0
    num_disp = 128   # 시차 범위 (16의 배수)
    block_size = 11

    stereo = cv2.StereoSGBM_create(
        minDisparity=min_disp,
        numDisparities=num_disp,
        blockSize=block_size,
        P1=8 * 3 * block_size ** 2,
        P2=32 * 3 * block_size ** 2,
        disp12MaxDiff=1,
        uniquenessRatio=10,
        speckleWindowSize=100,
        speckleRange=32,
        mode=cv2.STEREO_SGBM_MODE_SGBM_3WAY
    )

    # 시차 맵 계산 (16배 스케일로 반환됨)
    disparity = stereo.compute(gray_l, gray_r).astype(np.float32) / 16.0

    # 유효 시차 마스크
    valid_mask = disparity > 0

    # 깊이 계산: Z = f * b / d
    depth = np.zeros_like(disparity)
    depth[valid_mask] = (focal_length * baseline) / disparity[valid_mask]

    # 깊이 범위 제한 (0.5m ~ 80m)
    range_mask = (depth > 0.5) & (depth < 80.0)
    valid_mask = valid_mask & range_mask

    return depth, disparity, valid_mask

def visualize_stereo_results(img_left, disparity, depth, valid_mask):
    """스테레오 매칭 결과 시각화"""
    fig, axes = plt.subplots(2, 2, figsize=(16, 10))

    axes[0, 0].imshow(cv2.cvtColor(img_left, cv2.COLOR_BGR2RGB))
    axes[0, 0].set_title('왼쪽 이미지')
    axes[0, 0].axis('off')

    im1 = axes[0, 1].imshow(disparity, cmap='magma')
    axes[0, 1].set_title('시차 맵 (Disparity)')
    axes[0, 1].axis('off')
    plt.colorbar(im1, ax=axes[0, 1])

    depth_vis = depth.copy()
    depth_vis[~valid_mask] = 0
    im2 = axes[1, 0].imshow(depth_vis, cmap='turbo', vmin=0, vmax=50)
    axes[1, 0].set_title('깊이 맵 (GT, 미터)')
    axes[1, 0].axis('off')
    plt.colorbar(im2, ax=axes[1, 0])

    axes[1, 1].imshow(valid_mask.astype(np.uint8) * 255, cmap='gray')
    axes[1, 1].set_title('유효 마스크')
    axes[1, 1].axis('off')

    plt.tight_layout()
    plt.savefig('stereo_depth_results.png', dpi=150)
    print("결과 저장: stereo_depth_results.png")

# 실행
if __name__ == "__main__":
    # 실제로는 KITTI 스테레오 이미지 쌍을 로드
    # img_left = cv2.imread('kitti/left/000000.png')
    # img_right = cv2.imread('kitti/right/000000.png')

    # 테스트용 합성 이미지
    img_left = np.random.randint(50, 200, (375, 1242, 3), dtype=np.uint8)
    img_right = img_left.copy()

    focal_length = 721.5377
    baseline = 0.5372

    depth, disparity, valid_mask = compute_stereo_depth(
        img_left, img_right, focal_length, baseline
    )

    print(f"깊이 통계:")
    print(f"  유효 픽셀: {valid_mask.sum()} / {valid_mask.size}")
    if valid_mask.any():
        print(f"  깊이 범위: {depth[valid_mask].min():.2f} ~ {depth[valid_mask].max():.2f} m")

    visualize_stereo_results(img_left, disparity, depth, valid_mask)
```

### 1.4 체크포인트

```
 스테레오 매칭으로 시차 맵 생성
 시차 → 깊이 변환 (Z = f*b/d)
 유효 마스크 생성
 결과 시각화 확인
```

---

## Step 2: Depth 모델 추론 + Scale 정렬

### 2.1 Depth Anything 추론

```python
# depth_inference.py
"""Depth Anything 모델로 추론 및 GT 대비 Scale 정렬"""
import torch
import numpy as np
from transformers import AutoModelForDepthEstimation, AutoImageProcessor
from PIL import Image

def run_depth_inference(image_path, model_name="LiheYoung/depth-anything-small-hf"):
    """Depth Anything 모델로 깊이 추정"""

    # 모델 로드
    processor = AutoImageProcessor.from_pretrained(model_name)
    model = AutoModelForDepthEstimation.from_pretrained(model_name)
    model.eval()

    # 이미지 로드 및 전처리
    image = Image.open(image_path).convert('RGB')
    inputs = processor(images=image, return_tensors="pt")

    # 추론
    with torch.no_grad():
        outputs = model(**inputs)
        predicted_depth = outputs.predicted_depth

    # 후처리: 원본 크기로 리사이즈
    prediction = torch.nn.functional.interpolate(
        predicted_depth.unsqueeze(1),
        size=image.size[::-1],  # (H, W)
        mode="bicubic",
        align_corners=False,
    ).squeeze().numpy()

    return prediction

def align_depth_scale(pred_depth, gt_depth, valid_mask):
    """최소자승법으로 Scale + Shift 정렬"""

    pred_valid = pred_depth[valid_mask].flatten()
    gt_valid = gt_depth[valid_mask].flatten()

    if len(pred_valid) < 10:
        print("  경고: 유효 픽셀이 너무 적습니다.")
        return pred_depth, 1.0, 0.0

    # gt = scale * pred + shift
    A = np.vstack([pred_valid, np.ones_like(pred_valid)]).T
    result = np.linalg.lstsq(A, gt_valid, rcond=None)
    scale, shift = result[0]

    aligned = scale * pred_depth + shift

    print(f"  Scale 정렬 결과:")
    print(f"    scale = {scale:.4f}")
    print(f"    shift = {shift:.4f}")

    return aligned, scale, shift
```

### 2.2 체크포인트

```
 Depth Anything 추론 성공
 출력 shape 확인 (H, W)
 Scale/Shift 정렬 완료
 정렬 전후 시각화 비교
```

---

## Step 3: 정량 평가

### 3.1 평가 지표 계산

```python
# depth_metrics.py
"""Depth 평가 지표 계산 모듈"""
import numpy as np

def compute_depth_metrics(pred, gt, valid_mask, max_depth=80.0):
    """모든 Depth 평가 지표를 계산하여 딕셔너리로 반환"""

    # 유효 범위
    mask = valid_mask & (gt > 0.1) & (gt < max_depth) & (pred > 0.1)
    pred_v = pred[mask]
    gt_v = gt[mask]

    n = len(pred_v)
    if n == 0:
        return {'error': '유효 픽셀 없음'}

    # 1. AbsRel
    abs_rel = np.mean(np.abs(pred_v - gt_v) / gt_v)

    # 2. SqRel
    sq_rel = np.mean(((pred_v - gt_v) ** 2) / gt_v)

    # 3. RMSE
    rmse = np.sqrt(np.mean((pred_v - gt_v) ** 2))

    # 4. RMSE_log
    rmse_log = np.sqrt(np.mean((np.log(pred_v) - np.log(gt_v)) ** 2))

    # 5. Threshold accuracy (delta)
    ratio = np.maximum(pred_v / gt_v, gt_v / pred_v)
    d1 = np.mean(ratio < 1.25)
    d2 = np.mean(ratio < 1.25 ** 2)
    d3 = np.mean(ratio < 1.25 ** 3)

    metrics = {
        'AbsRel': abs_rel,
        'SqRel': sq_rel,
        'RMSE': rmse,
        'RMSE_log': rmse_log,
        'delta_1': d1,
        'delta_2': d2,
        'delta_3': d3,
        'num_valid': n,
    }

    return metrics

def print_metrics(metrics):
    """평가 결과를 표 형태로 출력"""

    print("\n" + "=" * 50)
    print("  Depth 평가 결과")
    print("=" * 50)

    if 'error' in metrics:
        print(f"  오류: {metrics['error']}")
        return

    print(f"  유효 픽셀 수:  {metrics['num_valid']:,}")
    print(f"  ─────────────────────────────")
    print(f"  AbsRel:        {metrics['AbsRel']:.4f}  {'(양호)' if metrics['AbsRel'] < 0.1 else '(개선필요)'}")
    print(f"  SqRel:         {metrics['SqRel']:.4f}")
    print(f"  RMSE:          {metrics['RMSE']:.4f} m")
    print(f"  RMSE_log:      {metrics['RMSE_log']:.4f}")
    print(f"  ─────────────────────────────")
    print(f"  delta < 1.25:  {metrics['delta_1']:.4f}  {'(양호)' if metrics['delta_1'] > 0.95 else '(개선필요)'}")
    print(f"  delta < 1.25^2:{metrics['delta_2']:.4f}")
    print(f"  delta < 1.25^3:{metrics['delta_3']:.4f}")
    print("=" * 50)

# 실행 예제
if __name__ == "__main__":
    # 시뮬레이션 데이터로 테스트
    np.random.seed(42)
    gt = np.random.uniform(1, 50, (375, 1242))
    pred = gt + np.random.normal(0, 2, gt.shape)  # GT에 노이즈 추가
    pred = np.clip(pred, 0.1, 100)
    valid = np.ones_like(gt, dtype=bool)

    metrics = compute_depth_metrics(pred, gt, valid)
    print_metrics(metrics)
```

### 3.2 체크포인트

```
 AbsRel, RMSE, delta 지표 계산 성공
 평가 결과 표 출력 확인
 지표 값의 의미 이해
```

---

## Step 4: 정성 평가

### 4.1 오차 시각화 및 취약점 분석

```python
# qualitative_eval.py
"""정성 평가: 경계선, 원거리, 반사 표면 취약점 분석"""
import numpy as np
import cv2
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def visualize_error_map(pred, gt, valid_mask, rgb_image, save_path='error_analysis.png'):
    """오차 맵 시각화"""

    error = np.abs(pred - gt)
    error[~valid_mask] = 0

    fig, axes = plt.subplots(2, 3, figsize=(18, 10))

    # 원본 이미지
    axes[0, 0].imshow(cv2.cvtColor(rgb_image, cv2.COLOR_BGR2RGB))
    axes[0, 0].set_title('입력 이미지')
    axes[0, 0].axis('off')

    # 예측 깊이
    im1 = axes[0, 1].imshow(pred, cmap='turbo', vmin=0, vmax=50)
    axes[0, 1].set_title('예측 깊이')
    axes[0, 1].axis('off')
    plt.colorbar(im1, ax=axes[0, 1])

    # GT 깊이
    gt_vis = gt.copy()
    gt_vis[~valid_mask] = 0
    im2 = axes[0, 2].imshow(gt_vis, cmap='turbo', vmin=0, vmax=50)
    axes[0, 2].set_title('GT 깊이')
    axes[0, 2].axis('off')
    plt.colorbar(im2, ax=axes[0, 2])

    # 절대 오차 맵
    im3 = axes[1, 0].imshow(error, cmap='hot', vmin=0, vmax=10)
    axes[1, 0].set_title('절대 오차 (m)')
    axes[1, 0].axis('off')
    plt.colorbar(im3, ax=axes[1, 0])

    # 상대 오차 맵
    rel_error = np.zeros_like(error)
    rel_error[valid_mask] = error[valid_mask] / gt[valid_mask]
    im4 = axes[1, 1].imshow(rel_error, cmap='hot', vmin=0, vmax=0.5)
    axes[1, 1].set_title('상대 오차 (AbsRel)')
    axes[1, 1].axis('off')
    plt.colorbar(im4, ax=axes[1, 1])

    # 거리별 오차 히스토그램
    distance_bins = [(0, 10), (10, 20), (20, 40), (40, 80)]
    bin_errors = []
    bin_labels = []
    for d_min, d_max in distance_bins:
        mask = valid_mask & (gt >= d_min) & (gt < d_max)
        if mask.any():
            bin_errors.append(np.mean(np.abs(pred[mask] - gt[mask]) / gt[mask]))
        else:
            bin_errors.append(0)
        bin_labels.append(f'{d_min}-{d_max}m')

    axes[1, 2].bar(bin_labels, bin_errors, color=['green', 'yellow', 'orange', 'red'])
    axes[1, 2].set_title('거리별 AbsRel')
    axes[1, 2].set_ylabel('AbsRel')

    plt.tight_layout()
    plt.savefig(save_path, dpi=150)
    print(f"오차 분석 저장: {save_path}")

def analyze_edge_error(pred, gt, valid_mask, rgb_image):
    """경계선 주변 오차 분석"""

    gray = cv2.cvtColor(rgb_image, cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(gray, 50, 150)
    edge_mask = edges > 0

    # 경계 영역 확장 (5픽셀)
    kernel = np.ones((5, 5), np.uint8)
    edge_region = cv2.dilate(edges, kernel, iterations=1) > 0

    combined_mask = valid_mask & (gt > 0.1)

    # 경계 vs 비경계 오차
    edge_combined = combined_mask & edge_region
    non_edge_combined = combined_mask & ~edge_region

    if edge_combined.any() and non_edge_combined.any():
        edge_error = np.mean(np.abs(pred[edge_combined] - gt[edge_combined]) / gt[edge_combined])
        non_edge_error = np.mean(np.abs(pred[non_edge_combined] - gt[non_edge_combined]) / gt[non_edge_combined])

        print(f"\n경계선 오차 분석:")
        print(f"  경계 영역 AbsRel:    {edge_error:.4f}")
        print(f"  비경계 영역 AbsRel:  {non_edge_error:.4f}")
        print(f"  비율:                {edge_error / non_edge_error:.2f}x")
    else:
        print("  유효 경계 데이터 부족")

# 실행
if __name__ == "__main__":
    # 시뮬레이션 데이터
    np.random.seed(42)
    H, W = 375, 1242
    rgb = np.random.randint(50, 200, (H, W, 3), dtype=np.uint8)
    gt = np.random.uniform(1, 50, (H, W))
    pred = gt + np.random.normal(0, 2, gt.shape)
    pred = np.clip(pred, 0.1, 100)
    valid = np.ones((H, W), dtype=bool)

    visualize_error_map(pred, gt, valid, rgb)
    analyze_edge_error(pred, gt, valid, rgb)
```

### 4.2 체크포인트

```
 오차 맵 시각화 생성
 경계선 오차 분석 완료
 거리별 오차 분포 확인
 취약점 3가지 (경계, 원거리, 반사) 이해
```

---

## Step 5: FP32 vs FP16 비교 (심화)

### 5.1 정밀도 비교 분석

```python
# fp_comparison.py
"""FP32 vs FP16 정밀도 비교"""
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def compare_fp32_fp16(depth_fp32, depth_fp16):
    """FP32와 FP16 추론 결과 비교"""

    diff = np.abs(depth_fp32 - depth_fp16)
    rel_diff = diff / (np.abs(depth_fp32) + 1e-6)

    print("\nFP32 vs FP16 비교:")
    print(f"  평균 절대 차이: {np.mean(diff):.6f}")
    print(f"  최대 절대 차이: {np.max(diff):.6f}")
    print(f"  평균 상대 차이: {np.mean(rel_diff) * 100:.4f}%")
    print(f"  최대 상대 차이: {np.max(rel_diff) * 100:.4f}%")

    # 시각화
    fig, axes = plt.subplots(1, 3, figsize=(15, 4))

    im1 = axes[0].imshow(depth_fp32, cmap='turbo')
    axes[0].set_title('FP32 Depth')
    plt.colorbar(im1, ax=axes[0])

    im2 = axes[1].imshow(depth_fp16, cmap='turbo')
    axes[1].set_title('FP16 Depth')
    plt.colorbar(im2, ax=axes[1])

    im3 = axes[2].imshow(rel_diff * 100, cmap='hot', vmin=0, vmax=5)
    axes[2].set_title('상대 차이 (%)')
    plt.colorbar(im3, ax=axes[2])

    plt.tight_layout()
    plt.savefig('fp_comparison.png', dpi=150)
    print("비교 결과 저장: fp_comparison.png")

# 시뮬레이션 실행
if __name__ == "__main__":
    np.random.seed(42)
    # 시뮬레이션: FP32 결과에 미세한 차이 추가
    depth_fp32 = np.random.uniform(0.5, 50, (384, 512))
    depth_fp16 = depth_fp32 + np.random.normal(0, 0.01, depth_fp32.shape)

    compare_fp32_fp16(depth_fp32, depth_fp16)
```

---

## ✅ 실습 체크리스트

### Step 1: GT 생성
- [ ] 스테레오 매칭 (SGBM) 구현
- [ ] 시차 → 깊이 변환
- [ ] 유효 마스크 생성
- [ ] 결과 시각화

### Step 2: Scale 정렬
- [ ] Depth Anything 추론 성공
- [ ] 최소자승법으로 scale/shift 계산
- [ ] 정렬 전후 비교

### Step 3: 정량 평가
- [ ] AbsRel 계산 및 의미 이해
- [ ] RMSE 계산
- [ ] delta < 1.25 계산
- [ ] 결과 표 출력

### Step 4: 정성 평가
- [ ] 오차 맵 시각화
- [ ] 경계선 오차 분석
- [ ] 거리별 오차 분포
- [ ] 반사 표면 관찰

### Step 5: FP 비교 (심화)
- [ ] FP32 vs FP16 차이 분석
- [ ] 상대 오차 < 1% 확인

---

## 💡 트러블슈팅

### 스테레오 매칭이 잘 안 될 때

```
문제: 시차 맵이 노이즈가 많음
해결: blockSize를 키우거나 (11 → 15), speckleWindowSize 조정

문제: 유효 픽셀이 너무 적음
해결: numDisparities를 늘리거나 (128 → 256), uniquenessRatio 줄이기
```

### Scale 정렬이 이상할 때

```
문제: scale이 음수 또는 너무 큰 값
해결: 유효 픽셀이 너무 적거나, pred/gt의 범위가 다름
     → 유효 마스크 조건 완화 또는 median 기반 정렬 시도
```

---

## 🔗 참고 자료

- [Depth Anything 논문](https://arxiv.org/abs/2401.10891)
- [KITTI Depth Benchmark](https://www.cvlibs.net/datasets/kitti/eval_depth.php)
- [Eigen split 평가 프로토콜](https://arxiv.org/abs/1406.2283)

---

**다음**: Quiz로 개념 점검!
