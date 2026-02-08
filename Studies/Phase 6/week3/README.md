# Week 3: KITTI 데이터셋 - 구조 이해, 레이블 파싱, 시각화

> **이번 주 목표**: KITTI 3D Object Detection 데이터셋의 구조를 이해하고, 레이블 파싱 및 2D/3D/BEV 시각화를 구현
> **예상 시간**: 12-15시간
> **핵심 질문**: "KITTI 데이터셋을 직접 로드하고, 레이블을 파싱하여, 3D bbox를 이미지와 BEV에 시각화할 수 있는가?"

---

## 시작하기 전에

### Week 2 복습

```
Week 2에서 배운 것:
  - Camera/LiDAR/World/BEV 좌표계 차이
  - KITTI 규약: [h, w, l, x, y, z, ry]
  - 캘리브레이션: P2, R0_rect, Tr_velo_to_cam
  - 3D -> 2D 투영: P2 @ [x,y,z,1]^T / z
  - BEV 변환: X-Z 평면 투영
```

### 이번 주 실습의 의미

```
지금까지 '가상 데이터'로 연습했다면,
이번 주부터 '실제 데이터셋'을 다룹니다!

Week 1-2: 개념 학습 (가상 데이터)
Week 3:   실제 KITTI 데이터셋 ← 이번 주!
Week 4:   모델 학습 및 추론

KITTI는 3D Detection의 '표준 벤치마크'입니다.
이 데이터를 직접 다루는 경험은 면접에서 큰 강점이 됩니다.
```

---

## 핵심 개념 자세히 알아보기

### 1. KITTI 3D Object Detection 데이터셋

#### 1.1 데이터셋 개요

```
KITTI (Karlsruhe Institute of Technology and Toyota Technological Institute):
  - 2012년 공개, 3D Detection의 사실상 표준 벤치마크
  - 독일 카를스루에 시내에서 수집
  - 7,481장 학습, 7,518장 테스트 이미지
  - 센서: 스테레오 카메라 + Velodyne LiDAR + GPS/IMU

핵심 통계:
  - 해상도: ~1242 x 375 픽셀
  - 클래스: Car, Pedestrian, Cyclist (+ Van, Truck, Person_sitting, Tram, Misc, DontCare)
  - 레이블: 2D bbox + 3D bbox + 캘리브레이션
```

#### 1.2 다운로드

```
KITTI 3D Object Detection 다운로드:
  URL: http://www.cvlibs.net/datasets/kitti/eval_object.php?obj_benchmark=3d

  필요한 파일:
  1. Left color images (12 GB)     ← 필수
  2. Camera calibration (16 MB)    ← 필수
  3. Training labels (5 MB)        ← 필수
  4. Velodyne point clouds (29 GB) ← 선택 (LiDAR 사용 시)

  미니 데이터셋 (연습용):
  → 전체를 안 받아도 10장 정도만 수동 다운로드하여 연습 가능
```

### 2. KITTI 디렉토리 구조

```
KITTI/
├── training/              # 학습 데이터 (7,481개)
│   ├── image_2/           # 좌측 컬러 카메라 이미지
│   │   ├── 000000.png
│   │   ├── 000001.png
│   │   └── ...
│   ├── image_3/           # 우측 컬러 카메라 이미지 (스테레오)
│   ├── calib/             # 캘리브레이션 파일
│   │   ├── 000000.txt
│   │   └── ...
│   ├── label_2/           # 레이블 파일
│   │   ├── 000000.txt
│   │   └── ...
│   └── velodyne/          # LiDAR 포인트 클라우드 (선택)
│       ├── 000000.bin
│       └── ...
└── testing/               # 테스트 데이터 (7,518개)
    ├── image_2/
    ├── calib/
    └── velodyne/

image_2: 좌측 컬러 카메라 (P2 투영 행렬에 대응)
image_3: 우측 컬러 카메라 (P3 투영 행렬에 대응)
calib:   각 프레임별 캘리브레이션 (P0~P3, R0_rect, Tr_velo_to_cam)
label_2: 좌측 카메라 기준 레이블 (Camera 좌표계)
```

