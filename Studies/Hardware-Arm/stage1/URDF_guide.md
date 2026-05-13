# Hardware-Arm Stage 1 - URDF 작성 가이드


> 2026.11


---


## URDF 의 기본 구조


```xml
<robot name="my_arm">
  <!-- Base -->
  <link name="base_link">
    <visual>
      <geometry><mesh filename="package://my_arm/meshes/base.stl"/></geometry>
    </visual>
    <collision>
      <geometry><mesh filename="package://my_arm/meshes/base.stl"/></geometry>
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
<robot xmlns:xacro="http://www.ros.org/wiki/xacro" name="my_arm">


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
xacro my_arm.urdf.xacro > my_arm.urdf
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


## ros2_control 인터페이스 추가


```xml
<ros2_control name="MyArmHardware" type="system">
  <hardware>
    <plugin>dynamixel_hardware/DynamixelHardware</plugin>
    <param name="usb_port">/dev/ttyUSB0</param>
    <param name="baud_rate">1000000</param>
  </hardware>
  <joint name="joint_1">
    <param name="id">1</param>
    <command_interface name="position"/>
    <state_interface name="position"/>
    <state_interface name="velocity"/>
    <state_interface name="effort"/>
  </joint>
  <!-- joint_2, joint_3 ... -->
</ros2_control>
```


---


## 검증 단계


```bash
# 1. URDF 파싱 검증
check_urdf my_arm.urdf


# 2. RViz 시각화
ros2 launch urdf_tutorial display.launch.py model:=my_arm.urdf


# 3. Tree 확인
urdf_to_graphviz my_arm.urdf
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


- [ ] base_link + 2~3 link 정의
- [ ] joint 의 axis / origin / limit 정확
- [ ] mesh 파일 로딩
- [ ] inertial 추가
- [ ] ros2_control 인터페이스
- [ ] check_urdf 통과
- [ ] RViz 시각화 정상
