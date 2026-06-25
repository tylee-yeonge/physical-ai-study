# Week 10 실습: vla_node 패키지 생성 + 골격 노드 + 빌드/실행


> **실습 목표**: 빈 동작 (dummy) 의 vla_node 가 colcon build + ros2 run 으로 정상 동작.
> **예상 시간**: 6-8시간


---


## 환경 설정


이번 주는 ROS2 패키지 골격만 만들고 inference 는 다음 주 (week 11) 에 통합한다. 로컬 GPU 는 필요 없음.

기본 환경은 [`../SETUP.md`](../SETUP.md) §6.1 (디렉토리 구조), §6.3 (ROS2 환경) 을 따른다. lifecycle node 로 작성한다는 결정도 SETUP.md §6.3 참고.


```bash
# ROS2 (현재 distro 에 맞춰 source)
source /opt/ros/${ROS_DISTRO}/setup.bash


# colcon + 의존성
sudo apt install python3-colcon-common-extensions ros-${ROS_DISTRO}-cv-bridge


# ROS2 workspace (SETUP.md §6.1 의 /workspace/phase4_workspace/ros2_ws/ 사용)
mkdir -p /workspace/phase4_workspace/ros2_ws/src
cd /workspace/phase4_workspace/ros2_ws
```


---


## 실습 1: 패키지 생성


```bash
cd /workspace/phase4_workspace/ros2_ws/src
ros2 pkg create vla_node \
  --build-type ament_python \
  --dependencies rclpy sensor_msgs geometry_msgs std_msgs cv_bridge


cd vla_node
tree
```


기대 출력:
```
vla_node/
  package.xml
  setup.py
  setup.cfg
  resource/
    vla_node
  vla_node/
    __init__.py
  test/
    test_copyright.py
    test_flake8.py
    test_pep257.py
```


---


## 실습 2: vla_inference_node.py 골격


**파일명**: `/workspace/phase4_workspace/ros2_ws/src/vla_node/vla_node/vla_inference_node.py`


```python
"""
vla_node: OpenVLA inference -> ROS2 topic
Week 10: dummy zero-action 발행 (실제 inference 는 week 11 에서 연결)
"""
import time


import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSDurabilityPolicy


from sensor_msgs.msg import Image
from geometry_msgs.msg import Twist
from std_msgs.msg import String, Float64




class VLANode(Node):
    def __init__(self):
        super().__init__('vla_node')


        # -- Parameters --
        self.declare_parameter('model_id', 'openvla/openvla-7b')
        self.declare_parameter('quant_type', '4bit')
        self.declare_parameter('unnormalize_key', 'bridge_orig')
        self.declare_parameter('image_topic', '/camera/image_raw')
        self.declare_parameter('instruction_topic', '/vla/instruction')
        self.declare_parameter('action_topic', '/vla/action')
        self.declare_parameter('gripper_topic', '/vla/gripper')
        self.declare_parameter('latency_topic', '/vla/latency_ms')


        image_topic = self.get_parameter('image_topic').get_parameter_value().string_value
        instr_topic = self.get_parameter('instruction_topic').get_parameter_value().string_value
        action_topic = self.get_parameter('action_topic').get_parameter_value().string_value
        gripper_topic = self.get_parameter('gripper_topic').get_parameter_value().string_value
        latency_topic = self.get_parameter('latency_topic').get_parameter_value().string_value


        # -- QoS --
        image_qos = QoSProfile(
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
            depth=1,
        )
        action_qos = QoSProfile(
            reliability=QoSReliabilityPolicy.RELIABLE,
            depth=10,
        )
        instr_qos = QoSProfile(
            reliability=QoSReliabilityPolicy.RELIABLE,
            durability=QoSDurabilityPolicy.TRANSIENT_LOCAL,
            depth=1,
        )


        # -- Subscribers --
        self.image_sub = self.create_subscription(
            Image, image_topic, self.image_callback, image_qos,
        )
        self.instr_sub = self.create_subscription(
            String, instr_topic, self.instruction_callback, instr_qos,
        )


        # -- Publishers --
        self.action_pub = self.create_publisher(Twist, action_topic, action_qos)
        self.gripper_pub = self.create_publisher(Float64, gripper_topic, action_qos)
        self.latency_pub = self.create_publisher(Float64, latency_topic, 10)


        # -- State --
        self.current_instruction = ""
        self.image_count = 0


        self.get_logger().info('VLANode initialized (dummy mode)')
        self.get_logger().info(f' image_topic: {image_topic}')
        self.get_logger().info(f' action_topic: {action_topic}')


    def instruction_callback(self, msg: String):
        self.current_instruction = msg.data
        self.get_logger().info(f'New instruction: {self.current_instruction}')


    def image_callback(self, msg: Image):
        self.image_count += 1
        if self.image_count % 30 == 0:
            self.get_logger().debug(f'Received {self.image_count} images')


        if not self.current_instruction:
            return


        # -- DUMMY ---
        # week 10 에서는 zero action 발행
        # week 11 에서 실제 VLAInference.predict() 연결
        t0 = time.time()
        twist = Twist()
        # zero (정지)
        twist.linear.x = 0.0
        twist.linear.y = 0.0
        twist.linear.z = 0.0
        twist.angular.x = 0.0
        twist.angular.y = 0.0
        twist.angular.z = 0.0
        latency_ms = (time.time() - t0) * 1000


        self.action_pub.publish(twist)
        gripper_msg = Float64()
        gripper_msg.data = 0.0
        self.gripper_pub.publish(gripper_msg)


        lat_msg = Float64()
        lat_msg.data = latency_ms
        self.latency_pub.publish(lat_msg)




def main(args=None):
    rclpy.init(args=args)
    node = VLANode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('Shutdown')
    finally:
        node.destroy_node()
        rclpy.shutdown()




if __name__ == '__main__':
    main()
```


