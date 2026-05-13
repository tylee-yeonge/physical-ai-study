# Week 3: YOLO 이론 (Section 5.2)

> [goal] **이번 주 목표**: YOLO의 발전사를 이해하고, YOLO11의 구조(Backbone-Neck-Head)와 Loss 함수, Detection 평가 지표를 학습한다.
> [time] **예상 시간**: 12시간
> [tip] **핵심 질문**: "YOLO11이 어떤 원리로 하나의 이미지에서 여러 객체를 한 번에 검출하는가?"

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | YOLO 구조, Anchor, mAP 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | IoU 계산, NMS 구현 코드 작성 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | YOLO 이론 검증 실습 |

---

## [*] 시작하기 전에

### Week 2와의 연결

| Week 2 | → | Week 3 |
|---------|---|---------|
| CV 라이브러리 도구 | → | YOLO Detection 이론 |
| Augmentation 파이프라인 | → | Detection 학습 데이터 이해 |
| Pretrained 모델 활용 | → | YOLO Backbone 구조 이해 |

### 왜 YOLO인가?

YOLO(You Only Look Once)는 **실시간 객체 검출(Real-time Object Detection)**의 대표 모델입니다. SLAM 시스템에서 주변 환경의 객체를 인식할 때 가장 널리 쓰입니다.

| 특징 | 설명 |
|------|------|
| **One-Stage** | 한 번의 Forward pass로 검출 (Two-Stage보다 빠름) |
| **실시간** | 30 FPS 이상으로 영상 처리 가능 |
| **End-to-End** | 입력 이미지 → BBox + Class + Confidence 직접 출력 |
| **범용성** | 자율주행, 로봇, CCTV 등 다양한 분야에서 활용 |

---

## [ref] 핵심 개념 자세히 알아보기

### 1. YOLO 발전사 개요

YOLO는 2015년 YOLOv1 이후 꾸준히 발전해왔습니다. 크게 **Anchor 기반**과 **Anchor-free** 방식으로 나뉩니다.

```
YOLO 발전 타임라인
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

2015  YOLOv1 --- 최초의 One-Stage Detector
        |
2016  YOLOv2 --- Anchor Box 도입, Batch Normalization
        |
2018  YOLOv3 --- Multi-Scale Detection (FPN)
        |         ---- Anchor 기반 시대 ----
        |
2020  YOLOv5 --- PyTorch 구현, 실용성 극대화
        |
2022  YOLOv7 --- E-ELAN, Auxiliary Head
        |
2023  YOLOv8 --- Anchor-Free, Decoupled Head, C2f
        |         ---- Anchor-Free 시대 ----
        |
2024  YOLO11 --- C3k2 블록 + C2PSA (Position-Sensitive
        |         Attention) Neck, v8 대비 파라미터 감소
        |         + mAP 향상
        |
2025  YOLO26 --- 최신 (커뮤니티 평가 미흡, 참고용)
```

---

### 2. Anchor 기반 vs Anchor-Free

#### Anchor 기반 (YOLOv1~v7, v5도 기본 anchor)

**Anchor**란 미리 정의한 BBox 템플릿입니다. 모델은 이 템플릿을 기준으로 **오프셋(offset)**을 예측합니다.

```
Anchor 기반 방식:
+-------------------------+
|  미리 정의된 Anchor Box  |
|  +--+  +----+  +-+     |
|  |  |  |    |  | |     |
|  +--+  +----+  +-+     |
|  작은   중간    큰      |
|                         |
|  모델 예측:             |
|    Δx, Δy (위치 보정)   |
|    Δw, Δh (크기 보정)   |
|    confidence           |
|    class probabilities  |
+-------------------------+
```

**문제점**:
- Anchor 크기/비율을 사전에 결정해야 함 (K-means clustering)
- Anchor 수가 많아지면 연산량 증가
- 데이터셋에 따라 최적 Anchor가 달라짐

#### Anchor-Free (YOLOv8 ~ YOLO11)

**Anchor-Free** 방식은 미리 정의된 Anchor 없이 **각 셀에서 직접 BBox를 예측**합니다. YOLO11은 v8에서 도입된 이 방식을 그대로 계승합니다.

