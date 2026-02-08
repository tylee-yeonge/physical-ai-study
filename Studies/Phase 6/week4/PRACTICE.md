# Week 4 실습: Monocular 3D Detection 핵심 구성요소 구현

> **목표**: SMOKE/FCOS3D의 핵심 구성요소를 직접 구현하여 모델 원리를 체득
> **언어**: Python (NumPy, PyTorch)
> **예상 시간**: 5시간

---

## 실습 개요

이번 실습에서는 Monocular 3D Detection 모델의 핵심 구성요소를 단계별로 구현합니다. 전체 모델을 학습하지는 않지만, 각 구성요소의 원리를 코드로 이해합니다.

---

## 환경 설정

```bash
pip install numpy matplotlib torch torchvision
```

---

## Step 1: Gaussian Heatmap 생성 (SMOKE 스타일)

```python
import numpy as np
import matplotlib.pyplot as plt
import torch
import torch.nn as nn


def generate_gaussian_heatmap(heatmap, center, radius):
    """
    Gaussian heatmap 생성 (CenterNet/SMOKE 스타일)

    Parameters:
        heatmap: (H, W) ndarray - 기존 heatmap (여러 객체 누적)
        center: (cx, cy) - 객체 중심 좌표
        radius: int - Gaussian 반경
    """
    x, y = int(center[0]), int(center[1])
    h, w = heatmap.shape

    # Gaussian 범위
    left = min(x, radius)
    right = min(w - x, radius + 1)
    top = min(y, radius)
    bottom = min(h - y, radius + 1)

    # 2D Gaussian
    sigma = radius / 3.0  # 3-sigma rule
    y_range = np.arange(-top, bottom)
    x_range = np.arange(-left, right)
    yy, xx = np.meshgrid(y_range, x_range, indexing='ij')

    gaussian = np.exp(-(xx**2 + yy**2) / (2 * sigma**2))

    # 기존 값과 max 취하기 (여러 객체가 겹칠 때)
    masked_heatmap = heatmap[y - top:y + bottom, x - left:x + right]
    masked_gaussian = gaussian[:bottom + top, :right + left]

    if masked_heatmap.shape == masked_gaussian.shape:
        np.maximum(masked_heatmap, masked_gaussian, out=masked_heatmap)

    return heatmap


def visualize_heatmap():
    """여러 객체의 Gaussian heatmap을 생성하고 시각화"""
    H, W = 96, 312  # Feature map 크기 (원본의 1/4)

    # 각 클래스별 heatmap
    heatmap_car = np.zeros((H, W))
    heatmap_ped = np.zeros((H, W))

    # 가상 객체 중심 (feature map 좌표)
    cars = [(150, 45, 15), (100, 50, 8), (200, 48, 6)]  # (cx, cy, radius)
    peds = [(250, 40, 5), (80, 35, 4)]

    for cx, cy, r in cars:
        generate_gaussian_heatmap(heatmap_car, (cx, cy), r)
    for cx, cy, r in peds:
        generate_gaussian_heatmap(heatmap_ped, (cx, cy), r)

    # 시각화
    fig, axes = plt.subplots(1, 3, figsize=(18, 4))

    axes[0].imshow(heatmap_car, cmap='hot', aspect='auto')
    axes[0].set_title('Car Heatmap')
    for cx, cy, _ in cars:
        axes[0].plot(cx, cy, 'g+', markersize=10, markeredgewidth=2)

    axes[1].imshow(heatmap_ped, cmap='hot', aspect='auto')
    axes[1].set_title('Pedestrian Heatmap')
    for cx, cy, _ in peds:
        axes[1].plot(cx, cy, 'g+', markersize=10, markeredgewidth=2)

    # 합성
    combined = np.stack([heatmap_car, heatmap_ped, np.zeros_like(heatmap_car)], axis=2)
    axes[2].imshow(combined, aspect='auto')
    axes[2].set_title('Combined (R=Car, G=Ped)')

    plt.suptitle('SMOKE 스타일 Gaussian Heatmap', fontsize=14)
    plt.tight_layout()
    plt.savefig('heatmap_generation.png', dpi=150)
    plt.show()
    print("Heatmap 생성 및 시각화 완료!")


visualize_heatmap()
```

