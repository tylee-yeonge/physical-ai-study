# Week 2: 좌표계 이해 - Camera/World/LiDAR/BEV 좌표 변환

> **이번 주 목표**: 3D Detection에 필수적인 좌표계 개념을 이해하고, 좌표 변환 및 3D bbox 투영을 구현
> **예상 시간**: 12-15시간
> **핵심 질문**: "Camera, World, LiDAR, BEV 좌표계의 차이는 무엇이며, 이들 사이의 변환은 어떻게 하는가?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | Camera 좌표계, KITTI 레이블 순서 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | 3D→2D 투영 계산, BEV 변환 코드 실습 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | 좌표 변환 및 3D bbox 투영 실습 |

---

## 시작하기 전에

### Week 1 복습

```
Week 1에서 배운 것:
  - 2D Detection의 한계 (거리, Occlusion, 경로 계획)
  - 3D BBox: [x, y, z, l, w, h, theta] 7개 파라미터
  - 3D Detection 방법론: LiDAR / Camera / Fusion
  - Depth Ambiguity 개념
```

### 이번 주가 중요한 이유

3D Detection에서 **좌표계**를 이해하지 못하면:
- 레이블 데이터를 잘못 해석합니다
- 3D bbox를 이미지에 투영할 수 없습니다
- 모델 출력을 올바르게 변환할 수 없습니다

```
좌표계는 3D Perception의 '언어'입니다:
┌────────────────────────────────────────┐
│  데이터셋 레이블 → 어떤 좌표계 기준?     │
│  모델 출력       → 어떤 좌표계로 나옴?   │
│  시각화          → 어떤 좌표계로 변환?   │
│  평가            → 어떤 좌표계에서 비교?  │
└────────────────────────────────────────┘
→ 전부 좌표계를 알아야 합니다!
```

---

## 핵심 개념 자세히 알아보기

### 1. 주요 좌표계 4가지

#### 1.1 Camera 좌표계 (Camera Coordinate System)

```
카메라를 원점으로 하는 좌표계:

        Y (아래)
        │
        │
        │────→ X (오른쪽)
       /
      /
     Z (전방, 카메라가 바라보는 방향)

특징:
  - 원점: 카메라 광학 중심
  - X: 오른쪽 (+)
  - Y: 아래쪽 (+)    ← 주의! 일반적인 위쪽이 아님
  - Z: 전방 (+)      ← 카메라가 바라보는 방향
  - 단위: 미터 (m)
  - KITTI의 기본 좌표계
```

#### 1.2 World 좌표계 (World Coordinate System)

```
고정된 기준점(보통 지도의 원점)에 대한 좌표계:

     Z (위)
     │
     │
     │────→ Y (왼쪽 또는 오른쪽, 규약에 따라 다름)
    /
   /
  X (전방)

특징:
  - 원점: 지도의 기준점
  - 위쪽이 Z+ (중력 반대 방향)
  - 자율주행에서는 IMU/GPS 기준
  - 여러 프레임의 데이터를 통합할 때 사용
```

#### 1.3 LiDAR 좌표계 (Velodyne Coordinate System)

```
LiDAR 센서를 원점으로 하는 좌표계:

     Z (위)
     │
     │
     │────→ Y (왼쪽)
    /
   /
  X (전방)

특징:
  - 원점: LiDAR 센서 중심
  - X: 전방 (+)
  - Y: 왼쪽 (+)
  - Z: 위쪽 (+)
  - Camera 좌표계와 축 방향이 다름!
```

#### 1.4 BEV 좌표계 (Bird's Eye View)

```
위에서 내려다본 시점의 2D 좌표계:

  ←───── X (왼쪽, 또는 오른쪽)
  │
  │
  ↓
  Z (전방)

특징:
  - 높이(Y) 정보를 무시한 2D 표현
  - X-Z 평면 (또는 X-Y 평면, 규약에 따라)
  - 자율주행에서 경로 계획에 직접 사용
  - BEVFormer 등 최신 모델의 출력 형태
```

### 2. 좌표계 비교 및 변환

```
4가지 좌표계 비교:
┌──────────┬──────────┬──────────┬──────────┐
│          │ 전방     │ 오른쪽   │ 위쪽     │
├──────────┼──────────┼──────────┼──────────┤
│ Camera   │ +Z       │ +X       │ -Y       │
│ LiDAR    │ +X       │ -Y       │ +Z       │
│ World    │ +X       │ -(규약)  │ +Z       │
│ BEV      │ +Z(2D)   │ +X(2D)   │ (없음)   │
└──────────┴──────────┴──────────┴──────────┘
```

#### Camera <-> LiDAR 변환

