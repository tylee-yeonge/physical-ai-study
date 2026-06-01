# Week 4 실습: YOLO11 학습 파이프라인


> **실습 목표**: YOLO11을 COCO128으로 학습하고, 커스텀 데이터셋을 준비하며, 평가 결과를 분석한다.
> **예상 시간**: 6-8시간


---


## 환경 설정


```bash
cd Studies/Phase\ 3/week4

# 가상환경 생성 및 의존성 설치 (apt 패키지 설치 포함, root 권한 필요)
# 내부에서 .venv-week4 생성 후 requirements.txt 설치
./pip_install.sh

# 가상환경 활성화 (sh 종료 후 현재 shell에서 직접 활성화 필요)
source .venv-week4/bin/activate


# Ultralytics 설치 확인
python -c "from ultralytics import YOLO; print('Ultralytics 설치 완료!')"


# 퀴즈 실행
python quiz_easy.py
python quiz_medium.py
```


---


## 실습 1: YOLO11 기본 추론

이 실습은 학습을 하지 않는다. 이미 COCO로 학습된 YOLO11을 그대로 가져와 이미지에 추론을 돌려보며, Ultralytics API의 입출력이 어떻게 생겼는지 손에 익힌다.

### 왜 추론부터 시작하나

week3에서 YOLO의 내부 구조와 NMS/지표를 이론으로 봤다. 그것을 직접 학습시키기 전에, 완성된 모델이 어떻게 동작하는지부터 본다. 모델 로드 -> 추론 -> 결과 파싱(박스 좌표/클래스/confidence) -> 시각화의 흐름이 이후 모든 detection 작업의 기본형이다. confidence 임계값을 바꿔보는 부분이 핵심 감각이다. 임계값을 올리면 확실한 것만 남고(검출 수 감소), 내리면 애매한 것까지 잡는다(오검출 증가). 실습 3의 mAP가 이 트레이드오프 위에서 정의된다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 모델 로드/추론 | `YOLO("yolo11n.pt")`, `model(img)` | API 기본 호출 |
| 결과 파싱 | `boxes.cls`/`conf`/`xyxy` | 검출 결과 자료구조 읽기 |
| 결과 저장 | `result.plot()`, `imwrite` | 박스가 그려진 이미지 저장 |
| 임계값 실험 | `conf=0.25`/`0.5`/`0.75` | 임계값과 검출 수의 관계 |

### 핵심 포인트

- 입력 이미지를 outputs 폴더에 받아두고 쓰는 이유는 cwd(수업 자료 폴더)가 다운로드 파일로 오염되는 것을 막기 위함이다(주석에 명시).
- `xyxy`는 픽셀 좌표(좌상단/우하단)다. week3의 정규화 YOLO 포맷과 다른 표현임을 구분한다.


**파일명**: `practice_inference.py`


```python
"""
실습 1: YOLO11 기본 추론
목표: Pretrained YOLO11으로 이미지/영상 추론을 수행한다.
"""
import os # 파일/디렉토리 작업
import urllib.request # URL 이미지 다운로드
from ultralytics import YOLO # YOLO 모델 로드/추론/학습
import cv2 # OpenCV (이미지 입출력)
import numpy as np
import matplotlib
matplotlib.use('Agg') # GUI 없이 파일로 저장하는 백엔드
import matplotlib.pyplot as plt


print("=" * 50)
print("실습 1: YOLO11 기본 추론")
print("=" * 50)


# -- 모델 로드 --
model = YOLO('yolo11n.pt') # nano 모델 로드 (파일 없으면 자동 다운로드)
print(f"\n모델 로드 완료: yolo11n.pt")


# -- 테스트 이미지 준비 --
# URL 이미지를 outputs 폴더에 받아두고 추론에 재사용 (cwd가 수업 자료로 오염되는 것 방지)
os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (수업 자료와 분리)
img_path = 'outputs/bus.jpg' # 입력 이미지 저장 위치
urllib.request.urlretrieve('https://ultralytics.com/images/bus.jpg', img_path) # 이미지 다운로드
results = model(img_path) # 로컬 이미지로 추론 실행


# -- 결과 분석 --
print("\n[1] 검출 결과 분석")
for result in results: # 입력 이미지별 결과 (여기선 1장)
    boxes = result.boxes # 검출된 박스 모음
    print(f"검출된 객체 수: {len(boxes)}")


    for i, box in enumerate(boxes): # 박스 하나씩 정보 추출
        cls_id = int(box.cls) # 클래스 인덱스
        cls_name = model.names[cls_id] # 인덱스 -> 클래스 이름
        conf = box.conf.item() # confidence (검출 신뢰도)
        xyxy = box.xyxy[0].tolist() # 박스 좌표 [x1, y1, x2, y2]


        print(f"객체 {i}: {cls_name} (conf={conf:.3f})")
        print(f"BBox: [{xyxy[0]:.1f}, {xyxy[1]:.1f}, "
              f"{xyxy[2]:.1f}, {xyxy[3]:.1f}]")


    # 결과 시각화 저장
    annotated = result.plot() # 박스/라벨이 그려진 이미지 생성
    cv2.imwrite('outputs/inference_result.jpg', annotated) # outputs 폴더에 저장
    print("\n결과 저장: outputs/inference_result.jpg")


# -- Confidence 임계값 변경 --
print("\n[2] Confidence 임계값 비교")
for conf_thresh in [0.25, 0.50, 0.75]: # 임계값을 바꿔가며 검출 수 비교
    results = model(img_path, conf=conf_thresh, verbose=False) # 받아둔 로컬 이미지 재사용
    n_detections = len(results[0].boxes)
    print(f"conf={conf_thresh:.2f}: {n_detections}개 검출")


print("\n 실습 1 완료!")
```


