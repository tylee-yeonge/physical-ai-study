# Week 7: Monocular Depth Estimation 이론 (Section 5.3)

> **이번 주 목표**: 단안 카메라 깊이 추정의 원리와 핵심 모델(MiDaS, DPT, Depth Anything) 이해하기
> **예상 시간**: 12시간
> **핵심 질문**: "카메라 한 대로 어떻게 물체의 거리를 알 수 있을까?"

---

## 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | Depth 추정 방식, MiDaS, ViT 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | Depth 모델 구조, Teacher-Student 학습 심화 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | Monocular Depth Estimation 이론 및 모델 탐구 |

---

## 시작하기 전에

### Week 6 복습

Week 3-6에서 YOLO로 "무엇이 있는가"를 검출하고, 그 모델을 Jetson에 빠르게 배포하는 법을 배웠다. 그런데 검출만으로는 부족한 정보가 있다 - **거리**다.

```
Week 3-4: YOLO11로 '무엇(What)'을 검출
Week 5-6: 모델 배포 최적화 (ONNX, TensorRT)


문제: 객체를 찾았지만 '얼마나 멀리(How far)' 있는지 모름!
```

거리를 모르면 로봇이 제대로 움직일 수 없다.

```
AMR (자율 이동 로봇) 시나리오:


카메라로 '사람'을 검출했다!
→ 사람이 2m 앞에 있나? 10m 앞에 있나?
→ 2m면 즉시 정지! 10m면 감속만!


바운딩 박스만으로는:
  +---------+
  | 사람 | ← 이 박스가 크면 가까운 건가?
  +---------+ 작은 사람일 수도 있지 않나?


→ 깊이 정보가 있으면 정확한 거리 판단 가능!
```

BBox 크기로 거리를 짐작하려 해도, "가까운 작은 사람"과 "먼 큰 사람"을 구별할 수 없다. 그래서 **깊이**(depth)를 따로 추정해야 한다.

```
YOLO = 눈으로 '무엇이 있는지' 보는 것
Depth = 눈으로 '얼마나 멀리 있는지' 판단하는 것


사람의 양안 시차(스테레오) → 깊이 인식
AI의 단안 깊이 추정 → 단일 이미지에서 깊이를 학습으로 추론!
```

---

## 핵심 개념 자세히 알아보기

### 1. 깊이 추정 방법론 비교

깊이를 얻는 방법은 크게 **하드웨어 센서**를 쓰는 것과 **딥러닝**(단안 추정)으로 추론하는 것으로 나뉜다.

**깊이 센서 종류 비교**

| 종류 | 원리 | 장점 | 단점 |
|---|---|---|---|
| **스테레오 카메라** (Stereo Camera) | 두 카메라의 시차(disparity)로 깊이 계산 | 정확함 | 카메라 2개 필요, 캘리브레이션 복잡 |
| **LiDAR** (Light Detection And Ranging) | 레이저 반사 시간으로 거리 측정 | 매우 정확, 넓은 범위 | 비쌈 ($200-$10,000+), 밀도 낮음 |
| **RGB-D 카메라** (Depth Camera) | 적외선 패턴으로 깊이 측정 (Intel RealSense 등) | 저렴, 밀집 깊이맵 | 야외/강한 빛에서 불안정, 범위 제한 |
| **Monocular Depth Estimation** (단안 깊이 추정) | 딥러닝으로 단일 이미지에서 깊이 추론 | 카메라 1개면 충분, 추가 센서 불필요 | 절대 깊이가 아닌 상대적 깊이 |

**왜 단안 깊이 추정인가?** AMR 같은 소형 로봇에서는 비용, 무게, 전력이 모두 제약이다.

```
AMR에서의 현실적 고려사항:


비용: LiDAR $5,000 vs 카메라 $30
무게: LiDAR 1kg+ vs 카메라 30g
전력: LiDAR 15W vs 카메라 0.5W
크기: LiDAR 큼 vs 카메라 작음


→ 이미 장착된 카메라로 깊이도 추정할 수 있다면?
→ 추가 비용 $0! 추가 센서 불필요!
```

이미 객체 검출용으로 달려 있는 카메라 한 대로 깊이까지 얻을 수 있다면, 추가 비용 없이 거리 정보를 얻는 셈이다. 단, 정밀도는 LiDAR보다 낮으므로 "대략적 거리 판단"에 적합하고, 정밀 측정이 필요하면 다른 센서와 융합(Sensor Fusion)한다. `quiz_easy.py` 문제 1이 이 비교를 묻는다.

---

### 2. 학습 방법론: Supervised vs Self-supervised vs Zero-shot

