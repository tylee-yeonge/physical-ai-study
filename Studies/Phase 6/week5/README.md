# Week 5: MMDetection3D 실습 - KITTI 3D Detection 프레임워크

> 🎯 **이번 주 목표**: MMDetection3D 환경을 세팅하고, KITTI 데이터셋에서 FCOS3D 모델을 학습시켜 3D 객체 검출 결과를 시각화하고 평가한다.
> ⏰ **예상 시간**: 12-15시간
> 💡 **핵심 질문**: "카메라 한 장으로 3D 바운딩 박스를 예측하는 모델을 직접 학습시키고 평가할 수 있는가?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | openmim 역할, Config 시스템, AP3D 평가 기준 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | FCOS3D Multi-task Learning, Config 분석 코드 실습 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | MMDetection3D 환경 세팅, FCOS3D 학습 및 추론 |

---

## 🌟 시작하기 전에

### Week 3-4에서 배운 것

**KITTI 데이터셋 구조:**
```
KITTI/
├── training/
│   ├── image_2/      # 왼쪽 카메라 이미지
│   ├── calib/        # 캘리브레이션 파라미터
│   └── label_2/      # 3D bbox 레이블 [h,w,l,x,y,z,ry]
└── testing/
```

**Monocular 3D Detection의 핵심:**
```
2D Detection: [x, y, w, h]           → 이미지 내 위치
3D Detection: [x, y, z, l, w, h, θ]  → 3D 공간 내 위치 + 크기 + 회전
```

**하지만 궁금하지 않았나요?**
```
❓ 실제로 모델을 학습시키려면 어떤 프레임워크를 써야 하지?
❓ MMDetection3D의 Config 시스템은 어떻게 동작하지?
❓ 학습 결과를 어떻게 평가하고 시각화하지?
```

**이번 주에 직접 해봅니다!**

> 💼 **포트폴리오 관점**: MMDetection3D는 자율주행/로봇 분야에서 표준 프레임워크입니다. 이 프레임워크 사용 경험은 면접에서 큰 강점이 됩니다.

---

## 📚 핵심 개념 자세히 알아보기

### 1. MMDetection3D 프레임워크 소개

#### 1.1 왜 MMDetection3D인가?

MMDetection3D는 OpenMMLab에서 개발한 **3D 객체 검출 오픈소스 프레임워크**입니다.

| 특징 | 설명 |
|------|------|
| **모듈화** | Backbone, Neck, Head를 자유롭게 조합 |
| **Config 기반** | Python config 파일로 실험 관리 |
| **다양한 모델** | FCOS3D, SMOKE, BEVFormer 등 20+ 모델 지원 |
| **다양한 데이터셋** | KITTI, nuScenes, Waymo 등 |
| **평가 도구** | AP3D, NDS 등 표준 메트릭 내장 |

#### 1.2 MMDetection3D 아키텍처

```
MMDetection3D 구조:
┌─────────────────────────────────────────┐
│                Config (.py)              │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ │
│  │ Backbone │→│   Neck   │→│   Head   │ │
│  │ (ResNet) │ │  (FPN)   │ │(FCOS3D)  │ │
│  └──────────┘ └──────────┘ └──────────┘ │
│       ↑                          ↓       │
│  ┌──────────┐            ┌──────────┐   │
│  │ Dataset  │            │  Loss    │   │
│  │ (KITTI)  │            │(Focal+L1)│   │
│  └──────────┘            └──────────┘   │
└─────────────────────────────────────────┘
```

#### 1.3 OpenMMLab 생태계

```
OpenMMLab 생태계:
├── mmcv          # 기본 유틸리티 (Config, Registry, Runner)
├── mmdet         # 2D Detection (YOLO, Faster R-CNN 등)
├── mmdet3d       # 3D Detection (FCOS3D, BEVFormer 등)
├── mmseg         # Segmentation
└── mmengine      # 학습 엔진 (v2.0+)
```

---

### 2. 환경 세팅

#### 2.1 Conda 환경 생성

