# Week 6: Jetson 배포 - TensorRT (C++)

> [goal] **이번 주 목표**: ONNX 모델을 TensorRT로 변환하고 C++로 실시간 추론 구현하기
> [time] **예상 시간**: 12시간
> [tip] **핵심 질문**: "왜 ONNX Runtime이 아니라 TensorRT를 사용할까?"

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | TensorRT 최적화, Layer Fusion 개념 확인 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | TensorRT 엔진 빌드, FP16 추론 심화 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | TensorRT C++ 추론 파이프라인 구축 |

---

## [*] 시작하기 전에

### Week 5 복습

**지금까지의 흐름:**
```
Week 3-4: YOLO11 학습 → .pt 모델 (PyTorch)
Week 5:   .pt → .onnx 변환 (ONNX Runtime 추론)

[!] 문제: Jetson에서 ONNX Runtime만으로는 30 FPS 달성 어려움!
```

**TensorRT가 필요한 이유:**
```
ONNX Runtime (GPU):    ~12 ms / 83 FPS (데스크톱 GPU)
ONNX Runtime (Jetson): ~40 ms / 25 FPS (Jetson Orin Nano)  [X]

TensorRT (Jetson):     ~15 ms / 66 FPS (Jetson Orin Nano)  [O]
TensorRT FP16:         ~8 ms  / 125 FPS (Jetson Orin Nano) [O][O]

→ TensorRT는 NVIDIA GPU에 특화된 최적화 엔진!
```

**비유:**
```
ONNX Runtime = 범용 통역사
  → 여러 언어(하드웨어)를 번역 가능
  → 하지만 특정 언어에 최적화되진 않음

TensorRT = NVIDIA 전문 통역사
  → NVIDIA GPU "만" 지원
  → 하지만 NVIDIA에서는 압도적으로 빠름!
  → Layer Fusion, Kernel Auto-Tuning, FP16/INT8 최적화
```

---

## [ref] 핵심 개념 자세히 알아보기

### 1. TensorRT란?

**TensorRT = NVIDIA의 고성능 딥러닝 추론 엔진**

```
핵심 최적화 기법:
+-----------------------------------------+
|           TensorRT 최적화                 |
|                                          |
|  [link] Layer Fusion (레이어 합치기)          |
|     Conv + BN + ReLU → FusedCBR         |
|     → 메모리 접근 감소, 커널 호출 감소     |
|                                          |
|   Kernel Auto-Tuning                  |
|     수백 개 커널 중 최적 선택              |
|     → GPU 아키텍처별 최적 커널 자동 탐색   |
|                                          |
|   Precision Calibration                |
|     FP32 → FP16 → INT8                  |
|     → 정확도 유지하며 속도 극대화          |
|                                          |
|   Dynamic Tensor Memory               |
|     메모리 재사용 최적화                   |
|     → Jetson 8GB 메모리 효율적 사용       |
|                                          |
|  [chart] Multi-Stream Execution               |
|     여러 추론을 병렬 실행                  |
|     → GPU 활용률 극대화                   |
+-----------------------------------------+
```

### 2. Layer Fusion 상세

**Before Fusion (일반적인 추론):**
```
Conv2D → 메모리 쓰기 → 메모리 읽기 → BatchNorm
BatchNorm → 메모리 쓰기 → 메모리 읽기 → ReLU
ReLU → 메모리 쓰기

총: 6번 메모리 접근, 3번 커널 호출
```

**After Fusion (TensorRT):**
```
FusedConvBNReLU → 메모리 쓰기

총: 1번 메모리 접근, 1번 커널 호출!

→ 메모리 대역폭 절약 = 속도 향상!
```

**YOLO에서의 Fusion 예시:**
```
YOLO 원본:          TensorRT 최적화 후:
Conv 3x3            +
BatchNorm           +→ FusedConvBNSiLU
SiLU                +
Conv 1x1            +
BatchNorm           +→ FusedConvBN
                    +
Concat              → 유지
Conv 3x3            +
BatchNorm           +→ FusedConvBNSiLU
SiLU                +

레이어 수: 180 → 80 (55% 감소!)
```

---

### 3. Kernel Auto-Tuning

