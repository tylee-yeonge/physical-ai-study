# Week 2 실습: CV 라이브러리 파이프라인


## 실습 목표
- Albumentations 변환 파이프라인으로 Detection 데이터 준비
- W&B로 실험 추적 설정
- timm Pretrained 모델을 Feature Extractor로 활용


---


## 실행 방법


```bash
cd Studies/Phase\ 3/week2

# 가상환경 생성 및 의존성 설치 (apt 패키지 설치 포함, root 권한 필요)
# 내부에서 .venv-week2 생성 후 requirements.txt 설치
./pip_install.sh

# 가상환경 활성화 (sh 종료 후 현재 shell에서 직접 활성화 필요)
source .venv-week2/bin/activate

python quiz_easy.py # 개념 퀴즈
python quiz_medium.py # 코드 퀴즈
```


---


## 핵심 코드


### 1. Albumentations Detection 파이프라인

이 코드는 Detection 학습에 넣을 데이터 증강(augmentation) 파이프라인을 만든다. 핵심은 이미지를 변형할 때 그 위의 BBox도 함께 따라 변형되게 하는 것이다.

#### 왜 증강을, 그리고 왜 bbox까지 함께

증강의 목적은 데이터를 늘리는 것이 아니라, 모델이 학습 이미지의 우연한 특징(특정 배경/조명/각도)을 통째로 외우지 못하게 막는 것이다(과적합 방지). 매 epoch 같은 이미지를 조금씩 다르게 보여줘 본질만 학습하게 한다. Detection이 분류와 다른 점은, 이미지를 좌우 반전하거나 크롭하면 정답 BBox 좌표도 같이 바뀌어야 한다는 것이다. 손으로 좌표를 다시 계산하지 않으려면 라이브러리가 이미지와 bbox를 동시에 변환해줘야 하고, 그 역할을 하는 것이 `bbox_params`다.

#### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 증강 조합 | `A.Compose([...])`, 각 `p` | 변환을 순서대로 적용, `p`는 적용 확률 |
| bbox 동시 변환 | `bbox_params(format='yolo', ...)` | 이미지가 변하면 bbox도 같은 변환을 받음 |
| 잘린 bbox 처리 | `min_visibility=0.3` | 변환 후 30% 미만만 남은 bbox는 버림 |
| 학습/검증 분기 | `train=True`/`False` | 검증에는 증강을 빼고 Resize와 정규화만 |
| 변환 순서 | `ToTensorV2()`는 맨 마지막 | 정규화까지 numpy로 끝낸 뒤 텐서로 변환 |

#### 핵심 포인트

- `format`을 반드시 맞춰야 한다. 같은 사각형도 YOLO/COCO/Pascal VOC에서 숫자가 전혀 다르다. 포맷을 착각하면 bbox가 엉뚱한 곳에 그려진다. 바로 아래 1-1 시각화가 이것을 눈으로 점검하는 단계다.
- 검증/테스트에는 무작위 증강을 넣지 않는다. 평가는 고정된 조건이어야 한다(week1 실습 3과 같은 원칙).


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

이 코드는 학습 지표(loss 등)를 W&B에 기록하는 최소 예제다. 모델을 학습하는 것이 아니라, 실험을 어떻게 기록하고 비교하는가를 익히는 것이 목적이다.

#### 왜 실험 추적이 필요한가

하이퍼파라미터를 바꿔가며 수십 번 실험하면 어떤 설정이 제일 좋았는지를 머리로 기억할 수 없다. W&B는 각 run의 config와 지표 곡선을 자동으로 저장하고 비교해준다. week4의 하이퍼파라미터 비교 실험이 이 추적의 실전 버전이다. 실습에서는 offline 모드를 권한다. 로컬 `outputs/wandb/`에 run 파일이 실제로 생겨 W&B가 무엇을 기록하는지 눈으로 확인할 수 있고, 계정이나 로그인이 필요 없다.

#### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 동작 모드 | `WANDB_MODE="offline"` | 로컬 기록, 계정 불필요. `init` 전에 설정해야 적용 |
| 실험 시작 | `wandb.init(project, name, config)` | `config`로 하이퍼파라미터를 함께 박제 |
| 지표 기록 | `wandb.log({...})` | epoch마다 loss 등을 기록 |
| 실험 종료 | `wandb.finish()` | 로그 업로드 마무리 |

#### 핵심 포인트

- `train_loss = 1.0 / (epoch + 1)`은 예시값이다. 실제 학습 루프가 아니라 워크플로우(`init` -> `log` -> `finish`)만 보여준다.
- `WANDB_DIR`/`WANDB_MODE`는 `wandb.init()`보다 먼저 설정해야 한다. 이미 시작된 run에는 나중에 바꿔도 적용되지 않는다.


```python
import os
import wandb # 실험 추적 도구 (Weights & Biases)
import torch


# W&B 동작 모드 선택 (README §2.3 표 참고)
# - offline (실습 권장): 로컬 outputs/wandb/ 폴더에만 기록, 계정/로그인 불필요
# - online: 아래 줄을 주석 처리하면 클라우드에 기록 (wandb login 필요)
# - disabled: "offline" 대신 "disabled" 를 넣으면 모든 wandb 호출이 no-op
os.makedirs("outputs", exist_ok=True) # 결과물 폴더 (wandb 로그를 수업 자료와 분리)
os.environ["WANDB_DIR"] = "outputs" # wandb가 outputs/wandb/ 에 기록하도록 (cwd 오염 방지)
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

이 코드는 사전학습된 EfficientNet에서 분류 결과가 아니라 중간 feature map들을 뽑아낸다. Detection 모델의 backbone이 어떻게 쓰이는지 미리 보는 것이 목적이다.

#### 왜 feature map을 뽑나

Detection 모델(YOLO 등)은 통째로 새로 만들지 않는다. 이미 ImageNet으로 잘 학습된 분류 모델의 앞부분(특징 추출부)을 그대로 backbone으로 빌려오고, 그 위에 detection head를 얹는다. `features_only=True`는 분류 헤드를 떼고 이 backbone 출력만 꺼내는 스위치다. `out_indices`로 여러 단계의 feature를 뽑는 이유는, 얕은 단계는 크고 세밀해서 작은 객체에 강하고 깊은 단계는 작고 추상적이라 큰 객체에 강하기 때문이다. 이 여러 스케일을 모은 것이 Feature Pyramid이고, week3에서 보는 YOLO 구조의 기반이다.

#### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| backbone만 추출 | `features_only=True` | 분류 헤드 제거, 중간 feature map 리스트 반환 |
| multi-scale | `out_indices=[2, 3, 4]` | 깊이별로 다른 스케일의 feature |
| shape 읽기 | `f.shape` 출력 | 깊을수록 해상도는 작고 채널은 많음 |

#### 핵심 포인트

- 출력이 분류 logits가 아니라 feature map 리스트라는 점이 핵심이다. 깊어질수록 80x80 -> 40x40 -> 20x20으로 작아지고 채널은 40 -> 112 -> 320으로 늘어난다. 작은 객체는 80x80에서, 큰 객체는 20x20에서 잘 잡힌다.
- 이 multi-scale feature 묶음(Feature Pyramid)이 week3의 YOLO 구조 학습으로 이어진다.


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
