# Week 10: BEVFormer 실습 - Pretrained 모델로 추론 및 시각화


> **이번 주 목표**: BEVFormer pretrained 모델로 추론을 실행하고, BEV Feature를 시각화하며, NDS/mAP 성능 지표를 이해한다.
> **예상 시간**: 12-15시간
> **핵심 질문**: "BEVFormer의 추론 결과를 어떻게 해석하고, 성능을 어떤 지표로 평가하는가?"


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | NDS 구성 요소, mAP 매칭 기준, BEV Feature |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | NDS 직접 계산, BEV Feature 시각화 코드 실습 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | BEVFormer 추론, 결과 시각화, 성능 지표 분석 |


---


## 시작하기 전에


### Week 9에서 배운 것


**BEVFormer 구조 복습:**
```
Multi-view Images (6대 카메라)
     ↓
Backbone (ResNet-101) + FPN
     ↓
BEV Encoder (6 layers):
  - BEV Queries (200x200)
  - Temporal Self-Attention ← 이전 프레임 BEV
  - Spatial Cross-Attention ← Image Features
  - Feed Forward Network
     ↓
BEV Feature Map (200x200x256)
     ↓
Detection Head → [x, y, z, l, w, h, theta, vx, vy]
```


**이번 주 핵심:**
```
이론을 넘어 실제로:
  1. BEVFormer 코드를 실행해본다
  2. Pretrained 모델로 추론한다
  3. 3D Detection 결과를 시각화한다
  4. BEV Feature Map을 시각화한다
  5. NDS, mAP 지표를 이해하고 계산한다
```


---


## 핵심 개념 자세히 알아보기


### 1. BEVFormer 코드 구조


#### 1.1 저장소 구조


```
BEVFormer/
+-- projects/
| +-- mmdet3d_plugin/
| +-- bevformer/
| +-- detectors/ # BEVFormer 모델 정의
| | +-- bevformer.py
| +-- modules/ # 핵심 모듈
| | +-- encoder.py # BEV Encoder
| | +-- spatial_cross_attention.py
| | +-- temporal_self_attention.py
| | +-- decoder.py # Detection Decoder
| +-- dense_heads/ # Detection Head
+-- tools/
| +-- train.py
| +-- test.py
| +-- visualize.py
+-- configs/
| +-- bevformer/
| +-- bevformer_base.py # Base 모델 config
| +-- bevformer_small.py # Small 모델 config
| +-- bevformer_tiny.py # Tiny 모델 (테스트용)
+-- ckpts/ # Pretrained weights
```


#### 1.2 핵심 파일 분석


```python
# BEVFormer 모델 구조 (간소화)
class BEVFormer:
    def __init__(self):
        self.backbone = ResNet101() # 이미지 특징 추출
        self.neck = FPN() # 다중 스케일 특징
        self.encoder = BEVFormerEncoder( # BEV 생성
            bev_h=200, bev_w=200,
            num_layers=6,
            spatial_cross_attention=SCA(),
            temporal_self_attention=TSA(),
        )
        self.decoder = DetectionDecoder( # 객체 검출
            num_queries=900,
            num_layers=6,
        )


    def forward(self, images, prev_bev=None):
        # 1. 이미지 특징 추출
        features = self.neck(self.backbone(images))


        # 2. BEV Feature 생성
        bev_feature = self.encoder(features, prev_bev)


        # 3. 객체 검출
        detections = self.decoder(bev_feature)


        return detections, bev_feature
```


---


### 2. 환경 세팅 및 Pretrained 모델 준비


#### 2.1 환경 세팅


```bash
# 1. BEVFormer 저장소 클론
git clone https://github.com/fundamentalvision/BEVFormer.git
cd BEVFormer


# 2. Conda 환경 생성
conda create -n bevformer python=3.8 -y
conda activate bevformer


# 3. PyTorch 설치
pip install torch==1.13.1+cu117 torchvision==0.14.1+cu117 \
    --extra-index-url https://download.pytorch.org/whl/cu117


# 4. 의존성 설치
pip install openmim
mim install mmcv-full==1.7.1
mim install mmdet==2.28.2
mim install mmsegmentation==0.30.0


# 5. MMDetection3D 설치
git clone https://github.com/open-mmlab/mmdetection3d.git
cd mmdetection3d
git checkout v1.0.0rc6
pip install -e .
cd ..


# 6. BEVFormer 설치
pip install -e .
```


