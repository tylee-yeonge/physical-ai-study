# Week 6: Jetson 배포 - TensorRT (C++)

> **이번 주 목표**: ONNX 모델을 TensorRT로 변환하고 C++로 실시간 추론 구현하기
> **예상 시간**: 12시간
> **핵심 질문**: "왜 ONNX Runtime이 아니라 TensorRT를 사용할까?"

---

## 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | TensorRT 최적화, Layer Fusion 개념 확인 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | TensorRT 엔진 빌드, FP16 추론 심화 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | TensorRT C++ 추론 파이프라인 구축 |

---

## 시작하기 전에

### Week 5 복습

Week 5에서 `.pt`를 `.onnx`로 바꿔 ONNX Runtime으로 추론했다. 그런데 Jetson 같은 임베디드 보드에서는 ONNX Runtime만으로 30 FPS를 내기가 어렵다.

```
Week 3-4: YOLO11 학습 → .pt 모델 (PyTorch)
Week 5: .pt → .onnx 변환 (ONNX Runtime 추론)


문제: Jetson에서 ONNX Runtime만으로는 30 FPS 달성 어려움!
```

같은 모델이라도 추론 엔진에 따라 속도가 크게 다르다.

```
ONNX Runtime (GPU): ~12 ms / 83 FPS (데스크톱 GPU)
ONNX Runtime (Jetson): ~40 ms / 25 FPS (Jetson Orin Nano)

TensorRT (Jetson): ~15 ms / 66 FPS (Jetson Orin Nano)
TensorRT FP16: ~8 ms / 125 FPS (Jetson Orin Nano)


→ TensorRT는 NVIDIA GPU에 특화된 최적화 엔진!
```

**비유**로 보면, ONNX Runtime은 범용 통역사이고 TensorRT는 NVIDIA 전문 통역사다.

```
ONNX Runtime = 범용 통역사
  → 여러 언어(하드웨어)를 번역 가능
  → 하지만 특정 언어에 최적화되진 않음


TensorRT = NVIDIA 전문 통역사
  → NVIDIA GPU "만" 지원
  → 하지만 NVIDIA에서는 압도적으로 빠름!
  → Layer Fusion, Kernel Auto-Tuning, FP16/INT8 최적화
```

범용성을 포기하는 대신 NVIDIA GPU에서의 속도를 극대화한 것이 TensorRT다.

---

## 핵심 개념 자세히 알아보기

### 1. TensorRT란?

**TensorRT**는 NVIDIA가 만든 고성능 딥러닝 추론 엔진이다. ONNX 모델을 받아 NVIDIA GPU 전용으로 최적화한 **엔진**(.trt 파일)을 만든다.

```
핵심 최적화 기법:
+-----------------------------------------+
| TensorRT 최적화 |
| |
| Layer Fusion (레이어 합치기) |
| Conv + BN + ReLU → FusedCBR |
| → 메모리 접근 감소, 커널 호출 감소 |
| |
| Kernel Auto-Tuning |
| 수백 개 커널 중 최적 선택 |
| → GPU 아키텍처별 최적 커널 자동 탐색 |
| |
| Precision Calibration |
| FP32 → FP16 → INT8 |
| → 정확도 유지하며 속도 극대화 |
| |
| Dynamic Tensor Memory |
| 메모리 재사용 최적화 |
| → Jetson 8GB 메모리 효율적 사용 |
| |
| Multi-Stream Execution |
| 여러 추론을 병렬 실행 |
| → GPU 활용률 극대화 |
+-----------------------------------------+
```

이 중 가장 중요한 두 가지가 Layer Fusion(§2)과 Kernel Auto-Tuning(§3)이다.

### 2. Layer Fusion 상세

**Layer Fusion**(레이어 융합)은 여러 레이어를 하나로 합치는 최적화다. 왜 이게 빨라질까? **GPU는 연산 자체보다 메모리를 읽고 쓰는 데서 시간을 더 쓰기 때문**이다.