---

## Step 2: sin/cos Rotation 인코딩/디코딩

```python
def encode_rotation(theta):
    """회전각을 sin/cos로 인코딩"""
    return np.sin(theta), np.cos(theta)


def decode_rotation(sin_val, cos_val):
    """sin/cos에서 회전각 복원"""
    return np.arctan2(sin_val, cos_val)


def rotation_encoding_demo():
    """sin/cos 인코딩의 장점을 시각적으로 보여줍니다"""

    # 다양한 각도 테스트
    thetas = np.linspace(-np.pi, np.pi, 100)

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))

    # 1. 원래 각도
    axes[0, 0].plot(thetas, thetas)
    axes[0, 0].set_title('원래 각도 (theta)')
    axes[0, 0].set_xlabel('GT theta')
    axes[0, 0].set_ylabel('theta 값')
    axes[0, 0].grid(True)

    # 2. sin/cos 인코딩
    sin_vals = np.sin(thetas)
    cos_vals = np.cos(thetas)
    axes[0, 1].plot(thetas, sin_vals, label='sin(theta)', color='blue')
    axes[0, 1].plot(thetas, cos_vals, label='cos(theta)', color='red')
    axes[0, 1].set_title('sin/cos 인코딩')
    axes[0, 1].set_xlabel('theta')
    axes[0, 1].legend()
    axes[0, 1].grid(True)

    # 3. 직접 회귀 시 문제점
    # theta=pi 근처에서 -pi와 pi의 불연속
    axes[1, 0].set_title('직접 회귀 문제: 불연속점')
    gt = np.pi - 0.01  # pi에 가까운 각도
    preds = np.linspace(-np.pi, np.pi, 100)
    l1_direct = np.abs(preds - gt)
    axes[1, 0].plot(preds, l1_direct, label='L1 Loss (직접)')
    axes[1, 0].axvline(x=-np.pi + 0.01, color='r', linestyle='--', label='실제로 가까운 예측')
    axes[1, 0].set_xlabel('예측 theta')
    axes[1, 0].set_ylabel('L1 Loss')
    axes[1, 0].legend()
    axes[1, 0].grid(True)

    # 4. sin/cos 회귀의 해결
    gt_sin, gt_cos = np.sin(gt), np.cos(gt)
    l1_sincos = np.sqrt((np.sin(preds) - gt_sin)**2 + (np.cos(preds) - gt_cos)**2)
    axes[1, 1].plot(preds, l1_sincos, label='L2 Loss (sin/cos)', color='green')
    axes[1, 1].set_title('sin/cos 인코딩: 연속적 손실')
    axes[1, 1].set_xlabel('예측 theta')
    axes[1, 1].set_ylabel('sin/cos L2 Loss')
    axes[1, 1].legend()
    axes[1, 1].grid(True)

    plt.suptitle('Rotation 인코딩 비교: 직접 회귀 vs sin/cos', fontsize=14)
    plt.tight_layout()
    plt.savefig('rotation_encoding.png', dpi=150)
    plt.show()
    print("Rotation 인코딩 비교 시각화 완료!")

    # 인코딩/디코딩 테스트
    print("\n인코딩/디코딩 테스트:")
    test_thetas = [0, np.pi/4, np.pi/2, np.pi, -np.pi/2, -np.pi + 0.01]
    for theta in test_thetas:
        s, c = encode_rotation(theta)
        recovered = decode_rotation(s, c)
        print(f"  theta={theta:>7.4f} -> sin={s:>7.4f}, cos={c:>7.4f} -> recovered={recovered:>7.4f}")


rotation_encoding_demo()
```

---

## Step 3: Depth 예측 실험

