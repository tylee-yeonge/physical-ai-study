# 측정 조건·절차 — so101-dual-latency

> 측정 대상: SO-101 팔로워 (Feetech STS3215 서보 6개, USB 시리얼 1 Mbaud) 에 위치 명령 1개를 보내 **관절이 움직이기 시작할 때까지의 시간**. 두 경로 — (a) LeRobot 직결, (b) ROS 2 (ros2_control + feetech_ros2_driver) 경유 — 와 그 차이 **(b)-(a) = 통합 오버헤드**
> 측정 코드: [`measure_latency_ros2.py`](../../Studies/Hardware-Arm/stage1/scripts/measure_latency_ros2.py) ((b)), [`measure_latency_lerobot.py`](../../Studies/Hardware-Arm/stage1/scripts/measure_latency_lerobot.py) ((a)). 실행에 쓴 커밋은 `environment.md` 에 적는다
> 원본 데이터: `raw/` — 세 실행 ((b) mock · (b) 실제 팔 · (a)) 의 npy (ms 배열, n=100, 실패는 NaN) 와 요약 csv (1행)
> 상태: **측정 완료 (2026-09-25, 세 실행 n=100, 실패 0).** 수치는 §5. 검증 6개 중 4개 통과, 2개 (문턱 5틱 · `update_rate` 200) 미실시 — §4. 환경은 [`environment.md`](environment.md)

## 0. 이 문서가 답하는 질문

> **"(b)-(a) 라는 숫자는 정확히 무엇에서 무엇을 뺀 값인가."**

"ROS 2 를 거치면 몇 ms 느려진다" 는 수치는 t0 · t1 을 어디서 찍었는지, 두 경로에서 관측 조건이 같았는지에 따라 전혀 다른 값이 된다. 이 문서는 그 조건을 착수 전에 전부 적어 두고, 측정은 여기 적힌 대로만 한다. 조건을 바꾸면 이 문서를 바꾼 뒤에 잰다.

문서 구성:

| 절 | 내용 |
|---|---|
| §1 | 무엇을 재고 무엇을 빼는가 (1회의 정의, 두 경로에서 시간이 드는 곳, 추론을 뺀 이유) |
| §2 | 측정 조건 (고정된 값과 그 이유, 두 경로에서 같아야 하는 것) |
| §3 | 절차 (실행 순서와 명령, 원본 보존) |
| §4 | 검증 기준 (수치가 "맞다" 고 보기 위한 조건) |
| §5 | 결과 통계 (실행 뒤 기입) |

해석 ("왜 그 값인가", "그래서 무엇을 판단했는가") 은 `findings.md` 가 담당한다. 수치의 원본은 이 문서 한 곳이다.

### 0.1 미리 풀어 두는 용어

| 용어 | 뜻 |
|---|---|
| **latency** | 명령을 넘긴 순간 (t0) 부터 관절이 움직이기 시작한 것을 관측한 순간 (t1) 까지의 시간. 이 측정에서는 t1 - t0 |
| **틱**(tick) | 서보 엔코더의 최소 단위. STS3215 는 1바퀴 4096틱, 1틱은 약 0.088도 (0.0015 rad) |
| **계단 명령** | 목표 위치를 한 번에 바꾸는 명령. 보간 없이 "지금부터 저기로" |
| **문턱** | 위치가 계단 전 값에서 이만큼 넘게 벗어나야 "움직였다" 로 본다. 서 있는 서보도 1-2틱은 떨리므로 필요하다 |
| **관측 주기** | 위치를 읽는 간격. 움직임은 읽는 순간에만 보이므로 이 간격이 t1 의 해상도다 |
| **제어 주기**(`update_rate`) | ros2_control 이 서보를 읽고 (read) 명령을 쓰는 (write) 루프의 주기. 100 Hz = 10 ms |
| **DDS** | ROS 2 의 통신 계층. 토픽 메시지가 프로세스 사이를 오가는 길 |
| **`forward_command_controller`** | 토픽으로 받은 목표를 보간 · 속도 제한 없이 그대로 하드웨어에 넘기는 ros2_control 컨트롤러 |
| **`header.stamp`** | ROS 메시지에 실린 "이 상태를 읽은 시각". 메시지가 도착한 시각과 다르다 |
| **mock** | 서보 대신 쓰는 가짜 하드웨어 (`mock_components/GenericSystem`). 받은 명령을 다음 주기에 그대로 현재 위치로 돌려준다 |
| **warm-up** | 본 측정 전에 버리는 예비 반복. 첫 호출에는 초기화 비용이 섞인다 |
| **mean / median / std / p95 / p99** | 평균 / 중앙값 / 표준편차 / 정렬했을 때 하위 95% · 99% 지점 ("느린 쪽 꼬리") |
| **정지 오차** | 목표로 보냈는데 몇 틱 못 미친 자리에 멈추는 것. 서보 위치 루프의 마찰 · 하중 때문이며 정상이다 |
| **`Acceleration` · `Goal_Velocity`** | 서보가 목표까지 갈 때 쓰는 가속도 · 최고 속도 레지스터. 움직이기 시작하는 시간에 직접 영향을 준다 |

