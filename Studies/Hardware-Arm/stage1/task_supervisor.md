# Hardware-Arm Stage 1 - Task Supervisor (W9)

> 시기: 2026.10, W5 뒤 · v2.5 착수 전 목표 (2026-10-03 Stage 1 must 로 추가)
> 목적: AMR 애플리케이션에서 설계해온 상태기계 · 예외 복구 패턴을 팔의 태스크 실행에 옮긴다. 웨이포인트 pick-and-place 를 상태기계로 돌리고, 실패를 감지하면 안전하게 멈춰 운영자를 기다린다. 타깃 JD 의 필요 기술 Task Planning & Execution 과 주요 업무 "실패 상황 예외 처리·복구" 의 직접 근거다
> **설계 원본**: [curriculum-career-fit spec](../../../docs/superpowers/specs/2026-10-03-curriculum-career-fit-design.md) §3 (상태 전이 · 인터페이스 · 감지 초기값 · 시험 기준). 이 문서는 만들고 돌리는 절차와 확정값 기록이다
> **범위 밖**: 정책 (SmolVLA) 실행과 v2.5 측정 — supervisor 는 v2.5 와 분리한다 (spec §8.2). 소프트 리밋 · 토크 상한은 W5 설정을 그대로 쓴다
> **타임박스**: 10h. 넘으면 RETRY 를 잘라 최소선 (감지 → SAFE_STOP → WAIT_OPERATOR) 으로 마감한다. 판정은 10h 시점에 한 번만 한다
> **학습 방식**: §3 의 세션 2 · 3 (상태기계 핵심) 은 LLM 없는 블록으로 짠다 (master roadmap 상시 항목). 막히면 가설을 적고 계속한다

---

## 0. 끝나면 남는 것

- `ros2_pkg/so101_description/scripts/task_supervisor.py` — rclpy 노드
- `ros2_pkg/so101_description/config/task_waypoints.yaml` — 실기 웨이포인트
- 상태 전이 로그 + 짧은 영상 클립 (실기 시험 R1 · R2) — 결과는 §6 기록 표에
- 면접 한 줄: "AMR 에서 설계해온 상태기계 · 복구 패턴을 매니퓰레이터 태스크 실행으로 옮겼다 — 감지 조건 4개, 실패 시 토크를 유지한 채 정지하고 운영자 판단을 기다린다"

---

## 1. 여는 곳 (순서대로)

1. spec §3 — 설계 전체 (상태 전이 다이어그램, 인터페이스 표, 감지 초기값, 시험 기준)
2. `ros2_pkg/so101_description/scripts/soft_stop.py` — 같은 패키지의 rclpy 노드. 서비스 클라이언트 · `ReentrantCallbackGroup` · `MultiThreadedExecutor` · `COMMAND_ORDER` 를 그대로 가져온다
3. [ros2_driver_setup.md](ros2_driver_setup.md) §4.2 (첫 명령은 지금 위치 그대로 + 조금) · §6.4 (soft stop 의 동작과 시험 결과 — 정지 중 들어온 명령은 버려진다, stop 을 두 번 부르면 두 번째는 `success=False`)
4. `scripts/print_joint_command.py` — 웨이포인트 자세를 잡을 때 쓰는 조그 도구

---

## 2. 만들 파일

| 파일 | 내용 |
|---|---|
| `scripts/task_supervisor.py` | rclpy 노드 (노드 이름 `task_supervisor`). 상태기계 + 50 Hz 보간 타이머 + 감지 |
| `config/task_waypoints.yaml` | 단계 목록 (spec §3.6 형식). `config/` 는 이미 설치 대상이다 |
| `CMakeLists.txt` | `install(PROGRAMS ...)` 에 `scripts/task_supervisor.py` 추가 |
| `package.xml` | `<exec_depend>python3-yaml</exec_depend>` 한 줄 (웨이포인트를 PyYAML 로 읽는다. ROS 2 설치에 이미 들어 있지만 rosdep 이 확인하게 적어 둔다) |

