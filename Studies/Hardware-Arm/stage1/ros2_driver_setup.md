# Hardware-Arm Stage 1 - ROS2 Driver Setup (feetech_ros2_driver)

> 시기: 2026.10 (Stage 1 첫 주 — 구 스파이크의 ROS2 검증 항목을 여기서 인수)
> 전제: 스파이크 (2026.09, 판정 2026-09-20 통과) 에서 팔이 LeRobot 네이티브로 이미 돈다 — teleop·캘리브레이션 완료 상태
> **이 환경의 사실**: 작업은 Ubuntu PC 위 Docker 컨테이너 (Ubuntu 24.04, ROS 2 Jazzy, root) 에서 한다. 팔로워 포트는 `so101-attach` 가 만드는 **`/dev/so101_follower`** 이고 (세션을 시작할 때마다 `so101-attach` 를 실행한다 — `ls` 로 노드가 보이는 것만으로는 연결돼 있다는 뜻이 아니다, "자주 발생 문제" 첫 행), 모터 ID 는 1-6, 보드레이트는 1,000,000 이다. `feetech_ros2_driver` 가 Jazzy 에서 빌드되는지는 §1 에서 가장 먼저 확인한다
> **역할 분리**: 데이터·학습 = LeRobot / 배포·통합 = ROS 2. **같은 시리얼 포트를 쓰므로 동시에 한 스택만 버스에 붙인다** (LeRobot 프로세스 종료 후 ROS2 기동)
> **코드 배치**: 워크스페이스 (빌드하는 곳) 는 **`/workspace/so101_ws`**, 직접 만드는 패키지의 원본은 **이 레포의 `stage1/ros2_pkg/`** 다. 둘은 심링크로 잇는다 (§2). 컨테이너의 홈 (`~` = `/root`) 은 컨테이너를 다시 만들면 지워지므로 워크스페이스를 `~/ros2_ws` 에 두지 않는다 — 남는 곳은 `/workspace` 뿐이다

---

## 0. 검증 순서 (구 스파이크 항목 — 조립된 팔에서 수행)

1. 모터 1개 경로: `feetech_ros2_driver` (ros2_control 하드웨어 인터페이스) 로 위치 명령 1회 전달
2. 6축 데이지체인: ID 1-6 인식 + joint_states 발행 확인
3. 최소 URDF → Foxglove 에서 joint state 반영 확인 (§4.3, [URDF_guide.md](URDF_guide.md))

1번에서 막히면 드라이버 이슈 (권한/보드레이트/프로토콜), 2번에서 막히면 ID·배선 이슈로 분리해 디버깅한다.

---

## 1. feetech_ros2_driver 설치

### 1.0 선행 설치 (이미지에 없을 때)

`vscode-tunnel` v1.18.0 보다 앞선 이미지로 만든 컨테이너에는 ROS 2 Jazzy 본체 (`ros-jazzy-desktop`) 만 들어 있고 아래 다섯 개가 없다. 없는 채로 §1.1 의 명령을 치면 `rosdep: command not found` 에서 멈춘다.

| 패키지 | 무엇인가 | 없으면 |
|---|---|---|
| `python3-rosdep` | `package.xml` 에 적힌 의존성 이름을 apt 패키지로 바꿔 한 번에 깔아 주는 도구 | `rosdep: command not found` |
| `python3-colcon-common-extensions` | ROS 2 워크스페이스 빌드 도구 `colcon` | 시스템에 `colcon` 이 없다 — venv 안에 pip 로 깔린 것만 잡힌다 |
| `ros-jazzy-ros2-control` | ros2_control 본체 (`hardware_interface` · `controller_manager`). 드라이버가 이것을 상속한다 | 드라이버 빌드 실패 |
| `ros-jazzy-ros2-controllers` | `joint_state_broadcaster` · `forward_command_controller` 등 §3 의 컨트롤러 | §4 에서 컨트롤러 로드 실패 |
| `ros-jazzy-xacro` | `.urdf.xacro` 를 `.urdf` 로 펴 주는 도구 | §2 · URDF_guide 의 `xacro` 명령 실패 |

```bash
deactivate                      # venv 가 켜져 있으면 끈다 (프롬프트 앞의 (.venv-...) 가 사라진다)
apt update
apt install -y python3-rosdep python3-colcon-common-extensions \
    ros-jazzy-ros2-control ros-jazzy-ros2-controllers ros-jazzy-xacro
rosdep init                     # 1회 — /etc/ros/rosdep/sources.list.d/ 를 만든다
rosdep update                   # 의존성 이름 -> apt 패키지 대응표를 받는다
which colcon rosdep             # 기대: /usr/bin/colcon, /usr/bin/rosdep
```

- **venv 를 끄는 이유**: 이 워크스페이스는 C++ 패키지라 파이썬 venv 가 필요 없다. venv 를 켠 채 빌드하면 `colcon` 과 CMake 가 venv 의 파이썬을 잡아 시스템 쪽 ROS 파이썬 모듈을 못 찾는 오류가 날 수 있다. `/workspace/so101_ws` 에서는 항상 venv 없이 작업한다.
- **이 패키지들은 컨테이너 이미지에도 들어 있다**: 호스트의 compose 프로젝트 (`vscode-tunnel`) 가 v1.18.0 부터 Dockerfile 에서 같은 목록을 설치하고 `rosdep init` · `rosdep update` 까지 끝낸다. v1.18.0 이후의 이미지로 만든 컨테이너에서는 이 절을 건너뛰고 아래 확인만 한다. 그보다 앞선 이미지의 컨테이너에서는 위 명령으로 직접 깐다 — `apt install` 은 컨테이너 안에만 남아 (`/workspace` 밖) 컨테이너를 다시 만들면 사라진다. 워크스페이스의 `build/` · `install/` 은 `/workspace` 에 있어 어느 쪽이든 남는다.

```bash
which colcon rosdep             # 기대: /usr/bin/colcon, /usr/bin/rosdep
ros2 pkg prefix controller_manager && ros2 pkg prefix xacro     # 기대: /opt/ros/jazzy 두 줄
```