```bash
# 1. Conda 환경 생성
conda create -n mmdet3d python=3.8 -y
conda activate mmdet3d

# 2. PyTorch 설치 (CUDA 11.8 기준)
pip install torch==1.13.1+cu117 torchvision==0.14.1+cu117 \
    --extra-index-url https://download.pytorch.org/whl/cu117

# 3. OpenMMLab 설치 (openmim 사용)
pip install openmim
mim install mmcv-full==1.7.1
mim install mmdet==2.28.2

# 4. MMDetection3D 설치 (소스 빌드 권장)
git clone https://github.com/open-mmlab/mmdetection3d.git
cd mmdetection3d
git checkout v1.1.1  # 안정 버전
pip install -e .

# 5. 설치 확인
python -c "import mmdet3d; print(mmdet3d.__version__)"
```

#### 2.2 버전 호환성 (중요!)

```
⚠️ 버전 매칭이 매우 중요합니다!

Python:    3.8
PyTorch:   1.13.1
CUDA:      11.7
mmcv-full: 1.7.1
mmdet:     2.28.2
mmdet3d:   1.1.1

→ 버전이 안 맞으면 import 에러 발생!
→ openmim이 호환 버전을 자동으로 찾아줌
```

#### 2.3 설치 확인 스크립트

```python
# check_install.py
import torch
import mmcv
import mmdet
import mmdet3d

print(f"PyTorch:  {torch.__version__}")
print(f"CUDA:     {torch.version.cuda}")
print(f"mmcv:     {mmcv.__version__}")
print(f"mmdet:    {mmdet.__version__}")
print(f"mmdet3d:  {mmdet3d.__version__}")
print(f"GPU:      {torch.cuda.get_device_name(0)}")
print(f"GPU OK:   {torch.cuda.is_available()}")
```

---

### 3. FCOS3D 모델 이해

#### 3.1 FCOS3D란?

FCOS3D는 **Fully Convolutional One-Stage 3D Detection** 모델입니다.

```
FCOS (2D)                    FCOS3D (3D)
──────                       ─────────
이미지 → 2D bbox             이미지 → 3D bbox
[x, y, w, h, cls]           [x, y, z, l, w, h, θ, cls]
                              ↑ Depth 추정 추가!
```

#### 3.2 FCOS3D 구조

```
입력 이미지 (1242 x 375)
    ↓
ResNet-101 (Backbone)
    ↓
FPN (Feature Pyramid Network)
    ↓ Multi-scale features
┌──────────────────────────────┐
│         FCOS3D Head          │
├──────────┬───────────────────┤
│ 분류 분기 │    회귀 분기      │
│  (cls)   │ offset(2D)       │
│          │ depth(z)          │
│          │ size(l,w,h)       │
│          │ rotation(sin,cos) │
│          │ velocity(vx,vy)   │
└──────────┴───────────────────┘
```

#### 3.3 핵심: Depth 예측

```
Monocular 3D의 최대 난관: Depth(깊이) 추정

방법 1: Direct Regression
  - 네트워크가 직접 z값을 회귀
  - 학습 데이터 분포에 의존

방법 2: Keypoint 기반
  - 2D → 3D 기하학적 관계 이용
  - P2 행렬(캘리브레이션) 필요

FCOS3D: Direct Regression + 기하학적 보조
```

---

### 4. KITTI 데이터 준비

#### 4.1 MMDetection3D용 데이터 변환

```bash
# KITTI 데이터를 MMDet3D 포맷으로 변환
cd mmdetection3d

python tools/create_data.py kitti \
    --root-path ./data/kitti \
    --out-dir ./data/kitti \
    --extra-tag kitti
```

#### 4.2 변환 후 구조

```
data/kitti/
├── training/
│   ├── image_2/
│   ├── calib/
│   └── label_2/
├── testing/
├── kitti_infos_train.pkl      # ← 생성됨
├── kitti_infos_val.pkl        # ← 생성됨
├── kitti_infos_trainval.pkl   # ← 생성됨
└── kitti_infos_test.pkl       # ← 생성됨
```