#### 2.2 Pretrained Weight 다운로드


```bash
# BEVFormer-Base (nuScenes)
mkdir -p ckpts
# Google Drive 또는 Baidu에서 다운로드 (README 참조)
# bevformer_r101_dcn_24ep.pth (~340MB)
```


#### 2.3 nuScenes Mini 데이터 준비


```bash
# nuScenes mini 다운로드 (약 4GB)
# https://www.nuscenes.org/download 에서 다운로드


# 데이터 구조
data/nuscenes/
+-- maps/
+-- samples/ # 센서 데이터
| +-- CAM_FRONT/
| +-- CAM_FRONT_LEFT/
| +-- CAM_FRONT_RIGHT/
| +-- CAM_BACK/
| +-- CAM_BACK_LEFT/
| +-- CAM_BACK_RIGHT/
+-- sweeps/ # 중간 프레임
+-- v1.0-mini/ # 메타데이터
| +-- sample.json
| +-- sample_data.json
| +-- ...
+-- nuscenes_infos_*.pkl # 전처리된 정보
```


---


### 3. 추론(Inference) 실행


#### 3.1 기본 추론


```bash
# BEVFormer 추론 실행
python tools/test.py \
    configs/bevformer/bevformer_base.py \
    ckpts/bevformer_r101_dcn_24ep.pth \
    --eval bbox \
    --show-dir results/bevformer_vis
```


#### 3.2 Python 스크립트로 추론


```python
"""
BEVFormer 추론 스크립트
Pretrained 모델로 nuScenes 샘플에 대해 추론하고 결과를 시각화한다.
"""
from mmdet3d.apis import init_model, inference_detector
from mmcv import Config
import numpy as np
import cv2
import matplotlib.pyplot as plt




def run_bevformer_inference():
    """BEVFormer 추론 실행"""
    # Config 및 Checkpoint
    config_file = 'configs/bevformer/bevformer_base.py'
    checkpoint = 'ckpts/bevformer_r101_dcn_24ep.pth'


    # 모델 초기화
    cfg = Config.fromfile(config_file)
    model = init_model(cfg, checkpoint, device='cuda:0')
    print(f"모델 로드 완료: {type(model).__name__}")


    # 추론
    # (실제로는 nuScenes 데이터 로더를 통해 입력)
    # 여기서는 테스트 파이프라인 사용
    result = inference_detector(model, data)


    return result
```


#### 3.3 추론 결과 형식


```
BEVFormer 출력 형식 (nuScenes):
+------------------------------------------+
| 각 검출 객체: |
| - center: [cx, cy, cz] # 3D 중심 |
| - size: [w, l, h] # 크기 |
| - rotation: quaternion # 회전 |
| - velocity: [vx, vy] # 속도 |
| - score: float # 신뢰도 |
| - class: int # 클래스 |
| |
| nuScenes 10 클래스: |
| car, truck, bus, trailer, |
| construction_vehicle, pedestrian, |
| motorcycle, bicycle, |
| barrier, traffic_cone |
+------------------------------------------+
```


---


### 4. BEV Feature 시각화


#### 4.1 BEV Feature Map이란?


```
BEV Feature Map: (200 x 200 x 256)
  - 200 x 200: BEV 공간의 그리드
  - 256: 각 셀의 특징 벡터 차원
  - 차원 축소 (PCA 또는 channel sum)로 시각화 가능


시각화 방법:
  1. Channel Sum: 모든 채널을 합산 → 히트맵
  2. PCA: 256차원 → 3차원 (RGB)으로 변환
  3. 특정 채널: 의미 있는 채널 선택
```


#### 4.2 시각화 코드


