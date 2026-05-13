# Week 3 실습: YOLO 이론 검증


> **실습 목표**: YOLO의 핵심 개념을 코드로 직접 구현하여 이해를 깊게 한다.
> **예상 시간**: 6~8시간


---


## 환경 설정


```bash
cd Studies/Phase\ 5/week3
pip install -r requirements.txt
python quiz_easy.py # 개념 퀴즈
python quiz_medium.py # 코드 퀴즈
```


---


## 실습 1: IoU 계열 Loss 직접 구현


**파일명**: `practice_iou.py`


```python
"""
실습 1: IoU, GIoU, DIoU, CIoU 직접 구현
목표: 각 IoU 변형이 어떤 상황에서 어떤 gradient를 제공하는지 이해한다.
"""
import torch
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np


print("=" * 50)
print("실습 1: IoU 계열 Loss 비교")
print("=" * 50)




def compute_iou(box1, box2):
    """기본 IoU 계산 (xyxy 형식)"""
    x1 = torch.max(box1[0], box2[0])
    y1 = torch.max(box1[1], box2[1])
    x2 = torch.min(box1[2], box2[2])
    y2 = torch.min(box1[3], box2[3])


    inter = torch.clamp(x2 - x1, min=0) * torch.clamp(y2 - y1, min=0)
    area1 = (box1[2] - box1[0]) * (box1[3] - box1[1])
    area2 = (box2[2] - box2[0]) * (box2[3] - box2[1])
    union = area1 + area2 - inter


    return inter / (union + 1e-7)




def compute_giou(box1, box2):
    """GIoU 계산"""
    iou = compute_iou(box1, box2)


    # 포함하는 최소 영역 C
    c_x1 = torch.min(box1[0], box2[0])
    c_y1 = torch.min(box1[1], box2[1])
    c_x2 = torch.max(box1[2], box2[2])
    c_y2 = torch.max(box1[3], box2[3])
    c_area = (c_x2 - c_x1) * (c_y2 - c_y1)


    area1 = (box1[2] - box1[0]) * (box1[3] - box1[1])
    area2 = (box2[2] - box2[0]) * (box2[3] - box2[1])


    x1 = torch.max(box1[0], box2[0])
    y1 = torch.max(box1[1], box2[1])
    x2 = torch.min(box1[2], box2[2])
    y2 = torch.min(box1[3], box2[3])
    inter = torch.clamp(x2 - x1, min=0) * torch.clamp(y2 - y1, min=0)
    union = area1 + area2 - inter


    giou = iou - (c_area - union) / (c_area + 1e-7)
    return giou




def compute_ciou(box1, box2):
    """CIoU 계산"""
    import math


    iou = compute_iou(box1, box2)


    # 중심점 거리
    cx1 = (box1[0] + box1[2]) / 2
    cy1 = (box1[1] + box1[3]) / 2
    cx2 = (box2[0] + box2[2]) / 2
    cy2 = (box2[1] + box2[3]) / 2
    d2 = (cx1 - cx2) ** 2 + (cy1 - cy2) ** 2


    # 포함 영역 대각선
    c_x1 = torch.min(box1[0], box2[0])
    c_y1 = torch.min(box1[1], box2[1])
    c_x2 = torch.max(box1[2], box2[2])
    c_y2 = torch.max(box1[3], box2[3])
    c2 = (c_x2 - c_x1) ** 2 + (c_y2 - c_y1) ** 2


    # 종횡비
    w1 = box1[2] - box1[0]
    h1 = box1[3] - box1[1]
    w2 = box2[2] - box2[0]
    h2 = box2[3] - box2[1]
    v = (4 / math.pi ** 2) * (
        torch.atan(w2 / (h2 + 1e-7)) - torch.atan(w1 / (h1 + 1e-7))
    ) ** 2


    with torch.no_grad():
        alpha = v / (1 - iou + v + 1e-7)


    ciou = iou - d2 / (c2 + 1e-7) - alpha * v
    return ciou




# -- 시나리오 비교 --
print("\n[시나리오 1] BBox가 겹치는 경우")
gt = torch.tensor([100., 100., 200., 200.])
pred1 = torch.tensor([120., 110., 220., 210.])
print(f"GT: {gt.tolist()}")
print(f"Pred: {pred1.tolist()}")
print(f"IoU: {compute_iou(pred1, gt):.4f}")
print(f"GIoU: {compute_giou(pred1, gt):.4f}")
print(f"CIoU: {compute_ciou(pred1, gt):.4f}")


print("\n[시나리오 2] BBox가 떨어진 경우")
pred2 = torch.tensor([300., 300., 400., 400.])
print(f"GT: {gt.tolist()}")
print(f"Pred: {pred2.tolist()}")
print(f"IoU: {compute_iou(pred2, gt):.4f}")
print(f"GIoU: {compute_giou(pred2, gt):.4f}")
print(f"CIoU: {compute_ciou(pred2, gt):.4f}")


print("\n[시나리오 3] 거의 정확한 경우")
pred3 = torch.tensor([102., 98., 202., 198.])
print(f"GT: {gt.tolist()}")
print(f"Pred: {pred3.tolist()}")
print(f"IoU: {compute_iou(pred3, gt):.4f}")
print(f"GIoU: {compute_giou(pred3, gt):.4f}")
print(f"CIoU: {compute_ciou(pred3, gt):.4f}")


# -- 시각화 --
fig, axes = plt.subplots(1, 3, figsize=(15, 5))
scenarios = [
    ("겹치는 경우", pred1),
    ("떨어진 경우", pred2),
    ("거의 정확", pred3),
]


for ax, (title, pred) in zip(axes, scenarios):
    ax.set_xlim(0, 500)
    ax.set_ylim(500, 0)
    ax.set_aspect('equal')
    ax.set_title(title, fontsize=12)


    # GT Box (녹색)
    rect_gt = patches.Rectangle(
        (gt[0], gt[1]), gt[2]-gt[0], gt[3]-gt[1],
        linewidth=2, edgecolor='green', facecolor='green', alpha=0.3,
        label='GT')
    ax.add_patch(rect_gt)


    # Pred Box (빨간)
    rect_pred = patches.Rectangle(
        (pred[0], pred[1]), pred[2]-pred[0], pred[3]-pred[1],
        linewidth=2, edgecolor='red', facecolor='red', alpha=0.3,
        label='Pred')
    ax.add_patch(rect_pred)


    iou_val = compute_iou(pred, gt).item()
    ciou_val = compute_ciou(pred, gt).item()
    ax.text(250, 450, f"IoU={iou_val:.3f}\nCIoU={ciou_val:.3f}",
            fontsize=10, ha='center')
    ax.legend(loc='upper right')


plt.tight_layout()
plt.savefig('iou_comparison.png', dpi=100)
print("\n시각화 저장: iou_comparison.png")
print("\n 실습 1 완료!")
```