```python
def depth_estimation_comparison():
    """
    다양한 depth 예측 방법을 비교합니다.
    """
    # GT depth 범위 (KITTI 전형적)
    z_gt = np.linspace(1, 80, 100)

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))

    # 방법 1: Direct regression
    z_max = 80.0
    output_direct = z_gt / z_max  # sigmoid 출력 (0~1)
    z_pred_direct = output_direct * z_max

    axes[0, 0].plot(z_gt, z_pred_direct, 'b-', label='예측')
    axes[0, 0].plot(z_gt, z_gt, 'r--', label='GT')
    axes[0, 0].set_title('방법 1: Direct Regression\nz = sigmoid(out) * z_max')
    axes[0, 0].set_xlabel('GT Depth [m]')
    axes[0, 0].set_ylabel('Predicted Depth [m]')
    axes[0, 0].legend()
    axes[0, 0].grid(True)

    # 방법 2: Log-space regression
    output_log = np.log(z_gt)
    z_pred_log = np.exp(output_log)

    axes[0, 1].plot(z_gt, output_log, 'b-', label='네트워크 출력 (log(z))')
    axes[0, 1].set_title('방법 2: Log-space\noutput = log(z), z = exp(output)')
    axes[0, 1].set_xlabel('GT Depth [m]')
    axes[0, 1].set_ylabel('Log(depth)')
    axes[0, 1].legend()
    axes[0, 1].grid(True)

    # 방법 3: 같은 절대 오차에 대한 상대적 영향
    abs_error = 2.0  # 2m 오차
    relative_error = abs_error / z_gt * 100

    axes[1, 0].plot(z_gt, relative_error, 'r-')
    axes[1, 0].set_title(f'절대 오차 {abs_error}m의 상대적 영향')
    axes[1, 0].set_xlabel('GT Depth [m]')
    axes[1, 0].set_ylabel('상대 오차 [%]')
    axes[1, 0].grid(True)
    axes[1, 0].axhline(y=10, color='gray', linestyle='--', alpha=0.5, label='10%')
    axes[1, 0].legend()

    # 방법 4: 기하학적 방법
    fx = 720.0
    H_real = 1.5  # 차 높이 (m)
    h_pixels = fx * H_real / z_gt  # 이미지에서의 높이 (pixels)

    axes[1, 1].plot(z_gt, h_pixels, 'g-')
    axes[1, 1].set_title('기하학적 방법: h_pixel = fx * H_real / z')
    axes[1, 1].set_xlabel('GT Depth [m]')
    axes[1, 1].set_ylabel('이미지 높이 [pixels]')
    axes[1, 1].grid(True)
    axes[1, 1].axhline(y=25, color='gray', linestyle='--', alpha=0.5, label='25px (Moderate 기준)')
    axes[1, 1].legend()

    plt.suptitle('Depth 추정 방법 비교', fontsize=14)
    plt.tight_layout()
    plt.savefig('depth_comparison.png', dpi=150)
    plt.show()
    print("Depth 추정 방법 비교 완료!")

    # Log-space의 장점 계산
    print("\nLog-space의 장점:")
    print("  z=5m에서 2m 오차:")
    print(f"    L1(direct): |5 - 3| = 2.0")
    print(f"    L1(log):    |log(5) - log(3)| = |{np.log(5):.3f} - {np.log(3):.3f}| = {abs(np.log(5) - np.log(3)):.3f}")
    print("  z=50m에서 2m 오차:")
    print(f"    L1(direct): |50 - 48| = 2.0")
    print(f"    L1(log):    |log(50) - log(48)| = |{np.log(50):.3f} - {np.log(48):.3f}| = {abs(np.log(50) - np.log(48)):.3f}")
    print()
    print("  → Direct: 가까이든 멀든 같은 loss (2.0)")
    print("  → Log: 가까울 때 loss가 크고, 멀 때 작음")
    print("  → Log가 가까운 객체의 정확도를 더 중시 (자율주행에 유리)")


depth_estimation_comparison()
```

