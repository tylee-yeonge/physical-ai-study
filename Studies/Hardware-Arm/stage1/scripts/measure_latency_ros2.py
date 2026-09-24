"""ROS2 경유 명령 latency 측정 -- Stage 1 이중 latency 의 (b) (ros2_driver_setup.md §5).

무엇을 재는가 (§5.3):
    1회 = shoulder_pan 에 계단 명령 1개를 /position_controller/commands 로 보내고 (t0),
    /joint_states 에서 그 관절이 계단 전 값에서 3틱 넘게 벗어난 첫 샘플의 header.stamp (t1)
    까지. t1 - t0 에는 DDS 전송 + 컨트롤러가 다음 제어 주기까지 기다리는 시간 + 드라이버
    write + 시리얼 + 서보가 움직이기 시작하는 시간이 들어 있다 (§5.2 의 (b) 열).
    t1 을 메시지를 받은 시각이 아니라 header.stamp 로 잡는 이유: 상태가 되돌아오는 길의
    DDS 시간이 (b) 에만 더해지지 않게 하기 위해서다.

같은 스크립트로 두 번 잰다 (§5.4 순서 1, 2):
    mock -- bringup.launch.py use_mock_hardware:=true. 서보 · 시리얼이 빠진
            "DDS + 제어 주기 대기" 만의 시간. 스크립트 검증이자 (b) 의 하한 대조군
    real -- 실제 팔
    어느 쪽인지는 /robot_description 의 플러그인 이름으로 스크립트가 판정해 파일명에 붙인다.

실행 (bringup.launch.py 가 떠 있는 상태에서 다른 터미널로):
    python3 Studies/Hardware-Arm/stage1/scripts/measure_latency_ros2.py
출력 (outputs/ 는 gitignore 대상 -- 기록에 쓸 실행은 §5.4 순서 5 대로 시각을 붙여 복사한다):
    Studies/Hardware-Arm/stage1/outputs/latency_ros2_mock.npy (+ latency_ros2_mock_summary.csv)
    Studies/Hardware-Arm/stage1/outputs/latency_ros2_real.npy (+ latency_ros2_real_summary.csv)

실제 팔에서는 shoulder_pan 이 33틱 (약 2.9도) 을 105번 오간다. 나머지 다섯 관절은 시작할
때 읽은 위치를 그대로 목표로 보내므로 움직이지 않는다. 팔은 낮은 자세로 받쳐 두고, W5 의
안전 기초 (소프트 리밋 · 토크 상한 · 소프트웨어 정지) 가 선 뒤에 돌린다 (§5 "언제").
"""

import csv
import datetime
import math
import os
import random
import time
from typing import List
from typing import Tuple

import numpy as np
import rclpy
from rcl_interfaces.srv import GetParameters
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy
from rclpy.qos import QoSProfile
from rclpy.time import Time
from sensor_msgs.msg import JointState
from std_msgs.msg import Float64MultiArray
from std_msgs.msg import String

# config/so101_controllers.yaml 의 joints 순서 = 명령 배열의 순서
COMMAND_ORDER: List[str] = [
    "shoulder_pan",
    "shoulder_lift",
    "elbow_flex",
    "wrist_flex",
    "wrist_roll",
    "gripper",
]
JOINT = "shoulder_pan"  # 움직일 관절. 중력을 받지 않아 방향에 따라 응답이 달라지지 않는다
STEP_TICKS = 33  # 계단 크기. 0.05 rad 에 가장 가까운 정수 틱 (0.05 * 4096 / 2pi = 32.6)
THRESHOLD_TICKS = 3  # 이만큼 넘게 벗어나야 "움직였다". 서 있는 서보도 1-2틱은 떨린다
N_WARMUP = 5  # 버리는 예비 반복 (must 4 와 같다)
N_ITER = 100  # 본 측정 반복 (must 4 와 같다)
OBSERVE_PERIOD_S = 0.010  # /joint_states 의 주기 (update_rate 100 Hz). (a) 도 이 간격으로 읽는다
SETTLE_S = 0.5  # 팔이 멈춘 뒤 다음 반복까지 쉬는 시간
STOP_WINDOW = 20  # 최근 20 샘플 (0.2 s) 이
STOP_SPAN_TICKS = 2  # 2틱 안에 있으면 "멈췄다"
MOTION_TIMEOUT_S = 1.0  # 이 시간 안에 못 움직이면 그 반복은 실패 (NaN)
STOP_TIMEOUT_S = 3.0  # 이 시간 안에 안 멈추면 그냥 다음 반복으로

