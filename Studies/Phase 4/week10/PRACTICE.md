# Week 10 실습: BEVFormer 추론 및 BEV Feature 시각화

> [goal] **목표**: BEVFormer pretrained 모델로 추론하고, BEV Feature를 시각화하며, NDS/mAP 지표를 직접 계산해본다
> [code] **언어**: Python (PyTorch, NumPy, Matplotlib)
> [time] **예상 시간**: 8시간

---

## [list] 실습 개요

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | NDS 지표 계산 시뮬레이션 | 필수 | 2시간 |
| 2 | BEV Feature 시각화 실습 | 필수 | 2시간 |
| 3 | 3D Detection 결과 BEV 시각화 | 필수 | 2시간 |
| 4 | 거리별 성능 분석 | 필수 | 2시간 |

---

## [tool] 환경 설정

```bash
pip install -r requirements.txt
```

---

## Step 1: NDS 지표 계산 시뮬레이션

```python
"""
NDS (nuScenes Detection Score) 계산 시뮬레이션
NDS의 구성 요소를 이해하고 직접 계산한다.
"""
import numpy as np


def compute_nds(mAP, mATE, mASE, mAOE, mAVE, mAAE):
    """
    NDS(nuScenes Detection Score) 계산

    Parameters:
        mAP: mean Average Precision (0~1, 높을수록 좋음)
        mATE: mean Average Translation Error (m, 낮을수록 좋음)
        mASE: mean Average Scale Error (0~1, 낮을수록 좋음)
        mAOE: mean Average Orientation Error (rad, 낮을수록 좋음)
        mAVE: mean Average Velocity Error (m/s, 낮을수록 좋음)
        mAAE: mean Average Attribute Error (0~1, 낮을수록 좋음)

    Returns:
        nds: NDS 점수
        details: 세부 항목 딕셔너리
    """
    # TP 메트릭을 0~1로 정규화 (clamp)
    tp_metrics = {
        'mATE': max(1.0 - mATE, 0.0),
        'mASE': max(1.0 - mASE, 0.0),
        'mAOE': max(1.0 - mAOE, 0.0),
        'mAVE': max(1.0 - mAVE, 0.0),
        'mAAE': max(1.0 - mAAE, 0.0),
    }

    tp_sum = sum(tp_metrics.values())

    # NDS = 1/10 * (5 * mAP + sum(1 - mTP_i))
    nds = (5.0 * mAP + tp_sum) / 10.0

    details = {
        'mAP': mAP,
        'NDS': nds,
        **tp_metrics,
    }

    return nds, details


def compare_models():
    """다양한 모델의 NDS 비교"""
    print("=" * 60)
    print("NDS 계산 시뮬레이션: 모델 비교")
    print("=" * 60)

    models = {
        'BEVFormer-Base': {
            'mAP': 0.416, 'mATE': 0.673, 'mASE': 0.274,
            'mAOE': 0.372, 'mAVE': 0.394, 'mAAE': 0.198
        },
        'BEVFormer-Small': {
            'mAP': 0.370, 'mATE': 0.725, 'mASE': 0.279,
            'mAOE': 0.408, 'mAVE': 0.456, 'mAAE': 0.200
        },
        'DETR3D': {
            'mAP': 0.346, 'mATE': 0.773, 'mASE': 0.268,
            'mAOE': 0.383, 'mAVE': 0.842, 'mAAE': 0.216
        },
        'BEVDet': {
            'mAP': 0.312, 'mATE': 0.691, 'mASE': 0.271,
            'mAOE': 0.514, 'mAVE': 0.859, 'mAAE': 0.243
        },
    }

    results = {}
    for name, metrics in models.items():
        nds, details = compute_nds(**metrics)
        results[name] = details
        print(f"\n{name}:")
        print(f"  mAP:  {metrics['mAP']:.3f}")
        print(f"  mATE: {metrics['mATE']:.3f} → 1-mATE = {details['mATE']:.3f}")
        print(f"  mASE: {metrics['mASE']:.3f} → 1-mASE = {details['mASE']:.3f}")
        print(f"  mAOE: {metrics['mAOE']:.3f} → 1-mAOE = {details['mAOE']:.3f}")
        print(f"  mAVE: {metrics['mAVE']:.3f} → 1-mAVE = {details['mAVE']:.3f}")
        print(f"  mAAE: {metrics['mAAE']:.3f} → 1-mAAE = {details['mAAE']:.3f}")
        print(f"  NDS:  {nds:.3f}")

    # 비교 표
    print("\n" + "=" * 60)
    print(f"{'모델':<20} {'mAP':<8} {'NDS':<8}")
    print("-" * 36)
    for name, details in sorted(results.items(), key=lambda x: x[1]['NDS'], reverse=True):
        print(f"{name:<20} {details['mAP']:.3f}    {details['NDS']:.3f}")


compare_models()
```

