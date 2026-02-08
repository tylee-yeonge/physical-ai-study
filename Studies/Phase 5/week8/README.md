# Week 8: Depth Anything 사용 (Section 5.3)

> 🎯 **이번 주 목표**: Depth Anything 모델로 실제 이미지의 깊이맵을 추론하고 시각화하기
> ⏰ **예상 시간**: 12시간
> 💡 **핵심 질문**: "Depth Anything으로 추정한 깊이맵을 실제로 어떻게 활용할 수 있을까?"

---

## 🌟 시작하기 전에

### Week 7 복습

**지금까지의 흐름:**
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

**Week 8에서 답할 질문:**
```
Q1: HuggingFace에서 모델을 어떻게 불러오나?
Q2: 이미지 한 장에서 깊이맵을 어떻게 만드나?
Q3: 깊이맵을 시각적으로 어떻게 표현하나?
Q4: Relative Depth를 Metric Depth로 변환할 수 있나?
Q5: Fine-tuning은 언제, 어떻게 하나?
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. HuggingFace Pipeline으로 간단 추론

**가장 쉬운 방법: pipeline API**

```python
from transformers import pipeline

# 한 줄로 모델 로드 + 추론 준비!
depth_pipe = pipeline(
    task="depth-estimation",
    model="LiheYoung/depth-anything-small-hf"
)

# 추론도 한 줄!
result = depth_pipe("image.jpg")
depth_map = result["depth"]  # PIL Image

# 결과: 깊이맵 이미지 (가까울수록 밝음)
```

**Pipeline이 내부적으로 하는 일:**
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
   - 정규화 (0~1 → 0~255)
   - PIL Image 변환
```

---

### 2. 직접 모델 로드 (상세 제어)

**더 세밀한 제어가 필요할 때:**

```python
from transformers import AutoModelForDepthEstimation, AutoImageProcessor
import torch
from PIL import Image

# 모델과 이미지 프로세서 로드
model = AutoModelForDepthEstimation.from_pretrained(
    "LiheYoung/depth-anything-small-hf"
)
processor = AutoImageProcessor.from_pretrained(
    "LiheYoung/depth-anything-small-hf"
)
model.eval()

# 이미지 로드 및 전처리
image = Image.open("image.jpg")
inputs = processor(images=image, return_tensors="pt")

# 추론
with torch.no_grad():
    outputs = model(**inputs)

# 결과 추출
depth = outputs.predicted_depth  # [1, H, W] 텐서

# 원본 크기로 리사이즈
depth = torch.nn.functional.interpolate(
    depth.unsqueeze(1),
    size=image.size[::-1],  # (H, W)
    mode="bicubic",
    align_corners=False
).squeeze()
```

**직접 로드의 장점:**
```
1. 배치 처리 가능 (여러 이미지 동시 추론)
2. GPU 사용 제어 (model.to("cuda"))
3. 중간 출력값 접근 가능
4. 커스텀 전처리/후처리 적용
5. ONNX/TensorRT 변환을 위한 모델 접근
```

---

### 3. 모델 크기별 비교

**Depth Anything 변종:**

```
┌──────────────────┬──────────┬──────────┬──────────┬──────────┐
│  모델             │ 파라미터  │ HF 모델명              │ 추론 속도  │
├──────────────────┼──────────┼──────────┼──────────┼──────────┤
│ ViT-S (Small)    │  24.8M   │ depth-anything-small-hf│  ~12ms   │
│ ViT-B (Base)     │  97.5M   │ depth-anything-base-hf │  ~25ms   │
│ ViT-L (Large)    │  335.3M  │ depth-anything-large-hf│  ~48ms   │
└──────────────────┴──────────┴──────────┴──────────┴──────────┘

Depth Anything v2:
┌──────────────────┬──────────┬──────────────────────────────────┐
│ ViT-S v2         │  24.8M   │ depth-anything-v2-small-hf       │
│ ViT-B v2         │  97.5M   │ depth-anything-v2-base-hf        │
│ ViT-L v2         │  335.3M  │ depth-anything-v2-large-hf       │
└──────────────────┴──────────┴──────────────────────────────────┘

선택 기준:
  Jetson 실시간   → ViT-S (Small)
  데스크톱 고품질 → ViT-B (Base)
  연구/최고 품질  → ViT-L (Large)
```