---

## Step 4: 간단한 3D Detection Head (PyTorch)

```python
class Simple3DHead(nn.Module):
    """
    SMOKE/FCOS3D 스타일의 간단한 3D Detection Head

    입력: backbone feature map (B, C, H, W)
    출력: heatmap, offset, depth, size, rotation
    """
    def __init__(self, in_channels=64, num_classes=3):
        super().__init__()

        # 공유 convolution
        self.shared_conv = nn.Sequential(
            nn.Conv2d(in_channels, 64, 3, padding=1),
            nn.BatchNorm2d(64),
            nn.ReLU(inplace=True),
        )

        # 분류 분기: Heatmap (각 클래스별 중심점 확률)
        self.cls_head = nn.Sequential(
            nn.Conv2d(64, 64, 3, padding=1),
            nn.ReLU(inplace=True),
            nn.Conv2d(64, num_classes, 1),  # (B, C, H, W) C=클래스 수
        )

        # 회귀 분기
        self.offset_head = nn.Conv2d(64, 2, 1)     # 2D offset (dx, dy)
        self.depth_head = nn.Conv2d(64, 1, 1)       # Depth (z)
        self.size_head = nn.Conv2d(64, 3, 1)         # Size (h, w, l)
        self.rot_head = nn.Conv2d(64, 2, 1)          # Rotation (sin, cos)

    def forward(self, x):
        """
        Parameters:
            x: (B, C, H, W) backbone feature

        Returns:
            dict of predictions
        """
        feat = self.shared_conv(x)

        heatmap = torch.sigmoid(self.cls_head(feat))   # (B, num_cls, H, W)
        offset = self.offset_head(feat)                  # (B, 2, H, W)
        depth = torch.relu(self.depth_head(feat)) + 1.0  # (B, 1, H, W), 최소 1m
        size = self.size_head(feat)                       # (B, 3, H, W)
        rotation = self.rot_head(feat)                    # (B, 2, H, W)

        return {
            'heatmap': heatmap,
            'offset': offset,
            'depth': depth,
            'size': size,
            'rotation': rotation,
        }


# 테스트
head = Simple3DHead(in_channels=64, num_classes=3)
dummy_input = torch.randn(2, 64, 96, 312)  # (B, C, H, W)

outputs = head(dummy_input)

print("3D Detection Head 출력:")
for key, val in outputs.items():
    print(f"  {key:12s}: shape={list(val.shape)}")

# 파라미터 수
total_params = sum(p.numel() for p in head.parameters())
print(f"\n총 파라미터 수: {total_params:,}")
```

---

## Step 5: Focal Loss 구현

```python
def focal_loss(pred, target, alpha=2.0, beta=4.0):
    """
    CenterNet/SMOKE 스타일 Focal Loss

    Parameters:
        pred: (B, C, H, W) - sigmoid 후 예측값
        target: (B, C, H, W) - Gaussian heatmap GT

    Positive: target == 1 인 위치
    Negative: target < 1 인 위치 (가중치 감소)
    """
    pos_mask = target.eq(1).float()
    neg_mask = target.lt(1).float()

    # Positive loss
    pos_loss = -torch.log(pred + 1e-8) * torch.pow(1 - pred, alpha) * pos_mask

    # Negative loss (배경 위치에서의 손실 감소)
    neg_loss = (-torch.log(1 - pred + 1e-8) *
                torch.pow(pred, alpha) *
                torch.pow(1 - target, beta) * neg_mask)

    num_pos = pos_mask.sum()
    pos_loss = pos_loss.sum()
    neg_loss = neg_loss.sum()

    if num_pos == 0:
        return neg_loss
    return (pos_loss + neg_loss) / num_pos


# 테스트
pred = torch.rand(1, 3, 10, 10)
target = torch.zeros(1, 3, 10, 10)
target[0, 0, 5, 5] = 1.0  # 하나의 positive

loss = focal_loss(pred, target)
print(f"Focal Loss: {loss.item():.4f}")
```

