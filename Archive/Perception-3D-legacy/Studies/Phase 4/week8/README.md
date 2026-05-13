# Week 8: BEV 개념 이해 - Bird's Eye View로 세상 보기


> **이번 주 목표**: Bird's Eye View(BEV)의 정의와 필요성을 이해하고, BEV 생성의 3가지 주요 방법론(IPM, MLP, Transformer)을 비교 분석한다.
> **예상 시간**: 12-15시간
> **핵심 질문**: "카메라 이미지를 왜, 어떻게 Bird's Eye View로 변환하는가?"


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | BEV 정의, IPM 원리, Lift-Splat-Shoot 개념 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | BEV 그리드 설계, IPM Homography 변환 코드 실습 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | IPM 구현, BEV 시각화, 3가지 방법론 비교 정리 |


---


## 시작하기 전에


### Week 7에서 배운 것


**nuScenes 데이터셋:**
```
nuScenes:
  - 6대 카메라 → 360도 커버리지
  - 23개 클래스, 1000개 장면
  - NDS 평가 지표 (mAP + 5개 TP 메트릭)


하지만 근본적인 질문:
  6대 카메라의 이미지를 어떻게 "하나의 통합된 표현"으로 만들까?


  CAM_FRONT → +
  CAM_FRONT_LEFT → |
  CAM_FRONT_RIGHT → |→ ??? → 통합 표현 → 3D Detection
  CAM_BACK → |
  CAM_BACK_LEFT → |
  CAM_BACK_RIGHT → +
```


**답: Bird's Eye View (BEV)!**


```
6개 카메라 이미지
      ↓
  BEV 변환
      ↓
+-------------+
| BEV Feature | ← 위에서 본 2D 맵
| (X-Y 평면) |
| |
| 자동차 |
| * | ← Ego Vehicle
| |
+-------------+
      ↓
  3D Detection
```


> **포트폴리오 관점**: BEV 기반 인식은 자율주행의 최신 트렌드입니다. "BEV가 무엇이고 왜 필요한지 설명할 수 있습니까?"는 면접 필수 질문입니다. BEVFormer, BEVDet 등의 이름을 들었을 때 원리를 설명할 수 있어야 합니다.


---


## 핵심 개념 자세히 알아보기


### 1. Bird's Eye View(BEV)란?


#### 1.1 정의


```
Bird's Eye View = "새가 위에서 내려다본 시점"


기술적 정의:
  3D 공간을 위에서 X-Y 평면으로 투영한 2D 표현


+---------------------------------------------+
| Camera View (전방 시점): |
| |
| +-----------------+ |
| | [car] (멀리) | ← 원근법 적용 |
| | [car] (가까이) | |
| | ---도로--- | |
| +-----------------+ |
| |
| BEV (위에서 본 시점): |
| |
| +-----------------+ |
| | [car] | ← 실제 크기 비율 |
| | | |
| | [car] | ← 거리 정보 보존 |
| | * (Ego) | |
| +-----------------+ |
+---------------------------------------------+
```


#### 1.2 BEV의 좌표계


```
BEV 좌표계:


  ↑ Y (전방)
  |
  |
  +------→ X (오른쪽)


BEV 그리드:
  - 크기: H x W (예: 200 x 200)
  - 셀 크기: 0.5m x 0.5m
  - 커버 범위: 100m x 100m
  - 중심: Ego Vehicle 위치


BEV에서의 객체 표현:
  - 위치: (x, y) 좌표
  - 크기: length x width (높이는 무시)
  - 방향: yaw 각도
  - 추가: 속도 (vx, vy)
```


---


### 2. 왜 BEV인가? - BEV의 3가지 장점


#### 2.1 장점 1: Occlusion 해결


```
Camera View에서의 Occlusion:


  카메라 ----→
         +---+
         |Car| ← 이 차가 뒤의 차를 가림!
         | A |
         +---+
              +---+
              |Car| ← 가려져서 안 보임
              | B |
              +---+


BEV에서:
  +-----------------+
  | |
  | [Car A] | ← 둘 다 보임!
  | |
  | [Car B] | ← 가려짐 없음!
  | |
  | * (Ego) |
  +-----------------+


이유:
  - Camera View는 깊이 방향으로 겹침
  - BEV는 위에서 보므로 X-Y 평면에서 겹침이 적음
  - 서로 다른 깊이의 객체가 분리됨
```


