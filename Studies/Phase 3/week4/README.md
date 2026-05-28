# Week 4: YOLO11 학습 (Section 5.2)

> **이번 주 목표**: Ultralytics를 사용하여 YOLO11을 직접 학습시키고, 커스텀 데이터셋 준비부터 모델 평가까지 전체 파이프라인을 구축한다.
> **예상 시간**: 12시간
> **핵심 질문**: "커스텀 데이터셋으로 YOLO11을 학습하고, mAP를 기반으로 모델을 평가할 수 있는가?"

---

## 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | Ultralytics 사용법, 데이터셋 구조 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | Hyperparameter 튜닝, 학습 결과 분석 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | YOLO11 학습 파이프라인 구축 |

---

## 시작하기 전에

### Week 3과의 연결

Week 3은 YOLO가 "어떤 원리로" 동작하는지 이론을 봤다. Week 4는 그 이론을 실제로 돌린다 - 커스텀 데이터를 준비하고, 학습시키고, mAP로 평가하고, 결과를 분석해 다시 개선하는 한 바퀴를 직접 경험한다.

| Week 3 | -> | Week 4 |
|---------|---|---------|
| YOLO 이론 이해 | -> | YOLO 실전 학습 |
| CIoU, mAP 개념 | -> | 실제 mAP 확인 및 분석 |
| 모델 구조 이해 | -> | Hyperparameter 튜닝 |

### 이번 주 전체 흐름

학습은 한 번에 끝나지 않는다. "학습 -> 평가 -> 분석 -> 개선" 사이클을 돌며 모델을 점점 좋게 만든다.

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

## 핵심 개념 자세히 알아보기

### 1. Ultralytics 사용법

**Ultralytics**는 YOLO11의 공식 라이브러리다. 모델 학습/추론/내보내기를 몇 줄로 끝낼 수 있게 해 준다. 두 가지 사용 방식이 있다 - 터미널에서 바로 쓰는 **CLI**와, 파이썬 코드 안에서 쓰는 **Python API**다.

```bash
# 설치 (한 번만)
pip install ultralytics

# CLI로 추론: 학습된 모델(yolo11n.pt)로 image.jpg에서 객체 검출
yolo detect predict model=yolo11n.pt source='image.jpg'

# CLI로 학습: coco128 데이터로 100 에폭 학습, 입력 크기 640
yolo detect train data=coco128.yaml model=yolo11n.pt epochs=100 imgsz=640
```

Python API는 같은 일을 코드로 한다. 실습과 자동화에는 이쪽을 주로 쓴다.

```python
from ultralytics import YOLO   # Ultralytics의 YOLO 클래스 불러오기

# -- 모델 로드 (세 가지 방식) --
model = YOLO("yolo11n.pt")     # 사전학습된 nano 모델 (가장 흔한 방식)
model = YOLO("yolo11n.yaml")   # 구조만 정의된 빈 모델 (가중치 없이 from scratch 학습)
model = YOLO("best.pt")        # 내가 이전에 학습시켜 저장한 모델

# -- 추론: 이미지/영상/폴더 어느 것이든 받는다 --
results = model("image.jpg")           # 이미지 한 장
results = model("video.mp4")           # 동영상
results = model("path/to/images/")     # 폴더 안 모든 이미지

# -- 학습: model.train()에 데이터와 설정을 넘긴다 --
results = model.train(
    data="coco128.yaml",   # 데이터셋 설정 파일
    epochs=100,            # 전체 데이터를 100번 반복 학습
    imgsz=640,             # 입력 이미지를 640x640으로
    batch=16,              # 한 번에 16장씩 묶어 학습
)

# -- 평가: 검증 데이터로 mAP 등을 계산 --
metrics = model.val()

# -- 내보내기: 다른 런타임용 포맷으로 변환 (Week 5-6에서 사용) --
model.export(format="onnx")     # ONNX 포맷
model.export(format="engine")   # TensorRT 엔진
```

#### 추론 결과 다루기

`model(...)`이 돌려주는 `results`에서 BBox, confidence, 클래스를 꺼내 쓸 수 있다.