이미지에 들어가는 목록은 위의 다섯 개에 다섯 개를 더한 것이다 — `rosdep install` 이 드라이버용으로 깔던 `libserial-dev` · `libexpected-dev` · `librange-v3-dev`, 패키지용으로 깔던 `ros-jazzy-joint-state-publisher` · `ros-jazzy-joint-state-publisher-gui`. 이 이미지에서 아무것도 손으로 깔지 않고 `/workspace/so101_ws` 의 두 패키지가 빌드되는 것을 확인했다.

### 1.1 드라이버 빌드

```bash
mkdir -p /workspace/so101_ws/src      # 워크스페이스는 /workspace 아래 (컨테이너 재생성에도 남는다)
cd /workspace/so101_ws/src
git clone https://github.com/JafarAbdi/feetech_ros2_driver.git
git -C feetech_ros2_driver log --oneline -1   # 받은 커밋 확인 — §1.2 는 18aed7f 기준이다
cd ..
rosdep install --from-paths src --ignore-src -r -y
colcon build --packages-select feetech_ros2_driver
source install/setup.bash
```

> 플러그인 클래스명·파라미터 스키마는 저장소 버전에 따라 다르다. 받은 커밋이 `18aed7f` 가 아니면 저장소의 `doc/user.md` 를 다시 읽고 §1.2 와 아래 예시를 그 버전에 맞춘다.
> `feetech_ros2_driver` 는 남의 저장소라 워크스페이스에만 둔다 (이 레포에 넣지 않는다).

### 1.2 이 드라이버의 사실 (v0.2.2, 커밋 `18aed7f` — `doc/user.md` 와 `src/feetech_ros2_driver.cpp` 에서 확인)

| 항목 | 사실 | 이 작업에 주는 영향 |
|---|---|---|
| 지원 배포판 | 저장소 CI 가 jazzy · humble · rolling 을 빌드한다 | Jazzy 빌드는 저장소가 관리하는 경로다 |
| 플러그인 이름 | `feetech_ros2_driver/FeetechHardwareInterface` | [URDF_guide.md](URDF_guide.md) 의 `<plugin>` 예시와 같다 |
| 하드웨어 파라미터 | `usb_port` (필수), `joint_config_file` (선택) 두 개만 읽는다. 보드레이트는 파라미터가 아니라 코드 기본값 1,000,000 이다 | `/dev/so101_follower` 만 넣으면 된다. 서보가 1,000,000 bps 로 설정돼 있어 그대로 맞는다 |
| 관절 파라미터 | `id` (필수). 선택: `homing_offset`, `range_min`, `range_max`, `max_torque_limit`, `protection_current`, `overload_torque`, `p/i/d_coefficient`, `return_delay_time`, `acceleration` | W5 의 토크 상한 = `max_torque_limit`, 서보 단의 각도 한계 = `range_min` · `range_max` (단위는 도가 아니라 0-4095 틱) |
| **선택 파라미터를 적으면 서보에 기록된다** | 기동할 때 토크를 끄고 EEPROM 잠금을 푼 뒤, **적혀 있는 파라미터만** 서보에 쓴다. 적지 않은 것은 건드리지 않는다 | 서보의 EEPROM 은 LeRobot 과 공유한다. LeRobot 캘리브레이션도 같은 자리 (`Homing_Offset` · `Min/Max_Position_Limit`) 에 값을 써 두었다. LeRobot 은 연결할 때마다 서보에서 이 값들을 읽어 캘리브 json 과 비교하고, 하나라도 다르면 "캘리브가 안 맞는다" 로 보고 캘리브 절차로 들어간다 (lerobot 0.6.2 `is_calibrated`). **W1-W2 에서는 `id` 만 적는다.** 다른 값은 W4 · W5 에서 LeRobot 캘리브 json 과 대조하며 하나씩 넣는다 |
| 영점 | 드라이버는 항상 2048 틱을 0 rad 로 본다 | **틱 기준은 두 스택이 같고, 각도의 0 은 다르다.** LeRobot 캘리브는 호밍 자세가 2047 틱으로 읽히게 `Homing_Offset` 을 서보에 써 둔다 (lerobot 0.6.2 `_get_half_turn_homings`) — 드라이버의 2048 과 1틱 (0.09도) 차이라 틱 기준은 같다고 봐도 된다. 그런데 LeRobot 이 내주는 **도 단위 값의 0 은 2047 틱이 아니라 캘리브 범위의 중점** `(range_min + range_max) / 2` 다 (`motors_bus.py` 의 DEGREES 정규화). 이 팔의 캘리브 json 으로 계산한 차이 (중점 - 2047): `shoulder_pan` +0.2도, `shoulder_lift` -0.9도, `elbow_flex` **-13.9도**, `wrist_flex` +3.8도, `wrist_roll` 0도 (그리퍼는 LeRobot 에서 0-100 단위라 해당 없음). 같은 자세를 두 스택이 다른 숫자로 읽는다는 뜻이다 — `elbow_flex` 는 ROS2 쪽 값이 LeRobot 쪽 값보다 약 14도 작게 나온다. W4 에서 두 스택의 값을 대조하거나 W6-7 · v2.5 에서 값을 오갈 때 이 차이를 관절별로 더해 줘야 한다 |
| 기동 시 | 명령 인터페이스가 있는 관절의 토크를 켜고, 첫 목표를 현재 위치로 둔다 | launch 를 띄우는 순간 팔이 튀지 않고 그 자세로 굳는다. 실제 팔에서 확인했다 (2026-09-22) — 기동 직후 6개 관절 모두 목표값 (command interface) 이 현재 위치 (state interface) 와 같았다 (`/controller_manager/introspection_data/full` 로 읽음). 그래도 **기동은 팔을 휴식 자세에 두고 한다** — 끌 때 토크가 풀리기 때문이다 (아래 행) |
| **종료 시 토크 OFF** | `on_deactivate` 가 모든 관절의 토크를 끈다 | **launch 를 `Ctrl+C` 로 끄면 팔이 자중으로 떨어진다.** 끄기 전에 팔을 휴식 자세로 보내거나 손으로 받친다. W5 의 소프트웨어 정지를 "launch 종료" 로 구현하면 안 되는 이유이기도 하다 |

---

## 2. so101_description 패키지

