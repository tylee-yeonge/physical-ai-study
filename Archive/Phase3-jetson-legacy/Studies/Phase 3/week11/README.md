# Week 11: Detection + Depth 융합 (Section 5.4)

> [goal] **이번 주 목표**: YOLO 2D 검출 + Depth 맵을 결합하여 3D 위치 추정 파이프라인을 구축하고, ROS2와 통합하기
> [time] **예상 시간**: 12시간
> [tip] **핵심 질문**: "2D 바운딩 박스와 깊이 정보만으로 물체의 3D 좌표를 어떻게 구하는가?"

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 설정 | `requirements.txt` | 첫 실행 시 `pip install -r requirements.txt` |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | Python 퀴즈 (초급) | `quiz_easy.py` | 2D-3D 역투영, 카메라 내부 파라미터 개념 확인 |
| 4 | Python 퀴즈 (중급) | `quiz_medium.py` | Detection + Depth 융합 파이프라인 심화 |
| 5 | 실습 | [PRACTICE.md](./PRACTICE.md) | Detection + Depth 융합 및 ROS2 통합 |

---

## [*] 시작하기 전에

### Week 10에서 배운 것

**Depth 모델 검증 완료:**
```
- AbsRel < 0.1 달성 가능
- 근거리(0~30m)에서는 신뢰할 수 있는 수준
- 경계선/반사/원거리는 취약점
```

**이제 다음 단계:**
```
[?] YOLO가 "여기에 차가 있다"고 알려줬다면?
[?] Depth 맵에서 "그 차까지의 거리"를 알 수 있다면?
[?] 두 정보를 합치면 "차가 3D 공간 어디에 있는지" 알 수 있지 않을까?
[?] 이걸 ROS2로 실시간 퍼블리시할 수 있을까?
```

**이번 주에 답합니다!**

---

## [ref] 핵심 개념 자세히 알아보기

### 1. 3D 위치 추정 파이프라인 전체 흐름

#### 1.1 파이프라인 구조

```
[카메라 이미지]
      |
      +--→ [YOLO 검출] --→ 2D BBox (x, y, w, h) + class + confidence
      |
      +--→ [Depth 추정] --→ Depth Map (H × W)
               |
               v
    [BBox 중심점의 Depth 추출]
               |
               v
    [2D → 3D 역투영 (Unprojection)]
      (u, v, Z) → (X, Y, Z)
               |
               v
    [3D BBox / 포인트 클라우드]
               |
               v
    [ROS2 퍼블리시]
      - /detections_3d (MarkerArray)
      - /objects (DetectionArray)
```

#### 1.2 각 단계의 역할

| 단계 | 입력 | 출력 | 도구 |
|------|------|------|------|
| YOLO 검출 | RGB 이미지 | 2D BBox + 클래스 | YOLOv8/v11 |
| Depth 추정 | RGB 이미지 | Depth 맵 | Depth Anything |
| Depth 추출 | BBox + Depth 맵 | 물체 깊이 (Z) | NumPy 연산 |
| 역투영 | (u, v, Z) + 카메라 파라미터 | (X, Y, Z) | 핀홀 모델 역변환 |
| ROS2 퍼블리시 | 3D 좌표 | 시각화 마커 | rclpy |

---

### 2. 2D → 3D 역투영 (Unprojection)

#### 2.1 핀홀 카메라 모델 복습

```
투영 (3D → 2D):
  u = fx * X / Z + cx
  v = fy * Y / Z + cy

역투영 (2D → 3D):
  X = (u - cx) * Z / fx
  Y = (v - cy) * Z / fy
  Z = depth_map[v, u]

카메라 내부 파라미터 행렬 K:
  K = [fx  0  cx]
      [ 0 fy  cy]
      [ 0  0   1]
```

#### 2.2 BBox 중심점에서 Depth 추출

