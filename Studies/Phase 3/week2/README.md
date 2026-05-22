# Week 2: 컴퓨터 비전용 라이브러리 (Section 5.1)

> **이번 주 목표**: Albumentations, W&B, torchvision/timm 라이브러리를 활용한 CV 파이프라인 구축
> **예상 시간**: 12시간
> **핵심 질문**: "효율적인 CV 실험을 위한 도구 세트는 무엇이고, 각 도구는 어떤 원리로 동작하는가?"

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

Week 1에서는 PyTorch의 기본 연산과 직접 만든 단순 CNN으로 학습 루프를 굴려 봤다. Week 2는 그 위에, 실무에서 실제로 쓰는 **전문 라이브러리**를 얹는다. 직접 구현하던 부분을 검증된 도구로 대체하는 단계다.

| Week 1 | -> | Week 2 |
|---------|---|---------|
| PyTorch 기본 연산 | -> | CV 전문 라이브러리 활용 |
| 단순 CNN 직접 학습 | -> | Pretrained 모델 + Augmentation |
| 수동 실험 기록 | -> | W&B 자동 실험 관리 |

이번 주에 다루는 도구는 세 종류다.

- **Albumentations**: 데이터 증강(Augmentation). 학습 이미지를 인위적으로 다양하게 만들어 과적합을 막는다.
- **W&B (Weights & Biases)**: 실험 관리. 학습 중 손실/정확도를 자동 기록하고 웹에서 비교한다.
- **timm / torchvision**: 사전학습(Pretrained) 모델 모음. 남이 학습해 둔 모델을 backbone으로 가져다 쓴다.

---

## 핵심 개념 자세히 알아보기

### 1. Albumentations (Data Augmentation)

#### 1.1 Augmentation이란, 그리고 왜 과적합을 막는가

**Data Augmentation(데이터 증강)** 은 원본 학습 이미지를 좌우 반전, 밝기 변경, 노이즈 추가 등으로 변형해 **학습 데이터를 인위적으로 늘리는 기법**이다.

왜 이게 과적합을 막는가? 과적합(Overfitting)은 모델이 학습 데이터의 **본질이 아닌 우연한 특징**(특정 배경, 특정 조명, 특정 각도)까지 통째로 외워 버리는 현상이다. 처음 보는 데이터에서는 그 우연한 특징이 없으니 성능이 떨어진다.

```
원본 이미지 1장
        |
        v  Augmentation 적용
+------------------------------------------+
| 좌우 반전본 / 밝게 한 본 / 어둡게 한 본   |
| 노이즈 추가본 / 약간 회전한 본 ...        |
+------------------------------------------+
        |
        v
모델은 "반전해도, 밝아도, 노이즈가 껴도
        여전히 고양이는 고양이"라는
        변형에 불변인 본질적 특징을 학습
```

효과를 정리하면:

- 같은 객체를 다양한 모습으로 보여줘 모델이 **robust(강건)한 특징**을 학습한다.
- 실제 환경의 다양성(밝기, 각도, 가림)을 미리 모사한다.
- 데이터가 적어도 일반화 성능을 끌어올린다.

#### 1.2 A.Compose 파이프라인 구조

Albumentations는 여러 변환을 `A.Compose`로 묶어 **순서대로** 적용한다. 각 변환의 `p`는 그 변환이 적용될 확률이다.

```python
import albumentations as A  # 이미지 증강 라이브러리

# 변환들을 리스트로 묶어 순서대로 적용하는 파이프라인 생성
transform = A.Compose([
    A.Resize(640, 640),               # 이미지를 640x640 정사각형으로 크기 통일
    A.HorizontalFlip(p=0.5),          # 50% 확률로 좌우 반전
    A.RandomBrightnessContrast(p=0.3),# 30% 확률로 밝기/대비 무작위 변경
    A.Normalize(                      # 픽셀값 정규화 (ImageNet 평균/표준편차)
        mean=[0.485, 0.456, 0.406],
        std=[0.229, 0.224, 0.225]),
])

# numpy 이미지(H, W, C)를 넣으면 변환된 이미지를 돌려준다
result = transform(image=image)       # 키워드 인자 image= 로 전달
augmented = result["image"]           # 결과는 딕셔너리, "image" 키로 꺼냄
```