직접 만드는 패키지는 **원본을 이 레포에 두고, 워크스페이스에는 심링크 (바로가기) 만 건다.** 이유는 두 가지다.

- 캘리브 오프셋을 반영한 URDF 와 controller config 는 Stage 1 의 must 산출물이다 — 레포 안에 있어야 git 으로 이력이 남는다.
- 빌드 결과물 (`build/` · `install/` · `log/`) 은 `colcon build` 를 실행한 폴더에 생긴다. 워크스페이스에서 빌드하면 레포에는 소스만 남는다. colcon 은 `src/` 안의 심링크를 따라가 패키지를 찾는다.

```bash
# 1) 원본은 레포 안에 만든다
PKG_HOME=/workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/ros2_pkg
mkdir -p $PKG_HOME
cd $PKG_HOME
ros2 pkg create so101_description --build-type ament_cmake

# 2) 워크스페이스 src/ 에 심링크를 건다
ln -s $PKG_HOME/so101_description /workspace/so101_ws/src/so101_description
ls -la /workspace/so101_ws/src      # 기대: so101_description -> /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/ros2_pkg/so101_description

# 3) 빌드는 항상 워크스페이스에서 한다 (레포 폴더 안에서 colcon build 를 돌리지 않는다)
cd /workspace/so101_ws
colcon build --packages-select so101_description --symlink-install
source install/setup.bash

# 디렉토리 구조 (원본 = 레포의 stage1/ros2_pkg/so101_description/)
so101_description/
  CMakeLists.txt            # 컴파일할 코드 없음 — 아래 폴더 4개를 share/ 로 설치만 한다
  package.xml               # launch 가 쓰는 패키지를 exec_depend 로 적어 둠 (rosdep 이 읽는다)
  urdf/so101.urdf.xacro     # SO-101 공개 URDF 재사용 + <ros2_control> 블록 — URDF 는 여기 한 곳에만 둔다
  meshes/*.stl              # 공개 URDF 가 참조하는 STL 13개 (16 MB)
  config/so101_controllers.yaml
  launch/
    display.launch.py       # 팔 없이 URDF 만 본다
    bringup.launch.py       # 드라이버 + 컨트롤러 (팔에 붙는다)
```

> `--symlink-install` 은 설치 폴더에 파일을 복사하지 않고 원본을 가리키게 한다 — URDF · yaml · launch 를 고친 뒤 다시 빌드하지 않아도 바로 반영된다 (새 파일을 추가했을 때만 다시 빌드).

**패키지가 기대는 것을 한 번 더 깐다.** `package.xml` 에 적힌 패키지 중 컨테이너에 없는 것 (`joint_state_publisher` · `joint_state_publisher_gui`) 이 있다. `display.launch.py` 가 이것을 쓴다.

```bash
cd /workspace/so101_ws
rosdep install --from-paths src --ignore-src -r -y
```

**URDF 의 출처**: TheRobotStudio/SO-ARM100 의 `Simulation/SO101/so101_new_calib.urdf` (커밋 `eecbe3e`, Apache-2.0). `new_calib` 은 각 관절의 0 이 가동 범위의 가운데인 버전이고 LeRobot 캘리브레이션과 같은 규약이다. 원본에서 바꾼 것은 네 가지뿐이다 — mesh 경로를 `package://so101_description/meshes/` 로, robot 이름을 `so101` 로, ROS 1 형식 `<transmission>` 블록 6개 제거, 끝에 `<ros2_control>` 블록 추가. link · joint 의 수치는 원본 그대로다.

**launch 인자 두 개**

| 인자 | 기본값 | 뜻 |
|---|---|---|
| `use_mock_hardware` (bringup 만) | `false` | `true` 면 실제 드라이버 대신 가짜 하드웨어 (`mock_components/GenericSystem`) 를 쓴다. 시리얼 포트를 열지 않고, 받은 명령을 그대로 현재 위치로 돌려준다. 팔 없이 launch · 컨트롤러 설정을 시험하는 용도다 |
| `gui` | `false` | `true` 면 RViz (display 는 관절 슬라이더 창도) 를 띄운다. 이 컨테이너에는 화면 (`DISPLAY`) 이 없어 창을 띄울 수 없으므로 기본값이 `false` 다. 화면은 §4.3 의 Foxglove 로 본다 |

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

띄우기 전과 끄기 전에 한 가지씩 지킨다 (근거는 §1.2).

- **띄우기 전**: 팔을 휴식 자세에 둔다. launch 가 뜨면 토크가 켜지면서 팔이 그 자세로 굳는다.
- **끄기 전**: Terminal 1 에서 `Ctrl+C` 를 누르면 드라이버가 모든 관절의 토크를 끄고, 팔이 자중으로 떨어진다. 먼저 팔을 휴식 자세로 보내거나 손으로 받친 뒤에 끈다.

### 4.1 먼저 팔 없이 (mock)

실제 팔에 붙이기 전에 launch · 컨트롤러 설정 · 명령 순서를 가짜 하드웨어로 확인한다. 팔은 움직이지 않는다.

```bash
# Terminal 1
ros2 launch so101_description bringup.launch.py use_mock_hardware:=true
# 로그에서 확인: Loaded hardware 'SO101Hardware' from plugin 'mock_components/GenericSystem'

# Terminal 2
ros2 control list_controllers
# 기대: position_controller ... active / joint_state_broadcaster ... active
ros2 topic pub --once /position_controller/commands std_msgs/msg/Float64MultiArray \
    "data: [0.1, 0.0, 0.0, 0.0, 0.0, 0.0]"
ros2 topic echo /joint_states --once
# 기대: shoulder_pan 의 position 만 0.1 (mock 은 받은 명령을 그대로 현재 위치로 돌려준다)
```

여기서 **두 토픽의 관절 순서가 다르다**는 것을 눈으로 확인해 둔다.

| 토픽 | 순서 |
|---|---|
| `/position_controller/commands` (보내는 쪽) | yaml 의 `joints` 순서 — `shoulder_pan`, `shoulder_lift`, `elbow_flex`, `wrist_flex`, `wrist_roll`, `gripper` |
| `/joint_states` (읽는 쪽) | 알파벳 순서 — `elbow_flex`, `gripper`, `shoulder_lift`, `shoulder_pan`, `wrist_flex`, `wrist_roll` |

