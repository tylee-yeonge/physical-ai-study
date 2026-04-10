# Week 1: 3D Detection 개념 - 왜 3D를 검출해야 하는가?

> 🎯 **이번 주 목표**: 3D Object Detection의 필요성과 기본 개념을 이해하고, 2D와 3D Detection의 차이를 명확히 파악
> ⏰ **예상 시간**: 12-15시간
> 💡 **핵심 질문**: "2D Detection만으로 왜 자율주행과 로봇에 부족하고, 3D Detection이 필수인가?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | 3D BBox 파라미터, Detection 방법론 분류 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | 3D BBox Corners 계산, Depth Ambiguity 코드 실습 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | 3D Detection 개념 종합 실습 |

---

## 🌟 시작하기 전에

### Phase 4은 커리어 준비의 핵심입니다

Phase 4는 단순한 학습이 아닙니다. **자율주행, AMR, 로봇 분야 이직을 위한 포트폴리오의 핵심**이 되는 Phase입니다. 3D Perception은 면접에서 가장 자주 묻는 주제 중 하나이며, KITTI/nuScenes 데이터셋 경험은 실무 역량의 증거가 됩니다.

```
💼 포트폴리오 가치:
┌──────────────────────────────────────────────┐
│  Phase 2-4: SLAM 기초 (이론 역량)              │
│  Phase 3:   2D Detection (기본기)              │
│  Phase 4:   3D Perception (★ 핵심 어필 포인트)  │
│              → "카메라만으로 3D 검출 가능합니다"    │
│              → "KITTI AP3D 지표를 이해합니다"     │
│              → "BEV 기반 접근을 설명할 수 있습니다" │
└──────────────────────────────────────────────┘
```

### 2D Detection의 한계를 느껴봅시다

**비유: 사진 한 장으로 물체 잡기**
```
2D Detection으로 본 세상:
┌─────────────────────────────┐
│    [Car: 0.95]              │
│    ┌──────────┐             │
│    │  🚗      │             │
│    └──────────┘             │
│                             │
│         [Car: 0.87]         │
│         ┌────┐              │
│         │ 🚗 │              │
│         └────┘              │
└─────────────────────────────┘

질문:
  - 두 차의 실제 거리는? → ❌ 알 수 없음
  - 두 차의 실제 크기는? → ❌ 알 수 없음 (작은 차가 가까이? 큰 차가 멀리?)
  - 두 차 사이를 지나갈 수 있나? → ❌ 판단 불가
```

```
3D Detection으로 본 세상:
┌─────────────────────────────┐
│    [Car: x=2.1, y=0, z=8.5] │
│    ┌──────────┐  l=4.5m     │
│    │  🚗      │  w=1.8m     │
│    └──────────┘  h=1.5m     │
│                  θ=0.1rad   │
│    [Car: x=-1.2, y=0, z=25] │
│         ┌────┐  l=4.2m     │
│         │ 🚗 │  w=1.7m     │
│         └────┘  θ=-0.05rad │
└─────────────────────────────┘

답변:
  - 첫 번째 차: 전방 8.5m, 오른쪽 2.1m ✅
  - 두 번째 차: 전방 25m, 왼쪽 1.2m ✅
  - 사이 통과 가능 여부: 계산 가능! ✅
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. 2D Detection의 한계

2D Object Detection은 이미지 평면 위에서 객체의 위치를 bounding box `[x, y, w, h]`로 표현합니다. 이 방식은 아래와 같은 근본적인 한계가 있습니다.

#### 한계 1: 거리 정보 부족
```
카메라 이미지에서:
  - 멀리 있는 큰 차 vs 가까이 있는 작은 차 → 같은 크기로 보임
  - "이 물체가 3m 앞에 있는지, 30m 앞에 있는지" 알 수 없음

실제 상황:
  ┌────────────────────────────────────┐
  │                                    │
  │     ┌──────┐                       │
  │     │ 트럭  │  ← 50m 거리, 큰 차    │
  │     └──────┘                       │
  │     ┌──────┐                       │
  │     │ 승용차│  ← 10m 거리, 작은 차   │
  │     └──────┘                       │
  │                                    │
  │  두 2D bbox 크기가 비슷할 수 있음!    │
  └────────────────────────────────────┘
