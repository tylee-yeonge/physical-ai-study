# Hardware-Arm Stage 1 - Isaac Sim URDF 임포트


> 2026.12


---


## Stage 1 의 Isaac Sim 임포트


Phase 6 (2027.02~) 의 디지털 트윈의 첫 사이클.
본 단계는 **단순 임포트 + Joint 동작 확인** 만.


---


## 임포트 방법


```python
"""
practice_import_my_arm.py
"""
from isaacsim import SimulationApp
sim_app = SimulationApp({"headless": False})


from omni.importer.urdf import _urdf
import os


urdf_path = os.path.expanduser("~/ros2_ws/src/my_arm_description/urdf/my_arm.urdf")


cfg = _urdf.ImportConfig()
cfg.merge_fixed_joints = False
cfg.fix_base = True
cfg.import_inertia_tensor = True
cfg.distance_scale = 1.0


status, stage_path = _urdf.parse_and_import_urdf(urdf_path, cfg)
print(f"Imported to: {stage_path}")


from omni.isaac.core import World
from omni.isaac.core.articulations import Articulation


world = World()
arm = Articulation(prim_path=stage_path)
world.reset()
arm.initialize()
print(f"DOF: {arm.num_dof}")
print(f"Joint names: {arm.dof_names}")


# Simple motion
import numpy as np
target = np.array([0.3, 0.2, -0.1]) # 3 DOF
arm.set_joint_position_targets(target)


for _ in range(200):
    world.step(render=True)


sim_app.close()
```


---


## 검증 항목


```
- [ ] URDF 임포트 성공
- [ ] mesh 가 사실적으로 보임
- [ ] 각 joint 가 동작
- [ ] Sim 의 joint 가 URDF 의 limit 안에서
- [ ] 충돌 가짐 (필요 시)
```


---


## Sim Joint <-> Real Joint 매핑


```python
# Sim 의 dof_names
print(arm.dof_names)
# 예: ['joint_1', 'joint_2', 'joint_3']


# ROS2 의 /joint_states 의 name 과 일치 확인
# 다르면 매핑 표 작성 (Phase 6 week 5)
```


---


## 산출물 #2.5 의 영상 capture


```python
from omni.isaac.sensor import Camera
camera = Camera(prim_path="/World/Camera", position=[0.5, 0.5, 0.5], ...)
camera.initialize()


import imageio
frames = []
for i in range(300):
    world.step(render=True)
    if i % 5 == 0:
        frames.append(camera.get_rgba()[:,:,:3].astype('uint8'))


imageio.mimsave('stage1_demo.mp4', frames, fps=20)
```


영상: 60초 (Phase 4 week 12 와 같이).


---


## 다음 Stage (2027.02~)


Stage 2 의 6DOF 확장 시 본 가이드 그대로 + 추가:
- 6 joints
- teleop 통합
- 안전 인터록


---


## 체크리스트 (Stage 1 완료)


- [ ] Isaac Sim 임포트 성공
- [ ] joint 동작 확인
- [ ] 영상 capture
- [ ] 산출물 #2.5 디렉토리 정리
- [ ] git tag stage1