```python
from ultralytics import YOLO

model = YOLO("yolo11n.pt")        # 모델 로드
results = model("image.jpg")      # 추론 실행

# results는 이미지마다 하나의 result를 담은 리스트
for result in results:
    boxes = result.boxes          # 이 이미지에서 검출된 BBox 묶음
    print(boxes.xyxy)             # BBox 좌표 [x1, y1, x2, y2] 형식 (픽셀)
    print(boxes.conf)             # 각 BBox의 confidence(검출 확신도, 0-1)
    print(boxes.cls)              # 각 BBox의 클래스 인덱스 (0=person 등)
    print(boxes.xywh)             # BBox를 [중심x, 중심y, 너비, 높이]로도 제공

    # 검출 결과가 그려진 이미지를 numpy 배열로 받기
    annotated = result.plot()

    # 특정 클래스만 골라내기 (예: 클래스 0 = person)
    person_mask = boxes.cls == 0  # cls가 0인 위치만 True인 마스크
    person_boxes = boxes[person_mask]  # 마스크로 person BBox만 추림
```

---

### 2. YOLO 데이터셋 포맷

커스텀 데이터로 학습하려면, YOLO가 요구하는 **디렉토리 구조**와 **라벨 포맷**을 정확히 맞춰야 한다.

#### 디렉토리 구조

이미지와 라벨을 짝지어 두는 규칙이 있다. 같은 이름의 `.jpg`와 `.txt`가 한 쌍이다.

```
dataset/
+-- images/
| +-- train/
| | +-- img001.jpg
| | +-- img002.jpg
| | +-- ...
| +-- val/
| +-- img101.jpg
| +-- ...
+-- labels/
| +-- train/
| | +-- img001.txt ← 이미지와 동일한 이름
| | +-- img002.txt
| | +-- ...
| +-- val/
| +-- img101.txt
| +-- ...
+-- data.yaml ← 데이터셋 설정 파일
```

#### 라벨 포맷 (YOLO format)

라벨 파일(`.txt`)은 객체 하나당 한 줄이다. 각 줄은 다섯 개 숫자다.

```
# 각 행: class_id x_center y_center width height
# 모든 좌표는 0~1로 정규화 (이미지 크기로 나눔)


# img001.txt 예시:
0 0.5 0.4 0.3 0.6 # class=0(person), 중심(0.5, 0.4), 크기(0.3, 0.6)
1 0.2 0.3 0.1 0.15 # class=1(car), 중심(0.2, 0.3), 크기(0.1, 0.15)
0 0.8 0.7 0.15 0.5 # class=0(person), 두 번째 사람
```

이 포맷은 Week 2 §1.3에서 본 **YOLO BBox 포맷**(중심 좌표 + 크기, 0-1 정규화)과 같다. Pascal VOC 픽셀 좌표를 YOLO로 바꾸는 계산은 이렇다.

```
좌표 설명 (640x480 이미지 기준):


원본 BBox: x_min=100, y_min=50, x_max=300, y_max=350


YOLO 변환:
  x_center = (100 + 300) / 2 / 640 = 200 / 640 = 0.3125
  y_center = (50 + 350) / 2 / 480 = 200 / 480 = 0.4167
  width = (300 - 100) / 640 = 200 / 640 = 0.3125
  height = (350 - 50) / 480 = 300 / 480 = 0.6250


라벨: 0 0.3125 0.4167 0.3125 0.6250
```

좌표를 0-1로 정규화하는 이유는 **이미지 크기에 무관하게 같은 라벨을 쓰기 위해서**다. 640x480에서 만든 라벨을 1920x1080 이미지에서도 그대로 쓸 수 있고, 학습 중 이미지 크기를 바꿔도 라벨을 고칠 필요가 없다. `quiz_easy.py` 문제 1이 이 포맷의 의미를 묻는다.

#### data.yaml 설정 파일

`data.yaml`은 "데이터가 어디에 있고, 클래스가 무엇인지"를 YOLO에 알려주는 파일이다.

```yaml
# data.yaml
path: /path/to/dataset   # 데이터셋 루트 폴더의 절대 경로
train: images/train      # 학습 이미지 폴더 (path 기준 상대 경로)
val: images/val          # 검증 이미지 폴더


# 클래스: 인덱스 -> 이름 매핑
names:
  0: person
  1: car
  2: bicycle
  3: dog


nc: 4   # 클래스 개수 (Number of Classes), names 항목 수와 일치해야 함
```

---

### 3. 라벨링 도구

커스텀 데이터는 사람이 직접 BBox를 그려 라벨을 만들어야 한다. 두 가지 도구가 흔히 쓰인다.

#### LabelImg (간단한 BBox 라벨링)

로컬에서 돌리는 가벼운 GUI 도구다.

```bash
pip install labelImg   # 설치
labelImg               # GUI 실행
```