## 1. 무엇을 재고 무엇을 빼는가

### 1.1 1회의 정의

**1회 = 한 관절에 계단 명령 1개를 보내고 (t0), 그 관절이 움직이기 시작한 것을 관측할 때까지 (t1) 의 시간 t1 - t0.** 이것을 (a) 와 (b) 각각 100회 재고, 두 mean 의 차 (b)-(a) 를 통합 오버헤드로 정의한다.

### 1.2 두 경로에서 시간이 드는 곳

| 단계 | (a) LeRobot 직결 | (b) ROS 2 경유 |
|---|---|---|
| 명령을 넘긴다 (t0) | `robot.send_action()` 을 부르기 직전 | `/position_controller/commands` 에 publish 하기 직전 |
| 스택 안 | 함수 호출 → 시리얼 쓰기 | DDS 전송 → 컨트롤러가 받아서 보관 → **다음 제어 주기까지 대기** (100 Hz 면 0-10 ms) → 드라이버 `write()` → 시리얼 쓰기 |
| 두 경로 공통 | USB · 서보 버스 전송 → 서보가 움직이기 시작 | 같음 |
| 움직임을 본다 (t1) | `Present_Position` 을 읽어 변화 감지 | `/joint_states` 에서 변화 감지 |

"두 경로 공통" 은 뺄 때 사라진다. 남는 것이 "스택 안" 의 차이이고, 그것이 이 측정이 알고 싶은 값이다. 뺄셈이 성립하려면 공통 항이 실제로 같아야 한다 — §2.2 가 그 점검표다.

### 1.3 추론 시간을 넣지 않는 이유

정책 (VLA) 의 추론은 두 경로에 똑같이 들어가는 항이다. 넣어도 뺄 때 사라지고, 추론 시간의 흔들림 (스파이크 실측 표준편차 1.6 ms) 만 남아 몇 ms 짜리 오버헤드를 가린다. 그래서 추론을 빼고 **명령이 전달되는 길만** 잰다. 스파이크의 추론 latency 측정 (SmolVLA, RTX 4070, n=100) 에서 가져오는 것은 수치가 아니라 방법 — n=100, warm-up 5, mean / p95 / std, 원본 npy + 요약 csv, 환경 기록 — 이다.

## 2. 측정 조건 (고정)