---

### 4. Depth Map 시각화

**컬러맵(Colormap) 선택:**

```
깊이맵은 단일 채널 → 컬러맵으로 색상 매핑

자주 쓰는 컬러맵:
┌──────────┬────────────────────────────────┐
│ magma    │ 어두운 보라 → 밝은 노랑       │
│          │ 논문에서 가장 많이 사용          │
├──────────┼────────────────────────────────┤
│ inferno  │ 검정 → 빨강 → 노랑            │
│          │ 고대비, 프레젠테이션용            │
├──────────┼────────────────────────────────┤
│ turbo    │ 파랑 → 초록 → 빨강            │
│          │ 직관적 (빨강=가까움, 파랑=멀리)  │
├──────────┼────────────────────────────────┤
│ viridis  │ 보라 → 초록 → 노랑            │
│          │ 색각 이상 친화적                  │
└──────────┴────────────────────────────────┘
```

**시각화 코드:**
```python
import matplotlib.pyplot as plt
import numpy as np

def visualize_depth(depth_map, colormap='magma'):
    """깊이맵 시각화"""
    # 정규화 (0~1)
    depth_norm = (depth_map - depth_map.min()) / (depth_map.max() - depth_map.min())

    # 컬러맵 적용
    cmap = plt.get_cmap(colormap)
    colored = cmap(depth_norm)[:, :, :3]  # RGB만 (alpha 제거)
    colored = (colored * 255).astype(np.uint8)

    return colored

# 사용
depth_colored = visualize_depth(depth_numpy, 'magma')
```

---

### 5. Metric Depth vs Relative Depth 변환

**Relative → Metric 변환 방법:**

```
방법 1: Known Object Reference (알려진 물체 활용)
  1. 이미지에서 크기를 아는 물체 찾기 (예: 사람 키 170cm)
  2. 해당 물체의 상대 깊이값 읽기 (예: d_rel = 0.75)
  3. 실제 거리와 비교하여 scale 계산
     scale = d_metric_known / d_rel_known
  4. 전체 깊이맵에 scale 적용
     d_metric = scale * d_rel + shift

방법 2: Camera Intrinsics 활용
  focal_length (초점 거리)와 object_size (실제 크기)로
  실제 거리를 계산:
  distance = (focal_length * real_size) / pixel_size

방법 3: Metric Depth 모델 사용 (가장 정확)
  Depth Anything v2 Metric 모델:
  → 직접 미터 단위 깊이 출력
  → 실내용/야외용 모델 별도 제공
```

**Scale-Shift 보정 공식:**
```
d_metric = alpha * d_relative + beta

alpha (scale): 깊이의 배율
beta (shift): 깊이의 이동

계산 방법 (최소 2개 참조점 필요):
  d1_metric = alpha * d1_rel + beta
  d2_metric = alpha * d2_rel + beta

  alpha = (d1_metric - d2_metric) / (d1_rel - d2_rel)
  beta = d1_metric - alpha * d1_rel
```

---

### 6. Fine-tuning (선택 사항)

**언제 Fine-tuning이 필요한가?**

```
Fine-tuning 불필요한 경우:
  ✅ 일반적인 실내/야외 장면
  ✅ 장애물 유무 판단 (상대 깊이로 충분)
  ✅ 대략적인 깊이 순서만 필요

Fine-tuning 필요한 경우:
  ❌ 특수 환경 (수중, 야간, 안개)
  ❌ 특수 카메라 (어안렌즈, 적외선)
  ❌ 높은 Metric 정확도가 필요
  ❌ 특정 깊이 범위에 집중 (0~3m 실내)
```

**Fine-tuning 데이터셋:**