깊이 추정 모델을 "어떻게 학습시키느냐"에 세 가지 접근이 있다.

#### 2.1 Supervised (지도 학습)

정답 깊이맵(Ground Truth)을 주고 학습한다. 가장 직관적이지만, 정답을 모으는 비용이 크다.

```
훈련 데이터:
  RGB 이미지 + 정답 깊이맵 (Ground Truth)


데이터 수집:
  LiDAR로 깊이 측정 → 카메라 이미지와 동기화
  예: KITTI, NYU Depth V2 데이터셋


손실 함수:
  L_depth = |predicted_depth - gt_depth|


장점: 정확도 높음
단점: Ground Truth 수집 비용이 큼, 특정 환경에 과적합 가능
```

#### 2.2 Self-supervised (자기 지도 학습)

정답 깊이맵 없이, 영상의 기하학적 관계만으로 학습한다.

```
핵심 아이디어: "정답 깊이맵 없이 학습!"


연속된 프레임(또는 스테레오 쌍)의 기하학적 관계를 활용:


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

핵심 발상: 깊이를 제대로 예측했다면, 그 깊이를 이용해 한 프레임을 다음 프레임 시점으로 "변환(warp)"했을 때 실제 다음 프레임과 같아야 한다. 이 "같아야 한다"를 손실로 삼으면 정답 깊이 없이도 학습할 수 있다.

#### 2.3 Zero-shot (대규모 사전학습)

엄청나게 다양한 데이터로 학습해, 처음 보는 장면에서도 바로 잘 동작하게 한다.

```
핵심 아이디어: "엄청나게 다양한 데이터로 학습하면,
              처음 보는 장면에서도 잘 작동한다!"


KITTI (야외 운전) + NYU Depth (실내) + Waymo (자율주행)
+ MegaDepth (인터넷) + ... 다양한 환경/카메라


→ 환경/카메라 독립적인 깊이 이해 학습!
→ 학습하지 않은 새로운 장면에서도 잘 작동
→ Fine-tuning 없이 바로 사용 가능 (Zero-shot)


대표 모델: MiDaS, DPT, Depth Anything
```

**Zero-shot의 핵심 장점은 Fine-tuning이 필요 없다는 것**이다. 다양한 데이터로 미리 충분히 학습했기 때문에, 새 환경에 추가 학습 없이 바로 쓸 수 있다. `quiz_easy.py` 문제 2가 세 방법론을 구별하게 한다.

---

### 3. 핵심 모델 1: MiDaS (2019-2022)

**MiDaS**의 핵심 기여는 "서로 다른 데이터셋을 섞어 학습하는 방법"을 제시한 것이다.

```
문제점:
  각 데이터셋의 깊이 스케일이 다름
  KITTI: 0-80m (야외)
  NYU: 0-10m (실내)
  → 단순 혼합하면 스케일 충돌!


해결:
  Scale-and-Shift Invariant Loss
  (스케일과 이동에 무관한 손실 함수)
  → 서로 다른 깊이 범위의 데이터도 함께 학습 가능!
  → 12개 데이터셋 혼합 학습
```

야외 데이터(0-80m)와 실내 데이터(0-10m)를 그냥 섞으면, 손실 함수가 야외의 큰 숫자에 휘둘려 실내 학습이 무시된다. MiDaS는 **스케일에 무관한 손실**(§꼭 이해)로 이 문제를 풀었다.

```
MiDaS 아키텍처 변화:
MiDaS v1 (2019): ResNeXt-101 backbone (CNN)
MiDaS v2 (2020): EfficientNet backbone (CNN)
MiDaS v3 (2021): ViT (Vision Transformer) backbone → DPT
MiDaS v3.1 (2022): BEiT, Swin backbone 추가


→ Backbone이 CNN에서 Transformer로 발전!
```

---

### 4. 핵심 모델 2: DPT (2021)

**DPT = Dense Prediction Transformer.** ViT(Vision Transformer)를 깊이 추정에 적용한 모델이다.

CNN은 작은 영역씩 보며 점점 넓혀 가는 구조라, 이미지 전체를 한눈에 보는 능력이 약하다. ViT는 Self-Attention으로 이미지 전체의 관계를 한 번에 본다.

```
기존 CNN의 한계:
  → 전역 정보 부족, Receptive field 제한


ViT의 장점:
  +-------------------------+
  | Self-Attention으로 |
  | 이미지 전체를 한 번에 봄! |
  | → 전역 정보 + 지역 정보 |
  +-------------------------+


DPT 구조:
  이미지 → 패치 분할 → ViT Encoder → Multi-scale Feature → Decoder → 깊이맵