```python
"""
BEV Feature 시각화
BEVFormer의 출력 BEV Feature Map을 다양한 방법으로 시각화한다.
"""
import numpy as np
import matplotlib.pyplot as plt
from sklearn.decomposition import PCA




def visualize_bev_feature(bev_feature, save_path='bev_feature_viz.png'):
    """
    BEV Feature Map 시각화


    Args:
        bev_feature: (H, W, C) 또는 (C, H, W) BEV feature map
        save_path: 저장 경로
    """
    # shape 정리
    if bev_feature.shape[0] < bev_feature.shape[-1]:
        bev_feature = bev_feature.transpose(1, 2, 0) # (C,H,W) → (H,W,C)


    H, W, C = bev_feature.shape
    print(f"BEV Feature shape: ({H}, {W}, {C})")


    fig, axes = plt.subplots(1, 3, figsize=(18, 6))


    # 방법 1: Channel Sum
    channel_sum = np.abs(bev_feature).sum(axis=-1)
    channel_sum = (channel_sum - channel_sum.min()) / (channel_sum.max() - channel_sum.min())
    axes[0].imshow(channel_sum, cmap='hot', origin='lower')
    axes[0].set_title('Channel Sum (Activation Heatmap)')
    axes[0].set_xlabel('X')
    axes[0].set_ylabel('Y')


    # 방법 2: PCA → RGB
    flat = bev_feature.reshape(-1, C)
    pca = PCA(n_components=3)
    rgb = pca.fit_transform(flat)
    rgb = rgb.reshape(H, W, 3)
    rgb = (rgb - rgb.min()) / (rgb.max() - rgb.min())
    axes[1].imshow(rgb, origin='lower')
    axes[1].set_title(f'PCA (3 components, 분산 비율: {pca.explained_variance_ratio_.sum():.2f})')
    axes[1].set_xlabel('X')


    # 방법 3: Channel Max
    channel_max = np.max(bev_feature, axis=-1)
    channel_max = (channel_max - channel_max.min()) / (channel_max.max() - channel_max.min())
    axes[2].imshow(channel_max, cmap='viridis', origin='lower')
    axes[2].set_title('Channel Max')
    axes[2].set_xlabel('X')


    plt.suptitle('BEV Feature Map 시각화', fontsize=14)
    plt.tight_layout()
    plt.savefig(save_path, dpi=150)
    plt.show()
    print(f"저장 완료: {save_path}")




# 시뮬레이션 데이터로 테스트
def demo_bev_feature_visualization():
    """시뮬레이션 BEV Feature로 시각화 데모"""
    H, W, C = 200, 200, 256
    # 도로 구조를 시뮬레이션
    bev_feature = np.random.randn(H, W, C) * 0.1


    # 도로 영역 (중앙 세로선)
    bev_feature[80:120, :, :] += np.random.randn(40, W, C) * 0.5


    # 객체 시뮬레이션 (높은 활성화)
    bev_feature[100:105, 90:95, :] += 2.0 # 차량 1
    bev_feature[60:65, 110:115, :] += 1.5 # 차량 2
    bev_feature[140:143, 95:98, :] += 1.2 # 보행자


    visualize_bev_feature(bev_feature)




demo_bev_feature_visualization()
```


---


### 5. 성능 지표: NDS와 mAP


#### 5.1 nuScenes Detection Score (NDS)


```
NDS = 1/10 * (5 * mAP + Σ mTP_i)


mAP: mean Average Precision
mTP_i: 5가지 True Positive 메트릭


NDS 구성 요소:
+-------------------------------------------+
| 메트릭 | 무엇을 평가하는가? |
+-------------------------------------------+
| mAP | 검출 정확도 (위치 + 클래스) |
| mATE | Translation Error (위치 오차) |
| mASE | Scale Error (크기 오차) |
| mAOE | Orientation Error (방향 오차) |
| mAVE | Velocity Error (속도 오차) |
| mAAE | Attribute Error (속성 오차) |
+-------------------------------------------+


NDS = 1/10 * (5*mAP + (1-mATE) + (1-mASE) + (1-mAOE)
              + (1-mAVE) + (1-mAAE))


→ 모든 값이 낮을수록 좋은 것 (mAP만 높을수록 좋음)
→ NDS는 종합 점수로, 0~1 범위
```


#### 5.2 mAP (mean Average Precision)


