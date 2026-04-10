# Week 4 실습: YOLOv8 학습 파이프라인

> 🎯 **실습 목표**: YOLOv8을 COCO128으로 학습하고, 커스텀 데이터셋을 준비하며, 평가 결과를 분석한다.
> ⏰ **예상 시간**: 6~8시간

---

## 🔧 환경 설정

```bash
cd Studies/Phase\ 5/week4
pip install -r requirements.txt

# Ultralytics 설치 확인
python -c "from ultralytics import YOLO; print('Ultralytics 설치 완료!')"

# 퀴즈 실행
python quiz_easy.py
python quiz_medium.py
```

---

## 📝 실습 1: YOLOv8 기본 추론

**파일명**: `practice_inference.py`

```python
"""
실습 1: YOLOv8 기본 추론
목표: Pretrained YOLOv8으로 이미지/영상 추론을 수행한다.
"""
from ultralytics import YOLO
import cv2
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

print("=" * 50)
print("실습 1: YOLOv8 기본 추론")
print("=" * 50)

# ── 모델 로드 ──
model = YOLO('yolov8n.pt')  # 자동 다운로드
print(f"\n모델 로드 완료: yolov8n.pt")

# ── 테스트 이미지 생성 (실제로는 이미지 경로 사용) ──
# 실제 이미지가 있으면: results = model('image.jpg')
# 여기서는 Ultralytics 내장 이미지 사용
results = model('https://ultralytics.com/images/bus.jpg')

# ── 결과 분석 ──
print("\n[1] 검출 결과 분석")
for result in results:
    boxes = result.boxes
    print(f"  검출된 객체 수: {len(boxes)}")

    for i, box in enumerate(boxes):
        cls_id = int(box.cls)
        cls_name = model.names[cls_id]
        conf = box.conf.item()
        xyxy = box.xyxy[0].tolist()

        print(f"  객체 {i}: {cls_name} (conf={conf:.3f})")
        print(f"          BBox: [{xyxy[0]:.1f}, {xyxy[1]:.1f}, "
              f"{xyxy[2]:.1f}, {xyxy[3]:.1f}]")

    # 결과 시각화 저장
    annotated = result.plot()
    cv2.imwrite('inference_result.jpg', annotated)
    print("\n결과 저장: inference_result.jpg")

# ── Confidence 임계값 변경 ──
print("\n[2] Confidence 임계값 비교")
for conf_thresh in [0.25, 0.50, 0.75]:
    results = model('https://ultralytics.com/images/bus.jpg',
                     conf=conf_thresh, verbose=False)
    n_detections = len(results[0].boxes)
    print(f"  conf={conf_thresh:.2f}: {n_detections}개 검출")

print("\n✅ 실습 1 완료!")
```

**실행**:
```bash
python practice_inference.py
```

---

## 📝 실습 2: COCO128 학습

**파일명**: `practice_train_coco128.py`

```python
"""
실습 2: COCO128로 YOLOv8 학습
목표: 기본 학습을 수행하고 결과를 분석한다.
"""
from ultralytics import YOLO
import os

print("=" * 50)
print("실습 2: COCO128 학습")
print("=" * 50)

# ── 모델 로드 ──
model = YOLO('yolov8n.pt')

# ── 학습 ──
print("\n학습 시작...")
results = model.train(
    data='coco128.yaml',        # 내장 데이터셋 (자동 다운로드)
    epochs=30,                   # 빠른 실험을 위해 30 에폭
    imgsz=640,                   # 입력 크기
    batch=16,                    # 배치 크기 (GPU 메모리에 맞게 조절)
    device=0,                    # GPU (CPU면 'cpu')
    project='runs/detect',       # 결과 저장 경로
    name='coco128_baseline',     # 실험 이름
    patience=10,                 # Early stopping
    save=True,                   # 체크포인트 저장
    plots=True,                  # 결과 시각화
    verbose=True,
)

# ── 결과 확인 ──
print("\n[학습 결과]")
result_dir = 'runs/detect/coco128_baseline'

# 생성된 파일 확인
if os.path.exists(result_dir):
    files = os.listdir(result_dir)
    print(f"  결과 디렉토리: {result_dir}")
    print(f"  생성된 파일: {files}")

# ── 평가 ──
print("\n[모델 평가]")
best_model = YOLO(f'{result_dir}/weights/best.pt')
metrics = best_model.val()

print(f"\n  mAP@0.5:      {metrics.box.map50:.4f}")
print(f"  mAP@0.5:0.95: {metrics.box.map:.4f}")
print(f"  Precision:     {metrics.box.mp:.4f}")
print(f"  Recall:        {metrics.box.mr:.4f}")

print("\n결과 파일 확인:")
print(f"  - {result_dir}/results.png (학습 커브)")
print(f"  - {result_dir}/confusion_matrix.png (혼동 행렬)")
print(f"  - {result_dir}/PR_curve.png (PR 커브)")

print("\n✅ 실습 2 완료!")
```