```

#### 한계 2: Occlusion (가림)
```
실제 3D 공간:           카메라에서 보이는 것:
  🚗A  🚗B               🚗A만 보임 (B는 가려짐)
   ↑    ↑
  10m  12m               2D: A만 검출
                         3D: A(10m), B(12m) 둘 다 알 수 있음
```

#### 한계 3: 경로 계획 불가
```
자율주행 시나리오:
  - "앞 차까지 거리가 5m이니 브레이크를 밟아라"
  - "옆 차선의 차가 3m 옆에 있으니 차선 변경하지 마라"
  → 이 모든 것이 3D 정보 없이는 불가능!
```

### 2. 3D Detection이 필요한 이유

| 응용 분야 | 필요한 3D 정보 | 이유 |
|-----------|---------------|------|
| **AMR (자율주행 로봇)** | 장애물 위치, 크기 | 충돌 회피, 경로 계획 |
| **자율주행** | 차량 간 거리, 속도 | 안전 거리 유지, 경로 예측 |
| **로봇 조작 (Grasping)** | 물체 3D 위치, 방향 | 로봇 팔이 물체를 정확히 잡기 |
| **드론** | 장애물 3D 위치 | 3D 공간에서의 회피 기동 |
| **AR/VR** | 현실 물체 3D 배치 | 가상 물체의 자연스러운 배치 |

### 3. 3D Bounding Box

3D Detection의 출력은 **7개 파라미터**로 구성됩니다.

```
2D Bounding Box:
  [x, y, w, h]        ← 4개 파라미터
  (이미지 좌표)

3D Bounding Box:
  [x, y, z, l, w, h, θ]  ← 7개 파라미터
  └─ 중심 ─┘ └ 크기 ┘ └회전┘

  x, y, z : 3D 공간에서의 중심 좌표 (카메라 또는 월드 좌표계)
  l       : length (길이, 전후 방향)
  w       : width (폭, 좌우 방향)
  h       : height (높이, 상하 방향)
  θ       : yaw 각도 (수평면에서의 회전, BEV 관점)
```

**3D BBox의 8개 꼭짓점 (corners):**
```
        5 ─────── 6
       /│        /│
      / │       / │
     4 ─────── 7  │        ↑ y (높이)
     │  1 ─────│─ 2        │
     │ /       │ /          │
     │/        │/           └──→ x (좌우)
     0 ─────── 3           /
                          z (전방)
```

```python
# 3D bbox corners 계산 (기본 개념)
import numpy as np

def compute_box_3d(x, y, z, l, w, h, theta):
    """
    3D bounding box의 8개 꼭짓점 좌표 계산

    Parameters:
        x, y, z: 중심 좌표
        l, w, h: 크기 (length, width, height)
        theta: yaw 회전각 (라디안)

    Returns:
        corners: (8, 3) 꼭짓점 좌표
    """
    # 회전 행렬 (yaw만 고려)
    R = np.array([
        [np.cos(theta), 0, np.sin(theta)],
        [0,             1, 0            ],
        [-np.sin(theta), 0, np.cos(theta)]
    ])

    # 중심 기준 8개 꼭짓점 (회전 전)
    x_corners = [l/2, l/2, -l/2, -l/2, l/2, l/2, -l/2, -l/2]
    y_corners = [0, 0, 0, 0, -h, -h, -h, -h]
    z_corners = [w/2, -w/2, -w/2, w/2, w/2, -w/2, -w/2, w/2]

    corners = np.array([x_corners, y_corners, z_corners])  # (3, 8)

    # 회전 적용
    corners = R @ corners  # (3, 8)

    # 중심으로 이동
    corners[0, :] += x
    corners[1, :] += y
    corners[2, :] += z

    return corners.T  # (8, 3)
