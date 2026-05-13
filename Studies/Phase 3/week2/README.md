# Week 2: 컴퓨터 비전용 라이브러리 (Section 5.1)


> **이번 주 목표**: Albumentations, W&B, torchvision/timm 라이브러리를 활용한 CV 파이프라인 구축
> **예상 시간**: 12시간
> **핵심 질문**: "효율적인 CV 실험을 위한 도구 세트는 무엇인가?"


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | Albumentations, W&B, torchvision 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | Augmentation 파이프라인 코드 작성 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | CV 라이브러리 파이프라인 구축 |


---


## 시작하기 전에


### Week 1과의 연결


| Week 1 | → | Week 2 |
|---------|---|---------|
| PyTorch 기본 연산 | → | CV 전문 라이브러리 활용 |
| 단순 CNN 학습 | → | Pretrained 모델 + Augmentation |
| 수동 실험 기록 | → | W&B 자동 실험 관리 |


---


## 핵심 개념


### 1. Albumentations (Data Augmentation)


```python
import albumentations as A
from albumentations.pytorch import ToTensorV2


# 기본 변환 파이프라인
transform = A.Compose([
    A.Resize(640, 640),
    A.HorizontalFlip(p=0.5),
    A.RandomBrightnessContrast(p=0.3),
    A.Normalize(mean=[0.485, 0.456, 0.406],
                std=[0.229, 0.224, 0.225]),
    ToTensorV2()
])


# Detection용 (BBox 포함)
det_transform = A.Compose([
    A.Resize(640, 640),
    A.HorizontalFlip(p=0.5),
    A.RandomBrightnessContrast(p=0.3),
    A.Normalize(),
    ToTensorV2()
], bbox_params=A.BboxParams(
    format='pascal_voc', # [x_min, y_min, x_max, y_max]
    label_fields=['labels']
))
```


**장점**: OpenCV 기반 (빠름), BBox 자동 변환, 다양한 변환 내장


### 2. Weights & Biases (실험 관리)


```python
import wandb


# 초기화
wandb.init(project="yolo-training", name="exp-001")


# 학습 중 로깅
for epoch in range(100):
    wandb.log({
        "train_loss": train_loss,
        "val_loss": val_loss,
        "val_mAP": val_map
    })


# 이미지 로깅
wandb.log({"predictions": [
    wandb.Image(img, caption=f"pred: {cls}")
]})
```


### 3. torchvision vs timm


```
torchvision:
  → PyTorch 공식, Detection/Segmentation API 포함
  → torchvision.models.detection.fasterrcnn_resnet50_fpn()


timm:
  → 최신 모델 700+ 종류 (EfficientNet, ViT, ConvNeXt 등)
  → timm.create_model('efficientnet_b0', pretrained=True)
  → Feature extractor로 활용 가능
```


### 4. Pretrained 모델 활용


```python
import timm


# Backbone으로 사용
model = timm.create_model(
    'efficientnet_b0',
    pretrained=True,
    features_only=True, # Feature pyramid 추출
    out_indices=[2, 3, 4] # Multi-scale features
)
```


---


## [?] 자체 점검 퀴즈


1. **Albumentations의 bbox_params 역할은?**
   - Augmentation 시 BBox도 동일하게 변환
   - HorizontalFlip하면 BBox x좌표도 반전


2. **W&B Sweep의 용도는?**
   - 하이퍼파라미터 자동 탐색
   - Grid, Random, Bayesian search 지원


3. **timm의 features_only 모드란?**
   - Classification head 없이 중간 feature만 추출
   - Detection/Segmentation backbone으로 사용


---


## 이번 주 체크리스트


- [ ] Albumentations 설치 및 기본 변환 테스트
- [ ] Detection용 BBox 변환 파이프라인 구축
- [ ] W&B 가입, 프로젝트 생성, 학습 로깅
- [ ] timm으로 Pretrained 모델 로드 및 Feature 추출
- [ ] torchvision Detection API 간단 테스트


---


## 핵심 요약


```
+---------------------------------------------------------+
| CV 라이브러리 도구 세트 |
| |
| Albumentations: 빠른 Augmentation + BBox 지원 |
| W&B: 실험 로깅, 시각화, 하이퍼파라미터 탐색 |
| timm: 최신 Pretrained 모델 700+ (backbone) |
| torchvision: 공식 Detection/Segmentation API |
+---------------------------------------------------------+
```


---


이전: [Week 1 - PyTorch 기초 재정비](../week1/README.md)
다음: [Week 3 - YOLO 이론](../week3/README.md)