`/joint_states` 의 값은 위치 (몇 번째) 가 아니라 **`name` 배열의 이름으로** 찾아야 한다.

### 4.2 실제 팔

> **위 mock 의 명령 `[0.1, 0, 0, 0, 0, 0]` 을 실제 팔에 그대로 보내면 안 된다.** `position_controller` 는 배열의 6개 값을 전부 목표로 쓴다. 0 은 "그대로 둔다" 가 아니라 "0 rad (가동 범위의 가운데) 로 가라" 다. 휴식 자세의 팔은 0 rad 에서 멀리 떨어져 있어서 (스파이크 실측: `shoulder_lift` 약 -70도, `elbow_flex` 약 +100도), 이 명령은 다섯 관절을 한꺼번에 가운데 자세로 보낸다. 이 컨트롤러에는 보간도 속도 제한도 없어 서보의 최고 속도로 움직인다.
> **첫 명령은 "지금 읽은 위치 그대로 + 한 관절만 조금" 이다.**

```bash
# Terminal 1: 보드가 실제로 연결돼 있는지부터 (노드를 새로 만든다. 실패하면 USB 를 확인)
so101-attach

# Terminal 1: Launch (팔은 휴식 자세. LeRobot 프로세스는 꺼져 있어야 한다)
ros2 launch so101_description bringup.launch.py
# 로그에서 확인: ... from plugin 'feetech_ros2_driver/FeetechHardwareInterface'

# Terminal 2: 상태 확인
ros2 control list_controllers          # 둘 다 active

# Terminal 2: 붙여 넣을 명령을 만든다 (읽기만 한다 — 팔에는 아무것도 보내지 않는다)
python3 /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/print_joint_command.py
```

**Terminal 1 에 찍히는 것 — 어디까지가 정상인가**

정상 기동이면 아래 줄이 차례로 나오고, 마지막 줄 뒤로는 조용해진다. 멈춘 것이 아니라 제어 루프가 100 Hz 로 돌고 있는 상태다.

| 로그 | 뜻 |
|---|---|
| `Loaded hardware 'SO101Hardware' from plugin 'feetech_ros2_driver/FeetechHardwareInterface'` | 실제 드라이버를 불렀다 (mock 이면 `mock_components/GenericSystem`) |
| `Connecting to port: /dev/so101_follower` 다음에 `Successful initialization` · `Successful 'configure'` · `Successful 'activate'` | 포트가 열렸고 서보 6개가 응답했다. 이 시점에 토크가 켜진다 |
| `Configured and activated joint_state_broadcaster` / `... position_controller` | 컨트롤러 2개가 켜졌다 |
| `[spawner-3]: process has finished cleanly` | **오류가 아니다.** spawner 는 컨트롤러를 켜고 나면 할 일이 끝나 스스로 종료하는 프로그램이다 |

`[WARN]` 두 줄은 매번 나오고 무시해도 된다.

| 경고 | 뜻 |
|---|---|
| `Could not enable FIFO RT scheduling policy ... Operation not permitted` | 컨테이너에 실시간 스케줄링 권한이 없어 제어 루프가 보통 우선순위로 돈다. 100 Hz 는 유지된다 (`ros2 topic hz /joint_states` 실측 100.0 Hz). 주기의 흔들림이 문제가 되는 것은 W6-7 의 latency 측정 때다 |
| `kdl_parser: The root link base_link has an inertia specified in the URDF` | 공개 URDF 의 뿌리 링크에 관성값이 적혀 있는데 TF 계산 라이브러리 (KDL) 가 그것을 쓰지 않는다는 알림이다. 관절 위치 계산에는 영향이 없다 |

`[INFO]` 지만 기억해 둘 줄이 하나 있다 — `Enforcing command limits is disabled. Command limits from URDF will be ignored.` URDF 의 `<limit>` 가 적용되지 않는다는 뜻이고, W5 소프트 리밋의 출발점이다.

진짜 오류는 `[ERROR]` · `[error]` 로 찍히고, 그 뒤에 `process has died` 가 따라온다 ("자주 발생 문제" 표). 예외가 하나 있다 — `Ctrl+C` 로 끌 때 찍히는 `[ERROR] [controller_manager.pal_statistics]: Exception in publisher thread: context cannot be slept with because it's invalid!` 두 줄은 종료 중에 통계 발행 스레드가 내는 소음이다. 바로 뒤에 `Successful 'deactivate'` 와 `process has finished cleanly` 가 나오면 정상 종료다.

스크립트는 `/joint_states` 를 한 번 읽어 관절값을 이름으로 찾고, 명령 순서 (yaml 의 `joints` 순서) 로 다시 늘어놓은 명령 두 줄을 찍는다. 아래는 출력 예시다 — **숫자는 그때그때 다르므로 예시를 베끼지 말고 자기 터미널에 찍힌 줄을 붙여 넣는다.**

```
관절            rad        도
shoulder_pan    +0.0890     +5.10
shoulder_lift   +0.5139    +29.44
...

# shoulder_pan 만 +0.0500 rad (+2.86도) 움직인다
ros2 topic pub --once /position_controller/commands std_msgs/msg/Float64MultiArray "data: [0.139, 0.5139, 0.3513, 0.0092, 0.0046, -0.0046]"

# 지금 읽은 자세로 되돌린다
ros2 topic pub --once /position_controller/commands std_msgs/msg/Float64MultiArray "data: [0.089, 0.5139, 0.3513, 0.0092, 0.0046, -0.0046]"
```

- 첫 줄을 붙여 넣으면 `shoulder_pan` 하나만 약 3도 돈다. 나머지 다섯 값은 지금 읽은 위치 그대로라 움직이지 않는다. 둘째 줄로 되돌린다.
- 다른 관절은 뒤에 이름과 변화량 (rad) 을 붙인다:

  ```bash
  python3 /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/print_joint_command.py elbow_flex -0.05
  ```