```
nuScenes mAP 계산:


1. BEV 중심 거리 기준 매칭 (3D IoU 대신)
   - 거리 threshold: [0.5, 1.0, 2.0, 4.0] m


2. 각 클래스별 AP 계산:
   AP_class = average(AP_0.5, AP_1.0, AP_2.0, AP_4.0)


3. 모든 클래스 평균:
   mAP = mean(AP_car, AP_truck, ..., AP_traffic_cone)


KITTI와의 차이:
+------------------------------------------+
| | KITTI | nuScenes |
+------------------------------------------+
| 매칭 기준 | 3D IoU | BEV 중심 거리 |
| 클래스 | 3개 | 10개 |
| 카메라 | 1대 (전방) | 6대 (360도) |
| 종합 지표 | AP3D | NDS |
+------------------------------------------+
```


#### 5.3 BEVFormer 성능 분석


```
BEVFormer 성능 (nuScenes val set):


+--------------------------------------------+
| 모델 | NDS | mAP | FPS |
+--------------------------------------------+
| BEVFormer-Tiny | 0.354 | 0.252 | 4.3 |
| BEVFormer-Small | 0.478 | 0.370 | 2.3 |
| BEVFormer-Base | 0.517 | 0.416 | 1.7 |
+--------------------------------------------+


클래스별 AP (BEVFormer-Base):
+----------------------------------------+
| 클래스 | AP |
+----------------------------------------+
| Car | 0.636 |
| Truck | 0.401 |
| Bus | 0.457 |
| Pedestrian | 0.420 |
| Motorcycle | 0.393 |
| Bicycle | 0.226 |
| Traffic Cone | 0.505 |
| Barrier | 0.530 |
+----------------------------------------+


관찰:
- Car가 가장 높은 AP (크기가 크고 데이터 많음)
- Bicycle이 가장 낮은 AP (크기 작고 드뭄)
- NDS는 mAP보다 높음 (위치/크기/방향 정확도 반영)
```


---


### 6. 결과 시각화 및 분석


#### 6.1 3D 검출 결과를 이미지에 투영


```python
"""
3D 검출 결과를 6대 카메라 이미지에 투영하여 시각화
"""
def visualize_detection_on_images(detections, images, cam_params):
    """
    검출 결과를 각 카메라 이미지에 투영


    Args:
        detections: 3D bbox 리스트
        images: 6대 카메라 이미지
        cam_params: 카메라 내부/외부 파라미터
    """
    cam_names = ['FRONT', 'FRONT_LEFT', 'FRONT_RIGHT',
                 'BACK', 'BACK_LEFT', 'BACK_RIGHT']


    fig, axes = plt.subplots(2, 3, figsize=(18, 10))


    for idx, (cam_name, ax) in enumerate(zip(cam_names, axes.flat)):
        img = images[cam_name]
        K = cam_params[cam_name]['intrinsic']
        extrinsic = cam_params[cam_name]['extrinsic']


        ax.imshow(img)
        ax.set_title(cam_name)
        ax.axis('off')


        for det in detections:
            # 3D bbox corners 계산
            corners_3d = compute_3d_corners(det)


            # 카메라 좌표로 변환 및 투영
            corners_cam = extrinsic @ corners_3d
            if corners_cam[2].min() > 0: # 카메라 앞에 있는 경우만
                corners_2d = K @ corners_cam[:3]
                corners_2d = corners_2d[:2] / corners_2d[2:]
                draw_bbox_on_axis(ax, corners_2d.T, det['class'])


    plt.suptitle('BEVFormer 3D Detection 결과', fontsize=14)
    plt.tight_layout()
    plt.savefig('detection_on_images.png', dpi=150)
    plt.show()
```


#### 6.2 BEV 검출 결과 시각화


