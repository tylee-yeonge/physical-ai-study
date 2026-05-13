# Week 9: 반복성 측정 - Sim 의 결정성 vs Real


> **이번 주 목표**: 같은 명령을 100번 반복 후 Sim 과 Real 의 결과 분산을 측정. Sim 의 deterministic vs Real 의 variability.
> **예상 시간**: 6시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | Sim deterministic test | `PRACTICE.md` 1 | 동일 seed |
| 2 | Real variability test | `PRACTICE.md` 2 | 100회 반복 |
| 3 | 분산 시각화 | `PRACTICE.md` 3 | std / box plot |
| 4 | 퀴즈 | | |


---


## 핵심 개념


### 1. Sim 의 결정성


```
같은 seed + 같은 input -> 같은 output (정확히)
Sim 의 큰 장점: regression test 가능
```


단, Isaac Sim 의 PhysX 는 약간의 non-determinism (multi-thread). 그래도 std ~ 0.


### 2. Real 의 variability


```
같은 명령 -> 약간 다른 결과
원인:
- Mechanical play (gear backlash)
- Sensor noise
- Temperature drift
- Vibration / external force
```


### 3. 측정 패턴


```python
# 같은 task 100회 반복
results = []
for trial in range(100):
    # Reset to known initial state
    arm.set_joint_positions(initial)
    # Execute trajectory
    for joint_target in trajectory:
        arm.set_joint_position_targets(joint_target)
        world.step() # Sim
    # Final ee position 기록
    final_ee = arm.get_world_pose("/World/my_arm/ee_link")[0]
    results.append(final_ee)


# 분산
std = np.std(results, axis=0)
print(f"Std (x, y, z): {std}")
# Sim 기대: < 0.1 mm
# Real 기대: ~ 1~5 mm
```


### 4. 분포 분석


```python
import matplotlib.pyplot as plt
plt.boxplot([sim_x, sim_y, sim_z, real_x, real_y, real_z])
plt.savefig("repeatability.png")
```


### 5. 양산 의미


Real 의 std 가 크면:
- Pick-and-place 의 grasp success rate ↓
- Domain randomization 필수 (Sim 에 noise 주입)


### 6. Phase 7 의 활용


본 측정 결과:
- Sim 의 reliability 보장
- Real 의 baseline noise level
- Sim/Real gap 의 일부 (분산 차이)


---


## 자체 점검


**Q1. Sim 의 결정성 의미?** > 같은 input -> 같은 output. regression test 가능.
**Q2. Real 의 variability 원인?** > Mechanical play / sensor noise / temperature / vibration.
**Q3. 측정 표준?** > 100회 반복, std / box plot.
**Q4. Sim std 기대?** > < 0.1 mm.
**Q5. Real std 기대?** > 1-5 mm (자작 팔).


---


## 실습 + 다음


### 이번 주: 반복성 측정 + quiz
### 다음 주 (week 10): Force / torque feedback


---


## 핵심 요약


1. **Sim deterministic** std ~ 0
2. **Real variability** 1-5 mm std
3. **100회 반복** 측정
4. **box plot** 시각화
5. **Phase 7 의 baseline**


- [Week 8](../week8/README.md) | [Week 10](../week10/README.md)