### 3. KITTI 레이블 포맷 상세

각 레이블 파일(label_2/XXXXXX.txt)은 한 줄에 하나의 객체를 기술합니다.

```
포맷 (15개 필드):
  class truncated occluded alpha x1 y1 x2 y2 h w l x y z ry

예시:
  Car 0.00 0 -1.56 587.01 173.33 614.12 200.12 1.65 1.67 3.64 -0.65 1.71 46.70 -1.59

필드별 설명:
┌──────────┬─────────────────────────────────────────────┐
│ 필드      │ 설명                                        │
├──────────┼─────────────────────────────────────────────┤
│ class    │ 객체 클래스 (Car, Pedestrian, Cyclist, ...)  │
│ truncated│ 잘림 정도 (0.0~1.0, 0=안 잘림)              │
│ occluded │ 가려짐 정도 (0~3, 0=안 가려짐)              │
│ alpha    │ 관측 각도 (-pi ~ pi)                         │
│ x1,y1    │ 2D bbox 좌상단 (pixels)                     │
│ x2,y2    │ 2D bbox 우하단 (pixels)                     │
│ h        │ 높이 (m) ← Camera 좌표계                    │
│ w        │ 폭 (m)                                      │
│ l        │ 길이 (m)                                    │
│ x        │ 중심 x좌표 (m) ← Camera 좌표계              │
│ y        │ 바닥 y좌표 (m) ← Camera 좌표계              │
│ z        │ 중심 z좌표 (m) ← Camera 좌표계 (깊이)       │
│ ry       │ y축 회전각 (라디안) ← Camera 좌표계          │
└──────────┴─────────────────────────────────────────────┘
```

#### alpha vs ry 차이

```
alpha (관측 각도):
  - 카메라에서 바라본 객체의 방향
  - 카메라 원점 → 객체 중심 벡터와 객체 방향의 상대 각도
  - 이미지 위치에 따라 변함

ry (rotation_y):
  - 객체의 전역 방향 (Camera 좌표계)
  - 카메라 시점에 관계없이 일정
  - 3D bbox 계산에 사용하는 것은 ry!

관계:
  alpha = ry - arctan2(x, z)
```

### 4. 캘리브레이션 파일 상세

```python
# KITTI 캘리브레이션 파일 파싱 함수
import numpy as np

def read_kitti_calib(calib_path):
    """
    KITTI 캘리브레이션 파일을 읽어 딕셔너리로 반환

    Parameters:
        calib_path: 캘리브레이션 파일 경로 (예: calib/000000.txt)

    Returns:
        calib: dict with P2(3x4), R0_rect(4x4), Tr_velo_to_cam(4x4)
    """
    data = {}
    with open(calib_path, 'r') as f:
        for line in f.readlines():
            if ':' not in line:
                continue
            key, value = line.split(':', 1)
            data[key.strip()] = np.array([float(x) for x in value.split()])

    calib = {}

    # P2: 3x4 투영 행렬 (좌측 컬러 카메라)
    calib['P2'] = data['P2'].reshape(3, 4)

    # R0_rect: 3x3 -> 4x4 확장
    R0 = np.eye(4)
    R0[:3, :3] = data['R0_rect'].reshape(3, 3)
    calib['R0_rect'] = R0

    # Tr_velo_to_cam: 3x4 -> 4x4 확장
    Tr = np.eye(4)
    Tr[:3, :4] = data['Tr_velo_to_cam'].reshape(3, 4)
    calib['Tr_velo_to_cam'] = Tr

    return calib
```

### 5. 레이블 파싱

```python
def read_kitti_label(label_path):
    """
    KITTI 레이블 파일을 읽어 객체 리스트로 반환

    Parameters:
        label_path: 레이블 파일 경로 (예: label_2/000000.txt)

    Returns:
        objects: list of dict
    """
    objects = []
    with open(label_path, 'r') as f:
        for line in f.readlines():
            parts = line.strip().split()
            if len(parts) < 15:
                continue

            obj = {
                'class': parts[0],
                'truncated': float(parts[1]),
                'occluded': int(parts[2]),
                'alpha': float(parts[3]),
                'bbox_2d': [float(parts[4]), float(parts[5]),
                           float(parts[6]), float(parts[7])],
                'h': float(parts[8]),
                'w': float(parts[9]),
                'l': float(parts[10]),
                'x': float(parts[11]),
                'y': float(parts[12]),
                'z': float(parts[13]),
                'ry': float(parts[14]),
            }
            objects.append(obj)

    return objects
```

