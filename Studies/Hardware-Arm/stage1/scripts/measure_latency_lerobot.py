"""LeRobot 직결 명령 latency 측정 -- Stage 1 이중 latency 의 (a) (ros2_driver_setup.md §5).

무엇을 재는가 (§5.3):
    1회 = robot.send_action() 으로 shoulder_pan 에 계단 명령 1개를 보내고 (t0), Present_Position
    을 10 ms 마다 읽어 계단 전 값에서 3틱 넘게 벗어난 첫 읽기가 돌아온 시각 (t1) 까지.
    t1 - t0 에는 함수 호출 + 시리얼 쓰기 + 서보가 움직이기 시작하는 시간이 들어 있다
    (§5.2 의 (a) 열). t1 을 "읽기가 돌아온 시각" 으로 잡는 것은 (b) 의 header.stamp 가
    하드웨어 read 뒤에 찍히는 것과 같은 위치다.

(b) 와 조건을 맞춘 것 (§5.3 의 표):
    - 관측 주기 10 ms: /joint_states 가 100 Hz 로 나오는 (b) 와 같은 간격으로 읽는다. 읽는 시각은
      스크립트 시작 시점에 고정한 10 ms 격자이고, 명령을 보내는 시각은 그 격자와 무관하게 0-10 ms
      의 난수만큼 흩는다. (b) 에서 명령이 제어 주기의 아무 위상에나 떨어지는 것과 같게 하기 위해서다
    - max_relative_target=None: 켜면 send_action 이 쓰기 전에 현재 위치를 한 번 더 읽어 (a) 에만
      시간이 더해진다
    - 서보의 가속도 · 속도: ROS2 드라이버는 매 write 마다 Acceleration 50 · Goal_Velocity 2400 을
      목표 위치와 한 패킷으로 보낸다 (feetech_ros2_driver.cpp write()). LeRobot 은 connect() 에서
      Acceleration 을 254 로 두므로 그대로 재면 서보가 3틱을 넘는 데 걸리는 시간이 두 길에서
      달라진다 (정지 상태에서 3틱: 가속도 5000 tick/s^2 이면 약 35 ms, 25400 이면 약 15 ms).
      두 길 공통이어야 할 항이라 이 스크립트가 시작할 때 드라이버와 같은 값을 써 넣는다.
      P · I · D · Return_Delay_Time 은 EEPROM 값이라 LeRobot connect() 가 쓴 값이 ROS2 에서도
      그대로 쓰인다 (드라이버는 URDF 에 해당 param 이 없으면 건드리지 않는다).

실행 (ROS2 bringup 이 완전히 꺼진 뒤. 같은 시리얼 포트를 쓴다):
    so101-attach
    acl
    python /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/measure_latency_lerobot.py
출력 (outputs/ 는 gitignore 대상 -- 기록에 쓸 실행은 §5.4 순서 5 대로 시각을 붙여 복사한다):
    Studies/Hardware-Arm/stage1/outputs/latency_lerobot.npy (+ latency_lerobot_summary.csv)

shoulder_pan 이 33틱 (약 2.9도) 을 105번 오간다. 팔은 낮은 자세로 받쳐 둔다 -- connect() 의
configure() 가 토크를 잠깐 껐다 켜고, 끝날 때 disconnect() 가 토크를 끈다 (끄기 전에 묻는다).
"""

import csv
import datetime
import math
import os
import random
import sys
import time
from typing import List

import numpy as np

import lerobot
from lerobot.robots.so_follower import SO101Follower
from lerobot.robots.so_follower import SO101FollowerConfig

