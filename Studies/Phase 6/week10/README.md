# Week 10: Force / Torque Feedback - Real vs Sim


> **이번 주 목표**: Dynamixel torque feedback vs Sim 의 joint effort 비교.
> **예상 시간**: 6시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | Dynamixel 토크 피드백 | `PRACTICE.md` 1 | dynamixel_sdk |
| 2 | Sim joint effort | `PRACTICE.md` 2 | get_applied_joint_efforts |
| 3 | 비교 + 시각화 | `PRACTICE.md` 3 | timeseries |
| 4 | 퀴즈 | | |


---


## 핵심 개념


### 1. Dynamixel 토크 측정


```python
present_current = dxl_client.read_uint16(motor_id, ADDR_PRESENT_CURRENT)
# torque_constant: XM430 의 경우 4.1 mNm/mA
```


### 2. Sim effort


```python
effort = arm.get_applied_joint_efforts() # (n_dof,) Nm
```


### 3. 비교


시계열 plot. Sim/Real 의 magnitude 차이 visualize.


### 4. Sim/Real 차이 원인


- Friction (Sim 의 joint friction)
- Inertia (URDF tensor)
- Gear ratio (motor 와 link)
- Actuator response


### 5. 양산 의미


Force-sensitive task (조립 등) 에서 sim2real transfer 의 핵심.


---


## 자체 점검


**Q1. Dynamixel 토크 단위?** > Nm. PRESENT_CURRENT * torque_constant.
**Q2. Sim effort method?** > get_applied_joint_efforts.
**Q3. Gap 원인?** > Friction / inertia / gear / actuator.
**Q4. 양산 의미?** > Force-sensitive task 성공률.
**Q5. Domain randomization 의 friction?** > Sim friction 범위 random.


---


## 실습 + 다음


### 이번 주: torque 측정 + 비교 + quiz
### 다음 주 (week 11): 카메라 image gap


---


## 핵심 요약


1. **Dynamixel torque** PRESENT_CURRENT
2. **Sim effort** get_applied_joint_efforts
3. **시계열 비교**
4. **Friction parameter** gap 조정
5. **Force-sensitive task** 양산 토대


- [Week 9](../week9/README.md) | [Week 11](../week11/README.md)
