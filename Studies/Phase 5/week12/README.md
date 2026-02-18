# Week 12: 최종 데모 - 최적화 & 포트폴리오 (Section 5.4)

> 🎯 **이번 주 목표**: 전체 파이프라인의 성능을 최적화하고, Phase 5 학습 내용을 포트폴리오로 정리하기
> ⏰ **예상 시간**: 12시간
> 💡 **핵심 질문**: "실시간 2D Perception 시스템을 포트폴리오로 어떻게 보여줄 수 있는가?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | 파이프라인 병목 분석, 최적화 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | 성능 프로파일링, 포트폴리오 정리 심화 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | 최종 데모 최적화 및 포트폴리오 작성 |

---

## 🌟 시작하기 전에

### Phase 5 전체 여정 돌아보기

```
Week 1-2:   PyTorch 기초 + CV 라이브러리
Week 3-4:   YOLO 이론 + 학습
Week 5-6:   YOLO 배포 (ONNX, TensorRT)
Week 7-8:   Depth Estimation 이론 + 구현
Week 9:     Depth TensorRT 변환
Week 10:    Depth 정확도 검증
Week 11:    Detection + Depth 융합
Week 12:    최종 데모 + 포트폴리오   ◄── 지금 여기!
```

**이번 주에 할 일:**
```
❓ 전체 파이프라인이 목표 FPS를 달성하는가?
❓ 어디가 병목이고, 어떻게 최적화하는가?
❓ 성과를 포트폴리오로 어떻게 정리하는가?
❓ Phase 6(LiDAR)으로 넘어가기 전에 뭘 준비해야 하는가?
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. 성능 목표 및 현재 상태

#### 1.1 목표 성능

```
┌──────────────────────────────────────────────────┐
│           Phase 5 최종 성능 목표                    │
├────────────────────┬─────────────────────────────┤
│ 항목               │ 목표                          │
├────────────────────┼─────────────────────────────┤
│ 전체 파이프라인 FPS │ >= 10 FPS (Jetson)           │
│ YOLO 추론          │ < 40ms                       │
│ Depth 추론         │ < 70ms                       │
│ 융합 + 역투영      │ < 5ms                        │
│ GPU 메모리          │ < 6 GB (Jetson 8GB 기준)     │
│ Depth AbsRel       │ < 0.15                       │
│ YOLO mAP@50        │ > 0.45                       │
│ 3D 위치 오차       │ < 2m @ 20m 거리               │
└────────────────────┴─────────────────────────────┘
```

#### 1.2 파이프라인 병목 분석

```
전체 파이프라인 시간 분석:
┌──────────────────────────────────────────────┐
│  이미지 캡처     ██ (5ms)                      │
│  YOLO 전처리     █ (3ms)                       │
│  YOLO 추론       ████████████ (30ms)           │
│  YOLO 후처리     █ (2ms)                       │
│  Depth 전처리    █ (3ms)                       │
│  Depth 추론      ████████████████████ (55ms)   │  ← 병목!
│  Depth 후처리    █ (2ms)                       │
│  융합 + 역투영   █ (2ms)                       │
│  시각화/퍼블리시  █ (3ms)                       │
├──────────────────────────────────────────────┤
│  순차 합계:      ~105ms (9.5 FPS)              │
│  병렬 실행 시:   ~65ms  (15.4 FPS)             │
└──────────────────────────────────────────────┘
```

---

### 2. 최적화 전략

#### 2.1 모델 레벨 최적화

```
1. 모델 크기 줄이기
   YOLO: v8n(nano) → v8s(small) → v8m(medium)
   Depth: ViT-S(small) → ViT-B(base)
   → 작을수록 빠르지만 정확도 하락 트레이드오프

2. INT8 양자화 (고급)
   FP16 → INT8: 추가 ~1.5x 속도 향상
   단, 캘리브레이션 데이터 필요
   trtexec --int8 --calib=calibration_data/

3. 입력 크기 줄이기
   YOLO: 640 → 480 또는 320
   Depth: 384x512 → 288x384
   → 해상도↓ = 속도↑, 정확도↓
```

#### 2.2 시스템 레벨 최적화

```
1. YOLO + Depth 병렬 실행
   → CUDA 스트림 분리 또는 멀티스레딩
   → 순차 105ms → 병렬 65ms

2. 전처리/후처리 GPU 가속
   → OpenCV CUDA 모듈 사용
   → cv::cuda::resize, cv::cuda::cvtColor

3. Zero-copy 메모리 (Jetson)
   → cudaHostAllocMapped로 CPU/GPU 복사 제거
   → 메모리 복사 시간 절약

4. 프레임 스킵
   → 매 프레임 대신 2프레임마다 Depth 추론
   → YOLO는 매 프레임 실행
```

#### 2.3 Jetson 전력 모드 최적화

```bash
# MAX 성능 모드 (최대 전력, 최대 클럭)
sudo nvpmodel -m 0
sudo jetson_clocks

