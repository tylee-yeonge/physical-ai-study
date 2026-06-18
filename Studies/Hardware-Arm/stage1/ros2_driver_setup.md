# Hardware-Arm Stage 1 - ROS2 Driver Setup


> 2026.11


---


## dynamixel_hardware 패키지 설치


```bash
# Source 설치 (ROS2)
cd ~/ros2_ws/src
git clone https://github.com/dynamixel-community/dynamixel_hardware.git
cd ..
rosdep install --from-paths src --ignore-src -r -y
colcon build --packages-select dynamixel_hardware
source install/setup.bash
```


---


## my_arm_description 패키지 생성


```bash
cd ~/ros2_ws/src
ros2 pkg create my_arm_description --build-type ament_cmake


# 디렉토리 구조
my_arm_description/
  CMakeLists.txt
  package.xml
  urdf/my_arm.urdf.xacro
  meshes/
    base.stl
    link_1.stl
    ...
  config/
    my_arm_controllers.yaml
  launch/
    display.launch.py
    bringup.launch.py
```


---


## Controller config


`config/my_arm_controllers.yaml`:


```yaml
controller_manager:
  ros__parameters:
    update_rate: 100


    joint_state_broadcaster:
      type: joint_state_broadcaster/JointStateBroadcaster


    position_controller:
      type: forward_command_controller/ForwardCommandController


position_controller:
  ros__parameters:
    joints:
      - joint_1
      - joint_2
      - joint_3
    interface_name: position
```


---


## Launch file (bringup)


`launch/bringup.launch.py`:


```python
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess


def generate_launch_description():
    return LaunchDescription([
        # Robot state publisher
        Node(package='robot_state_publisher', executable='robot_state_publisher',
             parameters=[{'robot_description': open('urdf/my_arm.urdf').read()}]),


        # ros2_control_node
        Node(package='controller_manager', executable='ros2_control_node',
             parameters=[robot_description, controllers_yaml]),


        # Controllers
        ExecuteProcess(cmd=['ros2', 'control', 'load_controller', '--set-state', 'active',
                            'joint_state_broadcaster']),
        ExecuteProcess(cmd=['ros2', 'control', 'load_controller', '--set-state', 'active',
                            'position_controller']),
    ])
```


---


## 동작 검증


```bash
# Terminal 1: Launch
ros2 launch my_arm_description bringup.launch.py


# Terminal 2: Topic 확인
ros2 topic list
# 기대: /joint_states, /position_controller/commands


# Terminal 3: 명령 발행
ros2 topic pub --once /position_controller/commands std_msgs/Float64MultiArray \
    "data: [0.5, 0.3, -0.2]"


# 모터가 움직임 확인
```


---


## 자주 발생 문제


| 증상 | 해결 |
|---|---|
| /dev/ttyUSB0 권한 | sudo chmod 666 /dev/ttyUSB0 |
| Dynamixel 검출 안 됨 | Wizard 로 baud 확인 |
| Joint 이름 mismatch | URDF 와 yaml 의 joint name 동일 |
| Position 단위 | URDF: rad, Dynamixel: position step (0.088 deg) |


---


## 체크리스트
- [ ] dynamixel_hardware 빌드 성공
- [ ] my_arm_description 패키지
- [ ] bringup.launch.py 동작
- [ ] joint_states topic publish
- [ ] position_controller 명령 -> 모터 동작
