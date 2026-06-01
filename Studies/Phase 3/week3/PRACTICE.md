# Week 3 실습: YOLO 이론 검증


> **실습 목표**: YOLO의 핵심 개념을 코드로 직접 구현하여 이해를 깊게 한다.
> **예상 시간**: 6-8시간


---


## 환경 설정


```bash
cd Studies/Phase\ 3/week3

# 가상환경 생성 및 의존성 설치 (apt 패키지 설치 포함, root 권한 필요)
# 내부에서 .venv-week3 생성 후 requirements.txt 설치
./pip_install.sh

# 가상환경 활성화 (sh 종료 후 현재 shell에서 직접 활성화 필요)
source .venv-week3/bin/activate

python quiz_easy.py # 개념 퀴즈
python quiz_medium.py # 코드 퀴즈
```


---


## 실습 1: IoU 계열 Loss 직접 구현

이 실습은 모델을 학습시키는 것이 아니라, YOLO의 BBox 위치 손실(L_box)에 쓰이는 IoU 계열 지표 4종을 직접 구현해 각자 어떤 약점을 어떻게 메우는지 손으로 확인한다.

### 왜 IoU 변형을 직접 짜보나

Detection 학습은 예측 BBox를 정답 BBox에 맞춰가는 과정이고, 그 손실로 IoU 계열을 쓴다. 그런데 단순 IoU에는 치명적 약점이 있다. 두 박스가 겹치지 않으면 IoU가 0이고 조금 움직여도 여전히 0이라, 어느 방향으로 옮겨야 하는지에 대한 gradient 신호가 없다. GIoU/DIoU/CIoU는 이 빈틈을 차례로 메운 변형이다. YOLO11은 L_box로 CIoU를 쓴다(README 7절). 왜 그냥 IoU가 아니라 CIoU인지 이해하려면 각 변형이 어떤 시나리오에서 신호를 주는지 직접 봐야 한다. 이 실습은 6가지 시나리오에서 네 지표를 비교해 그 차이를 시각화한다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| IoU 한계 확인 | 겹치지 않는 박스에서 IoU=0 | gradient가 0이라 학습 신호가 없음 |
| GIoU | `IoU - (C-Union)/C` | 감싸는 박스의 빈 공간으로 "가까워지라" 신호 |
| DIoU | `IoU - d^2/c^2` | 중심점 거리를 직접 당겨 수렴이 빠름 |
| CIoU | DIoU + 종횡비 항 | 위치, 크기, 종횡비까지 맞춤 (YOLO11 채택) |

### 핵심 포인트

- 시각화의 핵심은 박스가 겹치지 않을 때 IoU는 멈춰 있지만 GIoU/DIoU는 계속 줄어든다는 것을 눈으로 보는 것이다. 손실이 계속 줄어야 gradient가 살아 학습이 진행된다.
- 콘솔 출력 설명은 한국어, matplotlib 제목은 영어로 둔 이유는 한글 폰트가 없으면 그림 제목이 깨지기 때문이다(코드 주석에 명시).


**파일명**: `practice_iou.py`


