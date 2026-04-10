# Week 1 실습: 3D Detection 개념 탐구 및 3D BBox 시각화

> 🎯 **목표**: 3D BBox의 파라미터를 이해하고, 기본적인 3D 시각화를 Python으로 구현
> 💻 **언어**: Python (NumPy, Matplotlib)
> ⏰ **예상 시간**: 4시간

---

## 📋 실습 개요

이번 실습에서는 3D Bounding Box의 7개 파라미터를 코드로 다루고, matplotlib을 이용해 3D 공간에서 시각화합니다. 이후 2D 이미지에 투영하는 기초도 실습합니다.

---

## 🔧 환경 설정

```bash
pip install numpy matplotlib opencv-python
```

---

## Step 1: 3D BBox Corners 계산

```python
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

def compute_box_3d(x, y, z, l, w, h, theta):
    """
    3D bounding box의 8개 꼭짓점 좌표 계산

    KITTI 좌표계 기준:
      - x: 오른쪽 (+)
      - y: 아래쪽 (+)
      - z: 전방 (+)
      - theta (ry): y축 회전 (yaw)

    Parameters:
        x, y, z: 중심 좌표
        l: length (z 방향, 전후)
        w: width (x 방향, 좌우)
        h: height (y 방향, 상하)
        theta: yaw 회전각 (라디안)

    Returns:
        corners: (8, 3) ndarray - 8개 꼭짓점의 [x, y, z] 좌표
    """
    # yaw 회전 행렬 (y축 회전)
    c = np.cos(theta)
    s = np.sin(theta)
    R = np.array([
        [ c, 0, s],
        [ 0, 1, 0],
        [-s, 0, c]
    ])

    # 중심 기준 8개 꼭짓점 (회전 전)
    # 순서: 바닥 4개 → 윗면 4개
    x_corners = [ w/2,  w/2, -w/2, -w/2,  w/2,  w/2, -w/2, -w/2]
    y_corners = [   0,    0,    0,    0,   -h,   -h,   -h,   -h ]
    z_corners = [ l/2, -l/2, -l/2,  l/2,  l/2, -l/2, -l/2,  l/2]

    corners_3d = np.array([x_corners, y_corners, z_corners])  # (3, 8)

    # 회전 적용
    corners_3d = R @ corners_3d

    # 중심으로 이동
    corners_3d[0, :] += x
    corners_3d[1, :] += y
    corners_3d[2, :] += z

    return corners_3d.T  # (8, 3)


# 테스트: 전형적인 승용차
car_corners = compute_box_3d(
    x=2.0, y=1.65, z=15.0,   # 중심 (오른쪽 2m, 카메라 높이, 전방 15m)
    l=4.5, w=1.8, h=1.5,     # 크기 (길이, 폭, 높이)
    theta=0.1                 # yaw (약간 돌아간 상태)
)

print("3D BBox corners:")
print(f"Shape: {car_corners.shape}")
for i, corner in enumerate(car_corners):
    print(f"  Corner {i}: ({corner[0]:.2f}, {corner[1]:.2f}, {corner[2]:.2f})")
```

---

## Step 2: 3D 공간에서 BBox 시각화