#### 4.3 데이터 포맷 확인

```python
import pickle

with open('data/kitti/kitti_infos_train.pkl', 'rb') as f:
    infos = pickle.load(f)

print(f"학습 샘플 수: {len(infos)}")
print(f"첫 번째 샘플 키: {infos[0].keys()}")

# 출력 예:
# 학습 샘플 수: 3712
# 키: dict_keys(['image', 'point_cloud', 'calib', 'annos'])
```

---

### 5. Config 파일 이해

#### 5.1 Config 시스템

```python
# configs/fcos3d/fcos3d_r101_caffe_fpn_gn-head_dcn_2x8_1x_kitti-mono3d.py

_base_ = [
    '../_base_/datasets/kitti-mono3d.py',    # 데이터 설정
    '../_base_/models/fcos3d.py',            # 모델 설정
    '../_base_/schedules/mmdet_schedule_1x.py',  # 학습 스케줄
    '../_base_/default_runtime.py',          # 런타임 설정
]

# 모델 수정
model = dict(
    backbone=dict(
        type='ResNet',
        depth=101,
        dcn=dict(type='DCNv2', deform_groups=1, fallback_on_stride=False),
    ),
    bbox_head=dict(
        type='FCOSMono3DHead',
        num_classes=3,  # KITTI: Car, Pedestrian, Cyclist
    ),
)

# 데이터 경로
data_root = 'data/kitti/'
data = dict(
    samples_per_gpu=2,    # 배치 크기 (GPU당)
    workers_per_gpu=2,
)
```

#### 5.2 주요 Config 요소

```
Config 구조:
├── model          # 모델 아키텍처 (backbone, neck, head)
├── dataset_type   # 데이터셋 종류 (KittiMonoDataset)
├── data_root      # 데이터 경로
├── data           # train/val/test 데이터 설정
├── optimizer      # 옵티마이저 (SGD, AdamW)
├── lr_config      # 학습률 스케줄러
├── runner         # 학습 반복 설정 (epoch, max_epochs)
└── evaluation     # 평가 주기 및 메트릭
```

---

### 6. 학습 실행

#### 6.1 학습 명령어

```bash
# 단일 GPU 학습
python tools/train.py \
    configs/fcos3d/fcos3d_r101_caffe_fpn_gn-head_dcn_2x8_1x_kitti-mono3d.py \
    --work-dir work_dirs/fcos3d_kitti \
    --gpu-ids 0

# 다중 GPU 학습 (2 GPU)
bash tools/dist_train.sh \
    configs/fcos3d/fcos3d_r101_caffe_fpn_gn-head_dcn_2x8_1x_kitti-mono3d.py \
    2 \
    --work-dir work_dirs/fcos3d_kitti
```

#### 6.2 학습 모니터링

```bash
# 로그 확인
tail -f work_dirs/fcos3d_kitti/20250101_120000.log

# TensorBoard (설정 시)
tensorboard --logdir work_dirs/fcos3d_kitti/tf_logs
```

#### 6.3 학습 시간 예상

```
GPU: RTX 3090 (24GB)
배치: 2/GPU
학습 시간: ~12시간 (12 epoch)

GPU: RTX 4090 (24GB)
배치: 4/GPU
학습 시간: ~8시간

⚠️ Pretrained weights 사용 시 훨씬 빠름!
```

---

### 7. Inference & 시각화

#### 7.1 추론 실행

```bash
# 테스트 셋에서 추론
python tools/test.py \
    configs/fcos3d/fcos3d_r101_caffe_fpn_gn-head_dcn_2x8_1x_kitti-mono3d.py \
    work_dirs/fcos3d_kitti/latest.pth \
    --eval mAP \
    --show-dir work_dirs/fcos3d_kitti/show_results
```

#### 7.2 3D Bbox 시각화 코드

