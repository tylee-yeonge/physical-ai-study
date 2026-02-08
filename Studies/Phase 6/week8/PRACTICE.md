# Week 8 실습: BEV 개념 이해 및 구현

> **목표**: IPM 기반 BEV 변환을 직접 구현하고, Lift-Splat-Shoot의 Depth 분포 개념을 시각화하며, BEV 표현의 장점을 코드로 확인한다
> **언어**: Python (NumPy, Matplotlib, OpenCV)
> **예상 시간**: 10시간

---

## 실습 개요

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | IPM 기반 BEV 변환 구현 | 필수 | 3시간 |
| 2 | Depth 분포와 Lift-Splat 개념 | 필수 | 2시간 |
| 3 | BEV vs Camera View 비교 시각화 | 필수 | 2시간 |
| 4 | 3가지 BEV 방법론 비교 정리 | 필수 | 3시간 |

---

## 환경 설정

```bash
pip install numpy matplotlib opencv-python Pillow
```

---

## Step 1: IPM 기반 BEV 변환

### 1.1 Homography 기반 IPM 구현

```python
# ipm_transform.py
"""
IPM (Inverse Perspective Mapping) 구현
카메라 이미지를 BEV(위에서 본 시점)로 변환합니다.
"""
import numpy as np
import cv2
import matplotlib.pyplot as plt


def create_camera_intrinsics(fx=721.5, fy=721.5, cx=609.6, cy=172.9):
    """카메라 내부 파라미터 행렬 생성 (KITTI 기준)"""
    K = np.array([
        [fx,  0, cx],
        [ 0, fy, cy],
        [ 0,  0,  1],
    ])
    return K


def create_camera_extrinsics(height=1.65, pitch=-0.05):
    """
    카메라 외부 파라미터 생성

    Args:
        height: 카메라 높이 (지면에서, 단위: m)
        pitch: 카메라 기울기 (라디안, 음수 = 아래를 봄)
    """
    # Pitch 회전 행렬 (X축 회전)
    R = np.array([
        [1, 0,              0],
        [0, np.cos(pitch), -np.sin(pitch)],
        [0, np.sin(pitch),  np.cos(pitch)],
    ])

    # 이동 벡터 (카메라가 지면에서 height만큼 위)
    t = np.array([0, height, 0])

    return R, t


def compute_ipm_homography(K, R, t, bev_params):
    """
    IPM을 위한 Homography 행렬 계산

    Args:
        K: 카메라 내부 파라미터 (3x3)
        R: 회전 행렬 (3x3)
        t: 이동 벡터 (3,)
        bev_params: BEV 설정 dict

    Returns:
        H: Homography 행렬 (3x3)
    """
    bev_w = bev_params['width']     # BEV 이미지 너비 (pixel)
    bev_h = bev_params['height']    # BEV 이미지 높이 (pixel)
    x_range = bev_params['x_range'] # X축 범위 (m)
    z_range = bev_params['z_range'] # Z축 범위 (m)

    # 대응점 설정: BEV 공간의 4개 점 → 이미지의 4개 점
    # BEV 좌표 (미터) → 이미지 좌표 (픽셀)
    bev_points = np.float32([
        [-x_range/2, z_range],      # 왼쪽 위 (멀리)
        [ x_range/2, z_range],      # 오른쪽 위 (멀리)
        [ x_range/2, 2],            # 오른쪽 아래 (가까이)
        [-x_range/2, 2],            # 왼쪽 아래 (가까이)
    ])

    img_points = []
    for bev_pt in bev_points:
        x_world, z_world = bev_pt
        y_world = 0  # 지면 (y = 0)

        # 3D → 카메라 좌표
        point_3d = np.array([x_world, y_world, z_world])
        point_cam = R @ point_3d + t

        # 카메라 → 이미지 좌표
        point_img = K @ point_cam
        u = point_img[0] / point_img[2]
        v = point_img[1] / point_img[2]
        img_points.append([u, v])

    img_points = np.float32(img_points)

    # BEV 이미지 좌표
    bev_img_points = np.float32([
        [0, 0],
        [bev_w, 0],
        [bev_w, bev_h],
        [0, bev_h],
    ])

    # Homography 계산
    H, _ = cv2.findHomography(img_points, bev_img_points)

    return H


def apply_ipm(image, H, bev_params):
    """IPM 적용하여 BEV 이미지 생성"""
    bev_size = (bev_params['width'], bev_params['height'])
    bev_image = cv2.warpPerspective(image, H, bev_size)
    return bev_image


def demo_ipm():
    """IPM 데모"""
    # 카메라 파라미터
    K = create_camera_intrinsics()
    R, t = create_camera_extrinsics(height=1.65, pitch=-0.05)

    # BEV 설정
    bev_params = {
        'width': 400,      # BEV 이미지 너비 (pixel)
        'height': 600,     # BEV 이미지 높이 (pixel)
        'x_range': 20,     # X축 범위: -10m ~ +10m
        'z_range': 40,     # Z축 범위: 0m ~ 40m
    }

    # Homography 계산
    H = compute_ipm_homography(K, R, t, bev_params)

    # 합성 이미지로 테스트 (도로 장면 시뮬레이션)
    img = np.zeros((375, 1242, 3), dtype=np.uint8)

    # 도로 그리기 (원근법 적용)
    pts_road = np.array([
        [0, 375], [1242, 375],
        [800, 173], [442, 173]
    ], dtype=np.int32)
    cv2.fillPoly(img, [pts_road], (80, 80, 80))

    # 차선 그리기
    for x_offset in [-100, 0, 100]:
        pt1 = (621 + x_offset, 375)
        pt2 = (621 + x_offset//3, 173)
        cv2.line(img, pt1, pt2, (255, 255, 255), 2)

    # 차량 그리기 (사각형)
    cv2.rectangle(img, (550, 250), (650, 300), (0, 0, 255), -1)
    cv2.rectangle(img, (350, 220), (420, 260), (0, 255, 0), -1)

    # IPM 적용
    bev_image = apply_ipm(img, H, bev_params)

    # 시각화
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    axes[0].imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    axes[0].set_title('Camera View (전방 시점)', fontsize=13, fontweight='bold')
    axes[0].axis('off')

    axes[1].imshow(cv2.cvtColor(bev_image, cv2.COLOR_BGR2RGB))
    axes[1].set_title('BEV (위에서 본 시점) - IPM', fontsize=13, fontweight='bold')
    axes[1].set_xlabel('X (좌우)')
    axes[1].set_ylabel('Z (전방)')

    plt.suptitle('IPM (Inverse Perspective Mapping) 데모', fontsize=15, fontweight='bold')
    plt.tight_layout()
    plt.savefig('ipm_demo.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("  저장: ipm_demo.png")


if __name__ == '__main__':
    demo_ipm()
```