PORT = "/dev/so101_follower"  # so101-attach 가 만드는 고정 경로. URDF 의 usb_port 와 같다
ROBOT_ID = "so101_follower_01"  # 캘리브 파일 $HF_LEROBOT_CALIBRATION/robots/so_follower/<id>.json
JOINT = "shoulder_pan"  # 움직일 관절. 중력을 받지 않아 방향에 따라 응답이 달라지지 않는다
STEP_TICKS = 33  # 계단 크기. 0.05 rad 에 가장 가까운 정수 틱 (0.05 * 4096 / 2pi = 32.6)
THRESHOLD_TICKS = 3  # 이만큼 넘게 벗어나야 "움직였다". 서 있는 서보도 1-2틱은 떨린다
N_WARMUP = 5  # 버리는 예비 반복 (must 4 와 같다)
N_ITER = 100  # 본 측정 반복 (must 4 와 같다)
OBSERVE_PERIOD_S = 0.010  # (b) 의 /joint_states 주기와 같은 읽기 간격
SETTLE_S = 0.5  # 팔이 멈춘 뒤 다음 반복까지 쉬는 시간
STOP_WINDOW = 20  # 최근 20 읽기 (0.2 s) 가
STOP_SPAN_TICKS = 2  # 2틱 안에 있으면 "멈췄다"
MOTION_TIMEOUT_S = 1.0  # 이 시간 안에 못 움직이면 그 반복은 실패 (NaN)
STOP_TIMEOUT_S = 3.0  # 이 시간 안에 안 멈추면 그냥 다음 반복으로

# ROS2 드라이버가 매 write 마다 보내는 값과 같게 맞춘다 (위 docstring)
SERVO_ACCELERATION = 50
SERVO_GOAL_VELOCITY = 2400
# lerobot 의 도 <-> 틱 변환 분모 (model_resolution 4096 - 1). 1틱 = 360 / 4095 도
MAX_RES = 4095
OUT_DIR = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "outputs")
)