```
LiDAR → Camera 변환:
  Pc = R_rect @ Tr_velo_to_cam @ Pl

  Tr_velo_to_cam: LiDAR → Camera 외부 파라미터 (4x4)
  R_rect: 스테레오 정류 회전 (3x3 → 4x4로 확장)
  Pl: LiDAR 좌표 [x, y, z, 1]
  Pc: Camera 좌표 [x, y, z, 1]

Camera → 이미지 투영:
  p = P2 @ Pc

  P2: 투영 행렬 (3x4)
  Pc: Camera 좌표 [x, y, z, 1]
  p: 이미지 좌표 [u*z, v*z, z] → (u, v) = (p[0]/p[2], p[1]/p[2])
```

### 3. KITTI 좌표계 규약

KITTI 데이터셋은 고유한 좌표계 규약을 사용합니다.

```
KITTI 3D 레이블 포맷 (label_2):
  Car 0.0 0 -1.56 587.01 173.33 614.12 200.12 1.65 1.67 3.64 -0.65 1.71 46.70 -1.59
  │   │   │  │    └─────── 2D bbox ──────┘ │    │    │    │    │    │     │
  │   │   │  alpha                         h    w    l    x    y    z     ry
  │   │   occluded
  │   truncated
  class

KITTI 3D 정보: [h, w, l, x, y, z, ry]
                │  │  │  └─ 중심 ─┘  │
                └ 크기 ┘            yaw

주의: KITTI는 [h, w, l] 순서 (일반적인 [l, w, h]와 다름!)
```

#### KITTI 좌표계 세부사항

```
KITTI Camera 좌표계:
  - x: 오른쪽 (+)
  - y: 아래쪽 (+)      ← y가 아래로!
  - z: 전방 (+)

  중요: y 좌표의 의미
    - 3D bbox의 y는 객체 '바닥'의 y 좌표
    - 도로면의 y ≈ 1.65m (카메라 높이)
    - 차량 중심의 y ≈ 1.65 - h/2

  ry (rotation_y):
    - y축 기준 회전 (yaw)
    - 0: 전방(+z)을 바라봄
    - pi/2: 오른쪽(+x)을 바라봄
    - -pi/2: 왼쪽(-x)을 바라봄
    - pi: 뒤(−z)를 바라봄
```

### 4. 캘리브레이션 행렬

KITTI 캘리브레이션 파일에는 여러 변환 행렬이 있습니다.

```
calib/000000.txt 내용:
  P0: ...  # 좌측 흑백 카메라 투영 행렬 (3x4)
  P1: ...  # 우측 흑백 카메라 투영 행렬 (3x4)
  P2: ...  # 좌측 컬러 카메라 투영 행렬 (3x4) ← 이것을 주로 사용!
  P3: ...  # 우측 컬러 카메라 투영 행렬 (3x4)
  R0_rect: ...  # 스테레오 정류 행렬 (3x3)
  Tr_velo_to_cam: ...  # LiDAR → Camera 변환 (3x4)
  Tr_imu_to_velo: ...  # IMU → LiDAR 변환 (3x4)
```

```python
# 캘리브레이션 파일 파싱
def read_calib(calib_path):
    """KITTI 캘리브레이션 파일을 읽어 행렬로 반환"""
    calib = {}
    with open(calib_path, 'r') as f:
        for line in f.readlines():
            if ':' not in line:
                continue
            key, value = line.split(':', 1)
            calib[key.strip()] = np.array([float(x) for x in value.split()])

    # P2: 3x4 투영 행렬
    P2 = calib['P2'].reshape(3, 4)

    # R0_rect: 3x3 → 4x4 확장
    R0 = np.eye(4)
    R0[:3, :3] = calib['R0_rect'].reshape(3, 3)

    # Tr_velo_to_cam: 3x4 → 4x4 확장
    Tr = np.eye(4)
    Tr[:3, :4] = calib['Tr_velo_to_cam'].reshape(3, 4)

    return {'P2': P2, 'R0_rect': R0, 'Tr_velo_to_cam': Tr}
```

### 5. 3D BBox Corners 계산 및 이미지 투영

3D bbox를 이미지에 투영하는 전체 파이프라인:

```
파이프라인:
  1. 3D BBox 파라미터 [h,w,l,x,y,z,ry] 파싱
  2. 8개 corners 계산 (Camera 좌표계)
  3. corners → 이미지 좌표 투영 (P2 사용)
  4. 이미지 위에 12개 edge 그리기
```