#### 2.2 장점 2: Multi-camera 융합


```
6대 카메라 → 각각의 Feature Map


  +------------+ +------------+ +------------+
  | FRONT_LEFT | | FRONT | |FRONT_RIGHT |
  +------------+ +------------+ +------------+
  +------------+ +------------+ +------------+
  | BACK_LEFT | | BACK | | BACK_RIGHT |
  +------------+ +------------+ +------------+


문제: 6개의 서로 다른 Feature를 어떻게 합칠까?
  - 단순 concatenate? → 시점이 다르므로 의미 없음
  - 각각 독립 처리? → 겹침 영역 일관성 없음


해결: BEV 공간에서 융합!
  +------------------+
  | BEV Feature Map |
  | (200 x 200) |
  | |
  | 6개 카메라의 |
  | 정보가 통합됨 |
  | |
  +------------------+


각 카메라의 Feature가 BEV의 해당 위치에 투영되어 자연스럽게 융합!
```


#### 2.3 장점 3: 경로 계획과의 직접 연결


```
자율주행 파이프라인:


기존 (Camera View 기반):
  Image → 2D Detection → 3D 변환 → 경로 계획
  (복잡한 좌표 변환 필요, 정보 손실)


BEV 기반:
  Image → BEV Feature → 3D Detection + 경로 계획
  (BEV에서 직접 경로 계획 가능!)


BEV에서 직접 할 수 있는 것들:
  +--------------------------+
  | BEV Feature |
  | +-- 3D Object Detection |
  | +-- Lane Detection |
  | +-- Map Segmentation |
  | +-- Motion Prediction |
  | +-- Path Planning |
  +--------------------------+


→ BEV는 "통합 인식 표현"의 역할을 함!
→ 자율주행의 모든 모듈이 BEV를 공유
```


---


### 3. BEV 생성 방법 1: IPM (Inverse Perspective Mapping)


#### 3.1 IPM 원리


```
IPM = 역 원근 변환


원리:
  "지면이 평평하다고 가정하고, 이미지를 위에서 본 시점으로 변환"


수학적 과정:
  1. 이미지 픽셀 (u, v) 선택
  2. 지면 평면 (y = 0) 가정
  3. 역투영: (u, v) → 카메라 광선 → 지면 교점 → (X, Z)
  4. BEV 좌표로 매핑: (X, Z) → BEV (row, col)


수식:
  [u]
  [0] = K^(-1) * [v] * d (d: 지면까지 깊이)
  [Z] [1]


  BEV(x, z) = Image(u, v) (homogrraphy 변환)
```


#### 3.2 IPM의 Homography 변환


```
Homography 행렬 H:


  [u_bev] [u_img]
  [v_bev] = H * [v_img]
  [ 1 ] [ 1 ]


H = K_bev * [r1 r2 t] * K_img^(-1)


여기서:
  K_bev: BEV 해상도 설정
  [r1 r2 t]: 외부 파라미터 (회전 + 이동)
  K_img: 카메라 내부 파라미터


IPM 시각화:
  원본 이미지: IPM 결과 (BEV):
  +--------------+ +--------------+
  | 하늘 | | |
  | --- 수평선 | --→ | +----+ |
  | / \ | | | 차 | |
  | / 도로 \ | | +----+ |
  |/ \ | | 도로 |
  +--------------+ | |
                         | * Ego |
                         +--------------+
```


#### 3.3 IPM의 한계