```python
"""
실습 1: IoU, GIoU, DIoU, CIoU 직접 구현
목표: 각 IoU 변형이 어떤 상황에서 어떤 gradient를 제공하는지 이해한다.
"""
import os
import math
import torch
import matplotlib
matplotlib.use('Agg') # GUI 없이 파일로 저장하는 백엔드
import matplotlib.pyplot as plt
import matplotlib.patches as patches # 사각형 등 도형 그리기용
import numpy as np


print("=" * 50)
print("실습 1: IoU 계열 Loss 비교")
print("=" * 50)




def compute_iou(box1, box2):
    """기본 IoU 계산 (xyxy 형식)"""
    # 교집합 영역 좌표 (좌상단은 둘 중 큰 값, 우하단은 둘 중 작은 값)
    x1 = torch.max(box1[0], box2[0])
    y1 = torch.max(box1[1], box2[1])
    x2 = torch.min(box1[2], box2[2])
    y2 = torch.min(box1[3], box2[3])


    inter = torch.clamp(x2 - x1, min=0) * torch.clamp(y2 - y1, min=0) # 교집합 넓이 (안 겹치면 0)
    area1 = (box1[2] - box1[0]) * (box1[3] - box1[1]) # box1 넓이
    area2 = (box2[2] - box2[0]) * (box2[3] - box2[1]) # box2 넓이
    union = area1 + area2 - inter # 합집합 넓이


    return inter / (union + 1e-7) # IoU = 교집합 / 합집합 (1e-7은 0 나눗셈 방지)




def compute_giou(box1, box2):
    """GIoU 계산"""
    iou = compute_iou(box1, box2)


    # 두 박스를 모두 감싸는 최소 영역 C
    c_x1 = torch.min(box1[0], box2[0])
    c_y1 = torch.min(box1[1], box2[1])
    c_x2 = torch.max(box1[2], box2[2])
    c_y2 = torch.max(box1[3], box2[3])
    c_area = (c_x2 - c_x1) * (c_y2 - c_y1) # C 영역 넓이


    area1 = (box1[2] - box1[0]) * (box1[3] - box1[1])
    area2 = (box2[2] - box2[0]) * (box2[3] - box2[1])


    # 합집합 넓이 다시 계산
    x1 = torch.max(box1[0], box2[0])
    y1 = torch.max(box1[1], box2[1])
    x2 = torch.min(box1[2], box2[2])
    y2 = torch.min(box1[3], box2[3])
    inter = torch.clamp(x2 - x1, min=0) * torch.clamp(y2 - y1, min=0)
    union = area1 + area2 - inter


    # GIoU = IoU - (C 안에서 합집합이 차지하지 않는 빈 공간의 비율)
    giou = iou - (c_area - union) / (c_area + 1e-7)
    return giou




def compute_ciou(box1, box2):
    """CIoU 계산"""
    iou = compute_iou(box1, box2)


    # 두 박스 중심점 사이 거리의 제곱 (d2)
    cx1 = (box1[0] + box1[2]) / 2
    cy1 = (box1[1] + box1[3]) / 2
    cx2 = (box2[0] + box2[2]) / 2
    cy2 = (box2[1] + box2[3]) / 2
    d2 = (cx1 - cx2) ** 2 + (cy1 - cy2) ** 2


    # 두 박스를 감싸는 영역 C의 대각선 길이 제곱 (c2)
    c_x1 = torch.min(box1[0], box2[0])
    c_y1 = torch.min(box1[1], box2[1])
    c_x2 = torch.max(box1[2], box2[2])
    c_y2 = torch.max(box1[3], box2[3])
    c2 = (c_x2 - c_x1) ** 2 + (c_y2 - c_y1) ** 2


    # 두 박스의 종횡비(가로세로 비율) 차이를 나타내는 항 v
    w1 = box1[2] - box1[0]
    h1 = box1[3] - box1[1]
    w2 = box2[2] - box2[0]
    h2 = box2[3] - box2[1]
    v = (4 / math.pi ** 2) * (
        torch.atan(w2 / (h2 + 1e-7)) - torch.atan(w1 / (h1 + 1e-7))
    ) ** 2


    with torch.no_grad(): # alpha는 가중치이므로 미분 추적 제외
        alpha = v / (1 - iou + v + 1e-7)


    # CIoU = IoU - 중심거리 패널티 - 종횡비 패널티
    ciou = iou - d2 / (c2 + 1e-7) - alpha * v
    return ciou




# -- 시나리오 비교 --
# GT는 (100, 100)~(200, 200)의 100x100 정사각형, 중심은 (150, 150)으로 고정
gt = torch.tensor([100., 100., 200., 200.]) # 정답 박스 (x1, y1, x2, y2)

# 6가지 시나리오: (시각화용 영어 제목, pred 박스, 콘솔 출력용 한국어 설명)
# title은 matplotlib에 한글 폰트가 없을 때 깨지므로 영어, desc는 콘솔 출력용이라 한국어 유지
scenarios = [
    ("Nearly accurate",
     torch.tensor([102., 98., 202., 198.]),
     "GT와 거의 일치 -> IoU 1에 근접"),

    ("Partial overlap",
     torch.tensor([120., 110., 220., 210.]),
     "대각선으로 약간 이동 -> 일반적인 학습 중 상황"),

    ("Adjacent (IoU=0)",
     torch.tensor([220., 100., 320., 200.]),
     "GT 바로 옆 (IoU=0) -> GIoU는 약한 음수로 거리 신호 전달"),

    ("Far apart (IoU=0)",
     torch.tensor([400., 400., 500., 500.]),
     "GT에서 멀리 (IoU=0) -> GIoU가 더 작음 (GIoU의 거리 민감도)"),

    ("Pred contains GT",
     torch.tensor([50., 50., 250., 250.]),
     "pred가 GT를 완전히 감쌈 -> 외접 영역 C = pred, GIoU=IoU"),

    ("Same center, diff aspect",
     torch.tensor([100., 130., 200., 170.]),
     "중심점은 같으나 가로로 납작 -> CIoU의 종횡비 패널티가 드러남"),
]

print(f"\nGT: {gt.tolist()} (100x100 정사각형, 중심 (150, 150))")
for title, pred, desc in scenarios: # 시나리오마다 IoU/GIoU/CIoU 출력
    iou = compute_iou(pred, gt).item()
    giou = compute_giou(pred, gt).item()
    ciou = compute_ciou(pred, gt).item()
    print(f"\n[{title}] {desc}")
    print(f"  Pred: {pred.tolist()}")
    print(f"  IoU={iou:.4f}, GIoU={giou:.4f}, CIoU={ciou:.4f}")


# -- 시각화 --
fig, axes = plt.subplots(2, 3, figsize=(15, 10)) # 6개 시나리오를 2x3 그리드로 배치
axes = axes.flatten() # 2x3을 1차원으로 펴서 zip으로 순회


for ax, (title, pred, _) in zip(axes, scenarios): # 시나리오마다 박스 그리기
    ax.set_xlim(0, 550)
    ax.set_ylim(550, 0) # y축 뒤집기 (이미지 좌표계는 위가 0)
    ax.set_aspect('equal') # 가로세로 비율 동일하게
    ax.set_title(title, fontsize=11)


    # GT Box (녹색)
    rect_gt = patches.Rectangle(
        (gt[0], gt[1]), gt[2]-gt[0], gt[3]-gt[1], # (좌상단 좌표), 너비, 높이
        linewidth=2, edgecolor='green', facecolor='green', alpha=0.3,
        label='GT')
    ax.add_patch(rect_gt) # 그래프에 사각형 추가


    # Pred Box (빨간)
    rect_pred = patches.Rectangle(
        (pred[0], pred[1]), pred[2]-pred[0], pred[3]-pred[1],
        linewidth=2, edgecolor='red', facecolor='red', alpha=0.3,
        label='Pred')
    ax.add_patch(rect_pred)


    iou_val = compute_iou(pred, gt).item()
    giou_val = compute_giou(pred, gt).item()
    ciou_val = compute_ciou(pred, gt).item()
    ax.text(275, 520, # 각 subplot 하단 중앙에 세 지표 모두 표시
            f"IoU={iou_val:.3f}  GIoU={giou_val:.3f}  CIoU={ciou_val:.3f}",
            fontsize=9, ha='center')
    ax.legend(loc='upper right', fontsize=8)


plt.tight_layout()
os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (시각화를 수업 자료와 분리)
plt.savefig('outputs/iou_comparison.png', dpi=100) # 결과를 이미지 파일로 저장
print("\n시각화 저장: outputs/iou_comparison.png")
print("\n 실습 1 완료!")
```