---


## 실습 3: setup.py 수정


**파일명**: `/workspace/phase4_workspace/ros2_ws/src/vla_node/setup.py`


```python
from setuptools import setup


package_name = 'vla_node'


setup(
    name=package_name,
    version='0.1.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Your Name',
    maintainer_email='you@example.com',
    description='OpenVLA inference ROS2 node',
    license='MIT',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'vla_inference_node = vla_node.vla_inference_node:main',
        ],
    },
)
```


---


## 실습 4: 빌드 + 실행


```bash
# 빌드
cd /workspace/phase4_workspace/ros2_ws
colcon build --packages-select vla_node --symlink-install


# source
source install/setup.bash


# 노드 실행
ros2 run vla_node vla_inference_node
```


다른 터미널에서:


```bash
# topic 목록 확인
ros2 topic list
# 기대 출력:
# /camera/image_raw (subscribe)
# /vla/action
# /vla/gripper
# /vla/instruction (subscribe)
# /vla/latency_ms


# instruction 발행 (latch)
ros2 topic pub --once /vla/instruction std_msgs/String "data: 'pick up the can'"


# image 발행 (dummy, image_publisher 패키지 사용 또는 직접 publish)
# 또는 ros2 bag play 로 카메라 bag 재생


# action 출력 확인
ros2 topic echo /vla/action
# 기대: linear/angular 모두 0.0 (dummy)
```


> 주의: `image_callback` 은 instruction 이 설정되고 이미지가 들어와야만 action 을 발행한다 (instruction 없으면 early return). 따라서 action echo 를 보려면 instruction 발행과 **별개로 이미지 스트림이 필요**하다 — 실습 5 의 카메라 publisher 를 먼저 띄우거나 `ros2 bag play` 로 카메라 토픽을 공급한 뒤 echo 를 확인한다.


---


## 실습 5: USB 카메라로 image 발행

`/camera/image_raw` 에 이미지를 공급하는 방법은 두 가지다.

### 방법 A: image_publisher 패키지 (정지 이미지)

```bash
sudo apt install ros-${ROS_DISTRO}-image-publisher

ros2 run image_publisher image_publisher_node /path/to/test.jpg --ros-args -r image_raw:=/camera/image_raw
```

### 방법 B: USB 카메라 단독 발행 스크립트