```
Before Fusion (일반적인 추론):


Conv2D → 메모리 쓰기 → 메모리 읽기 → BatchNorm
BatchNorm → 메모리 쓰기 → 메모리 읽기 → ReLU
ReLU → 메모리 쓰기


총: 6번 메모리 접근, 3번 커널 호출
```

```
After Fusion (TensorRT):


FusedConvBNReLU → 메모리 쓰기


총: 1번 메모리 접근, 1번 커널 호출!


→ 메모리 대역폭 절약 = 속도 향상!
```

Fusion 전에는 Conv 결과를 메모리에 쓰고, BN이 다시 읽고, BN 결과를 쓰고, ReLU가 또 읽는다. Fusion 후에는 세 연산을 하나의 커널에서 연속 처리하므로 중간 결과를 메모리에 들락거릴 필요가 없다.

```
YOLO에서의 Fusion 예시:


YOLO 원본: TensorRT 최적화 후:
Conv 3x3 +
BatchNorm +→ FusedConvBNSiLU
SiLU +
Conv 1x1 +
BatchNorm +→ FusedConvBN
                    +
Concat → 유지
Conv 3x3 +
BatchNorm +→ FusedConvBNSiLU
SiLU +


레이어 수: 180 → 80 (55% 감소!)
```

`quiz_easy.cpp` 문제 1과 `quiz_medium.cpp` 문제 1이 이 Fusion을 묻는다.

### 3. Kernel Auto-Tuning

같은 연산(예: convolution)도 구현 방식(커널)이 여러 가지다. TensorRT는 빌드할 때 후보 커널들을 실제로 돌려 보고 가장 빠른 것을 고른다.

```
TensorRT의 빌드 과정:


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


빌드 시간: 5-30분 (한 번만 하면 됨!)
GPU마다 다른 엔진 생성 (이식 불가)
```

여기서 중요한 결론: **TensorRT 엔진은 빌드한 GPU에서만 동작한다.**

```
왜 GPU마다 다를까?
Jetson Orin Nano: Ampere 아키텍처, 1024 CUDA cores
RTX 4090: Ada Lovelace, 16384 CUDA cores


→ 같은 연산이라도 최적 커널이 다름!
→ Jetson에서 빌드한 .trt는 RTX에서 실행 불가
```

GPU 아키텍처가 다르면 최적 커널도 다르다. 그래서 Jetson에서 빌드한 `.trt`는 RTX에서 못 쓰고, 각 GPU에서 따로 빌드해야 한다. (반면 `.onnx`는 이식 가능하므로, 배포 대상에서 `.onnx`로부터 빌드한다.) `quiz_easy.cpp` 문제 3이 이 이식성을 묻는다.

---

### 4. ONNX -> TensorRT 변환 (trtexec)

**trtexec**는 TensorRT에 포함된 명령줄 도구로, ONNX를 TensorRT 엔진으로 변환한다.

```bash
# 기본 변환 (FP32): onnx를 받아 trt 엔진으로 저장
trtexec --onnx=yolo11n.onnx \
        --saveEngine=yolo11n.trt

# FP16 변환 (권장): --fp16 플래그로 FP16 정밀도 활성화 (속도 약 2배)
trtexec --onnx=yolo11n.onnx \
        --saveEngine=yolo11n_fp16.trt \
        --fp16

# INT8 변환: --int8 + 캘리브레이션 데이터 폴더 지정 (Week 5 §6 참고)
trtexec --onnx=yolo11n.onnx \
        --saveEngine=yolo11n_int8.trt \
        --int8 \
        --calib=calibration_data/

# 벤치마크: 저장된 엔진의 추론 속도 측정 (warmUp 후 100회 평균)
trtexec --loadEngine=yolo11n_fp16.trt \
        --batch=1 \
        --warmUp=500 \
        --avgRuns=100
```

주요 파라미터:

```
--fp16 : FP16 정밀도 사용 (속도 2배, 정확도 유지)
--int8 : INT8 정밀도 (캘리브레이션 필요)
--workspace : GPU 워크스페이스 크기 (MB)
--minShapes : 최소 입력 크기 (동적)
--optShapes : 최적 입력 크기 (동적)
--maxShapes : 최대 입력 크기 (동적)
--saveEngine : 엔진 저장 경로
--verbose : 상세 로그
```