**실행**:
```bash
python practice_train_coco128.py
```

**예상 결과**:
- COCO128 (30 에폭): mAP@0.5 약 0.5~0.6

---

## 📝 실습 3: Hyperparameter 비교 실험

**파일명**: `practice_hyperparameter.py`

```python
"""
실습 3: Hyperparameter 비교 실험
목표: 주요 Hyperparameter를 변경하며 성능 차이를 비교한다.
"""
from ultralytics import YOLO
import json
import os

print("=" * 50)
print("실습 3: Hyperparameter 비교 실험")
print("=" * 50)

# ── 실험 설정 ──
experiments = [
    {
        "name": "exp_lr_low",
        "desc": "낮은 학습률 (lr0=0.001)",
        "params": {"lr0": 0.001, "epochs": 30}
    },
    {
        "name": "exp_lr_high",
        "desc": "높은 학습률 (lr0=0.02)",
        "params": {"lr0": 0.02, "epochs": 30}
    },
    {
        "name": "exp_no_mosaic",
        "desc": "Mosaic 비활성화",
        "params": {"mosaic": 0.0, "epochs": 30}
    },
    {
        "name": "exp_heavy_aug",
        "desc": "강한 Augmentation",
        "params": {"mosaic": 1.0, "mixup": 0.15, "degrees": 10.0,
                   "scale": 0.9, "epochs": 30}
    },
]

results_summary = []

for exp in experiments:
    print(f"\n{'='*50}")
    print(f"실험: {exp['desc']}")
    print(f"{'='*50}")

    model = YOLO('yolov8n.pt')

    try:
        result = model.train(
            data='coco128.yaml',
            imgsz=640,
            batch=16,
            device=0,
            project='runs/detect',
            name=exp['name'],
            patience=10,
            plots=True,
            verbose=False,
            **exp['params']
        )

        # 평가
        best_path = f'runs/detect/{exp["name"]}/weights/best.pt'
        if os.path.exists(best_path):
            eval_model = YOLO(best_path)
            metrics = eval_model.val(verbose=False)

            results_summary.append({
                "name": exp["name"],
                "desc": exp["desc"],
                "map50": metrics.box.map50,
                "map50_95": metrics.box.map,
                "precision": metrics.box.mp,
                "recall": metrics.box.mr,
            })
    except Exception as e:
        print(f"  실험 실패: {e}")
        results_summary.append({
            "name": exp["name"],
            "desc": exp["desc"],
            "map50": 0, "map50_95": 0,
            "precision": 0, "recall": 0,
        })

# ── 결과 비교 ──
print("\n" + "=" * 70)
print("실험 결과 비교")
print("=" * 70)
print(f"{'실험':20s} │ {'mAP@0.5':>8s} │ {'mAP@0.5:0.95':>12s} │ {'Precision':>9s} │ {'Recall':>6s}")
print("─" * 70)
for r in results_summary:
    print(f"{r['desc']:20s} │ {r['map50']:>8.4f} │ {r['map50_95']:>12.4f} │ "
          f"{r['precision']:>9.4f} │ {r['recall']:>6.4f}")

# 결과 저장
with open('experiment_results.json', 'w') as f:
    json.dump(results_summary, f, indent=2, ensure_ascii=False)
print("\n결과 저장: experiment_results.json")

print("\n✅ 실습 3 완료!")
```