```
Anchor-Free 방식:
+-------------------------+
|  각 그리드 셀이 직접     |
|  BBox 좌표를 예측        |
|                         |
|    +----------+         |
|    |  *       |  * = 셀 중심  |
|    |  <-> top   |         |
|    |←left  right→|      |
|    |  <-> bottom|         |
|    +----------+         |
|                         |
|  예측값:                |
|    left, top, right, bottom |
|    (셀 중심으로부터 거리) |
+-------------------------+
```

**장점**:
- Anchor 설계/튜닝이 불필요
- 더 유연한 BBox 예측
- 학습이 더 안정적

---

### 3. YOLO11 전체 구조

YOLO11은 **Backbone → Neck → Head** 세 부분으로 구성됩니다. v8 대비 핵심 변경점은 두 가지로, Backbone의 **C2f → C3k2** 교체와 Neck에 **C2PSA(Position-Sensitive Attention)** 모듈 추가입니다.

```
입력 이미지 [B, 3, 640, 640]
        |
        v
+--------------------------+
|     Backbone (CSPDarknet) |  ← 특징 추출
|                          |
|  Conv --→ C3k2 --→ Conv  |  (v8의 C2f를 C3k2로 교체)
|    ↓         ↓       ↓   |
|   P3       P4      P5    |  ← Multi-Scale Feature Maps
|  [80x80]  [40x40] [20x20]|
|                          |
|  + SPPF                  |
|  + C2PSA (Attention)     |  ← YOLO11 신규
+------+-------+-------+---+
       |       |       |
       v       v       v
+--------------------------+
|      Neck (PANet/FPN)     |  ← 특징 융합
|                          |
|  P5 --→ Upsample --→ P4 |  Top-Down (FPN)
|  P4 --→ Upsample --→ P3 |
|  P3 --→ Downsample → P4 |  Bottom-Up (PAN)
|  P4 --→ Downsample → P5 |
|                          |
|  출력: N3, N4, N5        |
+------+-------+-------+---+
       |       |       |
       v       v       v
+--------------------------+
|   Head (Decoupled Head)   |  ← 예측
|                          |
|  각 스케일별 독립 예측:   |
|    +- BBox Branch (reg)  |  → [B, 64, H, W]
|    +- Cls Branch (cls)   |  → [B, nc, H, W]
|                          |
|  ※ Objectness 제거 (v8~)|
+--------------------------+
```

---

### 4. Backbone: CSPDarknet

CSPDarknet은 **CSP(Cross Stage Partial)** 구조를 활용한 특징 추출기입니다.

#### C3k2 모듈 (YOLO11 핵심 블록)

YOLO11은 v8의 **C2f**를 **C3k2** 블록으로 교체했습니다. C3k2는 v5의 C3 블록 계열을 잇는 변형으로, 내부 Bottleneck의 커널 크기를 작게(k=2 단위) 가져가 연산량 대비 표현력을 더 끌어올립니다.

```
C3k2 (C3 with smaller kernel-2 Bottlenecks)

입력 -→ Conv1x1 -→ Split -→ Branch1 (direct)
                      |                    |
                      +→ C3k(k=2) -+→ Concat -→ Conv1x1 -→ 출력
                         C3k(k=2) -+
                         ...

장점:
  - C2f 대비 파라미터 / FLOPs 감소
  - 작은 커널 Bottleneck으로 표현력 유지
  - Gradient flow 분리는 CSP 계열 그대로 계승
```

```python
# C3k2 모듈의 개념적 구현 (Ultralytics 구현 참조)
import torch
import torch.nn as nn

class C3k2(nn.Module):
    """C3-style block with kernel-2 Bottlenecks (YOLO11)"""
    def __init__(self, c_in, c_out, n=1, c3k=False):
        super().__init__()
        self.c = c_out // 2
        self.cv1 = nn.Conv2d(c_in, 2 * self.c, 1)            # 1x1 Conv -> split
        self.cv2 = nn.Conv2d((2 + n) * self.c, c_out, 1)
        # c3k=True면 내부 블록을 C3k, 아니면 일반 Bottleneck 사용
        self.m = nn.ModuleList(
            [C3kBlock(self.c, self.c) if c3k else Bottleneck(self.c, self.c)
             for _ in range(n)]
        )

    def forward(self, x):
        y = list(self.cv1(x).chunk(2, 1))
        for m in self.m:
            y.append(m(y[-1]))
        return self.cv2(torch.cat(y, 1))
```