- 표의 값이 눈앞의 팔 자세와 말이 되는지 본다 (어느 관절이 0 근처이고 어느 관절이 크게 꺾여 있는가). 이 대조가 W4 의 출발점이다.
- 변화량은 작게 (0.05 rad 안팎) 시작한다. 이 컨트롤러는 받은 목표로 서보 최고 속도로 가고, 소프트 리밋 · 토크 상한은 W5 에 가서야 생긴다.
- 명령을 직접 손으로 짜지 않는다. 값 6개의 순서나 부호를 틀리면 그 관절이 그만큼 튄다.
- 끄기 전에는 이 절 맨 위의 "끄기 전" 을 따른다.

### 4.3 화면으로 보기 (Foxglove)

이 컨테이너에는 화면 (`DISPLAY`) 이 없어 RViz 를 띄울 수 없다. 대신 컨테이너에서 **foxglove_bridge** (ROS2 토픽을 WebSocket 으로 내보내는 노드) 를 띄우고, 맥북의 **Foxglove** (웹앱 app.foxglove.dev 또는 데스크톱 앱) 로 붙어서 본다. 브리지는 컨테이너 이미지에 들어 있다 (vscode-tunnel v1.19.0+).

**경로**: 컨테이너 8765 (브리지) → 호스트 루프백 `127.0.0.1:8766` (compose 매핑) → `tailscale serve` 가 Tailnet 에 https 8766 으로 공개 → 맥북. 웹앱은 https 페이지라 `wss://` 로만 붙을 수 있고, 인증서는 Tailscale 이 발급한다. 호스트 쪽 설정은 vscode-tunnel 레포 README 의 "Foxglove 브리지" 절 (v1.20.0) 에 있고 재부팅 후에도 유지된다. VS Code 의 포트 전달 (devtunnels 주소) 은 로그인 쿠키가 있는 브라우저 탭만 통과시키므로 Foxglove 에는 쓸 수 없다.

**터미널 배치** — 세 개를 쓴다. 1 · 2 는 켜 둔 채로 두고 3 에서만 명령을 친다.

| 터미널 | 역할 | 켜 두는가 |
|---|---|---|
| 1 | bringup (§4.1 mock 또는 §4.2 실제 팔) | 켜 둔다. 끄면 `/joint_states` 가 사라진다 |
| 2 | 브리지 | 켜 둔다. bringup 을 껐다 켜도 그대로 둔다 (Foxglove 가 알아서 다시 구독한다) |
| 3 | `print_joint_command.py`, `ros2 topic pub`, `ros2 control ...` | 그때그때 |

`print_joint_command.py` 를 실행했는데 아무것도 안 찍히고 멈춰 있으면, 그 터미널의 문제가 아니라 **터미널 1 의 bringup 이 꺼져 있는 것**이다. 스크립트는 `/joint_states` 의 첫 메시지를 기다리는데 발행자가 없으면 영원히 기다린다. `Ctrl+C` 로 끊고 bringup 부터 띄운다.

```bash
# Terminal 2: 브리지. so101 워크스페이스를 반드시 소싱한다 — URDF 의 mesh 경로가 package://so101_description/... 이라
# 브리지가 이 패키지를 알아야 Foxglove 에 mesh 파일을 넘겨줄 수 있다. 빼먹으면 팔이 축과 이름표로만 보인다
source /opt/ros/jazzy/setup.bash && source /workspace/so101_ws/install/setup.bash && ros2 launch foxglove_bridge foxglove_bridge_launch.xml
```

**Foxglove 에서**

1. Open connection → Foxglove WebSocket → 주소는 `wss://` + 호스트의 Tailnet 이름 + `:8766`. 이름은 호스트 터미널에서 아래로 확인한다 (레포가 공개라 문서에는 적지 않는다).

   ```bash
   tailscale status --json | grep -m1 DNSName
   ```
2. 붙으면 하단의 시계가 흐른다. Problems 탭에 빨간 표시가 있으면 연결 실패다. 앱이 오래된 버전이면 브리지 3.x 의 프로토콜 (`foxglove.sdk.v1`) 을 몰라 거절되므로 최신으로 올린다. 웹앱은 Chrome 130 이상을 요구한다.
3. 3D 패널: 설정 (톱니) 의 Topics 에서 `/robot_description` 을 켠다. 팔은 30 cm 라 1 m 격자 가운데에 작게 보이므로 스크롤로 확대한다. 링크 이름표 (`upper_arm_link` 등) 는 `/tf` 에서 온 것이고, 팔 형상이 같이 보이면 URDF 와 mesh 까지 된 것이다.
4. 모델은 `/tf` 로 움직인다 — bringup 의 robot_state_publisher 가 `/joint_states` 를 읽어 `/tf` 를 낸다. `/joint_states` 를 Foxglove 에서 따로 켤 필요는 없다.
5. (선택) Plot 패널에 `/position_controller/commands` 를 넣으면 보낸 명령이 찍힌다.

**확인 순서** (W1 ③ 의 완료 기준)

- mock: 터미널 3 에서 헬퍼로 명령을 뽑아 보낸다. mock 은 팔이 없으니 변화량을 크게 잡아도 된다. 3D 패널의 팔이 그만큼 돌면 된다.

  ```bash
  source /opt/ros/jazzy/setup.bash && source /workspace/so101_ws/install/setup.bash && python3 /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/print_joint_command.py shoulder_pan 0.5
  ```
- 실제 팔: §4.2 대로 헬퍼의 +0.05 rad 명령을 보내고, 눈앞의 팔과 화면의 팔이 같은 관절을 같은 방향으로 움직이는지 본다. 같으면 W1 ③ 완료이고, 이 화면이 W4 (자세 대조) 의 도구다. 손으로 관절을 밀어서 확인하지 않는다 — 토크가 켜져 있어 서보가 버틴다.

---

## 5. 이중 latency 측정 (Stage 1 must, W6-7)