```python
import numpy as np
import cv2
from mmdet3d.apis import init_model, inference_mono_3d_detector

# 모델 로드
config_file = 'configs/fcos3d/fcos3d_r101_kitti.py'
checkpoint_file = 'work_dirs/fcos3d_kitti/latest.pth'
model = init_model(config_file, checkpoint_file, device='cuda:0')

# 추론
img = 'data/kitti/training/image_2/000000.png'
ann_file = 'data/kitti/training/calib/000000.txt'
result = inference_mono_3d_detector(model, img, ann_file)

# 시각화
def draw_3d_bbox(img, corners_2d, color=(0, 255, 0)):
    """3D bbox의 12개 edge를 이미지에 그리기"""
    edges = [
        [0,1],[1,2],[2,3],[3,0],  # 아래 면
        [4,5],[5,6],[6,7],[7,4],  # 위 면
        [0,4],[1,5],[2,6],[3,7],  # 수직 edge
    ]
    for i, j in edges:
        pt1 = tuple(corners_2d[i].astype(int))
        pt2 = tuple(corners_2d[j].astype(int))
        cv2.line(img, pt1, pt2, color, 2)
    return img
```

---

### 8. 평가 지표: AP3D

#### 8.1 AP3D (Average Precision 3D)

```
AP3D = 3D 공간에서의 정밀도-재현율 곡선 아래 면적

평가 기준:
┌──────────────────────────────────────────┐
│  난이도     │ 기준                        │
├──────────────────────────────────────────┤
│  Easy      │ 크고, 안 가려진 객체         │
│  Moderate  │ 중간 크기, 부분 가려짐       │
│  Hard      │ 작고, 많이 가려진 객체        │
└──────────────────────────────────────────┘

IoU Threshold:
- Car: 0.7 (매우 엄격)
- Pedestrian: 0.5
- Cyclist: 0.5
```

#### 8.2 AP3D vs AP2D

```
AP2D: 2D bbox IoU로 평가
  → [x1, y1, x2, y2] 간의 겹침

AP3D: 3D bbox IoU로 평가
  → [x, y, z, l, w, h, θ] 간의 3D 겹침
  → Depth가 틀리면 IoU가 급격히 낮아짐!

예시:
  2D IoU = 0.85 (거의 정확)
  3D IoU = 0.15 (Depth가 2m 벗어나면 실패)

→ 3D Detection이 훨씬 어려운 이유!
```

#### 8.3 Monocular SOTA 성능 참고

```
KITTI Car Moderate AP3D (2024 기준):
┌────────────────────────────────────┐
│ 방법            │ AP3D (Moderate) │
├────────────────────────────────────┤
│ FCOS3D          │ ~12%            │
│ MonoDLE         │ ~17%            │
│ MonoFlex        │ ~19%            │
│ GUPNet          │ ~22%            │
│ MonoDETR (SOTA) │ ~25%            │
└────────────────────────────────────┘

→ Monocular 특성상 AP3D 15% 이상이면 성공적!
→ LiDAR 기반은 80%+ (센서 차이)
```

---

## 💡 꼭 이해해야 할 핵심 개념

### 1. MMDetection3D Config 시스템

```
_base_: 기본 설정 상속
  → 하위 config에서 dict()로 오버라이드
  → 실험 관리에 매우 효율적

예: 학습률만 바꾸기
  optimizer = dict(lr=0.001)  # 나머지는 _base_에서 상속
```

### 2. 3D Detection의 Multi-task Learning

```
FCOS3D는 하나의 네트워크에서 동시에 예측:
┌─────────────────────────────┐
│ Task          │ Output      │
├─────────────────────────────┤
│ Classification│ cls score   │
│ 2D offset     │ (dx, dy)    │
│ Depth         │ z           │
│ Size          │ (l, w, h)   │
│ Rotation      │ (sin θ, cos θ) │
└─────────────────────────────┘

→ 각 Task의 Loss를 합산하여 학습
→ Loss 가중치 조절이 성능에 큰 영향
```

### 3. IoU 3D 계산의 어려움