---

## Step 2: Depth 분포와 Lift-Splat 개념

### 2.1 Depth 분포 시각화

```python
# depth_distribution.py
"""
Lift-Splat-Shoot의 Depth 분포 개념 시각화
각 픽셀의 Depth 확률 분포를 이해합니다.
"""
import numpy as np
import matplotlib.pyplot as plt


def simulate_depth_distribution():
    """Depth 분포 시뮬레이션"""
    depth_bins = np.linspace(1, 60, 60)  # 1m ~ 60m, 1m 간격

    # 시나리오 1: 가까운 차량 (명확한 Depth)
    depth_dist_near = np.zeros(60)
    depth_dist_near[8:12] = [0.1, 0.6, 0.25, 0.05]  # 10m 부근에 집중

    # 시나리오 2: 먼 차량 (불확실한 Depth)
    depth_dist_far = np.zeros(60)
    for i in range(25, 40):
        depth_dist_far[i] = np.exp(-0.1 * (i - 32)**2)
    depth_dist_far /= depth_dist_far.sum()

    # 시나리오 3: 하늘 (Depth 없음)
    depth_dist_sky = np.ones(60) / 60  # 균일 분포

    fig, axes = plt.subplots(1, 3, figsize=(18, 5))

    axes[0].bar(depth_bins, depth_dist_near, width=0.8, color='#2ecc71', alpha=0.8)
    axes[0].set_title('가까운 차량 (z~10m)\n: 명확한 Depth', fontsize=12, fontweight='bold')
    axes[0].set_xlabel('Depth (m)')
    axes[0].set_ylabel('확률')

    axes[1].bar(depth_bins, depth_dist_far, width=0.8, color='#e74c3c', alpha=0.8)
    axes[1].set_title('먼 차량 (z~32m)\n: 불확실한 Depth', fontsize=12, fontweight='bold')
    axes[1].set_xlabel('Depth (m)')

    axes[2].bar(depth_bins, depth_dist_sky, width=0.8, color='#95a5a6', alpha=0.8)
    axes[2].set_title('하늘 (Depth 없음)\n: 균일 분포', fontsize=12, fontweight='bold')
    axes[2].set_xlabel('Depth (m)')

    plt.suptitle('Lift-Splat-Shoot: 픽셀별 Depth 분포', fontsize=14, fontweight='bold')
    plt.tight_layout()
    plt.savefig('depth_distribution.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("  저장: depth_distribution.png")


def visualize_lift_process():
    """Lift 과정 시각화: 2D Feature → 3D Feature Volume"""
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))

    # 1. 2D Feature (이미지)
    ax = axes[0]
    img = np.random.rand(8, 12)
    ax.imshow(img, cmap='viridis', aspect='auto')
    ax.set_title('2D Image Feature\n(H x W)', fontsize=12, fontweight='bold')
    ax.set_xlabel('Width')
    ax.set_ylabel('Height')

    # 2. Depth 분포 예측
    ax = axes[1]
    depth_map = np.random.rand(8, 12, 10)  # H x W x D
    depth_slice = depth_map[:, :, 5]  # 특정 depth slice
    ax.imshow(depth_slice, cmap='hot', aspect='auto')
    ax.set_title('Depth 분포 예측\n(H x W x D)', fontsize=12, fontweight='bold')
    ax.set_xlabel('Width')
    ax.set_ylabel('Height')

    # 3. 3D Feature Volume → BEV
    ax = axes[2]
    bev = np.random.rand(12, 12)
    ax.imshow(bev, cmap='plasma', aspect='equal')
    ax.set_title('BEV Feature Map\n(Z축 합산 후)', fontsize=12, fontweight='bold')
    ax.set_xlabel('X (좌우)')
    ax.set_ylabel('Y (전방)')
    ax.plot(6, 11, 'w^', markersize=15)
    ax.text(6, 11.8, 'Ego', ha='center', color='white', fontsize=9)

    plt.suptitle('Lift-Splat-Shoot: 2D → 3D → BEV 과정',
                fontsize=14, fontweight='bold')
    plt.tight_layout()
    plt.savefig('lift_splat_process.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("  저장: lift_splat_process.png")


if __name__ == '__main__':
    simulate_depth_distribution()
    visualize_lift_process()
```