**실행**:
```bash
python practice_inference.py
```


---


## 실습 2: COCO128 학습

이 실습에서 처음으로 YOLO11을 직접 학습시킨다. week1에서 손으로 짠 학습 루프를 Ultralytics가 한 줄(`model.train()`)로 감싸준다는 것을 확인하는 것이 핵심이다.

### 왜 COCO128로 학습하나

COCO128은 COCO에서 128장만 추린 초소형 데이터셋이다. 진짜 성능을 내려는 것이 아니라, 학습 파이프라인이 끝까지 도는지(데이터 로드 -> 학습 -> 검증 -> 결과 파일 생성)를 빠르게 확인하기 위한 것이다. 128장이라 몇 분이면 끝난다. `model.train()` 한 줄 안에 week1에서 본 모든 것(DataLoader, 순전파, loss, backward, optimizer, 평가, 체크포인트)이 들어 있다. 고수준 API가 그 골격을 감췄을 뿐이다. 학습이 끝나면 `results.csv`, 학습 커브, 혼동 행렬 등 분석용 파일이 자동 생성된다(실습 5에서 분석).

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 학습 실행 | `model.train(data="coco128.yaml", ...)` | 한 줄로 전체 학습 루프 |
| 주요 인자 | `epochs`/`imgsz`/`batch`/`patience` | 학습 설정의 의미 |
| 평가 | `best.pt` 로드 후 `val()` | mAP/Precision/Recall 측정 |
| 결과 파일 | `results.png`/`confusion_matrix`/`PR_curve` | 자동 생성되는 분석 자료 |

### 핵심 포인트

- `best.pt`는 검증 mAP가 최고였던 시점, `last.pt`는 마지막 epoch이다. 배포에는 `best.pt`를 쓴다(README). week1 실습 3의 `best_acc` 저장과 같은 원리다.
- `project` 경로를 `RUNS_DIR` 기준으로 조립하는 이유는 머신 종속 절대경로 하드코딩을 피하고 `runs/detect` 중첩을 막기 위함이다(주석).


**파일명**: `practice_train_coco128.py`


