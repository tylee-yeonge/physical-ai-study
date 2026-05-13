# Week 5 실습: Joint 매핑 + FK 검증

> [time] **예상 시간**: 4시간

---

## [note] 실습 1: dof_names 매핑

```python
"""
practice_dof_mapping.py
"""
# (Sim setup 그대로)

print(f"DOF count: {arm.num_dof}")
print(f"dof_names: {arm.dof_names}")

# URDF joint 순서 (예: ROS2 의 joint_state_publisher 결과)
urdf_joints = ['joint_1', 'joint_2', 'joint_3', 'joint_4', 'joint_5', 'joint_6', 'gripper_joint']

# 매핑 표
mapping = {}
for i, name in enumerate(arm.dof_names):
    if name in urdf_joints:
        urdf_idx = urdf_joints.index(name)
        mapping[urdf_idx] = i
    else:
        print(f"[WARN] {name} not in URDF list")

print(f"Mapping (URDF_idx -> Sim_idx): {mapping}")
```

---

## [note] 실습 2: 단독 joint 동작

```python
"""
practice_joint_sweep.py
"""
import numpy as np

for i, name in enumerate(arm.dof_names):
    print(f"\nMoving {name}...")

    # Sweep: -limit -> 0 -> +limit
    lower = arm.dof_properties["lower"][i]
    upper = arm.dof_properties["upper"][i]
    for t in np.linspace(lower, upper, 100):
        target = arm.get_joint_positions()
        target[i] = t
        arm.set_joint_positions(target)
        world.step(render=True)

    # 영상 capture
    # 시각 확인: 해당 joint 만 움직임
```

---

## [note] 실습 3: FK 비교 (PyKDL 사용)

```bash
pip install pykdl  # 또는 robotics_toolbox-python
```

```python
"""
practice_fk_compare.py
"""
import numpy as np
import PyKDL as kdl
# ... or use existing FK code from Hardware-Arm Stage 1

# Real FK (URDF 의 KDL chain)
def fk_real(joint_angles):
    chain = ... # PyKDL chain from URDF
    fk_solver = kdl.ChainFkSolverPos_recursive(chain)
    end_frame = kdl.Frame()
    fk_solver.JntToCart(joint_angles, end_frame)
    return end_frame

# Sim FK
def fk_sim(arm, target_angles):
    arm.set_joint_positions(target_angles)
    for _ in range(10):
        world.step()
    return arm.get_world_pose("/World/my_arm/ee_link")[0]

# 비교 5 cases
test_cases = [
    np.zeros(6),
    np.ones(6) * 0.5,
    np.array([0.1, 0.5, -0.3, 0.0, 0.0, 0.0]),
    # ...
]

for case in test_cases:
    real = fk_real(case)
    sim = fk_sim(arm, case)
    err = np.linalg.norm(real - sim)
    print(f"angles {case}: error = {err*1000:.3f} mm")
```

기대: 모든 case 에서 < 1 mm.

---

## [O] 체크리스트
- [ ] dof_names 매핑 표 작성
- [ ] 단독 joint 동작 시각 검증
- [ ] FK 5 case 모두 < 1 mm
- [ ] quiz