```
KITTI (야외):
  - 자율주행 데이터 (독일 도로)
  - LiDAR Ground Truth
  - 깊이 범위: 0~80m
  - 해상도: 375x1242

NYU Depth V2 (실내):
  - 실내 장면 (Microsoft Kinect)
  - RGB-D Ground Truth
  - 깊이 범위: 0~10m
  - 해상도: 480x640
```

**Fine-tuning 손실 함수:**

```
Scale-Invariant Log Loss:

d_i = log(pred_i) - log(gt_i)

L = sqrt( (1/n) * sum(d_i^2) - (lambda/n^2) * (sum(d_i))^2 )

lambda = 0.5 (일반적)

의미:
  - 로그 공간에서 오차 측정 → 가까운 곳의 오차가 더 중요
  - 전체 스케일 차이는 무시 (scale-invariant)
  - lambda가 1에 가까울수록 더 scale-invariant

추가 손실:
  Gradient Loss: 깊이맵의 경계(edge)를 보존
  L_grad = |grad(pred) - grad(gt)|

  SSIM Loss: 구조적 유사도
  L_ssim = 1 - SSIM(pred, gt)

  총 손실:
  L_total = L_si + w1 * L_grad + w2 * L_ssim
```

---

### 7. 실시간 비디오 깊이 추정

**카메라 입력으로 실시간 깊이맵:**

```python
import cv2
from transformers import pipeline

# 깊이 추정 파이프라인
depth_pipe = pipeline(
    "depth-estimation",
    model="LiheYoung/depth-anything-small-hf",
    device=0  # GPU 사용
)

cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # BGR → RGB
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # 깊이 추정
    result = depth_pipe(rgb)
    depth_image = result["depth"]

    # 시각화
    # ...

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
```

**성능 참고:**
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

**객체 검출 + 깊이 정보:**

```
YOLO 결과: [사람, x1=100, y1=50, x2=300, y2=400, conf=0.92]
                                    ↓
깊이맵에서 해당 영역의 깊이 추출:
  roi_depth = depth_map[50:400, 100:300]
  mean_depth = roi_depth.mean()  → 0.82 (상대 깊이)

결합 결과:
  "사람이 감지됨, 상대 깊이 0.82 (매우 가까움!)"
  → 장애물 회피 시스템에 전달
```

```
향후 파이프라인 (Phase 5 전체):

카메라 → YOLO (객체 검출) → '무엇이' 있는지
     ↘                      ↗
      Depth Anything → '얼마나 멀리' 있는지
                         ↓
              장애물 회피 / 경로 계획
```

---

## 💡 꼭 이해해야 할 핵심 개념

### HuggingFace 모델 캐싱

```
최초 다운로드:
  ~/.cache/huggingface/hub/ 에 모델 저장
  ViT-S: ~100MB, ViT-B: ~400MB, ViT-L: ~1.4GB

오프라인 사용:
  # 환경 변수 설정
  export TRANSFORMERS_OFFLINE=1
  → 캐시된 모델만 사용 (인터넷 불필요)

캐시 경로 변경:
  export HF_HOME=/custom/path
```

### 전처리의 중요성

```
Depth Anything 전처리:
  1. Resize: 518x518 (ViT 패치 크기 14의 배수 + 여유)
  2. Normalize: ImageNet 통계
     mean = [0.485, 0.456, 0.406]
     std  = [0.229, 0.224, 0.225]
  3. ToTensor: [0,255] → [0,1] → 정규화

주의:
  - 전처리를 잘못하면 결과가 완전히 틀림!
  - AutoImageProcessor가 올바른 전처리를 자동 적용
  - 직접 전처리할 때는 반드시 모델의 설정 확인
```

---

## 🔍 자체 점검 - 이해했는지 확인!

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
  - 중간 출력 접근 가능
  - ONNX 변환을 위한 모델 접근
  - 커스텀 전처리/후처리

→ 프로토타이핑: pipeline
→ 실제 배포: 직접 로드
```

### Q2: 깊이맵 해석
**Q:** 깊이맵에서 값이 0.9인 픽셀은 가까운 건가요, 먼 건가요?

**A:**
```
모델에 따라 다름!

