# Week 3 실습: KITTI 데이터셋 로드, 파싱, 시각화


> **목표**: KITTI 레이블을 파싱하고, 2D/3D/BEV 시각화를 구현
> **언어**: Python (NumPy, Matplotlib, OpenCV)
> **예상 시간**: 5시간


---


## 실습 개요


실제 KITTI 데이터가 없어도 가상 레이블로 파이프라인을 구축하고, 데이터를 받으면 바로 적용할 수 있도록 준비합니다.


---


## 환경 설정


```bash
pip install numpy matplotlib opencv-python
```


---


## Step 1: KITTI 레이블 파싱


```python
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches




def parse_kitti_label(label_str):
    """
    KITTI 레이블 문자열을 파싱하여 객체 딕셔너리 리스트 반환


    Parameters:
        label_str: KITTI label 파일의 전체 텍스트


    Returns:
        objects: list of dict
    """
    objects = []
    for line in label_str.strip().split('\n'):
        parts = line.strip().split()
        if len(parts) < 15:
            continue


        obj = {
            'class': parts[0],
            'truncated': float(parts[1]),
            'occluded': int(parts[2]),
            'alpha': float(parts[3]),
            'x1': float(parts[4]),
            'y1': float(parts[5]),
            'x2': float(parts[6]),
            'y2': float(parts[7]),
            'h': float(parts[8]),
            'w': float(parts[9]),
            'l': float(parts[10]),
            'x': float(parts[11]),
            'y': float(parts[12]),
            'z': float(parts[13]),
            'ry': float(parts[14]),
        }
        objects.append(obj)


    return objects




# 가상 KITTI 레이블 (실제 데이터 형식과 동일)
sample_label = """Car 0.00 0 -1.56 587.01 173.33 614.12 200.12 1.65 1.67 3.64 -0.65 1.71 46.70 -1.59
Car 0.00 0 1.85 387.63 181.54 423.81 203.12 1.52 1.64 3.88 -2.01 1.74 22.17 1.58
Pedestrian 0.00 0 -0.20 712.40 143.00 810.73 307.92 1.89 0.48 0.88 1.84 1.47 8.41 0.01
Cyclist 0.00 0 -1.65 548.00 171.33 572.40 194.42 1.75 0.50 1.95 -2.60 1.55 18.61 -1.63
DontCare -1 -1 -10 527.08 174.91 547.75 186.95 -1 -1 -1 -1000 -1000 -1000 -10"""


objects = parse_kitti_label(sample_label)


print(f"파싱된 객체 수: {len(objects)}")
for obj in objects:
    print(f"{obj['class']:12s} | 2D: ({obj['x1']:.0f},{obj['y1']:.0f})-({obj['x2']:.0f},{obj['y2']:.0f}) | "
          f"3D: ({obj['x']:.1f},{obj['y']:.1f},{obj['z']:.1f}) | "
          f"size: h={obj['h']:.2f} w={obj['w']:.2f} l={obj['l']:.2f} | "
          f"ry={obj['ry']:.2f}")
```


---


## Step 2: DontCare 필터링 및 클래스별 분리


```python
def filter_objects(objects, classes=None, max_truncation=0.5, max_occlusion=2):
    """
    객체 필터링


    Parameters:
        objects: 파싱된 객체 리스트
        classes: 허용할 클래스 목록 (None이면 DontCare 제외 전부)
        max_truncation: 최대 잘림 비율
        max_occlusion: 최대 가려짐 단계
    """
    filtered = []
    for obj in objects:
        if obj['class'] == 'DontCare':
            continue
        if classes and obj['class'] not in classes:
            continue
        if obj['truncated'] > max_truncation:
            continue
        if obj['occluded'] > max_occlusion:
            continue
        filtered.append(obj)
    return filtered




# Car만 필터링
cars = filter_objects(objects, classes=['Car'])
print(f"\nCar만 필터링: {len(cars)}대")
for car in cars:
    print(f"위치: ({car['x']:.1f}, {car['y']:.1f}, {car['z']:.1f})m, "
          f"크기: {car['l']:.1f}x{car['w']:.1f}x{car['h']:.1f}m")


# 모든 클래스 (DontCare 제외)
valid_objects = filter_objects(objects)
print(f"\n유효 객체: {len(valid_objects)}개")
```


---


## Step 3: 2D BBox 시각화


