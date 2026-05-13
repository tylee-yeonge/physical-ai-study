# Week 7: Monocular Depth Estimation 이론 (Section 5.3)

> [goal] **이번 주 목표**: 단안 카메라 깊이 추정의 원리와 핵심 모델(MiDaS, DPT, Depth Anything) 이해하기
> [time] **예상 시간**: 12시간
> [tip] **핵심 질문**: "카메라 한 대로 어떻게 물체의 거리를 알 수 있을까?"

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | Depth 추정 방식, MiDaS, ViT 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | Depth 모델 구조, Teacher-Student 학습 심화 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | Monocular Depth Estimation 이론 및 모델 탐구 |

---

## [*] 시작하기 전에

### Week 6 복습

**지금까지의 흐름:**
```
Week 3-4: YOLO11로 '무엇(What)'을 검출
Week 5-6: 모델 배포 최적화 (ONNX, TensorRT)

[!] 문제: 객체를 찾았지만 '얼마나 멀리(How far)' 있는지 모름!
```

**왜 깊이 정보가 필요한가?**
```
AMR (자율 이동 로봇) 시나리오:

카메라로 '사람'을 검출했다!
→ 사람이 2m 앞에 있나? 10m 앞에 있나?
→ 2m면 즉시 정지! 10m면 감속만!

바운딩 박스만으로는:
  +---------+
  |  사람    |  ← 이 박스가 크면 가까운 건가?
  +---------+     작은 사람일 수도 있지 않나?

→ 깊이 정보가 있으면 정확한 거리 판단 가능!
```

**비유:**
```
YOLO = 눈으로 '무엇이 있는지' 보는 것
Depth = 눈으로 '얼마나 멀리 있는지' 판단하는 것

사람의 양안 시차(스테레오) → 깊이 인식
AI의 단안 깊이 추정 → 단일 이미지에서 깊이를 학습으로 추론!
```

---

## [ref] 핵심 개념 자세히 알아보기

### 1. 깊이 추정 방법론 비교

**하드웨어 기반:**
```
+-------------------------------------------------+
|                 깊이 센서 종류                     |
|                                                   |
|  [img] 스테레오 카메라 (Stereo Camera)               |
|     두 카메라의 시차(disparity)로 깊이 계산         |
|     장점: 정확함                                  |
|     단점: 두 카메라 필요, 캘리브레이션 복잡          |
|                                                   |
|   LiDAR (Light Detection And Ranging)           |
|     레이저 반사 시간으로 거리 측정                   |
|     장점: 매우 정확, 넓은 범위                      |
|     단점: 비쌈 ($200~$10,000+), 밀도 낮음          |
|                                                   |
|  [!] RGB-D 카메라 (Depth Camera)                    |
|     적외선 패턴으로 깊이 측정 (Intel RealSense 등)   |
|     장점: 저렴, 밀집 깊이맵                         |
|     단점: 야외/강한 빛에서 불안정, 범위 제한         |
|                                                   |
|   Monocular Depth Estimation (단안 깊이 추정)    |
|     딥러닝으로 단일 이미지에서 깊이를 추론!          |
|     장점: 카메라 1개면 충분, 추가 센서 불필요        |
|     단점: 절대 깊이가 아닌 상대적 깊이              |
+-------------------------------------------------+
```

**왜 단안 깊이 추정을 사용하나?**
```
AMR에서의 현실적 고려사항:

비용:     LiDAR $5,000 vs 카메라 $30
무게:     LiDAR 1kg+ vs 카메라 30g
전력:     LiDAR 15W vs 카메라 0.5W
크기:     LiDAR 큼 vs 카메라 작음

→ 이미 장착된 카메라로 깊이도 추정할 수 있다면?
→ 추가 비용 $0! 추가 센서 불필요!

단, 정밀도는 LiDAR보다 낮음
→ 장애물 감지 등 대략적 거리 판단에 적합
→ 정밀 측정이 필요하면 다른 센서와 융합(Sensor Fusion)
```

---

### 2. 학습 방법론: Supervised vs Self-supervised vs Zero-shot

#### 2.1 Supervised (지도 학습)