```python
def draw_box_3d(ax, corners, color='green', linewidth=2):
    """
    matplotlib 3D axes에 3D bounding box를 그립니다.

    Parameters:
        ax: matplotlib 3D axes
        corners: (8, 3) ndarray - 8개 꼭짓점
        color: 선 색상
        linewidth: 선 두께
    """
    # 12개 엣지 정의 (바닥 4, 윗면 4, 기둥 4)
    edges = [
        [0, 1], [1, 2], [2, 3], [3, 0],  # 바닥면
        [4, 5], [5, 6], [6, 7], [7, 4],  # 윗면
        [0, 4], [1, 5], [2, 6], [3, 7],  # 기둥
    ]

    for i, j in edges:
        ax.plot3D(
            [corners[i, 0], corners[j, 0]],
            [corners[i, 2], corners[j, 2]],  # z를 y축으로 (전방 = 위쪽)
            [corners[i, 1], corners[j, 1]],  # y를 z축으로
            color=color, linewidth=linewidth
        )


def visualize_scene_3d():
    """
    여러 3D 객체가 있는 장면을 시각화합니다.
    """
    fig = plt.figure(figsize=(14, 10))
    ax = fig.add_subplot(111, projection='3d')

    # 장면 구성: 여러 차량
    objects = [
        # (x, y, z, l, w, h, theta, color, label)
        (2.0, 1.65, 15.0, 4.5, 1.8, 1.5, 0.1, 'green', 'Car A (15m)'),
        (-3.0, 1.65, 25.0, 4.2, 1.7, 1.5, -0.05, 'blue', 'Car B (25m)'),
        (0.5, 1.65, 8.0, 4.8, 2.0, 1.8, 0.0, 'red', 'Car C (8m)'),
        (5.0, 1.5, 20.0, 1.8, 0.8, 1.7, 1.57, 'orange', 'Pedestrian (20m)'),
    ]

    for (x, y, z, l, w, h, theta, color, label) in objects:
        corners = compute_box_3d(x, y, z, l, w, h, theta)
        draw_box_3d(ax, corners, color=color)
        # 라벨 표시
        ax.text(x, z, y - h, label, fontsize=8, color=color)

    # 카메라 위치 표시
    ax.scatter([0], [0], [0], color='black', s=100, marker='^', label='Camera')

    # 축 설정
    ax.set_xlabel('X (좌우)')
    ax.set_ylabel('Z (전방)')
    ax.set_zlabel('Y (높이)')
    ax.set_title('3D Object Detection - 장면 시각화')
    ax.legend()

    plt.tight_layout()
    plt.savefig('scene_3d.png', dpi=150)
    plt.show()
    print("✅ 3D 장면 시각화 완료! → scene_3d.png 저장됨")


visualize_scene_3d()
```

---

## Step 3: BEV (Bird's Eye View) 시각화

```python
def visualize_bev():
    """
    위에서 본 시점(BEV)으로 장면을 시각화합니다.
    """
    fig, ax = plt.subplots(1, 1, figsize=(10, 10))

    objects = [
        (2.0, 1.65, 15.0, 4.5, 1.8, 1.5, 0.1, 'green', 'Car A'),
        (-3.0, 1.65, 25.0, 4.2, 1.7, 1.5, -0.05, 'blue', 'Car B'),
        (0.5, 1.65, 8.0, 4.8, 2.0, 1.8, 0.0, 'red', 'Car C'),
        (5.0, 1.5, 20.0, 1.8, 0.8, 1.7, 1.57, 'orange', 'Pedestrian'),
    ]

    for (x, y, z, l, w, h, theta, color, label) in objects:
        corners = compute_box_3d(x, y, z, l, w, h, theta)

        # BEV: x-z 평면 (위에서 본 시점)
        bev_corners = corners[:4, [0, 2]]  # 바닥면의 x, z 좌표

        # 폴리곤 그리기
        polygon = plt.Polygon(bev_corners, fill=True, alpha=0.3,
                             facecolor=color, edgecolor=color, linewidth=2)
        ax.add_patch(polygon)

        # 라벨
        ax.text(x, z, label, fontsize=10, ha='center', color=color, fontweight='bold')

        # 방향 표시 (전면 = 화살표)
        front_center = (corners[0, [0, 2]] + corners[3, [0, 2]]) / 2
        center = np.array([x, z])
        ax.annotate('', xy=front_center, xytext=center,
                    arrowprops=dict(arrowstyle='->', color=color, lw=2))

    # 카메라 표시
    ax.plot(0, 0, 'k^', markersize=15, label='Camera (Ego)')
    ax.annotate('Camera', (0, 0), textcoords="offset points",
                xytext=(10, -15), fontsize=10)

    # 시야각 표시 (FOV ~90도)
    fov_range = 40
    ax.plot([0, -fov_range], [0, fov_range], 'k--', alpha=0.3)
    ax.plot([0, fov_range], [0, fov_range], 'k--', alpha=0.3)

    ax.set_xlabel('X (좌우) [m]', fontsize=12)
    ax.set_ylabel('Z (전방) [m]', fontsize=12)
    ax.set_title('Bird\'s Eye View (BEV) - 위에서 본 시점', fontsize=14)
    ax.set_xlim(-15, 15)
    ax.set_ylim(-5, 40)
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.3)
    ax.legend(fontsize=10)

    plt.tight_layout()
    plt.savefig('bev_view.png', dpi=150)
    plt.show()
    print("✅ BEV 시각화 완료! → bev_view.png 저장됨")


visualize_bev()
```

