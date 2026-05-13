# Week 9 실습: 반복성 측정


> **예상 시간**: 4시간


---


## 실습 1: Sim 의 결정성 (100회)


```python
"""
practice_sim_repeatability.py
"""
import numpy as np


# 일정 trajectory
trajectory = [
    np.array([0.0, 0.5, 0.0, 0.0, 0.0, 0.0]),
    np.array([0.5, 0.5, 0.0, 0.0, 0.0, 0.0]),
    np.array([0.5, 0.0, 0.0, 0.0, 0.0, 0.0]),
    np.array([0.0, 0.0, 0.0, 0.0, 0.0, 0.0]),
]


results = []
for trial in range(100):
    arm.set_joint_positions(np.zeros(6)) # reset
    for _ in range(20):
        world.step()
    for target in trajectory:
        arm.set_joint_position_targets(target)
        for _ in range(20):
            world.step()
    ee = arm.get_world_pose("/World/my_arm/ee_link")[0]
    results.append(ee)


results = np.array(results)
print(f"Sim std (mm): {np.std(results, axis=0) * 1000}")
# 기대 < 0.1 mm
```


---


## 실습 2: Real 의 variability (100회)


```python
"""
practice_real_repeatability.py - 자작 팔에 실행
"""
# Same trajectory, but on real robot via ROS2
# Cycle: reset -> trajectory -> read final ee pose -> repeat


results_real = []
for trial in range(100):
    # Reset
    publish_command(np.zeros(6))
    time.sleep(2)


    # Execute trajectory
    for target in trajectory:
        publish_command(target)
        time.sleep(0.5)


    # Read final
    ee = read_real_ee_pose() # FK 또는 vision tracking
    results_real.append(ee)


print(f"Real std (mm): {np.std(results_real, axis=0) * 1000}")
# 기대 1~5 mm
```


---


## 실습 3: 시각화


```python
import matplotlib.pyplot as plt
import numpy as np


sim = np.array(sim_results) * 1000 # mm
real = np.array(real_results) * 1000


fig, ax = plt.subplots(1, 3, figsize=(12, 4))
for i, axis_name in enumerate(['X', 'Y', 'Z']):
    ax[i].boxplot([sim[:, i], real[:, i]], labels=['Sim', 'Real'])
    ax[i].set_title(f"EE position {axis_name}")
    ax[i].set_ylabel("mm")
plt.tight_layout()
plt.savefig("repeatability.png")
```


---


## 체크리스트
- [ ] Sim 100회 반복
- [ ] Real 100회 반복
- [ ] Box plot 시각화
- [ ] 보고서 작성
- [ ] quiz