---

## Step 3: BEV vs Camera View 비교

### 3.1 Occlusion 문제 시각화

```python
# bev_advantage.py
"""
BEV의 장점을 Camera View와 비교하여 시각화합니다.
특히 Occlusion 해결 능력을 강조합니다.
"""
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches


def compare_occlusion():
    """Occlusion 문제: Camera View vs BEV"""
    fig, axes = plt.subplots(1, 2, figsize=(16, 7))

    # Camera View
    ax = axes[0]
    ax.set_xlim(0, 640)
    ax.set_ylim(480, 0)
    ax.set_facecolor('#87CEEB')  # 하늘색

    # 도로
    road = plt.Polygon([[0, 480], [640, 480], [450, 200], [190, 200]],
                       facecolor='gray', alpha=0.5)
    ax.add_patch(road)

    # 가까운 차 (A) - 크게 보임
    car_a = plt.Rectangle((200, 280), 200, 150, facecolor='green',
                          edgecolor='black', linewidth=2)
    ax.add_patch(car_a)
    ax.text(300, 360, 'Car A\n(z=10m)', ha='center', va='center',
           fontsize=11, fontweight='bold', color='white')

    # 먼 차 (B) - 가려짐!
    car_b = plt.Rectangle((250, 250), 100, 50, facecolor='red',
                          edgecolor='black', linewidth=2, alpha=0.5)
    ax.add_patch(car_b)
    ax.text(300, 240, 'Car B (가려짐!)\n(z=25m)', ha='center',
           fontsize=10, color='red', fontweight='bold')

    # 다른 차 (C) - 보임
    car_c = plt.Rectangle((450, 300), 120, 100, facecolor='blue',
                          edgecolor='black', linewidth=2)
    ax.add_patch(car_c)
    ax.text(510, 350, 'Car C\n(z=15m)', ha='center', va='center',
           fontsize=10, fontweight='bold', color='white')

    ax.set_title('Camera View (전방 시점)\nCar B가 Car A에 가려짐!',
                fontsize=13, fontweight='bold')
    ax.set_xlabel('u (pixels)')
    ax.set_ylabel('v (pixels)')

    # BEV
    ax = axes[1]
    ax.set_xlim(-15, 15)
    ax.set_ylim(-5, 35)
    ax.set_facecolor('#f0f0f0')

    # 도로 그리기
    road_bev = plt.Rectangle((-4, -5), 8, 40, facecolor='gray', alpha=0.3)
    ax.add_patch(road_bev)

    # 차량들 (BEV에서는 모두 보임!)
    def draw_car_bev(ax, x, y, l, w, yaw, color, label):
        cos_y, sin_y = np.cos(yaw), np.sin(yaw)
        corners = np.array([[-l/2, -w/2], [l/2, -w/2],
                           [l/2, w/2], [-l/2, w/2]])
        R = np.array([[cos_y, -sin_y], [sin_y, cos_y]])
        rotated = (R @ corners.T).T + np.array([x, y])
        polygon = plt.Polygon(rotated, facecolor=color, edgecolor='black',
                             linewidth=2, alpha=0.7)
        ax.add_patch(polygon)
        ax.text(x, y, label, ha='center', va='center',
               fontsize=9, fontweight='bold', color='white')

    draw_car_bev(ax, 0, 10, 4.5, 1.8, 0, 'green', 'Car A')
    draw_car_bev(ax, 0.5, 25, 4.5, 1.8, 0, 'red', 'Car B')
    draw_car_bev(ax, 3, 15, 4.5, 1.8, 0.1, 'blue', 'Car C')

    # Ego
    ax.plot(0, 0, 'k^', markersize=15)
    ax.text(0, -2, 'Ego', ha='center', fontsize=10, fontweight='bold')

    # 거리 원
    for r in [10, 20, 30]:
        circle = plt.Circle((0, 0), r, fill=False, color='gray',
                           alpha=0.3, linestyle=':')
        ax.add_patch(circle)
        ax.text(r*0.7, r*0.7, f'{r}m', fontsize=8, color='gray')

    ax.set_title('BEV (위에서 본 시점)\n모든 차량이 보임!',
                fontsize=13, fontweight='bold')
    ax.set_xlabel('X (m)')
    ax.set_ylabel('Y (m)')
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.2)

    plt.suptitle('Occlusion 문제: Camera View vs BEV', fontsize=15, fontweight='bold')
    plt.tight_layout()
    plt.savefig('bev_vs_camera_occlusion.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("  저장: bev_vs_camera_occlusion.png")


def compare_multicamera_fusion():
    """Multi-camera 융합: BEV의 역할"""
    fig, axes = plt.subplots(1, 2, figsize=(16, 7))

    # 6대 카메라 개별 처리
    ax = axes[0]
    camera_names = ['FRONT', 'FL', 'FR', 'BACK', 'BL', 'BR']
    angles = [0, 60, -60, 180, 120, -120]
    colors = ['#e74c3c', '#f39c12', '#2ecc71', '#3498db', '#9b59b6', '#1abc9c']

    for name, angle, color in zip(camera_names, angles, colors):
        rad = np.radians(angle)
        x = 20 * np.sin(rad)
        y = 20 * np.cos(rad)
        ax.annotate('', xy=(x, y), xytext=(0, 0),
                   arrowprops=dict(arrowstyle='->', color=color, lw=2))
        ax.text(x*1.2, y*1.2, name, ha='center', fontsize=9,
               color=color, fontweight='bold')

    ax.plot(0, 0, 'k^', markersize=15)
    ax.set_xlim(-30, 30)
    ax.set_ylim(-30, 30)
    ax.set_aspect('equal')
    ax.set_title('개별 카메라 처리\n통합이 어려움', fontsize=13, fontweight='bold')
    ax.grid(True, alpha=0.2)

    # BEV 통합
    ax = axes[1]
    bev_size = 25

    # BEV Feature Map 시뮬레이션
    bev_map = np.zeros((50, 50))
    for _ in range(20):
        x, y = np.random.randint(5, 45), np.random.randint(5, 45)
        bev_map[max(0, y-1):min(50, y+2), max(0, x-1):min(50, x+2)] = np.random.uniform(0.5, 1)

    ax.imshow(bev_map, cmap='plasma', extent=[-bev_size, bev_size, -bev_size, bev_size])
    ax.plot(0, 0, 'w^', markersize=15)
    ax.text(0, -3, 'Ego', ha='center', color='white', fontsize=10, fontweight='bold')
    ax.set_title('BEV Feature Map (통합)\n6대 카메라 정보가 하나로!',
                fontsize=13, fontweight='bold')
    ax.set_xlabel('X (m)')
    ax.set_ylabel('Y (m)')

    plt.suptitle('Multi-camera 융합: BEV의 역할', fontsize=15, fontweight='bold')
    plt.tight_layout()
    plt.savefig('bev_multicam_fusion.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("  저장: bev_multicam_fusion.png")


if __name__ == '__main__':
    compare_occlusion()
    compare_multicamera_fusion()
```

