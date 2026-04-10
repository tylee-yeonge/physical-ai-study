# Week 2 실습: 좌표계 변환 및 3D BBox 이미지 투영

> **목표**: KITTI 좌표계 규약을 코드로 구현하고, 3D bbox를 이미지에 투영하는 전체 파이프라인을 완성
> **언어**: Python (NumPy, Matplotlib, OpenCV)
> **예상 시간**: 4시간

---

## 실습 개요

이번 실습에서는 KITTI 좌표계를 기반으로 3D bbox의 corners를 계산하고, 이를 이미지 평면과 BEV 평면에 투영합니다.

---

## 환경 설정

```bash
pip install numpy matplotlib opencv-python
```

---

## Step 1: KITTI 캘리브레이션 파일 파싱

```python
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches

def parse_calib(calib_str):
    """
    KITTI 캘리브레이션 문자열을 파싱합니다.
    (실제 파일 대신 문자열로 연습)
    """
    # KITTI 전형적인 P2 값 (단위: 픽셀)
    P2 = np.array([
        [721.5377, 0.0,      609.5593, 44.85728],
        [0.0,      721.5377, 172.854,  0.216379],
        [0.0,      0.0,      1.0,      0.002746]
    ])

    # 스테레오 정류 행렬 (단위 행렬에 가까움)
    R0_rect = np.eye(4)
    R0_rect[:3, :3] = np.array([
        [0.9999, 0.0098, -0.0074],
        [-0.0099, 0.9999, -0.0044],
        [0.0074, 0.0045, 0.9999]
    ])

    # LiDAR → Camera 변환
    Tr_velo_to_cam = np.eye(4)
    Tr_velo_to_cam[:3, :4] = np.array([
        [7.533745e-03, -9.999714e-01, -6.166020e-04, -4.069766e-03],
        [1.480249e-02,  7.280733e-04, -9.998902e-01, -7.631618e-02],
        [9.998621e-01,  7.523790e-03,  1.480755e-02, -2.717806e-01]
    ])

    return {'P2': P2, 'R0_rect': R0_rect, 'Tr_velo_to_cam': Tr_velo_to_cam}


calib = parse_calib(None)
print("P2 투영 행렬:")
print(calib['P2'])
print(f"\nfx = {calib['P2'][0, 0]:.2f}")
print(f"fy = {calib['P2'][1, 1]:.2f}")
print(f"cx = {calib['P2'][0, 2]:.2f}")
print(f"cy = {calib['P2'][1, 2]:.2f}")
```

---

## Step 2: 3D BBox Corners 계산 (KITTI 규약)

```python
def compute_box_3d_kitti(h, w, l, x, y, z, ry):
    """
    KITTI 규약에 따른 3D bbox corners 계산

    Parameters:
        h, w, l: 높이, 폭, 길이 (KITTI 순서)
        x, y, z: 중심 좌표 (Camera 좌표계)
        ry: yaw 회전각 (y축 회전)

    Returns:
        corners: (8, 3) 꼭짓점 좌표
    """
    c = np.cos(ry)
    s = np.sin(ry)
    R = np.array([
        [ c, 0, s],
        [ 0, 1, 0],
        [-s, 0, c]
    ])

    # 8개 꼭짓점 (중심 기준)
    x_c = [ l/2,  l/2, -l/2, -l/2,  l/2,  l/2, -l/2, -l/2]
    y_c = [   0,    0,    0,    0,   -h,   -h,   -h,   -h ]
    z_c = [ w/2, -w/2, -w/2,  w/2,  w/2, -w/2, -w/2,  w/2]

    corners = np.array([x_c, y_c, z_c])  # (3, 8)
    corners = R @ corners

    corners[0, :] += x
    corners[1, :] += y
    corners[2, :] += z

    return corners.T  # (8, 3)


# 테스트: KITTI 전형적인 Car 레이블
# Car 0.0 0 -1.56 ... 1.65 1.67 3.64 -0.65 1.71 46.70 -1.59
h, w, l = 1.65, 1.67, 3.64
x, y, z = -0.65, 1.71, 46.70
ry = -1.59

corners = compute_box_3d_kitti(h, w, l, x, y, z, ry)
print("3D BBox Corners (KITTI Car):")
for i, c in enumerate(corners):
    print(f"  Corner {i}: ({c[0]:>7.2f}, {c[1]:>7.2f}, {c[2]:>7.2f})")
```

---

## Step 3: 3D Corners를 이미지에 투영