```

깊이 추정에 전역 정보가 왜 중요한가? "하늘은 멀고 바닥은 가깝다" 같은 단서는 이미지 전체 맥락을 봐야 잡힌다. ViT의 전역 시야가 이런 단서를 잘 포착한다.

CNN vs ViT 깊이 추정 비교:

| 특성      | CNN  | ViT       |
|-----------|------|-----------|
| 전역 정보 | 약함 | 강함      |
| 경계 정밀 | 보통 | 우수      |
| 연산 비용 | 낮음 | 높음      |
| 파라미터  | 적음 | 많음      |
| 정확도    | 좋음 | 매우 좋음 |

---

### 5. 핵심 모델 3: Depth Anything (2024)

**Depth Anything**의 핵심 혁신은 **라벨 없는 데이터 62M장**을 활용한 것이다.

```
기존 모델 한계:
  MiDaS/DPT: 라벨 있는 데이터만 사용 (~1.5M 이미지)
  → 다양한 환경에 대한 일반화 한계


Depth Anything의 접근:
  Step 1: 라벨 있는 1.5M으로 Teacher 모델 학습
  Step 2: Teacher가 62M 비라벨 이미지에 의사(pseudo) 깊이맵 생성
  Step 3: Student 모델이 Teacher의 의사 라벨로 학습
           + 강한 Augmentation 적용 (Teacher보다 강건하게!)
```

이것이 **Teacher-Student 프레임워크**다. 라벨이 있는 데이터는 비싸서 적게(1.5M)밖에 못 모은다. 반면 라벨 없는 인터넷 이미지는 거의 무한하다(62M). 그래서 적은 라벨로 똑똑한 Teacher를 만들고, Teacher가 62M장에 "의사 정답(pseudo label)"을 붙여 주면, Student가 그 방대한 데이터로 학습한다.

```
+-------------------------------+
| Teacher 모델 |
| (라벨 데이터 1.5M으로 학습) |
| (DPT-Large 기반) |
+--------------+----------------+
               | 의사 깊이맵 생성
               ↓
     +-------------------+
     | 비라벨 이미지 62M |
     | + 의사 깊이맵 |
     +--------+----------+
              |
              ↓
+-------------------------------+
| Student 모델 |
| (ViT-S / ViT-B / ViT-L) |
| + 강한 Augmentation |
| + 라벨 데이터 1.5M 병행 학습 |
+-------------------------------+
```

**Student가 Teacher보다 좋아질 수 있는 이유**는 (1) 62M의 다양한 데이터로 일반화 능력이 극대화되고, (2) Teacher보다 강한 Augmentation으로 더 어려운 조건에서 학습하기 때문이다. `quiz_medium.py` 문제 1이 이 과정을 단계별로 묻는다.

Depth Anything 성능 비교:

| 모델        | 파라미터 | Zero-shot 성능 |
|-------------|---------|----------------|
| MiDaS v3.1  | 345M    | 보통           |
| DPT-Large   | 343M    | 좋음           |
| Depth Any-S | 24.8M   | 우수           |
| Depth Any-B | 97.5M   | 매우 우수      |
| Depth Any-L | 335.3M  | 최고           |

핵심: ViT-S(24.8M)로도 DPT-Large(343M)보다 좋은 성능 - 파라미터 14배 적으면서 더 나은 일반화. Jetson 배포에 최적.

작은 ViT-S가 14배 큰 DPT-Large를 능가한다는 것이 핵심이다. 작고 빠르면서도 성능이 좋아 Jetson에 적합하다. `quiz_easy.py` 문제 4가 Jetson용 모델을 고르게 한다.

---

### 6. Metric Depth vs Relative Depth

깊이맵에는 두 종류가 있다. 이 구분이 매우 중요하다.

```
Relative Depth (상대 깊이):
  값 범위: 0.0 - 1.0 (정규화)
  의미: 이 픽셀이 다른 픽셀보다 가까운가/먼가?
  예: 사람(0.8) vs 건물(0.3) → 사람이 더 가까움
  → MiDaS, DPT, Depth Anything 기본 출력

  장점: 카메라 파라미터 불필요, 어떤 이미지에서든 동작
  단점: "3미터 앞에 장애물" 같은 절대 거리 판단 불가


Metric Depth (절대 깊이):
  값 범위: 0.0 - 80.0 m (실제 거리, 미터 단위)
  의미: 이 픽셀의 실제 거리는 몇 미터?
  예: 사람(2.5m), 건물(15.3m)
  → Metric3D, ZoeDepth, Depth Anything v2 (metric)

  장점: 실제 거리 판단 가능, SLAM/장애물 회피에 직접 사용
  단점: 카메라 캘리브레이션 필요, 학습 데이터 스케일에 의존
