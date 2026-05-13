# Week 5: ONNX 변환 - 프레임워크 독립적 모델 배포

> [goal] **이번 주 목표**: PyTorch 모델을 ONNX로 변환하고 Runtime 추론 및 양자화 적용하기
> [time] **예상 시간**: 12시간
> [tip] **핵심 질문**: "왜 PyTorch 모델을 그대로 배포하지 않고 ONNX로 변환할까?"

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | ONNX 포맷, opset, 변환 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | ONNX 변환 및 Runtime 추론 심화 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | ONNX 변환 및 Runtime 추론 실습 |

---

## [*] 시작하기 전에

### Week 1-4 복습

**지금까지의 흐름:**
```
Week 3-4: YOLO11 학습 (PyTorch) → .pt 모델 생성
Week 3-4: 커스텀 데이터셋 학습 + 평가

[!] 문제: .pt 파일은 PyTorch에서만 사용 가능!
```

**현실 세계의 배포 문제:**
```
팀 A: PyTorch로 학습 (Python)
팀 B: TensorFlow Serving으로 배포하고 싶음
팀 C: Jetson에서 TensorRT로 추론하고 싶음
팀 D: 모바일에서 CoreML로 실행하고 싶음

→ 각 프레임워크에 맞게 다시 구현해야 할까? [X]
→ 중간 포맷이 필요하다! → ONNX [O]
```

**비유:**
```
ONNX = 모델의 "PDF"

.docx → Word에서만 편집 가능 (= .pt → PyTorch에서만 실행)
.pdf  → 어디서든 볼 수 있음   (= .onnx → 어디서든 추론)

PyTorch (.pt) --→ ONNX (.onnx) --→ TensorRT (.trt)
TensorFlow (.pb) --->              --→ CoreML (.mlmodel)
                                   --→ ONNX Runtime
```

---

## [ref] 핵심 개념 자세히 알아보기

### 1. ONNX란 무엇인가?

**ONNX = Open Neural Network Exchange**

```
핵심 구성요소:
+-----------------------------------------+
|              ONNX 모델                    |
|                                          |
|   Graph (계산 그래프)                   |
|  +-- Node: Conv, ReLU, BatchNorm, ...   |
|  +-- Input: 입력 텐서 형상               |
|  +-- Output: 출력 텐서 형상              |
|  +-- Initializer: 학습된 가중치           |
|                                          |
|  [list] Opset Version (연산자 버전)           |
|  [pkg] Metadata (모델 정보)                  |
+-----------------------------------------+
```

**지원하는 프레임워크:**
```
PyTorch    --→ ONNX --→ TensorRT (Jetson)
TensorFlow --→ ONNX --→ ONNX Runtime (서버)
PaddlePaddle→ ONNX --→ CoreML (iOS)
JAX        --→ ONNX --→ OpenVINO (Intel)
                    --→ DirectML (Windows)
```

### 2. PyTorch → ONNX 변환 과정

**단계별 과정:**
```
Step 1: PyTorch 모델 로드
  model = YOLO('yolo11n.pt')

Step 2: 모델을 eval() 모드로 전환
  model.eval()  # BatchNorm, Dropout 고정

Step 3: 더미 입력 생성
  dummy = torch.randn(1, 3, 640, 640)

Step 4: torch.onnx.export() 호출
  → PyTorch가 모델을 실행하면서 연산을 추적(trace)
  → 추적된 연산을 ONNX 그래프로 변환
  → .onnx 파일로 저장
```

**torch.onnx.export 핵심 파라미터:**
```python
torch.onnx.export(
    model,                    # PyTorch 모델
    dummy_input,              # 더미 입력 (추적용)
    "model.onnx",             # 출력 파일
    opset_version=12,         # ONNX 연산자 버전
    input_names=['images'],   # 입력 이름
    output_names=['output'],  # 출력 이름
    dynamic_axes={            # 동적 축 (선택)
        'images': {0: 'batch'},
        'output': {0: 'batch'}
    }
)
```

---

### 3. Opset Version 이해

**Opset = Operator Set (연산자 집합)**
```
Opset 9:  기본 연산 (Conv, ReLU, MaxPool)
Opset 11: Resize, Pad 개선
Opset 12: Einsum, Dropout 업데이트
Opset 13: Squeeze/Unsqueeze 변경
Opset 17: LayerNorm, GroupNorm 추가

[tip] 권장: opset_version=12 (안정적, 호환성 좋음)

주의: 높은 Opset ≠ 더 좋음
  → TensorRT가 지원하지 않는 연산이 포함될 수 있음!
  → Jetson 배포 시 opset 12-13 권장
```

**호환성 체크:**
```
TensorRT 8.x: Opset 9-17 부분 지원
ONNX Runtime: Opset 7-18 지원
CoreML: Opset 9-15 지원

→ 배포 타겟에 맞는 Opset 선택이 중요!
```

---

### 4. Dynamic Shape vs Static Shape

**Static Shape (고정 크기):**
```
입력: [1, 3, 640, 640]  → 항상 동일한 크기

장점:
[O] 추론 속도 빠름 (메모리 사전 할당)
[O] TensorRT 최적화 극대화
[O] 디버깅 쉬움

단점:
[X] 다른 크기 입력 불가
[X] 배치 크기 변경 불가
```