> [tip] C2f vs C3k2: 외형은 유사하지만, C3k2는 내부 Bottleneck의 receptive field를 작게 가져가 모델 크기를 줄이면서 mAP를 유지/향상시킵니다.

#### C2PSA: Position-Sensitive Attention (YOLO11 신규)

YOLO11은 SPPF 뒤(Backbone 마지막)에 **C2PSA** 모듈을 추가했습니다. PSA(Position-Sensitive Self-Attention)를 CSP 형태로 감싼 블록으로, 다음 효과를 노립니다.

- 작은 객체 / 가려진 객체에 대한 공간적(positional) 주의(attention)
- Transformer의 전역 의존성을 일부 도입 (Backbone 끝단에서만 적용해 비용 최소화)
- v8의 순수 CNN 구조 대비 mAP 향상의 핵심 근거

#### Multi-Scale Feature Maps

```
입력: 640 x 640

Backbone 출력:
  P3: [B, 256,  80, 80]  ← stride 8  (작은 객체)
  P4: [B, 512,  40, 40]  ← stride 16 (중간 객체)
  P5: [B, 1024, 20, 20]  ← stride 32 (큰 객체)

※ stride가 작을수록 해상도가 높아 작은 객체 검출에 유리
```

---

### 5. Neck: PANet (Path Aggregation Network)

Neck은 Backbone에서 추출한 Multi-Scale Feature를 **융합(fusion)**합니다.

```
FPN (Top-Down) + PAN (Bottom-Up) 결합:

Top-Down (FPN):                Bottom-Up (PAN):
P5 -----→ Upsample + P4        N3 ----→ Downsample + N4
              |                              |
              v                              v
P4' ----→ Upsample + P3        N4' ---→ Downsample + N5
              |                              |
              v                              v
            P3'                            N5'

효과:
  - 고해상도(P3)는 작은 객체의 위치 정보를 가짐
  - 저해상도(P5)는 큰 객체의 의미적(semantic) 정보를 가짐
  - PANet은 이 두 정보를 양방향으로 전파
```

#### 왜 Multi-Scale이 중요한가?

```
640x640 이미지에서:

작은 객체 (사람 얼굴):
  → 이미지 내 20x20 픽셀 크기
  → P3 (80x80) Feature Map에서 검출

중간 객체 (자동차):
  → 이미지 내 80x80 픽셀 크기
  → P4 (40x40) Feature Map에서 검출

큰 객체 (건물):
  → 이미지 내 200x200 픽셀 크기
  → P5 (20x20) Feature Map에서 검출
```

---

### 6. Head: Decoupled Head

YOLO11의 Head는 v8과 마찬가지로 **Detection(BBox)과 Classification을 분리(Decoupled)**합니다.

```
이전 YOLO (Coupled Head):
  Feature → 하나의 Conv → [BBox, Objectness, Class]

YOLOv8 ~ YOLO11 (Decoupled Head):
  Feature -+-→ BBox Branch --→ BBox 예측 [B, 64, H, W]
           |    (Conv → Conv)
           |
           +-→ Cls Branch ---→ Class 예측 [B, nc, H, W]
                (Conv → Conv)

변경점 (v5 → v8 → v11):
  1. Objectness score 제거 (v8부터)
  2. BBox와 Classification을 독립적으로 학습
  3. BBox는 DFL(Distribution Focal Loss) 방식으로 예측
  4. YOLO11: Cls 분기에 Depthwise-Separable Conv 도입으로 경량화
```

#### DFL (Distribution Focal Loss)

```
기존 방식: BBox 좌표를 하나의 값으로 직접 예측
  → 부정확할 수 있음 (특히 경계가 애매한 객체)

DFL 방식: BBox 좌표를 확률 분포로 예측
  → 16개 bin의 확률 분포를 예측 후 기대값 계산
  → 더 정확한 BBox 위치 추정

예시 (left 거리 예측):
  bins:  [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
  probs: [0, 0, 0, 0.1, 0.5, 0.3, 0.1, 0, ...]
  예측값 = sum(bins * probs) = 4.4
```

