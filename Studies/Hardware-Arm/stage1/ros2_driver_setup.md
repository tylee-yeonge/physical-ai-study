# Hardware-Arm Stage 1 - ROS2 Driver Setup (feetech_ros2_driver)

> 시기: 2026.10 (Stage 1 첫 주 — 구 스파이크의 ROS2 검증 항목을 여기서 인수)
> 전제: 스파이크 (2026.09) 에서 팔이 LeRobot 네이티브로 이미 돈다 — teleop·캘리브레이션 완료 상태
> **역할 분리**: 데이터·학습 = LeRobot / 배포·통합 = ROS 2. **같은 시리얼 포트를 쓰므로 동시에 한 스택만 버스에 붙인다** (LeRobot 프로세스 종료 후 ROS2 기동)

---

## 0. 검증 순서 (구 스파이크 항목 — 조립된 팔에서 수행)

1. 모터 1개 경로: `feetech_ros2_driver` (ros2_control 하드웨어 인터페이스) 로 위치 명령 1회 전달
2. 6축 데이지체인: ID 1-6 인식 + joint_states 발행 확인
3. 최소 URDF → RViz 에서 joint state 반영 확인 ([URDF_guide.md](URDF_guide.md))

1번에서 막히면 드라이버 이슈 (권한/보드레이트/프로토콜), 2번에서 막히면 ID·배선 이슈로 분리해 디버깅한다.

---

## 1. feetech_ros2_driver 설치

```bash
cd ~/ros2_ws/src
git clone https://github.com/JafarAbdi/feetech_ros2_driver.git
cd ..
rosdep install --from-paths src --ignore-src -r -y
colcon build --packages-select feetech_ros2_driver
source install/setup.bash
```

> 플러그인 클래스명·파라미터 스키마는 저장소 버전에 따라 다르다 — 진입 시 저장소 README 로 재확인하고 아래 예시를 맞춘다.

---

## 2. so101_description 패키지

```bash
cd ~/ros2_ws/src
ros2 pkg create so101_description --build-type ament_cmake

# 디렉토리 구조
so101_description/
  CMakeLists.txt
  package.xml
  urdf/so101.urdf.xacro     # SO-101 공개 URDF 재사용 + 오프셋 (URDF_guide.md)
  config/so101_controllers.yaml
  launch/
    display.launch.py
    bringup.launch.py
```

---

## 3. Controller config

`config/so101_controllers.yaml`:

```yaml
controller_manager:
  ros__parameters:
    update_rate: 100

    joint_state_broadcaster:
      type: joint_state_broadcaster/JointStateBroadcaster

    position_controller:
      type: forward_command_controller/ForwardCommandController

position_controller:
  ros__parameters:
    joints:
      - shoulder_pan
      - shoulder_lift
      - elbow_flex
      - wrist_flex
      - wrist_roll
      - gripper
    interface_name: position
```

> joint 이름은 SO-101 공개 URDF 의 명명을 따른다 (URDF 와 yaml 에서 동일해야 함).

---

## 4. 동작 검증

```bash
# Terminal 1: Launch
ros2 launch so101_description bringup.launch.py

# Terminal 2: Topic 확인
ros2 topic list   # 기대: /joint_states, /position_controller/commands

# Terminal 3: 소폭 명령 (안전 기초 적용 전 — 팔 주변 비우고 소각도만)
ros2 topic pub --once /position_controller/commands std_msgs/msg/Float64MultiArray \
    "data: [0.1, 0.0, 0.0, 0.0, 0.0, 0.0]"
```

---

## 5. 이중 latency 측정 (Stage 1 must)

같은 추론 출력을 두 경로로 보내 왕복을 잰다:

| 경로 | 측정 |
|---|---|
| (a) LeRobot 직결 | 스파이크 must 4 방법 재사용 (n=100) |
| (b) ROS2 토픽 경유 | 추론 → `/position_controller/commands` → 모터 응답 (n=100) |

**(b)-(a) = 통합 오버헤드** — 셋째 층 증거로 기록 (Measurements 3분할 형식: environment / methodology / findings).

---

## 자주 발생 문제

| 증상 | 해결 |
|---|---|
| /dev/ttyUSB* 권한 | `dialout` 그룹 추가 (chmod 666 은 임시) |
| 모터 검출 안 됨 | LeRobot 프로세스가 포트 점유 중인지 먼저 확인 → 보드레이트·프로토콜 확인 |
| Joint 이름 mismatch | URDF 와 yaml 의 joint name 동일하게 |
| 위치 단위/오프셋 | URDF: rad. STS3215: 12비트 스텝 — LeRobot 캘리브레이션 오프셋과 드라이버 영점이 일치하는지 대조 |

---

## 체크리스트

- [ ] 검증 순서 1-3 통과 (모터 1개 → 데이지체인 → URDF+RViz)
- [ ] feetech_ros2_driver 빌드 성공
- [ ] so101_description 패키지 + bringup 동작
- [ ] joint_states 발행 + position 명령 → 모터 동작
- [ ] 이중 latency 측정 기록 ((a)/(b)/오버헤드)
