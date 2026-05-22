# Week 8: Depth Anything 사용 (Section 5.3)

> **이번 주 목표**: Depth Anything 모델로 실제 이미지의 깊이맵을 추론하고 시각화하기
> **예상 시간**: 12시간
> **핵심 질문**: "Depth Anything으로 추정한 깊이맵을 실제로 어떻게 활용할 수 있을까?"

---

## 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | HuggingFace Pipeline, Depth Map 시각화 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | Depth 추론 파이프라인, Metric Depth 변환 심화 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | Depth Anything 모델 사용 및 시각화 |

---

## 시작하기 전에

### Week 7 복습

Week 7은 단안 깊이 추정의 이론 - MiDaS, DPT, Depth Anything의 원리 - 을 봤다. Week 8은 그 Depth Anything을 실제로 돌린다.

```
Week 7: Monocular Depth Estimation 이론
  → MiDaS, DPT, Depth Anything 모델 이해
  → Relative vs Metric Depth 구분
  → Teacher-Student 학습 프레임워크 이해


이번 주: 이론 → 실전!
  → HuggingFace에서 모델 다운로드
  → 단일 이미지 깊이 추론
  → Depth map 시각화
  → Metric Depth 변환 실험
```

이번 주에 답할 질문들:

```
Q1: HuggingFace에서 모델을 어떻게 불러오나?
Q2: 이미지 한 장에서 깊이맵을 어떻게 만드나?
Q3: 깊이맵을 시각적으로 어떻게 표현하나?
Q4: Relative Depth를 Metric Depth로 변환할 수 있나?
Q5: Fine-tuning은 언제, 어떻게 하나?
```

---

## 핵심 개념 자세히 알아보기

### 1. HuggingFace Pipeline으로 간단 추론

**HuggingFace**는 사전학습 모델을 공유하는 플랫폼이다. 그 중 `pipeline` API는 가장 간단한 사용법 - 모델 로드부터 추론까지 몇 줄로 끝낸다.

```python
from transformers import pipeline   # HuggingFace의 pipeline 함수

# 깊이 추정 파이프라인 생성: 모델 로드 + 전처리/후처리 설정을 한 번에
depth_pipe = pipeline(
    task="depth-estimation",                     # 수행할 작업 종류
    model="LiheYoung/depth-anything-small-hf",    # 사용할 모델 (HuggingFace 모델 ID)
)

# 추론: 이미지 경로를 넣으면 결과 딕셔너리를 돌려준다
result = depth_pipe("image.jpg")
depth_map = result["depth"]   # 결과의 "depth" 키에 깊이맵(PIL Image)이 들어 있음

# depth_map: 깊이맵 이미지 (가까울수록 밝게 표현됨)
```

`quiz_easy.py` 문제 1이 이 코드의 빈칸 - `pipeline`, `'depth-estimation'`, `'depth'` - 을 채우게 한다.

`pipeline`이 내부적으로 하는 일을 펼쳐 보면:

```
1. 모델 다운로드 (최초 1회, 캐시됨)
2. Image Processor 로드 (전처리 설정)
3. 입력 이미지 로드 + 전처리
   - Resize (518x518)
   - Normalize (ImageNet mean/std)
   - Tensor 변환
4. 모델 추론 (forward pass)
5. 후처리
   - 깊이맵 리사이즈 (원본 크기)
   - 정규화 (0-1 → 0-255)
   - PIL Image 변환
```

즉 `pipeline`은 전처리와 후처리를 알아서 다 해 준다. 빠른 프로토타이핑에 좋다.

---

### 2. 직접 모델 로드 (상세 제어)

`pipeline`은 편하지만 세밀한 제어가 안 된다. 배포용으로는 모델과 전처리기를 따로 불러 직접 다룬다.