```
훈련 데이터:
  RGB 이미지 + 정답 깊이맵 (Ground Truth)

데이터 수집:
  LiDAR로 깊이 측정 → 카메라 이미지와 동기화
  예: KITTI, NYU Depth V2 데이터셋

+------------+     +----------+     +------------+
| RGB 이미지   | --→ |  모델    | --→ | 예측 깊이맵  |
| (입력)      |     | (학습)   |     | (출력)      |
+------------+     +----------+     +------------+
                        ↑
                   +------------+
                   | 정답 깊이맵  |
                   | (LiDAR)    |
                   +------------+

손실 함수:
  L_depth = |predicted_depth - gt_depth|

장점: 정확도 높음
단점: Ground Truth 수집 비용이 큼
      특정 환경에 과적합 가능
```

#### 2.2 Self-supervised (자기 지도 학습)

```
핵심 아이디어: "정답 깊이맵 없이 학습!"

연속된 프레임(또는 스테레오 쌍)의 기하학적 관계를 활용:

프레임 t:     프레임 t+1:
+--------+    +--------+
| 시점 A  |    | 시점 B  |
| (원본)  |    | (변환)  |
+--------+    +--------+

깊이를 예측하면, 한 시점의 이미지를 다른 시점으로 변환 가능:
  I_reconstructed = warp(I_t+1, depth_t, pose_t→t+1)

손실 함수:
  L_photo = |I_t - I_reconstructed|
  (재구성된 이미지가 원본과 비슷해야 함)

장점: Ground Truth 깊이맵 불필요! 동영상만 있으면 학습
단점: 움직이는 물체, 가려짐 등에서 어려움
      절대 스케일 복구 불가 (scale ambiguity)

대표 모델: Monodepth2 (2019)
```

#### 2.3 Zero-shot (대규모 사전학습)

```
핵심 아이디어: "엄청나게 다양한 데이터로 학습하면,
              처음 보는 장면에서도 잘 작동한다!"

+-----------------------------------------+
|      대규모 데이터 혼합 학습              |
|                                          |
|  KITTI (야외 운전)     +  62M 이미지    |
|  NYU Depth (실내)      +  다양한 환경    |
|  Waymo (자율주행)      +  다양한 카메라   |
|  MegaDepth (인터넷)    +  다양한 해상도   |
|  ...                                    |
|                                          |
|  → 환경/카메라 독립적인 깊이 이해 학습!   |
+-----------------------------------------+

→ 학습하지 않은 새로운 장면에서도 잘 작동
→ Fine-tuning 없이 바로 사용 가능 (Zero-shot)

대표 모델: MiDaS, DPT, Depth Anything
```

---

### 3. 핵심 모델 1: MiDaS (2019~2022)

**MiDaS = Monocular Depth in the wild using Mixing Datasets**

```
핵심 기여:
  여러 데이터셋을 '혼합' 학습하는 방법론 제시!

문제점:
  각 데이터셋의 깊이 스케일이 다름
  KITTI: 0~80m (야외)
  NYU: 0~10m (실내)
  → 단순 혼합하면 스케일 충돌!

해결:
  Scale-and-Shift Invariant Loss
  (스케일과 이동에 무관한 손실 함수)

  L = |s * d_pred + t - d_gt|
  여기서 s, t는 최적화로 자동 결정

효과:
  → 서로 다른 깊이 범위의 데이터도 함께 학습 가능!
  → 12개 데이터셋 혼합 학습
```

**MiDaS 아키텍처 변화:**
```
MiDaS v1 (2019): ResNeXt-101 backbone
MiDaS v2 (2020): EfficientNet backbone
MiDaS v3 (2021): ViT (Vision Transformer) backbone → DPT
MiDaS v3.1 (2022): BEiT, Swin backbone 추가

→ Backbone이 CNN에서 Transformer로 발전!
```

---

### 4. 핵심 모델 2: DPT (2021)

**DPT = Dense Prediction Transformer**

```
핵심 아이디어:
  ViT(Vision Transformer)를 Dense Prediction에 적용!

기존 CNN의 한계:
  +-----+ → +---+ → +-+ → 전역 정보 부족
  |     |   |   |   | |   Receptive field 제한
  +-----+   +---+   +-+

ViT의 장점:
  +-------------------------+
  | Self-Attention으로       |
  | 이미지 전체를 한 번에 봄! |
  | → 전역 정보 + 지역 정보  |
  +-------------------------+

DPT 구조:
  이미지 → 패치 분할 → ViT Encoder → Multi-scale Feature → Decoder → 깊이맵
              ↓              ↓               ↓
         14x14 패치     전역 Self-Attention   피라미드 결합
```