```

**Relative**는 "A가 B보다 가깝다"는 순서만 안다. **Metric**은 "A는 정확히 2.5m"라는 실제 거리를 안다. 어느 것이 필요한지는 용도에 달렸다.

```
상대 깊이로 충분한 경우:
  - 장애물 유무 판단 (있다/없다)
  - 대략적 방향 결정 (왼쪽이 더 가까움)


절대 깊이가 필요한 경우:
  - 정확한 정지 거리 판단 (2m 앞에서 정지)
  - 경로 계획 (A 경로 5m vs B 경로 8m)
  - SLAM 스케일 복구
```

`quiz_easy.py` 문제 3이 시나리오별로 어느 깊이가 필요한지 고르게 한다.

---

### 7. Depth Anything v2 (2024)

```
v1 대비 개선점:


1. Synthetic Data 활용
   실제 데이터 + 합성 데이터 혼합 → 더 깨끗한 Ground Truth


2. 더 나은 Teacher 모델
   DINOv2 기반 Teacher → 더 정확한 의사 라벨


3. Metric Depth 지원
   상대 깊이뿐 아니라 절대 깊이 모델도 제공
   → 실내/야외 metric 모델 별도 제공


4. 성능 향상 (v1 → v2):
   Depth Any-S: 0.756 → 0.821
   Depth Any-B: 0.804 → 0.868
   Depth Any-L: 0.836 → 0.891
```

---

## 꼭 이해해야 할 핵심 개념

### Vision Transformer (ViT) 기초

ViT는 이미지를 작은 **패치**로 잘라, 각 패치를 단어처럼 취급해 Transformer에 넣는다.

```
이미지 → 패치 분할:
+--+--+--+--+
|P1|P2|P3|P4| 이미지를 14x14 패치로 분할
+--+--+--+--+ → 각 패치를 벡터로 변환 (Embedding)
|P5|P6|P7|P8| → Transformer에 입력
+--+--+--+--+


Self-Attention:
  각 패치가 다른 모든 패치와의 관계를 학습
  → 하늘 패치와 바닥 패치의 관계 → 깊이 단서!
```

**Self-Attention**은 "어느 패치가 어느 패치와 관련 있는지"를 학습하는 메커니즘이다. 깊이 추정에 좋은 이유: 하늘은 항상 멀고, 바닥의 텍스처 변화는 기울기 단서가 되고, 사물 크기 비교는 상대 거리 단서가 된다. 이런 전역 관계를 Self-Attention이 잘 포착한다.

### Scale-Invariant Loss 이해

MiDaS(§3)가 서로 다른 데이터셋을 섞을 수 있게 한 핵심이다. 문제부터 보자.

```
문제: 데이터셋마다 깊이 스케일이 다름
  KITTI: depth = 0 - 80m
  NYU: depth = 0 - 10m


일반 L1 Loss = |pred - gt| → 스케일에 민감!
  KITTI 오차 5m → L1 = 5.0
  NYU 오차 0.5m → L1 = 0.5
  → KITTI 오차가 항상 크므로 NYU 학습이 무시됨!