**실행**:
```bash
python practice_iou.py
```


---


## 실습 2: Detection 지표 계산

이 실습은 Detection 모델을 어떻게 평가하는가를 코드로 분해한다. Precision/Recall/AP/mAP가 추상적 용어가 아니라 구체적 계산임을 직접 확인한다.

### 왜 지표 계산을 직접 하나

분류는 정확도 하나로 평가되지만, Detection은 박스를 맞췄는가와 맞다고 한 것 중 진짜는 몇 퍼센트인가를 동시에 봐야 한다. 그래서 Precision(예측 중 맞은 비율)과 Recall(정답 중 찾은 비율)을 쓰고, 둘의 트레이드오프를 한 곡선(PR Curve)으로, 그 아래 면적을 한 숫자(AP)로 요약한다. 여러 클래스의 AP 평균이 mAP다. week4에서 학습 결과를 mAP로 평가하는데, 그 숫자가 어떻게 나오는지 모르면 mAP 0.45가 좋은 것인지를 해석할 수 없다. 이 실습은 가상 검출 결과로 그 계산을 직접 돌려본다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| confidence 정렬 | 예측을 conf 내림차순 정렬 | 임계값을 낮춰가며 PR 변화 추적 |
| Precision/Recall | `TP/(TP+FP)`, `TP/(TP+FN)` | 두 지표의 의미와 트레이드오프 |
| AP 계산 | 11-point interpolation | PR 곡선 아래 면적을 한 숫자로 |
| PR Curve 시각화 | matplotlib | 임계값에 따른 성능 변화를 곡선으로 |

