# Week 4: YOLOv8 학습 (Section 5.2)

> 🎯 **이번 주 목표**: Ultralytics를 사용하여 YOLOv8을 직접 학습시키고, 커스텀 데이터셋 준비부터 모델 평가까지 전체 파이프라인을 구축한다.
> ⏰ **예상 시간**: 12시간
> 💡 **핵심 질문**: "커스텀 데이터셋으로 YOLOv8을 학습하고, mAP를 기반으로 모델을 평가할 수 있는가?"

---

## 🌟 시작하기 전에

### Week 3과의 연결

| Week 3 | → | Week 4 |
|---------|---|---------|
| YOLO 이론 이해 | → | YOLO 실전 학습 |
| CIoU, mAP 개념 | → | 실제 mAP 확인 및 분석 |
| 모델 구조 이해 | → | Hyperparameter 튜닝 |

### 이번 주 전체 흐름

```
데이터 준비 → 학습 → 평가 → 분석 → 개선

1. Ultralytics 설치 및 사용법
2. 커스텀 데이터셋 준비 (YOLO format)
3. COCO128로 기본 학습
4. Hyperparameter 튜닝
5. 모델 평가 (mAP, Confusion Matrix)
6. False Positive 분석
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. Ultralytics 사용법

Ultralytics는 YOLOv8의 공식 라이브러리로, 학습/추론/내보내기를 매우 간단하게 수행합니다.

```bash
# 설치
pip install ultralytics

# CLI로 추론
yolo detect predict model=yolov8n.pt source='image.jpg'

# CLI로 학습
yolo detect train data=coco128.yaml model=yolov8n.pt epochs=100 imgsz=640
```

```python
from ultralytics import YOLO

# ── 모델 로드 ──
model = YOLO('yolov8n.pt')         # Pretrained 모델
model = YOLO('yolov8n.yaml')       # 새 모델 (from scratch)
model = YOLO('best.pt')            # 학습된 모델

# ── 추론 ──
results = model('image.jpg')
results = model('video.mp4')
results = model('path/to/images/')  # 디렉토리

# ── 학습 ──
results = model.train(
    data='coco128.yaml',
    epochs=100,
    imgsz=640,
    batch=16,
)

# ── 평가 ──
metrics = model.val()

# ── 내보내기 ──
model.export(format='onnx')        # ONNX
model.export(format='engine')      # TensorRT
```

#### 추론 결과 다루기

```python
from ultralytics import YOLO

model = YOLO('yolov8n.pt')
results = model('image.jpg')

# 결과 접근
for result in results:
    boxes = result.boxes           # BBox 결과
    print(boxes.xyxy)              # [x1, y1, x2, y2] 형식
    print(boxes.conf)              # Confidence 점수
    print(boxes.cls)               # 클래스 인덱스
    print(boxes.xywh)              # [x_center, y_center, w, h]

    # 시각화
    annotated = result.plot()      # 결과가 그려진 이미지 (numpy)

    # 특정 클래스만 필터링
    person_mask = boxes.cls == 0   # 'person' 클래스
    person_boxes = boxes[person_mask]
```

---

### 2. YOLO 데이터셋 포맷

YOLO는 특정한 디렉토리 구조와 라벨 포맷을 요구합니다.

#### 디렉토리 구조

```
dataset/
├── images/
│   ├── train/
│   │   ├── img001.jpg
│   │   ├── img002.jpg
│   │   └── ...
│   └── val/
│       ├── img101.jpg
│       └── ...
├── labels/
│   ├── train/
│   │   ├── img001.txt    ← 이미지와 동일한 이름
│   │   ├── img002.txt
│   │   └── ...
│   └── val/
│       ├── img101.txt
│       └── ...
└── data.yaml              ← 데이터셋 설정 파일
```

#### 라벨 포맷 (YOLO format)

```
# 각 행: class_id x_center y_center width height
# 모든 좌표는 0~1로 정규화 (이미지 크기로 나눔)

# img001.txt 예시:
0 0.5 0.4 0.3 0.6      # class=0(person), 중심(0.5, 0.4), 크기(0.3, 0.6)
1 0.2 0.3 0.1 0.15     # class=1(car), 중심(0.2, 0.3), 크기(0.1, 0.15)
0 0.8 0.7 0.15 0.5     # class=0(person), 두 번째 사람
```

```
좌표 설명 (640x480 이미지 기준):

원본 BBox: x_min=100, y_min=50, x_max=300, y_max=350

YOLO 변환:
  x_center = (100 + 300) / 2 / 640 = 200 / 640 = 0.3125
  y_center = (50 + 350) / 2 / 480  = 200 / 480 = 0.4167
  width    = (300 - 100) / 640      = 200 / 640 = 0.3125
  height   = (350 - 50) / 480       = 300 / 480 = 0.6250

