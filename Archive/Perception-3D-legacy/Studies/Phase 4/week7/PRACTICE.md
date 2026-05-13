# Week 7 실습: nuScenes 데이터셋 탐색 및 시각화


> **목표**: nuScenes Mini 데이터셋을 다운로드하고, Devkit을 사용하여 데이터를 탐색하고 시각화한다
> **언어**: Python (nuscenes-devkit, Matplotlib)
> **예상 시간**: 10시간


---


## 실습 개요


| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | 환경 설정 및 데이터 다운로드 | 필수 | 2시간 |
| 2 | nuScenes Devkit 기본 사용법 | 필수 | 2시간 |
| 3 | 6대 카메라 동시 시각화 | 필수 | 3시간 |
| 4 | BEV 시각화 및 통계 분석 | 필수 | 3시간 |


---


## 환경 설정


```bash
# nuScenes Devkit 설치
pip install nuscenes-devkit


# 추가 패키지
pip install numpy matplotlib opencv-python Pillow pyquaternion


# nuScenes Mini 데이터 다운로드
# https://www.nuscenes.org/download 에서 다운로드
# 압축 해제: data/nuscenes/ 디렉토리에 배치
```


---


## Step 1: 데이터 구조 확인 및 기본 탐색


### 1.1 nuScenes 로드 및 기본 통계


```python
# explore_nuscenes.py
"""
nuScenes Mini 데이터셋 기본 탐색
데이터 구조를 이해하고 기본 통계를 확인합니다.
"""
import os
import numpy as np




def explore_basic_info():
    """nuScenes 기본 정보 탐색 (Devkit 없이)"""
    import json


    data_root = './data/nuscenes/v1.0-mini'


    # JSON 메타데이터 로드
    json_files = [
        'scene.json', 'sample.json', 'sample_data.json',
        'sample_annotation.json', 'category.json',
        'calibrated_sensor.json', 'ego_pose.json',
        'sensor.json', 'instance.json',
    ]


    print("=" * 50)
    print("nuScenes Mini 데이터 구조")
    print("=" * 50)


    for jf in json_files:
        filepath = os.path.join(data_root, jf)
        if os.path.exists(filepath):
            with open(filepath, 'r') as f:
                data = json.load(f)
            print(f"{jf:30s}: {len(data):6d}개 엔트리")
        else:
            print(f"{jf:30s}: 파일 없음")




def explore_with_devkit():
    """nuScenes Devkit으로 탐색"""
    from nuscenes.nuscenes import NuScenes


    nusc = NuScenes(version='v1.0-mini', dataroot='./data/nuscenes', verbose=True)


    print(f"\n{'=' * 50}")
    print("nuScenes Mini 기본 통계")
    print(f"{'=' * 50}")
    print(f"장면 수: {len(nusc.scene)}")
    print(f"Key Frame 수: {len(nusc.sample)}")
    print(f"어노테이션 수: {len(nusc.sample_annotation)}")
    print(f"인스턴스 수: {len(nusc.instance)}")


    # 클래스별 어노테이션 수
    category_counts = {}
    for ann in nusc.sample_annotation:
        cat = ann['category_name']
        category_counts[cat] = category_counts.get(cat, 0) + 1


    print(f"\n클래스별 어노테이션 수:")
    for cat, count in sorted(category_counts.items(), key=lambda x: -x[1])[:15]:
        print(f"{cat:35s}: {count:5d}")


    # 장면 정보
    print(f"\n장면 목록:")
    for scene in nusc.scene:
        print(f"{scene['name']:20s}: {scene['description'][:50]}...")


    return nusc




if __name__ == '__main__':
    explore_basic_info()
    nusc = explore_with_devkit()
```


### 1.2 장면-샘플 관계 탐색