```python
import numpy as np

def compute_box_3d_kitti(h, w, l, x, y, z, ry):
    """
    KITTI 좌표계에서 3D bbox의 8개 꼭짓점 계산

    KITTI 규약:
      - (x, y, z)는 객체 바닥면 중심
      - y는 아래쪽이 양수
      - ry는 y축 회전 (yaw)

    Returns:
        corners_3d: (8, 3) ndarray
    """
    # 회전 행렬 (y축 회전)
    c = np.cos(ry)
    s = np.sin(ry)
    R = np.array([
        [ c, 0, s],
        [ 0, 1, 0],
        [-s, 0, c]
    ])

    # 8개 꼭짓점 (중심 기준, 회전 전)
    # KITTI: x=width, y=height, z=length
    x_corners = [ l/2,  l/2, -l/2, -l/2,  l/2,  l/2, -l/2, -l/2]
    y_corners = [   0,    0,    0,    0,   -h,   -h,   -h,   -h ]
    z_corners = [ w/2, -w/2, -w/2,  w/2,  w/2, -w/2, -w/2,  w/2]

    corners = np.array([x_corners, y_corners, z_corners])  # (3, 8)

    # 회전 적용
    corners = R @ corners

    # 중심으로 이동 (KITTI: y는 바닥 중심)
    corners[0, :] += x
    corners[1, :] += y
    corners[2, :] += z

    return corners.T  # (8, 3)


def project_to_image(corners_3d, P2):
    """
    3D 좌표를 이미지 좌표로 투영

    Parameters:
        corners_3d: (N, 3) Camera 좌표
        P2: (3, 4) 투영 행렬

    Returns:
        corners_2d: (N, 2) 이미지 좌표
    """
    # 동차 좌표로 변환 (N, 4)
    N = corners_3d.shape[0]
    pts_3d_hom = np.hstack([corners_3d, np.ones((N, 1))])

    # 투영
    pts_2d_hom = (P2 @ pts_3d_hom.T).T  # (N, 3)

    # 정규화
    pts_2d = pts_2d_hom[:, :2] / pts_2d_hom[:, 2:3]

    return pts_2d  # (N, 2)
```

### 6. BEV (Bird's Eye View) 변환

```
BEV 변환 = 3D 좌표에서 높이(y)를 무시하고 x-z 평면으로 투영

Camera 좌표 (x, y, z) → BEV 좌표 (x, z)

용도:
  - 자율주행 경로 계획 (2D 평면에서 동작)
  - 주차장 매핑
  - 물체 간 거리/방향 파악
  - BEVFormer 등 최신 모델의 출력 표현
```

```python
def camera_to_bev(corners_3d, bev_range=(-40, 40, 0, 70), bev_size=(800, 700)):
    """
    Camera 좌표를 BEV 이미지 좌표로 변환

    Parameters:
        corners_3d: (N, 3) Camera 좌표
        bev_range: (x_min, x_max, z_min, z_max) 미터 단위
        bev_size: (width, height) 픽셀 단위

    Returns:
        bev_pts: (N, 2) BEV 이미지 좌표
    """
    x_min, x_max, z_min, z_max = bev_range
    bev_w, bev_h = bev_size

    x = corners_3d[:, 0]
    z = corners_3d[:, 2]

    # 미터 → 픽셀 변환
    bev_x = ((x - x_min) / (x_max - x_min) * bev_w).astype(int)
    bev_z = ((z_max - z) / (z_max - z_min) * bev_h).astype(int)  # z 반전 (전방이 위)

    return np.stack([bev_x, bev_z], axis=1)
```

---

## 꼭 이해해야 할 핵심 개념

### Camera 좌표계 vs LiDAR 좌표계

```
가장 흔한 실수: 축 방향 혼동!

Camera:                LiDAR:
  Y (아래)              Z (위)
  │                     │
  │──→ X (오른쪽)       │──→ Y (왼쪽)
 /                     /
Z (전방)              X (전방)

변환 관계:
  Camera_x =  LiDAR_y (부호 반대는 Tr에 포함)
  Camera_y = -LiDAR_z
  Camera_z =  LiDAR_x
```

### KITTI 레이블에서 y 좌표의 의미

```
KITTI의 y 좌표는 객체 '바닥면 중심':
  - 도로 위 승용차: y ≈ 1.65 (카메라 높이)
  - 객체의 3D 중심: y - h/2

  카메라 (y=0)
  │
  │  (y 증가 = 아래로)
  │
  ├── 객체 윗면: y - h
  │
  ├── 객체 중심: y - h/2
  │
  └── 객체 바닥: y ← KITTI label의 y
  │
  ── 도로면: y ≈ 1.65m
```

### P2 투영 행렬의 구조

```
P2 = [fx  0  cx  tx]    (3x4)
     [ 0 fy  cy  ty]
     [ 0  0   1  tz]

  fx, fy: 초점 거리 (픽셀 단위)
  cx, cy: 주점 (이미지 중심 근처)
  tx, ty, tz: 스테레오 베이스라인 보정값

KITTI 전형적인 값:
  fx ≈ 721
  fy ≈ 721
  cx ≈ 609
  cy ≈ 173
```