**실행**:
```bash
python practice_iou.py
```


---


## 실습 2: Detection 지표 계산


**파일명**: `practice_metrics.py`


```python
"""
실습 2: Precision, Recall, mAP 직접 계산
목표: Detection 평가 지표의 의미를 코드로 확인한다.
"""
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


print("=" * 50)
print("실습 2: Detection 지표 계산")
print("=" * 50)




def compute_iou_np(box1, box2):
    """NumPy IoU 계산 (xyxy)"""
    x1 = max(box1[0], box2[0])
    y1 = max(box1[1], box2[1])
    x2 = min(box1[2], box2[2])
    y2 = min(box1[3], box2[3])


    inter = max(0, x2 - x1) * max(0, y2 - y1)
    area1 = (box1[2] - box1[0]) * (box1[3] - box1[1])
    area2 = (box2[2] - box2[0]) * (box2[3] - box2[1])


    return inter / (area1 + area2 - inter + 1e-7)




# -- 가상 검출 결과 --
# 10개의 GT box가 있는 이미지
gt_boxes = [
    [50, 50, 150, 150], # 객체 0
    [200, 100, 350, 250], # 객체 1
    [400, 200, 500, 350], # 객체 2
    [100, 300, 250, 450], # 객체 3
    [350, 350, 480, 480], # 객체 4
]


# 모델의 예측 결과 (confidence 순으로 정렬)
predictions = [
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
iou_threshold = 0.5
matched_gt = set()
precisions = []
recalls = []


tp_list = []
fp_list = []


for i, pred in enumerate(predictions):
    best_iou = 0
    best_gt_idx = -1


    for j, gt in enumerate(gt_boxes):
        iou = compute_iou_np(pred["box"], gt)
        if iou > best_iou:
            best_iou = iou
            best_gt_idx = j


    if best_iou >= iou_threshold and best_gt_idx not in matched_gt:
        tp_list.append(1)
        fp_list.append(0)
        matched_gt.add(best_gt_idx)
        status = "TP"
    else:
        tp_list.append(0)
        fp_list.append(1)
        status = "FP"


    tp_cumsum = sum(tp_list)
    fp_cumsum = sum(fp_list)
    precision = tp_cumsum / (tp_cumsum + fp_cumsum)
    recall = tp_cumsum / len(gt_boxes)


    precisions.append(precision)
    recalls.append(recall)


    print(f"예측 {i}: conf={pred['conf']:.2f}, IoU={best_iou:.3f}, "
          f"{status}, Precision={precision:.3f}, Recall={recall:.3f}")


# -- AP 계산 (11-point interpolation) --
recall_levels = np.linspace(0, 1, 11)
ap = 0
for r_level in recall_levels:
    # r_level 이상의 recall에서 최대 precision
    prec_at_level = [p for p, r in zip(precisions, recalls) if r >= r_level]
    if prec_at_level:
        ap += max(prec_at_level)


ap /= 11
print(f"\n AP@0.5 (11-point): {ap:.4f}")


# -- PR Curve 시각화 --
fig, ax = plt.subplots(1, 1, figsize=(8, 6))
ax.plot(recalls, precisions, 'b-o', linewidth=2, markersize=8)
ax.set_xlabel('Recall', fontsize=12)
ax.set_ylabel('Precision', fontsize=12)
ax.set_title(f'Precision-Recall Curve (AP@0.5 = {ap:.3f})', fontsize=14)
ax.set_xlim(0, 1.05)
ax.set_ylim(0, 1.05)
ax.grid(True, alpha=0.3)
ax.fill_between(recalls, precisions, alpha=0.2)


plt.tight_layout()
plt.savefig('pr_curve.png', dpi=100)
print("PR Curve 저장: pr_curve.png")
print("\n 실습 2 완료!")
```


