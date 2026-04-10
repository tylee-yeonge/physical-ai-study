# Week 5 실습: MMDetection3D로 KITTI 3D Detection

> 🎯 **목표**: MMDetection3D 환경을 세팅하고, FCOS3D 모델로 KITTI 3D Detection을 실습한다
> 💻 **언어**: Python (PyTorch, MMDetection3D)
> ⏰ **예상 시간**: 10시간

---

## 📋 실습 개요

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | MMDetection3D 환경 세팅 | 필수 | 2시간 |
| 2 | KITTI 데이터 준비 및 변환 | 필수 | 2시간 |
| 3 | FCOS3D 학습 실행 | 필수 | 3시간 |
| 4 | Inference 및 3D bbox 시각화 | 필수 | 3시간 |

---

## 🔧 환경 설정

```bash
# Conda 환경 생성
conda create -n mmdet3d python=3.8 -y
conda activate mmdet3d

# PyTorch 설치
pip install torch==1.13.1+cu117 torchvision==0.14.1+cu117 \
    --extra-index-url https://download.pytorch.org/whl/cu117

# OpenMMLab 도구 설치
pip install openmim
mim install mmcv-full==1.7.1
mim install mmdet==2.28.2

# MMDetection3D 소스 빌드
git clone https://github.com/open-mmlab/mmdetection3d.git
cd mmdetection3d
git checkout v1.1.1
pip install -e .

# 추가 도구
pip install numpy matplotlib opencv-python
```

---

## Step 1: 설치 확인 및 환경 테스트

### 1.1 설치 확인 스크립트

```python
# check_environment.py
"""
MMDetection3D 설치 확인 스크립트
모든 패키지가 올바르게 설치되었는지 확인합니다.
"""
import sys

def check_package(name, import_name=None):
    """패키지 설치 확인"""
    if import_name is None:
        import_name = name
    try:
        mod = __import__(import_name)
        version = getattr(mod, '__version__', 'unknown')
        print(f"  ✅ {name}: {version}")
        return True
    except ImportError:
        print(f"  ❌ {name}: 설치되지 않음")
        return False

print("=" * 50)
print("MMDetection3D 환경 확인")
print("=" * 50)

all_ok = True
all_ok &= check_package("Python", "sys")
print(f"  ✅ Python: {sys.version.split()[0]}")

all_ok &= check_package("PyTorch", "torch")
all_ok &= check_package("torchvision")
all_ok &= check_package("mmcv", "mmcv")
all_ok &= check_package("mmdet")
all_ok &= check_package("mmdet3d")
all_ok &= check_package("numpy")
all_ok &= check_package("cv2", "cv2")
all_ok &= check_package("matplotlib")

# GPU 확인
import torch
print(f"\n  GPU 사용 가능: {torch.cuda.is_available()}")
if torch.cuda.is_available():
    print(f"  GPU 이름: {torch.cuda.get_device_name(0)}")
    print(f"  GPU 메모리: {torch.cuda.get_device_properties(0).total_mem / 1e9:.1f} GB")

print("\n" + "=" * 50)
if all_ok:
    print("✅ 모든 패키지가 올바르게 설치되었습니다!")
else:
    print("❌ 일부 패키지 설치가 필요합니다.")
print("=" * 50)
```

### 1.2 간단한 모델 로드 테스트

```python
# test_model_load.py
"""
MMDetection3D 모델 로드 테스트
Config 파일에서 모델을 빌드할 수 있는지 확인합니다.
"""
from mmdet3d.models import build_detector
from mmcv import Config

# Config 로드
cfg = Config.fromfile(
    'configs/fcos3d/fcos3d_r101_caffe_fpn_gn-head_dcn_2x8_1x_nus-mono3d.py'
)

# 모델 빌드 (weight 없이)
model = build_detector(cfg.model)
print(f"모델 빌드 성공!")
print(f"모델 타입: {type(model).__name__}")

# 파라미터 수 확인
total_params = sum(p.numel() for p in model.parameters())
print(f"총 파라미터: {total_params / 1e6:.1f}M")
```

---