### 6. 시각화 파이프라인

3D Detection 시각화는 크게 3가지입니다.

#### 6.1 2D BBox 시각화

```
가장 기본적인 시각화:
  - label에서 x1, y1, x2, y2를 읽어 이미지에 사각형 그리기
  - 클래스와 함께 표시
```

#### 6.2 3D BBox 이미지 투영

```
파이프라인:
  1. label에서 [h, w, l, x, y, z, ry] 읽기
  2. compute_box_3d()로 8개 corners 계산
  3. project_to_image()로 2D 투영
  4. 12개 edge를 이미지에 그리기

  전면(front face)을 다른 색으로 표시하면
  객체의 방향을 한눈에 파악할 수 있습니다.
```

#### 6.3 BEV 시각화

```
파이프라인:
  1. label에서 [x, z, l, w, ry] 읽기
  2. 4개 바닥면 corners 계산
  3. X-Z 평면에 폴리곤으로 그리기
  4. 방향 화살표 추가

BEV는 자율주행에서 가장 직관적인 시각화입니다.
```

---

## 꼭 이해해야 할 핵심 개념

### DontCare 레이블

```
KITTI에는 'DontCare' 클래스가 있습니다:
  - 평가 시 무시할 영역을 표시
  - 너무 먼 객체, 극심하게 가려진 객체 등
  - 이 영역의 검출은 TP도 FP도 아닌 무시(Ignored)

레이블 파싱 시:
  objects = [obj for obj in objects if obj['class'] != 'DontCare']
```

### 난이도 기준 (Easy / Moderate / Hard)

```
KITTI 3D Detection 평가 난이도:

┌──────────┬──────────────┬──────────────┬──────────────┐
│          │ Easy         │ Moderate     │ Hard         │
├──────────┼──────────────┼──────────────┼──────────────┤
│ 최소 높이 │ 40 pixels    │ 25 pixels    │ 25 pixels    │
│ 최대 가림 │ 0 (안 가림)  │ 1 (부분)     │ 2 (대부분)   │
│ 최대 잘림 │ 0.15         │ 0.30         │ 0.50         │
└──────────┴──────────────┴──────────────┴──────────────┘

Moderate가 표준 벤치마크:
  - 대부분의 논문이 Moderate AP3D를 보고
  - Easy는 너무 쉬움, Hard는 너무 어려움
```

### Train/Val Split

```
KITTI 공식 학습 데이터 (7,481장)는 통상적으로:
  - Train: ~3,712장 (전반부)
  - Val:   ~3,769장 (후반부)

Chen et al.의 분할이 표준으로 사용됩니다.
많은 논문/코드가 이 분할을 따릅니다.
```

---

## 자체 점검 - 이해했는지 확인!

### Q1: 레이블 필드 이해
**Q:** KITTI 레이블에서 `Car 0.50 1 -1.20 400 180 550 280 1.5 1.8 4.0 3.0 1.65 20.0 0.10`
에서 각 값의 의미를 설명하시오.

**A:**
```
Car: 클래스 (승용차)
0.50: truncated (50% 잘림)
1: occluded (부분적 가려짐)
-1.20: alpha (관측 각도)
400 180 550 280: 2D bbox (x1,y1,x2,y2) - 이미지 좌표 픽셀
1.5: height (높이 1.5m)
1.8: width (폭 1.8m)
4.0: length (길이 4.0m)
3.0: x (Camera 좌표계 오른쪽 3m)
1.65: y (Camera 좌표계 아래 1.65m - 도로면)
20.0: z (Camera 좌표계 전방 20m)
0.10: ry (yaw 회전 0.1 라디안 - 거의 전방)
```

### Q2: image_2 vs image_3
**Q:** image_2와 image_3의 차이는 무엇이며, label_2는 어느 카메라 기준인가?