**실행**:
```bash
python practice_hyperparameter.py
```

---

## 📝 실습 4: 커스텀 데이터셋 준비

**파일명**: `practice_custom_dataset.py`

```python
"""
실습 4: 커스텀 데이터셋 준비 및 포맷 변환
목표: YOLO 포맷의 데이터셋 구조를 직접 만들고 검증한다.
"""
import os
import yaml
import numpy as np
import cv2
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches

print("=" * 50)
print("실습 4: 커스텀 데이터셋 준비")
print("=" * 50)


# ── 1. 디렉토리 구조 생성 ──
print("\n[1] 디렉토리 구조 생성")
base_dir = 'custom_dataset'
for split in ['train', 'val']:
    os.makedirs(f'{base_dir}/images/{split}', exist_ok=True)
    os.makedirs(f'{base_dir}/labels/{split}', exist_ok=True)
print(f"  {base_dir}/ 구조 생성 완료")


# ── 2. 가상 이미지와 라벨 생성 ──
print("\n[2] 가상 데이터 생성")
class_names = ['person', 'car', 'bicycle']
np.random.seed(42)

def create_dummy_data(split, n_images):
    """가상 이미지와 라벨 생성"""
    for i in range(n_images):
        # 가상 이미지 (640x480)
        img = np.random.randint(100, 200, (480, 640, 3), dtype=np.uint8)

        # 가상 객체 그리기
        n_objects = np.random.randint(1, 5)
        labels = []

        for _ in range(n_objects):
            cls_id = np.random.randint(0, len(class_names))
            # 랜덤 BBox (xyxy)
            x1 = np.random.randint(10, 500)
            y1 = np.random.randint(10, 350)
            w = np.random.randint(40, 150)
            h = np.random.randint(40, 150)
            x2 = min(x1 + w, 639)
            y2 = min(y1 + h, 479)

            # 이미지에 사각형 그리기
            colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
            cv2.rectangle(img, (x1, y1), (x2, y2), colors[cls_id], 2)

            # YOLO 포맷으로 변환
            x_center = (x1 + x2) / 2 / 640
            y_center = (y1 + y2) / 2 / 480
            width = (x2 - x1) / 640
            height = (y2 - y1) / 480

            labels.append(f"{cls_id} {x_center:.6f} {y_center:.6f} "
                         f"{width:.6f} {height:.6f}")

        # 저장
        img_name = f'img_{i:04d}.jpg'
        cv2.imwrite(f'{base_dir}/images/{split}/{img_name}', img)

        label_name = f'img_{i:04d}.txt'
        with open(f'{base_dir}/labels/{split}/{label_name}', 'w') as f:
            f.write('\n'.join(labels))

    print(f"  {split}: {n_images}장 생성")

create_dummy_data('train', 20)
create_dummy_data('val', 5)


# ── 3. data.yaml 생성 ──
print("\n[3] data.yaml 생성")
data_yaml = {
    'path': os.path.abspath(base_dir),
    'train': 'images/train',
    'val': 'images/val',
    'names': {i: name for i, name in enumerate(class_names)},
    'nc': len(class_names),
}

yaml_path = f'{base_dir}/data.yaml'
with open(yaml_path, 'w') as f:
    yaml.dump(data_yaml, f, default_flow_style=False)

print(f"  저장: {yaml_path}")
print(f"  내용:")
for key, value in data_yaml.items():
    print(f"    {key}: {value}")


# ── 4. 라벨 검증 ──
print("\n[4] 라벨 검증")
label_file = f'{base_dir}/labels/train/img_0000.txt'
print(f"  라벨 파일: {label_file}")
with open(label_file, 'r') as f:
    lines = f.readlines()
    for line in lines:
        parts = line.strip().split()
        cls_id = int(parts[0])
        x_c, y_c, w, h = map(float, parts[1:])
        print(f"    class={class_names[cls_id]}, "
              f"center=({x_c:.4f}, {y_c:.4f}), "
              f"size=({w:.4f}, {h:.4f})")

        # 범위 검증
        assert 0 <= x_c <= 1, f"x_center 범위 오류: {x_c}"
        assert 0 <= y_c <= 1, f"y_center 범위 오류: {y_c}"
        assert 0 < w <= 1, f"width 범위 오류: {w}"
        assert 0 < h <= 1, f"height 범위 오류: {h}"

print("  모든 좌표가 0~1 범위 내 (검증 통과)")


# ── 5. 시각화 ──
print("\n[5] 라벨 시각화")
img = cv2.imread(f'{base_dir}/images/train/img_0000.jpg')
img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
H, W = img.shape[:2]

fig, ax = plt.subplots(1, 1, figsize=(10, 7))
ax.imshow(img_rgb)

colors_plt = ['red', 'lime', 'blue']
for line in lines:
    parts = line.strip().split()
    cls_id = int(parts[0])
    x_c, y_c, w, h = map(float, parts[1:])

    # YOLO → 픽셀 좌표
    x1 = (x_c - w/2) * W
    y1 = (y_c - h/2) * H
    box_w = w * W
    box_h = h * H

    rect = patches.Rectangle(
        (x1, y1), box_w, box_h,
        linewidth=2, edgecolor=colors_plt[cls_id],
        facecolor='none')
    ax.add_patch(rect)
    ax.text(x1, y1 - 5, class_names[cls_id],
            fontsize=10, color=colors_plt[cls_id],
            fontweight='bold')

ax.set_title('YOLO Label Visualization')
plt.tight_layout()
plt.savefig('label_visualization.png', dpi=100)
print("  저장: label_visualization.png")

print("\n✅ 실습 4 완료!")
```