```
BBox: (x1, y1, x2, y2)
중심점: cx = (x1 + x2) / 2
        cy = (y1 + y2) / 2

Depth 추출 전략:
1. 단순 중심점: Z = depth_map[cy, cx]
   → 간단하지만 노이즈에 취약

2. 중심 영역 중앙값 (권장):
   → BBox 중심 20% 영역의 median
   → 노이즈에 강건

3. BBox 내 최솟값:
   → 가장 가까운 부분의 깊이
   → 안전 거리 계산에 적합
```

#### 2.3 역투영 공식

```python
def unproject_to_3d(u, v, depth, fx, fy, cx, cy):
    """2D 픽셀 + 깊이 → 3D 카메라 좌표"""
    Z = depth
    X = (u - cx) * Z / fx
    Y = (v - cy) * Z / fy
    return X, Y, Z
```

---

### 3. BBox에서 Robust하게 Depth 추출하기

#### 3.1 왜 단순 중심점이 위험한가?

```
BBox 영역:
+------------------+
|  배경 (먼 거리)    |
|  +------------+  |
|  |  물체       |  |
|  |    * 중심점  |  |  ← 물체가 BBox를 채우지 않으면
|  |            |  |     배경 깊이를 읽을 수 있음!
|  +------------+  |
|  배경 (먼 거리)    |
+------------------+

해결: BBox 중심 20% 영역의 median 사용
```

#### 3.2 Robust Depth 추출

```python
def extract_robust_depth(depth_map, bbox, ratio=0.2):
    """BBox 내에서 안정적으로 깊이를 추출"""
    x1, y1, x2, y2 = bbox
    w, h = x2 - x1, y2 - y1

    # 중심 20% 영역
    cx, cy = (x1 + x2) // 2, (y1 + y2) // 2
    half_w = int(w * ratio / 2)
    half_h = int(h * ratio / 2)

    roi = depth_map[
        max(0, cy - half_h):cy + half_h,
        max(0, cx - half_w):cx + half_w
    ]

    # 유효 깊이만 사용
    valid = roi[roi > 0]
    if len(valid) == 0:
        return -1  # 유효 깊이 없음

    # 중앙값 사용 (이상치에 강건)
    return np.median(valid)
```

---

### 4. 전체 Detection + Depth 융합 코드

#### 4.1 메인 파이프라인

```python
from ultralytics import YOLO
import numpy as np
import cv2

class DetectionDepthFusion:
    def __init__(self, yolo_model_path, depth_model, camera_params):
        self.yolo = YOLO(yolo_model_path)
        self.depth_model = depth_model
        self.fx = camera_params['fx']
        self.fy = camera_params['fy']
        self.cx = camera_params['cx']
        self.cy = camera_params['cy']

    def process_frame(self, image):
        """한 프레임 처리: 검출 + 깊이 + 3D 위치"""

        # 1. YOLO 검출
        results = self.yolo(image, verbose=False)
        detections_2d = self._parse_yolo_results(results)

        # 2. Depth 추정
        depth_map = self.depth_model.infer(image)

        # 3. 각 검출에 대해 3D 위치 계산
        detections_3d = []
        for det in detections_2d:
            bbox = det['bbox']  # (x1, y1, x2, y2)

            # 깊이 추출
            z = extract_robust_depth(depth_map, bbox)
            if z <= 0:
                continue

            # 2D → 3D 역투영
            u = (bbox[0] + bbox[2]) / 2
            v = (bbox[1] + bbox[3]) / 2

            x = (u - self.cx) * z / self.fx
            y = (v - self.cy) * z / self.fy

            detections_3d.append({
                'class': det['class'],
                'confidence': det['confidence'],
                'bbox_2d': bbox,
                'position_3d': (x, y, z),
                'depth': z,
            })

        return detections_3d, depth_map

    def _parse_yolo_results(self, results):
        """YOLO 결과를 딕셔너리 리스트로 변환"""
        detections = []
        for r in results:
            for box in r.boxes:
                detections.append({
                    'bbox': box.xyxy[0].cpu().numpy().astype(int),
                    'class': r.names[int(box.cls)],
                    'confidence': float(box.conf),
                })
        return detections
```

---

### 5. ROS2 통합

