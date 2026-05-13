# Week 6 실습: 디지털 트윈 - Real Joint State -> Sim


> **예상 시간**: 5시간


---


## 실습 1: ROS2 노드 (Isaac Sim 안)


```python
"""
practice_digital_twin.py
"""
from isaacsim import SimulationApp
sim_app = SimulationApp({"headless": False})


import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from omni.isaac.core import World
from omni.isaac.core.articulations import Articulation
import numpy as np
import threading




class DigitalTwinNode(Node):
    def __init__(self, arm):
        super().__init__('digital_twin')
        self.arm = arm
        # URDF order -> Sim order 매핑 (week 5 의 결과)
        self.dof_mapping = {0: 1, 1: 2, 2: 3, 3: 4, 4: 5, 5: 6} # 예시
        self.sub = self.create_subscription(
            JointState, '/joint_states', self.on_joint, 30)


    def on_joint(self, msg):
        positions = np.zeros(self.arm.num_dof)
        for urdf_idx, sim_idx in self.dof_mapping.items():
            positions[sim_idx] = msg.position[urdf_idx]
        self.arm.set_joint_position_targets(positions)




# Sim setup (week 4)
world = World()
# ... URDF 임포트 (week 4 결과)
arm = Articulation(prim_path="/World/my_arm")
world.reset()
arm.initialize()


# ROS2 init
rclpy.init()
node = DigitalTwinNode(arm)


# Thread 분리 (ROS2 callback)
ros_thread = threading.Thread(target=lambda: rclpy.spin(node), daemon=True)
ros_thread.start()


# Sim main loop
while sim_app.is_running():
    world.step(render=True)


node.destroy_node()
rclpy.shutdown()
sim_app.close()
```


다른 터미널:
```bash
# Real robot 의 joint_states publish (또는 mock)
ros2 topic pub /joint_states sensor_msgs/JointState ...
```


---


## 실습 2: 매칭 정확도 측정


```python
"""
measurement of Real <-> Sim 매칭
"""
import numpy as np


real_angles = np.array([0.5, 0.3, -0.2, 0.0, 0.1, 0.0])
sim_angles = arm.get_joint_positions() # 매칭된 후


joint_err = np.abs(real_angles - sim_angles).max()
print(f"Max joint error: {joint_err:.4f} rad = {np.degrees(joint_err):.2f} deg")


# ee 위치
real_ee = compute_fk(real_angles)
sim_ee = arm.get_world_pose("/World/my_arm/ee_link")[0]
ee_err = np.linalg.norm(real_ee - sim_ee)
print(f"EE error: {ee_err*1000:.2f} mm")
```


---


## 실습 3: side-by-side 영상


```python
# Real robot 카메라 image + Sim image 를 side-by-side
# Phase 7 산출물 #4 의 한 컨텐츠
```


---


## 체크리스트
- [ ] digital_twin_node 동작
- [ ] Real <-> Sim 매칭 < 0.5 deg / 5 mm
- [ ] (선택) side-by-side 영상
- [ ] quiz