# 드라이버가 매 write 마다 서보에 고정으로 보내는 값 (feetech_ros2_driver.cpp write() 의 상수).
# 서보의 가속도 · 속도 프로파일을 정하므로 (a) 의 LeRobot 쪽도 이 값으로 맞춘다
DRIVER_ACCELERATION = 50
DRIVER_GOAL_VELOCITY = 2400

# 드라이버의 rad <-> 틱 변환 (feetech_driver/common.hpp). 1바퀴 4096틱, 2048 이 0 rad
RAD_PER_TICK = 2.0 * math.pi / 4096.0
OUT_DIR = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "outputs")
)


def rad_to_ticks(rad: float) -> int:
    """/joint_states 의 rad 값을 서보 틱 (2048 기준 상대값) 으로 되돌린다.

    드라이버는 (틱 - 2048) * 2pi / 4096 을 그대로 내보내므로 정수 틱의 배수다.
    round 는 부동소수점 오차만 걷어낸다.

    Args:
        rad: /joint_states 에 실린 관절 위치 (rad)

    Returns:
        2048 을 뺀 틱 값
    """
    return round(rad / RAD_PER_TICK)


def ticks_to_rad(ticks: int) -> float:
    """목표 틱을 드라이버가 정확히 그 틱으로 되돌리는 rad 값으로 바꾼다.

    드라이버의 from_radians 는 static_cast<int> 로 0 쪽으로 버린다. 정수 틱의 배수를 그대로
    보내면 부동소수점 오차로 한 틱 아래로 떨어질 수 있어 반 틱을 바깥쪽으로 더한다.

    Args:
        ticks: 2048 을 뺀 목표 틱 값

    Returns:
        명령 토픽에 실을 rad 값
    """
    if ticks == 0:
        return 0.0
    nudge = 0.5 if ticks > 0 else -0.5
    return (ticks + nudge) * RAD_PER_TICK


def spin_for(node: Node, seconds: float) -> None:
    """메시지를 계속 받으면서 벽시계로 seconds 만큼 기다린다.

    Args:
        node: spin 할 노드
        seconds: 기다릴 시간 (s)
    """
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        rclpy.spin_once(node, timeout_sec=0.01)


def wait_until_stopped(node: Node, samples: List[Tuple[int, float]]) -> None:
    """관절이 멈출 때까지 (최근 STOP_WINDOW 샘플이 STOP_SPAN_TICKS 안) 기다린다.

    Args:
        node: spin 할 노드
        samples: 콜백이 (header.stamp ns, rad) 를 쌓는 리스트
    """
    span_limit = STOP_SPAN_TICKS * RAD_PER_TICK
    deadline = time.monotonic() + STOP_TIMEOUT_S
    while time.monotonic() < deadline:
        rclpy.spin_once(node, timeout_sec=0.01)
        recent = [pos for _, pos in samples[-STOP_WINDOW:]]
        if len(recent) == STOP_WINDOW and max(recent) - min(recent) <= span_limit:
            return
    print("경고: 팔이 멈추는 것을 확인하지 못했다. 다음 반복으로 넘어간다")


