# Week 7: nuScenes 데이터셋 - 360도 자율주행 데이터 이해

> **이번 주 목표**: nuScenes 데이터셋의 특징과 구조를 이해하고, nuScenes Devkit을 사용하여 데이터를 탐색하고 시각화한다.
> **예상 시간**: 12-15시간
> **핵심 질문**: "KITTI와 다른 nuScenes의 360도 Multi-camera 데이터를 어떻게 다루는가?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | nuScenes 핵심 특징, 데이터 구조, NDS 지표 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | 좌표계 변환, Quaternion 회전, Devkit 활용 코드 실습 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | nuScenes 데이터 탐색, 6대 카메라 시각화, BEV 어노테이션 |

---

## 시작하기 전에

### Week 5-6에서 배운 것

**KITTI 데이터셋 요약:**
```
KITTI:
  - 카메라: 1대 (전방 단안)
  - 시야각: ~90도 (전방만)
  - 장면 수: 7481개 학습 이미지
  - 클래스: 3개 (Car, Pedestrian, Cyclist)
  - 좌표계: Camera 좌표계 기준
  - 평가 지표: AP3D (Easy/Moderate/Hard)
```

**하지만 현실 자율주행은 다릅니다:**
```
현실 자율주행 환경:
  - 전방만 보면 안 됨! (사각지대 존재)
  - 차선 변경 시 옆/뒤를 봐야 함
  - 교차로에서는 360도 확인 필요
  - 다양한 객체 인식 필요 (공사 장벽, 교통 원뿔 등)

→ KITTI의 한계를 극복하는 데이터셋: nuScenes!
```

> **포트폴리오 관점**: nuScenes는 현재 자율주행 연구의 표준 벤치마크입니다. KITTI만 아는 것과 nuScenes까지 아는 것은 면접에서 큰 차이를 만듭니다. "nuScenes 경험이 있습니다"는 실무 역량의 증거가 됩니다.

---

## 핵심 개념 자세히 알아보기

### 1. nuScenes 데이터셋 소개

#### 1.1 nuScenes란?

nuScenes는 Motional(구 nuTonomy)에서 공개한 **대규모 자율주행 데이터셋**입니다.

```
nuScenes 핵심 특징:
┌─────────────────────────────────────────────────┐
│  360도 커버리지: 6대 카메라로 전 방향 촬영       │
│  1000개 장면:   각 20초, 총 5.5시간 주행 데이터  │
│  23개 클래스:   차량, 보행자, 장벽 등 다양한 객체  │
│  센서 융합:     카메라 6 + LiDAR 1 + RADAR 5     │
│  3D 어노테이션: 140만 개 3D bounding box          │
│  Key Frame:     2Hz (초당 2프레임) 어노테이션      │
│  위치:          보스턴 + 싱가포르 도심 주행         │
└─────────────────────────────────────────────────┘
```

#### 1.2 KITTI vs nuScenes 비교

```
┌──────────────────┬───────────────┬──────────────────┐
│ 항목              │ KITTI         │ nuScenes          │
├──────────────────┼───────────────┼──────────────────┤
│ 카메라 수        │ 1대 (전방)    │ 6대 (360도)       │
│ 시야각           │ ~90도         │ 360도             │
│ 장면 수          │ ~7K 이미지    │ 1000 scene (28K) │
│ 클래스 수        │ 3개           │ 23개              │
│ LiDAR           │ 1대 (64채널)  │ 1대 (32채널)      │
│ RADAR           │ 없음          │ 5대               │
│ 어노테이션       │ 2D + 3D      │ 3D + 속도         │
│ 도심 환경        │ 교외 도로     │ 복잡한 도심       │
│ 날씨 다양성      │ 맑은 날       │ 비, 야간 포함     │
│ 평가 지표        │ AP3D          │ mAP + NDS        │
│ 벤치마크 활용    │ 전통적 표준    │ 현재 표준         │
└──────────────────┴───────────────┴──────────────────┘
```

#### 1.3 6대 카메라 배치