**실행**:
```bash
python practice_custom_dataset.py
```

---

## 📝 실습 5: 학습 결과 분석

**파일명**: `practice_analysis.py`

```python
"""
실습 5: 학습 결과 분석
목표: 학습 결과 파일을 분석하고 개선점을 도출한다.
"""
from ultralytics import YOLO
import csv
import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

print("=" * 50)
print("실습 5: 학습 결과 분석")
print("=" * 50)

# ── 1. 학습 커브 직접 그리기 ──
print("\n[1] results.csv 분석")

result_dir = 'runs/detect/coco128_baseline'
csv_path = f'{result_dir}/results.csv'

if os.path.exists(csv_path):
    epochs = []
    train_box_loss = []
    train_cls_loss = []
    val_map50 = []
    val_map50_95 = []

    with open(csv_path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            # CSV 컬럼명 확인 후 조정 필요
            epochs.append(int(row.get('epoch', row.get('                  epoch', 0))))
            train_box_loss.append(float(row.get('train/box_loss',
                                                row.get('         train/box_loss', 0))))
            train_cls_loss.append(float(row.get('train/cls_loss',
                                                row.get('         train/cls_loss', 0))))
            val_map50.append(float(row.get('metrics/mAP50(B)',
                                           row.get('       metrics/mAP50(B)', 0))))
            val_map50_95.append(float(row.get('metrics/mAP50-95(B)',
                                              row.get('    metrics/mAP50-95(B)', 0))))

    # 그래프 그리기
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))

    # Loss 커브
    axes[0].plot(epochs, train_box_loss, 'b-', label='Box Loss')
    axes[0].plot(epochs, train_cls_loss, 'r-', label='Cls Loss')
    axes[0].set_xlabel('Epoch')
    axes[0].set_ylabel('Loss')
    axes[0].set_title('Training Loss')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)

    # mAP 커브
    axes[1].plot(epochs, val_map50, 'g-', label='mAP@0.5', linewidth=2)
    axes[1].plot(epochs, val_map50_95, 'b-', label='mAP@0.5:0.95', linewidth=2)
    axes[1].set_xlabel('Epoch')
    axes[1].set_ylabel('mAP')
    axes[1].set_title('Validation mAP')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)

    # 최종 성능
    axes[2].bar(['mAP@0.5', 'mAP@0.5:0.95'],
                [val_map50[-1], val_map50_95[-1]],
                color=['green', 'blue'], alpha=0.7)
    axes[2].set_ylabel('mAP')
    axes[2].set_title('Final Performance')
    axes[2].set_ylim(0, 1)
    for i, v in enumerate([val_map50[-1], val_map50_95[-1]]):
        axes[2].text(i, v + 0.02, f'{v:.3f}', ha='center', fontsize=12)

    plt.tight_layout()
    plt.savefig('training_analysis.png', dpi=100)
    print("  학습 커브 저장: training_analysis.png")
    print(f"  최종 mAP@0.5:      {val_map50[-1]:.4f}")
    print(f"  최종 mAP@0.5:0.95: {val_map50_95[-1]:.4f}")
else:
    print(f"  결과 파일 없음: {csv_path}")
    print("  실습 2를 먼저 실행하세요.")

# ── 2. 모델별 성능 비교 ──
print("\n[2] 모델 크기별 성능 (참고)")
print(f"  {'모델':10s} │ {'파라미터':>10s} │ {'mAP@0.5:0.95':>12s} │ {'용도':12s}")
print("  " + "─" * 55)
model_info = [
    ('YOLOv8n', '3.2M', '37.3', 'Edge/실시간'),
    ('YOLOv8s', '11.2M', '44.9', '경량 서버'),
    ('YOLOv8m', '25.9M', '50.2', '균형'),
    ('YOLOv8l', '43.7M', '52.9', '높은 정확도'),
    ('YOLOv8x', '68.2M', '53.9', '최고 성능'),
]
for name, params, mAP, usage in model_info:
    print(f"  {name:10s} │ {params:>10s} │ {mAP:>12s} │ {usage:12s}")

# ── 3. 개선 제안 ──
print("\n[3] 성능 개선 체크리스트")
print("  □ 더 큰 모델 시도 (n → s → m)")
print("  □ 이미지 크기 증가 (640 → 960)")
print("  □ 학습 에폭 증가 (30 → 100)")
print("  □ Augmentation 강화 (mosaic, mixup)")
print("  □ 학습률 조절 (lr0)")
print("  □ 데이터 추가/정제")

print("\n✅ 실습 5 완료!")
```

**실행**:
```bash
python practice_analysis.py
```

---

## ✅ 실습 체크리스트

- [ ] YOLOv8 Pretrained 모델로 추론 성공
- [ ] COCO128 학습 완료 (30+ 에폭)
- [ ] 학습 결과 파일 확인 (results.png, confusion_matrix.png)
- [ ] mAP@0.5, mAP@0.5:0.95 결과 기록
- [ ] Hyperparameter 변경 실험 2개 이상 수행
- [ ] 커스텀 데이터셋 구조 생성 및 라벨 검증
- [ ] 학습 커브 분석 및 개선점 도출

---

## 🔗 참고 자료

- [Ultralytics YOLOv8 공식 문서](https://docs.ultralytics.com/)
- [COCO Dataset](https://cocodataset.org/)
- [Roboflow](https://roboflow.com/)
- [LabelImg GitHub](https://github.com/heartexlabs/labelImg)
- [YOLOv8 학습 가이드](https://docs.ultralytics.com/modes/train/)

---

이전: [Week 3 PRACTICE](../week3/PRACTICE.md)
다음: [Week 5 PRACTICE](../week5/PRACTICE.md)