### 핵심 포인트

- 예측을 confidence 순으로 정렬한 뒤 위에서부터 TP/FP를 누적하며 PR을 찍는 것이 핵심 절차다. 이것이 mAP 계산의 뼈대다.
- IoU 임계값(보통 0.5)으로 TP/FP를 가른다. 예측 박스가 정답과 IoU 0.5 이상이면 TP다. 실습 1의 IoU가 여기서 쓰인다.


**파일명**: `practice_metrics.py`


```python
"""
실습 2: Precision, Recall, mAP 직접 계산
목표: Detection 평가 지표의 의미를 코드로 확인한다.

torch 대신 numpy/순수 Python을 쓰는 이유:
- 평가 지표는 학습 손실과 별개로 미분이 필요 없다 (autograd 불필요).
  실습 1의 IoU/GIoU/CIoU는 backward()로 gradient를 흘려야 해서 torch가 필수였다.
- 정렬, 누적합, set 기반 매칭처럼 control flow가 많아 텐서 연산보다
  Python 자료구조가 자연스럽다.
- 박스 수가 한 자릿수 수준이라 GPU 가속이 의미 없다.
- torchmetrics, pycocotools 같은 표준 라이브러리도 mAP 계산 내부는
  대부분 CPU/numpy 연산이다.
"""
import os
import numpy as np
import matplotlib
matplotlib.use('Agg') # GUI 없이 파일로 저장하는 백엔드
import matplotlib.pyplot as plt


print("=" * 50)
print("실습 2: Detection 지표 계산")
print("=" * 50)




def compute_iou_np(box1, box2):
    """NumPy IoU 계산 (xyxy)"""
    # 교집합 영역 좌표
    x1 = max(box1[0], box2[0])
    y1 = max(box1[1], box2[1])
    x2 = min(box1[2], box2[2])
    y2 = min(box1[3], box2[3])


    inter = max(0, x2 - x1) * max(0, y2 - y1) # 교집합 넓이 (안 겹치면 0)
    area1 = (box1[2] - box1[0]) * (box1[3] - box1[1])
    area2 = (box2[2] - box2[0]) * (box2[3] - box2[1])


    return inter / (area1 + area2 - inter + 1e-7) # IoU = 교집합 / 합집합




# -- 가상 검출 결과 --
# 5개의 GT box가 있는 이미지
gt_boxes = [ # 정답 박스 목록 [x1, y1, x2, y2]
    [50, 50, 150, 150], # 객체 0
    [200, 100, 350, 250], # 객체 1
    [400, 200, 500, 350], # 객체 2
    [100, 300, 250, 450], # 객체 3
    [350, 350, 480, 480], # 객체 4
]


# 모델의 예측 결과 (confidence 순으로 정렬)
predictions = [ # 모델 예측 목록: 박스 + confidence (신뢰도)
    {"box": [55, 48, 155, 148], "conf": 0.95}, # GT0과 매칭
    {"box": [205, 105, 345, 245], "conf": 0.90}, # GT1과 매칭
    {"box": [300, 300, 400, 400], "conf": 0.85}, # 어떤 GT와도 안 맞음 (FP)
    {"box": [60, 55, 145, 145], "conf": 0.80}, # GT0과 중복 (이미 매칭됨)
    {"box": [405, 205, 495, 345], "conf": 0.70}, # GT2와 매칭
    {"box": [105, 305, 245, 445], "conf": 0.60}, # GT3과 매칭
    {"box": [10, 10, 30, 30], "conf": 0.50}, # FP (잘못된 검출)
]


print(f"\nGT 객체 수: {len(gt_boxes)}")
print(f"예측 수: {len(predictions)}")


# -- Precision-Recall 계산 --
iou_threshold = 0.5 # IoU가 이 값 이상이면 정답(TP)으로 인정
matched_gt = set() # 이미 매칭된 GT 인덱스 (한 GT에 중복 매칭 방지)
precisions = []
recalls = []


tp_list = [] # 각 예측이 TP면 1, 아니면 0
fp_list = [] # 각 예측이 FP면 1, 아니면 0


for i, pred in enumerate(predictions): # confidence 높은 순으로 예측 처리
    best_iou = 0
    best_gt_idx = -1


    for j, gt in enumerate(gt_boxes): # 모든 GT와 비교해 가장 잘 맞는 것 찾기
        iou = compute_iou_np(pred["box"], gt)
        if iou > best_iou:
            best_iou = iou
            best_gt_idx = j


    # IoU가 충분하고 그 GT가 아직 매칭 안 됐으면 TP, 아니면 FP
    if best_iou >= iou_threshold and best_gt_idx not in matched_gt:
        tp_list.append(1)
        fp_list.append(0)
        matched_gt.add(best_gt_idx) # 이 GT를 매칭됨으로 표시
        status = "TP"
    else:
        tp_list.append(0)
        fp_list.append(1)
        status = "FP"


    # 현재까지 누적된 TP/FP로 precision, recall 계산
    tp_cumsum = sum(tp_list)
    fp_cumsum = sum(fp_list)
    precision = tp_cumsum / (tp_cumsum + fp_cumsum) # 예측한 것 중 맞은 비율
    recall = tp_cumsum / len(gt_boxes) # 전체 정답 중 찾아낸 비율


    precisions.append(precision)
    recalls.append(recall)


    print(f"예측 {i}: conf={pred['conf']:.2f}, IoU={best_iou:.3f}, "
          f"{status}, Precision={precision:.3f}, Recall={recall:.3f}")


# -- AP 계산 (11-point interpolation) --
recall_levels = np.linspace(0, 1, 11) # recall 기준점 11개 (0, 0.1, ..., 1.0)
ap = 0
for r_level in recall_levels:
    # r_level 이상의 recall에서 최대 precision
    prec_at_level = [p for p, r in zip(precisions, recalls) if r >= r_level]
    if prec_at_level:
        ap += max(prec_at_level) # 각 기준점의 최대 precision을 누적


ap /= 11 # 11개 기준점의 평균이 AP
print(f"\n AP@0.5 (11-point): {ap:.4f}")


# -- PR Curve 시각화 --
fig, ax = plt.subplots(1, 1, figsize=(8, 6))
ax.plot(recalls, precisions, 'b-o', linewidth=2, markersize=8) # recall-precision 곡선
ax.set_xlabel('Recall', fontsize=12)
ax.set_ylabel('Precision', fontsize=12)
ax.set_title(f'Precision-Recall Curve (AP@0.5 = {ap:.3f})', fontsize=14)
ax.set_xlim(0, 1.05)
ax.set_ylim(0, 1.05)
ax.grid(True, alpha=0.3)
ax.fill_between(recalls, precisions, alpha=0.2) # 곡선 아래 영역 (넓을수록 AP 큼)


plt.tight_layout()
os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (시각화를 수업 자료와 분리)
plt.savefig('outputs/pr_curve.png', dpi=100) # 결과 이미지 저장
print("PR Curve 저장: outputs/pr_curve.png")
print("\n 실습 2 완료!")
```


