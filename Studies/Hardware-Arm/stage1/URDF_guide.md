# Hardware-Arm Stage 1 - URDF 가이드 (SO-101 재사용 + 검증)


> 시기: 2026.10-11 (Stage 1) — **2026-08-30 갱신**: 처음부터 작성하지 않는다. **SO-101 공개 URDF 를 재사용**하고, 캘리브레이션 오프셋을 반영해 검증하는 것이 Stage 1 must 다.


---


## 0. 기본 경로 — SO-101 공개 URDF 재사용


1. 원본 확보: https://github.com/TheRobotStudio/SO-ARM100 (SO-101 URDF/STL — LeRobot 문서에서도 링크)
2. `so101_description/urdf/` 로 복사 후 joint 이름·mesh 경로를 패키지 기준으로 정리 — 패키지 원본은 이 레포의 `stage1/ros2_pkg/so101_description/` 이고 URDF 는 그 안 `urdf/` 한 곳에만 둔다 ([ros2_driver_setup.md](ros2_driver_setup.md) §2)
3. **캘리브레이션 오프셋 반영**: 스파이크의 LeRobot 캘리브레이션 (`lerobot-calibrate`) 영점과 URDF 영점을 대조 — 어긋나면 URDF 의 `<origin rpy>` 로 먼저 맞춘다. 드라이버의 `homing_offset` 은 서보 EEPROM 에 직접 기록돼 LeRobot 캘리브레이션과 같은 자리를 덮어쓰므로 마지막 수단이다 ([ros2_driver_setup.md](ros2_driver_setup.md) §1.2)
4. 아래 §1-§4 는 재사용한 URDF 를 **읽고 고치기 위한** 기초다 (백지 작성용 아님)


---


## 1. URDF 의 기본 구조 (읽는 법)


```xml
<robot name="so101">
  <!-- Base -->
  <link name="base_link">
    <visual>
      <geometry><mesh filename="package://so101/meshes/base.stl"/></geometry>
    </visual>
    <collision>
      <geometry><mesh filename="package://so101/meshes/base.stl"/></geometry>
    </collision>
    <inertial>
      <mass value="0.5"/>
      <inertia ixx="0.001" iyy="0.001" izz="0.001"
               ixy="0" ixz="0" iyz="0"/>
    </inertial>
  </link>


  <!-- Joint 1 -->
  <joint name="joint_1" type="revolute">
    <parent link="base_link"/>
    <child link="link_1"/>
    <origin xyz="0 0 0.1" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57"
           effort="0.16" velocity="3.14"/>
  </joint>


  <!-- Link 1 -->
  <link name="link_1">
    ...
  </link>


  <!-- Joint 2, Link 2, ... -->
</robot>
```


---


## XACRO 사용 권장


```xml
<?xml version="1.0"?>
<robot xmlns:xacro="http://www.ros.org/wiki/xacro" name="so101">


<xacro:property name="link_length" value="0.1"/>


<xacro:macro name="dxl_joint" params="name parent child *origin">
  <joint name="${name}" type="revolute">
    <parent link="${parent}"/>
    <child link="${child}"/>
    <xacro:insert_block name="origin"/>
    <axis xyz="0 0 1"/>
    <limit lower="-1.57" upper="1.57" effort="0.16" velocity="3.14"/>
  </joint>
</xacro:macro>


</robot>
```


`xacro` 명령으로 build:
```bash
xacro so101.urdf.xacro > so101.urdf
```


---


## 3D 프린트 부품 -> mesh 파일


```
1. Fusion 360 또는 OpenSCAD 로 디자인
2. STL 으로 export
3. ROS2 package 의 meshes/ 디렉토리에
4. URDF 에서 mesh filename="package://...
```


---


## ros2_control 인터페이스 추가 (feetech)


```xml
<ros2_control name="SO101Hardware" type="system">
  <hardware>
    <!-- 플러그인 클래스명은 feetech_ros2_driver 버전으로 확인 (ros2_driver_setup.md §1) -->
    <plugin>feetech_ros2_driver/FeetechHardwareInterface</plugin>
    <!-- 컨테이너에서는 so101-attach 가 만드는 고정 경로를 쓴다 -->
    <!-- 보드레이트는 파라미터가 없다 — 드라이버 코드의 기본값이 1000000 이다 (ros2_driver_setup.md §1.2) -->
    <param name="usb_port">/dev/so101_follower</param>
  </hardware>
  <joint name="shoulder_pan">
    <param name="id">1</param>
    <command_interface name="position"/>
    <!-- 드라이버가 내보내는 상태는 position / velocity 두 개다 (effort 는 없다) -->
    <state_interface name="position"/>
    <state_interface name="velocity"/>
  </joint>
  <!-- 나머지 5 joint 동일 패턴 (shoulder_lift 2, elbow_flex 3, wrist_flex 4, wrist_roll 5, gripper 6) -->
</ros2_control>
```

실제로 쓰는 블록은 `ros2_pkg/so101_description/urdf/so101.urdf.xacro` 의 끝에 있다 (6 joint 전부 + 팔 없이 시험하는 `use_mock_hardware` 분기).


---


## 검증 단계


```bash
# 1. URDF 파싱 검증
check_urdf so101.urdf


# 2. RViz 시각화
ros2 launch urdf_tutorial display.launch.py model:=so101.urdf


# 3. Tree 확인
urdf_to_graphviz so101.urdf
# 결과 PDF 확인
```


---


## 일반 문제


| 증상 | 해결 |
|---|---|
| Joint axis 잘못 | URDF 의 `<axis xyz=>` 확인 |
| Link 가 떠 있음 | `<origin>` xyz / rpy 확인 |
| Mesh 가 안 보임 | package:// path 확인 |
| Inertia 0 | mass + ixx/iyy/izz 추가 |
| 충돌 무시 | `<collision>` block 추가 |


---


## 체크리스트


- [ ] SO-101 공개 URDF 확보 + 패키지로 이식
- [ ] 캘리브레이션 오프셋 반영 (LeRobot 영점 대조)
- [ ] joint 6 + gripper 의 axis / origin / limit 확인 (소프트 리밋 값은 안전 기초와 일치)
- [ ] mesh 파일 로딩
- [ ] ros2_control 인터페이스 (feetech)
- [ ] check_urdf 통과
- [ ] RViz 시각화 정상