```python
# 클래스별 색상
CLASS_COLORS = {
    'Car': 'lime',
    'Pedestrian': 'cyan',
    'Cyclist': 'yellow',
    'Van': 'orange',
    'Truck': 'red',
    'Person_sitting': 'magenta',
}




def visualize_2d_bbox(objects, img_size=(1242, 375)):
    """
    2D bounding box를 이미지에 시각화
    """
    fig, ax = plt.subplots(1, 1, figsize=(14, 5))


    # 가상 이미지 배경
    img = np.ones((img_size[1], img_size[0], 3), dtype=np.uint8) * 180
    ax.imshow(img)


    for obj in objects:
        if obj['class'] == 'DontCare':
            continue


        color = CLASS_COLORS.get(obj['class'], 'white')
        x1, y1, x2, y2 = obj['x1'], obj['y1'], obj['x2'], obj['y2']


        # 2D bbox 그리기
        rect = patches.Rectangle(
            (x1, y1), x2 - x1, y2 - y1,
            linewidth=2, edgecolor=color, facecolor='none'
        )
        ax.add_patch(rect)


        # 클래스 + 거리 표시
        label = f"{obj['class']} z={obj['z']:.0f}m"
        ax.text(x1, y1 - 5, label, fontsize=8, color=color,
                fontweight='bold', bbox=dict(facecolor='black', alpha=0.7, pad=1))


    ax.set_xlim(0, img_size[0])
    ax.set_ylim(img_size[1], 0)
    ax.set_title('KITTI 2D BBox 시각화', fontsize=14)
    ax.set_xlabel('u (pixels)')
    ax.set_ylabel('v (pixels)')


    plt.tight_layout()
    plt.savefig('kitti_2d_bbox.png', dpi=150)
    plt.show()
    print("2D bbox 시각화 완료!")




visualize_2d_bbox(objects)
```


---


## Step 4: 3D BBox 이미지 투영 및 시각화


```python
def compute_box_3d_kitti(h, w, l, x, y, z, ry):
    """KITTI 규약에 따른 3D bbox 8개 꼭짓점 계산"""
    c, s = np.cos(ry), np.sin(ry)
    R = np.array([[ c, 0, s], [ 0, 1, 0], [-s, 0, c]])


    x_c = [ l/2, l/2, -l/2, -l/2, l/2, l/2, -l/2, -l/2]
    y_c = [ 0, 0, 0, 0, -h, -h, -h, -h ]
    z_c = [ w/2, -w/2, -w/2, w/2, w/2, -w/2, -w/2, w/2]


    corners = R @ np.array([x_c, y_c, z_c])
    corners[0, :] += x
    corners[1, :] += y
    corners[2, :] += z


    return corners.T # (8, 3)




def project_to_image(pts_3d, P2):
    """3D Camera 좌표를 이미지 좌표로 투영"""
    N = pts_3d.shape[0]
    pts_hom = np.hstack([pts_3d, np.ones((N, 1))])
    pts_2d = (P2 @ pts_hom.T).T
    pts_2d[:, 0] /= pts_2d[:, 2]
    pts_2d[:, 1] /= pts_2d[:, 2]
    return pts_2d[:, :2]




def draw_3d_bbox(ax, corners_2d, color='lime', linewidth=2):
    """3D bbox의 12개 edge를 이미지에 그리기"""
    edges = [
        [0, 1], [1, 2], [2, 3], [3, 0], # 바닥
        [4, 5], [5, 6], [6, 7], [7, 4], # 윗면
        [0, 4], [1, 5], [2, 6], [3, 7], # 기둥
    ]
    for i, j in edges:
        ax.plot([corners_2d[i, 0], corners_2d[j, 0]],
                [corners_2d[i, 1], corners_2d[j, 1]],
                color=color, linewidth=linewidth)


    # 전면(front face) 강조 (0-1-5-4)
    front_edges = [[0, 1], [1, 5], [5, 4], [4, 0]]
    for i, j in front_edges:
        ax.plot([corners_2d[i, 0], corners_2d[j, 0]],
                [corners_2d[i, 1], corners_2d[j, 1]],
                color='red', linewidth=linewidth + 1)




def visualize_3d_bbox(objects, img_size=(1242, 375)):
    """3D bbox를 이미지에 투영하여 시각화"""
    fig, ax = plt.subplots(1, 1, figsize=(14, 5))


    img = np.ones((img_size[1], img_size[0], 3), dtype=np.uint8) * 180
    ax.imshow(img)


    # KITTI 전형적인 P2
    P2 = np.array([
        [721.5377, 0.0, 609.5593, 44.85728],
        [0.0, 721.5377, 172.854, 0.216379],
        [0.0, 0.0, 1.0, 0.002746]
    ])


    for obj in objects:
        if obj['class'] == 'DontCare':
            continue


        color = CLASS_COLORS.get(obj['class'], 'white')


        # 3D corners 계산
        corners_3d = compute_box_3d_kitti(
            obj['h'], obj['w'], obj['l'],
            obj['x'], obj['y'], obj['z'], obj['ry']
        )


        # 이미지에 투영
        corners_2d = project_to_image(corners_3d, P2)


        # 유효 범위 체크
        if np.any(corners_2d[:, 0] < 0) or np.any(corners_2d[:, 0] > img_size[0]):
            continue


        draw_3d_bbox(ax, corners_2d, color=color)


        # 라벨
        center = corners_2d.mean(axis=0)
        ax.text(center[0], center[1] - 25,
                f"{obj['class']} z={obj['z']:.0f}m",
                fontsize=8, color=color, fontweight='bold', ha='center',
                bbox=dict(facecolor='black', alpha=0.7, pad=1))


    ax.set_xlim(0, img_size[0])
    ax.set_ylim(img_size[1], 0)
    ax.set_title('KITTI 3D BBox 이미지 투영', fontsize=14)


    plt.tight_layout()
    plt.savefig('kitti_3d_bbox.png', dpi=150)
    plt.show()
    print("3D bbox 이미지 투영 시각화 완료!")




valid_objects = [o for o in objects if o['class'] != 'DontCare']
visualize_3d_bbox(valid_objects)
```