- **bringup 에 넣지 않는다.** `ros2 run` 으로 따로 띄운다 — bringup 을 띄울 때마다 태스크 노드가 같이 뜨는 일을 막는다.
- 파라미터: `waypoints_file` (기본값 = 패키지 share 의 `config/task_waypoints.yaml`), `rate_hz` (50), `max_step_rad` (0.01).
- 한계값은 `/robot_description` (URDF 의 `<limit>` — W5 에서 캘리브 범위 ±0.05 rad 로 바꾼 값) 을 한 번 받아 `xml.etree` 로 읽는다. 값을 두 곳에 두지 않는다.
- 새 파일을 추가했으므로 빌드를 다시 한다 — 반드시 시스템 `/usr/bin/colcon`, venv 없이 ([ros2_driver_setup.md](ros2_driver_setup.md) §6.4 의 venv 경고).

```bash
cd /workspace/so101_ws
colcon build --packages-select so101_description --symlink-install
source install/setup.bash
ros2 pkg executables so101_description      # 기대: soft_stop.py, task_supervisor.py
```

---

## 3. 구현 순서 (세션 단위 — 22시 이후 1-2h 세션 하나에 한 줄)

| 세션 | 내용 | 확인 |
|---|---|---|
| 1 | 골격: `/joint_states` · `/robot_description` 구독, 서비스 `~/start` `~/resume` `~/abort`, `~/state` 발행, soft_stop 클라이언트 2개. 상태는 IDLE 만 | mock bringup 위에서 `ros2 service list \| grep task_supervisor` 에 세 서비스 |
| 2 (LLM 없는 블록) | 상태 열거 + 전이 함수 하나 — 상태가 바뀔 때마다 `~/state` 에 `OLD -> NEW: reason` 을 발행한다. PRECHECK → EXECUTING → VERIFYING → DONE 정상 흐름 | M1 |
| 3 (LLM 없는 블록) | 50 Hz 보간 타이머: 명령점에서 단계 목표로 틱당 관절별 최대 0.01 rad. 도달 · 정착 · 타임아웃 판정 (아래 정의) | M1 반복, 단계별 소요 시간 로그 |
| 4 | 감지 → SAFE_STOP → WAIT_OPERATOR, resume · abort | M2 · M3 · M4 · M5 (RETRY 없이 바로 SAFE_STOP) |
| 5 (완성선) | RETRY — 단계의 `retry_from` 으로 돌아가 1회 | M2 · M3 에서 RETRY 1회 확인 |
| 6 | 실기 웨이포인트 기록 (§5.1) + 실기 시험 R1-R3 | §6 기록 표 |

**판정 정의** (초기값은 spec §3.5):

- **EXECUTING**: 보간이 목표에 닿으면 정착 타이머를 시작한다. 정착 1.0 s 안에 모든 관절이 목표의 0.08 rad 이내로 들어오면 VERIFYING. 1.0 s 가 지나도 밖이면 추종 오차 → RETRY. 단계 시작부터 `timeout_s` (기본 5 s) 를 넘기면 타임아웃 → RETRY. `check: grasp` 단계의 gripper 는 추종 오차 검사에서 뺀다 — 물체를 쥐면 명령값에 닿지 않는 것이 정상이다.
- **한계 근접**: EXECUTING 동안 매 틱 `/joint_states` 를 한계값과 비교해, 어느 관절이든 0.03 rad 이내면 RETRY 없이 바로 SAFE_STOP.
- **VERIFYING**: `check` 가 `none` · `tracking` 이면 통과. `grasp` 면 gripper 위치가 -0.80 rad 보다 크면 통과 (물체를 쥐고 있다), 이하면 파지 실패 → RETRY.
- **SAFE_STOP**: 보간 타이머를 멈추고 `/soft_stop/stop` 을 부른다. 응답이 `success=False` 이고 메시지가 "이미 정지 중" 이면 정지된 것으로 본다. 그 밖의 실패는 `~/state` 에 남기고 WAIT_OPERATOR 로 간다 (최후 수단은 USB 분리).
- **PRECHECK** (start · resume 공통): `/joint_states` 수신 확인 → `/soft_stop/release` (이미 활성이라 실패하면 무시) → 명령점을 현재 위치로 다시 맞춘다. start 면 첫 단계부터, resume 이면 실패했던 단계의 `retry_from` 부터 시작한다.