`quiz_easy.cpp` 문제 2의 정답이 `--saveEngine=yolo11n_fp16.trt --fp16`인 것에 주의하자 - `--half`나 `--float16` 같은 옵션은 존재하지 않는다. FP16 플래그는 `--fp16`이다.

---

### 5. C++ TensorRT Inference 구조

Jetson에서 실시간 추론은 보통 C++로 짠다 (속도와 하드웨어 제어 때문). 전체 파이프라인은 카메라 -> 전처리 -> 추론 -> 후처리 -> 시각화다.

```
+-------------+ +--------------+ +--------------+
| 카메라 입력 | --→ | 전처리 | --→ | TensorRT |
| (OpenCV) | | (GPU에서) | | 추론 |
+-------------+ +--------------+ +--------------+
                                              |
+-------------+ +--------------+ |
| 시각화/출력 | ←-- | NMS | ←--------+
| (OpenCV) | | 후처리 |
+-------------+ +--------------+
```

TensorRT C++ API의 주요 클래스와 함수를 알아 두자.

```cpp
// --- TensorRT 주요 클래스 ---
nvinfer1::IRuntime          // 런타임: 저장된 엔진 파일을 로드하는 객체
nvinfer1::ICudaEngine       // CUDA 엔진: 최적화된 모델 그 자체
nvinfer1::IExecutionContext // 실행 컨텍스트: 실제 추론을 수행하는 객체

// --- CUDA 메모리 관리 함수 ---
cudaMalloc()   // GPU 메모리 할당
cudaMemcpy()   // CPU <-> GPU 간 데이터 복사
cudaFree()     // GPU 메모리 해제

// --- OpenCV (입출력) ---
cv::VideoCapture // 카메라 입력을 받는 객체
cv::Mat          // 이미지 데이터를 담는 행렬
cv::rectangle()  // 이미지에 바운딩 박스를 그리는 함수
```

---

### 6. NMS (Non-Maximum Suppression) 구현

YOLO 출력은 후보 박스가 수천 개다. 같은 객체에 여러 박스가 겹치므로, **NMS**로 중복을 제거해야 한다 (Week 3 §꼭 이해 참고).

```
YOLO 출력: 8400개 후보 박스!


같은 객체에 여러 박스가 겹침:
+-------------+
| +----------+| box1: conf=0.9
| |+--------+|| box2: conf=0.85
| || 사람 ||| box3: conf=0.7
| |+--------+||
| +----------+|
+-------------+


NMS 후: box1만 남김 (가장 높은 confidence)
```

```
NMS 알고리즘:
1. confidence 기준 정렬 (내림차순)
2. 가장 높은 confidence 박스 선택
3. 선택된 박스와 IoU > threshold인 박스 제거
4. 남은 박스에 대해 2-3 반복
5. 빈 리스트가 될 때까지 반복
```

```
IoU (Intersection over Union):
IoU = 교집합 면적 / 합집합 면적
IoU > 0.45 → 같은 객체로 간주 → 하나만 유지
```

**IoU threshold를 높이면(0.45 -> 0.7) 검출 수가 늘어난다.** IoU 0.7 이상으로 많이 겹친 박스만 "같은 객체"로 보고 제거하므로, 0.5-0.7 정도 겹친 박스는 살아남기 때문이다. `quiz_easy.cpp` 문제 4가 이를 묻는다.

---

### 7. 성능 최적화: 30 FPS @ 640x480

30 FPS는 프레임당 33.3 ms 안에 모든 처리를 끝내야 한다는 뜻이다. 각 단계 시간을 재서 병목을 찾는다.

```
프레임 처리 시간 = 33.3ms (30 FPS)


카메라 읽기: ~5ms
전처리 (resize): ~2ms
TensorRT 추론: ~8ms (FP16)
NMS 후처리: ~1ms
시각화: ~3ms
---------------------
총: ~19ms → OK!


여유: 33.3 - 19 = 14.3ms
```