```python
"""
실습 2: COCO128로 YOLO11 학습
목표: 기본 학습을 수행하고 결과를 분석한다.
"""
from ultralytics import YOLO
from ultralytics.utils import RUNS_DIR # ultralytics 기본 저장 위치 (physical-ai-study/runs)
import os # 파일/디렉토리 작업


print("=" * 50)
print("실습 2: COCO128 학습")
print("=" * 50)


# -- 모델 로드 --
model = YOLO('yolo11n.pt') # 사전학습 nano 모델 로드


# RUNS_DIR(= physical-ai-study/runs)의 부모인 physical-ai-study를 기준으로 week4
# 경로를 조립한다. /workspace/study 같은 머신 종속 경로를 하드코딩하지 않아도 되고,
# 절대 경로라 상대 경로처럼 runs/detect가 중첩되는 문제도 없다
project_dir = str(RUNS_DIR.parent / 'Studies/Phase 3/week4/outputs/runs/detect')


# -- 학습 --
print("\n학습 시작...")
results = model.train( # 학습 실행 (아래 인자로 학습 설정 전달)
    data='coco128.yaml', # 내장 데이터셋 (자동 다운로드)
    epochs=30, # 빠른 실험을 위해 30 에폭
    imgsz=640, # 입력 크기
    batch=16, # 배치 크기 (GPU 메모리에 맞게 조절)
    device=0, # GPU (CPU면 'cpu')
    project=project_dir, # 결과 저장 경로 (week4 하위 절대 경로)
    name='coco128_baseline', # 실험 이름
    patience=10, # Early stopping
    save=True, # 체크포인트 저장
    plots=True, # 결과 시각화
    verbose=True,
)


# -- 결과 확인 --
print("\n[학습 결과]")
result_dir = f'{project_dir}/coco128_baseline' # 학습 결과가 저장된 폴더


# 생성된 파일 확인
if os.path.exists(result_dir): # 결과 폴더가 있으면
    files = os.listdir(result_dir) # 폴더 안 파일 목록
    print(f"결과 디렉토리: {result_dir}")
    print(f"생성된 파일: {files}")


# -- 평가 --
print("\n[모델 평가]")
best_model = YOLO(f'{result_dir}/weights/best.pt') # 학습 중 가장 좋았던 가중치 로드
metrics = best_model.val(project=project_dir, name='coco128_baseline_val') # 검증셋으로 평가 (project 미지정 시 루트 runs/로 샘)


print(f"\n mAP@0.5: {metrics.box.map50:.4f}") # IoU 0.5 기준 평균 정밀도
print(f"mAP@0.5:0.95: {metrics.box.map:.4f}") # IoU 0.5~0.95 평균 (더 엄격한 지표)
print(f"Precision: {metrics.box.mp:.4f}") # 정밀도 (예측 중 맞은 비율)
print(f"Recall: {metrics.box.mr:.4f}") # 재현율 (정답 중 찾아낸 비율)


print("\n결과 파일 확인:")
print(f"- {result_dir}/results.png (학습 커브)")
print(f"- {result_dir}/confusion_matrix.png (혼동 행렬)")
print(f"- {result_dir}/PR_curve.png (PR 커브)")


print("\n 실습 2 완료!")
```


**실행**:
```bash
python practice_train_coco128.py
```


**예상 결과**:
- COCO128 (30 에폭): mAP@0.5 약 0.5-0.6


---


## 실습 3: Hyperparameter 비교 실험

같은 데이터와 모델로 학습률, 증강 같은 하이퍼파라미터만 바꿔 4가지 실험을 돌리고 mAP를 비교한다. 무엇을 바꾸면 성능이 어떻게 변하는가를 숫자로 확인하는 것이 목적이다.

### 왜 비교 실험을 하나

학습을 한 번 돌릴 줄 아는 것과, 결과를 보고 다음 설정을 정하는 것은 다른 능력이다. 이 실습은 `lr_low`/`lr_high`/`no_mosaic`/`heavy_aug` 네 조건을 자동으로 돌려 표로 비교한다. week2에서 본 W&B 실험 추적의 실전 버전이다. 중요한 교훈이 README에 있다. 데이터 품질이 하이퍼파라미터보다 압도적으로 중요하다. `lr0`를 0.001 대 0.02로 바꿔 얻는 차이보다 라벨 오류를 고치고 데이터를 늘리는 효과가 훨씬 크다. 이 실습은 그 사실을 작은 차이로 직접 보게 한다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 실험 자동화 | `experiments` 리스트 순회 | 설정만 바꿔 반복 학습 |
| 하이퍼파라미터 | `lr0`/`mosaic`/`mixup`/`degrees`/`scale` | 각 인자가 무엇을 바꾸나 |
| 결과 비교 | mAP/Precision/Recall 표 | 조건별 성능 차이 정량 비교 |
| 결과 저장 | `experiment_results.json` | 비교 결과 영속화 |

### 핵심 포인트

- `try`/`except`로 실패한 실험도 0점으로 기록하고 계속 진행한다. 한 실험이 깨져도 전체가 멈추지 않게 하기 위함이다.
- COCO128은 128장이라 실험 간 mAP 차이가 작거나 불안정할 수 있다. 하이퍼파라미터 튜닝보다 데이터가 중요하다는 것을 체감하는 것이 진짜 목적이다.