---

## Step 4: 2D vs 3D Detection 비교 시각화

```python
def compare_2d_3d():
    """
    2D Detection과 3D Detection의 차이를 시각적으로 비교합니다.
    """
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))

    # --- 2D Detection ---
    ax1 = axes[0]
    ax1.set_xlim(0, 640)
    ax1.set_ylim(480, 0)
    ax1.set_title('2D Detection', fontsize=14, fontweight='bold')

    # 2D bbox 그리기
    bboxes_2d = [
        (200, 200, 150, 100, 'Car A (0.95)', 'green'),
        (350, 220, 80, 60, 'Car B (0.87)', 'blue'),
        (100, 250, 120, 80, 'Car C (0.92)', 'red'),
    ]

    for (x, y, w, h, label, color) in bboxes_2d:
        rect = plt.Rectangle((x, y), w, h, fill=False,
                             edgecolor=color, linewidth=2)
        ax1.add_patch(rect)
        ax1.text(x, y - 5, label, fontsize=9, color=color, fontweight='bold')

    ax1.text(320, 460, '❌ 거리 정보 없음', fontsize=12, ha='center', color='red')
    ax1.set_xlabel('u (pixels)')
    ax1.set_ylabel('v (pixels)')

    # --- 3D Detection (BEV 표현) ---
    ax2 = axes[1]
    ax2.set_title('3D Detection (BEV)', fontsize=14, fontweight='bold')

    objects_3d = [
        (2.0, 15.0, 4.5, 1.8, 0.1, 'Car A (z=15m)', 'green'),
        (-3.0, 25.0, 4.2, 1.7, -0.05, 'Car B (z=25m)', 'blue'),
        (0.5, 8.0, 4.8, 2.0, 0.0, 'Car C (z=8m)', 'red'),
    ]

    for (x, z, l, w, theta, label, color) in objects_3d:
        corners = compute_box_3d(x, 1.65, z, l, w, 1.5, theta)
        bev = corners[:4, [0, 2]]
        polygon = plt.Polygon(bev, fill=True, alpha=0.3,
                             facecolor=color, edgecolor=color, linewidth=2)
        ax2.add_patch(polygon)
        ax2.text(x, z + 2, label, fontsize=9, ha='center', color=color, fontweight='bold')

    ax2.plot(0, 0, 'k^', markersize=12)
    ax2.set_xlabel('X [m]')
    ax2.set_ylabel('Z [m]')
    ax2.set_xlim(-10, 10)
    ax2.set_ylim(-2, 35)
    ax2.set_aspect('equal')
    ax2.grid(True, alpha=0.3)
    ax2.text(0, -1, '✅ 정확한 3D 위치 & 크기', fontsize=12, ha='center', color='green')

    plt.tight_layout()
    plt.savefig('2d_vs_3d.png', dpi=150)
    plt.show()
    print("✅ 2D vs 3D 비교 시각화 완료! → 2d_vs_3d.png 저장됨")


compare_2d_3d()
```