**장점**: OpenCV 기반이라 빠르고, BBox를 자동으로 함께 변환하며(§1.6), 변환 종류가 풍부하다.

#### 1.3 BBox 포맷 3종 (Pascal VOC / COCO / YOLO)

Detection에서는 이미지뿐 아니라 **BBox(Bounding Box, 객체를 감싸는 사각형)** 도 다룬다. BBox를 숫자로 표현하는 방식이 여러 가지라, 어느 포맷인지 항상 확인해야 한다.

| 포맷 | 표현 | 좌표 단위 | 예시 (640x480 이미지) |
|------|------|----------|----------------------|
| **Pascal VOC** | `[x_min, y_min, x_max, y_max]` | 픽셀 (절대) | `[100, 200, 300, 400]` |
| **COCO** | `[x_min, y_min, width, height]` | 픽셀 (절대) | `[100, 200, 200, 200]` |
| **YOLO** | `[x_center, y_center, width, height]` | 정규화 0-1 (상대) | `[0.3125, 0.625, 0.3125, 0.4167]` |

핵심 차이 두 가지:

- **무엇을 좌표로 쓰는가**: Pascal VOC는 좌상단/우하단 두 점, COCO와 YOLO는 좌상단(또는 중심) + 크기.
- **픽셀이냐 정규화냐**: Pascal VOC/COCO는 픽셀 절대값, YOLO는 이미지 크기로 나눈 0-1 상대값. YOLO가 정규화를 쓰는 이유는 [Week 4](../week4/README.md)에서 다룬다 (이미지 크기가 바뀌어도 라벨이 그대로 유효).

#### 1.4 BBox 포맷 변환 공식

가장 자주 쓰는 변환은 **Pascal VOC -> YOLO**다. 이미지 너비 `W`, 높이 `H`일 때:

```
x_center = (x_min + x_max) / 2 / W      # 중심 x를 구하고 W로 나눠 정규화
y_center = (y_min + y_max) / 2 / H      # 중심 y를 구하고 H로 나눠 정규화
width    = (x_max - x_min)     / W      # 너비를 W로 나눠 정규화
height   = (y_max - y_min)     / H      # 높이를 H로 나눠 정규화
```

숫자로 따라가 보자. 640x480 이미지, Pascal VOC `[100, 200, 300, 400]`:

```
x_center = (100 + 300) / 2 / 640 = 200 / 640 = 0.3125
y_center = (200 + 400) / 2 / 480 = 300 / 480 = 0.6250
width    = (300 - 100)     / 640 = 200 / 640 = 0.3125
height   = (400 - 200)     / 480 = 200 / 480 = 0.4167

-> YOLO: [0.3125, 0.6250, 0.3125, 0.4167]
```

역방향(YOLO -> Pascal VOC)은 위 식을 거꾸로 풀면 된다:

```
x_min = (x_center - width / 2)  * W
x_max = (x_center + width / 2)  * W
y_min = (y_center - height / 2) * H
y_max = (y_center + height / 2) * H
```

#### 1.5 변환 시 BBox 좌표가 어떻게 바뀌는가

이미지를 변환하면 BBox 좌표도 같이 바뀌어야 한다. 대표적인 두 변환:

**HorizontalFlip (좌우 반전)**: x축만 뒤집힌다. y와 크기는 그대로다.

```
YOLO 좌표 기준:  x_center_flip = 1.0 - x_center
픽셀 좌표 기준:  x_min_flip = W - x_max,  x_max_flip = W - x_min
```

위 예시 YOLO `[0.3125, 0.625, 0.3125, 0.4167]`을 좌우 반전하면:

```
x_center_flip = 1.0 - 0.3125 = 0.6875
-> YOLO: [0.6875, 0.625, 0.3125, 0.4167]   (x_center만 변함)
```

**Resize (크기 변경)**: 여기서 포맷에 따라 결과가 갈린다.

- **YOLO(정규화 좌표)**: Resize에 **불변**이다. 0-1 상대값이라 이미지 크기와 무관하다.
- **Pascal VOC(픽셀 좌표)**: 스케일 배수를 곱해야 한다.