```

### 4. 3D Detection 방법론 분류

3D Object Detection은 사용하는 센서에 따라 크게 3가지로 나뉩니다.

```
┌─────────────────────────────────────────────────────┐
│              3D Object Detection 방법론               │
├──────────────┬──────────────┬───────────────────────┤
│  LiDAR 기반   │  Camera 기반  │  Fusion (융합)         │
├──────────────┼──────────────┼───────────────────────┤
│ PointPillars  │ SMOKE        │ BEVFusion             │
│ VoxelNet      │ FCOS3D       │ TransFusion           │
│ CenterPoint   │ BEVFormer    │ PointPainting         │
├──────────────┼──────────────┼───────────────────────┤
│ 장점:         │ 장점:         │ 장점:                  │
│ - 정확한 거리  │ - 저렴한 센서  │ - 두 장점 결합          │
│ - 직접 3D 측정 │ - 텍스처 정보  │ - 최고 성능            │
│              │ - 색상 정보    │                       │
├──────────────┼──────────────┼───────────────────────┤
│ 단점:         │ 단점:         │ 단점:                  │
│ - 비싼 센서    │ - Depth 모호성 │ - 캘리브레이션 복잡      │
│ - 날씨 영향    │ - 어두운 환경  │ - 시스템 복잡도 높음     │
│ - 해상도 제한  │ - 정확도 낮음  │ - 센서 동기화 필요      │
└──────────────┴──────────────┴───────────────────────┘
```

| 방법 | 대표 모델 | 입력 | KITTI AP3D (Car, Mod.) |
|------|----------|------|----------------------|
| LiDAR | PointPillars | Point Cloud | ~77% |
| Camera (Mono) | FCOS3D | 단안 이미지 | ~15-25% |
| Camera (Multi) | BEVFormer | 다시점 이미지 | ~40% (nuScenes) |
| Fusion | BEVFusion | Camera + LiDAR | ~72% (nuScenes NDS) |

### 5. 이 Phase의 초점: Camera 기반 3D Detection

```
Camera 기반 3D Detection의 세부 분류:

1. Monocular (단안)
   - 입력: 단일 카메라 이미지 1장
   - 예: SMOKE, FCOS3D, MonoDLE
   - 장점: 가장 간단, 센서 1개
   - 단점: Depth 추정이 매우 어려움

2. Stereo (스테레오)
   - 입력: 좌우 카메라 이미지 2장
   - 예: DSGN, Pseudo-LiDAR
   - 장점: 시차(disparity)로 depth 계산
   - 단점: 카메라 2개 필요, 베이스라인 제한

3. Multi-view (다시점)
   - 입력: 여러 방향 카메라 이미지 (보통 6장)
   - 예: BEVFormer, DETR3D, PETR
   - 장점: 360도 커버리지, BEV 생성
   - 단점: 카메라 여러 개 필요, 연산량 큼
```

**왜 Camera 기반에 집중하는가?**
```
현실적 이유:
1. LiDAR는 비쌈 ($10,000~$75,000) vs 카메라 ($50~$500)
2. Tesla의 Pure Vision 접근 → 업계 트렌드
3. AMR/로봇에서 카메라가 기본 센서
4. Camera 기반이 면접에서 더 깊은 이해를 보여줌
   → "LiDAR 없이 어떻게 3D를 알 수 있나요?" 질문에 답변 가능!
```

---

## 💡 꼭 이해해야 할 핵심 개념

### Depth Ambiguity (깊이 모호성)

Camera 기반 3D Detection의 가장 큰 도전은 **단일 이미지에서 깊이를 추정**하는 것입니다.

```
핀홀 카메라 모델 복습 (Phase 2):

  u = fx * X/Z + cx
  v = fy * Y/Z + cy

  X, Y, Z: 3D 좌표
  u, v: 이미지 좌표 (픽셀)
  fx, fy: 초점 거리
  cx, cy: 주점

문제: (u, v) → (X, Y, Z) 변환 시
  X = (u - cx) * Z / fx
  Y = (v - cy) * Z / fy

  → Z (깊이)를 모르면 X, Y도 알 수 없음!
  → 하나의 픽셀은 무한한 3D 점에 대응