---

## Step 2: BEV Feature 시각화 실습

```python
"""
BEV Feature Map 시각화 실습
다양한 시각화 방법으로 BEV Feature의 의미를 이해한다.
"""
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, FancyArrowPatch


def create_simulated_bev_feature(bev_h=200, bev_w=200, channels=256):
    """
    시뮬레이션 BEV Feature 생성
    실제 모델 출력을 모사하여 도로, 차량, 보행자 등의 패턴을 생성한다.
    """
    feature = np.random.randn(bev_h, bev_w, channels) * 0.05

    # 도로 영역 (중앙 세로 도로)
    road_x = slice(80, 120)
    feature[road_x, :, :] += np.random.randn(40, bev_w, channels) * 0.3

    # 도로 영역 (중앙 가로 도로)
    road_y = slice(90, 110)
    feature[:, road_y, :] += np.random.randn(bev_h, 20, channels) * 0.3

    # 차량 1: (10m 전방, 2m 오른쪽) → 인덱스 (120, 104)
    feature[118:123, 102:107, :] += np.random.randn(5, 5, channels) * 2.0

    # 차량 2: (25m 전방, -3m 왼쪽) → 인덱스 (150, 94)
    feature[148:153, 92:97, :] += np.random.randn(5, 5, channels) * 1.8

    # 보행자: (8m 전방, 5m 오른쪽) → 인덱스 (116, 110)
    feature[115:117, 109:111, :] += np.random.randn(2, 2, channels) * 1.5

    # 트럭: (15m 전방, -5m 왼쪽) → 인덱스 (130, 90)
    feature[128:136, 88:93, :] += np.random.randn(8, 5, channels) * 2.5

    return feature


def visualize_bev_methods(bev_feature):
    """다양한 BEV Feature 시각화 방법 비교"""
    H, W, C = bev_feature.shape

    fig, axes = plt.subplots(2, 3, figsize=(18, 12))

    # 1. Channel Sum (절댓값)
    ch_sum = np.abs(bev_feature).sum(axis=-1)
    ch_sum = (ch_sum - ch_sum.min()) / (ch_sum.max() - ch_sum.min() + 1e-8)
    axes[0, 0].imshow(ch_sum, cmap='hot', origin='lower')
    axes[0, 0].set_title('Channel Sum (Activation)')

    # 2. Channel Max
    ch_max = np.max(np.abs(bev_feature), axis=-1)
    ch_max = (ch_max - ch_max.min()) / (ch_max.max() - ch_max.min() + 1e-8)
    axes[0, 1].imshow(ch_max, cmap='viridis', origin='lower')
    axes[0, 1].set_title('Channel Max')

    # 3. L2 Norm
    l2_norm = np.linalg.norm(bev_feature, axis=-1)
    l2_norm = (l2_norm - l2_norm.min()) / (l2_norm.max() - l2_norm.min() + 1e-8)
    axes[0, 2].imshow(l2_norm, cmap='magma', origin='lower')
    axes[0, 2].set_title('L2 Norm')

    # 4. PCA (3 components → RGB)
    from sklearn.decomposition import PCA
    flat = bev_feature.reshape(-1, C)
    pca = PCA(n_components=3)
    rgb = pca.fit_transform(flat).reshape(H, W, 3)
    rgb = (rgb - rgb.min()) / (rgb.max() - rgb.min() + 1e-8)
    axes[1, 0].imshow(rgb, origin='lower')
    axes[1, 0].set_title(f'PCA → RGB (분산: {pca.explained_variance_ratio_.sum():.2f})')

    # 5. t-SNE 느낌 (상위 2개 PCA)
    pca2 = PCA(n_components=2)
    components = pca2.fit_transform(flat).reshape(H, W, 2)
    axes[1, 1].imshow(components[:, :, 0], cmap='coolwarm', origin='lower')
    axes[1, 1].set_title('PCA Component 1')

    # 6. 특정 채널 시각화 (가장 분산이 큰 채널)
    channel_var = bev_feature.reshape(-1, C).var(axis=0)
    top_channel = np.argmax(channel_var)
    axes[1, 2].imshow(bev_feature[:, :, top_channel], cmap='RdBu_r', origin='lower')
    axes[1, 2].set_title(f'Channel {top_channel} (최대 분산)')

    for ax in axes.flat:
        ax.set_xlabel('X (셀)')
        ax.set_ylabel('Y (셀)')

    plt.suptitle('BEV Feature Map 시각화 방법 비교 (200x200x256)', fontsize=14)
    plt.tight_layout()
    plt.savefig('bev_feature_methods.png', dpi=150)
    plt.show()
    print("저장 완료: bev_feature_methods.png")


# 실행
bev_feat = create_simulated_bev_feature()
print(f"BEV Feature shape: {bev_feat.shape}")
visualize_bev_methods(bev_feat)
```