| 항목 | 값 | 출처 |
|---|---|---|
| 움직일 관절 | `shoulder_pan` (ID 1) | 두 스크립트 `JOINT` |
| 계단 크기 | 33틱 (0.0506 rad, 2.90도) | `STEP_TICKS` |
| 목표 | 시작 위치 (low) 와 시작 + 33틱 (high) 두 절대 목표를 번갈아 보낸다. 마지막에 low 로 되돌린다 | 두 스크립트의 반복 루프 |
| 나머지 관절 5개 | (b) 는 시작할 때 읽은 위치를 그대로 목표로 함께 보낸다 (컨트롤러가 6개 값을 전부 목표로 쓰므로). (a) 는 `shoulder_pan` 만 보낸다 | `hold_ticks` / `send_action` 인자 |
| t0 | (a) `send_action()` 직전의 `time.perf_counter()`. (b) publish 직전의 ROS 시각 (`node.get_clock().now()`, 시스템 시계) | 두 스크립트 |
| t1 | (a) 문턱을 넘은 첫 읽기가 **돌아온** 시각 (`perf_counter`). (b) 문턱을 넘은 첫 `/joint_states` 의 **`header.stamp`** (메시지 도착 시각이 아니다) | 두 스크립트 |
| 계단 전 값 | 명령 직전의 마지막 읽기 (a) / 마지막 `/joint_states` 샘플 (b) | 두 스크립트 `baseline` |
| 문턱 | 3틱 (0.26도) 넘게 벗어난 첫 샘플. "넘게" 는 두 스크립트 모두 `>` 비교라 4틱 이상이 처음 보인 샘플이다 | `THRESHOLD_TICKS` |
| 관측 주기 | 두 경로 모두 10 ms. (b) 는 `/joint_states` 의 주기 (= 제어 주기), (a) 는 스크립트 시작 시점에 고정한 10 ms 격자 위에서만 읽는다 | `OBSERVE_PERIOD_S`, `update_rate: 100` |
| 명령 시각의 위상 | 두 경로 모두 명령 직전에 0-10 ms 의 난수만큼 기다린다 | 두 스크립트 `random.uniform` |
| 반복 사이 | "멈춤" (최근 20 샘플 = 0.2 s 가 2틱 안) 을 확인한 뒤 0.5 s | `STOP_WINDOW` · `STOP_SPAN_TICKS` · `SETTLE_S` |
| n / warm-up | 100 / 5. warm-up 도 팔을 움직이지만 수치는 버린다 | `N_ITER` · `N_WARMUP` |
| 실패 | 1.0 s 안에 문턱을 못 넘으면 그 반복은 NaN. 통계는 NaN 을 뺀 값으로 내고 실패 수를 따로 적는다 | `MOTION_TIMEOUT_S` |
| LeRobot `max_relative_target` | `None` (끔) | (a) `SO101FollowerConfig` |
| 서보 가속도 · 속도 | 두 경로 모두 `Acceleration` 50 · `Goal_Velocity` 2400. (b) 는 드라이버가 매 `write()` 마다 목표 위치와 한 패킷으로 보내는 상수. (a) 는 스크립트가 시작할 때 같은 값을 써 넣고, 읽어서 요약 csv 에 남긴다 | `feetech_ros2_driver.cpp` `write()`, (a) `SERVO_ACCELERATION` · `SERVO_GOAL_VELOCITY` |
| 서보 P · I · D · `Return_Delay_Time` | EEPROM 값. LeRobot `connect()` 가 쓴 값 (P 16 · I 0 · D 32 · 지연 0) 이 두 경로에서 그대로 쓰인다. 드라이버는 URDF 에 해당 param 이 없으면 건드리지 않는다 | `so_follower.py` `configure()`, URDF `<ros2_control>` |
| 읽는 레지스터 | 두 경로 모두 `Present_Position` 원값 (틱). (a) 는 `normalize=False` 로 읽고, (b) 는 드라이버가 (틱 - 2048) x 2pi / 4096 로 바꾼 rad 를 `/joint_states` 로 받는다 | 두 스크립트 |
| 제어 주기 | 100 Hz. 실시간 스케줄링은 꺼져 있다 (컨테이너에 권한 없음 — bringup 로그 `Could not enable FIFO RT scheduling policy`) | `so101_controllers.yaml`, `environment.md` |
| 팔 자세 | 낮은 자세로 받쳐 둔다. (a) 와 (b) 실제 팔은 같은 날, 같은 자세, 같은 관절 | §3 |

### 2.1 각 조건이 왜 그 값인가

