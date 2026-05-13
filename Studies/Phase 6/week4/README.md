# Week 4: 자작 팔 URDF -> USD 임포트


> **이번 주 목표**: Hardware-Arm Stage 1/2 에서 작성한 URDF 를 Isaac Sim 에 임포트. 자작 팔의 디지털 트윈 첫 단계.
> **예상 시간**: 8시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | URDF 검토 | `PRACTICE.md` 1 | Hardware-Arm Stage 1/2 |
| 2 | URDF Importer 사용 | `PRACTICE.md` 2 | Isaac Sim GUI |
| 3 | Python API 임포트 | `PRACTICE.md` 3 | omni.importer.urdf |
| 4 | 임포트 결과 검증 | `PRACTICE.md` 4 | joint / link 확인 |
| 5 | 퀴즈 | | |


---


## 핵심 개념


### 1. URDF 의 구조


```xml
<robot name="my_arm">
  <link name="base_link">
    <inertial>...</inertial>
    <visual><geometry><mesh filename="..."/></geometry></visual>
    <collision>...</collision>
  </link>
  <link name="link_1">...</link>


  <joint name="joint_1" type="revolute">
    <parent link="base_link"/>
    <child link="link_1"/>
    <origin xyz="0 0 0.1" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14" effort="1.0" velocity="3.0"/>
  </joint>
</robot>
```


### 2. URDF -> USD 변환


```python
from omni.importer.urdf import _urdf


urdf_path = "/path/to/my_arm.urdf"
import_config = _urdf.ImportConfig()
import_config.merge_fixed_joints = False
import_config.convex_decomp = False
import_config.fix_base = True


status, stage_path = _urdf.parse_and_import_urdf(urdf_path, import_config)
print(f"Imported to: {stage_path}") # /World/my_arm
```


### 3. 임포트 시 주의 사항


| 항목 | 권장 설정 |
|---|---|
| merge_fixed_joints | False (각 link 명확하게) |
| convex_decomp | False (mesh 그대로) |
| fix_base | True (base 고정 - 6DOF 팔) |
| import_inertia_tensor | True (물리 정확) |
| 좌표계 | URDF / USD 모두 right-handed |


### 4. 자작 팔 (Hardware-Arm Stage 2) 의 URDF spec


```
- 6 revolute joints (Dynamixel XM430 x 6)
- base_link
- link_1 ~ link_6
- ee_link (end-effector)
- gripper (1 prismatic 또는 1 revolute)
- 카메라 마운트 (선택, ee 부착)


Total links: 8~9
Total joints: 7~8
```


### 5. Articulation 으로 동작


USD 안의 robot 은 **Articulation Root** 로 표시. Joint 들이 한 단위.


```python
from omni.isaac.core.articulations import Articulation


arm = Articulation(prim_path="/World/my_arm")
arm.initialize()


print(f"DOF: {arm.num_dof}") # 6 (revolute joints)
print(f"Joint names: {arm.dof_names}")
print(f"Current angles: {arm.get_joint_positions()}")
```


### 6. Joint type 매핑


| URDF type | USD 의 표현 |
|---|---|
| revolute | RevoluteJoint |
| prismatic | PrismaticJoint |
| fixed | FixedJoint (보통 merge) |
| continuous | RevoluteJoint (no limit) |


### 7. 임포트 후 시각 검증


```python
# GUI 에서:
# - 자작 팔의 모든 link 가 보이는지
# - joint 가 적절한 위치에
# - inertia ellipsoid 확인 (옵션)


# 또는 Python 으로:
print(arm.dof_names) # ['joint_1', 'joint_2', ..., 'joint_6']
```


### 8. 일반 문제


| 증상 | 원인 | 해결 |
|---|---|---|
| Robot 이 떠 있음 | fix_base=False | True 로 |
| Mesh 가 안 보임 | mesh 파일 경로 잘못 | absolute path |
| Joint 가 동작 안 함 | drive 가 없음 | set_kp / kd |
| Inertia 이상 | URDF inertia 잘못 | URDF 수정 |


---


## 자체 점검


**Q1. URDF -> USD 변환 method?**
> `omni.importer.urdf._urdf.parse_and_import_urdf`.


**Q2. 자작 팔 6DOF 의 DOF 수?**
> 6 (revolute joints) + 1 (gripper) = 7.


**Q3. Articulation 의 핵심 method?**
> get_joint_positions / set_joint_positions / get_joint_velocities.


**Q4. fix_base=True 의 의미?**
> Robot 의 base_link 가 world 에 고정. 6DOF 팔에 필수 (떠다니지 않게).


**Q5. 임포트 후 시각 검증 항목?**
> link 모두 표시, joint 위치 정확, mesh 색/형태 정상.


---


## 실습 + 다음


### 이번 주: URDF 검토 + 임포트 + 검증 + quiz
### 다음 주 (week 5): Joint / Link 매핑


---


## 핵심 요약


1. **URDF -> USD** Isaac Sim Importer
2. **Articulation Root** 로 robot
3. **fix_base=True** 6DOF 팔
4. **Joint type 매핑** revolute / prismatic
5. **시각 검증** 필수


- [Week 3](../week3/README.md) | [Week 5](../week5/README.md)