**웨이포인트 값의 규칙**: 파지 단계의 gripper 목표는 빈 손 닫힘 실측 -0.867 rad (W4) 로 둔다. 소프트 리밋 하한 -0.915 와의 거리가 0.048 rad 라 한계 근접 (0.03) 에 걸리지 않는다.

---

## 4. mock 시험

mock 은 받은 명령을 그대로 현재 위치로 돌려준다 (ros2_driver_setup.md §2 의 launch 인자 표). 그래서 정상 흐름과 실패 흐름을 모두 팔 없이 돌릴 수 있다. 시험용 웨이포인트 파일은 `stage1/outputs/task_supervisor/` (gitignore) 에 복사해 고친다.

```bash
# Terminal 1
ros2 launch so101_description bringup.launch.py use_mock_hardware:=true

# Terminal 2
ros2 run so101_description task_supervisor.py --ros-args -p waypoints_file:=<시험 파일>

# Terminal 3
ros2 topic echo /task_supervisor/state

# Terminal 4
ros2 service call /task_supervisor/start std_srvs/srv/Trigger
ros2 service call /task_supervisor/resume std_srvs/srv/Trigger
ros2 service call /task_supervisor/abort std_srvs/srv/Trigger
```

| ID | 시나리오 | 시험 파일 | 통과 기준 |
|---|---|---|---|
| M1 | 정상 흐름 | 파지 단계 `check: none` | IDLE → PRECHECK → (EXECUTING → VERIFYING) × 단계 수 → DONE → IDLE |
| M2 | 타임아웃 | 한 단계 `timeout_s: 0.1` | RETRY 1회 → SAFE_STOP → WAIT_OPERATOR (최소선에서는 RETRY 없이 SAFE_STOP) |
| M3 | 파지 실패 | 원본 그대로 (mock 은 -0.867 을 그대로 돌려줘 빈 손 닫힘이 항상 재현된다) | RETRY 1회 → SAFE_STOP → WAIT_OPERATOR |
| M4 | resume / abort | M2 또는 M3 의 WAIT_OPERATOR 에서 | resume → PRECHECK 재진입 / abort → IDLE |
| M5 | 한계 근접 | 한 단계의 한 관절 목표를 소프트 리밋 밖으로 (리미터가 한계값으로 자른다) | 한계 0.03 rad 이내 진입 시 RETRY 없이 SAFE_STOP |

---

## 5. 실기 시험

전제: W5 안전 기초가 켜진 bringup (`ros2 launch so101_description bringup.launch.py`), LeRobot 프로세스는 꺼져 있다. 큐브 · 목표 칸 · 시작 칸은 W3 에서 마킹한 것을 그대로 쓴다. 터미널 하나에 `/soft_stop/stop` 명령을 쳐 두고 Enter 만 남긴다 (ros2_driver_setup.md §6.4).

### 5.1 웨이포인트 기록

토크가 켜진 bringup 위에서 팔을 손으로 옮길 수는 없다. `print_joint_command.py` 로 한 관절씩 0.05-0.1 rad 씩 조그해 목표 자세를 만들고, 각 자세에서 `/joint_states` 값을 `COMMAND_ORDER` 순서로 옮겨 적는다.

| 단계 | 자세 | check | retry_from |
|---|---|---|---|
| approach | 큐브 위 5 cm | `tracking` | approach |
| descend | 큐브 높이, 그리퍼 열림 | `tracking` | approach |
| grasp | gripper 만 -0.867 | `grasp` | approach |
| lift | 큐브를 든 채 위로 | `tracking` | approach |
| place | 목표 칸 위, 내려놓는 높이 | `tracking` | lift |
| release | gripper 열림 | `none` | place |
| retreat | 휴식 자세 위쪽 | `tracking` | retreat |