| 조건 | 막는 오측정 |
|---|---|
| `shoulder_pan` | 중력을 받지 않는 관절이라 도는 방향에 따라 응답이 달라지지 않는다. 다른 관절은 올릴 때와 내릴 때가 다르다 |
| 계단 33틱 (0.05 rad 근처) | 작아서 안전하다. rad 가 아니라 틱으로 정한 것은 두 스택의 rad · 도 → 틱 변환 (4096 분모 · 4095 분모 · 0 쪽 버림) 이 달라서, rad 로 정하면 같은 틱 수를 보낼 수 없기 때문이다 |
| 절대 목표 두 개를 오간다 | 매번 "현재 위치 + 33틱" 으로 보내면 정지 오차 (실측 약 5틱) 가 반복마다 쌓여 팔이 한쪽으로 흘러간다 |
| 문턱 3틱 | 서 있는 서보의 1-2틱 떨림을 움직임으로 잡지 않는다. 계단 33틱에 견주면 충분히 작아 시작 시점을 놓치지 않는다 |
| (b) 의 t1 = `header.stamp` | 상태가 되돌아오는 길 (드라이버 → 브로드캐스터 → DDS → 스크립트) 의 시간은 명령이 팔에 닿는 시간이 아니다. 도착 시각으로 재면 그것이 (b) 에만 더해진다 |
| (b) 의 t0 를 ROS 시각으로 | `header.stamp` 와 같은 시계여야 뺄 수 있다. `perf_counter` 는 원점이 다른 시계다 |
| (a) 의 t1 = 읽기가 돌아온 시각 | (b) 의 `header.stamp` 도 하드웨어 read 뒤에 찍힌다. 같은 위치에서 찍어야 뺄셈이 성립한다 |
| 관측 주기 10 ms 를 두 경로에 | 10 ms 마다 읽으면 실제보다 평균 5 ms 늦게 알아챈다. (a) 만 촘촘히 읽으면 (a) 만 덜 늦게 보이고 그만큼 (b)-(a) 가 부풀려진다. 같은 간격이면 이 늦음이 양쪽에 똑같이 들어가 뺄 때 사라진다 |
| 명령 시각의 위상을 난수로 | "평균 5 ms" 는 명령이 관측 주기의 아무 위상에나 고르게 떨어질 때의 값이다. 두 스크립트 모두 읽기 도착에 맞춰 깨어나므로, 그냥 보내면 매번 같은 위상에 보내게 돼 늦음이 5 ms 가 아닌 고정값이 된다 |
| 멈춤 확인 + 0.5 s | 앞 반복의 움직임이 다음 반복의 "계단 전 값" 에 섞이지 않게 한다 |
| n = 100, warm-up 5 | 우연한 느린 반복 하나가 평균을 흔들지 않게 하고, 첫 반복의 초기 비용 (DDS 디스커버리, 캐시) 을 버린다 |
| `max_relative_target` 끔 | 켜면 `send_action` 이 쓰기 전에 현재 위치를 한 번 더 읽는다 (시리얼 왕복 1회). ROS 2 경로에는 없는 단계라 (a) 에만 시간이 더해진다 |
| 서보 가속도 · 속도를 맞춘다 | LeRobot 기본은 `Acceleration` 254 다. 드라이버의 50 과 다르면 정지 상태에서 3틱을 넘는 시간이 약 15 ms 와 약 35 ms 로 갈린다 (가속도 25400 tick/s^2 대 5000 tick/s^2). "두 경로 공통" 이어야 할 서보 응답이 재려는 오버헤드보다 크게 달라져 (b)-(a) 가 무의미해진다 |
| 실패를 NaN 으로 남긴다 | 실패한 반복을 지우면 n 이 줄어든 사실이 사라진다. 실패 수 자체가 결과다 |

### 2.2 두 경로에서 같아야 하는 것 (점검표)

뺄셈이 성립하는 조건이다. 실행 전에 하나씩 확인한다.

| 항목 | (a) | (b) | 같은가 |
|---|---|---|---|
| 계단 | 33틱 | 33틱 | 같다 (틱으로 정했다) |
| 문턱 | 3틱 | 3틱 | 같다 |
| 관측 주기 | 10 ms 격자 | `/joint_states` 10 ms | 같다 (`update_rate` 를 바꾸면 (a) 의 `OBSERVE_PERIOD_S` 도 같이 바꾼다) |
| 명령 위상 | 0-10 ms 난수 | 0-10 ms 난수 | 같다 |
| 읽는 값 | `Present_Position` 원값 | 같은 레지스터의 rad 변환 | 같다 |
| t1 의 위치 | 읽기 뒤 | read 뒤 (`header.stamp`) | 같다 |
| 서보 가속도 · 속도 | 50 · 2400 (스크립트가 씀) | 50 · 2400 (드라이버 상수) | 같다 |
| 서보 P · I · D | EEPROM | EEPROM | 같다 |
| 팔 자세 · 날짜 | 같은 날 같은 자세 | 같은 날 같은 자세 | 절차로 보장 (§3) |

