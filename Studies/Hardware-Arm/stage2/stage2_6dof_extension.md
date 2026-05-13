# Hardware-Arm Stage 2 - 6DOF 확장

> [time] 2027.02

---

## 추가 BOM

| 항목 | 수량 | 비용 |
|---|---|---|
| Dynamixel XM430-W350-T | 3 | ~90만 |
| 추가 케이블 + 부품 | - | ~5만 |
| 3D 프린트 새 부품 | - | ~5만 |

Total Stage 2 추가: ~ 100만.

XM430 vs XL330:
- XM430: 4.1 Nm stall torque (XL330 의 25배)
- 출력 모터로 적합
- 가격 ~ 30만/개

---

## 6DOF 구성

```
Joint 1 (base rotation)     : XM430 (큰 부하)
Joint 2 (shoulder pitch)    : XM430
Joint 3 (elbow pitch)        : XM430
Joint 4 (wrist roll)         : XL330 (소형)
Joint 5 (wrist pitch)        : XL330
Joint 6 (wrist yaw)          : XL330
Gripper                      : XL330 or 단순
```

---

## URDF 갱신

기존 Stage 1 URDF 에 추가:

```xml
<!-- New joints 4, 5, 6 -->
<joint name="joint_4" type="revolute">
  <parent link="link_3"/>
  <child link="link_4"/>
  <origin xyz="0 0 0.15" rpy="0 0 0"/>
  <axis xyz="1 0 0"/>
  <limit lower="-3.14" upper="3.14" effort="0.16" velocity="3.14"/>
</joint>

<!-- ... joint_5, joint_6 ... -->

<!-- Gripper -->
<joint name="gripper_joint" type="revolute">
  ...
</joint>
```

---

## ros2_control 인터페이스 확장

```xml
<joint name="joint_4">
  <param name="id">4</param>
  ...
</joint>
<joint name="joint_5">
  <param name="id">5</param>
  ...
</joint>
<joint name="joint_6">
  <param name="id">6</param>
  ...
</joint>
```

---

## 검증

```bash
# 6DOF 모두 detection
ros2 launch my_arm_description bringup.launch.py

# 각 joint 단독 동작 (Phase 6 week 5 의 패턴)
for joint_idx in 1 2 3 4 5 6; do
    # 단독 sweep test
done
```

---

## 체크리스트
- [ ] XM430 x 3 + XL330 x 3 조립
- [ ] URDF 6DOF 갱신
- [ ] RViz 시각화
- [ ] 각 joint 단독 동작
- [ ] FK 검증