- 단계 표는 출발점이다. 큐브 크기 · 카메라 구도에 따라 단계를 나눠도 된다.
- 기록한 파일은 먼저 mock 으로 M1 을 돌려 순서와 값을 확인한다 (Foxglove 3D 패널로 자세를 본다).

### 5.2 시험

| ID | 시나리오 | 통과 기준 |
|---|---|---|
| R1 | 정상 3회 | DONE 3/3, 큐브가 목표 칸에 놓임 |
| R2 | 큐브를 치운 뒤 실행 3회 | 파지 실패 감지 → RETRY 1회 → SAFE_STOP → WAIT_OPERATOR 3/3, 정지 뒤 팔이 토크를 유지 |
| R3 | R2 의 WAIT_OPERATOR 에서 큐브를 다시 놓고 resume 1회 | PRECHECK 재동기화 뒤 튐 없이 재개 → DONE |

- 처음 1회는 낮은 자세 · 느린 보간 (`-p max_step_rad:=0.005`) 으로 돌린다.
- R1 · R2 의 상태 전이는 `ros2 bag record /task_supervisor/state /joint_states` 로 남기고, 영상은 전체 뷰 카메라로 짧게 찍는다 (LeRobot 녹화가 아니라 일반 녹화).

---

## 6. 기록 (실행 후 채운다)

| 항목 | 초기값 (spec §3.5) | 확정값 | 근거 (날짜 · 시험 ID) |
|---|---|---|---|
| 단계 타임아웃 | 5 s | | |
| 추종 오차 문턱 / 정착 시간 | 0.08 rad / 1.0 s | | |
| 파지 실패 문턱 (gripper) | -0.80 rad | | |
| 한계 근접 | 0.03 rad | | |
| 보간 | 50 Hz · 0.01 rad/틱 | | |

| 시험 | 결과 | 날짜 |
|---|---|---|
| M1-M5 | | |
| R1 | | |
| R2 | | |
| R3 | | |
| 타임박스 판정 (10h 시점) | 완성선 / 최소선 | |

---

## 7. 막히면

| 증상 | 먼저 볼 것 |
|---|---|
| 명령을 보내도 팔이 안 움직인다 | soft_stop 이 정지 상태인가 (`ros2 control list_controllers` 에서 `position_controller` 가 inactive). PRECHECK 의 release 가 불렸는가 |
| resume 직후 팔이 튄다 | 명령점을 현재 위치로 다시 맞추지 않았다 — 정지 전의 보간 목표가 남아 있다 |
| mock 에서 VERIFYING 에 못 간다 | 정착 판정에 grasp 단계의 gripper 가 들어가 있지 않은가. 한계값을 `/robot_description` 에서 제대로 읽었는가 (transient local QoS 로 구독해야 늦게 떠도 받는다) |
| 실기에서 추종 오차가 잦다 | 처짐 (W5 실측 0.02 rad 이하) 보다 큰가. 보간을 늦추거나 정착 시간을 늘린 값을 §6 에 근거와 함께 적는다 |
| `ros2 run` 이 실행 파일을 못 찾는다 | `CMakeLists.txt` install 줄 추가 후 다시 빌드했는가, 파일에 실행 권한 (`chmod +x`) 이 있는가 |

---

## 8. 끝났다고 하기 전에 스스로 답해 보는 질문

1. 왜 감지 입력을 `/joint_states` 하나로 제한했는가? 서보의 부하 (present load) 를 매 틱 읽으면 무엇이 달라지는가?
2. 한계 근접은 왜 RETRY 없이 바로 SAFE_STOP 인가?
3. 재시도를 1회로 묶은 이유는? 무한 재시도가 현장에서 만드는 문제는 무엇인가?
4. 이 상태기계에 정책 (SmolVLA) 을 넣는다면 어느 상태가 바뀌고, 성공 판정은 누가 하는가? (v2.5 와 분리한 이유 — spec §8.2)
5. AMR 애플리케이션에서 설계한 상태기계와 무엇이 같고 무엇이 다른가? (면접 답의 뼈대)