```
2D IoU: 사각형 교집합 → 간단
3D IoU: 회전된 직육면체 교집합 → 복잡!

KITTI 평가:
  Car IoU ≥ 0.7 → 매우 엄격
  → Depth 1m 오차 → IoU 급감
  → Rotation 10도 오차 → IoU 감소
```

---

## 🔍 자체 점검 - 이해했는지 확인!

**Q1. MMDetection3D에서 openmim의 역할은 무엇인가?**
> openmim은 OpenMMLab 패키지 간의 버전 호환성을 자동으로 관리해주는 패키지 매니저입니다. `mim install mmcv-full`처럼 사용하면 현재 설치된 PyTorch와 호환되는 mmcv 버전을 자동으로 찾아 설치합니다.

**Q2. KITTI AP3D에서 Easy/Moderate/Hard의 차이는 무엇인가?**
> Easy는 크고 가려지지 않은 객체, Moderate는 부분적으로 가려진 중간 크기 객체, Hard는 많이 가려지거나 잘린 작은 객체를 의미합니다. Moderate가 일반적인 벤치마크 기준이며, 대부분의 논문에서 Moderate AP3D를 주요 지표로 보고합니다.

**Q3. Car의 IoU threshold가 0.7이고 Pedestrian이 0.5인 이유는?**
> Car는 크기가 크기 때문에 3D IoU 0.7을 달성하기 상대적으로 쉽고, Pedestrian은 크기가 작아서 약간의 위치 오차도 3D IoU를 크게 떨어뜨리기 때문입니다. 작은 객체에 더 느슨한 기준을 적용하여 공정한 평가를 합니다.

**Q4. Monocular 3D Detection에서 Depth 추정이 어려운 이유는?**
> 단안 카메라에서는 깊이 정보가 근본적으로 모호합니다(scale ambiguity). 동일한 2D 투영이 다른 깊이에서도 발생할 수 있으며, 학습 데이터의 깊이 분포에 의존해야 합니다. LiDAR는 직접 거리를 측정하지만, 카메라는 시각적 단서(원근법, 크기 비교 등)로 간접 추정해야 하므로 본질적으로 어렵습니다.

---

## 📝 이번 주 실습 & 다음 주 준비

### 이번 주 실습 과제

1. **MMDetection3D 환경 세팅**: conda 환경 + openmim 설치 + 버전 확인
2. **KITTI 데이터 변환**: `create_data.py`로 pkl 파일 생성
3. **FCOS3D Config 분석**: config 파일 구조 이해 및 수정
4. **모델 학습**: KITTI에서 FCOS3D 학습 (또는 pretrained 사용)
5. **추론 및 시각화**: 3D bbox를 이미지에 투영하여 시각화
6. **AP3D 평가**: Easy/Moderate/Hard 결과 확인

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

### 다음 주 준비

- 학습 로그(loss 커브) 캡처해두기
- 추론 결과 이미지 저장해두기
- 오류 분석을 위해 잘 맞는/못 맞는 케이스 모아두기

---

## 🎯 이번 주 핵심 요약

1. **MMDetection3D**는 OpenMMLab의 3D 검출 프레임워크로, Config 기반의 모듈화된 구조를 제공한다.
2. **openmim**으로 설치하면 버전 호환성 문제를 줄일 수 있다. 버전 매칭은 매우 중요하다.
3. **FCOS3D**는 Anchor-free 방식의 Monocular 3D Detection 모델로, Multi-task Learning으로 위치/크기/회전/깊이를 동시에 예측한다.
4. **AP3D**는 3D IoU 기반 평가 지표이며, Monocular 방식은 Depth 한계로 LiDAR보다 성능이 낮다(KITTI Car Moderate ~15-25%).
5. **포트폴리오**에 MMDetection3D 사용 경험과 KITTI 결과를 포함하면 자율주행/로봇 면접에서 강력한 어필이 된다.

---

✅ 이전: [Week 4 - Monocular 3D Detection 모델](../week4/README.md)

다음: [Week 6 - 성능 분석 및 개선](../week6/README.md)