#### 5.1 ROS2 퍼블리셔 노드

```python
# ros2_detection_3d_node.py (개념 코드)
import rclpy
from rclpy.node import Node
from visualization_msgs.msg import MarkerArray, Marker
from geometry_msgs.msg import Point

class Detection3DPublisher(Node):
    def __init__(self):
        super().__init__('detection_3d_publisher')

        # 퍼블리셔
        self.marker_pub = self.create_publisher(
            MarkerArray, '/detections_3d', 10)

        # 타이머 (30Hz)
        self.timer = self.create_timer(1.0/30, self.timer_callback)

        self.get_logger().info('Detection 3D 노드 시작')

    def timer_callback(self):
        # 이미지 캡처 → 검출 + 깊이 → 3D 위치
        # detections_3d = fusion.process_frame(image)

        # MarkerArray로 변환
        marker_array = MarkerArray()
        # ... (각 검출에 대해 마커 생성)

        self.marker_pub.publish(marker_array)

    def create_marker(self, detection, marker_id):
        """3D 검출 결과를 RViz2 마커로 변환"""
        marker = Marker()
        marker.header.frame_id = "camera_link"
        marker.id = marker_id
        marker.type = Marker.CUBE
        marker.action = Marker.ADD

        x, y, z = detection['position_3d']
        marker.pose.position = Point(x=z, y=-x, z=-y)  # 카메라→ROS 좌표 변환

        marker.scale.x = 1.0
        marker.scale.y = 1.0
        marker.scale.z = 1.0

        # 클래스별 색상
        if detection['class'] == 'car':
            marker.color.r = 0.0
            marker.color.g = 1.0
            marker.color.b = 0.0
        else:
            marker.color.r = 1.0
            marker.color.g = 0.0
            marker.color.b = 0.0

        marker.color.a = 0.7
        marker.lifetime.sec = 0
        marker.lifetime.nanosec = 100000000  # 0.1초

        return marker
```

#### 5.2 카메라 좌표계 → ROS 좌표계

```
카메라 좌표계:        ROS 좌표계 (REP 103):
  Z →  전방            X →  전방
  X →  오른쪽          Y →  왼쪽
  Y →  아래            Z →  위

변환:
  ROS_X = Camera_Z   (전방)
  ROS_Y = -Camera_X  (왼쪽)
  ROS_Z = -Camera_Y  (위)
```

#### 5.3 ROS2 토픽 구조

```
/camera/image_raw     ← 카메라 이미지 (sensor_msgs/Image)
/camera/camera_info   ← 카메라 파라미터 (sensor_msgs/CameraInfo)
/detections_2d        ← YOLO 2D 검출 (custom msg)
/depth_map            ← 깊이 맵 (sensor_msgs/Image)
/detections_3d        ← 3D 검출 마커 (visualization_msgs/MarkerArray)
```

---

## [tip] 꼭 이해해야 할 핵심 개념

### 개념 1: 역투영(Unprojection)의 원리

```
투영: 3D 세계 → 2D 이미지 (정보 손실)
역투영: 2D 이미지 + Depth → 3D 세계 (정보 복원)

핵심: Depth 정보가 없으면 역투영 불가능!
  → 2D 픽셀 (u, v)는 3D 공간에서 '직선(ray)' 위의 아무 점
  → Depth가 있어야 직선 위의 '정확한 점'을 결정
```

### 개념 2: BBox 중심 Depth vs 영역 Depth

```
방법 1: 중심점 (u, v) → 1개의 depth 값
  장점: 단순, 빠름
  단점: 노이즈에 취약, 배경 깊이를 읽을 수 있음

방법 2: BBox 내 중심 20% 영역 median
  장점: 이상치에 강건
  단점: 약간의 연산 오버헤드

방법 3: BBox 내 최솟값 (nearest depth)
  장점: 충돌 회피에 적합
  단점: 노이즈에 매우 민감

권장: 방법 2 (중심 영역 median)
```

### 개념 3: YOLO + Depth 동시 처리의 효율성