**무엇을**: 같은 명령 (한 관절의 목표값) 을 두 길로 팔에 보낸다 — (a) LeRobot 으로 직접, (b) ROS2 토픽을 거쳐. 각 길에서 "명령을 넘긴 순간부터 팔이 움직이기 시작할 때까지" 를 100번씩 잰다. 두 평균의 차이 **(b)-(a) 가 통합 오버헤드**, 곧 ROS2 를 거치는 대가로 늘어나는 시간이다.
**왜**: "VLA 를 ROS2 시스템에 얹으면 얼마나 느려지는가" 에 숫자로 답하기 위해서다. 스파이크 must 4 의 106 ms 는 모델이 action 을 만드는 비용이고, 이 측정은 그 뒤 — 만들어진 action 이 모터에 닿기까지의 비용이다.
**끝나면 손에 남는 것**: (a) · (b) · (b)-(a) 의 mean / p95 + Measurements 디렉토리 1개 ([`Measurements/so101-dual-latency/`](../../../Measurements/so101-dual-latency/) — 측정 정의는 [methodology.md](../../../Measurements/so101-dual-latency/methodology.md)).
**실측 (2026-09-25)**: (a) 65.4 / (b) 102.8 / (b) mock 15.6 / **(b)-(a) 37.5 ms** (각 n=100, 실패 0). 그중 메커니즘으로 설명되는 것은 10 ms 안팎이고 나머지 25-30 ms 의 출처는 미해결이다 — 해석과 다음 실험은 [findings.md](../../../Measurements/so101-dual-latency/findings.md). §5.5 의 검증 6개 중 문턱 5틱 · `update_rate` 200 은 미실시.
**언제**: W5 (안전 기초) 뒤. 팔을 200번 넘게 움직이는 측정이라 소프트 리밋 · 토크 상한 · 소프트웨어 정지가 선 다음에 한다.

### 5.1 추론 시간은 이 측정에 넣지 않는다

스파이크 must 4 는 팔 없이 GPU 의 추론 시간만 쟀다 ([week2_guide](../spike/week2/week2_guide.md) §4). 그 방법을 (a) 로 그대로 쓰면 (a) 는 "추론", (b) 는 "추론 + 전달 + 모터 응답" 이 돼서 빼도 뜻이 없다.

추론은 두 길에 똑같이 들어가는 항이다. 똑같이 들어가는 항은 뺄 때 사라지므로 넣어 봐야 얻는 것이 없고, 추론 시간의 흔들림 (표준편차 1.6 ms) 만 남아 몇 ms 짜리 오버헤드를 가린다. 그래서 **추론을 빼고 "명령이 전달되는 길" 만 잰다.** must 4 에서 가져오는 것은 수치가 아니라 방법이다 — n=100, warm-up 5, mean / p95 / std, 원본 npy + 요약 csv, 환경 기록.

### 5.2 두 길에서 시간이 드는 곳

| 단계 | (a) LeRobot 직결 | (b) ROS2 경유 |
|---|---|---|
| 명령을 넘긴다 (t0) | `robot.send_action()` 을 부르기 직전 | `/position_controller/commands` 에 publish 하기 직전 |
| 스택 안 | 함수 호출 → 시리얼 쓰기 | DDS 전송 → 컨트롤러가 받아서 보관 → **다음 제어 주기까지 대기** (100 Hz 면 0-10 ms) → 드라이버 `write()` → 시리얼 쓰기 |
| 두 길 공통 | USB · 서보 버스 전송 → 서보가 움직이기 시작 | 같음 |
| 움직임을 본다 (t1) | `Present_Position` 을 읽어 변화 감지 | `/joint_states` 에서 변화 감지 |

"두 길 공통" 은 뺄 때 사라진다. 남는 것이 "스택 안" 의 차이이고, 그것이 이 측정이 알고 싶은 값이다.

### 5.3 측정 정의 (고정 — 정본은 methodology.md)

정의는 착수 전에 고정했고, 정본은 [`Measurements/so101-dual-latency/methodology.md`](../../../Measurements/so101-dual-latency/methodology.md) 다 (조건 표 + 각 조건이 막는 오측정 + 두 경로에서 같아야 하는 것의 점검표 + 절차 + 검증 기준). 아래는 그 요약이다. 두 문서가 어긋나면 methodology.md 를 따르고 이 절을 고친다.

**1회 = 한 관절에 작은 계단 명령 1개를 보내고, 그 관절이 움직이기 시작할 때까지의 시간 (t1 - t0).**

| 항목 | 기본안 | 왜 |
|---|---|---|
| 움직일 관절 | `shoulder_pan` | 중력을 받지 않아 도는 방향에 따라 응답이 달라지지 않는다 |
| 계단 크기 | 33틱 (0.0506 rad, 2.9도). 시작 위치와 시작 + 33틱 두 목표를 번갈아 보낸다 | 0.05 rad 에 가장 가까운 정수 틱이라 두 길에서 같은 틱 수를 보낼 수 있다. 작아서 안전하고, 절대 목표 두 개를 오가므로 정지 오차 (되돌아온 자리가 몇 틱 어긋나는 것) 가 쌓여 흘러가지 않는다 |
| t0 | 명령을 스택에 넘기기 직전의 시각 | 두 길에서 같은 위치다 |
| t1 | 위치가 계단 전 값에서 문턱 (3틱 = 0.26도) 넘게 벗어난 첫 샘플의 시각. (b) 에서는 메시지를 받은 시각이 아니라 `/joint_states` 의 `header.stamp` | 서보는 서 있을 때도 1-2틱 떨린다 — 그것을 움직임으로 잡지 않기 위한 문턱이다. `header.stamp` 를 쓰는 것은 상태가 되돌아오는 길의 DDS 시간이 (b) 에만 더해지지 않게 하기 위해서다 |
| **관측 주기** | **두 길 모두 10 ms** | 아래 설명 |
| 반복 사이 | 팔이 멈춘 뒤 0.5 s | 앞 반복의 움직임이 다음 반복에 섞이지 않게 |
| n / warm-up | 100 / 5 | must 4 와 같다 |
| LeRobot 의 `max_relative_target` | 끄고 잰다 | 켜면 `send_action` 이 쓰기 전에 현재 위치를 한 번 더 읽는다. ROS2 길에는 없는 단계라 (a) 에만 시간이 더해진다 |
| 서보의 가속도 · 속도 | 두 길 모두 `Acceleration` 50 · `Goal_Velocity` 2400 | 드라이버는 매 `write()` 마다 이 두 값을 목표 위치와 한 패킷으로 보낸다 (`feetech_ros2_driver.cpp` 의 상수). LeRobot 은 `connect()` 에서 `Acceleration` 을 254 로 두므로 그대로 재면 서보가 문턱 3틱을 넘는 데 걸리는 시간이 두 길에서 달라진다 (정지에서 3틱: 가속도 5000 tick/s^2 이면 약 35 ms, 25400 이면 약 15 ms). 두 길 공통이어야 할 항이라 (a) 스크립트가 시작할 때 드라이버와 같은 값을 써 넣는다. P · I · D · `Return_Delay_Time` 은 EEPROM 값이라 LeRobot `connect()` 가 쓴 값이 ROS2 에서도 그대로 쓰인다 (URDF 에 해당 param 이 없으면 드라이버는 건드리지 않는다) |