**파일명**: `practice_hyperparameter.py`


```python
"""
실습 3: Hyperparameter 비교 실험
목표: 주요 Hyperparameter를 변경하며 성능 차이를 비교한다.
"""
from ultralytics import YOLO
from ultralytics.utils import RUNS_DIR # ultralytics 기본 저장 위치 (physical-ai-study/runs)
import json # 결과를 JSON 파일로 저장
import os


print("=" * 50)
print("실습 3: Hyperparameter 비교 실험")
print("=" * 50)


# RUNS_DIR(= physical-ai-study/runs)의 부모를 기준으로 week4 경로를 조립한다.
# 머신 종속 경로 하드코딩을 피하면서 절대 경로라 runs/detect 중첩도 없다
project_dir = str(RUNS_DIR.parent / 'Studies/Phase 3/week4/outputs/runs/detect')
os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (JSON을 수업 자료와 분리)


# -- 실험 설정 --
experiments = [ # 비교할 실험 목록 (각각 다른 하이퍼파라미터 적용)
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


results_summary = [] # 실험별 성능 결과 누적


for exp in experiments: # 실험을 하나씩 순서대로 실행
    print(f"\n{'='*50}")
    print(f"실험: {exp['desc']}")
    print(f"{'='*50}")


    model = YOLO('yolo11n.pt') # 매 실험마다 새 모델로 시작 (공정한 비교)


    try:
        result = model.train(
            data='coco128.yaml',
            imgsz=640,
            batch=16,
            device=0,
            project=project_dir, # 결과 저장 경로 (week4 하위 절대 경로)
            name=exp['name'],
            patience=10,
            plots=True,
            verbose=False,
            **exp['params'] # 실험별 하이퍼파라미터를 펼쳐서 전달
        )


        # 평가
        best_path = f'{project_dir}/{exp["name"]}/weights/best.pt'
        if os.path.exists(best_path):
            eval_model = YOLO(best_path) # 학습된 best 가중치 로드
            metrics = eval_model.val(verbose=False, project=project_dir, name=f'{exp["name"]}_val') # 검증셋 평가 (project 미지정 시 루트 runs/로 샘)


            results_summary.append({ # 이 실험의 성능 지표 기록
                "name": exp["name"],
                "desc": exp["desc"],
                "map50": metrics.box.map50,
                "map50_95": metrics.box.map,
                "precision": metrics.box.mp,
                "recall": metrics.box.mr,
            })
    except Exception as e: # 학습 실패 시 0으로 기록하고 다음 실험 진행
        print(f"실험 실패: {e}")
        results_summary.append({
            "name": exp["name"],
            "desc": exp["desc"],
            "map50": 0, "map50_95": 0,
            "precision": 0, "recall": 0,
        })


# -- 결과 비교 --
print("\n" + "=" * 70)
print("실험 결과 비교")
print("=" * 70)
print(f"{'실험':20s} | {'mAP@0.5':>8s} | {'mAP@0.5:0.95':>12s} | {'Precision':>9s} | {'Recall':>6s}")
print("-" * 70)
for r in results_summary: # 실험별 성능을 표 형태로 출력
    print(f"{r['desc']:20s} | {r['map50']:>8.4f} | {r['map50_95']:>12.4f} | "
          f"{r['precision']:>9.4f} | {r['recall']:>6.4f}")


# 결과 저장
with open('outputs/experiment_results.json', 'w') as f:
    json.dump(results_summary, f, indent=2, ensure_ascii=False) # 결과를 JSON 파일로 저장
print("\n결과 저장: outputs/experiment_results.json")


print("\n 실습 3 완료!")
```


**실행**:
```bash
python practice_hyperparameter.py
```


---


## 실습 4: 커스텀 데이터셋 준비

이 실습은 **모델이 무언가를 학습하는 실습이 아니다.** 이미지 내용 자체는 의미가 없고, 핵심은 YOLO 데이터셋 포맷(디렉토리 구조 + 라벨 규칙)을 직접 만들고 검증하는 것이다.

### 왜 난수 이미지를 쓰나