---

## Step 3: 3D Detection 결과 BEV 시각화

```python
"""
3D Detection 결과를 BEV로 시각화
검출된 객체의 위치, 크기, 방향, 속도를 BEV에 표현한다.
"""
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.transforms import Affine2D


def create_sample_detections():
    """시뮬레이션 검출 결과 생성"""
    detections = [
        {'class': 'car', 'center': [10.0, 2.0, 0.5], 'size': [4.5, 1.8, 1.5],
         'rotation': 0.1, 'velocity': [0.5, 8.0], 'score': 0.92},
        {'class': 'car', 'center': [25.0, -3.0, 0.5], 'size': [4.2, 1.7, 1.5],
         'rotation': -0.05, 'velocity': [0.0, 12.0], 'score': 0.87},
        {'class': 'truck', 'center': [15.0, -5.0, 1.0], 'size': [8.0, 2.5, 3.0],
         'rotation': 0.0, 'velocity': [0.0, 6.0], 'score': 0.78},
        {'class': 'pedestrian', 'center': [8.0, 5.0, 0.9], 'size': [0.6, 0.6, 1.7],
         'rotation': 0.3, 'velocity': [1.0, 0.5], 'score': 0.81},
        {'class': 'pedestrian', 'center': [12.0, -8.0, 0.9], 'size': [0.6, 0.6, 1.7],
         'rotation': -0.5, 'velocity': [-0.8, 0.3], 'score': 0.65},
        {'class': 'bicycle', 'center': [18.0, 6.0, 0.5], 'size': [1.8, 0.6, 1.5],
         'rotation': 0.2, 'velocity': [0.3, 4.0], 'score': 0.55},
        {'class': 'bus', 'center': [35.0, 1.0, 1.5], 'size': [12.0, 2.8, 3.5],
         'rotation': 0.0, 'velocity': [0.0, 5.0], 'score': 0.73},
        {'class': 'traffic_cone', 'center': [6.0, 3.0, 0.3], 'size': [0.5, 0.5, 0.8],
         'rotation': 0.0, 'velocity': [0.0, 0.0], 'score': 0.88},
    ]
    return detections


def visualize_detections_bev(detections, bev_range=50):
    """3D Detection 결과를 BEV로 시각화"""
    color_map = {
        'car': '#2ecc71', 'truck': '#3498db', 'bus': '#1abc9c',
        'pedestrian': '#e74c3c', 'motorcycle': '#f39c12',
        'bicycle': '#f1c40f', 'barrier': '#95a5a6',
        'traffic_cone': '#e67e22', 'trailer': '#9b59b6',
        'construction_vehicle': '#7f8c8d',
    }

    fig, axes = plt.subplots(1, 2, figsize=(20, 9))

    for ax_idx, (ax, title) in enumerate(zip(axes, ['검출 결과 (BEV)', '검출 결과 + 속도'])):
        # ego 차량
        ego_rect = patches.FancyBboxPatch(
            (-1, -2), 2, 4, boxstyle="round,pad=0.2",
            facecolor='black', edgecolor='white', linewidth=2, alpha=0.8
        )
        ax.add_patch(ego_rect)
        ax.text(0, 0, 'EGO', ha='center', va='center', color='white', fontsize=8, fontweight='bold')

        for det in detections:
            cx, cy = det['center'][0], det['center'][1]  # y=전방, x=좌우
            l, w = det['size'][0], det['size'][1]
            theta = det['rotation']
            color = color_map.get(det['class'], 'gray')

            # 회전된 직사각형
            rect = patches.Rectangle(
                (-l/2, -w/2), l, w,
                linewidth=2, edgecolor=color, facecolor=color, alpha=0.3
            )
            t = Affine2D().rotate(theta).translate(cy, cx) + ax.transData
            rect.set_transform(t)
            ax.add_patch(rect)

            # 라벨
            label = f"{det['class']}\n{det['score']:.2f}"
            ax.text(cy, cx, label, fontsize=6, ha='center', va='center',
                   color=color, fontweight='bold')

            # 속도 화살표 (두 번째 플롯에서만)
            if ax_idx == 1 and 'velocity' in det:
                vx, vy = det['velocity']
                speed = np.sqrt(vx**2 + vy**2)
                if speed > 0.5:
                    ax.annotate('', xy=(cy + vy*0.5, cx + vx*0.5),
                               xytext=(cy, cx),
                               arrowprops=dict(arrowstyle='->', color=color,
                                             lw=2, mutation_scale=15))
                    ax.text(cy + vy*0.3, cx + vx*0.3 + 1,
                           f'{speed:.1f}m/s', fontsize=6, color=color)

        ax.set_xlim(-5, bev_range)
        ax.set_ylim(-bev_range/2, bev_range/2)
        ax.set_xlabel('Y (전방) [m]')
        ax.set_ylabel('X (좌우) [m]')
        ax.set_title(title)
        ax.set_aspect('equal')
        ax.grid(True, alpha=0.2)

        # 거리 원 표시
        for r in [10, 20, 30, 40]:
            circle = plt.Circle((0, 0), r, fill=False, color='gray',
                               linestyle='--', alpha=0.3)
            ax.add_patch(circle)
            ax.text(r, 0, f'{r}m', fontsize=7, color='gray', alpha=0.5)

    # 범례
    legend_elements = [patches.Patch(facecolor=color_map[cls], label=cls, alpha=0.5)
                      for cls in ['car', 'truck', 'bus', 'pedestrian', 'bicycle', 'traffic_cone']]
    axes[0].legend(handles=legend_elements, loc='upper right', fontsize=8)

    plt.suptitle('BEVFormer 3D Detection 결과 (BEV 시각화)', fontsize=14)
    plt.tight_layout()
    plt.savefig('detection_bev_result.png', dpi=150)
    plt.show()
    print("저장 완료: detection_bev_result.png")


# 실행
detections = create_sample_detections()
print(f"검출 객체 수: {len(detections)}")
for det in detections:
    print(f"  {det['class']}: 위치=({det['center'][0]:.1f}, {det['center'][1]:.1f}), "
          f"score={det['score']:.2f}")
visualize_detections_bev(detections)
```