```
nuScenes 카메라 배치 (위에서 본 시점):

             FRONT_LEFT    FRONT    FRONT_RIGHT
                  ╲         │         ╱
                   ╲        │        ╱
                    ╲       │       ╱
              ───────┌─────────────┐───────
                     │             │
     BACK_LEFT ──────│   자동차    │────── BACK_RIGHT (없음)
                     │    (Ego)    │
              ───────└─────────────┘───────
                           │
                           │
                         BACK

카메라별 FOV:
  FRONT:       70도 (전방 중앙)
  FRONT_LEFT:  70도 (전방 좌측)
  FRONT_RIGHT: 70도 (전방 우측)
  BACK:        110도 (후방 중앙, 넓은 화각)
  BACK_LEFT:   70도 (후방 좌측)
  BACK_RIGHT:  70도 (후방 우측)

→ 6대 합쳐서 360도 전 방향 커버
→ 카메라 간 약간의 겹침(overlap) 존재
```

---

### 2. nuScenes 23개 클래스

```
nuScenes Detection 클래스 (23개 중 주요 10개):

┌──────────────────────────────────────────────────┐
│  차량 관련:                                       │
│    car            - 승용차                        │
│    truck          - 트럭                          │
│    bus            - 버스                          │
│    trailer        - 트레일러                       │
│    construction   - 공사 차량                      │
│                                                   │
│  사람 관련:                                       │
│    pedestrian     - 보행자                        │
│    motorcycle     - 오토바이                       │
│    bicycle        - 자전거                        │
│                                                   │
│  장애물:                                          │
│    barrier        - 장벽, 가드레일                  │
│    traffic_cone   - 교통 원뿔                      │
│                                                   │
│  (전체 23개 중 벤치마크는 10개 클래스로 평가)       │
└──────────────────────────────────────────────────┘
```

---

### 3. nuScenes 데이터 구조

#### 3.1 데이터 다운로드

```bash
# nuScenes 데이터 다운로드
# 방법 1: Mini 데이터셋 (10GB) - 먼저 이것부터!
# https://www.nuscenes.org/download 에서 다운로드

# 방법 2: Full 데이터셋 (~300GB)
# 주의: 매우 큰 용량이므로 Mini로 먼저 실습 후 필요시 다운로드

# 압축 해제 후 구조:
# v1.0-mini/
# ├── maps/
# ├── samples/         ← Key Frame 이미지/LiDAR
# ├── sweeps/          ← Key Frame 사이의 추가 데이터
# └── v1.0-mini/       ← 메타데이터 JSON 파일들
```

#### 3.2 디렉토리 구조

```
v1.0-mini/
├── maps/                          # 지도 데이터
│   ├── basemap/
│   └── expansion/
│
├── samples/                       # Key Frame 데이터 (2Hz)
│   ├── CAM_FRONT/                # 전방 카메라 이미지
│   ├── CAM_FRONT_LEFT/           # 전방 좌측 카메라
│   ├── CAM_FRONT_RIGHT/          # 전방 우측 카메라
│   ├── CAM_BACK/                 # 후방 카메라
│   ├── CAM_BACK_LEFT/            # 후방 좌측 카메라
│   ├── CAM_BACK_RIGHT/           # 후방 우측 카메라
│   ├── LIDAR_TOP/                # LiDAR 포인트 클라우드
│   ├── RADAR_FRONT/              # 전방 RADAR
│   ├── RADAR_FRONT_LEFT/
│   ├── RADAR_FRONT_RIGHT/
│   ├── RADAR_BACK_LEFT/
│   └── RADAR_BACK_RIGHT/
│
├── sweeps/                        # 중간 프레임 (비어노테이션)
│   ├── CAM_FRONT/
│   └── ...
│
└── v1.0-mini/                     # 메타데이터 (JSON)
    ├── scene.json                 # 장면 정보
    ├── sample.json               # Key Frame 정보
    ├── sample_data.json          # 센서 데이터 참조
    ├── sample_annotation.json    # 3D 어노테이션
    ├── ego_pose.json             # 차량 위치/자세
    ├── calibrated_sensor.json    # 센서 캘리브레이션
    ├── sensor.json               # 센서 정보
    ├── instance.json             # 객체 인스턴스 (추적용)
    ├── category.json             # 객체 카테고리
    ├── attribute.json            # 객체 속성 (움직임/정지)
    ├── visibility.json           # 가시성 수준
    ├── log.json                  # 주행 로그
    └── map.json                  # 지도 정보
```

