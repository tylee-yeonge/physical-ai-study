# 측정 환경 — so101-dual-latency

> 어떤 장비 · 어떤 버전에서 재었는가. 조건 · 절차 · 수치는 [`methodology.md`](methodology.md), 해석은 [`findings.md`](findings.md).
> 측정일: 2026-09-25 (세 실행 모두 같은 날 01:28-01:51, 같은 컨테이너)

## 1. 하드웨어

| 항목 | 값 |
|---|---|
| 팔 | SO-101 팔로워. Feetech STS3215 서보 6개 (ID 1-6), 서보 버스 1,000,000 baud. 움직인 관절은 ID 1 `shoulder_pan` |
| 서보 보드 | Feetech USB 시리얼 보드, USB CDC-ACM (`ttyACM0`, 시리얼 5B8E115934). 컨테이너 안에서는 `so101-attach` 가 만드는 고정 경로 `/dev/so101_follower` |
| PC | Intel Core i7-14700F (28 스레드), RAM 31 GB. 호스트 커널 6.8.0-138-generic |
| 실시간 스케줄링 | 꺼짐. 컨테이너에 권한이 없어 ros2_control 제어 루프가 보통 우선순위로 돈다 (bringup 로그 `Could not enable FIFO RT scheduling policy: ... Operation not permitted`). 그래도 `/joint_states` 는 실측 100.0 Hz |
| 팔 자세 | 낮은 자세로 받쳐 둔 상태. (a) 와 (b) 실제 팔 사이에 bringup 을 껐다 켰으므로 (토크 해제) 두 실행의 시작 자세가 같았는지는 기록으로 확인할 수 없다 — 스크립트가 시작 틱을 csv 에 남기지 않는다 (findings.md §4) |

## 2. 소프트웨어

| 항목 | 값 |
|---|---|
| 컨테이너 | Docker, Ubuntu 24.04.1 LTS, root. 베이스 이미지 `nvidia/cuda:12.6.3-cudnn-devel-ubuntu24.04`. 호스트 compose 프로젝트 `vscode-tunnel` v1.18.0 (ros2_control 빌드 · 실행 패키지 내장) |
| ROS 2 | Jazzy (`ros-jazzy-desktop` 0.11.0). RMW `rmw_fastrtps_cpp` 8.4.4 |
| ros2_control | `controller_manager` · `hardware_interface` · `ros2_control` 4.48.0. `ros2_controllers` (`joint_state_broadcaster`, `forward_command_controller`) 4.42.1 |
| 하드웨어 드라이버 | `feetech_ros2_driver` 커밋 `18aed7f` (v0.2.2, 2026-07-21). `/workspace/so101_ws` 에서 colcon 빌드 |
| ROS 2 패키지 | `so101_description` (이 레포 `Studies/Hardware-Arm/stage1/ros2_pkg/`). `update_rate: 100`, `forward_command_controller` (position), `joint_state_broadcaster`. mock 은 같은 launch 에 `use_mock_hardware:=true` (`mock_components/GenericSystem`) |
| LeRobot | 0.6.2. venv `/workspace/venvs/lerobot` — Python 3.12.3, numpy 2.2.6, pyserial 3.5. 로봇 클래스 `SO101Follower` (`use_degrees=True`, `max_relative_target=None`, 카메라 없음) |
| (b) 스크립트의 파이썬 | 시스템 Python 3.12.3 + rclpy (Jazzy), numpy 1.26.4 |
| 측정 스크립트 | `Studies/Hardware-Arm/stage1/scripts/measure_latency_ros2.py` ((b)), `measure_latency_lerobot.py` ((a)). 코드는 커밋 `fba60b7` 과 같다 (그 뒤 docstring 의 실행 경로 표기만 바뀜) |
| 캘리브레이션 | `$HF_LEROBOT_CALIBRATION/robots/so_follower/so101_follower_01.json` — `shoulder_pan` range 930-3169, homing_offset -1931. LeRobot 의 도 단위 변환 기준이다. ROS 2 드라이버는 이 파일을 쓰지 않고 (틱 - 2048) 을 rad 로 바꾼다. 두 경로가 같은 서보 레지스터를 읽으므로 계단 33틱 · 문턱 3틱은 어느 쪽 변환과도 무관하다 |
| 분포 그림 | `scripts/plot_latency_distribution.py`, 시스템 Python 3.12.3 + matplotlib 3.11.2 |

## 3. 서보 레지스터 (`shoulder_pan`, (a) 실행 시작 시 읽은 값)

| 레지스터 | 값 | 비고 |
|---|---|---|
| `P_Coefficient` / `I_Coefficient` / `D_Coefficient` | 16 / 0 / 32 | EEPROM. LeRobot `connect()` 가 쓴다. ROS 2 드라이버는 URDF 에 param 이 없어 건드리지 않는다 — 두 경로에서 같다 |
| `Return_Delay_Time` | 0 | EEPROM. 위와 같다 |
| `Maximum_Acceleration` | 254 | EEPROM. LeRobot `configure_motors()` 가 쓴다 |
| `Acceleration` | 50 | RAM. (a) 스크립트가 시작할 때 씀. (b) 는 드라이버가 매 `write()` 마다 목표 위치와 한 패킷으로 50 을 보낸다 |
| `Goal_Velocity` | 2400 | RAM. 위와 같다 |

## 4. 실행 타임라인 (2026-09-25)

| 시각 | 실행 | bringup |
|---|---|---|
| 01:28:47 | (b) 실제 팔 | 전날 23:58:41 에 띄운 실제 팔 bringup (`feetech_ros2_driver/FeetechHardwareInterface`). 실행 중 로그에 오류 없음. 01:44:45 정상 종료 |
| 01:45:15 | (b) mock | 01:44:58 에 띄운 mock bringup (`mock_components/GenericSystem`) |
| 01:47:17 | — | 실제 팔 bringup 을 다시 띄움 (오조작). LeRobot 스크립트가 `Device or resource busy` 로 실패. 01:49:05 종료 |
| 01:49:38 | (a) LeRobot | 없음 (LeRobot 이 포트를 소유) |
| 01:59 | `outputs/evidence/` 와 `raw/` 로 복사 | STAMP `20260925_0151` (세 실행을 한 STAMP 로 묶음). 세 곳의 md5 일치 확인 |

실행 순서가 methodology §3 의 순서 (mock → 실제 팔 → LeRobot) 와 다르다 (실제 팔 → mock → LeRobot). mock 은 팔과 무관하므로 결과에 영향이 없다.

## 5. 부수 실측

| 항목 | 값 |
|---|---|
| LeRobot 에서 `shoulder_pan` 의 `Present_Position` 을 한 번 읽는 시간 | mean 0.40 ms, max 2.0 ms ((a) 실행 중의 모든 읽기) |
| `/joint_states` 주기 | 100.0 Hz (`ros2 topic hz`, 50 샘플 창) |
| (b) mock 의 최솟값 | 10.30 ms — DDS 전송 + 제어 주기 1개. DDS 자체는 0.3 ms 안팎 |