연결된 `/dev/video*` 를 순서대로 열어 **실제로 스트리밍되는 첫 카메라 한 대**를 선택해 `/camera/image_raw` 로 발행한다.

`/dev/video1` 처럼 메타데이터 전용 노드는 한두 번 `read()` 후 실패한다. `read()` 1회로는 진짜 카메라와 구분되지 않으므로(가짜 양성으로 잡힘), **연속 5회 read 성공**을 요구해 메타데이터/중복 노드를 걸러낸다. 선택한 카메라는 닫지 않고 그대로 노드에 넘겨서, 재오픈 시 발생하는 V4L2 `index out of range` 실패를 피한다.

colcon 빌드 없이 python3 로 직접 실행하는 단독 스크립트다.

**파일명**: `/workspace/phase4_workspace/pub_test/practice_image_publisher.py`

```python
"""USB 카메라 한 대의 프레임을 /camera/image_raw 로 발행하는 단독 실행 스크립트.

colcon 패키지에 넣지 않고 python3 로 직접 실행한다:

    source /opt/ros/${ROS_DISTRO}/setup.bash
    source /workspace/phase4_workspace/.venv-vla/bin/activate
    python3 /workspace/phase4_workspace/pub_test/practice_image_publisher.py

동작:
1. 연결된 /dev/video* 를 순서대로 열어, 실제로 연속 프레임이 읽히는 첫 카메라를 선택
2. 30Hz 로 BGR 프레임을 읽어 sensor_msgs/Image 로 변환해 /camera/image_raw 에 발행
3. QoS 는 BEST_EFFORT(sensor data) 라 vla_inference_node 의 image 구독자와 호환됨
"""

import glob

import cv2
import rclpy
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import Image
from cv_bridge import CvBridge


def open_camera():
    """연결된 /dev/video* 중 실제로 스트리밍 가능한 첫 카메라를 열어 반환한다.

    /dev/video1 처럼 메타데이터 전용 노드는 한두 번 read 후 실패하므로,
    한 번 read 로는 진짜 카메라와 구분되지 않는다. 여러 번 연속 read 성공을
    요구해 메타데이터/중복 노드를 걸러낸다. 선택된 카메라는 열린 채로 반환해
    (재오픈 시 V4L2 의 'index out of range' 실패를 피한다).

    Returns:
        열려서 스트리밍이 확인된 cv2.VideoCapture. 사용 가능한 카메라가 없으면 None.
    """
    # /dev/video0, /dev/video1 ... 형태의 장치 노드를 번호 순으로 수집
    devices = sorted(glob.glob("/dev/video*"))
    for dev in devices:
        # "/dev/video0" -> 0 처럼 끝 숫자만 OpenCV 인덱스로 추출
        index = int(dev.replace("/dev/video", ""))
        # 해당 인덱스 카메라 열기 시도
        cap = cv2.VideoCapture(index)
        if not cap.isOpened():
            # 열리지 않으면 자원 해제 후 다음 장치로
            cap.release()
            continue
        # 5회 연속 프레임 읽기로 실제 스트리밍 여부 확인 (메타데이터 노드 걸러내기)
        streamable = True
        for _ in range(5):
            ok, frame = cap.read()
            if not ok or frame is None or frame.size == 0:
                streamable = False
                break
        if streamable:
            # 사용할 카메라는 닫지 않고 그대로 반환
            print(f"카메라 선택: /dev/video{index}")
            return cap
        # 가짜 양성(메타데이터 노드 등)은 해제하고 다음 후보 탐색
        cap.release()
    return None


class UsbCameraPublisher(Node):
    """USB 카메라 한 대의 프레임을 /camera/image_raw 로 발행하는 노드."""

    def __init__(self, cap):
        """노드를 초기화한다.

        Args:
            cap: 이미 열려 스트리밍이 확인된 cv2.VideoCapture.
        """
        super().__init__("usb_camera_publisher")

        # open_camera 에서 열어 넘겨받은 카메라 핸들
        self.cap = cap
        # OpenCV numpy(BGR) 프레임을 sensor_msgs/Image 로 변환하는 도구
        self.bridge = CvBridge()

        # vla_inference_node 의 image 구독 토픽과 동일하게 맞춘다
        self.publisher = self.create_publisher(
            Image, "/camera/image_raw", qos_profile_sensor_data
        )

        # 30Hz 주기 타이머 (약 0.0333초마다 timer_callback 호출)
        self.timer = self.create_timer(1.0 / 30.0, self.timer_callback)
        self.get_logger().info("USB 카메라 발행 시작 -> /camera/image_raw")

    def timer_callback(self):
        """카메라에서 한 프레임을 읽어 Image 메시지로 발행한다."""
        # 카메라에서 BGR 프레임 한 장 읽기 (ok=성공 여부, frame=numpy 배열)
        ok, frame = self.cap.read()
        if not ok:
            # 일시적 읽기 실패는 건너뛰고 다음 주기에 재시도
            self.get_logger().warn("프레임 읽기 실패, 이번 주기 건너뜀")
            return

        # numpy(BGR) -> sensor_msgs/Image. OpenCV 기본 채널 순서라 encoding 은 bgr8
        msg = self.bridge.cv2_to_imgmsg(frame, encoding="bgr8")
        # 헤더에 현재 ROS 시각과 프레임 좌표계 이름 채우기
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = "camera"

        self.publisher.publish(msg)

    def destroy_node(self):
        """노드 종료 시 카메라 자원을 해제한다."""
        # 카메라 핸들이 열려 있으면 반드시 release (장치 점유 해제)
        if self.cap is not None:
            self.cap.release()
        super().destroy_node()


def main(args=None):
    """카메라를 열어 발행 노드를 실행한다."""
    rclpy.init(args=args)

    # 스트리밍 가능한 카메라 한 대 열기
    cap = open_camera()
    if cap is None:
        print("사용 가능한 USB 카메라(/dev/video*)를 찾지 못했습니다.")
        rclpy.shutdown()
        return

    node = UsbCameraPublisher(cap)
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        # Ctrl-C 종료는 정상 흐름으로 처리
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
```