```

```
깊이 모호성 시각화:

카메라 ────→ Z (깊이)
  │
  │    ● 작은 물체 (가까이)
  │         ● 중간 물체 (중간)
  │              ● 큰 물체 (멀리)
  │
  ↓ 세 물체 모두 이미지에서 같은 크기로 보임!
```

**해결 방법들:**
1. **학습 기반 Depth 추정**: 네트워크가 단서(원근법, 그림자, 크기)로 깊이 학습
2. **기하학적 제약**: 차량 크기의 사전 지식 활용 (승용차 ~4.5m)
3. **Keypoint 기반**: 2D-3D 대응점으로 PnP 문제 풀기
4. **Multi-view**: 여러 시점에서 삼각측량

### 3D IoU (Intersection over Union)

3D Detection의 평가 지표로 **3D IoU**가 사용됩니다.

```
2D IoU: 두 사각형의 겹치는 면적 / 전체 면적
3D IoU: 두 직육면체의 겹치는 부피 / 전체 부피

3D IoU 계산:
  IoU_3D = Volume(A ∩ B) / Volume(A ∪ B)

  Volume(A ∪ B) = Volume(A) + Volume(B) - Volume(A ∩ B)

KITTI 평가 기준:
  - Car:        IoU ≥ 0.7 → True Positive
  - Pedestrian: IoU ≥ 0.5 → True Positive
  - Cyclist:    IoU ≥ 0.5 → True Positive
```

### AP3D (Average Precision 3D)

```
AP3D 계산 과정:
1. 모든 예측에 대해 confidence 순으로 정렬
2. 각 예측에 대해 GT와 3D IoU 계산
3. IoU ≥ threshold이면 TP, 아니면 FP
4. Precision-Recall 곡선 계산
5. 곡선 아래 면적 = AP3D

KITTI 난이도:
  Easy:     가려짐 없음, 충분히 큼, 잘 보임
  Moderate: 일부 가려짐, 중간 크기
  Hard:     많이 가려짐, 작은 크기

모노큘러 3D Detection 성능 (참고):
  Car (Moderate): ~15-25% AP3D
  → LiDAR 기반 (~77%)에 비해 훨씬 어려움!
```

---

## 🔍 자체 점검 - 이해했는지 확인!

### Q1: 2D vs 3D Detection 차이
**Q:** 2D Detection의 출력은 `[x, y, w, h]` 4개인데, 3D Detection은 왜 7개 파라미터가 필요한가요?

**A:**
```
2D: [x, y, w, h] → 이미지 평면 위의 위치와 크기
  - 2D 좌표로 위치 표현 (x, y)
  - 2D 크기로 범위 표현 (w, h)

3D: [x, y, z, l, w, h, θ] → 3D 공간에서의 위치, 크기, 방향
  - 3D 좌표로 위치 표현 (x, y, z) → +1차원 (깊이)
  - 3D 크기로 범위 표현 (l, w, h) → +1차원 (depth 방향)
  - 회전 각도 (θ) → +1 (물체의 방향)

추가된 정보:
  - z (깊이): "물체가 얼마나 멀리 있는가"
  - l (길이): "물체가 깊이 방향으로 얼마나 긴가"
  - θ (회전): "물체가 어느 방향을 향하고 있는가"
```

### Q2: Camera 기반의 장단점
**Q:** Camera 기반 3D Detection이 LiDAR보다 성능이 낮은데도 연구되는 이유는?

**A:**
```
1. 비용: LiDAR ($10K+) vs Camera ($50~500) → 100배 이상 차이
2. 정보 풍부: 색상, 텍스처, 의미 정보 → 객체 분류에 유리
3. 해상도: 카메라 수백만 픽셀 vs LiDAR 수만~수십만 포인트
4. 산업 트렌드: Tesla Pure Vision → 카메라만으로 자율주행
5. AMR/로봇: 대부분 카메라가 기본 센서
6. 기술 발전: BEVFormer 등으로 격차가 줄어들고 있음
```

### Q3: 3D IoU의 의미
**Q:** KITTI에서 Car의 3D IoU threshold가 0.7인 이유는 무엇일까요?

**A:**
```
IoU 0.7은 "두 3D 박스가 70% 이상 겹쳐야 정답으로 인정"하는 것입니다.