라벨: 0 0.3125 0.4167 0.3125 0.6250
```

#### data.yaml 설정 파일

```yaml
# data.yaml
path: /path/to/dataset        # 데이터셋 루트 경로
train: images/train            # 학습 이미지 (상대 경로)
val: images/val                # 검증 이미지

# 클래스
names:
  0: person
  1: car
  2: bicycle
  3: dog

nc: 4                          # 클래스 수 (Number of Classes)
```

---

### 3. 라벨링 도구

#### LabelImg (간단한 BBox 라벨링)

```bash
pip install labelImg
labelImg                        # GUI 실행
```

```
LabelImg 사용법:
1. Open Dir → 이미지 폴더 선택
2. Change Save Dir → 라벨 저장 폴더 선택
3. 저장 포맷: YOLO 선택
4. 'W' 키: BBox 그리기
5. Ctrl+S: 저장
6. 'D' 키: 다음 이미지
```

#### Roboflow (온라인 라벨링 + 버전 관리)

```
Roboflow 장점:
  - 웹 기반 라벨링 도구
  - 팀 협업 가능
  - Augmentation 자동 적용
  - YOLO, COCO, VOC 포맷 자동 변환
  - 데이터셋 버전 관리

사용 흐름:
  1. roboflow.com에서 프로젝트 생성
  2. 이미지 업로드
  3. 라벨링 (BBox 그리기)
  4. Augmentation 설정
  5. Generate → YOLO 포맷 다운로드
```

---

### 4. 학습 실습 (COCO128)

COCO128은 COCO 데이터셋에서 128장을 추출한 미니 데이터셋으로, 빠른 실험에 적합합니다.

```python
from ultralytics import YOLO

# ── 기본 학습 ──
model = YOLO('yolov8n.pt')  # Pretrained nano 모델

results = model.train(
    data='coco128.yaml',     # 내장 데이터셋 (자동 다운로드)
    epochs=50,               # 학습 에폭 수
    imgsz=640,               # 입력 이미지 크기
    batch=16,                # 배치 크기
    device=0,                # GPU 번호 (0=첫 번째 GPU)
    project='runs/detect',   # 결과 저장 경로
    name='coco128_exp',      # 실험 이름
)
```

#### 주요 학습 인자

```python
model.train(
    # ── 기본 설정 ──
    data='data.yaml',        # 데이터셋 설정 파일
    epochs=100,              # 에폭 수 (기본 100)
    patience=50,             # Early stopping (50 에폭 동안 개선 없으면 중단)
    batch=16,                # 배치 크기 (-1: 자동 결정)
    imgsz=640,               # 입력 이미지 크기
    device=0,                # GPU 번호

    # ── 최적화 ──
    optimizer='auto',        # 옵티마이저 (SGD, Adam, AdamW, auto)
    lr0=0.01,                # 초기 학습률
    lrf=0.01,                # 최종 학습률 비율 (lr0 * lrf)
    momentum=0.937,          # SGD momentum
    weight_decay=0.0005,     # 가중치 감쇠

    # ── Augmentation ──
    hsv_h=0.015,             # HSV-Hue 변형
    hsv_s=0.7,               # HSV-Saturation 변형
    hsv_v=0.4,               # HSV-Value 변형
    degrees=0.0,             # 회전 각도
    translate=0.1,           # 이동
    scale=0.5,               # 스케일 변형
    fliplr=0.5,              # 좌우 반전 확률
    mosaic=1.0,              # Mosaic augmentation
    mixup=0.0,               # MixUp augmentation

    # ── 기타 ──
    pretrained=True,         # Pretrained 가중치 사용
    resume=False,            # 이전 학습 이어서 진행
    val=True,                # 에폭마다 검증
    save=True,               # 체크포인트 저장
    plots=True,              # 학습 결과 시각화
)
```

#### Mosaic Augmentation

```
Mosaic: 4장의 이미지를 하나로 합쳐 학습
┌──────────┬──────────┐
│  이미지1  │  이미지2  │
│          │          │
├──────────┼──────────┤
│  이미지3  │  이미지4  │
│          │          │
└──────────┴──────────┘

효과:
  - 작은 객체를 더 많이 포함
  - 문맥(context) 다양화
  - 배치 크기를 효과적으로 4배 증가
  - 마지막 10 에폭에서 자동 비활성화 (close_mosaic=10)
