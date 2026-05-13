# Week 9 실습: BEVFormer 핵심 구조 구현 및 시각화

> [goal] **목표**: BEVFormer의 핵심 구성 요소(BEV Query, Spatial Cross-Attention, Temporal Self-Attention)를 Python으로 구현하고 시각화한다
> [code] **언어**: Python (PyTorch, NumPy, Matplotlib)
> [time] **예상 시간**: 6시간

---

## [list] 실습 개요

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | BEV Query Grid 생성 및 시각화 | 필수 | 1.5시간 |
| 2 | 3D Reference Points 생성 및 카메라 투영 | 필수 | 2시간 |
| 3 | Spatial Cross-Attention 간소화 구현 | 필수 | 1.5시간 |
| 4 | Temporal Self-Attention 개념 구현 | 필수 | 1시간 |

---

## [tool] 환경 설정

```bash
pip install -r requirements.txt
```

---

## Step 1: BEV Query Grid 생성 및 시각화

```python
"""
BEV Query Grid 생성 및 시각화
BEVFormer에서 200x200 BEV Query가 물리 공간을 어떻게 표현하는지 이해한다.
"""
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches


def create_bev_query_grid(bev_h=200, bev_w=200, cell_size=0.5):
    """
    BEV Query Grid 생성

    Parameters:
        bev_h: BEV 그리드 높이 (셀 수)
        bev_w: BEV 그리드 너비 (셀 수)
        cell_size: 각 셀의 물리적 크기 (m)

    Returns:
        grid_centers: (bev_h, bev_w, 2) 각 셀의 중심 좌표 (m)
        x_range: x축 범위 (m)
        y_range: y축 범위 (m)
    """
    # 전체 범위: bev_h * cell_size = 200 * 0.5 = 100m
    x_range = bev_w * cell_size  # 100m
    y_range = bev_h * cell_size  # 100m

    # 각 셀의 중심 좌표 계산
    # ego 차량이 중앙에 위치 → -50m ~ +50m
    x_coords = np.linspace(-x_range/2 + cell_size/2,
                            x_range/2 - cell_size/2, bev_w)
    y_coords = np.linspace(-y_range/2 + cell_size/2,
                            y_range/2 - cell_size/2, bev_h)

    # meshgrid로 2D 그리드 생성
    xx, yy = np.meshgrid(x_coords, y_coords)
    grid_centers = np.stack([xx, yy], axis=-1)  # (200, 200, 2)

    return grid_centers, x_range, y_range


def visualize_bev_grid():
    """BEV Query Grid 시각화"""
    grid_centers, x_range, y_range = create_bev_query_grid()

    fig, axes = plt.subplots(1, 2, figsize=(16, 7))

    # --- 전체 그리드 시각화 ---
    ax1 = axes[0]
    # 일부 셀만 표시 (10x10 간격)
    step = 10
    for i in range(0, 200, step):
        for j in range(0, 200, step):
            x, y = grid_centers[i, j]
            rect = patches.Rectangle(
                (x - 0.25, y - 0.25), 0.5 * step, 0.5 * step,
                linewidth=0.5, edgecolor='blue', facecolor='lightblue', alpha=0.3
            )
            ax1.add_patch(rect)

    # ego 차량 표시
    ax1.plot(0, 0, 'r^', markersize=15, label='Ego 차량')
    ax1.set_xlim(-55, 55)
    ax1.set_ylim(-55, 55)
    ax1.set_xlabel('X (좌우) [m]')
    ax1.set_ylabel('Y (전후) [m]')
    ax1.set_title(f'BEV Query Grid (200x200, 셀={0.5}m)\n범위: {x_range}m x {y_range}m')
    ax1.set_aspect('equal')
    ax1.grid(True, alpha=0.3)
    ax1.legend()

    # --- 중앙 확대 시각화 ---
    ax2 = axes[1]
    center_range = 5  # 중앙 +-5m 확대
    for i in range(80, 120):
        for j in range(80, 120):
            x, y = grid_centers[i, j]
            if abs(x) <= center_range and abs(y) <= center_range:
                rect = patches.Rectangle(
                    (x - 0.25, y - 0.25), 0.5, 0.5,
                    linewidth=0.5, edgecolor='blue', facecolor='lightblue', alpha=0.4
                )
                ax2.add_patch(rect)

    ax2.plot(0, 0, 'r^', markersize=12, label='Ego 차량')
    ax2.set_xlim(-center_range, center_range)
    ax2.set_ylim(-center_range, center_range)
    ax2.set_xlabel('X (좌우) [m]')
    ax2.set_ylabel('Y (전후) [m]')
    ax2.set_title('중앙 확대 (0.5m 셀 크기 확인)')
    ax2.set_aspect('equal')
    ax2.grid(True, alpha=0.3)
    ax2.legend()

    plt.tight_layout()
    plt.savefig('bev_query_grid.png', dpi=150)
    plt.show()
    print("저장 완료: bev_query_grid.png")

    # 정보 출력
    print(f"\nBEV Query Grid 정보:")
    print(f"  그리드 크기: 200 x 200 = {200*200} queries")
    print(f"  셀 크기: 0.5m x 0.5m")
    print(f"  물리적 범위: {x_range}m x {y_range}m")
    print(f"  Query 벡터 차원: 256")
    print(f"  총 파라미터: {200*200*256:,} (BEV Queries) + {200*200*256:,} (Position)")


visualize_bev_grid()
```