**실행**:
```bash
python practice_metrics.py
```


---


## 실습 3: NMS (Non-Maximum Suppression) 구현


**파일명**: `practice_nms.py`


```python
"""
실습 3: NMS 직접 구현
목표: NMS가 어떻게 중복 검출을 제거하는지 직접 구현하여 이해한다.
"""
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches


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
    x1 = boxes[:, 0]
    y1 = boxes[:, 1]
    x2 = boxes[:, 2]
    y2 = boxes[:, 3]
    areas = (x2 - x1) * (y2 - y1)


    # confidence 내림차순 정렬
    order = scores.argsort()[::-1]
    keep = []


    while order.size > 0:
        i = order[0]
        keep.append(i)


        if order.size == 1:
            break


        # 나머지 BBox와 IoU 계산
        xx1 = np.maximum(x1[i], x1[order[1:]])
        yy1 = np.maximum(y1[i], y1[order[1:]])
        xx2 = np.minimum(x2[i], x2[order[1:]])
        yy2 = np.minimum(y2[i], y2[order[1:]])


        inter = np.maximum(0, xx2 - xx1) * np.maximum(0, yy2 - yy1)
        iou = inter / (areas[i] + areas[order[1:]] - inter + 1e-7)


        # IoU가 threshold 미만인 것만 유지
        remaining = np.where(iou < iou_threshold)[0]
        order = order[remaining + 1]


    return keep




# -- 테스트 데이터 --
# 같은 객체를 여러 번 검출한 상황
boxes = np.array([
    [100, 100, 210, 210], # BBox 0: conf=0.9
    [105, 108, 215, 215], # BBox 1: conf=0.85 (0과 중복)
    [110, 105, 220, 218], # BBox 2: conf=0.7 (0과 중복)
    [300, 300, 420, 420], # BBox 3: conf=0.95 (다른 객체)
    [305, 310, 425, 425], # BBox 4: conf=0.6 (3과 중복)
    [500, 100, 600, 200], # BBox 5: conf=0.8 (독립 객체)
], dtype=np.float32)


scores = np.array([0.9, 0.85, 0.7, 0.95, 0.6, 0.8])


print(f"\nNMS 전: {len(boxes)}개 BBox")
for i, (box, score) in enumerate(zip(boxes, scores)):
    print(f"BBox {i}: {box.tolist()}, conf={score:.2f}")


# NMS 적용
keep = nms(boxes, scores, iou_threshold=0.5)
print(f"\nNMS 후: {len(keep)}개 BBox (유지: {keep})")
for i in keep:
    print(f"BBox {i}: {boxes[i].tolist()}, conf={scores[i]:.2f}")


# -- 시각화 --
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
colors = ['red', 'blue', 'green', 'orange', 'purple', 'cyan']


# NMS 전
ax1.set_title("NMS 전", fontsize=14)
ax1.set_xlim(0, 700)
ax1.set_ylim(500, 0)
for i, (box, score) in enumerate(zip(boxes, scores)):
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
for i in keep:
    box = boxes[i]
    rect = patches.Rectangle(
        (box[0], box[1]), box[2]-box[0], box[3]-box[1],
        linewidth=3, edgecolor=colors[i], facecolor=colors[i],
        alpha=0.3)
    ax2.add_patch(rect)
    ax2.text(box[0], box[1]-5, f"#{i} conf={scores[i]:.2f}",
             fontsize=9, color=colors[i], fontweight='bold')


plt.tight_layout()
plt.savefig('nms_result.png', dpi=100)
print("\n시각화 저장: nms_result.png")
print("\n 실습 3 완료!")
```