**TensorRT의 빌드 과정:**
```
ONNX 모델 입력
    ↓
각 레이어에 대해:
    +-- 커널 후보 A: im2col + GEMM
    +-- 커널 후보 B: Winograd
    +-- 커널 후보 C: FFT
    +-- 커널 후보 D: Direct convolution
    ↓
각 후보를 실제 GPU에서 실행하여 시간 측정
    ↓
가장 빠른 커널 선택!
    ↓
TensorRT Engine (.trt) 저장

[!] 빌드 시간: 5-30분 (한 번만 하면 됨!)
[!] GPU마다 다른 엔진 생성 (이식 불가)
```

**왜 GPU마다 다를까?**
```
Jetson Orin Nano: Ampere 아키텍처, 1024 CUDA cores
RTX 4090:        Ada Lovelace, 16384 CUDA cores

→ 같은 연산이라도 최적 커널이 다름!
→ Jetson에서 빌드한 .trt는 RTX에서 실행 불가
```

---

### 4. ONNX → TensorRT 변환 (trtexec)

**trtexec 도구:**
```bash
# 기본 변환 (FP32)
trtexec --onnx=yolo11n.onnx \
        --saveEngine=yolo11n.trt

# FP16 변환 (권장!)
trtexec --onnx=yolo11n.onnx \
        --saveEngine=yolo11n_fp16.trt \
        --fp16

# INT8 변환 (캘리브레이션 필요)
trtexec --onnx=yolo11n.onnx \
        --saveEngine=yolo11n_int8.trt \
        --int8 \
        --calib=calibration_data/

# 벤치마크
trtexec --loadEngine=yolo11n_fp16.trt \
        --batch=1 \
        --warmUp=500 \
        --avgRuns=100
```

**주요 파라미터:**
```
--fp16         : FP16 정밀도 사용 (속도 2배, 정확도 유지)
--int8         : INT8 정밀도 (캘리브레이션 필요)
--workspace    : GPU 워크스페이스 크기 (MB)
--minShapes    : 최소 입력 크기 (동적)
--optShapes    : 최적 입력 크기 (동적)
--maxShapes    : 최대 입력 크기 (동적)
--saveEngine   : 엔진 저장 경로
--verbose      : 상세 로그
```

---

### 5. C++ TensorRT Inference 구조

**전체 파이프라인:**
```
+-------------+     +--------------+     +--------------+
| 카메라 입력   | --→ |  전처리       | --→ |  TensorRT    |
| (OpenCV)     |     |  (GPU에서)   |     |  추론        |
+-------------+     +--------------+     +--------------+
                                              |
+-------------+     +--------------+          |
| 시각화/출력   | ←-- |  NMS         | ←--------+
| (OpenCV)     |     |  후처리       |
+-------------+     +--------------+
```

**핵심 클래스:**
```cpp
// TensorRT 주요 클래스
nvinfer1::IRuntime        // 런타임 (엔진 로드)
nvinfer1::ICudaEngine     // CUDA 엔진
nvinfer1::IExecutionContext // 실행 컨텍스트

// CUDA 메모리 관리
cudaMalloc()              // GPU 메모리 할당
cudaMemcpy()              // CPU <-> GPU 복사
cudaFree()                // GPU 메모리 해제

// OpenCV (입출력)
cv::VideoCapture          // 카메라 입력
cv::Mat                   // 이미지 데이터
cv::rectangle()           // 바운딩 박스 그리기
```

---

### 6. NMS (Non-Maximum Suppression) 구현

**NMS가 필요한 이유:**
```
YOLO 출력: 8400개 후보 박스!

같은 객체에 여러 박스가 겹침:
+-------------+
| +----------+|  box1: conf=0.9
| |+--------+||  box2: conf=0.85
| || 사람   |||  box3: conf=0.7
| |+--------+||
| +----------+|
+-------------+

NMS 후: box1만 남김 (가장 높은 confidence)
```

**NMS 알고리즘:**
```
1. confidence 기준 정렬 (내림차순)
2. 가장 높은 confidence 박스 선택
3. 선택된 박스와 IoU > threshold인 박스 제거
4. 남은 박스에 대해 2-3 반복
5. 빈 리스트가 될 때까지 반복
```