**A:**
```
image_2: 좌측 컬러 카메라 (P2 투영 행렬에 대응)
image_3: 우측 컬러 카메라 (P3 투영 행렬에 대응)

label_2는 좌측 카메라(image_2) 기준입니다.
따라서 3D bbox를 이미지에 투영할 때 P2를 사용해야 합니다.

번호 규약:
  0: 좌측 흑백
  1: 우측 흑백
  2: 좌측 컬러 ← 주로 사용!
  3: 우측 컬러
```

### Q3: truncated vs occluded
**Q:** truncated와 occluded의 차이는?

**A:**
```
truncated (잘림):
  - 이미지 경계에서 잘린 정도
  - 0.0: 완전히 보임
  - 1.0: 완전히 잘림 (이미지 밖으로 나감)

occluded (가려짐):
  - 다른 객체에 의해 가려진 정도
  - 0: 완전히 보임
  - 1: 부분적으로 가려짐
  - 2: 대부분 가려짐
  - 3: 알 수 없음

차이:
  truncated = 이미지 경계 문제 (카메라 시야각)
  occluded = 다른 물체 뒤에 숨김 (3D 공간 문제)
```

### Q4: 데이터셋 크기와 한계
**Q:** KITTI가 2012년 데이터셋인데도 아직 사용되는 이유는?

**A:**
```
1. 표준 벤치마크: 많은 논문이 KITTI로 비교하므로 공정한 비교 가능
2. 적절한 크기: 7,481장은 연구 실험에 적합 (nuScenes는 ~400GB)
3. 캘리브레이션 품질: 정밀한 센서 캘리브레이션 제공
4. 다양한 Task: 3D Detection, Depth, Flow, Stereo 등

단, 한계도 있습니다:
  - 독일 도시 한 곳의 데이터 (다양성 부족)
  - 전방 카메라 1대 (360도 커버리지 없음)
  - 야간/악천후 데이터 없음
  → 이 한계를 보완하기 위해 nuScenes, Waymo 등이 등장
```

---

## 이번 주 실습 & 다음 주 준비

### 이번 주 체크리스트

- [ ] KITTI 데이터셋 다운로드 (또는 일부만)
- [ ] 디렉토리 구조 (image_2, calib, label_2) 이해
- [ ] 캘리브레이션 파일 파싱 구현
- [ ] 레이블 파일 파싱 구현
- [ ] 2D bbox 시각화
- [ ] 3D bbox 이미지 투영 및 시각화
- [ ] BEV 시각화
- [ ] `PRACTICE.md` 실습 완료
- [ ] `quiz_easy.py`, `quiz_medium.py` 풀기

### 다음 주 미리보기: Monocular 3D Detection 모델

```
다음 주에는:
  - SMOKE (Keypoint 기반 3D Detection)
  - FCOS3D (Anchor-free, Multi-task Head)
  - Depth 추정 방법론
  - 3D Box 인코딩 (sin/cos rotation)
  → 실제 모델이 어떻게 3D bbox를 예측하는지 학습합니다!
```

---

## 이번 주 핵심 요약

1. **KITTI 데이터셋**: 3D Detection 표준 벤치마크. 7,481장 학습 + 7,518장 테스트. 센서: 스테레오 카메라 + LiDAR + GPS/IMU

2. **디렉토리 구조**: image_2(이미지), calib(캘리브레이션), label_2(레이블). 모두 6자리 숫자(000000~007480)로 정렬

3. **레이블 15개 필드**: class, truncated, occluded, alpha, bbox_2d(4), h, w, l, x, y, z, ry. [h,w,l] 순서에 주의

4. **시각화 3종류**: 2D bbox(이미지), 3D bbox(이미지 투영), BEV(X-Z 평면). 각각 다른 정보를 제공하며 모두 구현해야 함

5. **난이도 기준**: Easy/Moderate/Hard. Moderate가 표준 벤치마크 기준이며 대부분의 논문이 이를 보고

---

이전: [Week 2 - 좌표계 이해](../week2/README.md)

다음: [Week 4 - Monocular 3D Detection 모델](../week4/README.md)