```python
from transformers import AutoModelForDepthEstimation, AutoImageProcessor
import torch          # PyTorch 본체
from PIL import Image # 이미지 로드용 (PIL은 RGB 순서로 읽음)

# 모델과 이미지 전처리기를 각각 로드
model = AutoModelForDepthEstimation.from_pretrained(
    "LiheYoung/depth-anything-small-hf"
)
processor = AutoImageProcessor.from_pretrained(
    "LiheYoung/depth-anything-small-hf"
)
model.eval()   # 추론 모드로 전환 (BatchNorm/Dropout 고정 - 반드시 호출!)

# 이미지 로드 및 전처리
image = Image.open("image.jpg")                       # PIL로 이미지 열기 (RGB)
inputs = processor(images=image, return_tensors="pt") # 전처리기가 resize+정규화+텐서화

# 추론
with torch.no_grad():            # gradient 계산 끔 (추론이므로 불필요, 메모리 절약)
    outputs = model(**inputs)    # 모델에 전처리된 입력을 넣어 실행

# 결과 추출
depth = outputs.predicted_depth  # 예측된 깊이맵 [1, H, W] 텐서

# 모델 출력은 작은 크기이므로 원본 이미지 크기로 다시 키운다
depth = torch.nn.functional.interpolate(
    depth.unsqueeze(1),          # 채널 축 추가 [1, 1, H, W]
    size=image.size[::-1],       # 목표 크기 (PIL의 (W,H)를 (H,W)로 뒤집음)
    mode="bicubic",              # 보간 방식 (부드럽게 확대)
    align_corners=False,
).squeeze()                      # 추가했던 축 제거
```

직접 로드의 장점:

```
1. 배치 처리 가능 (여러 이미지 동시 추론)
2. GPU 사용 제어 (model.to("cuda"))
3. 중간 출력값 접근 가능
4. 커스텀 전처리/후처리 적용
5. ONNX/TensorRT 변환을 위한 모델 접근
```

`quiz_medium.py` 문제 2가 잘못된 전처리 코드를 고치게 한다. 위 코드가 올바른 형태다 - `model.eval()`, `with torch.no_grad()`, `processor` 사용, PIL로 RGB 로드가 핵심이다 (자세한 디버깅 포인트는 §전처리의 중요성).

---

### 3. 모델 크기별 비교

Depth Anything도 YOLO처럼 크기가 여러 가지다 (Week 7 §5 참고).

```
+------------------+----------+----------------------------+----------+
| 모델 | 파라미터 | HF 모델명 | 추론 속도 |
+------------------+----------+----------------------------+----------+
| ViT-S (Small) | 24.8M | depth-anything-small-hf | ~12ms |
| ViT-B (Base) | 97.5M | depth-anything-base-hf | ~25ms |
| ViT-L (Large) | 335.3M | depth-anything-large-hf | ~48ms |
+------------------+----------+----------------------------+----------+


선택 기준:
  Jetson 실시간 → ViT-S (Small)
  데스크톱 고품질 → ViT-B (Base)
  연구/최고 품질 → ViT-L (Large)
```

`quiz_easy.py` 문제 4가 상황별 모델 선택을 묻는다 - Jetson 실시간은 ViT-S, 서버 최고품질은 ViT-L, 데스크톱 균형은 ViT-B다.

---

### 4. Depth Map 시각화

깊이맵은 픽셀마다 깊이값 하나를 가진 **단일 채널** 데이터다. 그대로는 회색조라 보기 어렵다. **컬러맵(colormap)**으로 색을 입혀 본다.

```
깊이맵은 단일 채널 → 컬러맵으로 색상 매핑


자주 쓰는 컬러맵:
+----------+--------------------------------+
| magma | 어두운 보라 → 밝은 노랑 |
| | 논문에서 가장 많이 사용 |
+----------+--------------------------------+
| turbo | 파랑 → 초록 → 빨강 |
| | 직관적 (빨강=가까움, 파랑=멀리) |
+----------+--------------------------------+
| viridis | 보라 → 초록 → 노랑 |
| | 색각 이상 친화적 |
+----------+--------------------------------+
```

`quiz_easy.py` 문제 3이 컬러맵과 특징을 연결하게 한다 - magma=논문 표준, turbo=직관적, viridis=색각 이상 친화적.

```python
import matplotlib.pyplot as plt   # 컬러맵 제공
import numpy as np

def visualize_depth(depth_map, colormap="magma"):
    """깊이맵에 컬러맵을 입혀 RGB 이미지로 변환한다.

    Args:
        depth_map: 깊이값이 담긴 2D numpy 배열 (H, W)
        colormap: 적용할 컬러맵 이름

    Returns:
        컬러맵이 적용된 RGB 이미지 (H, W, 3), uint8
    """
    # 1. 0-1 범위로 정규화 (최솟값을 0, 최댓값을 1로)
    depth_norm = (depth_map - depth_map.min()) / (depth_map.max() - depth_map.min())

    # 2. 컬러맵 함수를 가져와 정규화된 깊이에 적용 -> RGBA 출력
    cmap = plt.get_cmap(colormap)
    colored = cmap(depth_norm)[:, :, :3]   # 마지막 알파(A) 채널 버리고 RGB만

    # 3. [0,1] 실수를 [0,255] uint8 이미지로 변환
    colored = (colored * 255).astype(np.uint8)

    return colored

# 사용 예시
depth_colored = visualize_depth(depth_numpy, "magma")
```