---

### 7. Loss Function

YOLO11의 Loss는 v8과 동일하게 세 가지 요소로 구성됩니다.

```
Total Loss = λ_box * L_box + λ_cls * L_cls + λ_dfl * L_dfl

+-------------------------------------------------+
|  L_box: CIoU Loss (BBox 위치 정확도)             |
|  L_cls: BCE Loss (클래스 분류 정확도)             |
|  L_dfl: Distribution Focal Loss (BBox 분포)      |
+-------------------------------------------------+
```

#### IoU 계열 비교

```
IoU (Intersection over Union):
  = 교집합 / 합집합
  문제: BBox가 겹치지 않으면 gradient = 0

GIoU (Generalized IoU):
  = IoU - (C - Union) / C     (C: 두 BBox를 포함하는 최소 영역)
  해결: BBox가 떨어져 있어도 gradient 존재

DIoU (Distance IoU):
  = IoU - d²/c²               (d: 중심 거리, c: 대각선 거리)
  해결: 중심점 거리 고려

CIoU (Complete IoU):  ← YOLOv8 / YOLO11 사용
  = IoU - d²/c² - αv          (v: 종횡비 일관성)
  해결: 중심 거리 + 종횡비 모두 고려
```

```python
import torch

def ciou_loss(pred_box, target_box):
    """CIoU Loss 개념적 구현
    pred_box, target_box: [x1, y1, x2, y2] 형식
    """
    # 교집합 계산
    inter_x1 = torch.max(pred_box[0], target_box[0])
    inter_y1 = torch.max(pred_box[1], target_box[1])
    inter_x2 = torch.min(pred_box[2], target_box[2])
    inter_y2 = torch.min(pred_box[3], target_box[3])

    inter_area = torch.clamp(inter_x2 - inter_x1, min=0) * \
                 torch.clamp(inter_y2 - inter_y1, min=0)

    # 합집합 계산
    area_pred = (pred_box[2] - pred_box[0]) * (pred_box[3] - pred_box[1])
    area_target = (target_box[2] - target_box[0]) * (target_box[3] - target_box[1])
    union_area = area_pred + area_target - inter_area

    iou = inter_area / (union_area + 1e-7)

    # 중심점 거리
    pred_cx = (pred_box[0] + pred_box[2]) / 2
    pred_cy = (pred_box[1] + pred_box[3]) / 2
    target_cx = (target_box[0] + target_box[2]) / 2
    target_cy = (target_box[1] + target_box[3]) / 2
    d2 = (pred_cx - target_cx) ** 2 + (pred_cy - target_cy) ** 2

    # 포함하는 최소 영역의 대각선 거리
    c_x1 = torch.min(pred_box[0], target_box[0])
    c_y1 = torch.min(pred_box[1], target_box[1])
    c_x2 = torch.max(pred_box[2], target_box[2])
    c_y2 = torch.max(pred_box[3], target_box[3])
    c2 = (c_x2 - c_x1) ** 2 + (c_y2 - c_y1) ** 2

    # 종횡비 일관성
    import math
    pred_w = pred_box[2] - pred_box[0]
    pred_h = pred_box[3] - pred_box[1]
    target_w = target_box[2] - target_box[0]
    target_h = target_box[3] - target_box[1]

    v = (4 / math.pi ** 2) * (
        torch.atan(target_w / (target_h + 1e-7)) -
        torch.atan(pred_w / (pred_h + 1e-7))
    ) ** 2
    alpha = v / (1 - iou + v + 1e-7)

    ciou = iou - d2 / (c2 + 1e-7) - alpha * v
    return 1 - ciou  # Loss = 1 - CIoU
```

#### BCE Loss (Binary Cross Entropy)