```python
# explore_scene_sample.py
"""
Scene -> Sample -> Annotation 관계 탐색
nuScenes의 관계형 데이터 구조를 이해합니다.
"""
from nuscenes.nuscenes import NuScenes
import numpy as np




def explore_scene_chain(nusc, scene_idx=0):
    """장면 내 Key Frame 체인 탐색"""
    scene = nusc.scene[scene_idx]
    print(f"\n{'=' * 50}")
    print(f"장면: {scene['name']}")
    print(f"설명: {scene['description']}")
    print(f"{'=' * 50}")


    # 첫 Key Frame에서 시작
    sample_token = scene['first_sample_token']
    frame_idx = 0


    while sample_token:
        sample = nusc.get('sample', sample_token)


        # 센서 데이터 확인
        print(f"\n--- Key Frame {frame_idx} ---")
        print(f"Token: {sample_token[:16]}...")
        print(f"Timestamp: {sample['timestamp']}")
        print(f"어노테이션 수: {len(sample['anns'])}")


        # 연결된 센서 데이터
        print(f"연결된 센서:")
        for sensor_name in ['CAM_FRONT', 'CAM_FRONT_LEFT', 'CAM_FRONT_RIGHT',
                           'CAM_BACK', 'CAM_BACK_LEFT', 'CAM_BACK_RIGHT',
                           'LIDAR_TOP']:
            if sensor_name in sample['data']:
                sd_token = sample['data'][sensor_name]
                sd = nusc.get('sample_data', sd_token)
                print(f"{sensor_name:20s}: {os.path.basename(sd['filename'])}")


        # 어노테이션 상세
        if frame_idx == 0:
            print(f"\n 3D 어노테이션 상세 (첫 프레임):")
            for ann_token in sample['anns'][:5]:
                ann = nusc.get('sample_annotation', ann_token)
                print(f"{ann['category_name']:30s}")
                print(f"위치: [{ann['translation'][0]:.1f}, "
                      f"{ann['translation'][1]:.1f}, {ann['translation'][2]:.1f}]")
                print(f"크기: [{ann['size'][0]:.2f}, "
                      f"{ann['size'][1]:.2f}, {ann['size'][2]:.2f}] (w, l, h)")


        # 다음 Key Frame으로
        sample_token = sample['next']
        frame_idx += 1


        if frame_idx >= 5:
            print(f"\n ... (이후 {scene['nbr_samples'] - 5}개 프레임 생략)")
            break


    print(f"\n 총 Key Frame 수: {scene['nbr_samples']}")




if __name__ == '__main__':
    import os
    nusc = NuScenes(version='v1.0-mini', dataroot='./data/nuscenes', verbose=False)
    explore_scene_chain(nusc, scene_idx=0)
```


---


## Step 2: 6대 카메라 동시 시각화


### 2.1 멀티카메라 시각화


```python
# visualize_multicam.py
"""
nuScenes 6대 카메라 동시 시각화
하나의 Key Frame에서 모든 카메라의 이미지를 배치하여 360도 뷰를 확인합니다.
"""
from nuscenes.nuscenes import NuScenes
import matplotlib.pyplot as plt
from PIL import Image
import os




def visualize_six_cameras(nusc, sample_token=None):
    """6대 카메라 이미지를 하나의 그림으로 시각화"""
    if sample_token is None:
        sample_token = nusc.sample[0]['token']


    sample = nusc.get('sample', sample_token)


    # 카메라 이름과 배치 순서
    camera_names = [
        'CAM_FRONT_LEFT', 'CAM_FRONT', 'CAM_FRONT_RIGHT',
        'CAM_BACK_LEFT', 'CAM_BACK', 'CAM_BACK_RIGHT',
    ]


    display_names = [
        '전방 좌측', '전방 중앙', '전방 우측',
        '후방 좌측', '후방 중앙', '후방 우측',
    ]


    fig, axes = plt.subplots(2, 3, figsize=(18, 10))


    for idx, (cam_name, disp_name) in enumerate(zip(camera_names, display_names)):
        row, col = idx // 3, idx % 3
        ax = axes[row, col]


        # 이미지 로드
        sd_token = sample['data'][cam_name]
        sd = nusc.get('sample_data', sd_token)
        img_path = os.path.join(nusc.dataroot, sd['filename'])


        if os.path.exists(img_path):
            img = Image.open(img_path)
            ax.imshow(img)
            ax.set_title(f'{disp_name}\n({cam_name})', fontsize=11, fontweight='bold')
        else:
            ax.text(0.5, 0.5, f'이미지 없음\n{cam_name}',
                   ha='center', va='center', fontsize=12)
            ax.set_title(disp_name, fontsize=11)


        ax.axis('off')


    plt.suptitle('nuScenes 6대 카메라 - 360도 뷰', fontsize=15, fontweight='bold')
    plt.tight_layout()
    plt.savefig('nuscenes_6cameras.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("저장: nuscenes_6cameras.png")




def visualize_with_annotations(nusc, sample_token=None):
    """카메라 이미지에 3D 어노테이션 투영하여 시각화"""
    if sample_token is None:
        sample_token = nusc.sample[0]['token']


    # nuScenes Devkit의 내장 시각화 기능 사용
    nusc.render_sample(sample_token)
    plt.savefig('nuscenes_annotated.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("저장: nuscenes_annotated.png")




if __name__ == '__main__':
    nusc = NuScenes(version='v1.0-mini', dataroot='./data/nuscenes', verbose=False)
    visualize_six_cameras(nusc)
    # visualize_with_annotations(nusc) # Devkit 시각화
```