**IoU (Intersection over Union):**
```
         +------+
    +----+      |
    |    | 교집합 |
    |    +------+
    +----+

IoU = 교집합 면적 / 합집합 면적

IoU > 0.45 → 같은 객체로 간주 → 하나만 유지
```

---

### 7. 성능 최적화: 30 FPS @ 640x480

**병목 분석:**
```
프레임 처리 시간 = 33.3ms (30 FPS)

카메라 읽기:      ~5ms
전처리 (resize):  ~2ms
TensorRT 추론:   ~8ms (FP16)
NMS 후처리:      ~1ms
시각화:          ~3ms
---------------------
총:              ~19ms → OK! [O]

여유: 33.3 - 19 = 14.3ms
```

**Multi-threading 전략:**
```
Thread 1: 카메라 읽기 (연속)
  → 최신 프레임을 공유 버퍼에 저장

Thread 2: 추론 + 후처리
  → 공유 버퍼에서 프레임 읽기
  → TensorRT 추론
  → NMS
  → 시각화

효과:
  카메라 대기 시간 제거!
  카메라 읽기와 추론이 병렬 실행
```

---

### 8. FP16 vs INT8 성능 (Jetson Orin Nano)

```
+----------+----------+----------+----------+----------+
|  정밀도   |  빌드 시간 | 엔진 크기  | 추론 시간  |  mAP     |
+----------+----------+----------+----------+----------+
|  FP32    |  5분     |  25 MB   |  30 ms   |  0.45    |
|  FP16    |  8분     |  13 MB   |   8 ms   |  0.44    |
|  INT8    |  15분    |   7 MB   |   5 ms   |  0.42    |
+----------+----------+----------+----------+----------+

[tip] 결론: FP16이 최적!
   → INT8 대비 약간 느리지만 캘리브레이션 불필요
   → 정확도 손실 거의 없음 (0.01 mAP)
```

---

## [tip] 꼭 이해해야 할 핵심 개념

### TensorRT 엔진 빌드 주의사항

```
[!] 1. GPU별 엔진 호환 불가
   Jetson에서 빌드 → Jetson에서만 실행
   RTX에서 빌드 → RTX에서만 실행

[!] 2. TensorRT 버전 호환
   TRT 8.x로 빌드한 엔진은 TRT 9.x에서 실행 불가
   → JetPack 버전에 맞는 TRT 사용!

[!] 3. 빌드 시간이 오래 걸림
   → 엔진을 파일로 저장하고 재사용!
   → 매번 빌드하면 안 됨

[!] 4. 메모리 관리
   → Jetson 8GB 제한 → workspace 크기 조절
   → GPU 메모리 누수 주의 (cudaFree)
```

### C++ TensorRT 코딩 패턴

```cpp
// 1. 엔진 로드 (한 번만)
IRuntime* runtime = createInferRuntime(logger);
ICudaEngine* engine = runtime->deserializeCudaEngine(data, size);
IExecutionContext* context = engine->createExecutionContext();

// 2. GPU 메모리 할당 (한 번만)
void* buffers[2];  // [input, output]
cudaMalloc(&buffers[0], input_size);
cudaMalloc(&buffers[1], output_size);

// 3. 추론 루프
while (running) {
    // CPU → GPU 복사
    cudaMemcpy(buffers[0], input_data, size, cudaMemcpyHostToDevice);

    // 추론
    context->executeV2(buffers);

    // GPU → CPU 복사
    cudaMemcpy(output_data, buffers[1], size, cudaMemcpyDeviceToHost);
}

// 4. 정리
cudaFree(buffers[0]);
cudaFree(buffers[1]);
context->destroy();
engine->destroy();
runtime->destroy();
```

---

## [search] 자체 점검 - 이해했는지 확인!

### Q1: Layer Fusion
**Q:** TensorRT의 Layer Fusion이 속도를 높이는 원리는?

**A:**
```
여러 레이어(Conv+BN+ReLU)를 하나로 합쳐서:
1. GPU 커널 호출 횟수 감소 (3번 → 1번)
2. 중간 텐서의 메모리 읽기/쓰기 제거
3. GPU 메모리 대역폭 절약

→ 메모리 바운드 연산에서 큰 효과!
```

