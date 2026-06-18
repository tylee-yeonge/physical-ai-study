# Week 10 실습: vla_node 패키지 생성 + 골격 노드 + 빌드/실행


> **실습 목표**: 빈 동작 (dummy) 의 vla_node 가 colcon build + ros2 run 으로 정상 동작.
> **예상 시간**: 6-8시간


---


## 환경 설정


이번 주는 ROS2 패키지 골격만 만들고 inference 는 다음 주 (week 11) 에 통합한다. 로컬 GPU 는 필요 없음.

기본 환경은 [`../SETUP.md`](../SETUP.md) §6.1 (디렉토리 구조), §6.3 (ROS2 환경) 을 따른다. lifecycle node 로 작성한다는 결정도 SETUP.md §6.3 참고.


```bash
# ROS2 Humble 또는 Jazzy
source /opt/ros/humble/setup.bash


# colcon + 의존성
sudo apt install python3-colcon-common-extensions ros-humble-cv-bridge


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


> 주의: `image_callback` 은 instruction 이 설정되고 이미지가 들어와야만 action 을 발행한다 (instruction 없으면 early return). 따라서 action echo 를 보려면 instruction 발행과 **별개로 이미지 스트림이 필요**하다. 실습 5 의 image_publisher 가 "(선택)" 으로 표기돼 있지만 실습 4 의 action 검증에는 사실상 필수다 — 실습 5 를 먼저 띄우거나 `ros2 bag play` 로 카메라 토픽을 공급한 뒤 echo 를 확인한다.


---


## 실습 5: image_publisher 로 dummy image 발행 (선택)


```bash
# 패키지 설치
sudo apt install ros-humble-image-publisher


# 실행
ros2 run image_publisher image_publisher_node /path/to/test.jpg --ros-args -r image_raw:=/camera/image_raw
```


또는 직접 publish 스크립트:


**파일명**: `/workspace/phase4_workspace/pub_test/practice_image_publisher.py`


```python
import cv2
import numpy as np
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge




class FakeImagePub(Node):
    def __init__(self):
        super().__init__('fake_image_pub')
        self.pub = self.create_publisher(Image, '/camera/image_raw', 1)
        self.bridge = CvBridge()
        self.timer = self.create_timer(1.0 / 30, self.publish)


    def publish(self):
        img = (np.random.rand(480, 640, 3) * 255).astype(np.uint8)
        msg = self.bridge.cv2_to_imgmsg(img, encoding='bgr8')
        msg.header.frame_id = 'camera_link'
        msg.header.stamp = self.get_clock().now().to_msg()
        self.pub.publish(msg)




def main():
    rclpy.init()
    node = FakeImagePub()
    rclpy.spin(node)




if __name__ == '__main__':
    main()
```


---


## 실습 체크리스트


- [ ] ros2 pkg create 성공
- [ ] vla_inference_node.py 작성 완료
- [ ] setup.py 의 entry_points 추가
- [ ] colcon build --packages-select vla_node 성공
- [ ] ros2 run vla_node vla_inference_node 실행
- [ ] ros2 topic list 에서 /vla/* 보임
- [ ] ros2 topic echo /vla/action 가 0.0 0.0 ... publish 확인
- [ ] git commit (ROS2 workspace 의 vla_node 만)
- [ ] quiz_easy / quiz_medium


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
| `cv_bridge ImportError` | sudo apt install ros-humble-cv-bridge |
| `Image topic 안 나옴` | image_publisher 또는 fake publisher 실행 |
| `Build fail with --packages-select` | colcon build (전체) 한 번 |