```python
def project_to_image(corners_3d, P2):
    """
    3D Camera 좌표를 이미지 좌표로 투영

    Parameters:
        corners_3d: (N, 3) Camera 좌표계의 3D 점
        P2: (3, 4) 투영 행렬

    Returns:
        corners_2d: (N, 2) 이미지 좌표 (u, v)
    """
    N = corners_3d.shape[0]
    pts_hom = np.hstack([corners_3d, np.ones((N, 1))])  # (N, 4)

    pts_2d = (P2 @ pts_hom.T).T  # (N, 3)

    # z로 나누어 정규화 (원근 투영)
    pts_2d[:, 0] /= pts_2d[:, 2]
    pts_2d[:, 1] /= pts_2d[:, 2]

    return pts_2d[:, :2]


# 투영 테스트
P2 = calib['P2']
corners_2d = project_to_image(corners, P2)

print("\n이미지에 투영된 2D 좌표:")
for i, pt in enumerate(corners_2d):
    print(f"  Corner {i}: ({pt[0]:>7.1f}, {pt[1]:>7.1f})")
```

---

## Step 4: 3D BBox를 이미지에 시각화

```python
def draw_3d_bbox_on_image(ax, corners_2d, color='lime', linewidth=2):
    """
    이미지 위에 3D bbox의 12개 edge를 그립니다.
    """
    edges = [
        [0, 1], [1, 2], [2, 3], [3, 0],  # 바닥면
        [4, 5], [5, 6], [6, 7], [7, 4],  # 윗면
        [0, 4], [1, 5], [2, 6], [3, 7],  # 기둥
    ]

    for i, j in edges:
        ax.plot([corners_2d[i, 0], corners_2d[j, 0]],
                [corners_2d[i, 1], corners_2d[j, 1]],
                color=color, linewidth=linewidth)


def visualize_3d_bbox_projection():
    """
    여러 객체의 3D bbox를 가상 이미지에 투영하여 시각화합니다.
    """
    fig, ax = plt.subplots(1, 1, figsize=(14, 5))

    # 가상 이미지 배경 (회색)
    img = np.ones((375, 1242, 3), dtype=np.uint8) * 200
    ax.imshow(img)

    # 여러 객체
    objects = [
        # (h, w, l, x, y, z, ry, color, label)
        (1.5, 1.8, 4.5, 2.0, 1.65, 15.0, 0.1, 'lime', 'Car A (15m)'),
        (1.5, 1.7, 4.2, -3.0, 1.65, 25.0, -0.05, 'cyan', 'Car B (25m)'),
        (1.8, 2.0, 4.8, 0.5, 1.65, 8.0, 0.0, 'red', 'Car C (8m)'),
    ]

    P2 = calib['P2']

    for (h, w, l, x, y, z, ry, color, label) in objects:
        corners_3d = compute_box_3d_kitti(h, w, l, x, y, z, ry)
        corners_2d = project_to_image(corners_3d, P2)

        # 유효 범위 체크
        if np.all(corners_2d[:, 0] > 0) and np.all(corners_2d[:, 0] < 1242):
            draw_3d_bbox_on_image(ax, corners_2d, color=color)
            # 라벨
            center_2d = corners_2d.mean(axis=0)
            ax.text(center_2d[0], center_2d[1] - 20, label,
                    fontsize=10, color=color, fontweight='bold',
                    ha='center', bbox=dict(boxstyle='round', facecolor='black', alpha=0.7))

    ax.set_xlim(0, 1242)
    ax.set_ylim(375, 0)
    ax.set_title('3D BBox -> 이미지 투영', fontsize=14)
    ax.set_xlabel('u (pixels)')
    ax.set_ylabel('v (pixels)')

    plt.tight_layout()
    plt.savefig('3d_bbox_projection.png', dpi=150)
    plt.show()
    print("3D bbox 이미지 투영 시각화 완료!")


visualize_3d_bbox_projection()
```

---

## Step 5: BEV 시각화