```

일반 손실 함수를 쓰면 야외(큰 숫자) 데이터의 오차가 항상 커 보여, 모델이 야외 데이터에만 신경 쓰고 실내 데이터를 무시한다. **Scale-Invariant Loss**는 아래 수식으로 이를 푼다.

```
d_i = log(pred_i) - log(gt_i)              (로그 공간에서의 차이)
L = (1/n) * sum(d_i^2) - (lambda/n^2) * (sum(d_i))^2
```

수식의 의미를 풀면:

1. **로그 공간에서 계산**한다 - `log`를 씌우면 80m와 10m의 절대 차이가 줄어, 스케일 차이의 영향이 완화된다.
2. **두 번째 항이 전체 평균 차이(shift)를 빼준다** - 장면 전체가 일정하게 멀거나 가까운 것(이동)은 무시한다.
3. 결과적으로 모델은 절대값이 아니라 **"상대적인 깊이 순서와 비율"**만 학습한다.

비유하면, 시험 점수(절대값)가 아니라 등수(상대 순서)만 맞추는 것이다. 실제 거리가 80m든 10m든, "A가 B보다 2배 멀다"는 관계만 맞추면 된다. 그래서 스케일이 다른 데이터를 자연스럽게 섞을 수 있다. `quiz_medium.py` 문제 2가 이 손실을 묻는다.

### 깊이 추정과 SLAM의 관계

단안 깊이 추정과 단안 SLAM은 둘 다 "카메라 1대로 3D를 이해"하려는 시도이고, 같은 약점(스케일 모호성)을 공유한다. 둘을 어떻게 결합하는지 정리한다.

먼저 **Monocular SLAM의 근본 문제는 Scale Ambiguity**(스케일 모호성)다. 단일 카메라 영상만으로 3D를 복원하면 실제 단위(미터)를 알 수 없다. 카메라가 실제로 5m를 움직여도 SLAM은 "1 단위 이동"으로만 표현하므로, 복원된 지도의 단위가 미터가 아닌 임의 단위가 된다. (이 한계 자체는 아래 자체 점검 Q1에서도 다룬다.)

여기에 **단안 깊이 추정의 상대 깊이를 결합**하면 SLAM을 다음과 같이 보강할 수 있다.

- **Depth Prior**(깊이 사전값)로 활용: 깊이맵을 Bundle Adjustment(번들 조정, 카메라 자세와 3D 점을 동시에 최적화하는 과정)의 초기값으로 넣으면 수렴이 빨라진다.
- **Scale 보정**: 사람 키(약 170cm)처럼 크기를 아는 물체를 참조점으로 삼아 상대 깊이를 절대 깊이로 변환한다 (`d_metric = scale * d_relative + shift`, 변환 식은 Week 8에서 다룬다).
- **Dense 초기화**: 특징점 기반 SLAM이 만드는 희소(sparse) 깊이를, 모든 픽셀에 값이 있는 밀집(dense) 깊이맵으로 보완한다.

다만 **깊이 추정의 오차는 SLAM에 누적된다**는 점을 알아야 한다.

- 잘못된 깊이는 3D 포인트 위치 오류로 이어져 지도가 왜곡된다.
- 스케일 오차가 쌓이면 장기 주행 시 **스케일 드리프트**(scale drift)가 발생한다.
- 잘못된 매칭은 **루프 클로징**(loop closing, 같은 장소를 다시 방문했음을 인식해 누적 오차를 보정하는 단계) 실패로 이어진다.
- 텍스처가 없는 영역(흰 벽, 하늘)에서 깊이 오차가 특히 크다.

그래서 **실제 시스템에서는** 깊이 추정을 단독 정답으로 믿지 않고 보조 정보로만 쓴다. RGB 카메라에 IMU(관성 센서)나 RGB-D/LiDAR를 더하는 **센서 융합**(Sensor Fusion), 깊이를 GT 대신 초기 추정값(Prior)으로만 사용하기, 추정 신뢰도가 낮은 영역은 가중치를 낮추는 **불확실성 기반** 처리, 기지 물체 기반 스케일 보정의 주기적 수행 등을 조합한다. `quiz_medium.py` 문제 3이 이 관계를 단계별로 묻는다.

---

## 자체 점검 - 이해했는지 확인!

### Q1: Monocular Depth의 한계
**Q:** 단안 깊이 추정의 근본적인 한계는 무엇이고, 어떻게 보완하나요?

**A:**
```
근본 한계: Scale Ambiguity (스케일 모호성)
- 단일 이미지에서는 절대 거리를 알 수 없음
- 작은 물체가 가까이 vs 큰 물체가 멀리를 구분 불가


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


ViT-B, ViT-L은 Jetson에서 실시간 어려움 → 서버/데스크톱용
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
  또는 Scale 보정 (known reference 활용)
```

---

## 이번 주 실습 & 다음 주 준비

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

## 이번 주 핵심 요약

1. **왜 Monocular Depth인가?**
   - 카메라 1대로 깊이 정보 추출, 추가 센서 비용 $0
   - AMR 장애물 거리 판단에 필수

2. **학습 방법론**
   - Supervised: GT 깊이 필요, 정확
   - Self-supervised: GT 불필요, 제한적
   - Zero-shot: 대규모 학습, 강건한 일반화, Fine-tuning 불필요

3. **핵심 모델 발전사**
   - MiDaS (2019): 다중 데이터셋 혼합 학습 (Scale-Invariant Loss)
   - DPT (2021): ViT를 깊이 추정에 적용
   - Depth Anything (2024): 62M 비라벨 데이터 + Teacher-Student

4. **Relative vs Metric Depth**
   - Relative: 순서만, 어디서든 동작
   - Metric: 실제 거리, 카메라 의존

5. **Jetson 배포 전략**
   - Depth Anything ViT-S 선택
   - TensorRT FP16으로 15-20 FPS 달성, YOLO와 동시 실행 가능

---

이전: [Week 6 - Jetson 배포: TensorRT (C++)](../week6/README.md)
다음: [Week 8 - Depth Anything 사용](../week8/README.md)