Depth Anything (기본 출력):
  큰 값 = 가까운 물체 (disparity와 유사)
  작은 값 = 먼 물체

하지만 후처리 방식에 따라 반전 가능:
  depth_inverted = 1.0 - depth  (반전 시 큰 값 = 멀리)

항상 모델 문서를 확인하고,
시각화해서 가까운/먼 물체가 올바른지 검증!
```

### Q3: Metric 변환
**Q:** 상대 깊이에서 절대 깊이로 변환하려면 최소 몇 개의 참조점이 필요한가요?

**A:**
```
최소 2개의 참조점!

d_metric = alpha * d_relative + beta (선형 변환)
→ 미지수 2개 (alpha, beta)
→ 방정식 2개 필요 → 참조점 2개

참조점 예시:
  바닥 (0m, d_rel=0.95) + 천장 (2.5m, d_rel=0.1)

더 많은 참조점이 있으면:
  → 최소제곱법(least squares)으로 더 정확한 보정
  → 또는 비선형 변환 (다항식) 적용 가능
```

### Q4: Fine-tuning 필요성
**Q:** 공장 내부에서 깊이 추정을 하려면 Fine-tuning이 필요한가요?

**A:**
```
일반적인 공장 환경이라면:
  → Fine-tuning 없이 Zero-shot으로 충분할 수 있음
  → Depth Anything은 매우 다양한 환경에서 학습됨

Fine-tuning이 필요한 특수 상황:
  - 반사가 많은 금속 표면 (깊이 추정 혼란)
  - 매우 좁은 깊이 범위 (0~2m만 정밀하게)
  - 특수 조명 (적외선, UV)
  - 투명한 물체 (유리 컨테이너 등)

추천 순서:
  1. Zero-shot 먼저 시도 (충분할 수 있음)
  2. 결과가 불만족스러우면 Fine-tuning 검토
  3. Fine-tuning 데이터: 공장 환경 RGB + RGB-D 카메라 GT
```

---

## 📝 이번 주 실습 & 다음 주 준비

### 실습 항목

1. **HuggingFace Pipeline 추론**
   - Depth Anything 모델 다운로드
   - 단일 이미지 깊이 추론
   - Pipeline API 사용법 익히기

2. **직접 모델 로드 및 추론**
   - AutoModelForDepthEstimation 사용
   - 배치 처리 구현
   - GPU 추론 테스트

3. **Depth Map 시각화**
   - 다양한 컬러맵 비교 (magma, turbo, inferno)
   - 원본 이미지와 나란히 시각화
   - 3D 시각화 (선택)

4. **Metric Depth 변환 실험**
   - 참조점 기반 스케일 보정
   - 깊이 분포 분석
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
  Jetson 환경 준비 (JetPack SDK)
```

---

## 🎯 이번 주 핵심 요약

1. **HuggingFace Pipeline으로 간단 추론**
   - `pipeline("depth-estimation", model="...")`
   - 한 줄로 깊이맵 생성 가능
   - 프로토타이핑에 최적

2. **직접 모델 로드로 세밀한 제어**
   - AutoModelForDepthEstimation
   - AutoImageProcessor
   - 배치/GPU/커스텀 후처리 가능

3. **Depth Map 시각화**
   - 컬러맵: magma (논문), turbo (직관적)
   - 정규화 후 시각화 (0~1 범위)
   - 원본과 나란히 비교

4. **Metric vs Relative Depth**
   - Depth Anything 기본: Relative (상대)
   - Metric 변환: scale + shift 보정 (참조점 2개+)
   - v2 Metric 모델: 직접 미터 단위 출력

5. **Fine-tuning (선택)**
   - 데이터셋: KITTI (야외), NYU (실내)
   - 손실: Scale-Invariant Log Loss
   - Zero-shot으로 먼저 시도 → 필요 시 Fine-tuning

---

이전: [Week 7 - Monocular Depth 이론](../week7/README.md)
다음: [Week 9 - ONNX & TensorRT 변환: Depth 모델](../week9/README.md)