```
IPM의 핵심 가정: 지면이 평평하다!


이 가정이 깨지는 경우:


1. 경사로/언덕:
   +-----------------+
   | / |
   | / ← 경사 |
   | / |
   +-----------------+
   → BEV 왜곡 발생!


2. 3D 객체 (차량, 건물):
   차량은 지면이 아니므로 IPM이 정확하지 않음
   → 차량 윗부분이 늘어나거나 찌그러짐


3. 높이 정보 손실:
   지면(y=0) 가정이므로 높이 정보 완전 손실
   → 교통 표지판, 신호등 등 위치 부정확


IPM의 위치:
  +----------------------------------+
  | IPM: 간단하지만 가정이 강함 |
  | +-- 장점: 학습 불필요, 빠름 |
  | +-- 단점: 평면 가정 필요 |
  | +-- 사용처: 차선 검출, 주차장 |
  +----------------------------------+
```


---


### 4. BEV 생성 방법 2: MLP 기반 (Lift-Splat-Shoot)


#### 4.1 Lift-Splat-Shoot 원리


```
논문: "Lift, Splat, Shoot: Encoding Images from Arbitrary
       Camera Rigs by Implicitly Unprojecting to 3D"
저자: Jonah Philion, Sanja Fidler (ECCV 2020)


3단계 파이프라인:


Step 1: Lift (들어올리기)
  - 각 픽셀에 대해 Depth 분포를 예측
  - 2D Feature → 3D Feature Volume으로 확장


  이미지 픽셀 (u, v)
       ↓
  Depth 분포 예측: [d1, d2, ..., dD] (D개 depth bin)
       ↓
  3D 공간에 Feature를 "뿌림" (scatter)


Step 2: Splat (펼치기)
  - 3D Feature Volume을 BEV 그리드에 투영
  - Pillar 방식: Z축을 따라 합산 (sum pooling)


  3D Feature Volume
       ↓
  Z축 합산 (sum along height)
       ↓
  BEV Feature Map (H x W x C)


Step 3: Shoot (쏘기)
  - BEV Feature에서 Task 수행
  - 3D Detection, Segmentation 등
```


#### 4.2 Depth 분포 예측의 직관


```
각 픽셀에 대해 "이 픽셀이 어느 깊이에 있을 확률"을 예측:


  픽셀 (u, v)의 Depth 분포:


  확률
  |
  | *
  | / \
  | / \
  | / \
  |--/-------\--→ Depth
  | 5m 10m 15m


  → "이 픽셀은 10m 깊이에 있을 확률이 가장 높다"


이 정보를 이용해 Feature를 3D 공간에 배치:
  d=5m: 0.1 * feature
  d=10m: 0.7 * feature ← 여기에 가장 많은 정보
  d=15m: 0.2 * feature
```


#### 4.3 Lift-Splat-Shoot의 장단점


```
장점:
  1. 평면 가정 불필요 (IPM 한계 극복)
  2. 학습 기반으로 다양한 장면에 적응
  3. Multi-camera 자연스러운 융합
  4. 높이 정보도 활용 가능


단점:
  1. Depth 예측이 부정확하면 BEV도 부정확
  2. 3D Feature Volume이 메모리 많이 사용
  3. Depth 예측을 위한 추가 감독(supervision) 필요
  4. 연산량이 많음


성능:
  BEVDet (Lift-Splat-Shoot 기반):
    nuScenes NDS: ~0.39
    nuScenes mAP: ~0.30
```


---


### 5. BEV 생성 방법 3: Transformer 기반 (BEVFormer)


#### 5.1 BEVFormer 핵심 아이디어


```
BEVFormer의 접근:
  "BEV Query가 카메라 Feature에 질문을 던져 BEV를 생성"


핵심 차이점:
  IPM: 기하학적 변환 (학습 X)
  Lift-Splat: Depth 예측 → 3D scatter → BEV
  BEVFormer: Query + Attention → BEV (Depth 명시적 예측 불필요!)


파이프라인:
  6대 카메라 이미지
       ↓
  Backbone + FPN → Image Features
       ↓
  BEV Queries (200x200 learnable)
       ↓
  Spatial Cross-Attention
  (BEV Query → Image Feature 참조)
       ↓
  Temporal Self-Attention
  (이전 프레임 BEV 활용)
       ↓
  BEV Feature Map (200x200x256)
       ↓
  Detection Head → 3D BBox
```


#### 5.2 Spatial Cross-Attention 직관