```

---

### 5. Hyperparameter 튜닝

#### 핵심 Hyperparameter

```
┌─────────────────────────────────────────────────┐
│  성능에 큰 영향을 미치는 Hyperparameter          │
│                                                 │
│  1. 학습률 (lr0):                               │
│     0.01 (SGD), 0.001 (Adam) 기본값             │
│     너무 크면 발산, 너무 작으면 수렴 느림         │
│                                                 │
│  2. 배치 크기 (batch):                           │
│     GPU 메모리에 맞게 설정                       │
│     클수록 안정적, 작으면 정규화 효과             │
│                                                 │
│  3. 이미지 크기 (imgsz):                         │
│     클수록 정확, 작으면 빠름                     │
│     320, 640, 1280 등 (32의 배수)               │
│                                                 │
│  4. Augmentation 강도:                           │
│     데이터 적으면 강하게, 많으면 약하게           │
│                                                 │
│  5. 모델 크기 (n/s/m/l/x):                      │
│     데이터/GPU에 맞게 선택                       │
└─────────────────────────────────────────────────┘
```

#### 튜닝 전략

```
1단계: 기본값으로 Baseline 확립
  → yolov8n.pt, epochs=50, imgsz=640, batch=16

2단계: 모델 크기 실험
  → yolov8n → yolov8s → yolov8m (데이터/GPU 여유 시)

3단계: 학습률 조절
  → lr0: 0.001, 0.005, 0.01, 0.02

4단계: Augmentation 조절
  → 데이터 적으면: mosaic=1.0, mixup=0.1
  → 데이터 많으면: mosaic=0.5, mixup=0.0

5단계: 이미지 크기
  → 640 → 960 → 1280 (GPU 메모리 허용 시)
```

---

### 6. 모델 평가

#### 기본 평가

```python
from ultralytics import YOLO

model = YOLO('runs/detect/coco128_exp/weights/best.pt')

# 검증 데이터로 평가
metrics = model.val()

# 결과 확인
print(f"mAP@0.5:     {metrics.box.map50:.4f}")
print(f"mAP@0.5:0.95:{metrics.box.map:.4f}")
print(f"Precision:    {metrics.box.mp:.4f}")
print(f"Recall:       {metrics.box.mr:.4f}")
```

#### 학습 결과 파일

```
runs/detect/coco128_exp/
├── weights/
│   ├── best.pt             ← 최고 mAP 모델
│   └── last.pt             ← 마지막 에폭 모델
├── results.csv             ← 에폭별 메트릭
├── results.png             ← 학습 커브 그래프
├── confusion_matrix.png    ← 혼동 행렬
├── P_curve.png             ← Precision 커브
├── R_curve.png             ← Recall 커브
├── PR_curve.png            ← PR 커브
├── F1_curve.png            ← F1 커브
├── val_batch0_pred.jpg     ← 검증 예측 시각화
└── val_batch0_labels.jpg   ← 검증 GT 시각화
```

#### Confusion Matrix 분석

```
Confusion Matrix (혼동 행렬):

              예측: person  car  bicycle  background
실제: person  │  85    2     1      12   │
      car     │   3   78     0      19   │
      bicycle │   1    0    65      34   │
      background│ 5    3     2       -   │

대각선: 올바른 분류 (높을수록 좋음)
비대각선: 오분류 (낮을수록 좋음)

분석 포인트:
  - person → background (12): 12개의 사람을 못 찾음 (FN)
  - background → person (5): 5번 사람이 아닌 것을 사람으로 오검출 (FP)
  - bicycle → background (34): 자전거 미검출이 많음 → 데이터 부족?
```

---

### 7. False Positive 분석

False Positive(오검출)는 모델 개선의 핵심입니다.

```
FP 유형 분석:

1. 유사 객체 오검출 (Similar Object FP)
   → 소화전을 사람으로 인식
   → 해결: 해당 유사 객체를 Hard Negative로 추가

2. 배경 오검출 (Background FP)
   → 벽면 패턴을 객체로 인식
   → 해결: Confidence threshold 높이기

3. 중복 검출 (Duplicate FP)
   → 같은 객체를 여러 번 검출
   → 해결: NMS IoU threshold 조절

4. 부분 검출 (Partial FP)
   → 객체 일부만 검출
   → 해결: Augmentation 강화 (scale, crop)
```

```python
# FP 분석 코드
from ultralytics import YOLO
import cv2

model = YOLO('best.pt')

# 검증 이미지에서 FP 찾기
results = model.val(data='data.yaml', save_json=True)

# 낮은 confidence 검출 확인 (FP일 가능성 높음)
results = model('val_images/', conf=0.25)  # 낮은 threshold
for result in results:
    for box in result.boxes:
        if box.conf < 0.5:  # 불확실한 검출
            print(f"  의심 FP: class={box.cls.item():.0f}, "
                  f"conf={box.conf.item():.3f}, "
                  f"box={box.xyxy[0].tolist()}")