---


## Step 3: BEV 시각화


### 3.1 BEV에서 객체 배치 시각화


```python
# visualize_bev_nuscenes.py
"""
nuScenes 어노테이션을 BEV(Bird's Eye View)에서 시각화
자동차 주변 360도의 객체 배치를 확인합니다.
"""
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from nuscenes.nuscenes import NuScenes
from pyquaternion import Quaternion




def get_annotations_in_ego(nusc, sample_token):
    """
    어노테이션을 Ego Vehicle 좌표계로 변환


    nuScenes 어노테이션은 Global 좌표계이므로,
    Ego 좌표계로 변환하여 BEV 시각화합니다.
    """
    sample = nusc.get('sample', sample_token)


    # Ego pose 가져오기 (LiDAR 기준)
    lidar_token = sample['data']['LIDAR_TOP']
    lidar_data = nusc.get('sample_data', lidar_token)
    ego_pose = nusc.get('ego_pose', lidar_data['ego_pose_token'])


    ego_translation = np.array(ego_pose['translation'])
    ego_rotation = Quaternion(ego_pose['rotation'])


    annotations = []
    for ann_token in sample['anns']:
        ann = nusc.get('sample_annotation', ann_token)


        # Global -> Ego 좌표 변환
        global_pos = np.array(ann['translation'])
        ego_pos = ego_rotation.inverse.rotate(global_pos - ego_translation)


        # 회전 변환
        global_rot = Quaternion(ann['rotation'])
        ego_rot = ego_rotation.inverse * global_rot
        yaw = ego_rot.yaw_pitch_roll[0]


        annotations.append({
            'category': ann['category_name'],
            'position': ego_pos,
            'size': ann['size'], # w, l, h
            'yaw': yaw,
        })


    return annotations




def visualize_bev(nusc, sample_token=None, bev_range=50):
    """BEV 시각화"""
    if sample_token is None:
        sample_token = nusc.sample[0]['token']


    annotations = get_annotations_in_ego(nusc, sample_token)


    fig, ax = plt.subplots(1, 1, figsize=(10, 10))


    # 색상 맵
    color_map = {
        'vehicle.car': '#2ecc71',
        'vehicle.truck': '#27ae60',
        'vehicle.bus': '#1abc9c',
        'vehicle.trailer': '#16a085',
        'vehicle.construction': '#f39c12',
        'human.pedestrian': '#e74c3c',
        'vehicle.motorcycle': '#9b59b6',
        'vehicle.bicycle': '#8e44ad',
        'movable_object.barrier': '#95a5a6',
        'movable_object.trafficcone': '#e67e22',
    }


    for ann in annotations:
        x, y = ann['position'][0], ann['position'][1]


        if abs(x) > bev_range or abs(y) > bev_range:
            continue


        w, l = ann['size'][0], ann['size'][1] # width, length
        yaw = ann['yaw']


        # 카테고리 매칭 (부분 매칭)
        color = '#bdc3c7' # 기본 색상
        short_name = ann['category']
        for key, col in color_map.items():
            if key in ann['category']:
                color = col
                break


        # 회전된 사각형 그리기
        cos_y, sin_y = np.cos(yaw), np.sin(yaw)
        corners = np.array([
            [-l/2, -w/2],
            [ l/2, -w/2],
            [ l/2, w/2],
            [-l/2, w/2],
        ])


        R = np.array([[cos_y, -sin_y], [sin_y, cos_y]])
        rotated = (R @ corners.T).T + np.array([x, y])


        polygon = plt.Polygon(rotated, fill=True, alpha=0.4,
                             facecolor=color, edgecolor=color, linewidth=2)
        ax.add_patch(polygon)


        # 방향 표시
        front = R @ np.array([l/2, 0]) + np.array([x, y])
        ax.plot([x, front[0]], [y, front[1]], color=color, linewidth=2)


    # Ego Vehicle 표시
    ego_rect = plt.Rectangle((-1, -2.3), 2, 4.6, fill=True,
                             facecolor='black', alpha=0.8)
    ax.add_patch(ego_rect)
    ax.text(0, 0, 'EGO', ha='center', va='center',
           color='white', fontsize=8, fontweight='bold')


    # 카메라 FOV 표시
    fov_range = bev_range * 0.8
    for angle, name in [(0, 'FRONT'), (60, 'FL'), (-60, 'FR'),
                         (180, 'BACK'), (120, 'BL'), (-120, 'BR')]:
        rad = np.radians(angle)
        x_end = fov_range * np.sin(rad)
        y_end = fov_range * np.cos(rad)
        ax.plot([0, x_end], [0, y_end], '--', color='gray', alpha=0.3)


    # 그리드 및 범위 동심원
    for r in [10, 20, 30, 40, 50]:
        circle = plt.Circle((0, 0), r, fill=False, color='gray',
                           alpha=0.2, linestyle=':')
        ax.add_patch(circle)
        ax.text(r * 0.7, r * 0.7, f'{r}m', fontsize=8, color='gray', alpha=0.5)


    ax.set_xlim(-bev_range, bev_range)
    ax.set_ylim(-bev_range, bev_range)
    ax.set_aspect('equal')
    ax.set_xlabel('X (m)', fontsize=12)
    ax.set_ylabel('Y (m)', fontsize=12)
    ax.set_title('nuScenes BEV 시각화', fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.2)


    # 범례
    legend_items = []
    for key, color in list(color_map.items())[:6]:
        name = key.split('.')[-1]
        legend_items.append(plt.Line2D([0], [0], color=color, linewidth=3, label=name))
    ax.legend(handles=legend_items, loc='upper right', fontsize=9)


    plt.tight_layout()
    plt.savefig('nuscenes_bev.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("저장: nuscenes_bev.png")




if __name__ == '__main__':
    nusc = NuScenes(version='v1.0-mini', dataroot='./data/nuscenes', verbose=False)
    visualize_bev(nusc)
```