### Q2: 엔진 이식성
**Q:** Jetson Orin Nano에서 빌드한 .trt 파일을 RTX 4090에서 실행할 수 있을까?

**A:**
```
[X] 불가능!

TensorRT 엔진은 빌드 시 다음이 고정됨:
1. GPU 아키텍처 (Ampere vs Ada Lovelace)
2. 최적 커널 (아키텍처별로 다름)
3. TensorRT 버전

→ 각 GPU에서 따로 빌드해야 함!
→ .onnx는 이식 가능 → 타겟에서 빌드
```

### Q3: NMS
**Q:** NMS에서 IoU threshold를 낮추면 어떤 일이 발생하나요?

**A:**
```
IoU threshold ↓ (예: 0.45 → 0.25):
→ 더 많은 박스가 "같은 객체"로 간주
→ 더 많은 박스가 제거됨
→ 최종 검출 수 감소

문제: 가까운 다른 객체도 제거될 수 있음!

IoU threshold ↑ (예: 0.45 → 0.7):
→ 덜 겹치는 박스도 유지
→ 최종 검출 수 증가
→ 같은 객체에 여러 박스가 남을 수 있음
```

### Q4: Multi-threading
**Q:** 카메라 읽기를 별도 스레드로 분리하는 이유는?

**A:**
```
카메라 읽기는 I/O 바운드 (USB/CSI 대기)
TensorRT 추론은 GPU 바운드

분리하지 않으면:
  카메라 대기(5ms) → 추론(8ms) → 총 13ms

분리하면:
  [Thread 1] 카메라 계속 읽기 (최신 프레임 유지)
  [Thread 2] 추론 시작 시 최신 프레임 바로 사용
  → 카메라 대기 시간 0ms → 총 8ms

→ 30fps → 50fps+ 향상 가능!
```

---

## [note] 이번 주 실습 & 다음 주 준비

### 실습 항목

1. **ONNX → TensorRT 변환**
   - trtexec로 FP32, FP16 엔진 빌드
   - 벤치마크 실행 (latency, throughput)

2. **C++ TensorRT Inference**
   - 엔진 로드 및 실행 컨텍스트 생성
   - GPU 메모리 할당/해제
   - 전처리 → 추론 → 후처리 파이프라인

3. **NMS 구현 (C++)**
   - IoU 계산 함수
   - NMS 알고리즘 구현
   - 검출 결과 시각화

4. **성능 측정**
   - 640x480 @ 30 FPS 목표
   - FPS 카운터 추가
   - Multi-threading 카메라 읽기

### 다음 주 준비

```
Week 7에서는 Monocular Depth Estimation을 학습합니다!

준비:
[O] TensorRT 추론 파이프라인 완성
[O] Python 환경 준비 (transformers, timm)
[O] MiDaS, Depth Anything 논문 훑어보기
```

---

## [goal] 이번 주 핵심 요약

1. **TensorRT = NVIDIA GPU 전용 최적화 엔진**
   - Layer Fusion으로 레이어 합치기
   - Kernel Auto-Tuning으로 최적 커널 선택
   - FP16/INT8로 정밀도 조절

2. **trtexec로 변환**
   - `trtexec --onnx=model.onnx --saveEngine=model.trt --fp16`
   - 빌드한 GPU에서만 실행 가능
   - 한 번 빌드 → 파일로 저장 → 재사용

3. **C++ 추론 파이프라인**
   - 엔진 로드 → 컨텍스트 생성 → GPU 메모리 할당
   - cudaMemcpy로 데이터 전송
   - executeV2로 추론 실행

4. **NMS 구현**
   - 8400개 후보 → 수십 개 최종 검출
   - IoU 기반 중복 제거
   - confidence threshold + IoU threshold

5. **성능 목표**
   - Jetson Orin Nano: 30+ FPS @ 640x480
   - FP16 권장 (속도/정확도 최적 균형)
   - Multi-threading으로 카메라 병렬화

---

다음: [Week 7 - Monocular Depth 이론](../week7/README.md)