```
순차 실행:
  YOLO (30ms) → Depth (55ms) = 총 85ms (12 FPS)

병렬 실행 (GPU 2 스트림):
  YOLO  ████████ (30ms)
  Depth ████████████████ (55ms)
  총 시간 = max(30, 55) = 55ms (18 FPS)

병렬 처리를 위해:
  - CUDA 스트림 분리
  - 또는 asyncio 활용
  - 또는 ROS2 멀티스레딩
```

---

## [search] 자체 점검 - 이해했는지 확인!

**Q1: 역투영(Unprojection) 공식에서 X, Y, Z를 구하는 식은?**

<details>
<summary>정답 보기</summary>

- Z = depth_map[v, u] (깊이 맵에서 직접 읽음)
- X = (u - cx) * Z / fx
- Y = (v - cy) * Z / fy
- (u, v): 픽셀 좌표, (fx, fy): 초점 거리, (cx, cy): 주점

</details>

**Q2: BBox에서 Depth를 추출할 때 median을 사용하는 이유는?**

<details>
<summary>정답 보기</summary>

- BBox에는 물체뿐 아니라 배경도 포함될 수 있음
- 단순 평균은 배경 깊이에 의해 왜곡됨
- Median은 이상치(outlier)에 강건하여 물체의 실제 깊이를 더 잘 반영
- 추가로 BBox 중심 20% 영역만 사용하면 배경 영향을 더 줄일 수 있음

</details>

**Q3: 카메라 좌표계와 ROS 좌표계의 차이는?**

<details>
<summary>정답 보기</summary>

- 카메라: X(오른쪽), Y(아래), Z(전방)
- ROS (REP 103): X(전방), Y(왼쪽), Z(위)
- 변환: ROS_X = Cam_Z, ROS_Y = -Cam_X, ROS_Z = -Cam_Y
- 이 변환을 빠뜨리면 RViz2에서 물체가 잘못된 위치에 표시됨

</details>

**Q4: YOLO와 Depth를 병렬로 실행하면 FPS가 향상되는 원리는?**

<details>
<summary>정답 보기</summary>

- 두 모델이 독립적으로 같은 입력 이미지를 처리
- CUDA 스트림을 분리하여 GPU에서 동시 실행 가능
- 총 시간 = max(YOLO 시간, Depth 시간) (순차: 합산)
- 예: YOLO 30ms + Depth 55ms → 순차 85ms → 병렬 55ms

</details>

---

## [note] 이번 주 실습 & 다음 주 준비

### 실습 구성

| Step | 내용 | 예상 시간 |
|------|------|----------|
| 1 | YOLO + Depth 개별 추론 확인 | 2시간 |
| 2 | BBox에서 Robust Depth 추출 | 2시간 |
| 3 | 2D → 3D 역투영 파이프라인 | 3시간 |
| 4 | ROS2 MarkerArray 퍼블리시 | 3시간 |
| 5 | 통합 데모 및 시각화 | 2시간 |

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

### 다음 주 준비

- Week 12에서는 **최종 데모 + 최적화 + 포트폴리오 정리**를 수행합니다
- 전체 파이프라인의 FPS 최적화
- 성능 목표 달성 확인
- Phase 3 전체 내용 정리

---

## [goal] 이번 주 핵심 요약

### 1. 3D 위치 추정 파이프라인

```
YOLO(2D BBox) + Depth(깊이 맵) → Unprojection → 3D 좌표
```

### 2. 역투영 공식

```
X = (u - cx) * Z / fx
Y = (v - cy) * Z / fy
Z = depth_map[v, u]
```

### 3. Robust Depth 추출

```
BBox 중심 20% 영역의 median 사용
이상치(배경)에 강건
```

### 4. ROS2 통합

```
/detections_3d (MarkerArray) → RViz2에서 실시간 시각화
카메라 좌표계 → ROS 좌표계 변환 필수
```

---

이전: [Week 10 - Depth 정확도 검증](../week10/README.md)

다음: [Week 12 - 최종 데모](../week12/README.md)