---

## Step 2: 3D Reference Points 생성 및 카메라 투영

```python
"""
3D Reference Points 생성 및 카메라 투영
BEV Query가 이미지의 '어디를 봐야 하는지' 결정하는 과정을 이해한다.
"""
import numpy as np
import matplotlib.pyplot as plt


def create_reference_points(bev_x, bev_y, z_heights=[-5.0, -3.0, -1.0, 1.0]):
    """
    BEV Query 위치에서 3D Reference Points 생성

    Parameters:
        bev_x: BEV 공간의 x 좌표 (m)
        bev_y: BEV 공간의 y 좌표 (m, 전방이 +)
        z_heights: 참조 높이 리스트 (m)

    Returns:
        ref_points_3d: (N_heights, 3) 3D reference points
    """
    ref_points = []
    for z in z_heights:
        # BEV (x, y) → 3D (x, z_height, y)
        # nuScenes 좌표계: x=오른쪽, y=전방, z=위
        ref_points.append([bev_x, bev_y, z])
    return np.array(ref_points)


def project_to_camera(points_3d, K, extrinsic):
    """
    3D 점을 카메라 이미지에 투영

    Parameters:
        points_3d: (N, 3) 3D 좌표 (월드 좌표계)
        K: (3, 3) 카메라 내부 파라미터
        extrinsic: (4, 4) 카메라 외부 파라미터 (world → camera)

    Returns:
        points_2d: (N, 2) 이미지 좌표 (u, v)
        valid: (N,) 이미지 범위 내 여부
    """
    N = points_3d.shape[0]
    # 동차 좌표로 변환
    points_homo = np.hstack([points_3d, np.ones((N, 1))])  # (N, 4)

    # 카메라 좌표계로 변환
    points_cam = (extrinsic @ points_homo.T).T  # (N, 4)
    points_cam = points_cam[:, :3]  # (N, 3)

    # 카메라 앞에 있는 점만 유효
    in_front = points_cam[:, 2] > 0

    # 이미지에 투영
    projected = (K @ points_cam.T).T  # (N, 3)
    points_2d = projected[:, :2] / (projected[:, 2:3] + 1e-8)

    # 이미지 범위 내 확인 (1600x900 기준)
    valid = (in_front &
             (points_2d[:, 0] >= 0) & (points_2d[:, 0] < 1600) &
             (points_2d[:, 1] >= 0) & (points_2d[:, 1] < 900))

    return points_2d, valid


def simulate_multi_camera_projection():
    """
    6대 카메라에 대한 Reference Point 투영 시뮬레이션
    """
    # 간단한 카메라 내부 파라미터 (nuScenes 근사)
    K = np.array([
        [800, 0, 800],
        [0, 800, 450],
        [0, 0, 1]
    ], dtype=float)

    # 6대 카메라 외부 파라미터 (간소화)
    cameras = {
        'FRONT': np.array([
            [1, 0, 0, 0],
            [0, 1, 0, -1.5],  # 높이 1.5m
            [0, 0, 1, 0],
            [0, 0, 0, 1]
        ], dtype=float),
        'FRONT_LEFT': np.array([
            [np.cos(np.pi/3), 0, np.sin(np.pi/3), -1.0],
            [0, 1, 0, -1.5],
            [-np.sin(np.pi/3), 0, np.cos(np.pi/3), 0],
            [0, 0, 0, 1]
        ], dtype=float),
        'FRONT_RIGHT': np.array([
            [np.cos(-np.pi/3), 0, np.sin(-np.pi/3), 1.0],
            [0, 1, 0, -1.5],
            [-np.sin(-np.pi/3), 0, np.cos(-np.pi/3), 0],
            [0, 0, 0, 1]
        ], dtype=float),
    }

    # 여러 BEV 위치에 대해 Reference Points 생성 및 투영
    bev_positions = [
        (0, 10, 'BEV (0, 10m)'),
        (5, 20, 'BEV (5, 20m)'),
        (-3, 15, 'BEV (-3, 15m)'),
        (0, 30, 'BEV (0, 30m)'),
    ]

    z_heights = [-3.0, -1.0, 0.0, 1.0]

    fig, axes = plt.subplots(1, 3, figsize=(18, 6))
    colors = ['red', 'blue', 'green', 'orange']

    for cam_idx, (cam_name, extrinsic) in enumerate(cameras.items()):
        ax = axes[cam_idx]
        ax.set_xlim(0, 1600)
        ax.set_ylim(900, 0)
        ax.set_title(f'{cam_name} 카메라')
        ax.set_xlabel('u (pixels)')
        ax.set_ylabel('v (pixels)')

        for bev_idx, (bx, by, label) in enumerate(bev_positions):
            ref_points = create_reference_points(bx, by, z_heights)
            pts_2d, valid = project_to_camera(ref_points, K, extrinsic)

            for h_idx in range(len(z_heights)):
                if valid[h_idx]:
                    ax.plot(pts_2d[h_idx, 0], pts_2d[h_idx, 1],
                           'o', color=colors[bev_idx], markersize=8, alpha=0.7)
                    if h_idx == 0:
                        ax.annotate(label, (pts_2d[h_idx, 0], pts_2d[h_idx, 1]),
                                   fontsize=7, color=colors[bev_idx])

        ax.set_facecolor('lightgray')

    plt.suptitle('Spatial Cross-Attention: Reference Points의 카메라 투영', fontsize=14)
    plt.tight_layout()
    plt.savefig('reference_points_projection.png', dpi=150)
    plt.show()
    print("저장 완료: reference_points_projection.png")

    # 통계 출력
    print("\nReference Points 투영 통계:")
    print(f"  BEV 위치 수: {len(bev_positions)}")
    print(f"  높이 수: {len(z_heights)}")
    print(f"  카메라 수: {len(cameras)}")
    print(f"  총 Reference Points: {len(bev_positions) * len(z_heights) * len(cameras)}")
    print(f"  실제 BEVFormer: 200x200 x 4높이 x 6카메라 = {200*200*4*6:,} points")


simulate_multi_camera_projection()
```