---

### 5. Metric Depth vs Relative Depth 변환

Week 7 §6에서 봤듯, Depth Anything 기본 출력은 **상대 깊이(Relative)**다. 실제 거리(미터)가 필요하면 **절대 깊이(Metric)**로 변환해야 한다.

변환의 기본 발상은 **선형 변환**이다. 상대 깊이 `d_relative`에 배율(alpha)을 곱하고 이동(beta)을 더해 미터 깊이를 만든다.

```
d_metric = alpha * d_relative + beta


alpha (scale): 깊이의 배율
beta (shift): 깊이의 이동
```

미지수가 alpha, beta 두 개이므로, **방정식 두 개 = 참조점 두 개**가 필요하다. 참조점이란 "상대 깊이도 알고 실제 거리도 아는 픽셀"이다.

```
계산 방법 (참조점 2개로 연립방정식):
  d1_metric = alpha * d1_rel + beta
  d2_metric = alpha * d2_rel + beta


  alpha = (d1_metric - d2_metric) / (d1_rel - d2_rel)
  beta  = d1_metric - alpha * d1_rel
```

`quiz_medium.py` 문제 1이 이 계산을 시킨다. 참조점 A(상대 0.85, 실제 1.5m)와 B(상대 0.30, 실제 6.0m)가 주어지면, `alpha = (1.5-6.0)/(0.85-0.30) ≈ -8.18`, `beta = 1.5 - (-8.18)*0.85 ≈ 8.45`다. alpha가 음수인 이유는, 상대 깊이는 "큰 값=가까움"인데 미터 깊이는 "작은 값=가까움"이라 방향이 반대이기 때문이다.

참조점 없이 변환하는 다른 방법들:

```
방법 1: Known Object Reference (알려진 물체 활용)
  사람 키 ~170cm처럼 크기를 아는 물체로 scale 계산


방법 2: Camera Intrinsics 활용
  초점 거리와 실제 크기로 거리 계산:
  distance = (focal_length * real_size) / pixel_size


방법 3: Metric Depth 모델 사용 (가장 정확)
  Depth Anything v2 Metric 모델 → 직접 미터 단위 출력
```

이 선형 변환의 한계도 알아 두자: 실제 깊이-상대깊이 관계가 비선형일 수 있고, 참조점이 부정확하면 전체가 틀어지며, 카메라 위치가 바뀌면 보정이 무효가 된다.

---

### 6. Fine-tuning (선택 사항)

Depth Anything은 Zero-shot 모델이라 보통 그대로 써도 된다. 특수 환경에서만 Fine-tuning(추가 학습)을 고려한다.

```
Fine-tuning 불필요한 경우:
  - 일반적인 실내/야외 장면
  - 장애물 유무 판단 (상대 깊이로 충분)


Fine-tuning 필요한 경우:
  특수 환경 (수중, 야간, 안개)
  특수 카메라 (어안렌즈, 적외선)
  높은 Metric 정확도가 필요
  특정 깊이 범위에 집중 (0-3m 실내)
```

Fine-tuning에는 깊이 추정 전용 손실 함수를 쓴다.

```
Scale-Invariant Log Loss (Week 7 §꼭 이해 참고):
  d_i = log(pred_i) - log(gt_i)
  L = sqrt( (1/n)*sum(d_i^2) - (lambda/n^2)*(sum(d_i))^2 )


추가 손실:
  Gradient Loss: 깊이맵의 경계(edge)를 보존
  SSIM Loss: 구조적 유사도


  총 손실: L_total = L_si + w1*L_grad + w2*L_ssim
```

대표 Fine-tuning 데이터셋은 KITTI(야외, 0-80m)와 NYU Depth V2(실내, 0-10m)다.

---

### 7. 실시간 비디오 깊이 추정

카메라 영상에 프레임마다 깊이 추정을 적용하면 실시간 깊이 영상이 된다.

```python
import cv2                          # 카메라/영상 처리
from transformers import pipeline

# 깊이 추정 파이프라인 (device=0 으로 GPU 사용)
depth_pipe = pipeline(
    "depth-estimation",
    model="LiheYoung/depth-anything-small-hf",
    device=0,                       # 0 = 첫 번째 GPU
)

cap = cv2.VideoCapture(0)           # 0번 카메라 열기

while True:
    ret, frame = cap.read()         # 한 프레임 읽기 (ret: 성공 여부)
    if not ret:                     # 읽기 실패하면 루프 종료
        break

    # OpenCV는 BGR 순서로 읽으므로 RGB로 변환 (모델은 RGB를 기대)
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # 깊이 추정
    result = depth_pipe(rgb)
    depth_image = result["depth"]   # 깊이맵

    # 시각화 (§4의 visualize_depth 등으로 색 입혀 화면 출력)
    # ...

    if cv2.waitKey(1) & 0xFF == ord("q"):  # 'q' 키를 누르면 종료
        break
```