```
LabelImg 사용법:
1. Open Dir → 이미지 폴더 선택
2. Change Save Dir → 라벨 저장 폴더 선택
3. 저장 포맷: YOLO 선택   ← 중요! 기본은 Pascal VOC라 바꿔야 함
4. 'W' 키: BBox 그리기
5. Ctrl+S: 저장
6. 'D' 키: 다음 이미지
```

#### Roboflow (온라인 라벨링 + 버전 관리)

웹 기반 도구로, 팀 협업과 데이터셋 관리 기능이 있다.

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

혼자 소규모로 하면 LabelImg, 팀이 대규모로 하면 Roboflow가 적합하다.

---

### 4. 학습 실습 (COCO128)

**COCO128**은 대형 데이터셋 COCO에서 128장만 추린 미니 데이터셋이다. 학습이 제대로 도는지 빠르게 확인하는 용도다 (실제 성능을 내는 용도는 아니다).

```python
from ultralytics import YOLO

# -- 기본 학습 --
model = YOLO("yolo11n.pt")     # 사전학습된 nano 모델에서 출발

results = model.train(
    data="coco128.yaml",       # 내장 데이터셋 (없으면 자동 다운로드)
    epochs=50,                 # 전체 데이터를 50번 반복
    imgsz=640,                 # 입력 이미지 크기 640x640
    batch=16,                  # 한 번에 16장씩
    device=0,                  # 사용할 GPU 번호 (0 = 첫 번째 GPU)
    project="runs/detect",     # 결과를 저장할 상위 폴더
    name="coco128_exp",        # 이번 실험 폴더 이름
)
```

#### 주요 학습 인자

`model.train()`에는 학습을 조절하는 인자가 많다. 자주 쓰는 것들을 그룹별로 정리했다.

```python
model.train(
    # -- 기본 설정 --
    data="data.yaml",      # 데이터셋 설정 파일
    epochs=100,            # 에폭 수 (전체 데이터 반복 횟수)
    patience=50,           # Early stopping: 50 에폭간 개선 없으면 중단
    batch=16,              # 배치 크기 (-1로 두면 GPU에 맞춰 자동 결정)
    imgsz=640,             # 입력 이미지 크기
    device=0,              # GPU 번호

    # -- 최적화 (모델 가중치를 갱신하는 방식) --
    optimizer="auto",      # 옵티마이저 (SGD, Adam, AdamW, auto)
    lr0=0.01,              # 초기 학습률 (한 번에 가중치를 얼마나 바꿀지)
    lrf=0.01,              # 최종 학습률 비율 (마지막엔 lr0 * lrf 까지 감소)
    momentum=0.937,        # SGD momentum (이전 갱신 방향을 얼마나 유지할지)
    weight_decay=0.0005,   # 가중치 감쇠 (값이 너무 커지지 않게 억제, 과적합 방지)

    # -- Augmentation (Week 2에서 배운 데이터 증강) --
    hsv_h=0.015,           # 색조(Hue) 변형 강도
    hsv_s=0.7,             # 채도(Saturation) 변형 강도
    hsv_v=0.4,             # 명도(Value) 변형 강도
    degrees=0.0,           # 회전 각도 범위
    translate=0.1,         # 이동(translate) 비율
    scale=0.5,             # 스케일 변형 비율
    fliplr=0.5,            # 좌우 반전 확률
    mosaic=1.0,            # Mosaic augmentation 확률 (아래 설명)
    mixup=0.0,             # MixUp augmentation 확률

    # -- 기타 --
    pretrained=True,       # 사전학습 가중치에서 시작
    resume=False,          # 중단된 학습을 이어서 할지
    val=True,              # 매 에폭 검증 수행
    save=True,             # 체크포인트 저장
    plots=True,            # 학습 결과 그래프 자동 생성
)
```

#### Mosaic Augmentation

**Mosaic**는 YOLO 학습에서 특히 중요한 증강 기법이다. 4장의 이미지를 하나로 합쳐 학습한다.

```
Mosaic: 4장의 이미지를 하나로 합쳐 학습
+----------+----------+
| 이미지1 | 이미지2 |
| | |
+----------+----------+
| 이미지3 | 이미지4 |
| | |
+----------+----------+


효과:
  - 작은 객체를 더 많이 포함
  - 문맥(context) 다양화
  - 배치 크기를 효과적으로 4배 증가
  - 마지막 10 에폭에서 자동 비활성화 (close_mosaic=10)
```

