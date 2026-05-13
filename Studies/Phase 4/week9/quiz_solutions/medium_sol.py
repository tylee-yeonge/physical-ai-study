"""
Phase 4 Week 9 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: action -> Twist + gripper 매핑")
    print("=" * 60 + "\n")
    mapping = [
        ("action[0] (dx)", "twist.linear.x"),
        ("action[1] (dy)", "twist.linear.y"),
        ("action[2] (dz)", "twist.linear.z"),
        ("action[3] (rx)", "twist.angular.x"),
        ("action[4] (ry)", "twist.angular.y"),
        ("action[5] (rz)", "twist.angular.z"),
        ("action[6] (gripper)", "gripper_msg.data (std_msgs/Float64)"),
    ]
    for src, dst in mapping:
        print(f"  {src:25s} -> {dst}")

    print()
    print("  [tip] geometry_msgs/Twist 는 한 번에 6-DoF velocity 또는 delta 표현.")
    print("       OpenVLA action 의 처음 6 dim 과 정확히 매칭.")
    print("       gripper 만 별도 topic 으로.")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: image age threshold")
    print("=" * 60 + "\n")
    print("  (a) 5 Hz 제어 cycle 200ms")
    print("      image age threshold ~ 100~200ms 권장")
    print("      (cycle 의 절반 이하면 안전)")
    print()
    print("  (b) 5 cm/s 속도 * 100 ms = 0.5 cm 위치 오차")
    print("      자작 6DOF 팔의 typical 정밀도 (~ 1mm) 대비 5배 큼")
    print("      -> image age 100ms 면 거의 hard limit")
    print()
    print("  [tip] image age 가 threshold 초과 시 동작:")
    print("    - 옵션 1: 그 frame 무시 (skip inference)")
    print("    - 옵션 2: 그래도 inference 후 safety 가 결정")
    print("    - 옵션 3: warning + 진행 (가장 일반적)")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: QoS 결정")
    print("=" * 60 + "\n")
    qos = [
        ("image_raw", "best_effort", "keep_last", 1,
         "drop 허용, 최신만 보존"),
        ("action", "reliable", "keep_last", 10,
         "모든 action 보존 (robot 제어)"),
        ("instruction", "transient_local", "keep_last", 1,
         "latch (새 노드가 join 해도 마지막 명령 수신)"),
        ("latency", "best_effort", "keep_last", 10,
         "drop 허용, monitoring 용"),
    ]
    print(f"  {'Topic':<14}{'reliability':<18}{'history':<12}{'depth':<6}{'이유'}")
    print(f"  {'-'*14}{'-'*18}{'-'*12}{'-'*6}{'-'*40}")
    for q in qos:
        print(f"  {q[0]:<14}{q[1]:<18}{q[2]:<12}{q[3]:<6}{q[4]}")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
