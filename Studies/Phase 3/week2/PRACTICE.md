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
            A.GaussNoise(std_range=(0.012, 0.028), p=0.2), # 가우시안 노이즈 추가 (20% 확률, std는 0-1 정규화 스케일)
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


#### 1-1. 변환 결과 시각화 (실습 확인용)


증강이 잘 들어갔는지 눈으로 확인한다. 같은 입력에 학습용 파이프라인을 5번 적용해서 매번 다르게 나오는지, BBox가 이미지와 함께 움직이는지(§1.6) 본다. 위 코드의 `image`, `bboxes`, `labels`, `transform`이 이미 정의되어 있다고 가정한다.


```python
import matplotlib.pyplot as plt # 이미지 격자 표시
import matplotlib.patches as patches # bbox 사각형 그리기

mean = np.array([0.485, 0.456, 0.406]) # 위 A.Normalize와 동일한 평균
std = np.array([0.229, 0.224, 0.225]) # 위 A.Normalize와 동일한 표준편차

fig, axes = plt.subplots(1, 6, figsize=(24, 4)) # 1행 6칸 (원본 1 + 증강 5)

# 0번 칸: 원본 (증강 없음, 그대로 표시)
axes[0].imshow(image) # uint8 이미지는 imshow가 그대로 처리
axes[0].set_title("Original")
H, W = image.shape[:2] # bbox 픽셀 환산용 크기
for xc, yc, w, h in bboxes: # YOLO 정규화 -> 좌상단 픽셀 좌표 (README §1.3 역변환)
    axes[0].add_patch(patches.Rectangle(
        ((xc - w / 2) * W, (yc - h / 2) * H), w * W, h * H,
        linewidth=2, edgecolor='red', facecolor='none'))

# 1-5번 칸: 같은 입력에 학습 파이프라인을 매번 새로 호출 (무작위 증강이 다르게 적용됨)
for i in range(5):
    r = transform(image=image, bboxes=bboxes, class_labels=labels)
    img = r['image'].permute(1, 2, 0).numpy() * std + mean # [C,H,W] -> [H,W,C], 정규화 역연산
    img = np.clip(img, 0, 1) # 부동소수 오차 보정
    axes[i + 1].imshow(img)
    axes[i + 1].set_title(f"Aug {i + 1}")
    H, W = img.shape[:2]
    for xc, yc, w, h in r['bboxes']: # 함께 변환된 bbox를 같은 공식으로 표시
        axes[i + 1].add_patch(patches.Rectangle(
            ((xc - w / 2) * W, (yc - h / 2) * H), w * W, h * H,
            linewidth=2, edgecolor='red', facecolor='none'))

plt.show()
```


**무엇을 확인할 것인가:**
- bbox가 객체 위치에 맞게 **이미지와 함께 움직이는가** (`bbox_params` 정상 동작 — §1.6).
- 5번 결과가 **서로 다르게** 나오는가 (무작위 증강이 작동 중).


### 2. W&B 실험 추적


```python
import os
import wandb # 실험 추적 도구 (Weights & Biases)
import torch


# W&B 동작 모드 선택 (README §2.3 표 참고)
# - offline (실습 권장): 로컬 ./wandb/ 폴더에만 기록, 계정/로그인 불필요
# - online: 아래 줄을 주석 처리하면 클라우드에 기록 (wandb login 필요)
# - disabled: "offline" 대신 "disabled" 를 넣으면 모든 wandb 호출이 no-op
os.environ["WANDB_MODE"] = "offline" # wandb.init() 보다 앞에서 설정해야 적용됨


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


train_with_wandb()
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


extract_features() # 함수 실행 (사전학습 가중치 다운로드 후 feature shape 출력)
```


---


## 참고 자료


- Albumentations: https://albumentations.ai/docs/
- W&B: https://docs.wandb.ai/
- timm: https://huggingface.co/docs/timm/


---


이전: [Week 1 PRACTICE](../week1/PRACTICE.md)
다음: [Week 3 PRACTICE](../week3/PRACTICE.md)
