# Phase 6: 3D Perception 실습 (Python)

> 🎯 **목표**: 카메라 기반 3D 객체 검출 (KITTI → nuScenes)  
> 💻 **언어**: **Python** (PyTorch, MMDetection3D)  
> 🛠️ **환경**: PC (GPU 필수) + Jetson (옵션)  
> ⏰ **예상 시간**: ~60시간 (2개월)
> 📌 각 주차별 상세 실습은 `week{N}/PRACTICE.md`를 참고하세요. 이 문서는 Phase 전체 개요입니다.

---

## 📋 실습 개요

Phase 6은 **3D Perception의 입문**으로, 카메라 영상에서 3D bounding box를 예측합니다. 로봇/자율주행 면접의 핵심 주제입니다.

### 데이터셋 전략

| 데이터셋 | 크기 | 난이도 | 용도 |
|---------|------|--------|------|
| **KITTI 3D** | ~20GB | 낮음 | ✅ **먼저 시작** (빠른 실험) |
| **nuScenes** | ~400GB | 높음 | 실무 수준 경험 |

**추천 순서**: KITTI 3D로 개념 학습 → nuScenes로 확장

---

## 🔧 실습 1: KITTI 3D Object Detection (4주)

### Week 1: 데이터셋 준비

**KITTI 다운로드**:
```bash
# http://www.cvlibs.net/datasets/kitti/eval_object.php?obj_benchmark=3d
# 필요한 파일:
# - left color images (12 GB)
# - Velodyne point clouds (29 GB)
# - Camera calibration matrices
# - Training labels

mkdir -p ~/datasets/KITTI
cd ~/datasets/KITTI
# 압축 해제
```

**구조**:
```
KITTI/
├── training/
│   ├── image_2/      # Left camera
│   ├── calib/        # 캘리브레이션
│   ├── label_2/      # 3D bbox 레이블
│   └── velodyne/     # LiDAR (선택)
└── testing/
    └── ...
```

**레이블 포맷** (`label_2/000000.txt`):
```
Car 0.0 0 -1.56 587.01 173.33 614.12 200.12 1.65 1.67 3.64 -0.65 1.71 46.70 -1.59
└─┬─┘                 └────────── 2D bbox ───────┘  └─────── 3D info ──────┘
  클래스               (x1,y1,x2,y2)              (h,w,l)(x,y,z)(ry)
```

### Week 2-3: Monocular 3D Detection 모델 학습

**MMDetection3D 설치**:
```bash
conda create -n mmdet3d python=3.8
conda activate mmdet3d

pip install torch torchvision
pip install openmim
mim install mmcv-full
mim install mmdet

git clone https://github.com/open-mmlab/mmdetection3d.git
cd mmdetection3d
pip install -e .
```

**FCOS3D 학습**:
```bash
# Config 파일 수정 (데이터 경로 등)
vim configs/fcos3d/fcos3d_r101_caffe_fpn_gn-head_dcn_2x8_1x_nus-mono3d.py

# 학습
python tools/train.py \
    configs/fcos3d/fcos3d_r101_caffe_fpn_gn-head_dcn_2x8_1x_nus-mono3d.py \
    --work-dir work_dirs/fcos3d_kitti
```

**커스텀 코드 (간소화 버전)**:
```python
# train_mono3d.py
from mmdet3d.apis import init_model, inference_detector
from mmdet3d.datasets import build_dataset
from mmdet3d.models import build_detector
import torch

# Config 로드
config = 'configs/fcos3d/fcos3d_kitti.py'
checkpoint = None  # 사전학습 없이 시작

# 모델 빌드
model = build_detector(config.model)
model.init_weights()

# 데이터셋
train_dataset = build_dataset(config.data.train)
val_dataset = build_dataset(config.data.val)

# 학습 루프
optimizer = torch.optim.AdamW(model.parameters(), lr=0.001)

for epoch in range(100):
    for batch in train_dataloader:
        losses = model(**batch, return_loss=True)
        
        loss = sum(losses.values())
        
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
    
    # Validation
    if epoch % 10 == 0:
        evaluate(model, val_dataset)
```

### Week 4: Inference 및 시각화

**3D bbox 시각화**:
```python
# visualize_3d.py
import numpy as np
import cv2
from mmdet3d.apis import init_model, inference_mono_3d_detector

model = init_model(config, checkpoint)

img = cv2.imread('kitti/000000.png')
calib = load_calib('kitti/calib/000000.txt')

# 추론
result, data = inference_mono_3d_detector(model, img, calib)

# 3D bbox를 2D로 투영
for bbox_3d in result.pred_instances_3d.bboxes_3d:
    # 8개 corner points
    corners_3d = bbox_3d.corners.numpy()  # [8, 3]
    
    # 3D → 2D 투영
    corners_2d = project_to_image(corners_3d, calib['P2'])
    
    # 12개 edge 그리기
    edges = [[0,1],[1,2],[2,3],[3,0],  # bottom
             [4,5],[5,6],[6,7],[7,4],  # top
             [0,4],[1,5],[2,6],[3,7]]  # vertical
    
    for i, j in edges:
        pt1 = tuple(corners_2d[i].astype(int))
        pt2 = tuple(corners_2d[j].astype(int))
        cv2.line(img, pt1, pt2, (0, 255, 0), 2)

cv2.imshow('3D Detection', img)
cv2.waitKey(0)
```

