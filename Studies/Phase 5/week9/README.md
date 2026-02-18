# Week 9: ONNX & TensorRT 변환 - Depth 모델 (C++)

> 🎯 **이번 주 목표**: Depth Anything 모델을 ONNX로 변환하고, TensorRT FP16으로 최적화하여 Jetson에서 15-20 FPS 달성하기
> ⏰ **예상 시간**: 12시간
> 💡 **핵심 질문**: "Depth 모델을 Jetson에서 실시간으로 돌리려면 어떤 최적화 파이프라인이 필요한가?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | ONNX-TensorRT 변환, FP16 최적화 개념 확인 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | Depth 모델 TensorRT 엔진 빌드 심화 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | ONNX & TensorRT 변환 - Depth 모델 (C++) |

---

## 🌟 시작하기 전에

### Week 8에서 배운 것

**Depth Anything 모델 이해:**
```
Depth Anything = ViT 기반 Monocular Depth Estimation
- 단안 카메라 이미지 → 상대적 Depth Map
- HuggingFace에서 사전학습 모델 제공
- ViT-S / ViT-B / ViT-L 다양한 크기
```

**하지만 실제 배포에서는?**
```
❓ PyTorch 모델을 Jetson에서 바로 쓸 수 있나?
❓ ONNX가 뭐고, 왜 중간 단계가 필요한가?
❓ TensorRT FP16이 어떻게 속도를 2배 올리나?
❓ 입력 크기가 왜 중요한가?
```

**이번 주에 답합니다!**

---

## 📚 핵심 개념 자세히 알아보기

### 1. 모델 배포 파이프라인 전체 흐름

#### 1.1 왜 PyTorch를 바로 쓰지 않나?

```
PyTorch 모델 (.pt)
  - Python 의존성
  - 동적 그래프 (매번 그래프 생성)
  - 최적화 없음
  - Jetson에서 느림 (5-8 FPS)

vs.

TensorRT 엔진 (.trt)
  - C++ 네이티브
  - 정적 그래프 (미리 최적화)
  - Layer 융합, 커널 자동 튜닝
  - Jetson에서 빠름 (15-25 FPS)
```

#### 1.2 변환 파이프라인

```
[PyTorch .pt] → [ONNX .onnx] → [TensorRT .trt]
     ↑               ↑                ↑
  학습용          중간 포맷         배포용
  (Python)      (프레임워크      (GPU 최적화)
                 독립적)
```

**각 단계의 역할:**

| 단계 | 포맷 | 역할 |
|------|------|------|
| PyTorch | .pt | 학습, 실험, 디버깅 |
| ONNX | .onnx | 프레임워크 독립, 표준 포맷 |
| TensorRT | .trt | GPU 최적 추론, HW 특화 |

---

### 2. Depth Anything → ONNX 변환

#### 2.1 HuggingFace 모델 로드

```python
# PC에서 실행 (Python)
# 이 단계는 Python으로 수행, 결과 .onnx 파일을 Jetson에 전송

from transformers import AutoModelForDepthEstimation
import torch

model = AutoModelForDepthEstimation.from_pretrained(
    "LiheYoung/depth-anything-small-hf"
)
model.eval()
```

**모델 크기별 비교:**

| 모델 | 파라미터 | ONNX 크기 | Jetson 추론 시간 |
|------|---------|----------|-----------------|
| ViT-S (Small) | 24.8M | ~95MB | ~50ms |
| ViT-B (Base) | 97.5M | ~370MB | ~120ms |
| ViT-L (Large) | 335.3M | ~1.3GB | ~350ms |

**권장: ViT-S** (Jetson Orin Nano에서 실시간 가능)

#### 2.2 Input Size 고정이 중요한 이유

```
ViT (Vision Transformer)의 특성:
- 이미지를 패치(patch)로 분할
- 패치 크기: 보통 14×14 또는 16×16
- 입력 크기가 패치 크기의 배수여야 함

권장 입력 크기: 384 × 512
- 384 = 14 × ~27.4 → 패딩으로 조정
- 512 = 14 × ~36.6 → 패딩으로 조정
- 해상도 vs 속도 균형점

왜 고정 크기인가?
- TensorRT는 정적 입력 크기에서 최적화
- 동적 크기 → 매번 재최적화 → 느림
- 고정 크기 → 미리 최적화 → 빠름
```

#### 2.3 ONNX Export 코드