640x480 이미지를 320x320으로 Resize하면 `scale_x = 320/640 = 0.5`, `scale_y = 320/480 = 0.6667`:

```
Pascal VOC [100, 200, 300, 400] 에 스케일 적용:
  x_min' = 100 * 0.5    = 50
  y_min' = 200 * 0.6667 = 133
  x_max' = 300 * 0.5    = 150
  y_max' = 400 * 0.6667 = 266
-> Pascal VOC: [50, 133, 150, 266]
```

`quiz_medium.py` 문제 1이 이 세 가지 계산(VOC->YOLO, Flip, Resize)을 그대로 묻는다.

#### 1.6 bbox_params: BBox를 이미지와 함께 자동 변환하기

`A.Compose`에 이미지만 넣으면 BBox는 변환되지 않는다. BBox도 함께 변환하려면 **`bbox_params`를 `A.Compose`에 전달**해야 한다. 이것이 `quiz_easy.py` 문제 1의 정답이 "B) bbox_params를 Compose에 전달"인 이유다.

```python
import albumentations as A

transform = A.Compose(
    [                                      # 적용할 변환 리스트
        A.Resize(640, 640),
        A.HorizontalFlip(p=0.5),
    ],
    bbox_params=A.BboxParams(              # BBox도 함께 변환되도록 설정
        format="yolo",                     # 입력 BBox의 포맷 (yolo/pascal_voc/coco)
        label_fields=["class_labels"],     # BBox에 대응하는 라벨이 담긴 인자 이름
        min_visibility=0.3,                # 변환 후 30% 미만만 남은 BBox는 버림
    ),
)

# 이미지 + BBox + 라벨을 함께 넣으면 셋 다 일관되게 변환된다
result = transform(
    image=image,                           # numpy 이미지
    bboxes=[[0.5, 0.5, 0.3, 0.4]],         # YOLO 포맷 BBox 리스트
    class_labels=[0],                      # 각 BBox의 클래스 인덱스
)
augmented_image = result["image"]          # 변환된 이미지
augmented_bboxes = result["bboxes"]        # 함께 변환된 BBox
```

`A.BboxParams`의 세 인자:

- **`format`**: 입력 BBox가 어느 포맷인지 (§1.3). `'yolo'`, `'pascal_voc'`, `'coco'`, `'albumentations'`.
- **`label_fields`**: BBox마다 붙는 클래스 라벨이 어느 인자에 들어오는지. 위 예시에서는 `class_labels`.
- **`min_visibility`**: 변환(특히 회전/크롭) 후 BBox가 이미지 밖으로 잘려 일부만 남을 수 있다. 원래 넓이의 이 비율 미만만 남은 BBox는 버린다. 0.3이면 "30% 미만 남으면 폐기".

#### 1.7 ToTensorV2가 무엇이고 왜 마지막에 오는가

`ToTensorV2`는 numpy 이미지를 PyTorch 텐서로 바꾸는 변환이다. 하는 일은 딱 두 가지다.

- 축 순서 변경: `HWC` (Height, Width, Channel) -> `CHW` (PyTorch 표준)
- numpy 배열 -> `torch.Tensor`

여기서 핵심은 **값을 건드리지 않는다**는 점이다. `[0, 255]` 범위의 픽셀 값을 `[0, 1]`로 나누지도 않고, dtype도 바꾸지 않는다 (uint8 입력이면 uint8 텐서 그대로).

이는 PyTorch 공식 `torchvision.transforms.ToTensor`와 다른 점이다.

| 항목 | `torchvision.transforms.ToTensor` | `ToTensorV2` (albumentations) |
|------|-----------------------------------|-------------------------------|
| 입력 타입 | PIL Image 또는 numpy | numpy만 (Albumentations가 numpy 기반) |
| 축 변환 | `HWC` -> `CHW` | `HWC` -> `CHW` |
| 값 스케일링 | `[0, 255]` -> `[0, 1]` (255로 나눔) | 안 함 |
| dtype 변환 | 무조건 `float32` | 안 함 (입력 dtype 유지) |