**실행**:
```bash
python practice_metrics.py
```


---


## 실습 3: NMS (Non-Maximum Suppression) 구현

모델은 같은 객체를 여러 박스로 중복 검출한다. NMS는 그 중복을 정리해 객체당 하나만 남기는 후처리다. 이 실습은 그 알고리즘을 직접 구현한다.

### 왜 NMS가 필요한가

YOLO는 그리드 셀마다 박스를 예측하므로 한 객체 주변에서 비슷한 박스가 여러 개 나온다. 그대로 두면 같은 차 한 대에 박스가 다섯 개 그려진다. NMS는 가장 confidence가 높은 박스를 남기고, 그것과 많이 겹치는(IoU가 큰) 나머지는 제거하는 과정을 반복한다. NMS는 거의 모든 detector의 마지막 단계이고, week6의 C++ 추론 파이프라인에서도 직접 구현한다. 여기서 원리를 손으로 짜두면 그것이 그대로 이어진다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 정렬 | confidence 내림차순 | 가장 확신하는 박스부터 처리 |
| 억제 | `IoU > 임계값` 제거 | 최고 박스와 많이 겹치는 중복 제거 |
| 반복 | 남은 박스에 재적용 | 객체당 하나만 남을 때까지 |
| 시각화 | NMS 전/후 비교 | 중복이 실제로 줄어드는지 확인 |