```python
# export_depth_onnx.py
import torch
from transformers import AutoModelForDepthEstimation

model = AutoModelForDepthEstimation.from_pretrained(
    "LiheYoung/depth-anything-small-hf"
)
model.eval()

# 고정 입력 크기: 384 × 512
H, W = 384, 512
dummy_input = torch.randn(1, 3, H, W)

torch.onnx.export(
    model,
    dummy_input,
    "depth_anything_small_384x512.onnx",
    opset_version=17,
    input_names=["pixel_values"],
    output_names=["predicted_depth"],
    dynamic_axes=None  # 동적 축 비활성화 (고정 크기)
)

print(f"ONNX 변환 완료: depth_anything_small_384x512.onnx")
print(f"입력 크기: (1, 3, {H}, {W})")
```

---

### 3. TensorRT FP16 변환

#### 3.1 FP32 vs FP16

```
FP32 (단정밀도):
  [1비트 부호][8비트 지수][23비트 가수]
  - 높은 정밀도
  - 메모리 4바이트/값
  - 느림

FP16 (반정밀도):
  [1비트 부호][5비트 지수][10비트 가수]
  - 약간의 정밀도 손실 (< 1%)
  - 메모리 2바이트/값 (절반!)
  - 속도 ~2배

Depth Estimation에서 FP16 영향:
  - 깊이 값의 상대적 순서는 유지
  - 절대값 오차 < 2%
  - SLAM에서 사용하기에 충분
```

#### 3.2 trtexec로 변환 (Jetson에서)

```bash
# Jetson에서 실행
/usr/src/tensorrt/bin/trtexec \
    --onnx=depth_anything_small_384x512.onnx \
    --saveEngine=depth_anything_small_fp16.trt \
    --fp16 \
    --workspace=4096 \
    --verbose

# 변환 결과 확인
# - 변환 시간: 5-15분 (Jetson Orin Nano)
# - 엔진 파일 크기: ~50MB (FP16)
```

#### 3.3 변환 옵션 상세

```bash
# 주요 옵션 설명
--onnx=<file>        # 입력 ONNX 파일
--saveEngine=<file>  # 출력 TRT 엔진 파일
--fp16               # FP16 활성화 (핵심!)
--workspace=4096     # GPU 워크스페이스 (MB)
--verbose            # 상세 로그

# 추가 최적화 옵션
--best               # FP16 + INT8 동시 시도
--minShapes=...      # 최소 입력 크기
--optShapes=...      # 최적 입력 크기
--maxShapes=...      # 최대 입력 크기
```

---

### 4. C++ TensorRT 추론 엔진

#### 4.1 TensorRT 추론 흐름

```
[이미지 입력]
    ↓
[전처리 (OpenCV)]
  - resize → 384 × 512
  - BGR → RGB
  - normalize (mean, std)
  - NCHW 변환
    ↓
[GPU 메모리 복사]
  - cudaMemcpy (Host → Device)
    ↓
[TensorRT 추론]
  - context->enqueueV2()
    ↓
[GPU → Host 복사]
  - cudaMemcpy (Device → Host)
    ↓
[후처리]
  - Depth map 시각화
  - 값 정규화
```

#### 4.2 핵심 클래스 구조

```cpp
class DepthTRTEngine {
private:
    nvinfer1::ICudaEngine* engine_;       // TRT 엔진
    nvinfer1::IExecutionContext* context_; // 실행 컨텍스트
    void* buffers_[2];                     // GPU 버퍼 (입/출력)
    cudaStream_t stream_;                  // CUDA 스트림

    int input_h_, input_w_;               // 입력 크기 (384, 512)
    int output_h_, output_w_;             // 출력 크기

public:
    DepthTRTEngine(const std::string& engine_file);
    ~DepthTRTEngine();

    cv::Mat infer(const cv::Mat& image);  // 핵심 추론 함수
    float getInferenceTime();              // 추론 시간 반환
};
```

#### 4.3 메모리 레이아웃

```
입력 버퍼 (GPU):
  [1 × 3 × 384 × 512] = 589,824 float = 2.36 MB (FP32)
                                       = 1.18 MB (FP16)

출력 버퍼 (GPU):
  [1 × 1 × 384 × 512] = 196,608 float = 0.79 MB (FP32)

TRT 엔진 (GPU VRAM):
  ~200-400 MB (모델 가중치 + 연산 버퍼)

Jetson Orin Nano 8GB:
  - 총 VRAM: 8GB (CPU/GPU 공유)
  - Depth 모델: ~400MB
  - YOLO 모델: ~200MB
  - 시스템: ~2GB
  - 여유: ~5.4GB ✅
```

---

### 5. 메모리 사용량 체크

#### 5.1 jtop으로 모니터링 (Jetson)

