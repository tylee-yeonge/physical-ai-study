# Week 5: Joint / Link 매핑 + 시각 검증


> **이번 주 목표**: 임포트된 URDF 의 joint / link 가 정확한지 검증.
> **예상 시간**: 6시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | Joint 순서 확인 | `PRACTICE.md` 1 | URDF vs Isaac Sim |
| 2 | 단독 joint 동작 | `PRACTICE.md` 2 | 시각 검증 |
| 3 | FK 검증 | `PRACTICE.md` 3 | ee 위치 비교 |
| 4 | 퀴즈 | | |


---


## 핵심 개념


### 1. Joint 순서 위험


URDF 순서 != Isaac Sim DOF index 순서.
**반드시 `arm.dof_names` 확인** + 매핑 표 작성.


### 2. 단독 joint 동작 검증


```python
for i, name in enumerate(arm.dof_names):
    target = np.zeros(arm.num_dof)
    target[i] = 1.0
    arm.set_joint_positions(target)
    for _ in range(50):
        world.step(render=True)
    # 시각: 해당 joint 만 움직임 확인
```


### 3. FK 비교


```python
real_ee = compute_fk_real(joint_angles) # PyKDL 또는 robotics_toolbox
sim_ee = arm.get_world_pose("/World/my_arm/ee_link")
err = np.linalg.norm(real_ee - sim_ee)
# 기대 < 1 mm
```


### 4. Joint limit


```python
for name in arm.dof_names:
    lower = arm.dof_properties["lower"]
    upper = arm.dof_properties["upper"]
    print(f"{name}: [{lower:.2f}, {upper:.2f}]")
```


URDF 의 `<limit>` 와 일치해야.


### 5. 일반 문제


| 증상 | 원인 | 해결 |
|---|---|---|
| Joint 순서 mismatch | dof_names 다름 | 매핑 표 |
| FK 오차 큼 | URDF mesh 잘못 | URDF 수정 |
| Joint limit 무한 | continuous type | revolute + limit |


---


## 자체 점검


**Q1. dof_names 순서 검증?** > 단독 joint 동작.
**Q2. FK 기준?** > Sim - Real < 1mm.
**Q3. Joint limit 위치?** > arm.dof_properties.
**Q4. URDF 와 Sim joint 매핑 표 위치?** > 본인 노트, 학습 시 필수.
**Q5. 시각 검증의 의미?** > URDF 가 사실적으로 임포트되었는가.


---


## 실습 + 다음


### 이번 주: 매핑 + 검증
### 다음 주 (week 6): Sim <-> Real 매칭


---


## 핵심 요약


1. **dof_names != URDF 순서** - 매핑 표 작성
2. **단독 joint 동작** 시각 검증
3. **FK 오차 < 1mm**
4. **Joint limit** 확인
5. **URDF 정확성** 토대


- [Week 4](../week4/README.md) | [Week 6](../week6/README.md)