---

## Step 6: 전체 파이프라인 시뮬레이션

```python
def full_pipeline_demo():
    """
    3D Detection 전체 파이프라인을 시뮬레이션합니다.
    (실제 이미지 없이 가상 데이터로)
    """
    print("=" * 50)
    print("Monocular 3D Detection 파이프라인 시뮬레이션")
    print("=" * 50)

    # 1. 가상 GT
    gt_objects = [
        {'class': 'Car', 'x': 2.0, 'y': 1.65, 'z': 15.0,
         'h': 1.5, 'w': 1.8, 'l': 4.5, 'ry': 0.1},
        {'class': 'Car', 'x': -3.0, 'y': 1.65, 'z': 25.0,
         'h': 1.5, 'w': 1.7, 'l': 4.2, 'ry': -0.05},
    ]

    print("\n1. Ground Truth:")
    for obj in gt_objects:
        print(f"   {obj['class']}: pos=({obj['x']}, {obj['y']}, {obj['z']}) "
              f"size=({obj['l']}, {obj['w']}, {obj['h']}) ry={obj['ry']}")

    # 2. 가상 모델 출력 (약간의 오차 포함)
    print("\n2. 모델 예측 (가상):")
    for obj in gt_objects:
        # 오차 추가
        pred_z = obj['z'] + np.random.normal(0, 1.5)  # depth 오차 ~1.5m
        pred_ry_sin = np.sin(obj['ry']) + np.random.normal(0, 0.05)
        pred_ry_cos = np.cos(obj['ry']) + np.random.normal(0, 0.05)
        pred_ry = np.arctan2(pred_ry_sin, pred_ry_cos)

        print(f"   {obj['class']}:")
        print(f"     GT depth: {obj['z']:.1f}m, Pred depth: {pred_z:.1f}m, "
              f"오차: {abs(pred_z - obj['z']):.1f}m")
        print(f"     GT ry: {obj['ry']:.3f}, Pred ry: {pred_ry:.3f}, "
              f"오차: {abs(pred_ry - obj['ry']):.3f} rad "
              f"({np.degrees(abs(pred_ry - obj['ry'])):.1f}도)")

    # 3. Depth 오차의 영향
    print("\n3. Depth 오차가 3D IoU에 미치는 영향:")
    for dz in [0.5, 1.0, 2.0, 5.0]:
        # 간단한 IoU 근사 (축 정렬, 같은 크기)
        l, w, h = 4.5, 1.8, 1.5
        vol = l * w * h
        overlap_z = max(0, l - dz)
        intersection = w * h * overlap_z
        iou = intersection / (2 * vol - intersection) if (2 * vol - intersection) > 0 else 0
        status = "TP" if iou >= 0.7 else "FP"
        print(f"   dz={dz:.1f}m: IoU~{iou:.3f} ({status}) "
              f"{'<- KITTI Car 통과!' if iou >= 0.7 else ''}")


full_pipeline_demo()
```

---

## 체크리스트

- [ ] Gaussian Heatmap 생성 이해 및 시각화
- [ ] sin/cos rotation 인코딩/디코딩 구현
- [ ] Depth 추정 방법 비교 (Direct, Log-space, 기하학적)
- [ ] Simple 3D Detection Head (PyTorch) 구조 이해
- [ ] Focal Loss 원리 이해
- [ ] 전체 파이프라인 시뮬레이션 실행

---

## 추가 실험 아이디어

1. **Multi-bin rotation**: 2-bin 방법 구현하여 sin/cos와 비교
2. **Size residual**: 카테고리 평균 + 잔차 방식 구현
3. **NMS (Non-Maximum Suppression)**: 3D NMS 구현
4. **Depth uncertainty**: 깊이 예측의 불확실성을 함께 출력하도록 head 수정

---

이전: [Week 3 실습](../week3/PRACTICE.md)

**다음**: Week 5에서 MMDetection3D를 사용하여 실제 모델을 학습합니다!