#### 3.3 관계형 데이터 구조

```
nuScenes의 데이터는 관계형 DB처럼 연결되어 있습니다:

scene → sample → sample_data → 파일 경로
                 sample_annotation → 3D bbox
                 ego_pose → 차량 위치
                 calibrated_sensor → 센서 캘리브레이션

관계도:
  scene (장면)
    ├── first_sample_token (첫 Key Frame)
    └── last_sample_token
          │
  sample (Key Frame)
    ├── data → sample_data (센서별 데이터)
    ├── anns → sample_annotation (3D bbox 목록)
    ├── prev → 이전 sample
    └── next → 다음 sample
          │
  sample_annotation (3D bbox)
    ├── translation [x, y, z]
    ├── size [w, l, h]
    ├── rotation [quaternion]
    ├── category_name
    ├── visibility_token
    └── instance_token → 같은 객체 추적용
```

---

### 4. nuScenes Devkit 사용법

#### 4.1 Devkit 설치

```bash
pip install nuscenes-devkit
```

#### 4.2 기본 사용법

```python
from nuscenes.nuscenes import NuScenes

# nuScenes 로드 (Mini 데이터셋)
nusc = NuScenes(version='v1.0-mini', dataroot='./data/nuscenes', verbose=True)

# 기본 정보 확인
print(f"장면 수: {len(nusc.scene)}")
print(f"샘플 수: {len(nusc.sample)}")
print(f"어노테이션 수: {len(nusc.sample_annotation)}")
```

#### 4.3 장면(Scene) 탐색

```python
# 첫 번째 장면 정보
scene = nusc.scene[0]
print(f"장면 이름: {scene['name']}")
print(f"설명: {scene['description']}")
print(f"프레임 수: {scene['nbr_samples']}")

# 장면의 첫 Key Frame
first_sample_token = scene['first_sample_token']
sample = nusc.get('sample', first_sample_token)

# Key Frame에 연결된 센서 데이터
print(f"\n연결된 센서 데이터:")
for sensor_name, data_token in sample['data'].items():
    sd = nusc.get('sample_data', data_token)
    print(f"  {sensor_name}: {sd['filename']}")
```

#### 4.4 어노테이션 확인

```python
# Key Frame의 3D 어노테이션
sample = nusc.sample[0]
for ann_token in sample['anns']:
    ann = nusc.get('sample_annotation', ann_token)
    print(f"카테고리: {ann['category_name']}")
    print(f"  위치: {ann['translation']}")
    print(f"  크기: {ann['size']} (w, l, h)")
    print(f"  회전: {ann['rotation']} (quaternion)")
    print(f"  가시성: {ann['visibility_token']}")
    print()
```

---

### 5. nuScenes 좌표계

```
nuScenes 좌표계 (Global, 즉 World 좌표계):

  ↑ Y (북쪽)
  │
  │
  └──────→ X (동쪽)
  Z: 위쪽 (+)

카메라 좌표계:
  X: 오른쪽
  Y: 아래쪽
  Z: 전방

LiDAR 좌표계:
  X: 오른쪽
  Y: 전방
  Z: 위쪽

주의: KITTI와 좌표계가 다름!
  KITTI Camera: x(오른), y(아래), z(전방)
  nuScenes Global: x(동), y(북), z(위)

→ 좌표 변환 시 반드시 확인해야 할 사항!
```

---

### 6. nuScenes 평가 지표

#### 6.1 mAP (mean Average Precision)