**관측 주기를 맞추는 이유.** 움직임은 "위치를 읽는 순간" 에만 보인다. 10 ms 마다 읽으면 실제보다 평균 5 ms 늦게 알아챈다. (b) 는 `/joint_states` 가 100 Hz 로 나오므로 10 ms 로 정해져 있다. (a) 에서 LeRobot 으로 그보다 촘촘히 읽으면 (a) 만 덜 늦게 보이고, 그만큼 (b)-(a) 가 부풀려진다. (a) 도 10 ms 마다 읽으면 이 늦음이 양쪽에 똑같이 들어가 뺄 때 사라진다. 단, "평균 5 ms" 는 명령이 읽기 주기의 아무 위상에나 고르게 떨어질 때의 값이다. 스크립트가 읽기 도착에 맞춰 깨어난 직후에 명령을 보내면 매번 같은 위상에 보내게 되므로, 두 스크립트 모두 명령 직전에 0-10 ms 의 난수만큼 기다린다.

기본안을 바꾸면 (관절 · 계단 크기 · 문턱) 바꾼 값과 이유를 methodology 에 적는다.

### 5.4 절차

스크립트는 `stage1/scripts/` 에 두 개다. 둘 다 5.3 의 표를 그대로 구현하고, 결과를 `stage1/outputs/` 에 npy (원본 100개, 실패한 반복은 NaN) 와 csv (요약 1행 — 통계 + 측정 조건 + 시작 시각) 로 남긴다 (`outputs/` 는 gitignore 대상).

| 스크립트 | 길 | 실행 |
|---|---|---|
| `measure_latency_ros2.py` | (b) | bringup 이 떠 있는 상태에서 다른 터미널로 `python3 /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/measure_latency_ros2.py`. mock 인지 실제 팔인지는 `/robot_description` 의 플러그인 이름으로 스스로 판정해 `latency_ros2_mock.*` 또는 `latency_ros2_real.*` 로 저장한다 |
| `measure_latency_lerobot.py` | (a) | bringup 을 끈 뒤 `acl` → `python /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/measure_latency_lerobot.py`. 끝날 때 토크를 풀기 전에 Enter 를 기다린다 (팔을 받친다) |

순서는 다음과 같다.

1. **(b) 를 mock 으로 먼저 잰다** (`use_mock_hardware:=true`, 팔 없음). mock 은 받은 명령을 다음 주기에 그대로 현재 위치로 돌려준다. 그래서 이 값은 서보와 시리얼이 빠진 "DDS + 제어 주기 대기" 만의 시간이다. 스크립트 검증이자 (b) 의 하한 대조군이다. 실측 (2026-09-25): 10.3-20.2 ms 에 고르게 퍼지고 mean 15.6 ms — 명령이 제어 주기의 아무 위상에나 떨어져 다음 주기까지 0-10 ms 를 기다리고, mock 이 그 명령을 그다음 주기의 read 에서야 돌려주므로 한 주기 10 ms 가 더 붙는다. DDS 전송 자체는 0.3 ms 안팎이다 (min 10.30 ms)
2. **(b) 를 실제 팔로 잰다.** `so101-attach` → 팔을 낮은 자세로 받치고 bringup → 측정 → 팔을 받치고 종료
3. **스택을 바꾼다.** ROS2 launch 가 완전히 꺼진 것을 확인한 뒤 LeRobot 으로 연결한다 (같은 시리얼 포트)
4. **(a) 를 잰다.** 2번과 같은 날, 같은 자세, 같은 관절로
5. **세 실행의 원본 (npy) 과 요약 (csv) 을 시각을 붙여 보존한다** ([week2_guide](../spike/week2/week2_guide.md) §4.2 의 "결과 파일을 고정해 둔다" 와 같은 방식). 스크립트는 돌릴 때마다 같은 이름에 덮어쓰므로, 기록에 쓸 실행을 마친 직후 아래를 그대로 실행한다. 로컬 `outputs/evidence/` (gitignore) 와 `Measurements/so101-dual-latency/raw/` (git 정본) 두 곳에 같은 STAMP 로 남는다.

   ```bash
   STAMP=$(date +%Y%m%d_%H%M)
   OUT=/workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/outputs
   RAW=/workspace/study/physical-ai-study/Measurements/so101-dual-latency/raw
   mkdir -p $OUT/evidence $RAW
   for f in latency_ros2_mock latency_ros2_real latency_lerobot; do
       cp $OUT/$f.npy $OUT/evidence/${f}_$STAMP.npy
       cp $OUT/${f}_summary.csv $OUT/evidence/${f}_summary_$STAMP.csv
       cp $OUT/$f.npy $RAW/${f}_$STAMP.npy
       cp $OUT/${f}_summary.csv $RAW/${f}_summary_$STAMP.csv
   done
   ls $OUT/evidence $RAW
   ```

### 5.5 값싼 검증 — "돌아간다" 와 "맞다" 는 다르다

