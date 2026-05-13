# Week 2 실습: CV 라이브러리 파이프라인


## 실습 목표
- Albumentations 변환 파이프라인으로 Detection 데이터 준비
- W&B로 실험 추적 설정
- timm Pretrained 모델을 Feature Extractor로 활용


---


## 실행 방법


```bash
cd Studies/Phase\ 5/week2
pip install -r requirements.txt
python quiz_easy.py # 개념 퀴즈
python quiz_medium.py # 코드 퀴즈
```


---


## 핵심 코드


### 1. Albumentations Detection 파이프라인


```python
import albumentations as A
from albumentations.pytorch import ToTensorV2
import cv2
import numpy as np


def get_detection_transforms(img_size=640, train=True):
    """Detection용 Augmentation 파이프라인"""
    if train:
        return A.Compose([
            A.Resize(img_size, img_size),
            A.HorizontalFlip(p=0.5),
            A.RandomBrightnessContrast(
                brightness_limit=0.2,
                contrast_limit=0.2, p=0.3),
            A.HueSaturationValue(p=0.3),
            A.GaussNoise(var_limit=(10, 50), p=0.2),
            A.Normalize(
                mean=[0.485, 0.456, 0.406],
                std=[0.229, 0.224, 0.225]),
            ToTensorV2()
        ], bbox_params=A.BboxParams(
            format='yolo', # [x_center, y_center, w, h] 정규화
            label_fields=['class_labels'],
            min_visibility=0.3
        ))
    else:
        return A.Compose([
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
image = cv2.imread("image.jpg")
image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
bboxes = [[0.5, 0.5, 0.3, 0.4]] # YOLO format
labels = [0]


transform = get_detection_transforms(train=True)
result = transform(
    image=image,
    bboxes=bboxes,
    class_labels=labels
)
# result['image']: Tensor [C, H, W]
# result['bboxes']: 변환된 BBox 리스트
```


### 2. W&B 실험 추적


```python
import wandb
import torch


def train_with_wandb():
    """W&B로 학습 추적"""
    config = {
        "lr": 0.001,
        "batch_size": 16,
        "epochs": 50,
        "model": "resnet18",
        "augmentation": "albumentations_v1"
    }


    wandb.init(
        project="cv-experiment",
        name="resnet18-aug-v1",
        config=config
    )


    for epoch in range(config["epochs"]):
        train_loss = 1.0 / (epoch + 1) # 예시
        val_loss = 1.2 / (epoch + 1)


        wandb.log({
            "epoch": epoch,
            "train/loss": train_loss,
            "val/loss": val_loss,
            "lr": config["lr"]
        })


    wandb.finish()
```


### 3. timm Feature Extractor


```python
import timm
import torch


def extract_features():
    """timm으로 Multi-scale Feature 추출"""
    model = timm.create_model(
        'efficientnet_b0',
        pretrained=True,
        features_only=True,
        out_indices=[2, 3, 4]
    )
    model.eval()


    x = torch.randn(1, 3, 640, 640)
    with torch.no_grad():
        features = model(x)


    for i, f in enumerate(features):
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