```python
# 클래스 분류에 사용 (Multi-label classification)
# 각 클래스에 대해 독립적으로 이진 분류

import torch.nn as nn

bce_loss = nn.BCEWithLogitsLoss()

# pred_cls: [B, num_classes, H, W] (sigmoid 적용 전)
# target_cls: [B, num_classes, H, W] (0 또는 1)
loss_cls = bce_loss(pred_cls, target_cls)

# ※ Softmax가 아닌 Sigmoid를 사용하는 이유:
#   하나의 객체가 여러 클래스에 속할 수 있는 경우를 허용
#   (예: "동물" + "개")
```

---

### 8. Detection 평가 지표

#### Precision & Recall

```
                    실제 양성 (GT)    실제 음성
                +----------------+----------------+
예측 양성       |  TP (True Pos)  |  FP (False Pos) |
                +----------------+----------------+
예측 음성       |  FN (False Neg) |  TN (True Neg)  |
                +----------------+----------------+

Precision = TP / (TP + FP)
  "모델이 검출한 것 중 실제 맞는 비율"
  → 높을수록 오검출(FP)이 적음

Recall = TP / (TP + FN)
  "실제 객체 중 모델이 찾은 비율"
  → 높을수록 미검출(FN)이 적음
```

#### IoU 임계값

```
IoU ≥ 0.5  →  TP (올바른 검출)
IoU < 0.5  →  FP (잘못된 검출)

예시:
  GT Box:   [100, 100, 200, 200]
  Pred Box: [110, 105, 210, 205]
  IoU = 교집합 / 합집합 ≈ 0.81  →  TP (0.5 이상)
```

#### mAP (mean Average Precision)

```
AP 계산 과정:
  1. Confidence 순으로 검출 결과 정렬
  2. 각 검출에서 Precision, Recall 계산
  3. Precision-Recall 곡선 그리기
  4. 곡선 아래 면적 = AP (하나의 클래스)
  5. 모든 클래스의 AP 평균 = mAP

주요 지표:
  mAP@0.5       IoU 임계값 0.5에서의 mAP
  mAP@0.5:0.95  IoU 0.5~0.95 (0.05 간격) 평균 mAP
                → COCO 공식 지표 (더 엄격)

비유:
  mAP@0.5       = "대충 맞으면 OK" (위치 관대)
  mAP@0.5:0.95  = "정확히 맞아야 OK" (위치 엄격)
```

#### FPS (Frames Per Second)

```
FPS = 1초에 처리할 수 있는 이미지 수

실시간 기준:
  30 FPS 이상  → 실시간 (영상 처리)
  15 FPS 이상  → 준실시간
  15 FPS 미만  → 비실시간

모델별 대략적 비교 (640x640 기준, T4/RTX 클래스 GPU):
  Faster R-CNN: ~5 FPS   (정확하지만 느림)
  YOLOv5s:      ~140 FPS (빠르지만 덜 정확)
  YOLO11n:      ~180 FPS (가장 빠름, nano)
  YOLO11s:      ~140 FPS (균형)
  YOLO11x:      ~55 FPS  (정확, extra large)
```

---

## [tip] 꼭 이해해야 할 핵심 개념

### 1. One-Stage vs Two-Stage Detector

```
Two-Stage (Faster R-CNN):
  입력 → Backbone → RPN(Region Proposal) → RoI Pooling → Classification
  장점: 높은 정확도
  단점: 느림 (RPN 단계 추가)

One-Stage (YOLO):
  입력 → Backbone → Neck → Head → 바로 BBox + Class
  장점: 빠름 (한 번에 처리)
  단점: 작은 객체 검출이 상대적으로 약함 (→ Multi-Scale로 보완)
```

### 2. YOLOv5 vs YOLOv8 vs YOLO11 핵심 차이

| 항목 | YOLOv5 | YOLOv8 | YOLO11 |
|------|--------|--------|--------|
| Anchor | Anchor 기반 | Anchor-Free | **Anchor-Free** |
| Head | Coupled | Decoupled | **Decoupled** |
| Objectness | 있음 | 없음 | **없음** |
| BBox 예측 | offset | DFL | **DFL** |
| Backbone 블록 | C3 | C2f | **C3k2** |
| Neck Attention | 없음 | 없음 | **C2PSA (PSA)** |
| 같은 크기 mAP | 기준 | +2~3% | **+3~5%** |
| 같은 mAP 파라미터 | 기준 | 기준 | **약 22% 감소** |