```
nuScenes mAP:
  - KITTI와 다른 점: BEV 중심 거리 기반 매칭
  - IoU 대신 중심 거리(Center Distance) 사용

매칭 기준:
  - 2D 중심 거리 (BEV 평면에서)
  - threshold: 0.5m, 1.0m, 2.0m, 4.0m
  - 4개 threshold의 평균 = mAP

예시:
  GT 위치: (10.0, 5.0) BEV
  예측 위치: (10.3, 5.2) BEV
  중심 거리: sqrt(0.3^2 + 0.2^2) = 0.36m
  → 0.5m threshold에서 TP!
```

#### 6.2 NDS (nuScenes Detection Score)

```
NDS = 종합 성능 지표

NDS = (1/10) * [5 * mAP + Σ(mTP_metric)]

5개 True Positive 메트릭:
  1. ATE (Average Translation Error): 위치 오차 (m)
  2. ASE (Average Scale Error): 크기 오차 (1 - IoU)
  3. AOE (Average Orientation Error): 방향 오차 (rad)
  4. AVE (Average Velocity Error): 속도 오차 (m/s)
  5. AAE (Average Attribute Error): 속성 오차 (1 - acc)

NDS = (1/10) * [5*mAP + (1-mATE) + (1-mASE) + (1-mAOE)
                      + (1-mAVE) + (1-mAAE)]

→ mAP만으로는 부족! 위치/크기/방향/속도까지 종합 평가
→ 이것이 nuScenes의 강점
```

#### 6.3 KITTI AP3D vs nuScenes NDS 비교

```
┌─────────────────────────────────────────────┐
│           │ KITTI AP3D     │ nuScenes NDS   │
├─────────────────────────────────────────────┤
│ 매칭 기준 │ 3D IoU         │ 중심 거리      │
│ 난이도    │ Easy/Mod/Hard  │ 없음 (통합)    │
│ 속도 평가 │ 없음           │ AVE 포함       │
│ 속성 평가 │ 없음           │ AAE 포함       │
│ 360도    │ 전방만         │ 360도 전체     │
│ 종합성   │ 검출만 평가    │ 검출+품질 평가  │
└─────────────────────────────────────────────┘
```

---

## 꼭 이해해야 할 핵심 개념

### 1. Key Frame vs Sweep

```
Key Frame (2Hz):
  - 모든 센서 데이터 + 3D 어노테이션
  - 1초에 2번 (0.5초 간격)
  - sample 테이블에 저장

Sweep (12Hz for camera, 20Hz for LiDAR):
  - 센서 데이터만 (어노테이션 없음)
  - Key Frame 사이의 추가 데이터
  - sample_data 테이블에서 is_key_frame=False

활용:
  - 학습: Key Frame의 어노테이션 사용
  - Temporal 모델: Sweep도 입력으로 활용
  - 속도 추정: 연속 프레임 필요
```

### 2. Instance Token의 의미

```
nuScenes는 같은 객체를 프레임 간 추적합니다:

Frame 1: Car_A (instance_token: 'abc123')
Frame 2: Car_A (instance_token: 'abc123')  ← 같은 토큰!
Frame 3: Car_A (instance_token: 'abc123')

→ 3D Object Tracking 연구에 활용
→ 속도 계산: 연속 프레임의 위치 차이로 산출
```

### 3. Quaternion 회전 표현

```
nuScenes는 회전을 Quaternion [w, x, y, z]로 표현합니다:
  KITTI: rotation_y (yaw 각도, 스칼라)
  nuScenes: [w, x, y, z] (쿼터니언, 4차원)

변환:
  yaw = 2 * arctan2(z, w)  (간소화, z-up 기준)

Quaternion의 장점:
  - Gimbal Lock 없음
  - 보간(interpolation)이 자연스러움
  - 3D 회전을 완전하게 표현
```

---

## 자체 점검 - 이해했는지 확인!

