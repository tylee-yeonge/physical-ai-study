# Week 11 실습: vla_node 의 실 inference 통합 + 1분 dry-run


> **실습 목표**: dummy 노드를 실제 inference 로 교체하고 1분간 0 fail dry-run.
> **예상 시간**: 6-8시간


---


## 환경 설정


week 8 의 양자화 inference + week 10 의 ROS2 패키지를 한 노드 안에서 결합하는 주차다. 양쪽 환경이 모두 활성화돼 있어야 한다.

활성화 순서 (중요; [`../SETUP.md`](../SETUP.md) §6.3 참고): ROS2 → workspace overlay → 공용 venv.


```bash
source /opt/ros/${ROS_DISTRO}/setup.bash
source /workspace/phase4_workspace/ros2_ws/install/setup.bash
source "/workspace/study/physical-ai-study/Studies/Phase 4/.venv-vla/bin/activate"


# vla_inference 를 pip 으로 (방법 2)
cd /workspace/phase4_workspace/vla_inference
pip install -e .
```


> Step 0 필수 — `pip install -e .` 를 빠뜨리면 실습 1 의 `from vla_inference.inference import VLAInference` 가 `ModuleNotFoundError` 로 죽는다. week 8 에서 만든 `vla_inference/` 패키지를 공용 venv 에 설치하는 단계이며, 노드 빌드·실행(실습 2) 전에 반드시 선행한다. "(선택)" 이 아니다.


> 통합 후 VRAM 점검 필수 — week 8 단독 측정값 (약 6GB) + ROS2 cv_bridge + image subscribe 큐 + KV 캐시가 더해진다. 12GB 4070 에서 빠듯하면 batch 1 고정 + action chunk 축소 (SETUP.md §9.3).


---


## 실습 1: vla_inference_node.py 통합


**파일명**: `/workspace/phase4_workspace/ros2_ws/src/vla_node/vla_node/vla_inference_node.py`


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


## 실습 2: 빌드 + 실행


```bash
cd /workspace/phase4_workspace/ros2_ws
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


## 실습 3: 1분 dry-run + 통계 수집


```bash
# 1분간 record
ros2 bag record /vla/action /vla/latency_ms /vla/status -o dry_run_$(date +%Y%m%d) &
BAGPID=$!
sleep 60
kill $BAGPID


# 분석
ros2 bag info dry_run_*/
```


**파일명**: `/workspace/phase4_workspace/dryrun_analysis/practice_analyze_dry_run.py`


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
print(f"mean : {arr.mean():.1f} ms")
print(f"median : {np.median(arr):.1f} ms")
print(f"p95 : {np.percentile(arr, 95):.1f} ms")
print(f"p99 : {np.percentile(arr, 99):.1f} ms")
print(f"max : {arr.max():.1f} ms")


# Throughput
total_time_s = 60
print(f"\nThroughput: {len(arr) / total_time_s:.2f} Hz")


# Success criteria
print("\n--- Success Criteria ---")
expected_min_count = 60 * 5 - 20 # 1분 * 5Hz, 약간 여유
print(f"count > {expected_min_count}: {len(arr) > expected_min_count}")
print(f"mean < 200ms: {arr.mean() < 200}")
print(f"p95 < 300ms: {np.percentile(arr, 95) < 300}")
```


---


## 실습 4 (옵션): Rerun 으로 dry-run 라이브 시각화


> **요구사항**: 로컬 (맥북 등) 에 `rerun-sdk` 설치 + (실시간 보기 시) VSCode Tunnel PORTS 에서 9876 forward. 자세한 설치/사용은 [`ENVIRONMENT.md` Section 4-1](../../../ENVIRONMENT.md) 참조.
> **학습 효과**: action vector 의 "reasonable 함" 을 시각적으로 즉시 검증. week 12 의 rerun_logger ROS 노드 작성 전에 패턴을 가볍게 익혀둔다 (week 12 의 10시간 부담 경감).
> **건너뛰기 OK**: 환경 준비 안 됐으면 실습 3 의 텍스트 latency 분석으로 합격 검증 가능.


### vla_rerun_logger.py


vla_node 와 별개의 **read-only 노드** 로 토픽을 구독해 Rerun 에 로깅. 메인 inference 노드 코드는 손대지 않는다.


**파일명**: `/workspace/phase4_workspace/ros2_ws/src/vla_node/vla_node/rerun_dryrun.py`