**DPT가 MiDaS v3에 사용된 이유:**
```
DPT의 성능:
- 전역 컨텍스트(하늘, 바닥 등) 이해 우수
- 가려진 물체의 깊이도 잘 추정
- Multi-scale feature로 세밀한 경계 유지

CNN vs ViT 깊이 추정 비교:
+----------+----------+----------+
|  특성     |   CNN    |   ViT    |
+----------+----------+----------+
| 전역 정보 |   약함   |   강함   |
| 경계 정밀 |   보통   |   우수   |
| 연산 비용 |   낮음   |   높음   |
| 파라미터  |   적음   |   많음   |
| 정확도    |   좋음   |  매우 좋음|
+----------+----------+----------+
```

---

### 5. 핵심 모델 3: Depth Anything (2024)

**Depth Anything = 대규모 비라벨 데이터 활용 Depth 모델**

```
핵심 혁신:
  라벨 있는 데이터 1.5M + 라벨 없는 데이터 62M!

기존 모델 한계:
  MiDaS/DPT: 라벨 있는 데이터만 사용 (~1.5M 이미지)
  → 다양한 환경에 대한 일반화 한계

Depth Anything의 접근:
  Step 1: 라벨 있는 1.5M으로 Teacher 모델 학습
  Step 2: Teacher가 62M 비라벨 이미지에 의사(pseudo) 깊이맵 생성
  Step 3: Student 모델이 Teacher의 의사 라벨로 학습
           + 강한 Augmentation 적용 (Teacher보다 강건하게!)
```

**Teacher-Student 학습 프레임워크:**
```
+-------------------------------+
|          Teacher 모델          |
|  (라벨 데이터 1.5M으로 학습)   |
|  (DPT-Large 기반)              |
+--------------+----------------+
               | 의사 깊이맵 생성
               ↓
     +-------------------+
     | 비라벨 이미지 62M   |
     | + 의사 깊이맵       |
     +--------+----------+
              |
              ↓
+-------------------------------+
|         Student 모델           |
|  (ViT-S / ViT-B / ViT-L)     |
|  + 강한 Augmentation          |
|  + 라벨 데이터 1.5M 병행 학습  |
+-------------------------------+
```

**Depth Anything 성능 비교:**
```
+--------------+----------+----------+----------+
|  모델         | 파라미터  |  Zero-shot|  속도    |
|              |          |  성능     | (A100)   |
+--------------+----------+----------+----------+
| MiDaS v3.1  |  345M    |  보통     |  ~40ms   |
| DPT-Large   |  343M    |  좋음     |  ~45ms   |
| Depth Any-S |  24.8M   |  우수     |  ~12ms   |
| Depth Any-B |  97.5M   |  매우 우수|  ~25ms   |
| Depth Any-L |  335.3M  |  최고     |  ~48ms   |
+--------------+----------+----------+----------+

핵심: ViT-S(24.8M)로도 DPT-Large(343M)보다 좋은 성능!
→ 파라미터 14배 적으면서 더 나은 일반화
→ Jetson 배포에 최적!
```

---

### 6. Metric Depth vs Relative Depth

**매우 중요한 개념!**

```
Relative Depth (상대 깊이):
  값 범위: 0.0 ~ 1.0 (정규화)
  의미: 이 픽셀이 다른 픽셀보다 가까운가/먼가?
  예: 사람(0.8) vs 건물(0.3) → 사람이 더 가까움
  → MiDaS, DPT, Depth Anything 기본 출력

  장점:
    - 카메라 파라미터 불필요
    - 어떤 이미지에서든 동작
    - 학습/일반화 쉬움

  단점:
    - "3미터 앞에 장애물" 같은 판단 불가
    - 절대 거리를 모름

Metric Depth (절대 깊이):
  값 범위: 0.0 ~ 80.0 m (실제 거리, 미터 단위)
  의미: 이 픽셀의 실제 거리는 몇 미터?
  예: 사람(2.5m), 건물(15.3m)
  → Metric3D, ZoeDepth, Depth Anything v2 (metric)

  장점:
    - 실제 거리 판단 가능
    - SLAM, 장애물 회피에 직접 사용 가능

  단점:
    - 카메라 캘리브레이션 필요한 경우 많음
    - 학습 데이터의 스케일에 의존
    - 일반화 어려움
```