| 확인 | 어긋나면 |
|---|---|
| (b)-(a) 가 0 보다 크다 | ROS2 를 거친 길이 직결보다 빠를 수는 없다. t0 · t1 을 찍는 위치나 관측 주기가 두 길에서 다르다 |
| (b) mock 이 (b) 실제 팔보다 작다 | mock 에는 서보와 시리얼이 없다. 크게 나오면 mock 측정이 다른 것을 재고 있다 |
| 문턱을 3틱에서 5틱으로 바꿔도 (b)-(a) 가 거의 같다 (두 스크립트의 `THRESHOLD_TICKS` 를 5 로 바꿔 다시 잰다) | 문턱은 두 길에 똑같이 들어가는 항이다. 달라지면 두 길의 관측 조건이 같지 않다 |
| (선택) `update_rate` 를 100 에서 200 으로 올리면 (b) 가 줄어든다 (`so101_controllers.yaml` 의 값. (b) 스크립트는 관측 주기를 `/joint_states` 주기에서 따라가므로 5 ms 가 되고, 요약 csv 의 `observe_period_ms` 에 그대로 찍힌다) | "스택 안" 에서 가장 큰 몫이 제어 주기 대기라면 줄어야 한다. 줄지 않으면 다른 곳에 시간이 들고 있다 — 그것을 찾는 것이 findings 다 |

### 5.6 기록

[`Measurements/so101-dual-latency/`](../../../Measurements/so101-dual-latency/) 한 곳. 기존 측정들과 같은 구조다 (환경 · 방법 · 결과 세 문서 + `raw/`). `methodology.md` 는 이미 있고, 나머지 둘과 `raw/` 는 세 실행 뒤에 채운다.

| 파일 | 담는 것 |
|---|---|
| `environment.md` | PC · 컨테이너 · ROS 2 Jazzy · ros2_control 버전 · 드라이버 커밋 (`18aed7f`) · lerobot 0.6.2 · `update_rate` ((b) 요약 csv 의 `update_rate_hz`) · 실시간 스케줄링이 꺼져 있다는 사실 (bringup 로그의 `Could not enable FIFO RT scheduling policy`) · LeRobot 에서 위치를 한 번 읽는 데 걸리는 시간 ((a) 요약 csv 의 `read_ms_mean` · `read_ms_max`) · 서보 레지스터 실제값 ((a) 요약 csv 의 `servo_*` 열 — P · I · D · Acceleration · Goal_Velocity · Return_Delay_Time) |
| [`methodology.md`](../../../Measurements/so101-dual-latency/methodology.md) | 5.1 (추론을 뺀 이유) · 5.3 의 표 (바꾼 값 포함 — 요약 csv 의 `step_ticks` · `threshold_ticks` · `observe_period_ms` 와 대조) · 5.4 의 순서 · 5.5 의 검증 결과 · 세 실행의 시작 시각 (요약 csv 의 `started_at`). 결과 수치는 §5 에 실행 뒤 기입 |
| `findings.md` | (a) · (b) · (b) mock · (b)-(a) 의 mean / p95 / std, 분포 그림 (npy 세 개에서 그린다), 시간이 어디에 드는가에 대한 해석 |
| `raw/` | 세 실행의 npy 와 요약 csv 사본 (5.4 순서 5 에서 시각을 붙인 것) |

### 5.7 끝났다고 하기 전에 스스로 답해 보는 질문

1. 추론 시간을 넣고 재면 (b)-(a) 의 평균은 어떻게 되고, 흔들림은 어떻게 되는가?
2. 서보가 명령을 받고 움직이기 시작할 때까지의 지연은 (a) · (b) 값에는 들어 있는데 (b)-(a) 에는 없다. 왜인가?
3. (a) 의 관측 주기를 2 ms 로, (b) 를 10 ms 로 두고 재면 결과가 어느 방향으로 얼마나 틀리는가?
4. `update_rate` 를 올리면 (b) 가 줄어든다. 어디까지 올릴 수 있고, 그 한계를 정하는 것은 무엇인가?
5. 정책을 30 Hz 로 돌릴 때 (action 하나에 33 ms) 이 오버헤드는 큰가 작은가? chunk 하나 (106 ms) 에 견주면?

---

## 자주 발생 문제

| 증상 | 해결 |
|---|---|
| launch 로그에 `Open [/dev/so101_follower]: Bad file descriptor` → `LibSerial::NotOpen` 으로 `ros2_control_node` 가 죽고, spawner 가 `waiting for service /controller_manager/list_controllers` 만 되풀이한다. `/joint_states` 가 안 나온다 | 서보 보드의 USB 가 호스트에 연결돼 있지 않다. **`/dev/so101_follower` 노드가 `ls` 에 보여도 소용없다** — 이 노드는 `so101-attach` 가 `mknod` 로 만든 것이라 USB 를 뽑아도 껍데기가 그대로 남고, 열 때에야 "그런 장치 없음" (ENXIO) 이 난다. 드라이버는 이것을 `Bad file descriptor` 로 찍는다. 확인: `so101-attach list` 의 `serial boards` 아래에 `ttyACM` 줄이 있어야 한다. 비어 있으면 팔로워 보드의 USB-C 를 꽂고 `so101-attach` 를 다시 실행한다 (DC 어댑터와는 별개다 — 보드의 USB 칩은 USB 전원으로 돈다). 이 상태에서 `Ctrl+C` 를 누르면 노드가 스택 트레이스를 찍느라 바로 안 죽고 15초쯤 뒤 강제 종료된다 |
| 포트는 있는데 권한 오류 | 컨테이너는 root 라 `dialout` 그룹은 필요 없다. 호스트에서 직접 돌릴 때만 `dialout` 그룹 추가 |
| 모터 검출 안 됨 | LeRobot 프로세스가 포트 점유 중인지 먼저 확인 → 보드레이트·프로토콜 확인 |
| Joint 이름 mismatch | URDF 와 yaml 의 joint name 동일하게 |
| 위치 단위/오프셋 | URDF: rad. STS3215: 12비트 스텝. lerobot 0.6.2 의 SO 팔로워는 도 단위다 (`use_degrees=True`, 그리퍼만 0-100) — LeRobot 캘리브레이션 오프셋과 드라이버 영점이 일치하는지 대조할 때 단위부터 맞춘다 |

---

## 체크리스트

- [ ] 검증 순서 1-3 통과 (모터 1개 → 데이지체인 → URDF+Foxglove)
- [ ] feetech_ros2_driver 빌드 성공
- [ ] so101_description 패키지 + bringup 동작
- [ ] joint_states 발행 + position 명령 → 모터 동작
- [ ] 이중 latency 측정 기록 ((a) / (b) / (b) mock / 오버헤드 — §5.3 의 정의대로)
