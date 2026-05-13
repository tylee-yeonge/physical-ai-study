# Week 11 실습: Detection + Depth 융합 (Python)


> **목표**: YOLO 검출 + Depth 맵으로 3D 위치 추정 파이프라인을 구축하고 ROS2와 통합
> **언어**: Python (PyTorch, OpenCV, ultralytics, rclpy)
> **예상 시간**: 12시간


---


## 실습 개요


| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | YOLO + Depth 개별 추론 확인 | 필수 | 2시간 |
| 2 | BBox에서 Robust Depth 추출 | 필수 | 2시간 |
| 3 | 2D → 3D 역투영 파이프라인 | 필수 | 3시간 |
| 4 | ROS2 MarkerArray 퍼블리시 | 심화 | 3시간 |
| 5 | 통합 데모 및 시각화 | 필수 | 2시간 |


---


## 환경 설정


```bash
# 가상환경 활성화
conda activate phase5


# 패키지 설치
pip install -r requirements.txt


# ROS2 사용 시 (선택)
source /opt/ros/humble/setup.bash
```


---


## Step 1: YOLO + Depth 개별 추론


### 1.1 YOLO 검출 테스트


```python
# test_yolo_detection.py
"""YOLO 검출 테스트"""
from ultralytics import YOLO
import cv2
import numpy as np


def test_yolo():
    """YOLOv8 검출 테스트"""
    print("=" * 50)
    print("Step 1: YOLO 검출 테스트")
    print("=" * 50)


    # 모델 로드
    model = YOLO('yolov8n.pt') # nano 모델


    # 테스트 이미지 (실제로는 카메라 이미지 사용)
    img = np.random.randint(50, 200, (480, 640, 3), dtype=np.uint8)
    # img = cv2.imread('test_image.jpg')


    # 추론
    results = model(img, verbose=False)


    # 결과 파싱
    for r in results:
        print(f"검출 수: {len(r.boxes)}")
        for box in r.boxes:
            bbox = box.xyxy[0].cpu().numpy().astype(int)
            cls = r.names[int(box.cls)]
            conf = float(box.conf)
            print(f"클래스: {cls}, 신뢰도: {conf:.2f}, BBox: {bbox}")


    return results


if __name__ == "__main__":
    test_yolo()
```


### 1.2 Depth 추론 테스트


```python
# test_depth_inference.py
"""Depth Anything 추론 테스트"""
import torch
import numpy as np
from transformers import AutoModelForDepthEstimation, AutoImageProcessor
from PIL import Image


def test_depth():
    """Depth Anything 추론 테스트"""
    print("=" * 50)
    print("Step 1: Depth 추론 테스트")
    print("=" * 50)


    model_name = "LiheYoung/depth-anything-small-hf"
    processor = AutoImageProcessor.from_pretrained(model_name)
    model = AutoModelForDepthEstimation.from_pretrained(model_name)
    model.eval()


    # 테스트 이미지
    img = np.random.randint(50, 200, (480, 640, 3), dtype=np.uint8)
    pil_image = Image.fromarray(img)


    # 추론
    inputs = processor(images=pil_image, return_tensors="pt")
    with torch.no_grad():
        outputs = model(**inputs)
        depth = outputs.predicted_depth


    # 원본 크기로 리사이즈
    depth_map = torch.nn.functional.interpolate(
        depth.unsqueeze(1),
        size=(480, 640),
        mode="bicubic",
        align_corners=False,
    ).squeeze().numpy()


    print(f"Depth map shape: {depth_map.shape}")
    print(f"Depth range: [{depth_map.min():.3f}, {depth_map.max():.3f}]")


    return depth_map


if __name__ == "__main__":
    test_depth()
```


### 1.3 체크포인트


```
 YOLO 검출 동작 확인
 Depth 추론 동작 확인
 출력 shape/범위 확인
```


---


## Step 2: BBox에서 Robust Depth 추출


### 2.1 구현