```python
def visualize_bev_with_coordinates():
    """
    BEV 시각화와 함께 좌표계를 표시합니다.
    """
    fig, axes = plt.subplots(1, 2, figsize=(16, 8))

    objects = [
        (1.5, 1.8, 4.5, 2.0, 1.65, 15.0, 0.1, 'green', 'Car A'),
        (1.5, 1.7, 4.2, -3.0, 1.65, 25.0, -0.05, 'blue', 'Car B'),
        (1.8, 2.0, 4.8, 0.5, 1.65, 8.0, 0.0, 'red', 'Car C'),
        (1.7, 0.8, 1.8, 5.0, 1.5, 20.0, 1.57, 'orange', 'Ped'),
    ]

    # --- 3D 시각화 (측면) ---
    ax1 = axes[0]
    ax1.set_title('측면 시점 (X-Z)', fontsize=14)
    for (h, w, l, x, y, z, ry, color, label) in objects:
        corners = compute_box_3d_kitti(h, w, l, x, y, z, ry)
        # 측면: x vs z
        bev = corners[:4, [0, 2]]
        polygon = plt.Polygon(bev, fill=True, alpha=0.3,
                             facecolor=color, edgecolor=color, linewidth=2)
        ax1.add_patch(polygon)
        ax1.text(x, z, label, fontsize=9, ha='center', color=color, fontweight='bold')

    ax1.plot(0, 0, 'k^', markersize=15, label='Camera')
    ax1.set_xlabel('X (좌우) [m]')
    ax1.set_ylabel('Z (전방) [m]')
    ax1.set_xlim(-10, 10)
    ax1.set_ylim(-2, 35)
    ax1.set_aspect('equal')
    ax1.grid(True, alpha=0.3)
    ax1.legend()

    # --- 좌표계 비교 ---
    ax2 = axes[1]
    ax2.set_title('좌표계 비교', fontsize=14)
    ax2.set_xlim(-5, 5)
    ax2.set_ylim(-5, 5)

    # Camera 좌표계
    ax2.annotate('', xy=(2, 0), xytext=(0, 0),
                arrowprops=dict(arrowstyle='->', color='blue', lw=2))
    ax2.text(2.2, 0, 'Camera X\n(오른쪽)', fontsize=9, color='blue')
    ax2.annotate('', xy=(0, -2), xytext=(0, 0),
                arrowprops=dict(arrowstyle='->', color='blue', lw=2))
    ax2.text(0.2, -2.3, 'Camera Z\n(전방)', fontsize=9, color='blue')

    # LiDAR 좌표계
    ax2.annotate('', xy=(0, -2), xytext=(0, 0),
                arrowprops=dict(arrowstyle='->', color='red', lw=2, linestyle='--'))
    ax2.text(-1.8, -2.3, 'LiDAR X\n(전방)', fontsize=9, color='red')
    ax2.annotate('', xy=(-2, 0), xytext=(0, 0),
                arrowprops=dict(arrowstyle='->', color='red', lw=2, linestyle='--'))
    ax2.text(-2.5, 0.3, 'LiDAR Y\n(왼쪽)', fontsize=9, color='red')

    ax2.plot(0, 0, 'ko', markersize=10)
    ax2.text(0.2, 0.3, '원점', fontsize=10, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    ax2.set_aspect('equal')

    plt.tight_layout()
    plt.savefig('bev_and_coordinates.png', dpi=150)
    plt.show()
    print("BEV 및 좌표계 비교 시각화 완료!")


visualize_bev_with_coordinates()
```

---

## Step 6: LiDAR -> Camera 좌표 변환

```python
def lidar_to_camera(pts_lidar, calib):
    """
    LiDAR 좌표를 Camera 좌표로 변환

    변환 순서:
      LiDAR → Camera = R0_rect @ Tr_velo_to_cam @ pts_lidar
    """
    N = pts_lidar.shape[0]
    pts_hom = np.hstack([pts_lidar, np.ones((N, 1))])  # (N, 4)

    # LiDAR → Camera
    pts_cam = (calib['R0_rect'] @ calib['Tr_velo_to_cam'] @ pts_hom.T).T

    return pts_cam[:, :3]


# 테스트: LiDAR 좌표의 점
pts_lidar = np.array([
    [10.0, 0.0, 0.5],    # 전방 10m, 지면 위 0.5m
    [20.0, -2.0, 1.0],   # 전방 20m, 오른쪽 2m, 높이 1m
    [5.0, 3.0, -0.5],    # 전방 5m, 왼쪽 3m, 지면 아래 0.5m
])

pts_cam = lidar_to_camera(pts_lidar, calib)

print("LiDAR → Camera 변환:")
for i in range(len(pts_lidar)):
    print(f"  LiDAR ({pts_lidar[i][0]:>6.1f}, {pts_lidar[i][1]:>6.1f}, {pts_lidar[i][2]:>6.1f})")
    print(f"  → Cam ({pts_cam[i][0]:>6.2f}, {pts_cam[i][1]:>6.2f}, {pts_cam[i][2]:>6.2f})")
    print()
```

---

## 체크리스트

- [ ] KITTI P2 투영 행렬의 구성요소(fx, fy, cx, cy) 이해
- [ ] 3D bbox corners 계산 (KITTI 규약: h, w, l 순서)
- [ ] 3D → 2D 이미지 투영 (P2 행렬 사용)
- [ ] 3D bbox를 이미지에 시각화
- [ ] BEV 시각화 및 좌표계 비교 이해
- [ ] LiDAR → Camera 좌표 변환 이해

---

## 추가 실험 아이디어

1. **다양한 ry 값**: ry를 0, pi/4, pi/2, pi로 바꿔서 투영 확인
2. **거리에 따른 크기**: z=5, 10, 30, 50m에 같은 차를 배치하여 원근 효과 확인
3. **LiDAR 점군 투영**: 가상 LiDAR 점군을 이미지에 투영
4. **역투영**: 이미지 좌표 + depth에서 3D 좌표 복원

---

이전: [Week 1 실습](../week1/PRACTICE.md)

**다음**: Week 3에서 실제 KITTI 데이터를 다루고 시각화합니다!