```python
"""
실습 4 (옵션): vla_node 의 토픽을 Rerun 으로 라이브 시각화.

vla_inference_node 와 동시에 실행. 이미지 / action / latency / status 를
한 화면에서 시간축 동기화해 본다.
"""
import numpy as np
import rclpy
import rerun as rr
from cv_bridge import CvBridge
from geometry_msgs.msg import Twist
from rclpy.node import Node
from sensor_msgs.msg import Image
from std_msgs.msg import Float64, String


class RerunDryRunLogger(Node):
    def __init__(self):
        super().__init__("rerun_dryrun")

        # Rerun: gRPC 서버로 띄워 로컬 viewer 가 connect
        # (또는 rr.save("dryrun.rrd") 로 파일 저장 후 사후 분석)
        rr.init("vla_dryrun", spawn=False)
        rr.serve_grpc(grpc_port=9876)

        self.bridge = CvBridge()

        # subscribers
        self.create_subscription(Image, "/camera/image_raw", self.on_image, 1)
        self.create_subscription(Twist, "/vla/action", self.on_action, 10)
        self.create_subscription(Float64, "/vla/latency_ms", self.on_latency, 10)
        self.create_subscription(String, "/vla/status", self.on_status, 10)
        self.create_subscription(String, "/vla/instruction", self.on_instr, 1)

        self.get_logger().info("Rerun dry-run logger ready (gRPC 9876)")

    def on_image(self, msg: Image):
        img = self.bridge.imgmsg_to_cv2(msg, desired_encoding="rgb8")
        rr.log("camera/image", rr.Image(img))

    def on_action(self, msg: Twist):
        # 7-DoF action 을 한 채널로 묶어 plot
        action = np.array([
            msg.linear.x, msg.linear.y, msg.linear.z,
            msg.angular.x, msg.angular.y, msg.angular.z,
        ])
        # 축별 line chart
        for i, name in enumerate(["lin_x", "lin_y", "lin_z", "ang_x", "ang_y", "ang_z"]):
            rr.log(f"vla/action/{name}", rr.Scalars(float(action[i])))

    def on_latency(self, msg: Float64):
        rr.log("vla/latency_ms", rr.Scalars(float(msg.data)))

    def on_status(self, msg: String):
        rr.log("vla/status", rr.TextLog(msg.data))

    def on_instr(self, msg: String):
        rr.log("vla/instruction", rr.TextLog(msg.data))


def main():
    rclpy.init()
    node = RerunDryRunLogger()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
```


**setup.py entry point** 등록:


```python
'rerun_dryrun = vla_node.rerun_dryrun:main',
```


**실행 절차**:


```bash
# Terminal 1: vla_inference_node (week 11 메인 실습)
ros2 run vla_node vla_inference_node

# Terminal 2: rerun dry-run logger
ros2 run vla_node rerun_dryrun

# Terminal 3: instruction + bag 재생 (실습 2 동일)
ros2 topic pub --once /vla/instruction std_msgs/String "data: 'pick up the can'"
ros2 bag play my_test_bag --loop --clock
```


**로컬 (맥북) 에서 viewer 연결** (ENVIRONMENT.md 방법 C):


```bash
conda activate rerun
rerun --connect rerun+http://localhost:9876/proxy
```


> VSCode Tunnel PORTS 패널에서 9876 이 forwarded 되어 있어야 한다.


**확인 포인트**:
- image / action / latency 가 같은 시간축에 정렬되어 보이는가
- 1분 dry-run 동안 status 가 `ready` 만 표시되고 `error: *` 가 안 뜨는가
- action 의 6 축이 갑자기 ±크기 폭주하지 않는가 (정상 inference 면 보통 ±0.1 이내)


**저장하고 싶다면** (사후 분석용):


```python
# rerun_dryrun.py 의 init 부분 교체
rr.init("vla_dryrun", spawn=False)
# rr.serve_grpc 대신
# 노드 종료 시 atexit 으로 save 호출
import atexit
atexit.register(lambda: rr.save("dryrun.rrd"))
```


---


## 실습 5: 자작 영상 입력 (선택)


```bash
# 자작 팔 환경 1분 동영상 녹화 (스마트폰 또는 ELP)
# -> 224x224 또는 640x480 으로 resize
# -> ROS2 bag 으로 변환


# ROS2 video_stream_opencv 패키지 사용
sudo apt install ros-${ROS_DISTRO}-video-stream-opencv


ros2 launch video_stream_opencv camera.launch.py \
  video_stream_provider:=/path/to/my_video.mp4 \
  loop_videofile:=true \
  camera_name:=my_camera
```


---


## 실습 체크리스트


- [ ] vla_inference_node.py 에 실 inference 통합
- [ ] colcon build 성공
- [ ] ros2 run 실행, 모델 로드 성공
- [ ] instruction 발행 시 노드 반응
- [ ] image bag 재생 시 action publish
- [ ] 1분 dry-run 0 fail
- [ ] latency 통계 확보 (mean < 200ms)
- [ ] (옵션) rerun_dryrun 노드로 라이브 시각화 확인
- [ ] quiz_easy / quiz_medium


---


## 참고 자료


- [ros2 bag tutorial](https://docs.ros.org/en/humble/Tutorials/Beginner-CLI-Tools/Recording-And-Playing-Back-Data.html)
- [rosbag2_py for analysis](https://github.com/ros2/rosbag2/tree/humble/rosbag2_py)
- [ros2_video_streamer](https://github.com/ros-perception/video_stream_opencv)