진짜 사진에 사람이 직접 BBox를 그려 라벨을 만들려면 라벨링 도구(LabelImg, Roboflow)로 수십 장을 작업해야 한다. 그건 이 실습의 본질이 아니다. 그래서 이미지는 의미 없는 랜덤 노이즈로 대체하고, 배워야 할 것(포맷과 구조)에만 집중한다. 노이즈 위에 박스가 의미 없이 떠 있는 건 의도된 것이고, 박스가 무엇을 맞췄는지는 볼 필요가 없다.

### 실제 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 1. 디렉토리 구조 | `images/{train,val}`, `labels/{train,val}` 폴더 생성 | YOLO는 같은 이름의 `.jpg`/`.txt`를 한 쌍으로 인식한다. 이 규칙을 손으로 만들어본다 |
| 2. 라벨 포맷 | `f"{cls_id} {x_center} {y_center} {width} {height}"` | 객체 1개 = 한 줄, 5개 숫자(클래스 + 중심좌표 + 크기) |
| 3. 좌표 정규화 | `x_center = (x1+x2)/2 / 640` 등 픽셀을 640/480으로 나눔 | 픽셀 좌표를 0-1로 변환한다. 이미지 크기가 바뀌어도 같은 라벨을 쓰기 위함 (README 2절) |
| 4. data.yaml | `path`/`train`/`val`/`names`/`nc` 작성 | 데이터가 어디 있고 클래스가 무엇인지를 YOLO에 알려주는 설정 파일 |

### 마지막 두 블록(검증 + 시각화)의 의미

라벨을 만들고 끝이 아니라, 내가 만든 라벨이 올바른지 스스로 점검하는 단계다. 이게 실습의 진짜 포인트다.

- **assert 검증**(`assert 0 <= x_c <= 1` 등): 정규화가 제대로 됐는지(좌표가 0-1 범위인지) 확인한다. 라벨링에서 가장 흔한 버그가 픽셀 좌표를 정규화하지 않고 그대로 넣는 것이다.
- **라벨 시각화**: `.txt`에 저장한 정규화 좌표를 다시 픽셀로 역변환(`x1 = (x_c - w/2)*W`)해서 이미지 위에 박스를 그려본다. 내가 저장한 라벨 숫자가 실제로 의도한 위치에 박스를 만드는지를 눈으로 확인하는 sanity check다. 박스가 엉뚱한 곳에 그려지면 정규화/역변환 공식이 틀린 것이다.

정리하면, 이번 실습에서 학습되는 건 모델이 아니라 작성자 자신이다. YOLO 폴더 구조, 라벨 한 줄의 형식, 픽셀-정규화 양방향 변환, 그리고 만든 라벨을 검증하는 습관을 익힌다. 실제 커스텀 학습은 이 구조에 진짜 이미지와 진짜 라벨만 채워 넣으면 그대로 돌아간다. 이번 주는 그 그릇을 만드는 연습이다.


**파일명**: `practice_custom_dataset.py`