# 현재 모드 확인
nvpmodel -q

# 모드별 비교:
# Mode 0 (MAXN):    15W, 최대 클럭 → 최고 FPS
# Mode 1 (15W):     15W, 균형
# Mode 2 (15W_6C):  15W, 6코어
```

---

### 3. 포트폴리오 정리

#### 3.1 포트폴리오 구조

```
portfolio/
├── README.md              ← 프로젝트 개요
├── docs/
│   ├── architecture.png   ← 시스템 아키텍처 다이어그램
│   ├── results.md         ← 성능 결과 정리
│   └── demo.gif           ← 데모 GIF/동영상
├── src/
│   ├── detection/         ← YOLO 관련 코드
│   ├── depth/             ← Depth 관련 코드
│   ├── fusion/            ← 융합 파이프라인
│   └── ros2/              ← ROS2 노드
├── models/
│   ├── yolov8n.trt        ← TensorRT 엔진
│   └── depth_fp16.trt     ← TensorRT 엔진
├── configs/
│   └── camera_params.yaml ← 카메라 파라미터
└── benchmarks/
    └── performance.py     ← 벤치마크 스크립트
```

#### 3.2 포트폴리오 README 핵심 요소

```markdown
# Real-time 2D Perception Pipeline

## 개요
YOLO 객체 검출 + Monocular Depth 추정을 결합한
실시간 3D 위치 추정 시스템

## 시스템 아키텍처
[아키텍처 다이어그램]

## 성능
| 항목 | 결과 |
|------|------|
| 전체 FPS | 15 FPS @ Jetson Orin Nano |
| YOLO mAP | 0.52 @ COCO |
| Depth AbsRel | 0.09 |

## 기술 스택
- PyTorch, TensorRT, ONNX
- YOLOv8, Depth Anything
- ROS2 Humble, OpenCV
- Jetson Orin Nano (8GB)

## 데모
[데모 GIF/동영상]
```

---

### 4. Phase 5 전체 복습

#### 4.1 핵심 지식 맵

```
Phase 5: 딥러닝 기반 2D Perception
├── 기초 (Week 1-2)
│   ├── PyTorch: Tensor, autograd, DataLoader
│   └── CV 라이브러리: Albumentations, W&B, timm
│
├── 객체 검출 (Week 3-6)
│   ├── YOLO 이론: Backbone, Neck, Head
│   ├── YOLO 학습: 커스텀 데이터, Augmentation
│   ├── ONNX 변환: torch.onnx.export
│   └── TensorRT 배포: FP16, C++ 추론
│
├── 깊이 추정 (Week 7-10)
│   ├── Monocular Depth: 원리, 한계
│   ├── Depth Anything: ViT 기반 모델
│   ├── TensorRT 변환: FP16, 성능 최적화
│   └── 정확도 검증: AbsRel, RMSE, delta
│
└── 융합 & 마무리 (Week 11-12)
    ├── Detection + Depth 융합
    ├── 3D 위치 추정 (역투영)
    ├── ROS2 통합
    └── 최적화 & 포트폴리오
```

#### 4.2 Phase 5에서 배운 핵심 수식

```
1. 역투영 (Unprojection):
   X = (u - cx) * Z / fx
   Y = (v - cy) * Z / fy

2. 스테레오 깊이:
   Z = f * b / d

3. AbsRel:
   AbsRel = mean(|pred - gt| / gt)

4. delta 임계값:
   delta_t = mean(max(pred/gt, gt/pred) < t)

5. FPS 계산:
   FPS = 1000 / avg_inference_ms
```

---

### 5. Phase 6 준비 사항

#### 5.1 Phase 6 미리보기

```
Phase 6: LiDAR 3D Perception
  - 3D 포인트 클라우드 처리
  - 3D 객체 검출 (PointPillars 등)
  - LiDAR-Camera 융합
  - 센서 캘리브레이션

필요한 선행 지식:
  ✅ PyTorch (Phase 5에서 완료)
  ✅ 3D 좌표 변환 (Phase 5에서 완료)
  ✅ TensorRT 배포 (Phase 5에서 완료)
  📌 PCL (Point Cloud Library) 기초
  📌 Open3D 라이브러리
```

---

## 💡 꼭 이해해야 할 핵심 개념

### 개념 1: 병목 식별과 최적화

```
Amdahl의 법칙:
  전체 성능 향상 = 1 / ((1 - P) + P / S)
  P: 병렬화 가능한 비율
  S: 병렬화 속도 향상

예시:
  Depth 추론(55ms)이 전체(105ms)의 52%를 차지
  Depth를 2배 빠르게 해도 전체는 77.5ms
  → 병목을 찾아 집중 최적화!
```

### 개념 2: 정확도 vs 속도 트레이드오프

```
정확도를 높이면:
  - 큰 모델 (ViT-L) → 느림
  - 높은 해상도 (640x640) → 느림
  - FP32 → 느림