**Dynamic Shape (가변 크기):**
```
입력: [batch, 3, height, width]  → 실행 시 결정

장점:
[O] 유연한 입력 크기
[O] 배치 크기 가변

단점:
[X] 약간 느린 추론 (동적 할당)
[X] TensorRT 최적화 제한적
```

**SLAM/로봇 환경에서의 선택:**
```
[O] Static Shape 권장!

이유:
1. 카메라 해상도 고정 (640x480)
2. 배치 크기 1 (실시간 단일 프레임)
3. 최대 성능 필요 (30+ FPS)
4. Jetson 메모리 제한 (8GB)

→ Static [1, 3, 640, 640] 사용
```

---

### 5. ONNX Runtime 추론

**ONNX Runtime이란?**
```
Microsoft가 개발한 고성능 추론 엔진

특징:
+-- 크로스 플랫폼 (Windows, Linux, macOS)
+-- GPU 가속 (CUDA, TensorRT, DirectML)
+-- CPU 최적화 (AVX, VNNI)
+-- 그래프 최적화 (연산 융합, 상수 폴딩)
+-- Python, C++, C#, Java API
```

**추론 파이프라인:**
```
이미지 입력
    ↓
전처리 (resize, normalize, transpose)
    ↓
ONNX Runtime Session
    +-- ExecutionProvider 선택
    |   +-- CUDAExecutionProvider (GPU)
    |   +-- TensorrtExecutionProvider (TensorRT)
    |   +-- CPUExecutionProvider (CPU)
    ↓
후처리 (NMS, bbox decode)
    ↓
결과 출력
```

---

### 6. 양자화 (Quantization): FP32 → FP16 → INT8

**정밀도 비교:**
```
FP32 (32-bit 부동소수점):
  +------+--------------+------------------+
  | 부호 |   지수 (8b)   |    가수 (23b)     |
  +------+--------------+------------------+
  → 메모리: 4 바이트
  → 범위: ±3.4 × 10³⁸
  → 정확도: 높음

FP16 (16-bit 부동소수점):
  +------+----------+-----------+
  | 부호 | 지수 (5b) | 가수 (10b) |
  +------+----------+-----------+
  → 메모리: 2 바이트 (50% 절약!)
  → 범위: ±65504
  → 정확도: 약간 감소

INT8 (8-bit 정수):
  +---------------------+
  |    정수 (8 bits)     |
  +---------------------+
  → 메모리: 1 바이트 (75% 절약!)
  → 범위: -128 ~ 127
  → 정확도: 감소 (캘리브레이션 필요)
```

**성능 비교 (Jetson Orin Nano 기준):**
```
+----------+----------+----------+----------+
|  정밀도   |  모델 크기 | 추론 속도  |  mAP     |
+----------+----------+----------+----------+
|  FP32    |  25 MB   |  15 FPS  |  0.45    |
|  FP16    |  12 MB   |  30 FPS  |  0.44    |
|  INT8    |   6 MB   |  55 FPS  |  0.42    |
+----------+----------+----------+----------+

[tip] FP16이 가성비 최고!
   → 정확도 손실 거의 없이 2배 빨라짐
```

**INT8 캘리브레이션:**
```
INT8은 단순 변환 불가!
→ 대표 데이터로 "캘리브레이션" 필요

과정:
1. 대표 이미지 100-500장 준비
2. FP32 모델로 각 레이어 출력 범위 측정
3. 최적의 스케일(scale)과 제로포인트(zero point) 결정
4. INT8로 양자화

quantize_dynamic(model, "model_int8.onnx")
```

---

### 7. 속도 비교: PyTorch vs ONNX Runtime

**벤치마크 실험 설계:**
```
조건:
- 모델: YOLO11n
- 입력: 640 × 640 × 3
- GPU: NVIDIA GPU (CUDA)
- 반복: 100회 평균

결과:
+----------------+----------+----------+
|    방법         | 추론 시간  |  비율     |
+----------------+----------+----------+
| PyTorch (GPU)  |  18 ms   |  1.0x    |
| ONNX RT (CPU)  |  45 ms   |  0.4x    |
| ONNX RT (GPU)  |  12 ms   |  1.5x    |
| ONNX RT (TRT)  |   6 ms   |  3.0x    |
+----------------+----------+----------+

[tip] ONNX Runtime + TensorRT EP가 가장 빠름!
```

**왜 ONNX Runtime이 더 빠를까?**
```
1. 그래프 최적화
   Conv + BN + ReLU → FusedConvBNRelu (연산 합침)

2. 메모리 최적화
   불필요한 중간 텐서 제거

3. 커널 튜닝
   하드웨어에 최적화된 연산 커널 선택

4. 상수 폴딩
   컴파일 시 계산 가능한 것은 미리 계산
```

---

## [tip] 꼭 이해해야 할 핵심 개념

### ONNX 변환 시 자주 발생하는 문제

