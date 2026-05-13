# Week 11 실습: vla_node 의 실 inference 통합 + 1분 dry-run

> [goal] **실습 목표**: dummy 노드를 실제 inference 로 교체하고 1분간 0 fail dry-run.
> [time] **예상 시간**: 6~8시간

---

## [tool] 환경 설정

```bash
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash

# vla_inference 를 pip 으로 (방법 2)
cd ~/phase4_notes/week8/vla_inference
pip install -e .
```

---

## [note] 실습 1: vla_inference_node.py 통합

**파일명**: `~/ros2_ws/src/vla_node/vla_node/vla_inference_node.py`

week 10 의 노드를 다음과 같이 수정:

```python
"""
vla_node Week 11: 실 inference 통합
"""
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSDurabilityPolicy

from sensor_msgs.msg import Image
from geometry_msgs.msg import Twist
from std_msgs.msg import String, Float64
from cv_bridge import CvBridge

from vla_inference.inference import VLAInference
from vla_inference.preprocess import opencv_to_pil
from vla_inference.exceptions import (
    VLAInferenceError, VLAOOMError, VLAInputError, VLAOutputError,
)


class VLANode(Node):
    def __init__(self):
        super().__init__('vla_node')

        # parameters
        self.declare_parameter('model_id', 'openvla/openvla-7b')
        self.declare_parameter('quant_type', '4bit')
        self.declare_parameter('unnormalize_key', 'bridge_orig')
        self.declare_parameter('image_age_threshold_ms', 200.0)
        self.declare_parameter('image_topic', '/camera/image_raw')

        model_id = self.get_parameter('model_id').get_parameter_value().string_value
        quant_type = self.get_parameter('quant_type').get_parameter_value().string_value
        unnorm_key = self.get_parameter('unnormalize_key').get_parameter_value().string_value
        self.age_threshold = self.get_parameter(
            'image_age_threshold_ms').get_parameter_value().double_value
        image_topic = self.get_parameter('image_topic').get_parameter_value().string_value

        # subscribers / publishers (week 10 그대로)
        image_qos = QoSProfile(reliability=QoSReliabilityPolicy.BEST_EFFORT, depth=1)
        action_qos = QoSProfile(reliability=QoSReliabilityPolicy.RELIABLE, depth=10)
        instr_qos = QoSProfile(
            reliability=QoSReliabilityPolicy.RELIABLE,
            durability=QoSDurabilityPolicy.TRANSIENT_LOCAL,
            depth=1)

        self.image_sub = self.create_subscription(
            Image, image_topic, self.image_callback, image_qos)
        self.instr_sub = self.create_subscription(
            String, '/vla/instruction', self.instruction_callback, instr_qos)
        self.action_pub = self.create_publisher(Twist, '/vla/action', action_qos)
        self.gripper_pub = self.create_publisher(Float64, '/vla/gripper', action_qos)
        self.latency_pub = self.create_publisher(Float64, '/vla/latency_ms', 10)
        self.status_pub = self.create_publisher(String, '/vla/status', 10)

        # VLA inference
        self.get_logger().info(f'Loading VLA: {model_id} ({quant_type})')
        self._publish_status('loading')
        self.vla = VLAInference(model_id=model_id, quant_type=quant_type, unnorm_key=unnorm_key)
        self.vla.warm_up(n_iter=5)
        self.bridge = CvBridge()

        self.current_instruction = ""
        self._publish_status('ready')
        self.get_logger().info('VLANode ready')

    def _publish_status(self, status):
        msg = String(); msg.data = status
        self.status_pub.publish(msg)

    def instruction_callback(self, msg: String):
        self.current_instruction = msg.data
        self.get_logger().info(f'New instruction: {self.current_instruction}')

    def image_callback(self, msg: Image):
        if not self.current_instruction:
            return

        # 1. image age check
        t_image = rclpy.time.Time.from_msg(msg.header.stamp)
        t_now = self.get_clock().now()
        age_ms = (t_now - t_image).nanoseconds / 1e6
        if age_ms > self.age_threshold:
            self.get_logger().warning(f'image too old: {age_ms:.1f}ms')
            return

        # 2. preprocess
        try:
            img_bgr = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
            img_pil = opencv_to_pil(img_bgr)
        except Exception as e:
            self.get_logger().error(f'preprocess fail: {e}')
            return

        # 3. inference with error handling
        t0 = self.get_clock().now()
        try:
            action = self.vla.predict(img_pil, self.current_instruction)
        except VLAOOMError as e:
            self.get_logger().error(f'OOM, skipping: {e}')
            self._publish_status('error: oom')
            return
        except VLAOutputError as e:
            self.get_logger().error(f'output error: {e}')
            return
        except VLAInferenceError as e:
            self.get_logger().error(f'inference fail: {e}')
            return

        # 4. publish
        twist = Twist()
        twist.linear.x = float(action[0])
        twist.linear.y = float(action[1])
        twist.linear.z = float(action[2])
        twist.angular.x = float(action[3])
        twist.angular.y = float(action[4])
        twist.angular.z = float(action[5])
        self.action_pub.publish(twist)

        gripper = Float64(); gripper.data = float(action[6])
        self.gripper_pub.publish(gripper)

        latency_ms = (self.get_clock().now() - t0).nanoseconds / 1e6
        lat = Float64(); lat.data = latency_ms
        self.latency_pub.publish(lat)


def main(args=None):
    rclpy.init(args=args)
    node = VLANode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
```

