# Week 2 실습: CV 라이브러리 파이프라인


## 실습 목표
- Albumentations 변환 파이프라인으로 Detection 데이터 준비
- W&B로 실험 추적 설정
- timm Pretrained 모델을 Feature Extractor로 활용


---


## 실행 방법


```bash
cd Studies/Phase\ 3/week2
pip install -r requirements.txt
python quiz_easy.py # 개념 퀴즈
python quiz_medium.py # 코드 퀴즈
```


---


## 핵심 코드


### 1. Albumentations Detection 파이프라인


```python
import albumentations as A # 이미지 증강 라이브러리 (bbox 동시 변환 지원)
from albumentations.pytorch import ToTensorV2 # numpy 이미지 -> PyTorch 텐서 변환
import cv2 # OpenCV (이미지 입출력)
import numpy as np


def get_detection_transforms(img_size=640, train=True):
    """Detection용 Augmentation 파이프라인"""
    if train:
        return A.Compose([ # 변환들을 순서대로 적용 (이미지 + bbox 함께)
            A.Resize(img_size, img_size), # 이미지를 정사각형 크기로 통일
            A.HorizontalFlip(p=0.5), # 50% 확률로 좌우 반전
            A.RandomBrightnessContrast( # 밝기/대비 무작위 변경 (30% 확률)
                brightness_limit=0.2,
                contrast_limit=0.2, p=0.3),
            A.HueSaturationValue(p=0.3), # 색조/채도/명도 무작위 변경 (30% 확률)
            A.GaussNoise(var_limit=(10, 50), p=0.2), # 가우시안 노이즈 추가 (20% 확률)
            A.Normalize( # ImageNet 평균/표준편차로 정규화
                mean=[0.485, 0.456, 0.406],
                std=[0.229, 0.224, 0.225]),
            ToTensorV2() # 마지막에 numpy -> 텐서로 변환
        ], bbox_params=A.BboxParams( # bbox도 이미지와 함께 변환되도록 설정
            format='yolo', # [x_center, y_center, w, h] 정규화
            label_fields=['class_labels'], # bbox에 대응하는 클래스 라벨 키 이름
            min_visibility=0.3 # 변환 후 30% 미만만 남은 bbox는 버림
        ))
    else:
        return A.Compose([ # 검증/테스트용 (증강 없이 크기 조정 + 정규화만)
            A.Resize(img_size, img_size),
            A.Normalize(
                mean=[0.485, 0.456, 0.406],
                std=[0.229, 0.224, 0.225]),
            ToTensorV2()
        ], bbox_params=A.BboxParams(
            format='yolo',
            label_fields=['class_labels']
        ))




# 사용 예시
image = cv2.imread("image.jpg") # 이미지 읽기 (OpenCV는 BGR 순서로 로드)
image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB) # BGR -> RGB 순서로 변환
bboxes = [[0.5, 0.5, 0.3, 0.4]] # YOLO format
labels = [0] # 각 bbox의 클래스 인덱스


transform = get_detection_transforms(train=True) # 학습용 파이프라인 생성
result = transform( # 이미지와 bbox를 함께 변환
    image=image,
    bboxes=bboxes,
    class_labels=labels
)
# result['image']: Tensor [C, H, W]
# result['bboxes']: 변환된 BBox 리스트
```


### 2. W&B 실험 추적


```python
import wandb # 실험 추적 도구 (Weights & Biases)
import torch


def train_with_wandb():
    """W&B로 학습 추적"""
    config = { # 이번 실험의 하이퍼파라미터 모음 (W&B에 함께 기록됨)
        "lr": 0.001,
        "batch_size": 16,
        "epochs": 50,
        "model": "resnet18",
        "augmentation": "albumentations_v1"
    }


    wandb.init( # 새 실험(run) 시작
        project="cv-experiment", # 실험들을 묶는 프로젝트 이름
        name="resnet18-aug-v1", # 이번 run의 이름
        config=config
    )


    for epoch in range(config["epochs"]):
        train_loss = 1.0 / (epoch + 1) # 예시값 (실제로는 학습 루프의 손실)
        val_loss = 1.2 / (epoch + 1)


        wandb.log({ # epoch마다 지표를 W&B 대시보드로 전송
            "epoch": epoch,
            "train/loss": train_loss,
            "val/loss": val_loss,
            "lr": config["lr"]
        })


    wandb.finish() # 실험 종료 (로그 업로드 마무리)
```


### 3. timm Feature Extractor


```python
import timm # 사전학습 이미지 모델 모음 라이브러리
import torch


def extract_features():
    """timm으로 Multi-scale Feature 추출"""
    model = timm.create_model( # EfficientNet-B0을 특징 추출기로 생성
        'efficientnet_b0',
        pretrained=True, # ImageNet 사전학습 가중치 로드
        features_only=True, # 분류 헤드 없이 중간 feature map만 반환
        out_indices=[2, 3, 4] # 추출할 feature 단계 (깊을수록 작고 추상적)
    )
    model.eval() # 평가 모드


    x = torch.randn(1, 3, 640, 640) # 가짜 입력 (배치 1, RGB, 640x640)
    with torch.no_grad(): # 추론이므로 gradient 계산 끔
        features = model(x) # 여러 스케일의 feature map 리스트 반환


    for i, f in enumerate(features): # 각 feature map의 shape 출력
        print(f"Feature {i}: shape = {f.shape}")
    # Feature 0: [1, 40, 80, 80]
    # Feature 1: [1, 112, 40, 40]
    # Feature 2: [1, 320, 20, 20]
```


---


## 참고 자료


- Albumentations: https://albumentations.ai/docs/
- W&B: https://docs.wandb.ai/
- timm: https://huggingface.co/docs/timm/


---


이전: [Week 1 PRACTICE](../week1/PRACTICE.md)
다음: [Week 3 PRACTICE](../week3/PRACTICE.md)