`ToTensorV2`가 스케일링을 하지 않기 때문에, 정규화는 바로 앞의 `A.Normalize()`가 전담한다. `A.Normalize()`는 기본값으로 255로 나눈 뒤 ImageNet 평균/표준편차를 적용한다. 즉 파이프라인 순서가 다음과 같아야 한다.

```
... 증강 변환들 ...
A.Normalize()   # 여기서 /255 + mean/std 정규화를 끝냄
ToTensorV2()    # 값은 그대로 두고 텐서 형태로만 변환 (반드시 Compose의 맨 마지막)
```

`ToTensorV2`를 `Normalize`보다 앞에 두거나, 여기에 torchvision `ToTensor`를 섞어 쓰면 스케일링이 두 번 적용되어 픽셀 값이 망가진다.

> **이름이 "V2"인 이유**: 구버전 `albumentations.pytorch.ToTensor`는 내부에서 스케일링을 암묵적으로 수행해 혼란을 일으켰다. 이를 deprecated 처리하고, 스케일링을 일절 하지 않는 `ToTensorV2`로 대체했다. 현재 Albumentations에는 구 `ToTensor`가 없으므로 항상 `ToTensorV2`를 쓴다.

---

### 2. Weights & Biases (실험 관리)

#### 2.1 W&B란, 그리고 왜 필요한가

**W&B (Weights & Biases)**는 머신러닝 **실험을 추적하고 관리하는 도구**다.

모델을 학습시킬 때는 설정을 바꿔 가며 여러 번 실험한다 (learning rate 바꾸고, 모델 바꾸고, augmentation 바꾸고). 이때 손으로 결과를 기록하면 누락되고, 비교가 어렵고, 손실 곡선을 보려면 매번 직접 그려야 한다.

W&B는 학습 중 손실/정확도 같은 지표를 **자동으로 서버에 기록**하고, 웹 대시보드에서 곡선과 실험 간 비교를 보여준다. Week 1의 "수동 실험 기록"이 Week 2의 "W&B 자동 실험 관리"로 바뀌는 지점이다.

#### 2.2 핵심 워크플로우 (init / log / finish)

W&B 사용은 세 함수로 끝난다.

- `wandb.init(...)`: 실험 하나(= run)를 시작한다.
- `wandb.log({...})`: 지표를 W&B로 전송한다. 학습 루프 안에서 epoch마다 호출.
- `wandb.finish()`: 실험을 종료한다.

용어 두 개를 먼저 정리한다.

- **project**: 관련된 실험들을 묶는 폴더 같은 단위 (예: `"yolo-training"`).
- **run**: 한 번의 실험. project 안에 여러 run이 쌓인다 (예: `"exp-001"`).
- **config**: 이번 run의 하이퍼파라미터 모음. run과 함께 저장돼 나중에 "어떤 설정의 실험이었나"를 알 수 있다.

```python
import os
import wandb

# wandb를 오프라인 모드로 동작 -> 계정/로그인 없이 로컬에만 기록 (실습용)
os.environ["WANDB_MODE"] = "offline"

# 실험(run) 시작
wandb.init(
    project="cv-experiment",          # 실험들을 묶는 프로젝트 이름
    name="resnet18-exp-001",          # 이번 run의 이름
    config={                          # 이번 실험의 하이퍼파라미터 (run과 함께 기록)
        "lr": 0.001,
        "batch_size": 16,
        "epochs": 50,
    },
)

# 학습 루프를 흉내 낸 예시 (실제로는 모델이 손실을 계산한다)
for epoch in range(50):                # 50 에폭 반복
    train_loss = 1.0 / (epoch + 1)     # 예시값: 에폭이 늘수록 감소하는 학습 손실
    val_loss = 1.2 / (epoch + 1)       # 예시값: 검증 손실
    wandb.log({                        # 이번 에폭 지표를 W&B로 전송
        "epoch": epoch,
        "train_loss": train_loss,
        "val_loss": val_loss,
    })

wandb.finish()                         # 실험 종료 (로그 업로드 마무리)
```

`wandb.log()`의 역할은 "학습 메트릭을 서버에 기록하여 시각화"다 — `quiz_easy.py` 문제 2의 정답이 이것이다.