왜 높은 기준인가:
  - 자율주행에서는 정확한 위치 추정이 안전과 직결
  - 위치가 조금만 틀려도 충돌 위험
  - 0.5면 너무 느슨해서 실용적이지 않음

왜 Pedestrian은 0.5인가:
  - 보행자는 크기가 작아서 IoU가 자연스럽게 낮음
  - 같은 오차라도 작은 물체에서 IoU가 더 크게 떨어짐
  - 예: 0.5m 오차 → 차(4.5m)는 IoU 영향 작음, 사람(0.6m)은 IoU 크게 감소
```

### Q4: 포트폴리오 관점
**Q:** 면접에서 "왜 LiDAR 없이 Camera만으로 3D Detection을 했나요?"라고 물으면 어떻게 답하시겠습니까?

**A:**
```
좋은 답변 예시:
"Camera 기반 3D Detection은 더 도전적인 문제이며,
깊이 추정, 좌표 변환, 기하학적 이해를 깊이 있게 요구합니다.

1. 비용 효율성: 실제 AMR/로봇에서는 카메라가 기본 센서
2. 기술 깊이: Depth Ambiguity를 이해하고 해결하는 과정에서
   핀홀 모델, 좌표 변환, 기하학적 제약을 깊이 학습했습니다
3. 최신 트렌드: BEVFormer 등 Camera 기반 방법이
   LiDAR와의 격차를 빠르게 줄이고 있습니다
4. 확장성: Monocular → Multi-view → Fusion으로
   자연스럽게 확장할 수 있는 기반을 마련했습니다"
```

---

## 📝 이번 주 실습 & 다음 주 준비

### 이번 주 체크리스트

- [ ] 2D Detection의 3가지 한계 (거리, Occlusion, 경로계획) 이해
- [ ] 3D BBox 7개 파라미터 `[x, y, z, l, w, h, θ]` 의미 파악
- [ ] 3D BBox의 8개 corners 계산 원리 이해
- [ ] 3D Detection 방법론 분류 (LiDAR, Camera, Fusion) 비교
- [ ] Camera 기반 3D Detection의 세부 분류 (Mono, Stereo, Multi-view) 이해
- [ ] Depth Ambiguity 개념 파악
- [ ] AP3D 평가 지표 이해
- [ ] `PRACTICE.md` 실습 완료
- [ ] `quiz_easy.py`, `quiz_medium.py` 풀기

### 다음 주 미리보기: 좌표계 이해

```
다음 주에는:
  - Camera / World / LiDAR / BEV 좌표계
  - KITTI 좌표계 규약 [h, w, l, x, y, z, ry]
  - 좌표 변환 실습 (Camera ↔ World)
  - 3D bbox corners → 2D 이미지 투영
  → Phase 2에서 배운 좌표 변환을 3D Detection에 적용합니다!
```

---

## 🎯 이번 주 핵심 요약

1. **2D Detection의 한계**: 거리 정보 부족, Occlusion 처리 어려움, 경로 계획 불가 → 3D 공간 이해가 필수

2. **3D Bounding Box = 7개 파라미터**: `[x, y, z, l, w, h, θ]` (중심 좌표 + 크기 + yaw 회전)

3. **3D Detection 방법론**: LiDAR 기반 (정확), Camera 기반 (저렴), Fusion (최고 성능) → 이 Phase는 Camera 기반에 집중

4. **Camera 기반의 핵심 도전**: Depth Ambiguity → 단일 이미지에서 깊이를 어떻게 추정할 것인가가 핵심 문제

5. **포트폴리오 가치**: 3D Perception 경험은 자율주행/로봇 면접의 핵심 어필 포인트 → GitHub + 블로그 + 데모로 증명

---

이전: [Phase 3 - Detection + Depth](../../../Roadmap/Phase%205.md)

다음: [Week 2 - 좌표계 이해](../week2/README.md)