---


## Step 4: 통계 분석


### 4.1 데이터셋 통계


```python
# dataset_statistics.py
"""
nuScenes Mini 데이터셋의 통계 분석
클래스 분포, 거리 분포, 크기 분포를 분석합니다.
"""
import numpy as np
import matplotlib.pyplot as plt
from nuscenes.nuscenes import NuScenes
from pyquaternion import Quaternion
from collections import Counter




def analyze_statistics(nusc):
    """데이터셋 전반적 통계 분석"""


    # 클래스별 어노테이션 수
    categories = [ann['category_name'] for ann in nusc.sample_annotation]
    cat_counter = Counter(categories)


    # 거리 분포 (Ego 기준)
    distances = []
    sizes_by_class = {}


    for sample in nusc.sample:
        lidar_token = sample['data']['LIDAR_TOP']
        lidar_data = nusc.get('sample_data', lidar_token)
        ego_pose = nusc.get('ego_pose', lidar_data['ego_pose_token'])
        ego_pos = np.array(ego_pose['translation'])


        for ann_token in sample['anns']:
            ann = nusc.get('sample_annotation', ann_token)
            global_pos = np.array(ann['translation'])
            dist = np.linalg.norm(global_pos[:2] - ego_pos[:2])
            distances.append(dist)


            cat = ann['category_name'].split('.')[-1]
            if cat not in sizes_by_class:
                sizes_by_class[cat] = []
            sizes_by_class[cat].append(ann['size'])


    # 시각화
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))


    # 1. 클래스 분포
    ax = axes[0, 0]
    top_cats = cat_counter.most_common(10)
    names = [c[0].split('.')[-1] for c in top_cats]
    counts = [c[1] for c in top_cats]
    ax.barh(range(len(names)), counts, color='#3498db', alpha=0.8)
    ax.set_yticks(range(len(names)))
    ax.set_yticklabels(names)
    ax.set_xlabel('어노테이션 수')
    ax.set_title('클래스별 어노테이션 분포 (Top 10)', fontsize=12, fontweight='bold')


    # 2. 거리 분포
    ax = axes[0, 1]
    ax.hist(distances, bins=50, color='#2ecc71', alpha=0.7, edgecolor='black')
    ax.set_xlabel('Ego에서의 거리 (m)')
    ax.set_ylabel('빈도')
    ax.set_title('객체까지의 거리 분포', fontsize=12, fontweight='bold')
    ax.axvline(x=np.median(distances), color='red', linestyle='--',
              label=f'중앙값: {np.median(distances):.1f}m')
    ax.legend()


    # 3. Car 크기 분포
    ax = axes[1, 0]
    if 'car' in sizes_by_class:
        car_sizes = np.array(sizes_by_class['car'])
        labels = ['Width (w)', 'Length (l)', 'Height (h)']
        ax.boxplot([car_sizes[:, 0], car_sizes[:, 1], car_sizes[:, 2]],
                  labels=labels)
        ax.set_ylabel('크기 (m)')
        ax.set_title('Car 크기 분포 (w, l, h)', fontsize=12, fontweight='bold')


    # 4. 장면당 객체 수
    ax = axes[1, 1]
    objects_per_frame = [len(s['anns']) for s in nusc.sample]
    ax.hist(objects_per_frame, bins=30, color='#e74c3c', alpha=0.7, edgecolor='black')
    ax.set_xlabel('Key Frame당 객체 수')
    ax.set_ylabel('빈도')
    ax.set_title('Key Frame당 객체 수 분포', fontsize=12, fontweight='bold')
    ax.axvline(x=np.mean(objects_per_frame), color='blue', linestyle='--',
              label=f'평균: {np.mean(objects_per_frame):.1f}개')
    ax.legend()


    plt.suptitle('nuScenes Mini 데이터셋 통계', fontsize=14, fontweight='bold')
    plt.tight_layout()
    plt.savefig('nuscenes_statistics.png', dpi=150, bbox_inches='tight')
    plt.show()
    print("저장: nuscenes_statistics.png")




if __name__ == '__main__':
    nusc = NuScenes(version='v1.0-mini', dataroot='./data/nuscenes', verbose=False)
    analyze_statistics(nusc)
```


---


## 체크리스트


### 환경 설정
- [ ] nuscenes-devkit 설치 확인
- [ ] nuScenes Mini 데이터 다운로드 및 압축 해제
- [ ] 데이터 경로 설정 확인


### 데이터 탐색
- [ ] Scene, Sample, Annotation 관계 이해
- [ ] 클래스별 어노테이션 수 확인
- [ ] Key Frame -> 센서 데이터 접근 방법 이해


### 시각화
- [ ] 6대 카메라 동시 시각화 완료
- [ ] BEV 시각화 완료
- [ ] 통계 분석 시각화 완료


### KITTI와 비교
- [ ] 좌표계 차이 정리
- [ ] 클래스 체계 차이 정리
- [ ] 평가 지표(AP3D vs NDS) 차이 정리


---


**다음**: [Week 8 - BEV 개념 이해](../week8/PRACTICE.md)