여기서 더 짜내려면 **Multi-threading**을 쓴다. 카메라 읽기는 I/O 대기(USB/CSI 신호를 기다림)라 GPU와 무관하다. 별도 스레드로 빼면 추론과 동시에 진행된다.

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

`quiz_medium.cpp` 문제 3이 병목 분석과 Multi-threading 최적화를 묻는다.

---

### 8. FP16 vs INT8 성능 (Jetson Orin Nano)

| 정밀도 | 빌드 시간 | 엔진 크기 | 추론 시간 | mAP  |
|--------|----------|----------|----------|------|
| FP32   | 5분      | 25 MB    | 30 ms    | 0.45 |
| FP16   | 8분      | 13 MB    | 8 ms     | 0.44 |
| INT8   | 15분     | 7 MB     | 5 ms     | 0.42 |

결론: FP16이 최적. INT8 대비 약간 느리지만 캘리브레이션 불필요, 정확도 손실 거의 없음(0.01 mAP).

INT8이 가장 빠르지만 캘리브레이션 데이터를 준비해야 하고 정확도 손실이 있다. FP16은 캘리브레이션 없이 정확도를 거의 유지하면서 FP32보다 훨씬 빠르다 - 그래서 Jetson 배포의 기본 선택은 FP16이다.

---

## 꼭 이해해야 할 핵심 개념

### TensorRT 엔진 빌드 주의사항

```
1. GPU별 엔진 호환 불가
   Jetson에서 빌드 → Jetson에서만 실행
   RTX에서 빌드 → RTX에서만 실행


2. TensorRT 버전 호환
   TRT 8.x로 빌드한 엔진은 TRT 9.x에서 실행 불가
   → JetPack 버전에 맞는 TRT 사용!


3. 빌드 시간이 오래 걸림
   → 엔진을 파일로 저장하고 재사용!
   → 매번 빌드하면 안 됨


4. 메모리 관리
   → Jetson 8GB 제한 → workspace 크기 조절
   → GPU 메모리 누수 주의 (cudaFree)
```

### C++ TensorRT 코딩 패턴

추론 코드의 표준 골격이다. **엔진 로드와 메모리 할당은 한 번만**, **추론은 루프 안에서 반복**, **끝나면 해제**가 핵심이다. 각 줄의 역할을 주석으로 따라가자.

```cpp
// --- 1. 엔진 로드 (프로그램 시작 시 한 번만) ---
IRuntime* runtime = createInferRuntime(logger);                  // 런타임 객체 생성
ICudaEngine* engine = runtime->deserializeCudaEngine(data, size); // 파일에서 엔진 복원
IExecutionContext* context = engine->createExecutionContext();    // 추론 실행 컨텍스트 생성

// --- 2. GPU 메모리 할당 + 텐서 주소 등록 (한 번만, TRT 10 API) ---
void* buffers[2];                          // [0]=입력, [1]=출력 버퍼 포인터
cudaMalloc(&buffers[0], input_size);       // 입력용 GPU 메모리 확보
cudaMalloc(&buffers[1], output_size);      // 출력용 GPU 메모리 확보 (둘 다 할당해야 함!)
context->setTensorAddress("images", buffers[0]);   // 입력 텐서 이름 -> 디바이스 주소 등록
context->setTensorAddress("output0", buffers[1]);  // 출력 텐서 이름 -> 디바이스 주소 등록

// --- 3. 추론 루프 (프레임마다 반복) ---
while (running) {
    // CPU -> GPU: 입력 데이터를 GPU로 복사 (방향 = HostToDevice)
    cudaMemcpy(buffers[0], input_data, size, cudaMemcpyHostToDevice);

    // 추론 실행: 등록된 텐서 주소를 사용해 default stream에 작업 등록 후 동기화
    context->enqueueV3(0);
    cudaStreamSynchronize(0);

    // GPU -> CPU: 결과를 CPU로 복사 (방향 = DeviceToHost)
    cudaMemcpy(output_data, buffers[1], size, cudaMemcpyDeviceToHost);
}

// --- 4. 정리 (프로그램 종료 시, 할당의 역순으로 해제. TRT 10에서는 delete 사용) ---
cudaFree(buffers[0]);   // 입력 GPU 메모리 해제
cudaFree(buffers[1]);   // 출력 GPU 메모리 해제
delete context;         // 실행 컨텍스트 해제 (TRT 10에서 destroy() 제거됨)
delete engine;          // 엔진 해제
delete runtime;         // 런타임 해제
```