```python
"""
실습 4: 커스텀 데이터셋 준비 및 포맷 변환
목표: YOLO 포맷의 데이터셋 구조를 직접 만들고 검증한다.
"""
import os
import yaml # YAML 파일 읽기/쓰기
import numpy as np
import cv2 # OpenCV (이미지 생성/저장)
import matplotlib
matplotlib.use('Agg') # GUI 없이 파일로 저장하는 백엔드
import matplotlib.pyplot as plt
import matplotlib.patches as patches # 사각형 그리기용


print("=" * 50)
print("실습 4: 커스텀 데이터셋 준비")
print("=" * 50)




# -- 1. 디렉토리 구조 생성 --
print("\n[1] 디렉토리 구조 생성")
base_dir = 'outputs/custom_dataset' # 생성 데이터셋을 outputs 폴더에 (수업 자료와 분리)
for split in ['train', 'val']: # 학습/검증 각각 폴더 생성
    os.makedirs(f'{base_dir}/images/{split}', exist_ok=True) # 이미지 폴더
    os.makedirs(f'{base_dir}/labels/{split}', exist_ok=True) # 라벨 폴더
print(f"{base_dir}/ 구조 생성 완료")




# -- 2. 가상 이미지와 라벨 생성 --
print("\n[2] 가상 데이터 생성")
class_names = ['person', 'car', 'bicycle'] # 클래스 3종
np.random.seed(42) # 난수 고정 (실행할 때마다 같은 데이터 생성)


def create_dummy_data(split, n_images):
    """가상 이미지와 라벨 생성"""
    for i in range(n_images):
        # 가상 이미지 (640x480)
        img = np.random.randint(100, 200, (480, 640, 3), dtype=np.uint8) # 랜덤 노이즈 이미지


        # 가상 객체 그리기
        n_objects = np.random.randint(1, 5) # 이미지당 객체 1~4개
        labels = []


        for _ in range(n_objects):
            cls_id = np.random.randint(0, len(class_names)) # 무작위 클래스 선택
            # 랜덤 BBox (xyxy)
            x1 = np.random.randint(10, 500)
            y1 = np.random.randint(10, 350)
            w = np.random.randint(40, 150)
            h = np.random.randint(40, 150)
            x2 = min(x1 + w, 639) # 이미지 너비 경계 안으로 제한
            y2 = min(y1 + h, 479) # 이미지 높이 경계 안으로 제한


            # 이미지에 사각형 그리기
            colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
            cv2.rectangle(img, (x1, y1), (x2, y2), colors[cls_id], 2)


            # YOLO 포맷으로 변환 (픽셀 좌표 -> 0~1로 정규화한 중심좌표/크기)
            x_center = (x1 + x2) / 2 / 640
            y_center = (y1 + y2) / 2 / 480
            width = (x2 - x1) / 640
            height = (y2 - y1) / 480


            labels.append(f"{cls_id} {x_center:.6f} {y_center:.6f} "
                         f"{width:.6f} {height:.6f}") # 한 줄 = 객체 하나


        # 저장
        img_name = f'img_{i:04d}.jpg'
        cv2.imwrite(f'{base_dir}/images/{split}/{img_name}', img) # 이미지 저장


        label_name = f'img_{i:04d}.txt'
        with open(f'{base_dir}/labels/{split}/{label_name}', 'w') as f:
            f.write('\n'.join(labels)) # 라벨 저장 (이미지와 같은 이름의 .txt)


    print(f"{split}: {n_images}장 생성")


create_dummy_data('train', 20) # 학습용 20장
create_dummy_data('val', 5) # 검증용 5장




# -- 3. data.yaml 생성 --
print("\n[3] data.yaml 생성")
data_yaml = { # YOLO 학습에 필요한 데이터셋 설정
    'path': os.path.abspath(base_dir), # 데이터셋 루트 절대경로
    'train': 'images/train', # 학습 이미지 경로 (path 기준 상대)
    'val': 'images/val', # 검증 이미지 경로
    'names': {i: name for i, name in enumerate(class_names)}, # 인덱스 -> 클래스 이름
    'nc': len(class_names), # 클래스 개수
}


yaml_path = f'{base_dir}/data.yaml'
with open(yaml_path, 'w') as f:
    yaml.dump(data_yaml, f, default_flow_style=False) # dict를 yaml 파일로 저장


print(f"저장: {yaml_path}")
print(f"내용:")
for key, value in data_yaml.items():
    print(f"{key}: {value}")




# -- 4. 라벨 검증 --
print("\n[4] 라벨 검증")
label_file = f'{base_dir}/labels/train/img_0000.txt'
print(f"라벨 파일: {label_file}")
with open(label_file, 'r') as f:
    lines = f.readlines()
    for line in lines: # 라벨 한 줄 = 객체 하나
        parts = line.strip().split()
        cls_id = int(parts[0]) # 첫 값은 클래스 인덱스
        x_c, y_c, w, h = map(float, parts[1:]) # 나머지는 중심좌표(x,y)와 크기(w,h)
        print(f"class={class_names[cls_id]}, "
              f"center=({x_c:.4f}, {y_c:.4f}), "
              f"size=({w:.4f}, {h:.4f})")


        # 범위 검증 (YOLO 좌표는 0~1로 정규화돼 있어야 정상)
        assert 0 <= x_c <= 1, f"x_center 범위 오류: {x_c}"
        assert 0 <= y_c <= 1, f"y_center 범위 오류: {y_c}"
        assert 0 < w <= 1, f"width 범위 오류: {w}"
        assert 0 < h <= 1, f"height 범위 오류: {h}"


print("모든 좌표가 0~1 범위 내 (검증 통과)")




# -- 5. 시각화 --
print("\n[5] 라벨 시각화")
img = cv2.imread(f'{base_dir}/images/train/img_0000.jpg') # 이미지 읽기
img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB) # BGR -> RGB 변환
H, W = img.shape[:2] # 이미지 높이, 너비


fig, ax = plt.subplots(1, 1, figsize=(10, 7))
ax.imshow(img_rgb)


colors_plt = ['red', 'lime', 'blue'] # 클래스별 박스 색
for line in lines: # 각 라벨을 이미지 위에 박스로 그리기
    parts = line.strip().split()
    cls_id = int(parts[0])
    x_c, y_c, w, h = map(float, parts[1:])


    # YOLO → 픽셀 좌표 (0~1 정규화 좌표를 실제 픽셀 값으로 환원)
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
plt.savefig('outputs/label_visualization.png', dpi=100) # 결과 이미지 저장
print("저장: outputs/label_visualization.png")


print("\n 실습 4 완료!")
```