---

## Step 3: Spatial Cross-Attention 간소화 구현

```python
"""
Spatial Cross-Attention 간소화 구현
BEV Query가 Image Feature에서 정보를 수집하는 과정을 이해한다.
"""
import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
import matplotlib.pyplot as plt


class SimplifiedSpatialCrossAttention(nn.Module):
    """
    간소화된 Spatial Cross-Attention

    실제 BEVFormer는 Deformable Attention을 사용하지만,
    여기서는 개념 이해를 위해 일반 Cross-Attention으로 구현한다.
    """
    def __init__(self, embed_dim=256, num_heads=8):
        super().__init__()
        self.embed_dim = embed_dim
        self.num_heads = num_heads

        # Query, Key, Value 프로젝션
        self.q_proj = nn.Linear(embed_dim, embed_dim)
        self.k_proj = nn.Linear(embed_dim, embed_dim)
        self.v_proj = nn.Linear(embed_dim, embed_dim)
        self.out_proj = nn.Linear(embed_dim, embed_dim)

    def forward(self, bev_query, image_features, reference_points_2d, valid_mask):
        """
        Parameters:
            bev_query: (B, N_q, C) BEV queries
            image_features: (B, N_cam, H*W, C) 카메라별 이미지 특징
            reference_points_2d: (B, N_q, N_cam, 2) 투영된 2D 좌표
            valid_mask: (B, N_q, N_cam) 유효한 투영 마스크

        Returns:
            output: (B, N_q, C) 업데이트된 BEV features
        """
        B, N_q, C = bev_query.shape
        N_cam = image_features.shape[1]

        # Query 투영
        Q = self.q_proj(bev_query)  # (B, N_q, C)

        # 각 카메라에서 feature 수집
        attended_features = torch.zeros_like(bev_query)
        valid_count = valid_mask.sum(dim=-1, keepdim=True).clamp(min=1)

        for cam_idx in range(N_cam):
            cam_feat = image_features[:, cam_idx]  # (B, H*W, C)

            # Key, Value 투영
            K = self.k_proj(cam_feat)  # (B, H*W, C)
            V = self.v_proj(cam_feat)  # (B, H*W, C)

            # Attention 계산
            attn = torch.bmm(Q, K.transpose(1, 2)) / (C ** 0.5)
            attn = F.softmax(attn, dim=-1)

            # Value 수집
            feat = torch.bmm(attn, V)  # (B, N_q, C)

            # 유효한 카메라만 반영
            cam_mask = valid_mask[:, :, cam_idx:cam_idx+1]  # (B, N_q, 1)
            attended_features += feat * cam_mask

        # 유효한 카메라 수로 평균
        output = self.out_proj(attended_features / valid_count)
        return output


def test_spatial_cross_attention():
    """Spatial Cross-Attention 테스트"""
    B = 1
    N_q = 100  # 10x10 BEV grid (간소화)
    N_cam = 6
    H, W = 8, 8
    C = 256

    # 임의 데이터 생성
    bev_query = torch.randn(B, N_q, C)
    image_features = torch.randn(B, N_cam, H*W, C)
    ref_points = torch.rand(B, N_q, N_cam, 2)
    valid_mask = (torch.rand(B, N_q, N_cam) > 0.3).float()

    # 모델 실행
    sca = SimplifiedSpatialCrossAttention(embed_dim=C)
    output = sca(bev_query, image_features, ref_points, valid_mask)

    print("Spatial Cross-Attention 테스트:")
    print(f"  입력 BEV Query: {bev_query.shape}")
    print(f"  Image Features: {image_features.shape}")
    print(f"  Reference Points: {ref_points.shape}")
    print(f"  Valid Mask: {valid_mask.shape}")
    print(f"  출력: {output.shape}")
    print(f"  유효 카메라 비율: {valid_mask.mean():.2f}")
    print(f"\n  파라미터 수: {sum(p.numel() for p in sca.parameters()):,}")


test_spatial_cross_attention()
```