```

---

## 💡 꼭 이해해야 할 핵심 개념

### 1. 학습 전체 파이프라인

```
데이터 수집        라벨링           학습 설정
──────────      ──────────      ──────────
이미지 촬영/수집   LabelImg/        data.yaml 작성
                 Roboflow로       모델 크기 선택
                 BBox 라벨링      Hyperparameter 설정
     │              │               │
     ▼              ▼               ▼
┌───────────────────────────────────────┐
│          model.train(...)             │
│                                       │
│  Epoch 1: mAP=0.10 ────→ 개선 중     │
│  Epoch 50: mAP=0.45 ────→ 수렴 중    │
│  Epoch 100: mAP=0.52 ───→ 최종       │
└───────────────────────────────────────┘
     │
     ▼
  평가 & 분석
  ──────────
  mAP 확인
  Confusion Matrix
  FP 분석
  → 데이터/하이퍼파라미터 개선 후 재학습
```

### 2. 모델 크기 선택 가이드

| 모델 | 파라미터 | mAP@0.5:0.95 | 추론 속도 | 용도 |
|------|---------|--------------|----------|------|
| YOLOv8n | 3.2M | 37.3 | 가장 빠름 | Edge 디바이스 |
| YOLOv8s | 11.2M | 44.9 | 빠름 | 실시간 응용 |
| YOLOv8m | 25.9M | 50.2 | 보통 | 균형 |
| YOLOv8l | 43.7M | 52.9 | 느림 | 높은 정확도 |
| YOLOv8x | 68.2M | 53.9 | 가장 느림 | 최고 정확도 |

### 3. Overfitting 판단법

```
정상 학습:
  train_loss ↓  val_loss ↓  val_mAP ↑

과적합 (Overfitting):
  train_loss ↓  val_loss ↑  val_mAP ↓ (또는 정체)
  → 해결: Augmentation 강화, 데이터 추가, 모델 축소

미적합 (Underfitting):
  train_loss 높음  val_mAP 낮음
  → 해결: 에폭 증가, 모델 확대, 학습률 조절
```

---

## 🔍 자체 점검 - 이해했는지 확인!

**Q1. YOLO 라벨 포맷에서 좌표가 0~1로 정규화된 이유는?**
> 이미지 크기에 상관없이 동일한 라벨을 사용하기 위해서입니다. 640x480 이미지와 1920x1080 이미지에서 같은 라벨 파일을 사용할 수 있습니다. 학습 시 이미지 크기를 변경해도 라벨 수정이 필요 없습니다.

**Q2. Mosaic Augmentation의 효과는?**
> 4장의 이미지를 하나로 합쳐 학습합니다. 이를 통해 작은 객체를 더 많이 포함하고, 다양한 문맥(context)을 제공하며, 배치 크기를 효과적으로 4배 증가시키는 효과가 있습니다. 마지막 10 에폭에서는 비활성화하여 최종 성능을 안정화합니다.

**Q3. best.pt와 last.pt의 차이는?**
> best.pt는 검증 mAP가 가장 높았던 에폭의 가중치입니다. last.pt는 마지막 에폭의 가중치입니다. 배포에는 항상 best.pt를 사용합니다.

**Q4. Confusion Matrix에서 background 행/열의 의미는?**
> background 열(예측이 background)은 FN(미검출)을 나타냅니다. background 행(실제가 background)은 FP(오검출)를 나타냅니다. 미검출이 많은 클래스는 데이터 추가가 필요할 수 있습니다.

---

## ✅ 이번 주 체크리스트

- [ ] Ultralytics 설치 및 기본 추론 성공
- [ ] YOLO 데이터셋 포맷 (디렉토리 구조, 라벨 형식) 이해
- [ ] COCO128로 YOLOv8n 학습 완료
- [ ] 학습 결과 파일 (results.png, confusion_matrix.png) 확인
- [ ] mAP@0.5, mAP@0.5:0.95 결과 해석
- [ ] Hyperparameter 변경 후 재학습 실험
- [ ] Confusion Matrix 분석
- [ ] False Positive 사례 분석

---

## 📝 핵심 요약

```
┌─────────────────────────────────────────────────────────┐
│  YOLOv8 학습 핵심 정리                                   │
│                                                         │
│  1. Ultralytics: pip install ultralytics (간편한 API)    │
│  2. 데이터셋: YOLO format (class x_c y_c w h, 정규화)   │
│  3. 학습: model.train(data, epochs, imgsz, batch)       │
│  4. 평가: mAP@0.5:0.95 + Confusion Matrix              │
│  5. 분석: FP 유형별 원인 파악 → 데이터/설정 개선        │
└─────────────────────────────────────────────────────────┘
```

---

이전: [Week 3 - YOLO 이론](../week3/README.md)
다음: [Week 5 - Depth Estimation](../week5/README.md)