> 위 코드의 변수 `train_loss`, `val_loss`는 예시값이다. 실제 학습에서는 모델이 계산한 손실값을 넣는다. README의 코드 예시는 정의되지 않은 변수 없이 그대로 실행되도록 작성한다.

#### 2.3 offline / disabled 모드

`wandb.init()`을 그냥 호출하면 W&B 계정 로그인을 요구한다. 실습 단계에서 계정 없이 코드만 돌리려면, `init` 전에 환경 변수로 동작 모드를 정한다.

| 모드 | 설정 | 동작 |
|------|------|------|
| online (기본) | (설정 없음) | W&B 클라우드 서버에 기록. 계정/로그인 필요 |
| offline | `os.environ["WANDB_MODE"] = "offline"` | 로컬 `./wandb/` 폴더에만 기록. 계정 불필요 |
| disabled | `os.environ["WANDB_MODE"] = "disabled"` | 모든 wandb 호출이 no-op (아무것도 기록 안 함) |

실습에서는 `offline`을 권한다. 로컬에 run 파일이 실제로 생겨 W&B가 무엇을 기록하는지 눈으로 확인할 수 있다. 클라우드 대시보드를 직접 보고 싶을 때만 https://wandb.ai 에서 계정을 만들고 `wandb login`을 한 번 하면 된다.

#### 2.4 train_loss와 val_mAP로 과적합 읽기

W&B로 기록한 지표 중 가장 중요한 조합이 **학습 손실(train_loss)**과 **검증 성능(val_mAP)**이다. 둘의 움직임으로 과적합을 판단한다.

```
정상 학습:   train_loss 감소  +  val_mAP 상승
            -> 모델이 일반화되며 잘 배우는 중

과적합:      train_loss 감소  +  val_mAP 정체 또는 하락
            -> 학습 데이터만 외우고 새 데이터엔 약함
```

`quiz_medium.py` 문제 3의 실험 표를 이 기준으로 읽어 보자.

| 실험 | lr | batch | aug | val_mAP | train_loss | 해석 |
|------|------|-------|------|---------|-----------|------|
| exp-A | 0.01 | 16 | basic | 0.45 | 0.8 | 손실이 덜 내려감 - 더 학습/조정 여지 |
| exp-B | 0.001 | 32 | heavy | **0.52** | 0.3 | val_mAP 최고 + 손실도 적절 - **가장 좋음** |
| exp-C | 0.001 | 16 | heavy | 0.38 | 0.15 | 손실은 최저인데 val_mAP는 최저 - **과적합** |

exp-C가 함정이다. `train_loss`가 0.15로 가장 낮아 "제일 잘 배운 것"처럼 보이지만, `val_mAP`는 0.38로 가장 낮다. 학습 데이터는 거의 외웠지만 검증 데이터에서 실패하는 전형적인 과적합이다. 손실값 하나만 보면 안 되고, 반드시 검증 성능과 함께 봐야 하는 이유다.

---

### 3. torchvision vs timm (사전학습 모델 모음)

**Pretrained 모델**은 남이 대규모 데이터(보통 ImageNet)로 미리 학습해 둔 모델이다. 이를 가져다 쓰면 적은 데이터와 짧은 시간으로 좋은 성능을 얻는다. 사전학습 모델을 제공하는 라이브러리가 두 개 있다.

| 항목 | torchvision | timm |
|------|-------------|------|
| 제공 주체 | PyTorch 공식 | 커뮤니티 (Hugging Face가 관리) |
| 모델 수 | 기본적인 모델 위주 | 최신 모델 700종 이상 (EfficientNet, ViT, ConvNeXt 등) |
| 강점 | Detection/Segmentation API 내장 | 최신 backbone, Feature extractor 기능 |

```python
import torchvision  # PyTorch 공식 모델/데이터셋 라이브러리

# torchvision: Detection 모델을 통째로 바로 사용 가능
model = torchvision.models.detection.fasterrcnn_resnet50_fpn(weights="DEFAULT")
```