**실행**:
```bash
python practice_nms.py
```


---


## 실습 4: YOLO11 구조 살펴보기


**파일명**: `practice_yolo11_structure.py`


```python
"""
실습 4: YOLO11 모델 구조 분석
목표: Ultralytics YOLO11 모델의 실제 구조를 확인하고,
      YOLOv8과의 차이(C2f -> C3k2, C2PSA 추가)를 눈으로 확인한다.
"""
from ultralytics import YOLO


print("=" * 50)
print("실습 4: YOLO11 구조 분석")
print("=" * 50)


# -- 모델 로드 --
model = YOLO('yolo11n.pt') # nano 모델 (가장 작음)


# -- 모델 정보 출력 --
print("\n[1] 모델 기본 정보")
print(f"모델명: {model.model.yaml.get('yaml_file', 'yolo11n')}")
print(f"총 파라미터: {sum(p.numel() for p in model.model.parameters()):,}")
print(f"학습 가능 파라미터: {sum(p.numel() for p in model.model.parameters() if p.requires_grad):,}")


# -- 레이어별 구조 (C3k2, C2PSA가 보이는지 확인) --
print("\n[2] 모델 레이어 구조")
for i, layer in enumerate(model.model.model):
    params = sum(p.numel() for p in layer.parameters())
    print(f"Layer {i:2d}: {layer.__class__.__name__:20s} → 파라미터: {params:>8,}")


# -- 모델 크기 비교 (COCO val2017, mAP@0.5:0.95) --
print("\n[3] YOLO11 모델 크기 비교")
model_sizes = {
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