```python
# robust_depth_extraction.py
"""BBox 내 Robust Depth 추출"""
import numpy as np


def extract_center_depth(depth_map, bbox, center_ratio=0.2):
    """BBox 중심 영역의 median으로 깊이 추출


    Args:
        depth_map: (H, W) 깊이 맵
        bbox: (x1, y1, x2, y2)
        center_ratio: 중심 영역 비율 (0.2 = 20%)


    Returns:
        depth: 추출된 깊이 값 (음수면 유효하지 않음)
    """
    x1, y1, x2, y2 = bbox
    w, h = x2 - x1, y2 - y1


    if w <= 0 or h <= 0:
        return -1.0


    # 중심 영역 좌표
    cx, cy = (x1 + x2) // 2, (y1 + y2) // 2
    half_w = max(1, int(w * center_ratio / 2))
    half_h = max(1, int(h * center_ratio / 2))


    # ROI 추출 (경계 처리)
    H, W = depth_map.shape
    roi_y1 = max(0, cy - half_h)
    roi_y2 = min(H, cy + half_h)
    roi_x1 = max(0, cx - half_w)
    roi_x2 = min(W, cx + half_w)


    roi = depth_map[roi_y1:roi_y2, roi_x1:roi_x2]


    # 유효 깊이만 사용
    valid = roi[roi > 0]
    if len(valid) == 0:
        return -1.0


    return float(np.median(valid))


def compare_depth_strategies(depth_map, bbox):
    """3가지 Depth 추출 전략 비교"""
    x1, y1, x2, y2 = bbox
    cx, cy = (x1 + x2) // 2, (y1 + y2) // 2


    # 방법 1: 중심점
    center_depth = depth_map[cy, cx] if 0 <= cy < depth_map.shape[0] and 0 <= cx < depth_map.shape[1] else -1


    # 방법 2: 중심 영역 median
    median_depth = extract_center_depth(depth_map, bbox, center_ratio=0.2)


    # 방법 3: BBox 내 최솟값
    roi = depth_map[y1:y2, x1:x2]
    valid = roi[roi > 0]
    min_depth = float(np.min(valid)) if len(valid) > 0 else -1


    print(f"BBox: ({x1}, {y1}, {x2}, {y2})")
    print(f"방법 1 (중심점): {center_depth:.3f}")
    print(f"방법 2 (median): {median_depth:.3f}")
    print(f"방법 3 (최솟값): {min_depth:.3f}")


    return center_depth, median_depth, min_depth


# 실행
if __name__ == "__main__":
    print("=" * 50)
    print("Step 2: Robust Depth 추출 비교")
    print("=" * 50)


    # 시뮬레이션 depth map
    depth_map = np.random.uniform(5, 30, (480, 640)).astype(np.float32)


    # BBox 예시
    bbox = (100, 100, 300, 350)


    compare_depth_strategies(depth_map, bbox)
```


### 2.2 체크포인트


```
 3가지 Depth 추출 전략 구현
 Median 방법이 가장 안정적임을 확인
 경계 처리 (0 이하 값, 범위 초과 등)
```


---


## Step 3: 2D → 3D 역투영 파이프라인


### 3.1 전체 융합 파이프라인