```python
import timm  # 사전학습 이미지 모델 모음 라이브러리

# timm: 최신 모델을 이름으로 불러옴
model = timm.create_model("efficientnet_b0", pretrained=True)  # 사전학습 가중치 로드
```

정리하면, 검출/분할 모델을 통째로 빠르게 쓰려면 torchvision, 최신 backbone을 골라 feature extractor로 쓰려면 timm이다.

---

### 4. Pretrained 모델을 Feature Extractor로 활용

#### 4.1 timm.create_model과 features_only

`timm.create_model`로 모델을 부를 때 `features_only` 인자가 동작을 크게 바꾼다.

```python
import timm

# (A) 일반 분류 모델: 마지막에 분류 결과(logits)를 출력
clf = timm.create_model("resnet50", pretrained=True)
# clf(x) -> [B, 1000]  (ImageNet 1000개 클래스 점수)

# (B) Feature Extractor: 분류 헤드를 떼고 중간 feature map들을 출력
backbone = timm.create_model(
    "resnet50",
    pretrained=True,        # ImageNet 사전학습 가중치 로드
    features_only=True,     # 분류 헤드 제거, 중간 feature map만 반환
    out_indices=[1, 2, 3, 4],  # 추출할 stage 번호 (어느 단계의 feature를 뽑을지)
)
# backbone(x) -> feature map들의 리스트
```

`features_only=True`의 출력은 **최종 분류 logits가 아니라 중간 Feature map들의 리스트**다. 이것이 `quiz_easy.py` 문제 3의 정답이 "B) 중간 Feature map들의 리스트"인 이유다.

- `features_only=True`: 모델 끝의 분류 헤드(Classification head)를 제거한다.
- `out_indices`: 모델의 여러 단계(stage) 중 어느 것을 출력으로 뽑을지 지정한다.

이렇게 뽑은 feature map들은 Detection 모델의 **backbone**으로 쓰인다 (Week 3의 YOLO 구조로 이어진다).

#### 4.2 stride와 feature map 크기의 관계

CNN backbone은 입력 이미지를 단계적으로 줄여 나간다. **stride**는 "입력 대비 몇 배 축소됐는가"를 뜻한다. stride 8이면 입력의 1/8 크기다.

feature map의 공간 해상도(높이/너비)는 간단한 나눗셈으로 정해진다.

```
feature map 크기 = 입력 크기 / stride
```

640x640 입력을 ResNet-50에 넣으면 각 stage가 이렇게 나온다.

```
입력: [1, 3, 640, 640]

Layer1 (stride  4): [1,  256, 160, 160]   <- 640 / 4  = 160
Layer2 (stride  8): [1,  512,  80,  80]   <- 640 / 8  =  80
Layer3 (stride 16): [1, 1024,  40,  40]   <- 640 / 16 =  40
Layer4 (stride 32): [1, 2048,  20,  20]   <- 640 / 32 =  20
```

규칙성이 보인다: **stride가 커질수록 공간 해상도는 작아지고, 채널 수는 많아진다.** `quiz_medium.py` 문제 2가 이 계산을 그대로 묻는다.

#### 4.3 Feature Pyramid와 Multi-Scale

서로 다른 stride의 feature map을 모은 것을 **Feature Pyramid(특징 피라미드)**라 한다. 각 단계가 잘 잡는 객체 크기가 다르다.

```
작은 stride (예: 4) -> 고해상도 feature -> 작은 객체 검출에 유리
큰   stride (예: 32) -> 저해상도 feature -> 큰   객체 검출에 유리
```

그래서 Detection 모델은 여러 stride의 feature를 동시에 쓴다(Multi-Scale). `out_indices`로 여러 stage를 뽑는 이유가 이것이다. 작은 객체부터 큰 객체까지 한 번에 잡기 위해서다. 이 구조는 Week 3에서 YOLO의 Backbone-Neck-Head로 자세히 이어진다.

---

## 꼭 이해해야 할 핵심 개념

### 1. Augmentation은 "데이터를 늘리는" 것이 아니라 "외우지 못하게 하는" 것

Augmentation의 진짜 목적은 데이터 개수를 늘리는 게 아니라, 모델이 우연한 특징을 외우지 못하게 막는 것이다. 변형해도 정답이 같으니, 모델은 변형에 흔들리지 않는 본질적 특징만 학습하게 된다 (§1.1).

