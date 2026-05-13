# Week 10: ROS2 패키지 골격 작성 (vla_node)


> **이번 주 목표**: `vla_node` 라는 ROS2 ament_python 패키지를 0 부터 만들어 빌드까지 성공. callback / publisher / subscriber 의 골격 완성. 아직 inference 실제 연결 안 함 (dummy 동작).
> **예상 시간**: 10시간 (패키지 setup 3h + 노드 코드 4h + 빌드/실행 3h)
> **핵심 질문**: "내 노드가 빈 동작 (dummy zero action publish) 으로 ROS2 graph 안에서 정상 동작하는가? 빌드/실행 에러 없이 colcon build 통과하는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | ROS2 환경 점검 | bash | ros2 doctor, colcon |
| 2 | 패키지 생성 | `PRACTICE.md` 1 | ros2 pkg create |
| 3 | 노드 골격 작성 | `PRACTICE.md` 2 | VLANode class, callback 들 |
| 4 | setup.py / package.xml | `PRACTICE.md` 3 | entry point + 의존성 |
| 5 | 빌드 + 실행 | `PRACTICE.md` 4 | colcon build + ros2 run |
| 6 | 퀴즈 | quiz_easy / quiz_medium | 패키지 구조 / lifecycle |


---


## 시작하기 전에


이번 주는 inference 와는 무관. ROS2 패키지의 **골격만** 만든다. 다음 주 (week 11) 에 골격에 실제 inference 를 연결.


---


## 핵심 개념


### 1. ROS2 ament_python 패키지의 구조


```
vla_node/
  package.xml # 의존성, license, maintainer
  setup.py # entry_point (실행 가능한 노드)
  setup.cfg # build 설정
  resource/
    vla_node # marker file (빈 파일)
  vla_node/
    __init__.py
    vla_inference_node.py # 실제 노드 코드
    publisher.py # (선택) publisher 추상화
    subscriber.py # (선택) subscriber 추상화
  test/
    test_vla_inference_node.py
```


### 2. `ros2 pkg create` 명령


```bash
cd ~/ros2_ws/src
ros2 pkg create vla_node \
  --build-type ament_python \
  --dependencies rclpy sensor_msgs geometry_msgs std_msgs cv_bridge
```


이 명령이 위 구조의 골격을 자동 생성.


### 3. setup.py 의 entry_point


```python
setup(
    ...
    entry_points={
        'console_scripts': [
            'vla_inference_node = vla_node.vla_inference_node:main',
        ],
    },
)
```


이 한 줄로 `ros2 run vla_node vla_inference_node` 명령 사용 가능.


### 4. 노드 코드의 골격


```python
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from geometry_msgs.msg import Twist
from std_msgs.msg import String, Float64


class VLANode(Node):
    def __init__(self):
        super().__init__('vla_node')


        self.declare_parameter('model_id', 'openvla/openvla-7b')
        self.declare_parameter('image_topic', '/camera/image_raw')
        self.declare_parameter('action_topic', '/vla/action')
        # ... etc


        self.image_sub = self.create_subscription(
            Image, '/camera/image_raw', self.image_callback, 1,
        )
        self.instruction_sub = self.create_subscription(
            String, '/vla/instruction', self.instruction_callback, 1,
        )
        self.action_pub = self.create_publisher(Twist, '/vla/action', 10)
        self.gripper_pub = self.create_publisher(Float64, '/vla/gripper', 10)
        self.latency_pub = self.create_publisher(Float64, '/vla/latency_ms', 10)


        self.current_instruction = ""
        self.get_logger().info('VLANode initialized')


    def image_callback(self, msg):
        if not self.current_instruction:
            self.get_logger().debug('No instruction, skipping')
            return
        # week 10 (이번 주): dummy zero action 발행
        twist = Twist()
        # week 11 부터 실제 inference 연결
        self.action_pub.publish(twist)
        ...


    def instruction_callback(self, msg):
        self.current_instruction = msg.data
        self.get_logger().info(f'New instruction: {self.current_instruction}')


def main():
    rclpy.init()
    node = VLANode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
```


### 5. Parameter declaration


ROS2 parameter 로 hyper-param 노출:


```python
self.declare_parameter('model_id', 'openvla/openvla-7b')
self.declare_parameter('quant_type', '4bit')
self.declare_parameter('unnormalize_key', 'bridge_orig')
self.declare_parameter('device', 'cuda:0')
self.declare_parameter('image_age_threshold_ms', 200.0)
```


조회:


```python
model_id = self.get_parameter('model_id').get_parameter_value().string_value
```


### 6. Logging 표준


```python
self.get_logger().debug('Detailed info')
self.get_logger().info('Important state')
self.get_logger().warning('Recoverable issue')
self.get_logger().error('Major issue')
self.get_logger().fatal('Cannot continue')
```


### 7. 빌드 + 실행 워크플로우


```bash
# 빌드
cd ~/ros2_ws
colcon build --packages-select vla_node --symlink-install


# source
source install/setup.bash


# 실행
ros2 run vla_node vla_inference_node


# 다른 터미널에서 확인
ros2 topic list
ros2 topic echo /vla/action
ros2 topic pub --once /vla/instruction std_msgs/String "data: 'pick up the can'"
```


### 8. Launch file (선택)


```python
# launch/vla_demo.launch.py
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='vla_node',
            executable='vla_inference_node',
            name='vla_node',
            parameters=[{
                'model_id': 'openvla/openvla-7b',
                'quant_type': '4bit',
            }],
        ),
    ])
```


---


## 자체 점검


**Q1. ament_python 의 entry_point 의 의미는?**
> setup.py 의 console_scripts. `ros2 run <pkg> <exec>` 가 호출할 Python 함수 등록.


**Q2. 빌드 시 `--symlink-install` 의 효과는?**
> source 파일을 install 디렉토리로 복사 안 하고 symlink. 코드 수정 후 재빌드 불필요 (Python 한정).


**Q3. parameter 와 환경 변수의 차이는?**
> parameter: ROS2 graph 안에서 동적 변경 가능 (`ros2 param set`). 환경 변수: 프로세스 시작 시점 고정. parameter 가 더 유연.


**Q4. image_callback 이 비동기로 자주 호출되는데 instruction 이 비어있을 때 어떻게?**
> 그냥 return (skip). instruction 없이는 inference 무의미.


**Q5. 다른 노드가 instruction publish 안 하면 어떻게 테스트?**
> `ros2 topic pub` 으로 직접 명령어 발행. transient_local QoS 면 latch 되어 새 노드가 join 해도 받음.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. ROS2 workspace 셋업 (없으면)
2. `vla_node` 패키지 생성 (ros2 pkg create)
3. `vla_inference_node.py` 골격 작성 (dummy 동작)
4. colcon build 성공
5. ros2 run 으로 노드 실행
6. ros2 topic echo / pub 로 확인
7. quiz_easy / quiz_medium


### 다음 주 (week 11) 준비
- week 8 의 vla_inference 패키지를 ros2 workspace 에서 import 가능하게 정리
- 이번 주 dummy 코드를 실제 inference 호출로 교체할 계획


---


## 이번 주 핵심 요약


1. **ROS2 ament_python 패키지** 표준 구조.
2. **entry_point** 로 `ros2 run` 호환.
3. **parameter** 로 hyper-param 분리.
4. **QoS** 적절히 (week 9 의 spec 그대로).
5. **dummy 빈 동작** 으로 ROS2 graph 정상 통합 확인.


---


- 이전: [Week 9 - I/O 인터페이스 정리](../week9/README.md)


다음: [Week 11 - image subscribe -> inference -> action publish](../week11/README.md)