---


## Step 5: BEV 시각화


```python
def visualize_bev(objects, bev_range=(-20, 20, 0, 60)):
    """
    Bird's Eye View (BEV) 시각화


    Parameters:
        objects: 파싱된 객체 리스트
        bev_range: (x_min, x_max, z_min, z_max) 미터
    """
    fig, ax = plt.subplots(1, 1, figsize=(10, 12))


    x_min, x_max, z_min, z_max = bev_range


    for obj in objects:
        if obj['class'] == 'DontCare':
            continue


        color = CLASS_COLORS.get(obj['class'], 'gray')


        # 바닥면 4개 corners
        corners_3d = compute_box_3d_kitti(
            obj['h'], obj['w'], obj['l'],
            obj['x'], obj['y'], obj['z'], obj['ry']
        )
        bev_corners = corners_3d[:4, [0, 2]] # 바닥면의 x, z


        # 폴리곤
        polygon = plt.Polygon(bev_corners, fill=True, alpha=0.4,
                             facecolor=color, edgecolor=color, linewidth=2)
        ax.add_patch(polygon)


        # 방향 화살표 (전면 중심)
        front = (corners_3d[0, [0, 2]] + corners_3d[3, [0, 2]]) / 2
        center = np.array([obj['x'], obj['z']])
        ax.annotate('', xy=front, xytext=center,
                    arrowprops=dict(arrowstyle='->', color=color, lw=2))


        # 라벨
        ax.text(obj['x'], obj['z'] + 1.5,
                f"{obj['class']}\nz={obj['z']:.0f}m",
                fontsize=8, ha='center', color=color, fontweight='bold')


    # 카메라(Ego) 위치
    ax.plot(0, 0, 'k^', markersize=15, label='Camera (Ego)')


    # FOV 표시 (약 90도)
    fov_range = z_max
    ax.plot([0, -fov_range * 0.7], [0, fov_range], 'k--', alpha=0.2)
    ax.plot([0, fov_range * 0.7], [0, fov_range], 'k--', alpha=0.2)


    # 거리 원
    for d in [10, 20, 30, 40, 50]:
        circle = plt.Circle((0, 0), d, fill=False, linestyle='--',
                           alpha=0.15, color='gray')
        ax.add_patch(circle)
        ax.text(0.5, d, f'{d}m', fontsize=7, color='gray', alpha=0.5)


    ax.set_xlabel('X (좌우) [m]', fontsize=12)
    ax.set_ylabel('Z (전방) [m]', fontsize=12)
    ax.set_title('KITTI BEV (Bird\'s Eye View)', fontsize=14)
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(z_min - 2, z_max)
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.2)
    ax.legend(fontsize=10, loc='upper right')


    plt.tight_layout()
    plt.savefig('kitti_bev.png', dpi=150)
    plt.show()
    print("BEV 시각화 완료!")




visualize_bev(valid_objects)
```


---


## Step 6: 통합 시각화 (2D + 3D + BEV)