**실행**:
```bash
python practice_custom_dataset.py
```


---


## 실습 5: 학습 결과 분석

실습 2/3이 생성한 결과 파일(`results.csv` 등)을 직접 읽어 학습 커브를 그리고, 과적합과 수렴 여부를 판단한다. 학습을 돌리는 것에서 해석하는 것으로 넘어가는 단계다.

### 왜 결과를 직접 분석하나

학습이 끝나면 mAP 숫자 하나만 보는 것이 아니라, epoch별 loss와 mAP 곡선을 봐야 한다. loss가 계속 떨어지는데 val mAP가 정체하거나 하락하면 과적합, 둘 다 아직 오르는 중이면 epoch을 늘릴 여지가 있다는 식의 판단을 한다. Ultralytics가 `results.png`를 자동으로 그려주지만, 이 실습은 CSV를 직접 파싱해 그 그래프가 어떤 숫자에서 나오는지를 손으로 확인한다. 이것이 한 바퀴(학습 -> 평가 -> 분석 -> 개선)의 마지막 고리다. week4 전체가 이 사이클을 한 번 도는 구성이다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| CSV 파싱 | `csv.DictReader`로 `results.csv` | epoch별 지표를 직접 읽기 |
| 커브 그리기 | loss/mAP를 epoch축으로 | 수렴/과적합 패턴 판독 |
| 개선점 도출 | 곡선 해석 | 다음 실험에서 무엇을 바꿀지 |

### 핵심 포인트

- 컬럼명 앞 공백까지 대비한다(`row.get('epoch', row.get(' epoch'))`). Ultralytics CSV 헤더에 공백이 섞이는 경우가 있어서다. 실데이터 파싱의 현실적 함정이다.
- `box_loss`는 위치, `cls_loss`는 분류 손실이다. 둘을 나눠 보면 모델이 어디를 찾는가와 무엇인지 맞히는가 중 어느 쪽에서 막히는지 구분된다.


**파일명**: `practice_analysis.py`