```python
# detection_depth_fusion.py
"""Detection + Depth 융합 파이프라인"""
import numpy as np
import cv2
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


class CameraParams:
    """카메라 내부 파라미터"""
    def __init__(self, fx, fy, cx, cy, width, height):
        self.fx = fx
        self.fy = fy
        self.cx = cx
        self.cy = cy
        self.width = width
        self.height = height


    @property
    def K(self):
        return np.array([
            [self.fx, 0, self.cx],
            [0, self.fy, self.cy],
            [0, 0, 1]
        ])


def unproject_point(u, v, depth, camera):
    """2D 픽셀 + 깊이 → 3D 카메라 좌표


    Args:
        u, v: 픽셀 좌표
        depth: 깊이 값 (미터)
        camera: CameraParams 객체


    Returns:
        (X, Y, Z) 카메라 좌표계에서의 3D 위치
    """
    Z = depth
    X = (u - camera.cx) * Z / camera.fx
    Y = (v - camera.cy) * Z / camera.fy
    return X, Y, Z


def process_detections_3d(detections_2d, depth_map, camera):
    """2D 검출 + Depth → 3D 위치 추정


    Args:
        detections_2d: list of {'bbox': (x1,y1,x2,y2), 'class': str, 'confidence': float}
        depth_map: (H, W) 깊이 맵
        camera: CameraParams 객체


    Returns:
        detections_3d: list of 3D 검출 결과
    """
    detections_3d = []


    for det in detections_2d:
        bbox = det['bbox']
        x1, y1, x2, y2 = bbox


        # 1. Robust Depth 추출
        depth = extract_center_depth(depth_map, bbox, center_ratio=0.2)
        if depth <= 0:
            continue


        # 2. BBox 중심점
        u = (x1 + x2) / 2.0
        v = (y1 + y2) / 2.0


        # 3. 역투영
        X, Y, Z = unproject_point(u, v, depth, camera)


        detections_3d.append({
            'class': det['class'],
            'confidence': det['confidence'],
            'bbox_2d': bbox,
            'pixel_center': (u, v),
            'depth': depth,
            'position_3d': (X, Y, Z),
            'distance': np.sqrt(X**2 + Y**2 + Z**2),
        })


    return detections_3d


def visualize_3d_detections(image, detections_3d, depth_map):
    """3D 검출 결과 시각화"""
    fig, axes = plt.subplots(1, 3, figsize=(18, 6))


    # 1. 원본 + 2D BBox
    img_vis = image.copy()
    for det in detections_3d:
        x1, y1, x2, y2 = det['bbox_2d']
        label = f"{det['class']} {det['depth']:.1f}m"
        cv2.rectangle(img_vis, (x1, y1), (x2, y2), (0, 255, 0), 2)
        cv2.putText(img_vis, label, (x1, y1 - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)


    axes[0].imshow(cv2.cvtColor(img_vis, cv2.COLOR_BGR2RGB))
    axes[0].set_title('2D 검출 + 거리')
    axes[0].axis('off')


    # 2. Depth 맵
    im = axes[1].imshow(depth_map, cmap='turbo')
    axes[1].set_title('Depth 맵')
    axes[1].axis('off')
    plt.colorbar(im, ax=axes[1])


    # 3. Bird's Eye View (3D → 탑뷰)
    ax = axes[2]
    for det in detections_3d:
        X, Y, Z = det['position_3d']
        color = 'green' if det['class'] in ['car', 'truck'] else 'blue'
        ax.scatter(X, Z, c=color, s=100)
        ax.annotate(f"{det['class']}\n{det['distance']:.1f}m",
                    (X, Z), fontsize=8, ha='center')


    ax.set_xlabel('X (m, 오른쪽)')
    ax.set_ylabel('Z (m, 전방)')
    ax.set_title("Bird's Eye View")
    ax.set_xlim(-20, 20)
    ax.set_ylim(0, 50)
    ax.grid(True, alpha=0.3)


    plt.tight_layout()
    plt.savefig('detection_3d_results.png', dpi=150)
    print("결과 저장: detection_3d_results.png")


# 실행 예제
if __name__ == "__main__":
    print("=" * 50)
    print("Step 3: Detection + Depth 융합")
    print("=" * 50)


    # 카메라 파라미터 (KITTI 예시)
    camera = CameraParams(
        fx=721.5, fy=721.5, cx=609.6, cy=172.9,
        width=1242, height=375
    )


    # 시뮬레이션 데이터
    np.random.seed(42)
    image = np.random.randint(50, 200, (375, 1242, 3), dtype=np.uint8)
    depth_map = np.random.uniform(5, 50, (375, 1242)).astype(np.float32)


    # 시뮬레이션 2D 검출
    detections_2d = [
        {'bbox': (200, 150, 400, 300), 'class': 'car', 'confidence': 0.92},
        {'bbox': (600, 160, 750, 280), 'class': 'car', 'confidence': 0.85},
        {'bbox': (900, 170, 980, 250), 'class': 'person', 'confidence': 0.78},
    ]


    # 3D 위치 추정
    detections_3d = process_detections_3d(detections_2d, depth_map, camera)


    for det in detections_3d:
        X, Y, Z = det['position_3d']
        print(f"{det['class']}: 3D=({X:.2f}, {Y:.2f}, {Z:.2f})m, "
              f"거리={det['distance']:.2f}m, 신뢰도={det['confidence']:.2f}")


    visualize_3d_detections(image, detections_3d, depth_map)
```


### 3.2 체크포인트


```
 역투영 공식 구현
 시뮬레이션 데이터로 3D 좌표 확인
 Bird's Eye View 시각화 생성
```


---


## Step 4: ROS2 통합 (심화)


### 4.1 ROS2 노드 구현


```python
# detection_3d_ros2.py
"""ROS2 Detection 3D 퍼블리셔 노드


실행:
  ros2 run <패키지명> detection_3d_node
  또는
  python detection_3d_ros2.py
"""
# ROS2가 설치된 환경에서만 동작합니다.
# 설치 안 된 경우 Step 3까지만 수행해도 됩니다.


try:
    import rclpy
    from rclpy.node import Node
    from visualization_msgs.msg import MarkerArray, Marker
    from std_msgs.msg import Header
    ROS2_AVAILABLE = True
except ImportError:
    print("ROS2가 설치되지 않았습니다. 이 Step은 건너뛰세요.")
    ROS2_AVAILABLE = False


if ROS2_AVAILABLE:
    class Detection3DNode(Node):
        def __init__(self):
            super().__init__('detection_3d_node')
            self.marker_pub = self.create_publisher(
                MarkerArray, '/detections_3d', 10)
            self.timer = self.create_timer(0.1, self.timer_callback)
            self.get_logger().info('Detection 3D 노드 시작')


        def timer_callback(self):
            # 실제로는 카메라 콜백에서 처리
            marker_array = MarkerArray()
            self.marker_pub.publish(marker_array)


        def detections_to_markers(self, detections_3d):
            """3D 검출 → MarkerArray 변환"""
            marker_array = MarkerArray()


            for i, det in enumerate(detections_3d):
                marker = Marker()
                marker.header = Header()
                marker.header.frame_id = "camera_link"
                marker.id = i
                marker.type = Marker.CUBE
                marker.action = Marker.ADD


                # 카메라 → ROS 좌표 변환
                X, Y, Z = det['position_3d']
                marker.pose.position.x = float(Z) # 전방
                marker.pose.position.y = float(-X) # 왼쪽
                marker.pose.position.z = float(-Y) # 위


                marker.scale.x = 2.0 # 차량 크기
                marker.scale.y = 1.5
                marker.scale.z = 1.5


                marker.color.a = 0.7
                marker.color.r = 0.0
                marker.color.g = 1.0
                marker.color.b = 0.0


                marker_array.markers.append(marker)


            return marker_array


    def main():
        rclpy.init()
        node = Detection3DNode()
        rclpy.spin(node)
        node.destroy_node()
        rclpy.shutdown()
```