### 핵심 포인트

- IoU 임계값이 핵심 손잡이다. 너무 낮으면 가까이 붙은 다른 객체까지 지우고, 너무 높으면 중복이 안 지워진다.
- 여기서도 실습 1의 IoU 계산이 재사용된다. IoU -> 평가 지표(실습 2) -> NMS(실습 3)로 한 개념이 계속 쓰이는 흐름이다.


**파일명**: `practice_nms.py`


```python
"""
실습 3: NMS 직접 구현
목표: NMS가 어떻게 중복 검출을 제거하는지 직접 구현하여 이해한다.
"""
import os
import numpy as np
import matplotlib
matplotlib.use('Agg') # GUI 없이 파일로 저장하는 백엔드
import matplotlib.pyplot as plt
import matplotlib.patches as patches # 사각형 그리기용


print("=" * 50)
print("실습 3: NMS (Non-Maximum Suppression)")
print("=" * 50)




def nms(boxes, scores, iou_threshold=0.5):
    """
    NMS 구현


    Parameters:
        boxes: np.array, shape [N, 4] (x1, y1, x2, y2)
        scores: np.array, shape [N]
        iou_threshold: float


    Returns:
        keep: 유지할 BBox의 인덱스 리스트
    """
    x1 = boxes[:, 0] # 모든 박스의 좌상단 x 좌표
    y1 = boxes[:, 1]
    x2 = boxes[:, 2]
    y2 = boxes[:, 3]
    areas = (x2 - x1) * (y2 - y1) # 각 박스의 넓이


    # confidence 내림차순 정렬
    order = scores.argsort()[::-1] # 점수 높은 순서의 인덱스 배열
    keep = [] # 최종적으로 유지할 박스 인덱스


    while order.size > 0:
        i = order[0] # 남은 것 중 점수가 가장 높은 박스
        keep.append(i) # 최고 점수 박스는 무조건 유지


        if order.size == 1:
            break


        # 나머지 BBox와 IoU 계산
        xx1 = np.maximum(x1[i], x1[order[1:]])
        yy1 = np.maximum(y1[i], y1[order[1:]])
        xx2 = np.minimum(x2[i], x2[order[1:]])
        yy2 = np.minimum(y2[i], y2[order[1:]])


        inter = np.maximum(0, xx2 - xx1) * np.maximum(0, yy2 - yy1) # 교집합 넓이
        iou = inter / (areas[i] + areas[order[1:]] - inter + 1e-7) # i와 나머지 박스들의 IoU


        # IoU가 threshold 미만인 것만 유지 (많이 겹치는 박스는 중복으로 보고 제거)
        remaining = np.where(iou < iou_threshold)[0]
        order = order[remaining + 1] # +1: order[1:] 기준 인덱스를 order 기준으로 보정


    return keep




# -- 테스트 데이터 --
# 같은 객체를 여러 번 검출한 상황
boxes = np.array([ # 박스 6개 [x1, y1, x2, y2]
    [100, 100, 210, 210], # BBox 0: conf=0.9
    [105, 108, 215, 215], # BBox 1: conf=0.85 (0과 중복)
    [110, 105, 220, 218], # BBox 2: conf=0.7 (0과 중복)
    [300, 300, 420, 420], # BBox 3: conf=0.95 (다른 객체)
    [305, 310, 425, 425], # BBox 4: conf=0.6 (3과 중복)
    [500, 100, 600, 200], # BBox 5: conf=0.8 (독립 객체)
], dtype=np.float32)


scores = np.array([0.9, 0.85, 0.7, 0.95, 0.6, 0.8]) # 각 박스의 confidence 점수


print(f"\nNMS 전: {len(boxes)}개 BBox")
for i, (box, score) in enumerate(zip(boxes, scores)):
    print(f"BBox {i}: {box.tolist()}, conf={score:.2f}")


# NMS 적용
keep = nms(boxes, scores, iou_threshold=0.5) # 중복 검출 박스 제거
print(f"\nNMS 후: {len(keep)}개 BBox (유지: {keep})")
for i in keep:
    print(f"BBox {i}: {boxes[i].tolist()}, conf={scores[i]:.2f}")


# -- 시각화 --
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6)) # NMS 전/후를 나란히 배치
colors = ['red', 'blue', 'green', 'orange', 'purple', 'cyan'] # 박스별 구분 색


# NMS 전
ax1.set_title("NMS 전", fontsize=14)
ax1.set_xlim(0, 700)
ax1.set_ylim(500, 0) # 이미지 좌표계 (위가 0)
for i, (box, score) in enumerate(zip(boxes, scores)): # 모든 박스 그리기
    rect = patches.Rectangle(
        (box[0], box[1]), box[2]-box[0], box[3]-box[1],
        linewidth=2, edgecolor=colors[i], facecolor=colors[i],
        alpha=0.3)
    ax1.add_patch(rect)
    ax1.text(box[0], box[1]-5, f"#{i} conf={score:.2f}",
             fontsize=9, color=colors[i])


# NMS 후
ax2.set_title("NMS 후", fontsize=14)
ax2.set_xlim(0, 700)
ax2.set_ylim(500, 0)
for i in keep: # 살아남은 박스만 그리기
    box = boxes[i]
    rect = patches.Rectangle(
        (box[0], box[1]), box[2]-box[0], box[3]-box[1],
        linewidth=3, edgecolor=colors[i], facecolor=colors[i],
        alpha=0.3)
    ax2.add_patch(rect)
    ax2.text(box[0], box[1]-5, f"#{i} conf={scores[i]:.2f}",
             fontsize=9, color=colors[i], fontweight='bold')


plt.tight_layout()
os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (시각화를 수업 자료와 분리)
plt.savefig('outputs/nms_result.png', dpi=100) # 결과 이미지 저장
print("\n시각화 저장: outputs/nms_result.png")
print("\n 실습 3 완료!")
```