def main() -> None:
    """(b) 를 N_WARMUP + N_ITER 회 재고 npy · csv 로 저장한다."""
    started_at = datetime.datetime.now().isoformat(timespec="seconds")
    rclpy.init()
    node = Node("measure_latency_ros2")

    # --- 어느 하드웨어인가: /robot_description 의 <plugin> 이름으로 mock / real 을 가른다 ------
    # robot_state_publisher 가 transient_local 로 내보내므로 늦게 붙어도 마지막 값을 받는다
    descriptions: List[str] = []
    node.create_subscription(
        String,
        "/robot_description",
        lambda msg: descriptions.append(msg.data),
        QoSProfile(depth=1, durability=DurabilityPolicy.TRANSIENT_LOCAL),
    )
    while not descriptions:
        rclpy.spin_once(node, timeout_sec=1.0)
    hardware = "mock" if "mock_components/GenericSystem" in descriptions[0] else "real"

    # --- update_rate: 제어 주기 (환경 기록용. §5.5 의 선택 검증이 이 값을 바꾼다) ----------------
    client = node.create_client(GetParameters, "/controller_manager/get_parameters")
    client.wait_for_service()
    future = client.call_async(GetParameters.Request(names=["update_rate"]))
    rclpy.spin_until_future_complete(node, future)
    update_rate = future.result().values[0].integer_value

    # --- /joint_states 구독: JOINT 의 (header.stamp ns, rad) 를 쌓는다 ------------------------
    samples: List[Tuple[int, float]] = []
    latest = {}  # 관절 이름 -> 마지막 rad (다른 다섯 관절의 유지 목표를 만들 때 쓴다)

    def on_joint_states(msg: JointState) -> None:
        """/joint_states 한 개를 받아 JOINT 의 값과 시각을 기록한다."""
        latest.update(zip(msg.name, msg.position))
        stamp_ns = Time.from_msg(msg.header.stamp).nanoseconds
        samples.append((stamp_ns, latest[JOINT]))

    node.create_subscription(JointState, "/joint_states", on_joint_states, 100)
    publisher = node.create_publisher(
        Float64MultiArray, "/position_controller/commands", 10
    )
    # 컨트롤러가 구독을 맺고 첫 상태가 들어올 때까지 기다린다 (ros2 topic pub 의 "Waiting for ..." 와 같다)
    while publisher.get_subscription_count() == 0 or not samples:
        rclpy.spin_once(node, timeout_sec=0.1)

    # --- 목표 두 개: 시작 위치 (low) 와 시작 + STEP_TICKS (high) 를 번갈아 보낸다 --------------
    # 매번 현재 위치에서 더하지 않고 절대 목표 두 개를 오가므로 정지 오차가 쌓여 흘러가지 않는다
    hold_ticks = {name: rad_to_ticks(latest[name]) for name in COMMAND_ORDER}
    low = hold_ticks[JOINT]
    high = low + STEP_TICKS
    command = Float64MultiArray()
    joint_index = COMMAND_ORDER.index(JOINT)
    threshold_rad = THRESHOLD_TICKS * RAD_PER_TICK

    def publish_target(ticks: int) -> int:
        """JOINT 만 ticks 로, 나머지는 시작 위치 그대로 보내고 t0 (ns) 를 돌려준다."""
        data = [ticks_to_rad(hold_ticks[name]) for name in COMMAND_ORDER]
        data[joint_index] = ticks_to_rad(ticks)
        command.data = data
        # header.stamp 와 같은 시계 (ROS 시간 = 시스템 시계) 로 t0 를 찍는다. 다른 시계를 쓰면 뺄 수 없다
        t0_ns = node.get_clock().now().nanoseconds
        publisher.publish(command)
        return t0_ns

    print(f"hardware={hardware} update_rate={update_rate} Hz joint={JOINT}")
    print(
        f"시작 위치 {low:+d}틱 ({low * RAD_PER_TICK:+.4f} rad), "
        f"계단 {STEP_TICKS}틱 ({STEP_TICKS * RAD_PER_TICK:.4f} rad), 문턱 {THRESHOLD_TICKS}틱"
    )

    latencies_ms: List[float] = []  # 본 측정 (warm-up 제외). 실패는 NaN
    for i in range(N_WARMUP + N_ITER):
        # 명령을 보내는 순간이 제어 주기의 어느 위상에 떨어지는지를 고르게 흩는다. 이 루프는
        # /joint_states 도착에 맞춰 깨어나므로 그냥 보내면 매번 같은 위상에 보내게 된다
        time.sleep(random.uniform(0.0, OBSERVE_PERIOD_S))
        baseline = samples[-1][1]  # 계단 전 값 = 명령 직전의 마지막 샘플
        samples.clear()  # 이 뒤에 들어오는 샘플만 본다
        t0_ns = publish_target(high if i % 2 == 0 else low)

        # 계단 전 값에서 문턱 넘게 벗어난 첫 샘플의 header.stamp 가 t1
        t1_ns = None
        seen = 0
        deadline = time.monotonic() + MOTION_TIMEOUT_S
        while t1_ns is None and time.monotonic() < deadline:
            rclpy.spin_once(node, timeout_sec=0.01)
            while seen < len(samples):
                stamp_ns, pos = samples[seen]
                seen += 1
                # t0 보다 앞선 stamp 는 명령이 닿기 전의 상태라 제외한다
                if stamp_ns >= t0_ns and abs(pos - baseline) > threshold_rad:
                    t1_ns = stamp_ns
                    break

        elapsed_ms = math.nan if t1_ns is None else (t1_ns - t0_ns) / 1e6
        if i >= N_WARMUP:
            latencies_ms.append(elapsed_ms)
            if (i - N_WARMUP + 1) % 10 == 0:
                print(f"{i - N_WARMUP + 1}/{N_ITER}: latest = {elapsed_ms:.1f} ms")
        elif i == N_WARMUP - 1:
            print("warm-up 완료")

        wait_until_stopped(node, samples)
        spin_for(node, SETTLE_S)

    # 마지막 반복은 high 로 끝나므로 시작 위치로 되돌리고 끝낸다
    publish_target(low)
    wait_until_stopped(node, samples)
    node.destroy_node()
    rclpy.shutdown()

    # --- 통계: 실패 (NaN) 를 뺀 값으로 ------------------------------------------------------
    arr = np.array(latencies_ms)
    ok = arr[~np.isnan(arr)]
    n_timeout = int(np.isnan(arr).sum())
    print(f"\n[latency 통계 -- (b) ROS2 경유, {hardware}]")
    print(f"mean   : {ok.mean():.2f} ms")
    print(f"median : {np.median(ok):.2f} ms")
    print(f"std    : {ok.std():.2f} ms")
    print(f"min    : {ok.min():.2f} ms")
    print(f"max    : {ok.max():.2f} ms")
    print(f"p95    : {np.percentile(ok, 95):.2f} ms")
    print(f"p99    : {np.percentile(ok, 99):.2f} ms")
    print(f"실패   : {n_timeout} / {N_ITER} (문턱을 {MOTION_TIMEOUT_S} s 안에 못 넘음)")
    print(
        f"\nmethodology 1줄: (b) ROS2 경유 ({hardware}, update_rate {update_rate} Hz): "
        f"mean {ok.mean():.2f} / p95 {np.percentile(ok, 95):.2f} / std {ok.std():.2f} ms "
        f"(n={len(ok)}, {JOINT} {STEP_TICKS}틱 계단, 문턱 {THRESHOLD_TICKS}틱, 관측 10 ms)"
    )

    # --- 저장: 원본 배열 (npy, NaN 포함) + 기계 판독 요약 (csv 1행) ------------------------------
    os.makedirs(OUT_DIR, exist_ok=True)
    stem = f"latency_ros2_{hardware}"
    np.save(os.path.join(OUT_DIR, f"{stem}.npy"), arr)
    summary = {
        "path": "ros2",
        "hardware": hardware,
        "joint": JOINT,
        "step_ticks": STEP_TICKS,
        "threshold_ticks": THRESHOLD_TICKS,
        "observe_period_ms": 1000.0 / update_rate,
        "update_rate_hz": update_rate,
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
        "servo_acceleration": DRIVER_ACCELERATION,
        "servo_goal_velocity": DRIVER_GOAL_VELOCITY,
        "started_at": started_at,
    }
    with open(os.path.join(OUT_DIR, f"{stem}_summary.csv"), "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(summary.keys())  # 헤더
        writer.writerow(summary.values())  # 값 1행
    print(f"저장: {OUT_DIR}/{stem}.npy, {stem}_summary.csv")


if __name__ == "__main__":
    main()