## Step 2: KITTI 데이터 준비

### 2.1 데이터 다운로드 및 구조 확인

```python
# prepare_kitti.py
"""
KITTI 데이터 구조 확인 및 시각화
다운로드 후 데이터가 올바른지 점검합니다.
"""
import os
import numpy as np
import cv2
import matplotlib.pyplot as plt

KITTI_ROOT = './data/kitti'

def check_kitti_structure():
    """KITTI 데이터 구조 확인"""
    required = [
        'training/image_2',
        'training/calib',
        'training/label_2',
    ]

    print("KITTI 데이터 구조 확인:")
    for path in required:
        full_path = os.path.join(KITTI_ROOT, path)
        exists = os.path.exists(full_path)
        count = len(os.listdir(full_path)) if exists else 0
        status = "✅" if exists else "❌"
        print(f"  {status} {path}: {count} 파일")

def parse_kitti_label(label_file):
    """KITTI 레이블 파싱"""
    objects = []
    with open(label_file, 'r') as f:
        for line in f.readlines():
            parts = line.strip().split()
            obj = {
                'type': parts[0],
                'truncated': float(parts[1]),
                'occluded': int(parts[2]),
                'alpha': float(parts[3]),
                'bbox_2d': [float(x) for x in parts[4:8]],
                'dimensions': [float(x) for x in parts[8:11]],  # h, w, l
                'location': [float(x) for x in parts[11:14]],   # x, y, z
                'rotation_y': float(parts[14]),
            }
            objects.append(obj)
    return objects

def parse_kitti_calib(calib_file):
    """KITTI 캘리브레이션 파싱"""
    calib = {}
    with open(calib_file, 'r') as f:
        for line in f.readlines():
            if ':' in line:
                key, value = line.split(':', 1)
                calib[key.strip()] = np.array(
                    [float(x) for x in value.strip().split()]
                )
    # P2: 3x4 투영 행렬
    calib['P2'] = calib['P2'].reshape(3, 4)
    return calib

def visualize_kitti_sample(idx=0):
    """KITTI 샘플 시각화"""
    img_file = os.path.join(KITTI_ROOT, f'training/image_2/{idx:06d}.png')
    label_file = os.path.join(KITTI_ROOT, f'training/label_2/{idx:06d}.txt')
    calib_file = os.path.join(KITTI_ROOT, f'training/calib/{idx:06d}.txt')

    # 이미지 로드
    img = cv2.imread(img_file)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    # 레이블 파싱
    objects = parse_kitti_label(label_file)
    calib = parse_kitti_calib(calib_file)

    print(f"\n샘플 {idx:06d}:")
    print(f"  이미지 크기: {img.shape}")
    print(f"  객체 수: {len(objects)}")
    for obj in objects:
        print(f"  - {obj['type']}: 위치=({obj['location'][0]:.1f}, "
              f"{obj['location'][1]:.1f}, {obj['location'][2]:.1f}), "
              f"깊이={obj['location'][2]:.1f}m")

    # 2D bbox 시각화
    for obj in objects:
        if obj['type'] in ['Car', 'Pedestrian', 'Cyclist']:
            x1, y1, x2, y2 = [int(v) for v in obj['bbox_2d']]
            color = {'Car': (255, 0, 0), 'Pedestrian': (0, 255, 0),
                     'Cyclist': (0, 0, 255)}[obj['type']]
            cv2.rectangle(img, (x1, y1), (x2, y2), color, 2)
            cv2.putText(img, f"{obj['type']} z={obj['location'][2]:.1f}m",
                       (x1, y1 - 5), cv2.FONT_HERSHEY_SIMPLEX,
                       0.5, color, 1)

    plt.figure(figsize=(15, 5))
    plt.imshow(img)
    plt.title(f'KITTI Sample {idx:06d}')
    plt.axis('off')
    plt.savefig(f'kitti_sample_{idx:06d}.png', dpi=150, bbox_inches='tight')
    plt.show()
    print(f"  → 저장: kitti_sample_{idx:06d}.png")

if __name__ == '__main__':
    check_kitti_structure()
    visualize_kitti_sample(0)
    visualize_kitti_sample(100)
```