**실행**:
```bash
python practice_nms.py
```


---


## 실습 4: YOLO11 구조 살펴보기

앞 세 실습이 YOLO의 구성 요소(IoU, 지표, NMS)를 손으로 짰다면, 이 실습은 실제 Ultralytics YOLO11 모델을 로드해 그 구조를 눈으로 확인한다.

### 왜 구조를 들여다보나

이론으로 본 Backbone/Neck/Head, C3k2/C2PSA 같은 블록이 실제 모델에 정말 들어 있는지 출력으로 확인하는 단계다. 코드를 한 줄씩 짜는 것이 아니라, 모델이 어떤 레이어들로 쌓여 있고 크기별(n/s/m/l/x)로 성능과 파라미터가 어떻게 다른지 본다. week4에서 이 모델을 실제로 학습시키기 직전에, 내가 학습시킬 것이 어떻게 생겼는지를 먼저 파악하는 워밍업이다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 모델 로드 | `YOLO("yolo11n.pt")` | 사전학습 모델 불러오기 |
| 구조 출력 | `model.info()`, 레이어 순회 | C3k2/C2PSA가 실제로 보이는지 확인 |
| 크기 비교 | n/s/m/l/x의 mAP와 파라미터 | 정확도-속도 트레이드오프 |

### 핵심 포인트