**마지막 10 에폭에서 Mosaic을 끄는 이유**가 중요하다. Mosaic으로 합성된 이미지는 실제 입력 이미지와 분포가 다르다. 학습 막바지에는 실제 이미지로 마무리해야 모델이 실제 추론 환경에 적응해 최종 성능이 안정된다. 이것이 `quiz_easy.py` 문제 3의 정답이 "B) 최종 에폭에서는 원본 이미지로 학습하여 성능을 안정화"인 이유다. 비유하면, 공부할 땐 다양한 변형 문제를 풀다가 시험 직전엔 실제 시험지 형식으로 정리하는 것과 같다.

---

### 5. Hyperparameter 튜닝

**Hyperparameter**(하이퍼파라미터)는 학습 전에 사람이 정하는 설정값이다 (모델이 학습으로 알아내는 가중치와 구별된다). 어떤 값을 쓰느냐가 성능을 크게 좌우한다.

#### 핵심 Hyperparameter

성능에 큰 영향을 미치는 항목들이다.

1. **학습률** (`lr0`): 0.01 (SGD), 0.001 (Adam) 기본값. 너무 크면 발산, 너무 작으면 수렴이 느림.
2. **배치 크기** (`batch`): GPU 메모리에 맞게 설정. 클수록 안정적, 작으면 정규화 효과.
3. **이미지 크기** (`imgsz`): 클수록 정확, 작으면 빠름. 320, 640, 1280 등 32의 배수.
4. **Augmentation 강도**: 데이터 적으면 강하게, 많으면 약하게.
5. **모델 크기** (`n/s/m/l/x`): 데이터/GPU에 맞게 선택.

#### 튜닝 전략

여러 값을 한꺼번에 바꾸면 무엇이 효과였는지 알 수 없다. **한 번에 하나씩** 바꾸며 단계적으로 접근한다.

```
1단계: 기본값으로 Baseline 확립
  → yolo11n.pt, epochs=50, imgsz=640, batch=16


2단계: 모델 크기 실험
  → yolo11n → yolo11s → yolo11m (데이터/GPU 여유 시)


3단계: 학습률 조절
  → lr0: 0.001, 0.005, 0.01, 0.02


4단계: Augmentation 조절
  → 데이터 적으면: mosaic=1.0, mixup=0.1
  → 데이터 많으면: mosaic=0.5, mixup=0.0


5단계: 이미지 크기
  → 640 → 960 → 1280 (GPU 메모리 허용 시)
```

`quiz_medium.py` 문제 3이 실험 결과 표를 보고 "다음에 무엇을 바꿀지" 정하게 한다. 핵심 원칙은 (1) 한 번에 한 변수만, (2) 과적합이 보이면 먼저 Augmentation 강화, (3) 항상 `val_mAP` 기준으로 판단(train_loss 아님)이다.

---

### 6. 모델 평가

#### 기본 평가

학습이 끝나면 검증 데이터로 성능을 측정한다.

```python
from ultralytics import YOLO

# 학습으로 저장된 최고 성능 모델 로드
model = YOLO("runs/detect/coco128_exp/weights/best.pt")

# 검증 데이터로 평가 실행
metrics = model.val()

# 결과 지표 출력 (지표 의미는 Week 3 §8 참고)
print(f"mAP@0.5:     {metrics.box.map50:.4f}")   # IoU 0.5 기준 mAP
print(f"mAP@0.5:0.95:{metrics.box.map:.4f}")     # COCO 공식 지표 (더 엄격)
print(f"Precision:   {metrics.box.mp:.4f}")      # 평균 Precision
print(f"Recall:      {metrics.box.mr:.4f}")      # 평균 Recall
```

#### 학습 결과 파일

학습이 끝나면 결과 폴더에 여러 파일이 생긴다.

```
runs/detect/coco128_exp/
+-- weights/
| +-- best.pt ← 최고 mAP 모델
| +-- last.pt ← 마지막 에폭 모델
+-- results.csv ← 에폭별 메트릭
+-- results.png ← 학습 커브 그래프
+-- confusion_matrix.png ← 혼동 행렬
+-- P_curve.png ← Precision 커브
+-- R_curve.png ← Recall 커브
+-- PR_curve.png ← PR 커브
+-- F1_curve.png ← F1 커브
+-- val_batch0_pred.jpg ← 검증 예측 시각화
+-- val_batch0_labels.jpg ← 검증 GT 시각화
```