**AMR에서의 선택:**
```
상대 깊이로 충분한 경우:
  - 장애물 유무 판단 (있다/없다)
  - 대략적 방향 결정 (왼쪽이 더 가까움)

절대 깊이가 필요한 경우:
  - 정확한 정지 거리 판단 (2m 앞에서 정지)
  - 경로 계획 (A 경로 5m vs B 경로 8m)
  - SLAM 스케일 복구

추천 전략:
  Step 1: 상대 깊이로 시작 (Depth Anything)
  Step 2: 필요하면 스케일 보정 추가 (known object 활용)
  Step 3: 고정밀 필요하면 RGB-D 카메라 추가
```

---

### 7. Depth Anything v2 (2024)

```
v1 대비 개선점:

1. Synthetic Data 활용
   실제 데이터 + 합성 데이터 혼합
   → 더 깨끗한 Ground Truth

2. 더 나은 Teacher 모델
   DINOv2 기반 Teacher
   → 더 정확한 의사 라벨

3. Metric Depth 지원
   상대 깊이 뿐 아니라 절대 깊이 모델도 제공
   → 실내/야외 metric 모델 별도 제공

4. 성능 향상
   +--------------+----------+----------+
   |  모델         |   v1     |   v2     |
   +--------------+----------+----------+
   | Depth Any-S  |  0.756   |  0.821   |
   | Depth Any-B  |  0.804   |  0.868   |
   | Depth Any-L  |  0.836   |  0.891   |
   +--------------+----------+----------+
   (AbsRel 기준, 낮을수록 좋음이 아닌 정확도 기준)
```

---

## [tip] 꼭 이해해야 할 핵심 개념

### Vision Transformer (ViT) 기초

```
이미지 → 패치 분할:
+--+--+--+--+
|P1|P2|P3|P4|    이미지를 14x14 패치로 분할
+--+--+--+--+    → 각 패치를 벡터로 변환 (Embedding)
|P5|P6|P7|P8|    → Transformer에 입력
+--+--+--+--+
|P9|..|..|..|
+--+--+--+--+

Self-Attention:
  각 패치가 다른 모든 패치와의 관계를 학습
  → P1이 P8과 관련있으면 높은 attention
  → 하늘 패치와 바닥 패치의 관계 → 깊이 단서!

왜 깊이 추정에 좋은가?
  - 하늘은 항상 멀리 있음 → 전역 패턴
  - 바닥의 텍스처 변화 → 기울기/깊이 단서
  - 사물의 크기 비교 → 상대적 거리
  → Self-Attention이 이런 전역 관계를 잘 포착!
```

### Scale-Invariant Loss 이해

```
문제: 데이터셋마다 깊이 스케일이 다름
  KITTI: depth = 0 ~ 80m
  NYU:   depth = 0 ~ 10m

일반 L1 Loss:
  |pred - gt| → 스케일에 민감!
  80m 짜리 예측은 항상 큰 오차

Scale-Invariant Loss:
  d_i = log(pred_i) - log(gt_i)   (로그 공간에서 차이)
  L = (1/n) * sum(d_i^2) - (1/n^2) * (sum(d_i))^2

  → 전체적인 스케일 차이는 무시하고
  → 상대적인 깊이 순서(구조)만 학습!

비유:
  일반 시험: 정확한 점수 맞추기 (80점!)
  Scale-Invariant: 순서만 맞추기 (1등 > 2등 > 3등)
```

---

## [search] 자체 점검 - 이해했는지 확인!

### Q1: Monocular Depth의 한계
**Q:** 단안 깊이 추정의 근본적인 한계는 무엇이고, 어떻게 보완하나요?

**A:**
```
근본 한계: Scale Ambiguity (스케일 모호성)
- 단일 이미지에서는 절대 거리를 알 수 없음
- 작은 물체가 가까이 있는 것 vs 큰 물체가 멀리 있는 것 구분 불가

보완 방법:
1. Known Object 크기 활용 (사람 키 ~170cm)
2. Camera intrinsics로 스케일 복구
3. LiDAR/RGB-D와 센서 융합
4. Metric Depth 모델 사용 (Depth Anything v2 Metric)
```