### 2.2 데이터 변환 (pkl 생성)

```bash
# MMDetection3D 포맷으로 변환
cd mmdetection3d

python tools/create_data.py kitti \
    --root-path ./data/kitti \
    --out-dir ./data/kitti \
    --extra-tag kitti

# 결과 확인
ls -la data/kitti/*.pkl
```

---

## Step 3: FCOS3D 학습

### 3.1 Config 수정

```python
# my_fcos3d_kitti.py (커스텀 config)
"""
KITTI용 FCOS3D Config
학습 환경에 맞게 수정합니다.
"""

_base_ = 'configs/fcos3d/fcos3d_r101_caffe_fpn_gn-head_dcn_2x8_1x_nus-mono3d.py'

# 데이터 경로 수정
data_root = 'data/kitti/'
dataset_type = 'KittiMonoDataset'

# 클래스 수정 (KITTI: 3 classes)
model = dict(
    bbox_head=dict(
        num_classes=3,
    ),
)

# 배치 크기 (GPU 메모리에 맞게 조절)
data = dict(
    samples_per_gpu=2,  # RTX 3090: 2, RTX 4090: 4
    workers_per_gpu=2,
)

# 학습률 조정
optimizer = dict(
    type='AdamW',
    lr=0.0002,
    weight_decay=0.01,
)

# 학습 스케줄 (짧게 테스트)
runner = dict(type='EpochBasedRunner', max_epochs=12)

# 평가 설정
evaluation = dict(interval=2, metric='mAP')

# 로그 설정
log_config = dict(
    interval=50,
    hooks=[
        dict(type='TextLoggerHook'),
    ],
)
```

### 3.2 학습 실행

```bash
# 학습 시작
python tools/train.py \
    my_fcos3d_kitti.py \
    --work-dir work_dirs/fcos3d_kitti \
    --gpu-ids 0

# 또는 Pretrained weight 사용 (권장: 빠른 수렴)
python tools/train.py \
    my_fcos3d_kitti.py \
    --work-dir work_dirs/fcos3d_kitti \
    --gpu-ids 0 \
    --cfg-options load_from='checkpoints/fcos3d_r101_caffe_fpn_gn-head_dcn_2x8_1x_nus-mono3d.pth'
```

### 3.3 학습 로그 분석

```python
# analyze_log.py
"""
학습 로그 분석 및 Loss 시각화
"""
import json
import matplotlib.pyplot as plt

def parse_log(log_file):
    """학습 로그에서 loss 추출"""
    losses = {'epoch': [], 'loss': [], 'loss_cls': [], 'loss_bbox': []}

    with open(log_file, 'r') as f:
        for line in f:
            try:
                log = json.loads(line.strip())
                if 'epoch' in log and 'loss' in log:
                    losses['epoch'].append(log['epoch'])
                    losses['loss'].append(log['loss'])
                    losses['loss_cls'].append(log.get('loss_cls', 0))
                    losses['loss_bbox'].append(log.get('loss_bbox', 0))
            except json.JSONDecodeError:
                continue
    return losses

def plot_loss(losses):
    """Loss 커브 시각화"""
    fig, axes = plt.subplots(1, 3, figsize=(15, 4))

    axes[0].plot(losses['loss'], alpha=0.7)
    axes[0].set_title('Total Loss')
    axes[0].set_xlabel('Iteration')
    axes[0].set_ylabel('Loss')

    axes[1].plot(losses['loss_cls'], alpha=0.7, color='orange')
    axes[1].set_title('Classification Loss')
    axes[1].set_xlabel('Iteration')

    axes[2].plot(losses['loss_bbox'], alpha=0.7, color='green')
    axes[2].set_title('Bbox Regression Loss')
    axes[2].set_xlabel('Iteration')

    plt.tight_layout()
    plt.savefig('training_loss.png', dpi=150)
    plt.show()
    print("→ 저장: training_loss.png")

if __name__ == '__main__':
    log_file = 'work_dirs/fcos3d_kitti/None.log.json'  # 실제 파일명으로 변경
    losses = parse_log(log_file)
    plot_loss(losses)
```