---

## 🔧 실습 2: nuScenes 3D Detection (4주)

### Week 1: nuScenes 데이터셋

**다운로드** (~400GB, 시간 많이 걸림):
```bash
# https://www.nuscenes.org/download
# Full dataset (v1.0-trainval) 필요

# 압축 해제는 병렬 처리 권장
tar -xzf v1.0-trainval_meta.tgz &
tar -xzf v1.0-trainval01_blobs.tgz &
# ...
```

**구조**:
```
nuScenes/
├── maps/
├── samples/          # 키프레임 (2Hz)
│   ├── CAM_FRONT/
│   ├── CAM_FRONT_LEFT/
│   └── ...
├── sweeps/           # 중간 프레임
└── v1.0-trainval/    # 메타데이터
```

### Week 2-3: BEVFormer 개념 학습

**BEV (Bird's Eye View)**:
- 카메라 → 위에서 본 시점 변환
- Multi-camera fusion
- 3D 객체 검출, Segmentation

**BEVFormer 구조**:
```
Multi-view Images → Feature Extraction → BEV Queries → Transformer → 3D Detections
```

**코드 (MMDetection3D)**:
```bash
# BEVFormer config
python tools/train.py \
    configs/bevformer/bevformer_base.py \
    --work-dir work_dirs/bevformer_nuscenes
```

### Week 4: Inference & 시각화

**nuScenes 추론**:
```python
from nuscenes import NuScenes
from mmdet3d.apis import init_model, inference_multi_view_mono3d_detector

nusc = NuScenes(version='v1.0-trainval', dataroot='~/datasets/nuScenes')

model = init_model('bevformer_config.py', 'checkpoint.pth')

# 샘플 선택
sample = nusc.sample[100]

# 6개 카메라 이미지
images = []
for cam in ['CAM_FRONT', 'CAM_FRONT_LEFT', 'CAM_FRONT_RIGHT',
            'CAM_BACK', 'CAM_BACK_LEFT', 'CAM_BACK_RIGHT']:
    img_path = nusc.get_sample_data_path(sample['data'][cam])
    images.append(cv2.imread(img_path))

# BEV 추론
result = inference_multi_view_mono3d_detector(model, images)

# BEV 시각화
bev_map = render_bev(result, nusc)
cv2.imshow('BEV Detection', bev_map)
```

---

## 🔧 실습 3: Jetson 배포 (선택, 2주)

**목표**: 경량 3D 검출 모델을 Jetson에 배포

**방법**:
1. Knowledge Distillation으로 작은 모델 학습
2. ONNX → TensorRT 변환
3. C++ Inference

**예상 성능**:
- Jetson Orin: **10-15 FPS** (단안 3D)
- BEV는 Jetson에서 힘듦 (Multi-view → 계산량 과다)

---

## ✅ 체크리스트

### KITTI 3D
- [ ] 데이터셋 구조 이해
- [ ] Monocular 3D Detection 학습
- [ ] 3D bbox 시각화
- [ ] AP3D 지표 이해

### nuScenes
- [ ] 데이터셋 다운로드 및 탐색
- [ ] BEVFormer 개념 이해
- [ ] Multi-view 추론
- [ ] BEV 맵 시각화

### 포트폴리오
- [ ] GitHub 코드 정리
- [ ] Demo 영상 제작
- [ ] 학습 과정 블로그 작성

---

## 📚 참고 자료

- [MMDetection3D Docs](https://mmdetection3d.readthedocs.io/)
- [nuScenes Devkit](https://github.com/nutonomy/nuscenes-devkit)
- [BEVFormer Paper](https://arxiv.org/abs/2203.17270)
- [KITTI Benchmark](http://www.cvlibs.net/datasets/kitti/eval_object.php?obj_benchmark=3d)

---

## 💼 이직 준비 팁

### 포트폴리오 구성
1. **GitHub**: 전체 코드 + README
2. **블로그**: 3D Detection 개념 정리
3. **데모 영상**: 
   - KITTI 3D bbox 시각화
   - nuScenes BEV 맵
   - (옵션) Jetson 실시간 추론

### 면접 준비 질문
- Monocular vs Stereo vs LiDAR 3D Detection 차이?
- BEV의 장점은?
- Depth Estimation과 3D Detection의 차이?
- 실시간성을 위한 최적화 방법은?

---

**다음 단계**: Blender 시뮬레이션 에셋 제작 (병행)