```python
def visualize_detection_bev(detections, bev_range=50):
    """
    검출 결과를 BEV(위에서 본 시점)로 시각화


    Args:
        detections: 3D bbox 리스트
        bev_range: BEV 시각화 범위 (m)
    """
    fig, ax = plt.subplots(figsize=(10, 10))


    color_map = {
        'car': 'green', 'truck': 'blue', 'bus': 'cyan',
        'pedestrian': 'red', 'motorcycle': 'orange',
        'bicycle': 'yellow', 'barrier': 'gray',
        'traffic_cone': 'magenta',
    }


    for det in detections:
        cx, cy = det['center'][:2]
        w, l = det['size'][:2]
        theta = det['rotation']


        # BEV 직사각형
        corners = get_bev_corners(cx, cy, w, l, theta)
        color = color_map.get(det['class'], 'white')


        polygon = plt.Polygon(corners, fill=True, alpha=0.3,
                             facecolor=color, edgecolor=color, linewidth=2)
        ax.add_patch(polygon)


        # 속도 화살표
        if 'velocity' in det:
            vx, vy = det['velocity']
            ax.arrow(cx, cy, vx, vy, head_width=0.3,
                    head_length=0.2, fc=color, ec=color)


        ax.text(cx, cy, f"{det['class']}\n{det['score']:.2f}",
               fontsize=7, ha='center', color=color)


    # ego 차량
    ax.plot(0, 0, 'k^', markersize=15, label='Ego')


    ax.set_xlim(-bev_range, bev_range)
    ax.set_ylim(-bev_range, bev_range)
    ax.set_xlabel('X [m]')
    ax.set_ylabel('Y [m]')
    ax.set_title('BEVFormer Detection (BEV)')
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.3)
    ax.legend()


    plt.tight_layout()
    plt.savefig('detection_bev.png', dpi=150)
    plt.show()
```


---


### 7. 오류 분석 (Error Analysis)


#### 7.1 자주 발생하는 오류 패턴


```
패턴 1: 먼 거리 객체 놓침
  - 원인: 작은 이미지 영역, 해상도 부족
  - BEV에서 확인: 30m 이상에서 검출률 급감


패턴 2: 가려진 객체
  - 원인: Spatial Cross-Attention이 가려진 영역 참조
  - Temporal이 도움: 이전 프레임에서 보인 객체


패턴 3: 크기 추정 오류
  - 원인: Scale Ambiguity
  - mASE 지표로 확인


패턴 4: 속도 추정 오류
  - 원인: Temporal 정보 부족 (첫 프레임)
  - mAVE 지표로 확인
```


#### 7.2 거리별 성능 분석


```
거리별 AP (BEVFormer-Base, Car):
+--------------------------------------+
| 거리 범위 | AP | 비고 |
+--------------------------------------+
| 0-10m | 0.78 | 매우 좋음 |
| 10-20m | 0.72 | 좋음 |
| 20-30m | 0.55 | 보통 |
| 30-40m | 0.35 | 낮음 |
| 40-50m | 0.18 | 매우 낮음 |
+--------------------------------------+


→ 거리가 멀어질수록 성능 급감
→ 이미지에서 작아지므로 Feature 추출이 어려움
→ BEV 해상도 한계 (0.5m/cell에서 먼 거리는 몇 pixel)
```


---


## 꼭 이해해야 할 핵심 개념


### 1. NDS가 mAP보다 중요한 이유


```
mAP만으로는 부족:
  - 검출은 했지만 크기가 완전히 틀릴 수 있음
  - 방향(orientation)이 반대일 수 있음
  - 속도 추정이 부정확할 수 있음


NDS는 이 모든 것을 종합:
  "검출도 잘하고, 위치/크기/방향/속도도 정확한가?"


자율주행에서는 NDS가 더 실용적:
  → 검출만 하면 안 되고, 정확한 속성까지 필요
  → 경로 계획에 3D 크기와 속도가 직접 사용됨
```


### 2. BEV Feature의 의미


```
BEV Feature (200x200x256):
  각 셀이 담고 있는 정보:
  - 해당 위치에 어떤 객체가 있는가
  - 객체의 유형 (차량, 보행자 등)
  - 도로/비도로 구분 정보
  - Temporal로 누적된 이전 프레임 정보


왜 중요한가:
  - Detection Head의 입력
  - Segmentation 등 다른 Task에도 활용 가능
  - 경로 계획(Planning)의 입력으로 직접 사용 가능
```


### 3. 추론 속도와 실시간성


```
BEVFormer 추론 속도:
  - Base: ~1.7 FPS (RTX 3090)
  - Small: ~2.3 FPS
  - Tiny: ~4.3 FPS


실시간 기준:
  - 자율주행: 최소 10 FPS 권장
  - BEVFormer는 아직 실시간에 부족


개선 방향:
  - BEVFormer v2: 속도 개선
  - StreamPETR: Streaming 방식으로 가속
  - BEVPoolv2: Efficient BEV 생성
```


