# Week 3 실습: ROS2 Bridge 통합

> [time] **예상 시간**: 5시간

---

## [note] 실습 1: Bridge 활성화 + 첫 publish

```python
"""
practice_bridge_basic.py
"""
from isaacsim import SimulationApp
sim_app = SimulationApp({"headless": False, "renderer": "RayTracedLighting"})

# Bridge 활성화
import omni.kit.app
m = omni.kit.app.get_app().get_extension_manager()
m.set_extension_enabled_immediate("omni.isaac.ros2_bridge", True)

import numpy as np
from omni.isaac.core import World

world = World()
world.scene.add_default_ground_plane()

# Articulation 추가 (간단 robot - 또는 cube)
from omni.isaac.core.objects import DynamicCuboid
cube = world.scene.add(DynamicCuboid(prim_path="/World/cube", scale=[0.3]*3))

world.reset()

# Action Graph (Python API)
import omni.graph.core as og

og.Controller.edit(
    {"graph_path": "/Graph", "evaluator_name": "execution"},
    {og.Controller.Keys.CREATE_NODES: [
        ("OnTick", "omni.graph.action.OnTick"),
        ("RosBridgePub", "omni.isaac.ros2_bridge.ROS2PublishTransformTree"),
    ],
     og.Controller.Keys.CONNECT: [
        ("OnTick.outputs:tick", "RosBridgePub.inputs:execIn"),
     ]},
)

# Sim 실행 (별도 터미널: ros2 topic echo /tf)
for _ in range(1000):
    world.step(render=True)

sim_app.close()
```

다른 터미널:
```bash
ros2 topic list
# 기대: /tf, /clock 등

ros2 topic echo /tf
```

---

## [note] 실습 2: Camera + joint_state publish

Action Graph 에 추가:
```
ROS2 Camera Helper (image publisher)
ROS2 Publish JointState (joint states)
```

이게 Phase 7 의 vla_node 와 통합되는 흐름.

---

## [note] 실습 3: ROS2 closed loop test

```bash
# Terminal 1: Isaac Sim 실행 (위 스크립트)
# Terminal 2: 표준 ros2 명령들
ros2 topic list
ros2 topic hz /camera/image_raw   # 기대 ~ 30 Hz
ros2 topic echo /joint_states     # joint angle 출력
```

---

## [O] 체크리스트
- [ ] Bridge extension 활성 동작
- [ ] /tf publish 동작
- [ ] (확장) Camera image / joint state publish
- [ ] ros2 topic echo 로 검증
- [ ] quiz