---

## Step 4: Temporal Self-Attention 개념 구현

```python
"""
Temporal Self-Attention 개념 구현
이전 프레임의 BEV Feature를 ego-motion으로 정렬하고 현재와 합치는 과정을 이해한다.
"""
import numpy as np
import matplotlib.pyplot as plt


def warp_bev_with_ego_motion(prev_bev, ego_motion, bev_h=20, bev_w=20, cell_size=0.5):
    """
    이전 프레임의 BEV Feature를 ego-motion으로 정렬

    Parameters:
        prev_bev: (bev_h, bev_w, C) 이전 프레임의 BEV feature
        ego_motion: dict {'dx': m, 'dy': m, 'dtheta': rad}
        bev_h, bev_w: BEV 그리드 크기
        cell_size: 셀 크기 (m)

    Returns:
        warped_bev: (bev_h, bev_w, C) 정렬된 BEV feature
    """
    dx = ego_motion['dx']
    dy = ego_motion['dy']
    dtheta = ego_motion['dtheta']

    # 회전 행렬
    c, s = np.cos(dtheta), np.sin(dtheta)
    R = np.array([[c, -s], [s, c]])

    warped_bev = np.zeros_like(prev_bev)

    for i in range(bev_h):
        for j in range(bev_w):
            # 현재 셀의 물리적 좌표
            x = (j - bev_w/2) * cell_size
            y = (i - bev_h/2) * cell_size

            # ego-motion 역변환: 현재 좌표 → 이전 좌표
            prev_xy = R.T @ np.array([x - dx, y - dy])

            # 이전 BEV에서의 인덱스
            prev_j = int(prev_xy[0] / cell_size + bev_w/2)
            prev_i = int(prev_xy[1] / cell_size + bev_h/2)

            if 0 <= prev_i < bev_h and 0 <= prev_j < bev_w:
                warped_bev[i, j] = prev_bev[prev_i, prev_j]

    return warped_bev


def visualize_temporal_attention():
    """Temporal Self-Attention 과정 시각화"""
    bev_h, bev_w = 20, 20
    cell_size = 0.5

    # 이전 프레임 BEV (객체 시뮬레이션)
    prev_bev = np.zeros((bev_h, bev_w, 1))
    # 객체 A: (3m, 5m) 위치
    prev_bev[int(5/cell_size + bev_h/2), int(3/cell_size + bev_w/2)] = 1.0
    # 객체 B: (-2m, 3m) 위치
    prev_bev[int(3/cell_size + bev_h/2), int(-2/cell_size + bev_w/2)] = 0.7

    # 3x3 가우시안 블러로 객체 확산
    from scipy.ndimage import gaussian_filter
    prev_bev[:,:,0] = gaussian_filter(prev_bev[:,:,0], sigma=1.0)

    # ego-motion: 전방 1m 이동, 약간 회전
    ego_motion = {'dx': 0.0, 'dy': 1.0, 'dtheta': 0.05}

    # Warp 수행
    warped_bev = warp_bev_with_ego_motion(prev_bev, ego_motion, bev_h, bev_w, cell_size)

    # 현재 프레임 BEV (약간 다른 위치의 객체)
    curr_bev = np.zeros((bev_h, bev_w, 1))
    curr_bev[int(4/cell_size + bev_h/2), int(3/cell_size + bev_w/2)] = 1.0
    curr_bev[int(2/cell_size + bev_h/2), int(-2/cell_size + bev_w/2)] = 0.7
    curr_bev[:,:,0] = gaussian_filter(curr_bev[:,:,0], sigma=1.0)

    # 시각화
    fig, axes = plt.subplots(1, 4, figsize=(20, 5))

    axes[0].imshow(prev_bev[:,:,0], cmap='hot', origin='lower')
    axes[0].set_title('이전 프레임 BEV (t-1)')
    axes[0].set_xlabel('X')
    axes[0].set_ylabel('Y')

    axes[1].imshow(warped_bev[:,:,0], cmap='hot', origin='lower')
    axes[1].set_title('Ego-motion 정렬 후 BEV')
    axes[1].set_xlabel('X')

    axes[2].imshow(curr_bev[:,:,0], cmap='hot', origin='lower')
    axes[2].set_title('현재 프레임 BEV (t)')
    axes[2].set_xlabel('X')

    # 합쳐진 결과
    combined = np.maximum(warped_bev[:,:,0], curr_bev[:,:,0])
    axes[3].imshow(combined, cmap='hot', origin='lower')
    axes[3].set_title('Temporal 결합 결과')
    axes[3].set_xlabel('X')

    plt.suptitle('Temporal Self-Attention: 이전 BEV를 ego-motion으로 정렬 후 결합', fontsize=13)
    plt.tight_layout()
    plt.savefig('temporal_attention_viz.png', dpi=150)
    plt.show()
    print("저장 완료: temporal_attention_viz.png")

    print("\nTemporal Self-Attention 과정:")
    print(f"  1. 이전 프레임 BEV Feature 로드")
    print(f"  2. Ego-motion 적용: dx={ego_motion['dx']}m, dy={ego_motion['dy']}m, "
          f"dtheta={np.degrees(ego_motion['dtheta']):.1f}도")
    print(f"  3. 정렬된 이전 BEV와 현재 BEV Query를 concat")
    print(f"  4. Self-Attention으로 필요한 정보 선택")


visualize_temporal_attention()
```