---


## 자체 점검 - 이해했는지 확인!


**Q1. NDS에서 mAP의 가중치가 5배인 이유는?**


> NDS 공식에서 mAP에 5를 곱하는 이유는 mAP가 가장 근본적인 검출 능력을 반영하기 때문이다. 5가지 TP 메트릭(ATE, ASE, AOE, AVE, AAE)은 검출된 객체에 대한 세부 정확도이므로, 검출 자체의 중요도를 균형있게 반영하기 위해 mAP 가중치를 5로 설정하여 전체 10개 항목 중 절반을 차지하게 한다.


**Q2. nuScenes mAP가 KITTI AP3D와 다른 매칭 기준을 사용하는 이유는?**


> KITTI는 3D IoU를 매칭 기준으로 사용하지만, nuScenes는 BEV 중심 거리를 사용한다. nuScenes는 360도 6대 카메라를 사용하여 검출 범위가 넓고, 먼 거리의 객체는 3D IoU 계산이 불안정할 수 있다. BEV 중심 거리 기반 매칭은 더 직관적이고, 다양한 거리 threshold(0.5m, 1m, 2m, 4m)로 세밀한 평가가 가능하다.


**Q3. BEV Feature를 시각화할 때 PCA를 사용하는 이유는?**


> BEV Feature는 256차원의 고차원 벡터이므로 직접 시각화가 불가능하다. PCA는 분산이 가장 큰 3개의 주성분을 추출하여 RGB로 매핑함으로써, 유사한 특징을 가진 영역을 비슷한 색상으로 표현한다. 이를 통해 도로 영역, 객체 영역, 빈 공간 등이 자연스럽게 구분되는 것을 확인할 수 있다.


**Q4. BEVFormer 추론 속도가 실시간에 못 미치는 주된 원인은?**


> 주된 병목은 (1) Backbone으로 ResNet-101을 사용하여 6대 카메라 이미지 모두에 대해 Feature를 추출해야 하고, (2) 6개 BEV Encoder Layer에서 40,000개(200x200)의 BEV Query에 대해 Spatial Cross-Attention을 수행하며, (3) Temporal Self-Attention과 Detection Decoder의 추가 연산이 필요하기 때문이다. 특히 Deformable Attention도 Reference Point 수가 많아 연산량이 크다.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제


1. **환경 세팅**: BEVFormer 코드 클론, 의존성 설치, pretrained weight 다운로드
2. **추론 실행**: nuScenes mini 데이터셋에서 BEVFormer 추론
3. **결과 시각화**: 3D bbox를 6대 카메라 이미지에 투영
4. **BEV Feature 시각화**: Channel Sum, PCA 방법으로 BEV Feature 확인
5. **성능 지표 분석**: NDS, mAP 결과 해석, 클래스별 AP 비교
6. **오류 분석**: 거리별/클래스별 성능 차이 분석


자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고


### 다음 주 준비


- 추론 결과 이미지 캡처 (포트폴리오용)
- GitHub 저장소에 코드 정리 시작
- README 초안 작성


---


## 이번 주 핵심 요약


1. **BEVFormer 추론**은 6대 카메라 이미지를 입력으로 받아, BEV Feature Map을 생성하고, 3D Bounding Box + 속도를 출력한다.
2. **NDS(nuScenes Detection Score)**는 mAP와 5가지 TP 메트릭을 종합한 점수로, mAP만으로는 부족한 3D 속성 정확도를 평가한다.
3. **BEV Feature 시각화**를 통해 모델이 BEV 공간에서 학습한 표현을 직관적으로 확인할 수 있으며, PCA나 Channel Sum 방법을 사용한다.
4. **거리별 성능 분석**을 통해 BEVFormer의 한계(먼 거리, 작은 객체)를 파악하고, 개선 방향을 이해한다.
5. **추론 속도**는 아직 실시간에 못 미치지만 (Base: 1.7 FPS), 후속 연구들이 활발히 진행 중이다.


---


이전: [Week 9 - BEVFormer 이해](../week9/README.md)


다음: [Week 11 - 코드 및 문서 정리](../week11/README.md)