`quiz_medium.cpp` 문제 2가 이 패턴을 어긴 버그 코드를 준다 - `buffers[1]` 미할당, `cudaMemcpy` 방향 반대, `context`/`runtime` 해제 누락이 답이다. 위 표준 패턴과 비교하며 찾으면 된다. (참고: 문제 본문은 TRT 8.x 시점의 `executeV2 / destroy()` 코드로 작성돼 있다. TRT 10에서는 `enqueueV3 / delete`가 정답이지만 학습 의도상 "리소스 해제 누락"을 찾는 것이 핵심이므로 그대로 둔다.)

> **`cudaMemcpy` 방향**: 세 번째 인자 다음에 오는 방향 플래그가 중요하다. 입력(CPU->GPU)은 `cudaMemcpyHostToDevice`, 출력(GPU->CPU)은 `cudaMemcpyDeviceToHost`다. Host가 CPU, Device가 GPU다. 방향을 반대로 쓰면 데이터가 엉뚱하게 복사된다.

---

## 자체 점검 - 이해했는지 확인!

### Q1: Layer Fusion
**Q:** TensorRT의 Layer Fusion이 속도를 높이는 원리는?

**A:**
```
여러 레이어(Conv+BN+ReLU)를 하나로 합쳐서:
1. GPU 커널 호출 횟수 감소 (3번 → 1번)
2. 중간 텐서의 메모리 읽기/쓰기 제거
3. GPU 메모리 대역폭 절약


→ GPU는 연산보다 메모리 접근이 병목이므로 큰 효과!
```

### Q2: 엔진 이식성
**Q:** Jetson Orin Nano에서 빌드한 .trt 파일을 RTX 4090에서 실행할 수 있을까?

**A:**
```
불가능!


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

## 이번 주 실습 & 다음 주 준비

### 실습 항목

1. **ONNX -> TensorRT 변환**
   - trtexec로 FP32, FP16 엔진 빌드
   - 벤치마크 실행 (latency, throughput)

2. **C++ TensorRT Inference**
   - 엔진 로드 및 실행 컨텍스트 생성
   - GPU 메모리 할당/해제
   - 전처리 -> 추론 -> 후처리 파이프라인

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
- TensorRT 추론 파이프라인 완성
- Python 환경 준비 (transformers, timm)
- MiDaS, Depth Anything 논문 훑어보기
```

---

## 이번 주 핵심 요약

1. **TensorRT = NVIDIA GPU 전용 최적화 엔진**
   - Layer Fusion으로 레이어 합치기 (메모리 접근 감소)
   - Kernel Auto-Tuning으로 최적 커널 선택
   - FP16/INT8로 정밀도 조절

2. **trtexec로 변환**
   - `trtexec --onnx=model.onnx --saveEngine=model.trt --fp16`
   - 빌드한 GPU에서만 실행 가능
   - 한 번 빌드 -> 파일로 저장 -> 재사용

3. **C++ 추론 파이프라인 (TRT 10)**
   - 엔진 로드 -> 컨텍스트 생성 -> GPU 메모리 할당 + setTensorAddress (한 번만)
   - cudaMemcpy로 데이터 전송 (방향 주의)
   - enqueueV3 + cudaStreamSynchronize로 추론, 끝나면 cudaFree/delete로 정리

4. **NMS 구현**
   - 8400개 후보 -> 수십 개 최종 검출
   - IoU 기반 중복 제거
   - confidence threshold + IoU threshold

5. **성능 목표**
   - Jetson Orin Nano: 30+ FPS @ 640x480
   - FP16 권장 (속도/정확도 최적 균형)
   - Multi-threading으로 카메라 병렬화

---

다음: [Week 7 - Monocular Depth 이론](../week7/README.md)