---

## Step 4: Inference 및 3D Bbox 시각화

### 4.1 3D Bbox 투영 시각화

```python
# visualize_3d_detection.py
"""
3D Detection 결과를 이미지에 시각화합니다.
3D bbox의 8개 꼭짓점을 2D 이미지에 투영합니다.
"""
import numpy as np
import cv2
import matplotlib.pyplot as plt


def compute_3d_box_corners(dimensions, location, rotation_y):
    """
    3D bbox의 8개 꼭짓점 계산 (카메라 좌표계)

    Args:
        dimensions: [h, w, l] 높이, 너비, 길이
        location: [x, y, z] 3D 중심 좌표
        rotation_y: Y축 회전각 (라디안)

    Returns:
        corners: [8, 3] 꼭짓점 좌표
    """
    h, w, l = dimensions
    x, y, z = location

    # 회전 행렬 (Y축 기준)
    R = np.array([
        [ np.cos(rotation_y), 0, np.sin(rotation_y)],
        [0,                   1, 0                  ],
        [-np.sin(rotation_y), 0, np.cos(rotation_y)],
    ])

    # 8개 꼭짓점 (중심 기준)
    corners = np.array([
        [ l/2,  0,    w/2],
        [ l/2,  0,   -w/2],
        [-l/2,  0,   -w/2],
        [-l/2,  0,    w/2],
        [ l/2, -h,    w/2],
        [ l/2, -h,   -w/2],
        [-l/2, -h,   -w/2],
        [-l/2, -h,    w/2],
    ])

    # 회전 적용 + 위치 이동
    corners = (R @ corners.T).T + np.array([x, y, z])
    return corners


def project_to_image(points_3d, P2):
    """
    3D 점을 2D 이미지에 투영

    Args:
        points_3d: [N, 3] 3D 점들
        P2: [3, 4] 투영 행렬

    Returns:
        points_2d: [N, 2] 2D 투영 좌표
    """
    N = points_3d.shape[0]
    # 동차 좌표로 변환 [N, 4]
    points_homo = np.hstack([points_3d, np.ones((N, 1))])

    # 투영
    projected = P2 @ points_homo.T  # [3, N]
    projected = projected.T  # [N, 3]

    # z로 나누기 (정규화)
    points_2d = projected[:, :2] / projected[:, 2:3]
    return points_2d


def draw_3d_bbox_on_image(img, corners_2d, color=(0, 255, 0), thickness=2):
    """
    3D bbox의 12개 edge를 이미지에 그리기

    Args:
        img: BGR 이미지
        corners_2d: [8, 2] 투영된 2D 좌표
        color: BGR 색상
        thickness: 선 두께
    """
    # 12개 edge 정의
    edges = [
        [0, 1], [1, 2], [2, 3], [3, 0],  # 아래 면
        [4, 5], [5, 6], [6, 7], [7, 4],  # 위 면
        [0, 4], [1, 5], [2, 6], [3, 7],  # 수직 edge
    ]

    for i, j in edges:
        pt1 = tuple(corners_2d[i].astype(int))
        pt2 = tuple(corners_2d[j].astype(int))
        cv2.line(img, pt1, pt2, color, thickness)

    # 전면 표시 (더 두껍게)
    front_edges = [[0, 1], [0, 4], [1, 5], [4, 5]]
    for i, j in front_edges:
        pt1 = tuple(corners_2d[i].astype(int))
        pt2 = tuple(corners_2d[j].astype(int))
        cv2.line(img, pt1, pt2, color, thickness + 1)

    return img


def visualize_kitti_3d(idx=0, kitti_root='./data/kitti'):
    """KITTI 샘플에 3D bbox 시각화"""

    # 파일 로드
    img_file = f'{kitti_root}/training/image_2/{idx:06d}.png'
    label_file = f'{kitti_root}/training/label_2/{idx:06d}.txt'
    calib_file = f'{kitti_root}/training/calib/{idx:06d}.txt'

    img = cv2.imread(img_file)

    # 캘리브레이션 로드
    calib = {}
    with open(calib_file, 'r') as f:
        for line in f:
            if ':' in line:
                key, value = line.split(':', 1)
                calib[key.strip()] = np.array(
                    [float(x) for x in value.strip().split()]
                )
    P2 = calib['P2'].reshape(3, 4)

    # 레이블 파싱
    color_map = {
        'Car': (0, 255, 0),
        'Pedestrian': (0, 165, 255),
        'Cyclist': (255, 0, 0),
    }

    with open(label_file, 'r') as f:
        for line in f:
            parts = line.strip().split()
            obj_type = parts[0]
            if obj_type not in color_map:
                continue

            dims = [float(x) for x in parts[8:11]]   # h, w, l
            loc = [float(x) for x in parts[11:14]]    # x, y, z
            ry = float(parts[14])

            # 3D 꼭짓점 계산
            corners_3d = compute_3d_box_corners(dims, loc, ry)

            # 2D 투영
            corners_2d = project_to_image(corners_3d, P2)

            # 그리기
            color = color_map[obj_type]
            draw_3d_bbox_on_image(img, corners_2d, color=color)

            # 깊이 정보 텍스트
            center_2d = corners_2d.mean(axis=0).astype(int)
            cv2.putText(img, f'{obj_type} {loc[2]:.1f}m',
                       (center_2d[0], center_2d[1] - 10),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)

    # 저장
    result_path = f'kitti_3d_vis_{idx:06d}.png'
    cv2.imwrite(result_path, img)

    # Matplotlib으로 표시
    plt.figure(figsize=(15, 5))
    plt.imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    plt.title(f'KITTI 3D Detection - Sample {idx:06d}')
    plt.axis('off')
    plt.savefig(result_path.replace('.png', '_plt.png'), dpi=150, bbox_inches='tight')
    plt.show()
    print(f"→ 저장: {result_path}")


if __name__ == '__main__':
    for idx in [0, 10, 50, 100]:
        visualize_kitti_3d(idx)
```