---

## Step 4: 거리별 성능 분석

```python
"""
거리별 성능 분석
BEVFormer의 검출 성능이 거리에 따라 어떻게 변하는지 분석한다.
"""
import numpy as np
import matplotlib.pyplot as plt


def analyze_distance_performance():
    """거리별 검출 성능 분석"""
    # 시뮬레이션 데이터 (BEVFormer-Base 근사)
    distances = ['0-10m', '10-20m', '20-30m', '30-40m', '40-50m']
    dist_centers = [5, 15, 25, 35, 45]

    # 클래스별, 거리별 AP
    performance = {
        'car': [0.82, 0.75, 0.58, 0.38, 0.20],
        'truck': [0.65, 0.55, 0.40, 0.25, 0.12],
        'pedestrian': [0.70, 0.55, 0.35, 0.18, 0.08],
        'bicycle': [0.50, 0.35, 0.20, 0.10, 0.03],
    }

    fig, axes = plt.subplots(1, 3, figsize=(18, 5))

    # 1. 클래스별 거리-AP 곡선
    ax1 = axes[0]
    colors = {'car': 'green', 'truck': 'blue', 'pedestrian': 'red', 'bicycle': 'orange'}
    for cls, aps in performance.items():
        ax1.plot(dist_centers, aps, 'o-', color=colors[cls], label=cls, linewidth=2)
    ax1.set_xlabel('거리 (m)')
    ax1.set_ylabel('AP')
    ax1.set_title('클래스별 거리-AP 곡선')
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    # 2. 전체 mAP 거리 분석
    ax2 = axes[1]
    mean_ap = np.mean(list(performance.values()), axis=0)
    bars = ax2.bar(distances, mean_ap, color=['#2ecc71', '#27ae60', '#f39c12', '#e74c3c', '#c0392b'])
    ax2.set_xlabel('거리 범위')
    ax2.set_ylabel('mAP')
    ax2.set_title('거리 범위별 mAP')
    for bar, val in zip(bars, mean_ap):
        ax2.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.01,
                f'{val:.2f}', ha='center', fontsize=10)
    ax2.grid(True, alpha=0.3, axis='y')

    # 3. 검출 수 분포 (GT 기준)
    ax3 = axes[2]
    gt_counts = [450, 380, 280, 180, 90]  # 근사적 GT 분포
    detected = [int(c * ap) for c, ap in zip(gt_counts, mean_ap)]
    missed = [c - d for c, d in zip(gt_counts, detected)]

    x = np.arange(len(distances))
    width = 0.35
    ax3.bar(x - width/2, detected, width, label='검출 성공', color='#2ecc71')
    ax3.bar(x + width/2, missed, width, label='놓침', color='#e74c3c')
    ax3.set_xlabel('거리 범위')
    ax3.set_ylabel('객체 수')
    ax3.set_title('거리별 검출/미검출 분포')
    ax3.set_xticks(x)
    ax3.set_xticklabels(distances)
    ax3.legend()
    ax3.grid(True, alpha=0.3, axis='y')

    plt.suptitle('BEVFormer 거리별 성능 분석', fontsize=14)
    plt.tight_layout()
    plt.savefig('distance_performance.png', dpi=150)
    plt.show()
    print("저장 완료: distance_performance.png")

    # 분석 요약
    print("\n거리별 성능 분석 요약:")
    print(f"  0-10m: mAP = {mean_ap[0]:.2f} (가장 높음)")
    print(f"  10-20m: mAP = {mean_ap[1]:.2f}")
    print(f"  20-30m: mAP = {mean_ap[2]:.2f}")
    print(f"  30-40m: mAP = {mean_ap[3]:.2f}")
    print(f"  40-50m: mAP = {mean_ap[4]:.2f} (가장 낮음)")
    print()
    print("  원인 분석:")
    print("  1. 먼 거리 객체는 이미지에서 작은 영역을 차지")
    print("  2. Feature 해상도가 부족하여 정보 손실")
    print("  3. BEV 0.5m 셀에서 먼 거리의 세밀한 구분이 어려움")
    print("  4. Depth 추정의 불확실성이 거리에 비례하여 증가")


analyze_distance_performance()
```