---

## [O] 체크리스트

### BEV Query Grid
- [ ] BEV Query Grid 생성 코드 실행
- [ ] 200x200 그리드가 100m x 100m를 커버하는 것 확인
- [ ] 셀 크기(0.5m)와 물리적 범위의 관계 이해

### Reference Points
- [ ] 3D Reference Points 생성 (4개 높이)
- [ ] 카메라 투영 결과 확인 (이미지 범위 내/외)
- [ ] 여러 카메라에 투영되는 패턴 이해

### Spatial Cross-Attention
- [ ] Cross-Attention 입출력 shape 이해
- [ ] Valid Mask의 역할 이해
- [ ] BEV Query → Image Feature → BEV Feature 흐름 파악

### Temporal Self-Attention
- [ ] Ego-motion 보상 과정 시각화 확인
- [ ] 이전 BEV와 현재 BEV의 정렬 결과 비교
- [ ] Temporal 정보 활용의 장점 설명 가능

---

## [tip] 추가 실험 아이디어

1. **BEV 해상도 변경**: 50x50, 100x100, 200x200에서 커버 범위와 해상도 비교
2. **높이 개수 변경**: Reference Point의 높이를 2개, 4개, 8개로 바꿔서 투영 패턴 비교
3. **다양한 ego-motion**: 직진, 좌회전, 우회전 시 BEV 정렬 결과 비교
4. **Attention 시각화**: Cross-Attention의 weight를 히트맵으로 시각화

---

이전: [Week 9 README](./README.md)

다음: [Week 10 - BEVFormer 실습](../week10/README.md)