```bash
# jtop 설치
sudo pip3 install jetson-stats

# 실행
jtop

# 확인 항목:
# - GPU 사용률 (%)
# - 메모리 사용량 (MB/GB)
# - 전력 소비 (W)
# - 온도 (°C)
```

#### 5.2 프로그래밍 방식 체크

```cpp
#include <cuda_runtime.h>

void checkGPUMemory() {
    size_t free_mem, total_mem;
    cudaMemGetInfo(&free_mem, &total_mem);

    float free_gb = free_mem / (1024.0f * 1024.0f * 1024.0f);
    float total_gb = total_mem / (1024.0f * 1024.0f * 1024.0f);
    float used_gb = total_gb - free_gb;

    printf("GPU Memory: %.2f / %.2f GB (사용: %.2f GB)\n",
           free_gb, total_gb, used_gb);
}
```

---

### 6. Jetson에서 추론 속도 측정

#### 6.1 정확한 FPS 측정 방법

```
주의: 단순히 1/시간으로 계산하면 안 됨!

정확한 측정:
1. Warm-up: 처음 10프레임은 무시 (GPU 초기화)
2. 측정: 100프레임 이상 평균
3. 전처리 + 추론 + 후처리 모두 포함
4. cudaDeviceSynchronize() 호출 (비동기 완료 대기)
```

#### 6.2 측정 코드 구조

```cpp
// 정확한 FPS 측정
void benchmarkFPS(DepthTRTEngine& engine, int num_frames = 100) {
    cv::Mat dummy(384, 512, CV_8UC3, cv::Scalar(128, 128, 128));

    // Warm-up (10프레임)
    for (int i = 0; i < 10; i++) {
        engine.infer(dummy);
    }
    cudaDeviceSynchronize();

    // 측정 시작
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_frames; i++) {
        engine.infer(dummy);
    }
    cudaDeviceSynchronize();

    auto end = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration<double, std::milli>(end - start).count();

    double avg_ms = total_ms / num_frames;
    double fps = 1000.0 / avg_ms;

    printf("평균 추론 시간: %.2f ms\n", avg_ms);
    printf("FPS: %.1f\n", fps);
}
```

#### 6.3 목표 성능 기준

```
┌─────────────────────────────────────────────────┐
│             Jetson Orin Nano 목표 성능            │
├─────────────────┬───────────────────────────────┤
│ 항목            │ 목표                           │
├─────────────────┼───────────────────────────────┤
│ 추론 시간       │ 50-66 ms/frame                │
│ FPS             │ 15-20 FPS                     │
│ GPU 사용률      │ < 80%                         │
│ 메모리 사용     │ < 4 GB                        │
│ 전력 소비       │ < 15W                         │
│ 온도            │ < 70°C                        │
└─────────────────┴───────────────────────────────┘

FP32 vs FP16 비교 (예상):
┌──────────┬──────────┬──────────┬──────────┐
│ 정밀도   │ 추론시간  │ FPS      │ 메모리    │
├──────────┼──────────┼──────────┼──────────┤
│ FP32     │ ~100ms   │ ~10      │ ~600MB   │
│ FP16     │ ~55ms    │ ~18      │ ~350MB   │
│ 향상     │ 1.8x     │ 1.8x    │ 0.6x     │
└──────────┴──────────┴──────────┴──────────┘
```

---

## 💡 꼭 이해해야 할 핵심 개념

### 개념 1: ONNX는 "중간 언어"

```
비유: 영어 → 에스페란토 → 일본어
      Python → ONNX → TensorRT

ONNX (Open Neural Network Exchange):
- Microsoft + Facebook 공동 개발
- 프레임워크 독립적 표준
- 연산자(Op) 정의 포함
- 그래프 형태로 모델 표현
```

### 개념 2: TensorRT 최적화 기법들

```
1. Layer Fusion (레이어 융합)
   Conv + BN + ReLU → 하나의 커널로

2. Kernel Auto-Tuning
   GPU에 맞는 최적 커널 자동 선택
   (Jetson Orin ≠ RTX 4090 → 다른 커널)

3. Precision Calibration
   FP32 → FP16/INT8 자동 변환
   정밀도 vs 속도 트레이드오프

4. Memory Optimization
   텐서 재사용, 메모리 풀링
```

### 개념 3: Jetson 통합 메모리 (Unified Memory)

```
일반 PC:
  CPU RAM [16GB] ←→ GPU VRAM [8GB]
  - cudaMemcpy 필요 (느림)

Jetson Orin:
  통합 메모리 [8GB] = CPU + GPU 공유
  - Zero-copy 가능!
  - cudaHostAlloc(..., cudaHostAllocMapped)
  - GPU가 CPU 메모리를 직접 접근

장점: 메모리 복사 오버헤드 감소
단점: 대역폭 공유 → 경합 가능
```