```python
def visualize_all(objects, img_size=(1242, 375)):
    """2D bbox, 3D bbox, BEV를 한 화면에 통합 시각화"""
    fig = plt.figure(figsize=(18, 12))


    P2 = np.array([
        [721.5377, 0.0, 609.5593, 44.85728],
        [0.0, 721.5377, 172.854, 0.216379],
        [0.0, 0.0, 1.0, 0.002746]
    ])


    valid = [o for o in objects if o['class'] != 'DontCare']


    # --- 2D BBox ---
    ax1 = fig.add_subplot(2, 2, 1)
    img = np.ones((img_size[1], img_size[0], 3), dtype=np.uint8) * 180
    ax1.imshow(img)
    for obj in valid:
        color = CLASS_COLORS.get(obj['class'], 'white')
        rect = patches.Rectangle(
            (obj['x1'], obj['y1']),
            obj['x2'] - obj['x1'], obj['y2'] - obj['y1'],
            linewidth=2, edgecolor=color, facecolor='none')
        ax1.add_patch(rect)
        ax1.text(obj['x1'], obj['y1'] - 3, obj['class'], fontsize=7, color=color)
    ax1.set_title('2D BBox')
    ax1.set_xlim(0, img_size[0])
    ax1.set_ylim(img_size[1], 0)


    # --- 3D BBox ---
    ax2 = fig.add_subplot(2, 2, 2)
    ax2.imshow(img)
    for obj in valid:
        color = CLASS_COLORS.get(obj['class'], 'white')
        c3d = compute_box_3d_kitti(obj['h'], obj['w'], obj['l'],
                                    obj['x'], obj['y'], obj['z'], obj['ry'])
        c2d = project_to_image(c3d, P2)
        if np.all(c2d[:, 0] > -100) and np.all(c2d[:, 0] < img_size[0] + 100):
            draw_3d_bbox(ax2, c2d, color=color, linewidth=1)
    ax2.set_title('3D BBox Projection')
    ax2.set_xlim(0, img_size[0])
    ax2.set_ylim(img_size[1], 0)


    # --- BEV ---
    ax3 = fig.add_subplot(2, 1, 2)
    for obj in valid:
        color = CLASS_COLORS.get(obj['class'], 'gray')
        c3d = compute_box_3d_kitti(obj['h'], obj['w'], obj['l'],
                                    obj['x'], obj['y'], obj['z'], obj['ry'])
        bev = c3d[:4, [0, 2]]
        polygon = plt.Polygon(bev, fill=True, alpha=0.4,
                             facecolor=color, edgecolor=color, linewidth=2)
        ax3.add_patch(polygon)
        ax3.text(obj['x'], obj['z'] + 1, f"{obj['class']} z={obj['z']:.0f}m",
                fontsize=8, ha='center', color=color, fontweight='bold')


    ax3.plot(0, 0, 'k^', markersize=12, label='Camera')
    ax3.set_xlabel('X [m]')
    ax3.set_ylabel('Z [m]')
    ax3.set_title('BEV (Bird\'s Eye View)')
    ax3.set_xlim(-20, 20)
    ax3.set_ylim(-2, 55)
    ax3.set_aspect('equal')
    ax3.grid(True, alpha=0.2)
    ax3.legend()


    plt.suptitle('KITTI 통합 시각화: 2D + 3D + BEV', fontsize=16, fontweight='bold')
    plt.tight_layout()
    plt.savefig('kitti_all_views.png', dpi=150)
    plt.show()
    print("통합 시각화 완료!")




visualize_all(objects)
```


---


## 체크리스트


- [ ] KITTI 레이블 파싱 함수 구현 및 테스트
- [ ] DontCare 필터링 및 클래스별 분리
- [ ] 2D bbox 시각화 (이미지 위 사각형)
- [ ] 3D bbox 이미지 투영 (12개 edge + 전면 강조)
- [ ] BEV 시각화 (X-Z 평면, 방향 화살표)
- [ ] 통합 시각화 (2D + 3D + BEV 한 화면)


---


## 추가 실험 아이디어


1. **실제 KITTI 데이터**: 다운로드 후 image_2에 실제 이미지를 배경으로 사용
2. **통계 분석**: 레이블의 거리(z) 분포, 크기 분포 히스토그램
3. **난이도 필터링**: Easy/Moderate/Hard 객체 분리 시각화
4. **alpha vs ry**: 같은 객체에 대해 alpha와 ry의 차이를 시각적으로 비교


---


이전: [Week 2 실습](../week2/PRACTICE.md)


**다음**: Week 4에서 Monocular 3D Detection 모델의 원리를 학습합니다!