- README의 이론(C3k2, C2PSA가 YOLO11 신규 블록)이 실제 레이어 목록에 나타나는지 대조하는 것이 포인트다.
- 모델 크기 선택은 정확도 대 속도/메모리의 트레이드오프다. week5-6에서 경량화/배포할 때 이 선택이 다시 등장한다.


**파일명**: `practice_yolo11_structure.py`


```python
"""
실습 4: YOLO11 모델 구조 분석
목표: Ultralytics YOLO11 모델의 실제 구조를 확인하고,
      YOLOv8과의 차이(C2f -> C3k2, C2PSA 추가)를 눈으로 확인한다.
"""
from ultralytics import YOLO # Ultralytics YOLO 라이브러리


print("=" * 50)
print("실습 4: YOLO11 구조 분석")
print("=" * 50)


# -- 모델 로드 --
model = YOLO('yolo11n.pt') # nano 모델 (가장 작음, 없으면 자동 다운로드)


# -- 모델 정보 출력 --
print("\n[1] 모델 기본 정보")
print(f"모델명: {model.model.yaml.get('yaml_file', 'yolo11n')}")
print(f"총 파라미터: {sum(p.numel() for p in model.model.parameters()):,}") # 전체 가중치 개수
print(f"학습 가능 파라미터: {sum(p.numel() for p in model.model.parameters() if p.requires_grad):,}") # 그중 학습되는 것


# -- 레이어별 구조 (C3k2, C2PSA가 보이는지 확인) --
print("\n[2] 모델 레이어 구조")
for i, layer in enumerate(model.model.model): # 각 레이어 이름과 파라미터 수 출력
    params = sum(p.numel() for p in layer.parameters())
    print(f"Layer {i:2d}: {layer.__class__.__name__:20s} → 파라미터: {params:>8,}")


# -- 모델 크기 비교 (COCO val2017, mAP@0.5:0.95) --
print("\n[3] YOLO11 모델 크기 비교")
model_sizes = { # 모델 크기별 파라미터/연산량(GFLOPs)/정확도(mAP)
    'yolo11n': '2.6M params, 6.5 GFLOPs, mAP 39.5',
    'yolo11s': '9.4M params, 21.5 GFLOPs, mAP 47.0',
    'yolo11m': '20.1M params, 68.0 GFLOPs, mAP 51.5',
    'yolo11l': '25.3M params, 86.9 GFLOPs, mAP 53.4',
    'yolo11x': '56.9M params, 194.9 GFLOPs, mAP 54.7',
}
for name, info in model_sizes.items():
    print(f"{name}: {info}")


print("\n 실습 4 완료!")
```


**실행**:
```bash
pip install ultralytics
python practice_yolo11_structure.py
```


---


## 실습 체크리스트


- [ ] IoU, GIoU, CIoU 직접 구현 및 결과 비교
- [ ] Precision, Recall, AP 수동 계산
- [ ] PR Curve 시각화 확인
- [ ] NMS 직접 구현 및 결과 확인
- [ ] YOLO11 모델 구조 분석 (C3k2, C2PSA 확인)


---


## 참고 자료


- [YOLO 원본 논문 (Redmon et al., 2015)](https://arxiv.org/abs/1506.02640)
- [YOLOv3 논문](https://arxiv.org/abs/1804.02767)
- [Ultralytics YOLO11 문서](https://docs.ultralytics.com/models/yolo11/)
- [CIoU 논문 (Zheng et al., 2020)](https://arxiv.org/abs/1911.08287)
- [mAP 계산 설명 (Jonathan Hui)](https://jonathan-hui.medium.com/map-mean-average-precision-for-object-detection-45c121a31173)


---


이전: [Week 2 PRACTICE](../week2/PRACTICE.md)
다음: [Week 4 PRACTICE](../week4/PRACTICE.md)