### 4.2 AP3D 평가 실행

```bash
# 평가 실행
python tools/test.py \
    my_fcos3d_kitti.py \
    work_dirs/fcos3d_kitti/latest.pth \
    --eval mAP

# 예상 출력:
# Car AP3D @ 0.70:
#   Easy:     18.52
#   Moderate: 13.87
#   Hard:     11.23
# Pedestrian AP3D @ 0.50:
#   Easy:     10.15
#   Moderate:  8.42
#   Hard:      7.11
```

---

## 🔧 트러블슈팅

### 자주 발생하는 문제

| 문제 | 원인 | 해결 |
|------|------|------|
| `ModuleNotFoundError: mmcv` | 버전 불일치 | `mim install mmcv-full==1.7.1` |
| `CUDA out of memory` | 배치 크기 큼 | `samples_per_gpu=1`로 줄이기 |
| `FileNotFoundError: pkl` | 데이터 변환 안 됨 | `create_data.py` 재실행 |
| `KeyError: 'KittiMonoDataset'` | mmdet3d 버전 | `v1.1.1` 확인 |

---

## ✅ 체크리스트

### 환경 세팅
- [ ] conda 환경 생성 및 활성화
- [ ] PyTorch + CUDA 설치 확인
- [ ] mmcv, mmdet, mmdet3d 설치 확인
- [ ] GPU 사용 가능 확인

### 데이터 준비
- [ ] KITTI 데이터셋 다운로드
- [ ] 데이터 구조 확인 (image_2, calib, label_2)
- [ ] pkl 파일 생성 (create_data.py)

### 학습 및 평가
- [ ] Config 파일 이해 및 수정
- [ ] FCOS3D 학습 실행
- [ ] Loss 커브 확인
- [ ] AP3D 결과 확인 (Easy/Moderate/Hard)

### 시각화
- [ ] 3D bbox를 이미지에 투영하여 시각화
- [ ] 결과 이미지 저장 (포트폴리오용)

---

**다음**: [Week 6 - 성능 분석 및 개선](../week6/PRACTICE.md)