---

## [note] 실습 2: 빌드 + 실행

```bash
cd ~/ros2_ws
colcon build --packages-select vla_node --symlink-install
source install/setup.bash

# 노드 실행 (모델 로딩 ~ 1분 + warm-up ~ 5초)
ros2 run vla_node vla_inference_node
```

다른 터미널:

```bash
# instruction 발행
ros2 topic pub --once /vla/instruction std_msgs/String "data: 'pick up the red can'"

# image bag 재생 (또는 image_publisher 사용)
ros2 bag play my_test_bag --loop --clock

# action 모니터링
ros2 topic echo /vla/action --no-arr | head -50
```

---

## [note] 실습 3: 1분 dry-run + 통계 수집

```bash
# 1분간 record
ros2 bag record /vla/action /vla/latency_ms /vla/status -o dry_run_$(date +%Y%m%d) &
BAGPID=$!
sleep 60
kill $BAGPID

# 분석
ros2 bag info dry_run_*/
```

**파일명**: `practice_analyze_dry_run.py`

```python
"""
실습 3: 1분 dry-run 의 latency / fail rate 분석
"""
import subprocess
import re
import numpy as np

print("=" * 60)
print("실습 3: dry-run 분석")
print("=" * 60)

# ros2 bag 의 latency_ms topic 을 읽음 (수동 또는 rosbag2_py 사용)
# 여기선 ros2 topic echo > log.txt 로 수집했다고 가정

latencies = []
with open("latency.log") as f:
    for line in f:
        m = re.search(r"data:\s*([\d.]+)", line)
        if m:
            latencies.append(float(m.group(1)))

arr = np.array(latencies)
print(f"\n총 inference 수 : {len(arr)}")
print(f"mean    : {arr.mean():.1f} ms")
print(f"median  : {np.median(arr):.1f} ms")
print(f"p95     : {np.percentile(arr, 95):.1f} ms")
print(f"p99     : {np.percentile(arr, 99):.1f} ms")
print(f"max     : {arr.max():.1f} ms")

# Throughput
total_time_s = 60
print(f"\nThroughput: {len(arr) / total_time_s:.2f} Hz")

# Success criteria
print("\n--- Success Criteria ---")
expected_min_count = 60 * 5 - 20  # 1분 * 5Hz, 약간 여유
print(f"  count > {expected_min_count}: {len(arr) > expected_min_count}")
print(f"  mean < 200ms:                  {arr.mean() < 200}")
print(f"  p95 < 300ms:                   {np.percentile(arr, 95) < 300}")
```

---

## [note] 실습 4: 자작 영상 입력 (선택)

```bash
# 자작 팔 환경 1분 동영상 녹화 (스마트폰 또는 ELP)
# -> 224x224 또는 640x480 으로 resize
# -> ROS2 bag 으로 변환

# ROS2 video_stream_opencv 패키지 사용
sudo apt install ros-humble-video-stream-opencv

ros2 launch video_stream_opencv camera.launch.py \
  video_stream_provider:=/path/to/my_video.mp4 \
  loop_videofile:=true \
  camera_name:=my_camera
```

---

## [O] 실습 체크리스트

- [ ] vla_inference_node.py 에 실 inference 통합
- [ ] colcon build 성공
- [ ] ros2 run 실행, 모델 로드 성공
- [ ] instruction 발행 시 노드 반응
- [ ] image bag 재생 시 action publish
- [ ] 1분 dry-run 0 fail
- [ ] latency 통계 확보 (mean < 200ms)
- [ ] quiz_easy / quiz_medium

---

## [link] 참고 자료

- [ros2 bag tutorial](https://docs.ros.org/en/humble/Tutorials/Beginner-CLI-Tools/Recording-And-Playing-Back-Data.html)
- [rosbag2_py for analysis](https://github.com/ros2/rosbag2/tree/humble/rosbag2_py)
- [ros2_video_streamer](https://github.com/ros-perception/video_stream_opencv)