**`best.pt`와 `last.pt`의 차이**가 중요하다. `best.pt`는 학습 도중 검증 mAP가 가장 높았던 에폭의 가중치이고, `last.pt`는 마지막 에폭의 가중치다. 마지막 에폭이 항상 최고는 아니다 - 과적합으로 오히려 나빠졌을 수 있다. 그래서 **배포에는 항상 `best.pt`**를 쓰고, `last.pt`는 학습을 이어서(resume) 할 때 쓴다. 이것이 `quiz_easy.py` 문제 2의 정답이다.

#### Confusion Matrix 분석

**Confusion Matrix**(혼동 행렬)는 "실제 클래스 vs 예측 클래스"를 표로 보여준다. 어디서 모델이 헷갈리는지 진단하는 도구다.

```
Confusion Matrix (혼동 행렬):


              예측: person car bicycle background
실제: person | 85 2 1 12 |
      car | 3 78 0 19 |
      bicycle | 1 0 65 34 |
      background| 5 3 2 - |


대각선: 올바른 분류 (높을수록 좋음)
비대각선: 오분류 (낮을수록 좋음)


분석 포인트:
  - person → background (12): 12개의 사람을 못 찾음 (FN)
  - background → person (5): 5번 사람이 아닌 것을 사람으로 오검출 (FP)
  - bicycle → background (34): 자전거 미검출이 많음 → 데이터 부족?
```

읽는 법: **`background` 열**(실제 객체인데 background로 예측)은 **미검출**(FN)을 뜻한다. **`background` 행**(실제는 background인데 객체로 예측)은 **오검출**(FP)을 뜻한다. 위 예시에서 bicycle은 background 열 값(34)이 커서 미검출이 많고, 이는 자전거 학습 데이터가 부족하다는 신호다. `quiz_medium.py` 문제 2가 이 행렬에서 Recall과 FN/FP가 많은 클래스를 찾게 한다.

---

### 7. False Positive 분석

**False Positive(오검출)** - 객체가 없는데 있다고 검출한 것 - 를 줄이는 것이 모델 개선의 핵심이다. FP는 원인별로 유형이 나뉜다.

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

낮은 confidence로 검출된 결과는 FP일 가능성이 높다. 아래 코드로 의심스러운 검출을 추려 눈으로 확인한다.

```python
from ultralytics import YOLO
import cv2   # 이미지 입출력용 (OpenCV)

model = YOLO("best.pt")   # 학습된 모델 로드

# 검증 데이터로 평가하고 결과를 JSON으로도 저장
results = model.val(data="data.yaml", save_json=True)

# confidence 임계값을 낮춰(0.25) 추론 -> 불확실한 검출까지 모두 잡아냄
results = model("val_images/", conf=0.25)
for result in results:                      # 이미지별로 순회
    for box in result.boxes:                # 그 이미지의 BBox별로 순회
        if box.conf < 0.5:                  # confidence가 0.5 미만이면 FP 의심
            print(f"의심 FP: class={box.cls.item():.0f}, "
                  f"conf={box.conf.item():.3f}, "
                  f"box={box.xyxy[0].tolist()}")   # 클래스/확신도/좌표 출력
```

---

## 꼭 이해해야 할 핵심 개념

### 1. 학습 전체 파이프라인

학습은 일직선이 아니라 순환이다. 평가/분석 결과로 데이터나 설정을 고쳐 다시 학습한다.

```
데이터 수집 라벨링 학습 설정
---------- ---------- ----------
이미지 촬영/수집 LabelImg/ data.yaml 작성
                 Roboflow로 모델 크기 선택
                 BBox 라벨링 Hyperparameter 설정
     | | |
     v v v
+---------------------------------------+
| model.train(...) |
| |
| Epoch 1: mAP=0.10 ----→ 개선 중 |
| Epoch 50: mAP=0.45 ----→ 수렴 중 |
| Epoch 100: mAP=0.52 ---→ 최종 |
+---------------------------------------+
     |
     v
  평가 & 분석
  ----------
  mAP 확인
  Confusion Matrix
  FP 분석
  → 데이터/하이퍼파라미터 개선 후 재학습
```

### 2. 모델 크기 선택 가이드

YOLO11은 n/s/m/l/x 다섯 크기로 나온다. 클수록 정확하지만 느리고 무겁다.