### 개념 4: Depth Map의 해석

```
Depth Anything 출력:
- 상대적 깊이 (relative depth)
- 값 범위: 0.0 ~ 1.0 (정규화)
- 가까운 물체 → 큰 값
- 먼 물체 → 작은 값

절대 깊이로 변환하려면?
- Scale + Shift 보정 필요
- Ground Truth 필요 (Week 10에서 다룸)
- depth_abs = scale * depth_rel + shift
```

---

## 🔍 자체 점검 - 이해했는지 확인!

**Q1: PyTorch → ONNX → TensorRT 파이프라인에서 각 단계의 역할은?**

<details>
<summary>정답 보기</summary>

- **PyTorch**: 학습 및 실험용. 동적 그래프로 유연하지만 배포에 부적합
- **ONNX**: 프레임워크 독립적 중간 포맷. 표준화된 연산자로 호환성 보장
- **TensorRT**: NVIDIA GPU에 최적화된 추론 엔진. Layer 융합, 커널 튜닝, 정밀도 최적화

</details>

**Q2: FP16 변환이 속도를 높이는 원리는?**

<details>
<summary>정답 보기</summary>

- 데이터 크기가 절반 (4바이트 → 2바이트)
- 메모리 대역폭 2배 효율적 사용
- Tensor Core 활용 가능 (FP16 전용 하드웨어)
- 연산량 자체가 줄어듦 (곱셈/덧셈 속도 향상)
- 정밀도 손실은 Depth 추정에서 무시할 수 있는 수준 (< 1%)

</details>

**Q3: 입력 크기를 384x512로 고정하는 이유는?**

<details>
<summary>정답 보기</summary>

- TensorRT는 정적 크기에서 최적화됨 (커널 튜닝, 메모리 할당)
- 동적 크기면 매번 재최적화 → 오버헤드 발생
- ViT 패치 크기(14)와의 호환성 고려
- 해상도 vs 속도의 적절한 균형점
- 실제 카메라 해상도에서 resize하여 사용

</details>

**Q4: Jetson Orin의 통합 메모리(Unified Memory)가 TensorRT 성능에 주는 영향은?**

<details>
<summary>정답 보기</summary>

- CPU와 GPU가 같은 물리 메모리를 공유
- Zero-copy 기법으로 cudaMemcpy 오버헤드 제거 가능
- 별도 VRAM이 없어 메모리 관리가 단순
- 단, CPU와 GPU가 대역폭을 공유하므로 동시 접근 시 경합 발생 가능
- 전체 8GB 중 모델과 시스템이 나눠 사용하므로 메모리 관리 중요

</details>

---

## 📝 이번 주 실습 & 다음 주 준비

### 실습 구성

| Step | 내용 | 예상 시간 |
|------|------|----------|
| 1 | Depth Anything → ONNX 변환 (PC) | 2시간 |
| 2 | ONNX → TensorRT FP16 변환 (Jetson) | 3시간 |
| 3 | C++ TensorRT 추론 코드 작성 | 4시간 |
| 4 | FPS 측정 및 메모리 프로파일링 | 3시간 |

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

### 다음 주 준비

- Week 10에서는 Depth 모델의 **정확도 검증**을 다룹니다
- 스테레오 카메라로 Ground Truth를 만들어 비교합니다
- Python 환경으로 돌아갑니다 (PyTorch, OpenCV)

---

## 🎯 이번 주 핵심 요약

### 1. 변환 파이프라인

```
PyTorch (.pt) → ONNX (.onnx) → TensorRT (.trt)
각 단계마다 최적화 수준이 높아짐
```

### 2. 입력 크기 고정

```
384 × 512 권장
- ViT 패치 크기 호환
- 속도 vs 정확도 균형
- TensorRT 정적 최적화 활용
```

### 3. FP16의 힘

```
속도 ~2배 향상
메모리 ~40% 절약
정밀도 손실 < 1%
```

### 4. Jetson 통합 메모리

```
CPU/GPU 메모리 공유 → Zero-copy 가능
별도 VRAM 없음 → 메모리 관리 중요
```

### 5. 목표 달성 기준

```
Jetson Orin Nano: 15-20 FPS @ 384×512
GPU 메모리 < 4GB
추론 시간 < 66ms/frame
```

---

이전: [Week 8 - Depth Anything 모델 이해](../week8/README.md)

다음: [Week 10 - Depth 정확도 검증](../week10/README.md)