---

## Step 5: 3D IoU 개념 이해

```python
def compute_3d_iou_simple(box1, box2):
    """
    축 정렬된 (axis-aligned) 3D 박스의 IoU를 계산합니다.
    (간소화 버전 - 회전 무시)

    Parameters:
        box1, box2: dict with keys 'center' (x,y,z), 'size' (l,w,h)

    Returns:
        iou: 3D IoU 값
    """
    # 각 축에서의 겹침 계산
    def overlap_1d(min1, max1, min2, max2):
        return max(0, min(max1, max2) - max(min1, min2))

    c1, s1 = box1['center'], box1['size']
    c2, s2 = box2['center'], box2['size']

    # 각 축의 min, max
    overlap_x = overlap_1d(c1[0]-s1[1]/2, c1[0]+s1[1]/2,
                           c2[0]-s2[1]/2, c2[0]+s2[1]/2)
    overlap_y = overlap_1d(c1[1]-s1[2]/2, c1[1]+s1[2]/2,
                           c2[1]-s2[2]/2, c2[1]+s2[2]/2)
    overlap_z = overlap_1d(c1[2]-s1[0]/2, c1[2]+s1[0]/2,
                           c2[2]-s2[0]/2, c2[2]+s2[0]/2)

    # 겹치는 부피
    intersection = overlap_x * overlap_y * overlap_z

    # 각 박스의 부피
    vol1 = s1[0] * s1[1] * s1[2]
    vol2 = s2[0] * s2[1] * s2[2]

    # IoU
    union = vol1 + vol2 - intersection
    iou = intersection / union if union > 0 else 0

    return iou


# 테스트
gt_box = {'center': [2.0, 1.65, 15.0], 'size': [4.5, 1.8, 1.5]}
pred_box_good = {'center': [2.1, 1.60, 15.2], 'size': [4.4, 1.7, 1.5]}
pred_box_bad = {'center': [3.5, 1.65, 18.0], 'size': [4.0, 1.6, 1.4]}

iou_good = compute_3d_iou_simple(gt_box, pred_box_good)
iou_bad = compute_3d_iou_simple(gt_box, pred_box_bad)

print("=== 3D IoU 계산 결과 ===")
print(f"GT Box:          center={gt_box['center']}, size={gt_box['size']}")
print(f"Good Prediction: center={pred_box_good['center']}, size={pred_box_good['size']}")
print(f"  → 3D IoU = {iou_good:.4f} {'✅ TP (≥0.7)' if iou_good >= 0.7 else '❌ FP (<0.7)'}")
print(f"Bad Prediction:  center={pred_box_bad['center']}, size={pred_box_bad['size']}")
print(f"  → 3D IoU = {iou_bad:.4f} {'✅ TP (≥0.7)' if iou_bad >= 0.7 else '❌ FP (<0.7)'}")
```

---

## ✅ 체크리스트

- [ ] 3D BBox corners 계산 함수 이해 및 실행
- [ ] 3D 장면 시각화 (matplotlib 3D) 확인
- [ ] BEV 시각화 이해 (x-z 평면)
- [ ] 2D vs 3D Detection 비교 시각화 확인
- [ ] 3D IoU 계산 원리 이해
- [ ] 파라미터 변경 실험 (위치, 크기, 각도)

---

## 💡 추가 실험 아이디어

1. **다양한 yaw 각도**: theta를 0, pi/4, pi/2, pi로 바꿔서 회전 확인
2. **크기 변경**: 트럭 (l=12, w=2.5, h=3.5) vs 자전거 (l=1.8, w=0.6, h=1.5)
3. **IoU 실험**: 예측 박스를 조금씩 이동하면서 IoU 변화 관찰
4. **여러 객체 장면**: 10개 이상의 객체로 복잡한 장면 구성

---

**다음**: Week 2에서 좌표계를 자세히 학습하고, 좌표 변환을 실습합니다!