**Q1. nuScenes가 KITTI보다 자율주행 연구에 적합한 이유를 3가지 이상 설명하세요.**
> (1) 360도 커버리지: 6대 카메라로 전 방향을 촬영하여 사각지대가 없다. (2) 다양한 환경: 보스턴과 싱가포르의 복잡한 도심, 비/야간 등 다양한 조건을 포함한다. (3) 풍부한 어노테이션: 23개 클래스, 속도, 속성(움직임/정지), 가시성 등 세부 정보가 포함된다. (4) 종합 평가 지표: NDS로 위치/크기/방향/속도를 모두 평가한다. (5) 시간 연속성: Instance Token으로 객체 추적이 가능하다.

**Q2. nuScenes에서 Key Frame과 Sweep의 차이는 무엇인가?**
> Key Frame은 2Hz로 모든 센서 데이터와 3D 어노테이션이 포함된 주요 프레임이다. Sweep는 Key Frame 사이에 더 높은 빈도(카메라 12Hz, LiDAR 20Hz)로 수집된 센서 데이터로, 어노테이션이 포함되지 않는다. 학습에는 주로 Key Frame을 사용하고, Temporal 정보가 필요한 모델에서 Sweep을 추가로 활용한다.

**Q3. nuScenes의 NDS가 KITTI의 AP3D보다 종합적인 평가 지표인 이유는?**
> NDS는 mAP(검출 정확도) 외에도 ATE(위치 오차), ASE(크기 오차), AOE(방향 오차), AVE(속도 오차), AAE(속성 오차)의 5가지 추가 메트릭을 종합하여 평가한다. KITTI의 AP3D는 3D IoU 기반 검출 정확도만 평가하므로, 속도 추정이나 객체 속성(정지/움직임) 같은 자율주행에 중요한 정보는 반영하지 않는다.

**Q4. Mini 데이터셋부터 시작하는 것이 권장되는 이유는?**
> Full nuScenes는 약 300GB로 용량이 매우 크고, 다운로드와 처리에 시간이 오래 걸린다. Mini 데이터셋은 10GB로 10개 장면(404개 Key Frame)만 포함하지만, 데이터 구조와 API 사용법은 동일하다. 코드 개발과 디버깅을 Mini로 먼저 수행한 후, 검증된 코드를 Full 데이터셋에 적용하는 것이 효율적이다.

---

## 이번 주 실습 & 다음 주 준비

### 이번 주 실습 과제

1. **nuScenes Mini 다운로드**: 회원가입 후 Mini 데이터셋 다운로드 및 압축 해제
2. **nuScenes Devkit 설치**: pip install nuscenes-devkit
3. **데이터 구조 탐색**: scene, sample, sample_annotation 관계 확인
4. **6대 카메라 이미지 시각화**: 하나의 Key Frame에서 6대 카메라 이미지 동시 표시
5. **3D 어노테이션 시각화**: BEV에서 객체 위치 시각화
6. **KITTI vs nuScenes 비교**: 좌표계, 클래스, 평가 지표 차이 정리

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

### 다음 주 준비

- BEV(Bird's Eye View) 개념 복습
- IPM(Inverse Perspective Mapping) 기본 원리 이해
- Transformer Attention 메커니즘 복습 (Phase 3에서 학습)

---

## 이번 주 핵심 요약

1. **nuScenes**는 6대 카메라로 360도를 커버하는 대규모 자율주행 데이터셋으로, 1000개 장면과 23개 클래스를 포함한다.
2. **데이터 구조**는 관계형(scene - sample - sample_data - sample_annotation)이며, JSON 메타데이터로 관리된다.
3. **Mini 데이터셋**(10GB)으로 먼저 실습한 후 Full 데이터셋(300GB)으로 확장하는 것이 권장된다.
4. **nuScenes Devkit**을 사용하면 데이터 로드, 시각화, 평가를 편리하게 수행할 수 있다.
5. **NDS**(nuScenes Detection Score)는 mAP에 위치/크기/방향/속도/속성 오차를 종합한 평가 지표로, 자율주행에 더 적합한 종합 성능 척도이다.

---

이전: [Week 6 - 성능 분석 및 개선](../week6/README.md)

다음: [Week 8 - BEV 개념 이해](../week8/README.md)