속도를 높이면:
  - 작은 모델 (ViT-S) → 부정확
  - 낮은 해상도 (320x320) → 부정확
  - INT8 → 정밀도 손실

균형점 찾기:
  → 응용 목적에 따라 결정
  → 자율주행: 정확도 우선
  → 로봇 네비게이션: 속도 우선
```

### 개념 3: 포트폴리오에서 보여줘야 할 것

```
1. 문제 정의: "무엇을 왜 만들었는가?"
2. 시스템 설계: "어떻게 설계했는가?"
3. 구현 상세: "핵심 코드는 무엇인가?"
4. 성능 결과: "얼마나 빠르고 정확한가?"
5. 데모: "실제로 동작하는 모습"
6. 개선 방향: "더 나아지려면?"
```

---

## 🔍 자체 점검 - 이해했는지 확인!

**Q1: 전체 파이프라인에서 가장 큰 병목은 무엇이고, 어떻게 해결하는가?**

<details>
<summary>정답 보기</summary>

- Depth 추론(~55ms)이 가장 큰 병목
- 해결 방법:
  1. YOLO와 Depth를 CUDA 스트림으로 병렬 실행
  2. Depth 모델 크기 줄이기 (ViT-S)
  3. 입력 해상도 줄이기 (384x512 → 288x384)
  4. INT8 양자화 적용
  5. 2프레임마다 Depth 실행 (프레임 스킵)

</details>

**Q2: FP16 → INT8 변환 시 주의할 점은?**

<details>
<summary>정답 보기</summary>

- 캘리브레이션 데이터가 필요 (대표적인 입력 이미지 100-1000장)
- FP16보다 정밀도 손실이 큼 (2-5%)
- 모든 레이어가 INT8을 지원하지 않을 수 있음 (자동 fallback)
- trtexec --int8 --calib=calibration_data/ 로 변환
- 반드시 변환 후 정확도 검증 필요 (AbsRel, mAP 재측정)

</details>

**Q3: 포트폴리오에서 가장 중요한 요소 3가지는?**

<details>
<summary>정답 보기</summary>

1. **데모 영상/GIF**: 실제 동작을 보여주는 것이 가장 임팩트 있음
2. **성능 수치**: FPS, 정확도, 메모리 사용량 등 정량적 결과
3. **시스템 아키텍처 다이어그램**: 전체 구조를 한눈에 볼 수 있는 그림

추가로 좋은 것:
- 트러블슈팅 과정 기록
- 개선 아이디어 (미래 작업)
- 깔끔한 코드 구조

</details>

**Q4: Phase 5에서 배운 역투영 공식을 한 줄로 쓰시오.**

<details>
<summary>정답 보기</summary>

```
X = (u - cx) * Z / fx,  Y = (v - cy) * Z / fy,  Z = depth_map[v, u]
```

- (u, v): 픽셀 좌표
- (fx, fy, cx, cy): 카메라 내부 파라미터
- Z: 깊이 맵에서 읽은 깊이 값

</details>

---

## 📝 이번 주 실습 & Phase 5 마무리

### 실습 구성

| Step | 내용 | 예상 시간 |
|------|------|----------|
| 1 | 전체 파이프라인 벤치마크 | 2시간 |
| 2 | 병목 분석 및 최적화 적용 | 3시간 |
| 3 | 최종 성능 측정 | 2시간 |
| 4 | 포트폴리오 정리 | 3시간 |
| 5 | Phase 5 복습 및 Phase 6 준비 | 2시간 |

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

### Phase 6 준비

```
Phase 6에서 다룰 내용:
- 3D 포인트 클라우드 처리 (Open3D, PCL)
- LiDAR 데이터 전처리 (복셀화, 지면 제거)
- 3D 객체 검출 (PointPillars, CenterPoint)
- LiDAR-Camera 센서 융합

사전 설치:
  pip install open3d
  # PCL은 OS 패키지 매니저로 설치
```

---

## 🎯 이번 주 핵심 요약

### 1. 최적화 전략

```
모델 레벨: 크기 줄이기, INT8, 입력 해상도↓
시스템 레벨: 병렬 실행, GPU 가속, Zero-copy
Jetson 레벨: MAXN 모드, jetson_clocks
```

### 2. 성능 목표

```
전체 FPS >= 10 (Jetson Orin Nano)
GPU 메모리 < 6GB
Depth AbsRel < 0.15
YOLO mAP@50 > 0.45
```

### 3. 포트폴리오 핵심

```
데모 영상 + 성능 수치 + 아키텍처 다이어그램
문제 정의 → 설계 → 구현 → 결과 → 개선 방향
```

### 4. Phase 5 핵심 수식

```
역투영: X = (u-cx)*Z/fx, Y = (v-cy)*Z/fy
깊이: Z = f*b/d (스테레오)
평가: AbsRel = mean(|pred-gt|/gt)
```

---

이전: [Week 11 - Detection + Depth 융합](../week11/README.md)

다음: [Phase 6 - LiDAR 3D Perception](../../../Roadmap/Phase%206.md)