### Q2: Teacher-Student 학습
**Q:** Depth Anything이 Teacher-Student 프레임워크를 사용하는 이유는?

**A:**
```
라벨 있는 데이터: 1.5M (수집 비용 높음)
라벨 없는 데이터: 62M (인터넷에서 쉽게 수집)

Teacher: 라벨 데이터로 학습 (정확하지만 일반화 한계)
Student: Teacher의 의사 라벨 + 강한 Augmentation으로 학습
  → 62M의 다양한 데이터로 일반화 능력 극대화
  → Teacher보다 더 강건한 모델!

핵심: 적은 라벨로 대규모 비라벨 데이터를 활용하는 전략
```

### Q3: 모델 선택
**Q:** Jetson Orin Nano에서 실시간 깊이 추정을 하려면 어떤 모델을 선택하나요?

**A:**
```
Depth Anything ViT-S (Small) 권장!

이유:
1. 파라미터 24.8M (가볍다)
2. Zero-shot 성능 우수 (다양한 환경 대응)
3. TensorRT FP16 변환 시 ~50ms 추론 (15-20 FPS)
4. 8GB 메모리에서 YOLO와 동시 실행 가능

ViT-B, ViT-L은 Jetson에서 실시간 어려움
→ 서버 또는 데스크톱 GPU용
```

### Q4: Relative vs Metric Depth
**Q:** AMR 장애물 회피에서 상대 깊이만으로 충분할까요?

**A:**
```
기본적인 장애물 회피에는 충분할 수 있음:
- "앞에 무언가 가까이 있다" → 정지/회전
- 상대적으로 더 먼 방향으로 이동

하지만 정밀한 제어에는 부족:
- "2m 앞에서 정확히 정지" → 절대 깊이 필요
- 경로 계획 시 실제 거리 필요

실용적 접근:
  상대 깊이 + 안전 마진(margin) 적용
  예: 상대 깊이 > 0.7이면 "매우 가까움" → 정지
  또는 Scale 보정 (known reference 활용)
```

---

## [note] 이번 주 실습 & 다음 주 준비

### 실습 항목

1. **MiDaS 논문 핵심 정리**
   - Scale-and-Shift Invariant Loss 이해
   - 다중 데이터셋 혼합 학습 전략

2. **Depth Anything 논문 핵심 정리**
   - Teacher-Student 프레임워크
   - 비라벨 데이터 활용 전략

3. **모델 비교 분석**
   - MiDaS vs DPT vs Depth Anything 비교표 작성
   - 용도별 최적 모델 선택 기준 정리

4. **Python 실험 환경 준비**
   - transformers 라이브러리 설치
   - HuggingFace 모델 다운로드 테스트

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

### 다음 주 준비

```
Week 8에서는 Depth Anything 모델을 실제로 사용합니다!

준비:
  Python 환경 (torch, transformers, timm)
  테스트 이미지 준비 (실내/야외)
  HuggingFace 계정 생성
  Depth Anything 논문 Abstract 읽기
```

---

## [goal] 이번 주 핵심 요약

1. **왜 Monocular Depth인가?**
   - 카메라 1대로 깊이 정보 추출
   - 추가 센서 비용 $0
   - AMR 장애물 거리 판단에 필수

2. **학습 방법론**
   - Supervised: GT 깊이 필요, 정확
   - Self-supervised: GT 불필요, 제한적
   - Zero-shot: 대규모 학습, 강건한 일반화

3. **핵심 모델 발전사**
   - MiDaS (2019): 다중 데이터셋 혼합 학습
   - DPT (2021): ViT를 깊이 추정에 적용
   - Depth Anything (2024): 62M 비라벨 데이터 활용

4. **Relative vs Metric Depth**
   - Relative: 순서만, 어디서든 동작
   - Metric: 실제 거리, 카메라 의존

5. **Jetson 배포 전략**
   - Depth Anything ViT-S 선택
   - TensorRT FP16으로 15-20 FPS 달성
   - YOLO와 동시 실행 가능

---

이전: [Week 6 - Jetson 배포: TensorRT (C++)](../week6/README.md)
다음: [Week 8 - Depth Anything 사용](../week8/README.md)