```
[X] 문제 1: 지원되지 않는 연산자
   → 해결: opset 버전 올리거나, 커스텀 연산 등록

[X] 문제 2: Dynamic control flow (if/for)
   → 해결: torch.jit.trace 대신 torch.jit.script

[X] 문제 3: 출력 shape 불일치
   → 해결: onnx.checker.check_model()로 검증

[X] 문제 4: 성능이 오히려 느려짐
   → 해결: 그래프 최적화 옵션 확인
   → ort.SessionOptions().graph_optimization_level
```

### 변환 후 검증 체크리스트

```
[O] 1. 모델 구조 검증
   onnx.checker.check_model(model)

[O] 2. 출력값 비교
   PyTorch 출력 vs ONNX 출력
   → np.allclose(pt_out, onnx_out, atol=1e-5)

[O] 3. 성능 벤치마크
   추론 시간 측정 (warm-up 포함)

[O] 4. 시각화
   netron.app 에서 그래프 확인
```

---

## [search] 자체 점검 - 이해했는지 확인!

### Q1: ONNX의 핵심 장점
**Q:** ONNX를 사용하는 가장 큰 이유는 무엇인가요?

**A:**
```
프레임워크 독립성 (Framework Interoperability)

PyTorch, TensorFlow 등 어떤 프레임워크로 학습하든
ONNX로 변환하면 다양한 런타임에서 추론 가능

PyTorch → ONNX → TensorRT (Jetson)
                → ONNX Runtime (서버)
                → CoreML (iOS)
```

### Q2: Static vs Dynamic Shape
**Q:** Jetson에서 YOLO를 실행할 때 Static과 Dynamic 중 어떤 것을 선택하나요?

**A:**
```
Static Shape [1, 3, 640, 640]

이유:
1. 카메라 해상도가 고정 (640x480)
2. 배치 크기 1 (실시간 단일 프레임)
3. TensorRT 최적화가 Static에서 극대화
4. 메모리 사전 할당으로 지연 시간 감소
```

### Q3: FP16 vs INT8
**Q:** FP16과 INT8의 가장 큰 차이점은?

**A:**
```
INT8은 캘리브레이션이 필요!

FP16: 단순 타입 캐스팅 (float→half)
  → 정확도 손실 거의 없음
  → 속도 ~2배 향상

INT8: 스케일 + 제로포인트 계산 필요
  → 대표 데이터셋으로 캘리브레이션
  → 정확도 손실 있을 수 있음 (1-3% mAP)
  → 속도 ~4배 향상
```

### Q4: Opset Version
**Q:** Opset 버전을 높이면 항상 좋은가요?

**A:**
```
아니요!

높은 Opset 문제점:
1. TensorRT가 지원하지 않는 연산 포함 가능
2. 호환성 문제 (구형 Runtime에서 실행 불가)
3. 더 많은 연산이 커스텀 구현 필요

권장: Opset 12-13 (안정적 + 호환성 우수)
→ 배포 타겟의 지원 범위를 반드시 확인!
```

---

## [note] 이번 주 실습 & 다음 주 준비

### 실습 항목

1. **PyTorch → ONNX 변환**
   - YOLO11n 모델 ONNX 변환
   - Static/Dynamic shape 모두 테스트
   - onnx.checker로 검증

2. **ONNX Runtime 추론**
   - CPU / GPU Provider 비교
   - 전처리/후처리 파이프라인 구현
   - 결과 시각화 (바운딩 박스)

3. **속도 벤치마크**
   - PyTorch vs ONNX Runtime 비교
   - warm-up 후 100회 평균
   - 표 정리

4. **양자화 실험**
   - FP16 변환
   - INT8 캘리브레이션 (선택)
   - 정확도/속도 트레이드오프 분석

### 다음 주 준비

```
Week 6에서는 ONNX를 TensorRT로 변환하여
Jetson에서 C++로 실시간 추론합니다!

준비:
[O] ONNX 파일 생성 완료
[O] Jetson 보드 준비 (Orin Nano)
[O] JetPack SDK 설치 확인
[O] C++ 기본 문법 복습
```

---

## [goal] 이번 주 핵심 요약

1. **ONNX = 모델의 PDF**
   - 프레임워크 독립적 포맷
   - PyTorch, TensorFlow 등에서 변환 가능
   - 다양한 런타임에서 실행 가능

2. **변환 과정**
   - model.eval() → dummy_input → torch.onnx.export()
   - Opset 12-13 권장
   - 변환 후 반드시 검증!

3. **Static vs Dynamic**
   - Jetson 배포: Static 권장
   - 서버 배포: Dynamic 가능
   - 실시간 시스템: Static 필수

4. **양자화 (Quantization)**
   - FP32 → FP16: 2배 빠름, 정확도 유지
   - FP32 → INT8: 4배 빠름, 캘리브레이션 필요
   - Jetson에서는 FP16이 가성비 최고

5. **ONNX Runtime**
   - 그래프 최적화로 PyTorch보다 빠름
   - ExecutionProvider로 하드웨어 선택
   - TensorRT EP가 가장 빠름

---

다음: [Week 6 - Jetson 배포: TensorRT (C++)](../week6/README.md)
