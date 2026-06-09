# Week 4 실습: 자작 팔 URDF 임포트


> **예상 시간**: 5시간


---


## 실습 1: URDF 검토


자작 팔 URDF (Hardware-Arm Stage 1 의 결과) 의 file:


```bash
# 위치
~/ros2_ws/src/my_arm_description/urdf/my_arm.urdf


# 검토
xacro my_arm.urdf.xacro > my_arm_check.urdf
diff my_arm.urdf my_arm_check.urdf


# Visualization (RViz2)
ros2 launch urdf_tutorial display.launch.py model:=my_arm.urdf
```


---


## 실습 2: URDF Importer (Python API)


```python
"""
practice_urdf_import.py
"""
from isaacsim import SimulationApp
sim_app = SimulationApp({"headless": False})


from omni.importer.urdf import _urdf
import os


urdf_path = os.path.expanduser("~/ros2_ws/src/my_arm_description/urdf/my_arm.urdf")


import_config = _urdf.ImportConfig()
import_config.merge_fixed_joints = False
import_config.convex_decomp = False
import_config.import_inertia_tensor = True
import_config.fix_base = True
import_config.distance_scale = 1.0


status, stage_path = _urdf.parse_and_import_urdf(urdf_path, import_config)
print(f"Imported to: {stage_path}") # /World/my_arm


# Articulation 으로 동작
from omni.isaac.core import World
from omni.isaac.core.articulations import Articulation


world = World()
arm = Articulation(prim_path="/World/my_arm")


world.reset()
arm.initialize()


print(f"DOF: {arm.num_dof}")
print(f"Joint names: {arm.dof_names}")
print(f"Initial angles: {arm.get_joint_positions()}")


# 100 step
for i in range(100):
    world.step(render=True)


sim_app.close()
```


---


## 실습 3: Joint 동작 시뮬레이션


```python
"""
practice_joint_motion.py
- 임포트된 robot 의 joint 를 sin wave 로 움직임
"""
import numpy as np
# ... (위 setup 그대로)


import math
n_dof = arm.num_dof
amplitude = 0.3 # 0.3 rad
freq = 0.5 # 0.5 Hz


for i in range(500):
    t = i / 60.0 # 60 FPS 가정
    target = np.array([amplitude * math.sin(2 * math.pi * freq * t + j) for j in range(n_dof)])
    arm.set_joint_positions(target)
    world.step(render=True)
```


각 joint 가 sin wave 로 움직이는 것 확인.


---


## 실습 4: 결과 영상 capture


```python
# Camera 추가
from omni.isaac.sensor import Camera
camera = Camera(prim_path="/World/Camera", position=[1.0, 0.5, 0.7], ...)
camera.initialize()


# 매 step 마다 image 저장
imgs = []
for i in range(300):
    world.step(render=True)
    if i % 5 == 0:
        imgs.append(camera.get_rgba()[:, :, :3])


# 영상으로 저장 (cv2 또는 imageio)
import imageio
imageio.mimsave('arm_motion.mp4', imgs, fps=20)
```


이게 Phase 7 산출물 v3 영상의 Sim 부분.


---


## 체크리스트
- [ ] URDF 검토
- [ ] Isaac Sim 임포트 성공
- [ ] joint 동작 확인
- [ ] (선택) 영상 capture
- [ ] quiz
