# Week 2 실습: Camera + Light + Material


> **예상 시간**: 4시간


---


## 실습 1: Camera + 첫 image capture


```python
"""
practice_camera.py
"""
from isaacsim import SimulationApp
sim_app = SimulationApp({"headless": True})


import numpy as np
from omni.isaac.core import World
from omni.isaac.core.objects import DynamicCuboid
from omni.isaac.sensor import Camera


world = World()
world.scene.add_default_ground_plane()
world.scene.add(DynamicCuboid(prim_path="/World/cube", position=[0, 0, 0.5], scale=[0.3]*3, color=[1, 0, 0]))


# Camera 추가
camera = Camera(
    prim_path="/World/Camera",
    position=np.array([1.0, 0.0, 0.5]),
    orientation=np.array([0.7071, 0, 0, 0.7071]), # look at origin
    resolution=(640, 480),
)
camera.initialize()
camera.set_focal_length(15.0)


world.reset()


# 10 step
for i in range(10):
    world.step(render=True)


# Image capture
img = camera.get_rgba()[:, :, :3] # (480, 640, 3)
print(f"Image shape: {img.shape}, dtype: {img.dtype}")


# 저장
from PIL import Image
Image.fromarray(img.astype('uint8')).save('isaac_capture.png')
print("saved: isaac_capture.png")


sim_app.close()
```


---


## 실습 2: Light + Material


```python
"""
practice_light_material.py
"""
from isaacsim import SimulationApp
sim_app = SimulationApp({"headless": True})


import numpy as np
from omni.isaac.core import World
from omni.isaac.core.objects import DynamicCuboid
from omni.isaac.core.utils.prims import create_prim
from omni.isaac.core.materials import OmniPBR


world = World()
world.scene.add_default_ground_plane()


cube = world.scene.add(DynamicCuboid(prim_path="/World/cube", scale=[0.3]*3, position=[0, 0, 0.5]))


# Light
create_prim(
    "/World/MainLight", "DistantLight",
    attributes={"intensity": 3000, "color": (1.0, 0.95, 0.8)}
)


# Material
red_mat = OmniPBR("/Materials/red",
                   color=np.array([1.0, 0.0, 0.0]),
                   metallic=0.3, roughness=0.5)
cube.apply_visual_material(red_mat)


world.reset()
for _ in range(10):
    world.step(render=True)
print("done")
sim_app.close()
```


---


## 실습 3: Stage 저장 / 로드


```python
from omni.isaac.core.utils.stage import save_stage, open_stage


# 저장
save_stage("my_scene.usd")


# 로드
open_stage("my_scene.usd")
```


---


## 체크리스트
- [ ] Camera image capture 성공
- [ ] Light + Material 적용
- [ ] Stage 저장/로드
- [ ] quiz