### 3. NMS (Non-Maximum Suppression)

```
NMS: 중복 검출 제거

1. Confidence 순으로 정렬
2. 가장 높은 Confidence의 BBox 선택
3. 선택된 BBox와 IoU > threshold인 BBox 제거
4. 남은 BBox 중 반복

예시:
  BBox A (conf=0.95)  ← 선택
  BBox B (conf=0.85, IoU with A = 0.8)  ← 제거 (중복)
  BBox C (conf=0.75, IoU with A = 0.1)  ← 유지 (다른 객체)
```

---

## [search] 자체 점검 - 이해했는지 확인!

**Q1. Anchor 기반과 Anchor-Free의 핵심 차이는?**
> Anchor 기반은 미리 정의한 BBox 템플릿(Anchor)의 오프셋을 예측하고, Anchor-Free는 각 그리드 셀에서 직접 BBox 좌표를 예측합니다. YOLOv8부터 Anchor-Free 방식을 사용하여 Anchor 설계 부담을 없앴고, YOLO11도 이를 그대로 계승합니다.

**Q2. YOLO11의 Backbone-Neck-Head 각각의 역할은?**
> Backbone(CSPDarknet + C3k2 + SPPF + C2PSA)은 입력 이미지에서 Multi-Scale Feature Map을 추출하며, 마지막 단계에서 PSA로 공간적 주의를 추가합니다. Neck(PANet)은 Top-Down과 Bottom-Up 경로로 Feature를 융합합니다. Head(Decoupled Head)는 BBox와 Classification을 분리된 Branch로 예측합니다.

**Q3. CIoU Loss가 IoU Loss보다 나은 점은?**
> 기본 IoU Loss는 BBox가 겹치지 않으면 gradient가 0입니다. CIoU는 중심점 거리와 종횡비 일관성까지 고려하여, 어떤 상황에서도 의미 있는 gradient를 제공합니다.

**Q4. mAP@0.5와 mAP@0.5:0.95의 차이는?**
> mAP@0.5는 IoU 임계값 0.5에서만 평가하여 위치가 대략 맞으면 인정합니다. mAP@0.5:0.95는 0.5부터 0.95까지 다양한 임계값에서 평가하여 더 정확한 위치 예측을 요구합니다. COCO 공식 지표는 mAP@0.5:0.95입니다.

---

## [O] 이번 주 체크리스트

- [ ] YOLO 발전사 (v1~v11) 흐름 이해
- [ ] Anchor 기반 vs Anchor-Free 차이 설명 가능
- [ ] YOLO11 Backbone(CSPDarknet + C3k2 + C2PSA) 구조 이해
- [ ] YOLO11 Neck(PANet) 구조 이해
- [ ] YOLO11 Decoupled Head 구조 이해
- [ ] CIoU Loss 수식과 의미 이해
- [ ] Precision, Recall, mAP 계산 방법 이해
- [ ] mAP@0.5 vs mAP@0.5:0.95 차이 설명 가능
- [ ] NMS 동작 원리 이해
- [ ] YOLOv8 → YOLO11 변경점 (C2f→C3k2, C2PSA 추가) 설명 가능

---

## [note] 핵심 요약

```
+---------------------------------------------------------+
|  YOLO11 핵심 정리                                       |
|                                                         |
|  1. Anchor-Free: Anchor 없이 직접 BBox 예측             |
|  2. 구조: CSPDarknet + C3k2 + SPPF + C2PSA (Backbone)   |
|           → PANet(Neck) → Decoupled Head(Head)          |
|  3. Loss: CIoU(위치) + BCE(분류) + DFL(분포)            |
|  4. 평가: mAP@0.5:0.95 (COCO 공식 지표)                |
|  5. NMS로 중복 검출 제거                                 |
|  6. v8 대비: C2f -> C3k2, Neck에 C2PSA 추가             |
|     -> 같은 mAP에서 파라미터 약 22% 감소                |
+---------------------------------------------------------+
```

---

이전: [Week 2 - CV 라이브러리](../week2/README.md)
다음: [Week 4 - YOLO11 학습](../week4/README.md)