```
BEV Query가 "어디를 봐야 할지" 기하학으로 결정:


BEV 위치 (i, j) → 3D 좌표 (x, y, z_ref)
                         ↓
                 각 카메라에 투영
                         ↓
              투영된 위치의 Feature 수집
                         ↓
              Deformable Attention으로 정보 추출


예시:
  BEV 위치 (100, 150) = 자동차 전방 25m, 오른쪽 0m
       ↓
  3D 좌표: (0, 0, 25) (여러 높이로)
       ↓
  CAM_FRONT에 투영 → (u=620, v=300) → Feature 추출!
  CAM_FRONT_LEFT에는 투영되지 않음 → 무시


장점:
  - Depth를 명시적으로 예측하지 않아도 됨
  - 카메라 캘리브레이션을 통한 정확한 투영
  - Deformable Attention으로 효율적 연산
```


#### 5.3 Temporal Self-Attention


```
시간 정보 활용의 핵심:


Frame t-1의 BEV Feature
       ↓
  Ego-motion으로 좌표 정렬
       ↓
  현재 BEV Query와 합침
       ↓
  Self-Attention으로 유용한 정보 선택


효과:
  1. 가려진 객체 복구 (이전 프레임에서 보였던 객체)
  2. 속도 추정 (연속 프레임의 위치 변화)
  3. 검출 안정성 향상 (시간적 일관성)


→ Temporal이 NDS +2~3% 향상에 기여!
```


---


### 6. 3가지 방법론 비교


```
+--------------+--------------+--------------+--------------+
| | IPM | Lift-Splat | BEVFormer |
+--------------+--------------+--------------+--------------+
| 핵심 원리 | 기하 변환 | Depth 예측 | Query+Attn |
| 학습 필요 | X | O | O |
| 평면 가정 | 필요 | 불필요 | 불필요 |
| Depth 예측 | 불필요 | 필요 (명시적) | 불필요 (암묵적)|
| Temporal | X | 확장 가능 | 내장 |
| 연산량 | 매우 적음 | 중간 | 높음 |
| 정확도 | 낮음 | 중간 | 높음 |
| 대표 모델 | - | BEVDet | BEVFormer |
| NDS (nuSc) | - | ~0.39 | ~0.52 |
+--------------+--------------+--------------+--------------+
| 사용처 | 차선/주차장 | 일반적 사용 | SOTA 모델 |
+--------------+--------------+--------------+--------------+


발전 방향:
  IPM (2015~) → Lift-Splat (2020) → BEVFormer (2022) → ???
  (기하 → 학습 기반 → Transformer 기반)
```


---


## 꼭 이해해야 할 핵심 개념


### 1. BEV의 본질


```
BEV = 3D 세계를 위에서 본 2D 표현


핵심 가치:
  - 다중 카메라 융합의 자연스러운 공간
  - Occlusion 해결
  - 경로 계획과 직접 연결


BEV는 "표현 방식"이지 "모델"이 아님!
  → BEV를 만드는 방법이 여러 가지
  → IPM, MLP, Transformer 등
```


### 2. Depth 추정의 역할


```
Camera → BEV 변환의 핵심 과제: "각 픽셀의 깊이를 어떻게 아는가?"


IPM: 지면 가정으로 깊이 결정 (가정)
Lift-Splat: 네트워크가 깊이를 명시적으로 예측
BEVFormer: Attention이 깊이를 암묵적으로 학습


→ 깊이를 어떻게 다루느냐가 방법론의 핵심 차이!
```


### 3. BEV Resolution의 중요성


```
BEV 해상도 = 셀 크기 결정


높은 해상도 (셀 0.25m):
  + 세밀한 표현
  - 많은 메모리/연산
  - Query 수 증가 (400x400 = 160,000)


낮은 해상도 (셀 1.0m):
  + 적은 연산
  - 작은 객체 표현 어려움
  - 위치 정확도 제한


균형점: 0.5m (200x200 = 40,000)
  → 대부분의 BEV 모델이 채택
  → 100m x 100m 범위 커버
```


---


## 자체 점검 - 이해했는지 확인!


