# Week 3 실습: YOLO 이론 검증


> **실습 목표**: YOLO의 핵심 개념을 코드로 직접 구현하여 이해를 깊게 한다.
> **예상 시간**: 6-8시간


---


## 환경 설정


```bash
cd Studies/Phase\ 3/week3
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
print("\n[시나리오 1] BBox가 겹치는 경우")
gt = torch.tensor([100., 100., 200., 200.]) # 정답 박스 (x1, y1, x2, y2)
pred1 = torch.tensor([120., 110., 220., 210.]) # 예측 박스 (GT와 일부 겹침)
print(f"GT: {gt.tolist()}")
print(f"Pred: {pred1.tolist()}")
print(f"IoU: {compute_iou(pred1, gt):.4f}")
print(f"GIoU: {compute_giou(pred1, gt):.4f}")
print(f"CIoU: {compute_ciou(pred1, gt):.4f}")


print("\n[시나리오 2] BBox가 떨어진 경우")
pred2 = torch.tensor([300., 300., 400., 400.]) # GT와 전혀 안 겹치는 예측 (IoU=0)
print(f"GT: {gt.tolist()}")
print(f"Pred: {pred2.tolist()}")
print(f"IoU: {compute_iou(pred2, gt):.4f}")
print(f"GIoU: {compute_giou(pred2, gt):.4f}")
print(f"CIoU: {compute_ciou(pred2, gt):.4f}")


print("\n[시나리오 3] 거의 정확한 경우")
pred3 = torch.tensor([102., 98., 202., 198.]) # GT와 거의 일치하는 예측
print(f"GT: {gt.tolist()}")
print(f"Pred: {pred3.tolist()}")
print(f"IoU: {compute_iou(pred3, gt):.4f}")
print(f"GIoU: {compute_giou(pred3, gt):.4f}")
print(f"CIoU: {compute_ciou(pred3, gt):.4f}")


# -- 시각화 --
fig, axes = plt.subplots(1, 3, figsize=(15, 5)) # 시나리오 3개를 가로로 배치
scenarios = [
    ("겹치는 경우", pred1),
    ("떨어진 경우", pred2),
    ("거의 정확", pred3),
]


for ax, (title, pred) in zip(axes, scenarios): # 시나리오마다 박스 그리기
    ax.set_xlim(0, 500)
    ax.set_ylim(500, 0) # y축 뒤집기 (이미지 좌표계는 위가 0)
    ax.set_aspect('equal') # 가로세로 비율 동일하게
    ax.set_title(title, fontsize=12)


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
    ciou_val = compute_ciou(pred, gt).item()
    ax.text(250, 450, f"IoU={iou_val:.3f}\nCIoU={ciou_val:.3f}", # 박스 아래에 수치 표시
            fontsize=10, ha='center')
    ax.legend(loc='upper right')


plt.tight_layout()
plt.savefig('iou_comparison.png', dpi=100) # 결과를 이미지 파일로 저장
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
plt.savefig('pr_curve.png', dpi=100) # 결과 이미지 저장
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
plt.savefig('nms_result.png', dpi=100) # 결과 이미지 저장
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