---

## [O] 체크리스트

### 환경 세팅
- [ ] BEVFormer 코드 클론 완료
- [ ] 의존성 패키지 설치 완료
- [ ] Pretrained weight 다운로드 (또는 시뮬레이션 데이터 사용)

### NDS 지표
- [ ] NDS 공식 이해 (mAP + 5개 TP 메트릭)
- [ ] 모델 간 NDS 비교 분석
- [ ] NDS vs mAP 차이 이해

### BEV Feature 시각화
- [ ] Channel Sum 방법 시각화
- [ ] PCA → RGB 시각화
- [ ] 객체 영역과 도로 영역 구분 확인

### 성능 분석
- [ ] 거리별 AP 분석
- [ ] 클래스별 성능 차이 이해
- [ ] 성능 저하 원인 분석

---

## [tip] 추가 실험 아이디어

1. **Temporal 효과 분석**: Temporal Attention 유무에 따른 NDS 차이 비교
2. **해상도 영향**: BEV 해상도(50x50, 100x100, 200x200)에 따른 성능/속도 트레이드오프
3. **날씨/조명 분석**: 낮/밤, 맑음/비 조건별 성능 변화 조사
4. **Confidence Threshold**: threshold 변경에 따른 Precision-Recall 곡선

---

이전: [Week 10 README](./README.md)

다음: [Week 11 - 코드 및 문서 정리](../week11/README.md)