**Q1. BEV가 Camera View보다 Occlusion에 강한 이유는?**
> Camera View에서는 가까운 객체가 먼 객체를 깊이(z) 방향으로 가리지만, BEV에서는 위에서 내려다보므로 X-Y 평면에서 객체가 겹치지 않는 한 모두 보인다. 서로 다른 깊이에 있는 객체가 BEV에서는 다른 위치에 배치되므로 가려짐이 자연스럽게 해결된다.


**Q2. IPM이 3D 객체(차량)에 대해 부정확한 이유는?**
> IPM은 "지면이 평평하다(y=0)"는 가정에 기반한다. 차량은 지면 위에 높이가 있는 3D 객체이므로, 차량의 상단 부분은 실제 지면 위에 있어 IPM의 투영이 정확하지 않다. 차량 윗부분이 BEV에서 실제보다 멀리 있는 것처럼 늘어나거나 왜곡이 발생한다.


**Q3. Lift-Splat-Shoot에서 Depth 예측이 중요한 이유는?**
> Lift 단계에서 각 픽셀의 Feature를 3D 공간에 배치하려면 해당 픽셀의 깊이를 알아야 한다. Depth 예측이 부정확하면 Feature가 잘못된 3D 위치에 배치되고, Splat 후 BEV Feature도 부정확해진다. 즉 Depth 예측의 품질이 전체 BEV 표현의 품질을 직접적으로 결정한다.


**Q4. BEVFormer가 Depth를 명시적으로 예측하지 않아도 되는 이유는?**
> BEVFormer는 BEV Query의 3D 위치를 카메라에 투영하여 Reference Point를 생성한다. 이 투영은 카메라 캘리브레이션(K, [R|t])을 사용한 기하학적 연산이므로, 네트워크가 Depth를 직접 예측할 필요가 없다. 대신 Deformable Attention이 Reference Point 주변에서 가장 유용한 Feature를 학습적으로 선택하므로, 깊이 정보가 암묵적으로 Attention weight에 인코딩된다.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제


1. **IPM 구현**: 간단한 Homography 기반 BEV 변환 직접 구현
2. **BEV 시각화**: nuScenes 데이터에서 BEV 표현 시각화
3. **Lift-Splat 개념 이해**: Depth 분포 예측과 Scatter 과정 코드로 이해
4. **BEVFormer 논문 읽기**: ECCV 2022 논문의 Figure 2, 3 분석
5. **3가지 방법론 비교 정리**: IPM vs Lift-Splat vs BEVFormer 표 작성
6. **BEV Resolution 실험**: 셀 크기에 따른 표현력 차이 시각화


자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고


### 다음 주 준비


- BEVFormer 논문 다운로드: https://arxiv.org/abs/2203.17270
- Transformer Attention 메커니즘 복습 (특히 Cross-Attention)
- Deformable DETR 개념 간단히 이해
- MMDetection3D 환경 확인 (Week 5에서 세팅)


---


## 이번 주 핵심 요약


1. **BEV(Bird's Eye View)**는 3D 세상을 위에서 X-Y 평면으로 투영한 표현으로, Occlusion 해결, Multi-camera 융합, 경로 계획 직접 연결이라는 3가지 핵심 장점을 가진다.
2. **IPM**은 지면 평면 가정 하에 기하학적 변환으로 BEV를 생성하며, 간단하지만 3D 객체에 대해 부정확하다.
3. **Lift-Splat-Shoot(MLP 기반)**은 Depth 분포를 명시적으로 예측하여 Feature를 3D에 배치하고 BEV로 합산하며, 평면 가정이 불필요하지만 Depth 정확도에 의존한다.
4. **BEVFormer(Transformer 기반)**는 BEV Query와 Spatial Cross-Attention으로 Image Feature에서 직접 BEV를 생성하며, Depth를 암묵적으로 처리하고 Temporal 정보도 자연스럽게 활용한다.
5. **BEV Resolution**(셀 크기 0.5m, 200x200)은 정확도와 연산 효율의 균형점이며, 대부분의 SOTA 모델이 이 설정을 채택한다.


---


이전: [Week 7 - nuScenes 데이터셋](../week7/README.md)


다음: [Week 9 - BEVFormer 이해](../week9/README.md)