## 3. 절차

순서가 고정이다. (b) mock 은 팔이 필요 없고, (b) 실제 팔과 (a) 는 같은 시리얼 포트를 쓰므로 동시에 한 스택만 붙인다.

**사전 조건**: Stage 1 W5 의 안전 기초 (소프트 리밋 · 토크 상한 · 소프트웨어 정지) 가 서 있다. 팔은 낮은 자세로 받쳐 둔다. 세션마다 `so101-attach` 를 먼저 실행한다.

**1. (b) 를 mock 으로**

```bash
# Terminal 1
ros2 launch so101_description bringup.launch.py use_mock_hardware:=true

# Terminal 2
python3 /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/measure_latency_ros2.py
# 저장: Studies/Hardware-Arm/stage1/outputs/latency_ros2_mock.npy, latency_ros2_mock_summary.csv
```

서보와 시리얼이 빠진 "DDS + 제어 주기 대기" 만의 시간이다. 스크립트 검증이자 (b) 의 하한 대조군이다.

**2. (b) 를 실제 팔로**

```bash
# Terminal 1 (mock 을 Ctrl+C 로 끈 뒤)
so101-attach
ros2 launch so101_description bringup.launch.py

# Terminal 2
python3 /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/measure_latency_ros2.py
# 저장: Studies/Hardware-Arm/stage1/outputs/latency_ros2_real.npy, latency_ros2_real_summary.csv
```

**3. 스택 전환**: Terminal 1 에서 `Ctrl+C` (드라이버가 토크를 끄므로 팔을 받친 채). `ros2 node list` 에 아무것도 없는 것을 확인한다.

**4. (a) 를 LeRobot 으로** — 2 와 같은 날, 같은 자세, 같은 관절

```bash
acl
python /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/measure_latency_lerobot.py
# 끝에 "팔을 받친 뒤 Enter" 를 기다린다. Enter 를 누르면 토크가 풀린다
# 저장: Studies/Hardware-Arm/stage1/outputs/latency_lerobot.npy, latency_lerobot_summary.csv
```

**5. 원본 보존** — `outputs/` 는 gitignore 대상이고 다음 실행이 같은 이름에 덮어쓴다. 기록에 쓸 실행을 마친 직후 시각을 붙여 두 곳에 복사한다: 로컬 `outputs/evidence/` (스파이크 week2 와 같은 방식, gitignore) 와 이 디렉토리의 `raw/` (git 에 남는 정본). 세 실행을 한 STAMP 로 묶는다. §5 의 표는 `raw/` 의 사본을 가리킨다.

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

## 4. 검증 기준

"돌아간다" 와 "맞다" 는 다르다. 세 실행이 끝난 뒤 아래를 순서대로 확인한다. 결과 열은 2026-09-25 실행 기준이다.

| 확인 | 어긋나면 | 결과 (2026-09-25) |
|---|---|---|
| (b) mock 이 제어 주기의 1-2배 (100 Hz 면 10-20 ms) 에 고르게 퍼진다 | 명령이 주기의 아무 위상에나 떨어져 다음 주기까지 0-10 ms 를 기다리고, mock 이 그 명령을 그다음 주기의 read 에서 돌려주므로 10 ms 가 더 붙는다. 이 범위 밖이면 t0 · t1 을 찍는 위치가 틀렸거나 다른 프로세스가 제어 루프를 밀고 있다 | **통과.** 10.30-20.22 ms. 2 ms 구간별 도수 21 / 11 / 26 / 23 / 19 (10-20 ms 를 다섯 칸으로) |
| (b) mock 이 (b) 실제 팔보다 작다 | mock 에는 서보와 시리얼이 없다. 크게 나오면 mock 측정이 다른 것을 재고 있다 | **통과.** 15.6 < 102.8 ms |
| (b)-(a) 가 0 보다 크다 | ROS 2 를 거친 길이 직결보다 빠를 수는 없다. t0 · t1 을 찍는 위치나 관측 주기가 두 경로에서 다르다 | **통과.** +37.5 ms (mean 차의 95% 신뢰구간 ±1.3 ms) |
| 실패 (NaN) 가 0 이거나 그에 가깝다 | 문턱을 1 s 안에 못 넘었다면 명령이 닿지 않았거나 서보가 안 움직였다. 실패 수를 그대로 적고 원인을 findings 에 쓴다 | **통과.** 세 실행 모두 0 / 100 |
| 문턱을 3틱에서 5틱으로 바꿔도 (b)-(a) 가 거의 같다 (두 스크립트의 `THRESHOLD_TICKS` 를 5 로 바꿔 다시 잰다) | 문턱은 두 경로에 똑같이 들어가는 항이다. 달라지면 두 경로의 관측 조건이 같지 않다 | 미실시 |
| (선택) `update_rate` 를 100 에서 200 으로 올리면 (b) 가 줄어든다 (`so101_controllers.yaml`. (a) 의 `OBSERVE_PERIOD_S` 도 0.005 로 맞춘다) | "스택 안" 에서 가장 큰 몫이 제어 주기 대기라면 줄어야 한다. 줄지 않으면 다른 곳에 시간이 들고 있다 — 그것을 찾는 것이 findings 다 | 미실시. findings.md §4 의 첫 실험이다 |

