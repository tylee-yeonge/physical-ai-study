"""
Phase 4 Week 9 - 중급 퀴즈
"""


def problem1_design_action_twist():
    """
    문제 1: action ndarray 를 Twist + Float64 로 변환

    action = [0.05, -0.03, 0.02, 0.5, -1.2, 0.0, 0.9]
    위 action 의 각 컴포넌트를 ROS msg 의 어느 필드에 넣을지 매핑.

    TODO: 각 컴포넌트의 매핑 명시.
    """
    print("\n" + "=" * 60)
    print("문제 1: action -> Twist + gripper Float64")
    print("=" * 60 + "\n")

    # TODO
    mapping = {
        "action[0] (dx)": "",
        "action[1] (dy)": "",
        "action[2] (dz)": "",
        "action[3] (rx)": "",
        "action[4] (ry)": "",
        "action[5] (rz)": "",
        "action[6] (gripper)": "",
    }

    expected = {
        "action[0] (dx)": "twist.linear.x",
        "action[1] (dy)": "twist.linear.y",
        "action[2] (dz)": "twist.linear.z",
        "action[3] (rx)": "twist.angular.x",
        "action[4] (ry)": "twist.angular.y",
        "action[5] (rz)": "twist.angular.z",
        "action[6] (gripper)": "gripper_msg.data (std_msgs/Float64)",
    }

    print("  당신의 매핑:")
    for k, v in mapping.items():
        mark = "[O]" if v == expected[k] else "[X]"
        print(f"    {mark} {k} -> {v}")
        print(f"        (기대: {expected[k]})")


def problem2_image_age_threshold():
    """
    문제 2: image age threshold

    아래 시나리오에서 OpenVLA inference 가 의미 있으려면 image age 의
    threshold 는 얼마여야 하는가?

    상황: robot 이 10 cm/s 로 이동 중. image age 가 100ms 면 robot 은
    이미 1cm 이동한 상태. action 이 늦은 정보 기반.

    질문:
      (a) 5 Hz 제어 시 image age threshold 는?
      (b) 자작 6DOF 팔 (Phase 7, 5 cm/s 속도) 에서 image age 100ms 면
          robot 의 위치 오차는?

    TODO 값 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 2: image age threshold")
    print("=" * 60 + "\n")

    # TODO
    threshold_ms = 0
    pos_error_cm = 0.0

    expected_threshold = 200  # 5 Hz = 200ms cycle. age 가 그 절반 이하 권장
    expected_pos_error = 0.5  # 5 cm/s * 0.1s = 0.5 cm

    print(f"  (a) 5 Hz 의 image age threshold : {threshold_ms} ms (기대 ~ 100~200)")
    print(f"  (b) 5cm/s * 100ms             : {pos_error_cm} cm (기대 ~ 0.5)")


def problem3_qos_table():
    """
    문제 3: 각 topic 의 QoS 결정

    아래 4 개 topic 의 QoS 를 선택:

    Topic                       | reliability    | history     | depth
    /camera/image_raw          | ?              | ?           | ?
    /vla/action                 | ?              | ?           | ?
    /vla/instruction           | ?              | ?           | ?
    /vla/latency_ms             | ?              | ?           | ?

    TODO: 각 항목을 채우시오.
    """
    print("\n" + "=" * 60)
    print("문제 3: QoS 결정")
    print("=" * 60 + "\n")

    # TODO
    qos = {
        "image_raw": ("", "", 0),
        "action": ("", "", 0),
        "instruction": ("", "", 0),
        "latency": ("", "", 0),
    }

    expected = {
        "image_raw": ("best_effort", "keep_last", 1),
        "action": ("reliable", "keep_last", 10),
        "instruction": ("transient_local", "keep_last", 1),
        "latency": ("best_effort", "keep_last", 10),
    }

    print("  당신의 QoS:")
    for k, v in qos.items():
        mark = "[O]" if v == expected[k] else "[X]"
        print(f"    {mark} {k}: {v}")
        print(f"          (기대: {expected[k]})")


if __name__ == "__main__":
    print("=" * 60)
    problem1_design_action_twist()
    problem2_image_age_threshold()
    problem3_qos_table()
    print("=" * 60)