### 4.2 체크포인트


```
 ROS2 노드 생성 (또는 Step 3까지 완료)
 MarkerArray 퍼블리시
 RViz2에서 마커 시각화 확인
 좌표계 변환 (카메라 → ROS) 적용
```


---


## Step 5: 통합 데모


### 5.1 전체 파이프라인 실행


```python
# full_pipeline_demo.py
"""Detection + Depth 전체 파이프라인 데모"""
import numpy as np
import cv2
import time


def run_full_pipeline_demo():
    """전체 파이프라인 성능 측정"""
    print("=" * 50)
    print("Step 5: 전체 파이프라인 데모")
    print("=" * 50)


    # 시뮬레이션 (실제로는 YOLO + Depth 모델 사용)
    num_frames = 50
    times = {'yolo': [], 'depth': [], 'fusion': [], 'total': []}


    for i in range(num_frames):
        total_start = time.time()


        # YOLO 시뮬레이션
        yolo_start = time.time()
        time.sleep(0.03) # ~30ms
        times['yolo'].append(time.time() - yolo_start)


        # Depth 시뮬레이션
        depth_start = time.time()
        time.sleep(0.05) # ~50ms
        times['depth'].append(time.time() - depth_start)


        # 융합 시뮬레이션
        fusion_start = time.time()
        time.sleep(0.002) # ~2ms
        times['fusion'].append(time.time() - fusion_start)


        times['total'].append(time.time() - total_start)


    # 결과 출력
    print(f"\n {num_frames}프레임 벤치마크 결과:")
    print(f"---------------------------------")
    for key, vals in times.items():
        avg_ms = np.mean(vals) * 1000
        print(f"{key:>8}: {avg_ms:>7.1f} ms")


    total_fps = 1000.0 / (np.mean(times['total']) * 1000)
    print(f"---------------------------------")
    print(f"{'FPS':>8}: {total_fps:>7.1f}")


    if total_fps >= 10:
        print(f"\n 목표 달성! (10 FPS 이상)")
    else:
        print(f"\n 목표 미달. 최적화 필요.")


if __name__ == "__main__":
    run_full_pipeline_demo()
```


---


## 실습 체크리스트


### Step 1: 개별 추론
- [ ] YOLO 검출 동작 확인
- [ ] Depth 추론 동작 확인


### Step 2: Depth 추출
- [ ] 3가지 전략 구현 및 비교
- [ ] Median 기반 추출이 가장 안정적임을 확인


### Step 3: 3D 역투영
- [ ] 역투영 공식 구현
- [ ] 3D 좌표 출력 확인
- [ ] Bird's Eye View 시각화


### Step 4: ROS2 (선택)
- [ ] MarkerArray 퍼블리시
- [ ] RViz2 시각화 확인
- [ ] 좌표계 변환 적용


### Step 5: 통합 데모
- [ ] 전체 파이프라인 성능 측정
- [ ] FPS 확인
- [ ] 결과 시각화


---


## 트러블슈팅


### YOLO 검출이 안 될 때


```
문제: 검출 결과가 0개
해결: confidence threshold 확인 (기본 0.25 → 0.1로 낮춰보기)
     model(img, conf=0.1)
```


### 3D 좌표가 이상할 때


```
문제: X, Y 값이 비정상적으로 큼
해결: 카메라 파라미터(fx, fy, cx, cy) 확인
     Depth 맵의 단위(미터 vs 밀리미터) 확인
     Depth가 상대적 깊이인 경우 Scale 정렬 필요
```


---


## 참고 자료


- [Ultralytics YOLOv8 문서](https://docs.ultralytics.com/)
- [Depth Anything HuggingFace](https://huggingface.co/LiheYoung/depth-anything-small-hf)
- [ROS2 Humble 문서](https://docs.ros.org/en/humble/)
- [RViz2 Marker 튜토리얼](https://docs.ros.org/en/humble/Tutorials.html)


---


**다음**: Quiz로 개념 점검!