### 2. BBox는 항상 "어느 포맷인가"를 먼저 확인한다

같은 사각형도 Pascal VOC, COCO, YOLO에서 숫자가 전혀 다르다. 포맷을 착각하면 BBox가 엉뚱한 곳에 그려진다. Albumentations에 BBox를 넘길 때 `BboxParams(format=...)`를 정확히 지정하는 것이 그래서 중요하다 (§1.3, §1.6).

### 3. 손실값만으로 모델을 평가하지 않는다

`train_loss`가 낮다고 좋은 모델이 아니다. 검증 성능(`val_mAP`)이 함께 올라야 한다. 손실만 낮고 검증 성능이 낮으면 과적합이다 (§2.4).

---

## 자체 점검 - 이해했는지 확인!

**Q1. Albumentations에서 HorizontalFlip을 적용할 때 BBox도 자동 변환하려면?**
> `A.Compose`에 `bbox_params=A.BboxParams(...)`를 전달하면 된다. 이미지만 넣으면 BBox는 변환되지 않는다. `BboxParams`에는 입력 BBox의 `format`과, 라벨이 담긴 `label_fields`를 지정한다 (§1.6).

**Q2. wandb.log()의 역할은?**
> 학습 중 손실/정확도 같은 메트릭을 W&B 서버에 기록해 웹 대시보드에서 시각화하게 한다. 보통 학습 루프 안에서 epoch마다 호출한다 (§2.2).

**Q3. timm.create_model('resnet50', features_only=True)의 출력은?**
> 최종 분류 logits가 아니라, 모델 중간 단계들의 Feature map 리스트다. `features_only=True`가 분류 헤드를 제거하기 때문이다. 이 feature map들은 Detection backbone으로 쓰인다 (§4.1).

**Q4. Detection 학습에서 Augmentation이 중요한 이유는?**
> 같은 이미지를 다양하게 변형해 데이터 다양성을 높이고, 모델이 학습 데이터의 우연한 특징을 외우는 과적합을 막는다. 결과적으로 처음 보는 데이터에서도 잘 동작하는 robust한 모델이 된다 (§1.1).

---

## 이번 주 체크리스트

- [ ] Augmentation이 과적합을 막는 원리 설명 가능
- [ ] BBox 포맷 3종(Pascal VOC / COCO / YOLO)의 차이 설명 가능
- [ ] Pascal VOC <-> YOLO 변환 공식으로 직접 계산 가능
- [ ] `bbox_params`로 Detection용 BBox 변환 파이프라인 구축
- [ ] `ToTensorV2`가 `A.Normalize` 뒤, Compose 맨 끝에 와야 하는 이유 설명 가능
- [ ] W&B `init` / `log` / `finish` 워크플로우 이해, offline 모드로 실행
- [ ] `train_loss`와 `val_mAP`로 과적합 판단 가능
- [ ] `timm`으로 Pretrained 모델 로드 및 `features_only`로 Feature 추출
- [ ] stride와 feature map 크기 관계 계산 가능

---

## 핵심 요약

```
+---------------------------------------------------------+
| Week 2: CV 라이브러리 도구 세트                          |
|                                                         |
| Albumentations: 빠른 Augmentation + BBox 자동 변환       |
|   - 과적합 방지: 변형해도 정답 동일 -> 본질 특징 학습    |
|   - BBox 포맷 3종, bbox_params로 함께 변환               |
|   - ToTensorV2는 Normalize 뒤, 맨 마지막                 |
|                                                         |
| W&B: 실험 로깅, 시각화                                   |
|   - init -> log -> finish                                |
|   - train_loss + val_mAP 로 과적합 판단                  |
|                                                         |
| timm / torchvision: Pretrained 모델 (backbone)           |
|   - features_only=True -> 중간 feature map 리스트        |
|   - feature map 크기 = 입력 크기 / stride                |
+---------------------------------------------------------+
```

---

이전: [Week 1 - PyTorch 기초 재정비](../week1/README.md)
다음: [Week 3 - YOLO 이론](../week3/README.md)