---

## Step 4: 3가지 BEV 방법론 비교

### 4.1 방법론 비교 시각화

```python
# bev_methods_comparison.py
"""
BEV 생성 3가지 방법론 비교 시각화
IPM vs Lift-Splat-Shoot vs BEVFormer
"""
import matplotlib.pyplot as plt
import numpy as np


def compare_bev_methods():
    """3가지 BEV 생성 방법론 파이프라인 비교"""
    fig, axes = plt.subplots(1, 3, figsize=(20, 8))

    methods = [
        {
            'name': 'IPM\n(Inverse Perspective Mapping)',
            'steps': ['Camera Image', 'Homography H', 'BEV Image'],
            'pros': ['학습 불필요', '빠른 처리', '구현 간단'],
            'cons': ['평면 가정 필수', '3D 객체 왜곡', '높이 정보 손실'],
            'color': '#3498db',
            'nds': '-',
        },
        {
            'name': 'Lift-Splat-Shoot\n(MLP 기반)',
            'steps': ['Camera Image', 'Depth 분포 예측', '3D Scatter', 'Z축 합산', 'BEV Feature'],
            'pros': ['평면 가정 불필요', '학습 기반 적응', 'Multi-cam 융합'],
            'cons': ['Depth 정확도 의존', '메모리 사용 큼', '추가 감독 필요'],
            'color': '#2ecc71',
            'nds': '~0.39',
        },
        {
            'name': 'BEVFormer\n(Transformer 기반)',
            'steps': ['Camera Image', 'Backbone+FPN', 'BEV Query', 'Cross-Attention',
                      'Temporal Attn', 'BEV Feature'],
            'pros': ['Depth 명시적 예측 X', 'Temporal 활용', 'SOTA 성능'],
            'cons': ['연산량 높음', '학습 복잡', '구현 난이도 높음'],
            'color': '#e74c3c',
            'nds': '~0.52',
        },
    ]

    for idx, method in enumerate(methods):
        ax = axes[idx]

        # 파이프라인 단계 표시
        n_steps = len(method['steps'])
        for i, step in enumerate(method['steps']):
            y = 1.0 - i * (0.8 / (n_steps - 1)) if n_steps > 1 else 0.5
            box = plt.Rectangle((0.1, y - 0.04), 0.8, 0.08,
                               facecolor=method['color'], alpha=0.3 + 0.1*i,
                               edgecolor=method['color'], linewidth=2)
            ax.add_patch(box)
            ax.text(0.5, y, step, ha='center', va='center',
                   fontsize=9, fontweight='bold')

            if i < n_steps - 1:
                ax.annotate('', xy=(0.5, y - 0.05), xytext=(0.5, y - 0.04 - 0.02),
                           arrowprops=dict(arrowstyle='->', color='gray'))

        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1.1)
        ax.set_title(method['name'], fontsize=12, fontweight='bold',
                    color=method['color'])
        ax.axis('off')

        # 장단점 표시
        y_info = -0.05
        ax.text(0.05, y_info, f"NDS: {method['nds']}", fontsize=10,
               fontweight='bold', transform=ax.transAxes)

    plt.suptitle('BEV 생성 3가지 방법론 비교', fontsize=15, fontweight='bold')
    plt.tight_layout()
    plt.savefig('bev_methods_comparison.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("  저장: bev_methods_comparison.png")


def compare_performance_radar():
    """성능 비교 레이더 차트"""
    categories = ['정확도', '속도', '구현 난이도\n(낮을수록 좋음)',
                  '메모리 효율', 'Temporal\n활용']

    ipm_values = [2, 9, 9, 9, 1]
    lss_values = [6, 5, 6, 4, 3]
    bevformer_values = [9, 3, 3, 3, 9]

    N = len(categories)
    angles = [n / float(N) * 2 * np.pi for n in range(N)]
    angles += angles[:1]

    fig, ax = plt.subplots(1, 1, figsize=(8, 8), subplot_kw=dict(polar=True))

    for values, label, color in [
        (ipm_values, 'IPM', '#3498db'),
        (lss_values, 'Lift-Splat', '#2ecc71'),
        (bevformer_values, 'BEVFormer', '#e74c3c'),
    ]:
        values_plot = values + values[:1]
        ax.plot(angles, values_plot, 'o-', linewidth=2, label=label, color=color)
        ax.fill(angles, values_plot, alpha=0.1, color=color)

    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(categories, fontsize=10)
    ax.set_ylim(0, 10)
    ax.set_title('BEV 방법론 비교\n(점수: 높을수록 좋음)',
                fontsize=14, fontweight='bold', pad=20)
    ax.legend(loc='upper right', bbox_to_anchor=(1.3, 1.1), fontsize=11)

    plt.tight_layout()
    plt.savefig('bev_methods_radar.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("  저장: bev_methods_radar.png")


if __name__ == '__main__':
    compare_bev_methods()
    compare_performance_radar()
```

---

## 체크리스트

### IPM 이해
- [ ] Homography 행렬 개념 이해
- [ ] IPM 코드 실행 및 결과 확인
- [ ] 지면 평면 가정의 한계 이해

### Lift-Splat-Shoot 이해
- [ ] Depth 분포 예측 개념 이해
- [ ] Lift (2D -> 3D) 과정 이해
- [ ] Splat (3D -> BEV) 과정 이해

### BEV 장점 이해
- [ ] Occlusion 해결 시각화 확인
- [ ] Multi-camera 융합 개념 이해
- [ ] 경로 계획과의 연결 이해

### 방법론 비교
- [ ] IPM vs Lift-Splat vs BEVFormer 비교표 작성
- [ ] 각 방법론의 장단점 정리
- [ ] 성능(NDS) 비교 이해

---

**다음**: [Week 9 - BEVFormer 이해](../week9/README.md)