```python
"""
실습 5: 학습 결과 분석
목표: 학습 결과 파일을 분석하고 개선점을 도출한다.
"""
from ultralytics import YOLO
from ultralytics.utils import RUNS_DIR # ultralytics 기본 저장 위치 (physical-ai-study/runs)
import csv # CSV 파일 읽기
import os
import matplotlib
matplotlib.use('Agg') # GUI 없이 파일로 저장하는 백엔드
import matplotlib.pyplot as plt


print("=" * 50)
print("실습 5: 학습 결과 분석")
print("=" * 50)


# -- 1. 학습 커브 직접 그리기 --
print("\n[1] results.csv 분석")


# 실습 2와 동일하게 RUNS_DIR.parent(physical-ai-study) 기준으로 week4 경로 조립
project_dir = str(RUNS_DIR.parent / 'Studies/Phase 3/week4/outputs/runs/detect')
result_dir = f'{project_dir}/coco128_baseline'
csv_path = f'{result_dir}/results.csv' # 학습 중 epoch별 지표가 기록된 파일
os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (분석 그래프를 수업 자료와 분리)


if os.path.exists(csv_path):
    epochs = []
    train_box_loss = []
    train_cls_loss = []
    val_map50 = []
    val_map50_95 = []


    with open(csv_path, 'r') as f:
        reader = csv.DictReader(f) # 헤더(첫 줄)를 키로 쓰는 CSV 리더
        for row in reader: # epoch 한 줄씩 읽기
            # CSV 컬럼명 확인 후 조정 필요
            epochs.append(int(row.get('epoch', row.get(' epoch', 0)))) # 컬럼명 앞 공백도 대비
            train_box_loss.append(float(row.get('train/box_loss',
                                                row.get(' train/box_loss', 0))))
            train_cls_loss.append(float(row.get('train/cls_loss',
                                                row.get(' train/cls_loss', 0))))
            val_map50.append(float(row.get('metrics/mAP50(B)',
                                           row.get(' metrics/mAP50(B)', 0))))
            val_map50_95.append(float(row.get('metrics/mAP50-95(B)',
                                              row.get(' metrics/mAP50-95(B)', 0))))


    # 그래프 그리기
    fig, axes = plt.subplots(1, 3, figsize=(18, 5)) # 그래프 3개를 가로로 배치


    # Loss 커브
    axes[0].plot(epochs, train_box_loss, 'b-', label='Box Loss') # 박스 위치 손실
    axes[0].plot(epochs, train_cls_loss, 'r-', label='Cls Loss') # 분류 손실
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
                [val_map50[-1], val_map50_95[-1]], # 마지막 epoch의 값
                color=['green', 'blue'], alpha=0.7)
    axes[2].set_ylabel('mAP')
    axes[2].set_title('Final Performance')
    axes[2].set_ylim(0, 1)
    for i, v in enumerate([val_map50[-1], val_map50_95[-1]]):
        axes[2].text(i, v + 0.02, f'{v:.3f}', ha='center', fontsize=12) # 막대 위에 수치 표시


    plt.tight_layout()
    plt.savefig('outputs/training_analysis.png', dpi=100) # 결과 이미지 저장
    print("학습 커브 저장: outputs/training_analysis.png")
    print(f"최종 mAP@0.5: {val_map50[-1]:.4f}")
    print(f"최종 mAP@0.5:0.95: {val_map50_95[-1]:.4f}")
else:
    print(f"결과 파일 없음: {csv_path}")
    print("실습 2를 먼저 실행하세요.")


# -- 2. 모델별 성능 비교 --
print("\n[2] 모델 크기별 성능 (참고)")
print(f"{'모델':10s} | {'파라미터':>10s} | {'mAP@0.5:0.95':>12s} | {'용도':12s}")
print("" + "-" * 55)
model_info = [ # 모델 크기별 참고 수치 (이름, 파라미터, mAP, 용도)
    ('YOLO11n', '2.6M', '39.5', 'Edge/실시간'),
    ('YOLO11s', '9.4M', '47.0', '경량 서버'),
    ('YOLO11m', '20.1M', '51.5', '균형'),
    ('YOLO11l', '25.3M', '53.4', '높은 정확도'),
    ('YOLO11x', '56.9M', '54.7', '최고 성능'),
]
for name, params, mAP, usage in model_info:
    print(f"{name:10s} | {params:>10s} | {mAP:>12s} | {usage:12s}")


# -- 3. 개선 제안 --
print("\n[3] 성능 개선 체크리스트")
print("[ ] 더 큰 모델 시도 (yolo11n → yolo11s → yolo11m)")
print("[ ] 이미지 크기 증가 (640 → 960)")
print("[ ] 학습 에폭 증가 (30 → 100)")
print("[ ] Augmentation 강화 (mosaic, mixup)")
print("[ ] 학습률 조절 (lr0)")
print("[ ] 데이터 추가/정제")


print("\n 실습 5 완료!")
```


**실행**:
```bash
python practice_analysis.py
```


---


## 실습 체크리스트


- [ ] YOLO11 Pretrained 모델로 추론 성공
- [ ] COCO128 학습 완료 (30+ 에폭)
- [ ] 학습 결과 파일 확인 (results.png, confusion_matrix.png)
- [ ] mAP@0.5, mAP@0.5:0.95 결과 기록
- [ ] Hyperparameter 변경 실험 2개 이상 수행
- [ ] 커스텀 데이터셋 구조 생성 및 라벨 검증
- [ ] 학습 커브 분석 및 개선점 도출


---


## 참고 자료


- [Ultralytics YOLO11 공식 문서](https://docs.ultralytics.com/models/yolo11/)
- [COCO Dataset](https://cocodataset.org/)
- [Roboflow](https://roboflow.com/)
- [LabelImg GitHub](https://github.com/heartexlabs/labelImg)
- [YOLO 학습 가이드 (Ultralytics)](https://docs.ultralytics.com/modes/train/)


---


이전: [Week 3 PRACTICE](../week3/PRACTICE.md)
다음: [Week 5 PRACTICE](../week5/PRACTICE.md)