실행:

```bash
source /opt/ros/${ROS_DISTRO}/setup.bash
source /workspace/phase4_workspace/.venv-vla/bin/activate
python3 /workspace/phase4_workspace/pub_test/practice_image_publisher.py
```

발행 토픽 확인:

```bash
ros2 topic list | grep camera
# /camera/image_raw 하나가 보이면 정상
```


---


## 실습 체크리스트


- [x] ros2 pkg create 성공
- [x] vla_inference_node.py 작성 완료
- [x] setup.py 의 entry_points 추가
- [x] colcon build --packages-select vla_node 성공
- [x] ros2 run vla_node vla_inference_node 실행
- [x] ros2 topic list 에서 /vla/* 보임
- [x] ros2 topic echo /vla/action 가 0.0 0.0 ... publish 확인
- [x] git commit (ROS2 workspace 의 vla_node 만)
- [x] quiz_easy / quiz_medium


---


## 참고 자료


- [ROS2 Humble: Creating a package](https://docs.ros.org/en/humble/Tutorials/Beginner-Client-Libraries/Creating-Your-First-ROS2-Package.html)
- [ROS2 Python publisher / subscriber](https://docs.ros.org/en/humble/Tutorials/Beginner-Client-Libraries/Writing-A-Simple-Py-Publisher-And-Subscriber.html)
- [QoS in Python](https://docs.ros.org/en/humble/Tutorials/Demos/Quality-of-Service.html)


---


## 트러블슈팅


| 증상 | 해결 |
|---|---|
| `Package 'vla_node' not found` | source install/setup.bash 다시 |
| `No executable 'vla_inference_node'` | setup.py 의 entry_points 확인 |
| `cv_bridge ImportError` | sudo apt install ros-${ROS_DISTRO}-cv-bridge |
| `Image topic 안 나옴` | 실습 5 의 USB 카메라 publisher 또는 image_publisher 실행 |
| `Build fail with --packages-select` | colcon build (전체) 한 번 |