| 모델 | 파라미터 | mAP@0.5:0.95 (COCO) | 추론 속도 | 용도 |
|------|---------|---------------------|----------|------|
| YOLO11n | 2.6M | 39.5 | 가장 빠름 | Edge 디바이스 |
| YOLO11s | 9.4M | 47.0 | 빠름 | 실시간 응용 |
| YOLO11m | 20.1M | 51.5 | 보통 | 균형 |
| YOLO11l | 25.3M | 53.4 | 느림 | 높은 정확도 |
| YOLO11x | 56.9M | 54.7 | 가장 느림 | 최고 정확도 |

### 3. Overfitting 판단법

Week 2 §2.4에서 본 과적합 판단을 학습 곡선으로 확인한다.

```
정상 학습:
  train_loss ↓ val_loss ↓ val_mAP ↑


과적합 (Overfitting):
  train_loss ↓ val_loss ↑ val_mAP ↓ (또는 정체)
  → 해결: Augmentation 강화, 데이터 추가, 모델 축소


미적합 (Underfitting):
  train_loss 높음 val_mAP 낮음
  → 해결: 에폭 증가, 모델 확대, 학습률 조절
```

`quiz_easy.py` 문제 4가 학습 곡선(train_loss는 계속 내려가는데 val_mAP가 떨어지는 상황)을 주고 과적합 여부와 해결책을 묻는다. 핵심 구분: **과적합은 train은 잘하는데 val을 못함**(정규화 필요), **미적합은 train도 val도 못함**(모델 확대 필요)이다.

---

## 자체 점검 - 이해했는지 확인!

**Q1. YOLO 라벨 포맷에서 좌표가 0-1로 정규화된 이유는?**
> 이미지 크기에 무관하게 같은 라벨을 쓰기 위해서다. 640x480에서 만든 라벨을 1920x1080 이미지에서도 그대로 쓸 수 있고, 학습 중 입력 크기를 바꿔도 라벨을 고칠 필요가 없다 (§2).

**Q2. Mosaic Augmentation의 효과와, 마지막 10 에폭에 끄는 이유는?**
> 4장을 하나로 합쳐 작은 객체를 더 많이 포함시키고 문맥을 다양화하며 배치 크기를 사실상 4배로 키운다. 다만 합성 이미지는 실제 입력과 분포가 달라, 마지막 10 에폭에는 꺼서 실제 이미지로 마무리해 최종 성능을 안정화한다 (§4).

**Q3. best.pt와 last.pt의 차이는?**
> best.pt는 학습 중 검증 mAP가 가장 높았던 에폭의 가중치, last.pt는 마지막 에폭의 가중치다. 마지막 에폭은 과적합으로 더 나빠졌을 수 있으므로, 배포에는 항상 best.pt를 쓰고 last.pt는 학습 재개용으로 쓴다 (§6).

**Q4. Confusion Matrix에서 background 행/열의 의미는?**
> background 열(실제 객체를 background로 예측)은 미검출(FN)을, background 행(실제 background를 객체로 예측)은 오검출(FP)을 나타낸다. 미검출이 많은 클래스는 학습 데이터 추가가 필요할 수 있다 (§6).

---

## 이번 주 체크리스트

- [ ] Ultralytics 설치 및 CLI/Python API로 기본 추론 성공
- [ ] 추론 결과(`boxes.xyxy`, `conf`, `cls`)에서 값 꺼내기
- [ ] YOLO 데이터셋 디렉토리 구조와 라벨 포맷 이해
- [ ] Pascal VOC -> YOLO 라벨 변환 직접 계산
- [ ] `data.yaml` 작성
- [ ] COCO128로 YOLO11n 학습 완료
- [ ] 학습 결과 파일(results.png, confusion_matrix.png) 확인
- [ ] mAP@0.5, mAP@0.5:0.95 결과 해석
- [ ] Hyperparameter를 한 번에 하나씩 바꿔 재학습 실험
- [ ] Confusion Matrix로 FN/FP 많은 클래스 진단
- [ ] False Positive 유형별 원인 분석

---

## 핵심 요약

**YOLO11 학습 핵심 정리**

1. **Ultralytics**: `pip install ultralytics` (간편한 API)
2. **데이터셋**: YOLO format (`class x_c y_c w h`, 정규화)
3. **학습**: `model.train(data, epochs, imgsz, batch)`
4. **평가**: mAP@0.5:0.95 + Confusion Matrix
5. **분석**: FP 유형별 원인 파악 -> 데이터/설정 개선
6. **체크포인트**: `best.pt` (배포용) vs `last.pt` (학습 재개용)

---

이전: [Week 3 - YOLO 이론](../week3/README.md)
다음: [Week 5 - Depth Estimation](../week5/README.md)