성능 참고:

```
데스크톱 GPU (RTX 3060):
  ViT-S: ~15ms → ~66 FPS
  ViT-B: ~30ms → ~33 FPS


Jetson Orin Nano (PyTorch):
  ViT-S: ~100ms → ~10 FPS
  → TensorRT FP16 필요! (Week 9에서 다룸)
```

---

### 8. YOLO + Depth 결합 (미리보기)

이번 Phase의 최종 그림이다. YOLO로 "무엇이 어디에"를 찾고, Depth로 "얼마나 멀리"를 알면, 장애물 회피가 가능해진다.

```
YOLO 결과: [사람, x1=100, y1=50, x2=300, y2=400, conf=0.92]
                                    ↓
깊이맵에서 해당 영역의 깊이 추출:
  roi_depth = depth_map[50:400, 100:300]
  → 이 영역의 깊이값으로 사람까지의 거리 판단


결합 결과:
  "사람이 감지됨, 가까움!" → 장애물 회피 시스템에 전달
```

여기서 한 가지 설계 결정이 있다 - BBox 영역의 깊이값을 어떻게 하나의 "거리"로 요약할까? 평균(mean)? 중앙값(median)? 최솟값(min)?

`quiz_medium.py` 문제 3이 이를 묻는다. 장애물 회피에서는 **가장 가까운 지점**(min, 또는 하위 10% percentile)이 적합하다. 평균은 BBox에 포함된 먼 배경 때문에 거리가 실제보다 멀게 왜곡될 수 있어, 안전을 위해서는 "최악의 경우(가장 가까운 부분)"를 기준으로 판단해야 한다.

```
향후 파이프라인 (Phase 3 전체):


카메라 → YOLO (객체 검출) → '무엇이' 있는지
      → Depth Anything → '얼마나 멀리' 있는지
                         ↓
              장애물 회피 / 경로 계획
```

---

## 꼭 이해해야 할 핵심 개념

### HuggingFace 모델 캐싱

HuggingFace 모델은 처음 한 번만 다운로드되고, 이후엔 로컬 캐시에서 불러온다.

```
최초 다운로드:
  ~/.cache/huggingface/hub/ 에 모델 저장
  ViT-S: ~100MB, ViT-B: ~400MB, ViT-L: ~1.4GB


오프라인 사용:
  export TRANSFORMERS_OFFLINE=1
  → 캐시된 모델만 사용 (인터넷 불필요)


캐시 경로 변경:
  export HF_HOME=/custom/path
```

### 전처리의 중요성

전처리가 틀리면 모델 출력이 완전히 엉뚱해진다. `quiz_medium.py` 문제 2가 묻는 잘못된 전처리 코드의 버그가 바로 이것들이다.

```
Depth Anything 올바른 전처리:
  1. 색 순서: BGR → RGB 변환
     OpenCV(cv2)는 BGR로 읽지만 모델은 RGB를 기대
     → cv2.cvtColor(image, cv2.COLOR_BGR2RGB) 또는 PIL.Image 사용
  2. Resize: 518x518 (ViT 패치 크기 14의 배수)
  3. Normalize: ImageNet 통계로 정규화
     단순 /255.0 만으로는 부족 - mean/std 정규화까지 필요
     mean = [0.485, 0.456, 0.406], std = [0.229, 0.224, 0.225]
  4. 추론 시 model.eval() + torch.no_grad() 필수


핵심: AutoImageProcessor를 쓰면 1-3을 올바르게 자동 처리한다.
      직접 전처리하면 위 항목을 빠뜨리기 쉽다.
```

전처리 디버깅 체크리스트 (quiz_medium 문제 2의 4가지 버그):

1. **BGR -> RGB 변환 누락** - OpenCV로 읽은 이미지는 BGR이다.
2. **ImageNet 정규화 누락** - `/255.0`만으로는 부족, mean/std 정규화 필요. `AutoImageProcessor`가 자동 처리.
3. **`model.eval()` 누락** - 추론 모드 전환 필수.
4. **`torch.no_grad()` 미사용** - 추론 시 gradient 불필요, 메모리 낭비.