§2.2 점검표의 "팔 자세 · 날짜" 는 날짜만 확인된다 (세 실행 모두 2026-09-25 01:28-01:51). 자세는 (a) 와 (b) 사이에 bringup 을 껐다 켜서 (토크 해제) 같았는지 기록으로 확인할 수 없다 — 스크립트가 시작 틱을 csv 에 남기지 않는다.

## 5. 결과 통계 (2026-09-25)

원본: `raw/latency_ros2_mock_20260925_0151.npy`, `raw/latency_ros2_real_20260925_0151.npy`, `raw/latency_lerobot_20260925_0151.npy` (각 float 100개, ms, NaN 없음). 통계의 원천은 같은 이름의 `_summary_20260925_0151.csv` 다. 같은 파일이 로컬 `Studies/Hardware-Arm/stage1/outputs/evidence/` 에도 있다. 분포 그림은 [`plots/latency_distribution.png`](plots/latency_distribution.png) (`scripts/plot_latency_distribution.py` 가 raw/ 에서 그린다).

| 실행 | n | 실패 | mean | median | std | min | max | p95 | p99 |
|---|---|---|---|---|---|---|---|---|---|
| (b) ROS 2 경유, mock | 100 | 0 | 15.58 | 16.04 | 3.03 | 10.30 | 20.22 | 19.92 | 20.15 |
| (a) LeRobot 직결 | 100 | 0 | 65.37 | 65.14 | 3.28 | 59.10 | 71.89 | 70.99 | 71.21 |
| (b) ROS 2 경유, 실제 팔 | 100 | 0 | 102.83 | 103.06 | 5.33 | 90.97 | 119.71 | 109.71 | 117.27 |
| **(b)-(a) 통합 오버헤드** | | | **37.46** | | (b) 5.33 · (a) 3.28 | | | | |

단위는 ms. (b)-(a) 는 두 mean 의 차이고, 두 실행의 std (5.33 · 3.28) 에 견주면 차이는 흔들림보다 열 배 크다 (mean 차의 95% 신뢰구간 ±1.25 ms). (b) 실제 팔 - (b) mock = 87.25 ms.

부수 통계 (해석은 findings.md):

| 항목 | (b) mock | (a) | (b) 실제 팔 |
|---|---|---|---|
| +33틱 방향 mean / std | 15.73 / 3.01 | 65.71 / 3.23 | 100.69 / 6.20 |
| -33틱 방향 mean / std | 15.42 / 3.05 | 65.04 / 3.29 | 104.97 / 3.03 |
| 처음 10회 / 마지막 10회 mean | 15.37 / 13.87 | 64.90 / 64.83 | 105.07 / 101.63 |
| 값을 10 ms 로 나눈 나머지의 2 ms 구간별 도수 | — | 24 / 28 / 16 / 17 / 15 | 21 / 11 / 26 / 23 / 19 |

실행 순서는 실제 팔 (01:28) → mock (01:45) → LeRobot (01:49) 였다. §3 의 순서와 다르지만 mock 은 팔과 무관하므로 결과에 영향이 없다.
