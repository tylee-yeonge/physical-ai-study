# Hardware-Arm Stage 2 - Teleop 입력 + 데이터 수집


> 2027.03


---


## Teleop 방식 선택


### 옵션 A: PS4 패드 (저비용, 빠름)
- 비용: ~5만 (DualShock 4)
- 입력: 2 joystick + 4 button
- 6DOF 매핑: 어색 (joystick 4 axis + L2/R2 2 axis)


### 옵션 B: Leader-follower (고비용, 자연스러움)
- 비용: ~30만 (additional Dynamixel arm)
- 사람이 leader arm 움직이면 follower 가 따라옴
- 6DOF 매핑: 자연스러움 (직관적)


본 phase 권장: **PS4 패드** (시간 절약), 시간 여유 시 leader-follower.


---


## PS4 패드 ROS2 통합


```bash
# ROS2 의 joy 패키지
sudo apt install ros-${ROS_DISTRO}-joy
ros2 run joy joy_node
# 출력: /joy topic
```


매핑 (custom 노드):
```python
class TeleopNode(Node):
    def on_joy(self, msg):
        # axes: [0]left_x, [1]left_y, [2]L2, [3]right_x, [4]right_y, [5]R2
        # 6 DOF 매핑
        joint_delta = [
            msg.axes[0] * 0.05, # joint_1 (base rotation)
            msg.axes[1] * 0.05, # joint_2 (shoulder)
            msg.axes[2] * 0.05, # joint_3 (elbow)
            msg.axes[3] * 0.05, # joint_4
            msg.axes[4] * 0.05, # joint_5
            msg.axes[5] * 0.05, # joint_6
        ]
        # Gripper: button
        gripper = 1.0 if msg.buttons[0] else 0.0
        # publish to /position_controller/commands
```


---


## 데이터 수집 파이프라인


```python
"""
collect_demos.py
"""
import rclpy
from rclpy.node import Node
import h5py
import numpy as np


class DemoCollector(Node):
    def __init__(self):
        super().__init__('demo_collector')
        self.episodes = []
        self.current_episode = []
        # subscriptions:
        # /joint_states, /camera/image_raw, /vla/instruction
        # /demo_start, /demo_end


    def on_step(self, ...):
        self.current_episode.append({
            'image': img,
            'joint_state': joints,
            'action': action,
            'instruction': instr,
        })


    def save_episode(self):
        # OpenX format HDF5
        with h5py.File(f'episode_{len(self.episodes):05d}.h5', 'w') as f:
            ...
        self.episodes.append(self.current_episode)
        self.current_episode = []
```


---


## 수집 task list


```
1. pick red cup, place on left
2. pick blue mouse, place on right
3. pick small box, place center
```


100 episodes per task = 총 300 episodes (Phase 7 week 1 의 input).


---


## 체크리스트
- [ ] PS4 패드 연결
- [ ] 6DOF 매핑
- [ ] Teleop 동작
- [ ] 데이터 수집 파이프라인
- [ ] 300 episodes 수집
- [ ] OpenX HDF5 format