def main() -> None:
    """(a) 를 N_WARMUP + N_ITER 회 재고 npy · csv 로 저장한다."""
    started_at = datetime.datetime.now().isoformat(timespec="seconds")

    # --- 연결: max_relative_target 없음, 카메라 없음 --------------------------------------
    config = SO101FollowerConfig(port=PORT, id=ROBOT_ID, max_relative_target=None)
    robot = SO101Follower(config)
    if not robot.calibration:
        sys.exit(f"캘리브 파일이 없다: {robot.calibration_fpath}")
    # 파일과 모터의 캘리브가 같으면 묻지 않고 지나간다. 다르면 lerobot 이 ENTER 로 확인을 받는다
    robot.connect()
    bus = robot.bus

    # --- 서보 프로파일을 ROS2 드라이버와 같게 --------------------------------------------------
    bus.write("Acceleration", JOINT, SERVO_ACCELERATION, normalize=False)
    bus.write("Goal_Velocity", JOINT, SERVO_GOAL_VELOCITY, normalize=False)
    # 실제로 서보에 들어 있는 값을 읽어 기록한다 (environment.md 의 재료)
    servo = {
        name: bus.read(name, JOINT, normalize=False)
        for name in [
            "P_Coefficient",
            "I_Coefficient",
            "D_Coefficient",
            "Acceleration",
            "Maximum_Acceleration",
            "Goal_Velocity",
            "Return_Delay_Time",
        ]
    }
    print(f"lerobot {lerobot.__version__} port={PORT} id={ROBOT_ID} joint={JOINT}")
    print(f"서보 레지스터 ({JOINT}): {servo}")

    # --- 틱 <-> 도: send_action 은 도 단위 (use_degrees=True) 로 받아 틱으로 되돌린다 ----------
    # lerobot 의 _unnormalize 는 int(deg * 4095 / 360 + mid) 로 0 쪽으로 버리므로, 정확히
    # ticks 가 되도록 반 틱을 더해 둔다. mid 는 캘리브 파일의 range 가운데다
    calibration = robot.calibration[JOINT]
    mid = (calibration.range_min + calibration.range_max) / 2.0

    def ticks_to_deg(ticks: int) -> float:
        """목표 틱을 lerobot 이 정확히 그 틱으로 되돌리는 도 값으로 바꾼다."""
        return (ticks + 0.5 - mid) * 360.0 / MAX_RES

    # --- 읽기: 틱 원값 (normalize=False). (b) 의 드라이버가 읽는 것과 같은 레지스터 -----------
    read_ms: List[float] = []  # 읽기 한 번에 걸린 시간 (environment.md 의 재료)

    def read_ticks() -> int:
        """JOINT 의 Present_Position 을 틱으로 읽는다."""
        start = time.perf_counter()
        ticks = bus.sync_read(
            "Present_Position",
            JOINT,
            normalize=False,
            num_retry=config.num_read_retries,
        )[JOINT]
        read_ms.append((time.perf_counter() - start) * 1000.0)
        return int(ticks)

    # 읽기 시각의 10 ms 격자. 스크립트 시작 시점에 고정하고 이후로는 이 격자 위에서만 읽는다
    grid_origin = time.perf_counter()

    def sleep_until_next_grid() -> None:
        """다음 격자 시각까지 기다린다."""
        k = math.ceil((time.perf_counter() - grid_origin) / OBSERVE_PERIOD_S)
        time.sleep(max(0.0, grid_origin + k * OBSERVE_PERIOD_S - time.perf_counter()))

    def wait_until_stopped() -> None:
        """관절이 멈출 때까지 (최근 STOP_WINDOW 읽기가 STOP_SPAN_TICKS 안) 기다린다."""
        recent: List[int] = []
        deadline = time.monotonic() + STOP_TIMEOUT_S
        while time.monotonic() < deadline:
            sleep_until_next_grid()
            recent = (recent + [read_ticks()])[-STOP_WINDOW:]
            if len(recent) == STOP_WINDOW and max(recent) - min(recent) <= STOP_SPAN_TICKS:
                return
        print("경고: 팔이 멈추는 것을 확인하지 못했다. 다음 반복으로 넘어간다")

    # --- 목표 두 개: 시작 위치 (low) 와 시작 + STEP_TICKS (high) 를 번갈아 보낸다 --------------
    # 매번 현재 위치에서 더하지 않고 절대 목표 두 개를 오가므로 정지 오차가 쌓여 흘러가지 않는다
    low = read_ticks()
    high = low + STEP_TICKS
    print(
        f"시작 위치 {low}틱 ({ticks_to_deg(low):+.2f}도), "
        f"계단 {STEP_TICKS}틱 ({STEP_TICKS * 360.0 / MAX_RES:.2f}도), 문턱 {THRESHOLD_TICKS}틱"
    )

    latencies_ms: List[float] = []  # 본 측정 (warm-up 제외). 실패는 NaN
    for i in range(N_WARMUP + N_ITER):
        # 명령 시각이 읽기 격자의 어느 위상에 떨어지는지를 고르게 흩는다 (docstring)
        time.sleep(random.uniform(0.0, OBSERVE_PERIOD_S))
        baseline = read_ticks()  # 계단 전 값 = 명령 직전 읽기
        target_deg = ticks_to_deg(high if i % 2 == 0 else low)
        t0 = time.perf_counter()
        robot.send_action({f"{JOINT}.pos": target_deg})

        # 격자 위에서 읽어, 계단 전 값에서 문턱 넘게 벗어난 첫 읽기가 돌아온 시각이 t1
        t1 = None
        while t1 is None and time.perf_counter() - t0 < MOTION_TIMEOUT_S:
            sleep_until_next_grid()
            position = read_ticks()
            now = time.perf_counter()
            if abs(position - baseline) > THRESHOLD_TICKS:
                t1 = now

        elapsed_ms = math.nan if t1 is None else (t1 - t0) * 1000.0
        if i >= N_WARMUP:
            latencies_ms.append(elapsed_ms)
            if (i - N_WARMUP + 1) % 10 == 0:
                print(f"{i - N_WARMUP + 1}/{N_ITER}: latest = {elapsed_ms:.1f} ms")
        elif i == N_WARMUP - 1:
            print("warm-up 완료")

        wait_until_stopped()
        time.sleep(SETTLE_S)

    # 마지막 반복은 high 로 끝나므로 시작 위치로 되돌리고 끝낸다
    robot.send_action({f"{JOINT}.pos": ticks_to_deg(low)})
    wait_until_stopped()

    # --- 통계: 실패 (NaN) 를 뺀 값으로 ------------------------------------------------------
    arr = np.array(latencies_ms)
    ok = arr[~np.isnan(arr)]
    n_timeout = int(np.isnan(arr).sum())
    reads = np.array(read_ms)
    print("\n[latency 통계 -- (a) LeRobot 직결]")
    print(f"mean   : {ok.mean():.2f} ms")
    print(f"median : {np.median(ok):.2f} ms")
    print(f"std    : {ok.std():.2f} ms")
    print(f"min    : {ok.min():.2f} ms")
    print(f"max    : {ok.max():.2f} ms")
    print(f"p95    : {np.percentile(ok, 95):.2f} ms")
    print(f"p99    : {np.percentile(ok, 99):.2f} ms")
    print(f"실패   : {n_timeout} / {N_ITER} (문턱을 {MOTION_TIMEOUT_S} s 안에 못 넘음)")
    print(f"위치 읽기 1회: mean {reads.mean():.2f} / max {reads.max():.2f} ms (n={len(reads)})")
    print(
        f"\nmethodology 1줄: (a) LeRobot 직결: "
        f"mean {ok.mean():.2f} / p95 {np.percentile(ok, 95):.2f} / std {ok.std():.2f} ms "
        f"(n={len(ok)}, {JOINT} {STEP_TICKS}틱 계단, 문턱 {THRESHOLD_TICKS}틱, 관측 10 ms, "
        f"Acceleration {servo['Acceleration']} · Goal_Velocity {servo['Goal_Velocity']})"
    )

    # --- 저장: 원본 배열 (npy, NaN 포함) + 기계 판독 요약 (csv 1행) ------------------------------
    os.makedirs(OUT_DIR, exist_ok=True)
    stem = "latency_lerobot"
    np.save(os.path.join(OUT_DIR, f"{stem}.npy"), arr)
    summary = {
        "path": "lerobot",
        "hardware": "real",
        "joint": JOINT,
        "step_ticks": STEP_TICKS,
        "threshold_ticks": THRESHOLD_TICKS,
        "observe_period_ms": OBSERVE_PERIOD_S * 1000.0,
        "n": len(ok),
        "warmup": N_WARMUP,
        "n_timeout": n_timeout,
        "mean_ms": f"{ok.mean():.3f}",
        "median_ms": f"{np.median(ok):.3f}",
        "std_ms": f"{ok.std():.3f}",
        "min_ms": f"{ok.min():.3f}",
        "max_ms": f"{ok.max():.3f}",
        "p95_ms": f"{np.percentile(ok, 95):.3f}",
        "p99_ms": f"{np.percentile(ok, 99):.3f}",
        "read_ms_mean": f"{reads.mean():.3f}",
        "read_ms_max": f"{reads.max():.3f}",
        "servo_acceleration": servo["Acceleration"],
        "servo_goal_velocity": servo["Goal_Velocity"],
        "servo_max_acceleration": servo["Maximum_Acceleration"],
        "servo_p": servo["P_Coefficient"],
        "servo_i": servo["I_Coefficient"],
        "servo_d": servo["D_Coefficient"],
        "servo_return_delay_time": servo["Return_Delay_Time"],
        "lerobot_version": lerobot.__version__,
        "started_at": started_at,
    }
    with open(os.path.join(OUT_DIR, f"{stem}_summary.csv"), "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(summary.keys())  # 헤더
        writer.writerow(summary.values())  # 값 1행
    print(f"저장: {OUT_DIR}/{stem}.npy, {stem}_summary.csv")

    # disconnect() 는 토크를 끈다 -- 팔이 자중으로 떨어지지 않게 받친 뒤에
    input("팔을 받친 뒤 Enter 를 누르면 토크를 풀고 끝낸다: ")
    robot.disconnect()


if __name__ == "__main__":
    main()