---

## 자체 점검 - 이해했는지 확인!

### Q1: 좌표계 축 방향
**Q:** KITTI Camera 좌표계에서 '위쪽'은 어느 축의 어느 방향인가?

**A:**
```
-Y 방향입니다.
KITTI Camera 좌표계에서 Y축은 '아래쪽'이 양수이므로,
위쪽은 -Y 방향입니다.

이것은 이미지 좌표계와 일관성을 가집니다:
  이미지에서도 v 좌표는 아래로 갈수록 증가합니다.
```

### Q2: KITTI 레이블 순서
**Q:** KITTI 레이블에서 3D 크기가 [h, w, l] 순서인 이유는 무엇일까?

**A:**
```
KITTI는 [height, width, length] 순서를 사용합니다.
이것은 KITTI 데이터셋의 규약(convention)입니다.

주의할 점:
  - 많은 논문/코드는 [l, w, h] 순서를 사용
  - 데이터셋마다 순서가 다를 수 있음
  - 변환 시 순서를 반드시 확인해야 함!

nuScenes: [w, l, h] (또 다른 순서!)
→ 데이터셋 규약을 항상 확인하는 습관이 중요합니다.
```

### Q3: 투영의 의미
**Q:** P2 행렬로 3D 점을 투영할 때 z로 나누는 이유는?

**A:**
```
원근 투영(perspective projection)의 원리입니다.

P2 @ [X, Y, Z, 1]^T = [u*Z, v*Z, Z]^T

(u, v) = (u*Z / Z, v*Z / Z)

Z로 나누는 것이 '원근감'을 만듭니다:
  - 가까운 물체(Z 작음) → u, v 변화 큼 → 크게 보임
  - 먼 물체(Z 큼) → u, v 변화 작음 → 작게 보임

이것이 핀홀 카메라 모델의 핵심입니다 (Phase 2 복습).
```

### Q4: BEV의 장점
**Q:** BEV 표현이 자율주행에서 특히 유용한 이유는?

**A:**
```
1. 경로 계획에 직접 사용 가능:
   - 차량은 2D 평면 위에서 이동
   - BEV = 2D 평면 표현 → 경로 계획에 자연스러움

2. 물체 간 관계 파악 용이:
   - 거리, 방향이 실제 물리 공간과 일치
   - Occlusion 없이 모든 객체를 볼 수 있음

3. 다중 센서 융합에 적합:
   - Camera, LiDAR 모두 BEV로 변환 가능
   - BEV 공간에서 feature fusion

4. 이미지 왜곡 없음:
   - 원근 투영에 의한 왜곡이 없음
   - 물체 크기가 거리에 무관하게 일정
```

---

## 이번 주 실습 & 다음 주 준비

### 이번 주 체크리스트

- [ ] Camera / LiDAR / World / BEV 좌표계 축 방향 이해
- [ ] KITTI 좌표계 규약 [h, w, l, x, y, z, ry] 숙지
- [ ] 캘리브레이션 파일 (P2, R0_rect, Tr_velo_to_cam) 파싱
- [ ] 3D bbox corners 계산 (KITTI 규약)
- [ ] 3D → 2D 이미지 투영 (P2 사용)
- [ ] BEV 변환 이해
- [ ] `PRACTICE.md` 실습 완료
- [ ] `quiz_easy.py`, `quiz_medium.py` 풀기

### 다음 주 미리보기: KITTI 데이터셋

```
다음 주에는:
  - KITTI 3D Object Detection 데이터셋 다운로드
  - 데이터 구조 (image_2, calib, label_2) 탐색
  - 레이블 파싱 파이프라인 구현
  - 2D bbox, 3D bbox, BEV 시각화
  → 이번 주의 좌표 변환 지식이 직접 활용됩니다!
```

---

## 이번 주 핵심 요약

1. **좌표계 4종류**: Camera(Z전방, Y아래), LiDAR(X전방, Z위), World(X전방, Z위), BEV(X-Z 평면)

2. **KITTI 규약**: 레이블은 [h, w, l, x, y, z, ry] 순서이며, Camera 좌표계 기준. y는 객체 바닥의 y좌표

3. **캘리브레이션 행렬**: P2(3x4 투영), R0_rect(3x3 정류), Tr_velo_to_cam(3x4 변환)으로 좌표계 간 변환

4. **3D→2D 투영**: corners_3d → P2 @ [x,y,z,1]^T → z로 나누기 → 이미지 좌표 (u, v)

5. **BEV 변환**: 높이(y)를 무시하고 x-z 평면에 투영. 자율주행 경로 계획과 물체 관계 파악에 필수

---

이전: [Week 1 - 3D Detection 개념](../week1/README.md)

다음: [Week 3 - KITTI 데이터셋](../week3/README.md)