---

## 자체 점검 - 이해했는지 확인!

### Q1: Pipeline vs 직접 로드
**Q:** HuggingFace pipeline과 직접 모델 로드의 차이는?

**A:**
```
Pipeline:
  - 한 줄로 추론 가능 (간편)
  - 전처리/후처리 자동 처리
  - 빠른 프로토타이핑에 적합


직접 로드:
  - 배치 처리 가능
  - GPU 제어 가능 (model.to("cuda"))
  - 중간 출력 접근, 커스텀 전처리/후처리
  - ONNX 변환을 위한 모델 접근


→ 프로토타이핑: pipeline / 실제 배포: 직접 로드
```

### Q2: 깊이맵 해석
**Q:** Depth Anything 깊이맵에서 값이 큰 픽셀은 가까운 건가요, 먼 건가요?

**A:**
```
Depth Anything 기본 출력: 큰 값 = 가까운 물체 (disparity와 유사)


주의:
  - 기본 출력은 상대 깊이 → 미터 단위가 아님
  - 깊이맵은 (H, W) 단일 채널
  - 후처리 방식에 따라 값이 반전될 수 있음
  → 항상 모델 문서 확인 + 시각화로 검증
```

### Q3: Metric 변환
**Q:** 상대 깊이에서 절대 깊이로 변환하려면 최소 몇 개의 참조점이 필요한가요?

**A:**
```
최소 2개의 참조점!


d_metric = alpha * d_relative + beta (선형 변환)
→ 미지수 2개 (alpha, beta)
→ 방정식 2개 필요 → 참조점 2개


더 많은 참조점이 있으면 최소제곱법으로 더 정확하게 보정 가능
```

### Q4: Fine-tuning 필요성
**Q:** 공장 내부에서 깊이 추정을 하려면 Fine-tuning이 필요한가요?

**A:**
```
일반적인 공장 환경이라면:
  → Fine-tuning 없이 Zero-shot으로 충분할 수 있음
  → Depth Anything은 매우 다양한 환경에서 학습됨


Fine-tuning이 필요한 특수 상황:
  - 반사가 많은 금속 표면
  - 매우 좁은 깊이 범위 (0-2m만 정밀하게)
  - 특수 조명 (적외선, UV), 투명한 물체


추천 순서: Zero-shot 먼저 시도 → 불만족 시 Fine-tuning 검토
```

---

## 이번 주 실습 & 다음 주 준비

### 실습 항목

1. **HuggingFace Pipeline 추론**
   - Depth Anything 모델 다운로드
   - 단일 이미지 깊이 추론

2. **직접 모델 로드 및 추론**
   - AutoModelForDepthEstimation 사용
   - 배치 처리, GPU 추론

3. **Depth Map 시각화**
   - 다양한 컬러맵 비교 (magma, turbo, inferno)
   - 원본 이미지와 나란히 시각화

4. **Metric Depth 변환 실험**
   - 참조점 기반 스케일 보정
   - YOLO + Depth 결합 (미리보기)

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

### 다음 주 준비

```
Week 9에서는 Depth Anything을 ONNX/TensorRT로 변환하여
Jetson에서 실시간 추론합니다!


준비:
  Week 8 실습 완료 (모델 동작 확인)
  ONNX 변환 복습 (Week 5)
  TensorRT 변환 복습 (Week 6)
```

---

## 이번 주 핵심 요약

1. **HuggingFace Pipeline으로 간단 추론**
   - `pipeline("depth-estimation", model="...")`
   - 한 줄로 깊이맵 생성, 전처리/후처리 자동

2. **직접 모델 로드로 세밀한 제어**
   - AutoModelForDepthEstimation + AutoImageProcessor
   - `model.eval()` + `torch.no_grad()` 필수

3. **Depth Map 시각화**
   - 단일 채널 깊이맵에 컬러맵 적용
   - magma(논문), turbo(직관적), viridis(색각 친화)

4. **Metric vs Relative Depth**
   - 기본 출력: Relative (상대)
   - Metric 변환: `d_metric = alpha*d_rel + beta`, 참조점 2개 필요

5. **전처리 주의 + YOLO+Depth 결합**
   - BGR->RGB, ImageNet 정규화, eval/no_grad
   - BBox 영역 깊이는 min(가장 가까운 점)으로 요약

---

이전: [Week 7 - Monocular Depth 이론](../week7/README.md)
다음: [Week 9 - ONNX & TensorRT 변환: Depth 모델](../week9/README.md)
