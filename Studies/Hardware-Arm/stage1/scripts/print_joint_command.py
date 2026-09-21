"""현재 관절 위치를 읽어, 붙여 넣을 위치 명령 두 줄을 출력한다.

이 스크립트는 읽기만 한다. 팔에는 아무것도 보내지 않는다.
bringup.launch.py 가 떠 있는 상태에서 다른 터미널로 실행한다.

    python3 print_joint_command.py                    # shoulder_pan 을 +0.05 rad
    python3 print_joint_command.py elbow_flex -0.05   # 관절 이름과 변화량 (rad) 을 지정

왜 필요한가: /joint_states 는 관절이 알파벳 순서로 나오고, 명령 토픽은 yaml 의
joints 순서로 받는다. 손으로 옮겨 적다 순서가 틀리면 엉뚱한 관절이 그만큼 튄다.
"""

import math
import sys
from typing import Dict
from typing import List

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState

# config/so101_controllers.yaml 의 joints 순서 = 명령 배열의 순서
COMMAND_ORDER: List[str] = [
    "shoulder_pan",
    "shoulder_lift",
    "elbow_flex",
    "wrist_flex",
    "wrist_roll",
    "gripper",
]
COMMAND_PREFIX = (
    "ros2 topic pub --once /position_controller/commands "
    "std_msgs/msg/Float64MultiArray"
)


def main() -> None:
    """현재 위치를 한 번 읽고, 유지 명령과 한 관절만 바꾼 명령을 출력한다."""
    # 인자가 없으면 shoulder_pan 을 +0.05 rad (약 2.9도) 움직이는 명령을 만든다
    joint = sys.argv[1] if len(sys.argv) > 1 else "shoulder_pan"
    delta = float(sys.argv[2]) if len(sys.argv) > 2 else 0.05
    if joint not in COMMAND_ORDER:
        sys.exit(f"모르는 관절 이름: {joint} (가능: {', '.join(COMMAND_ORDER)})")

    # /joint_states 메시지를 하나 받을 때까지 기다린다
    rclpy.init()
    node = Node("print_joint_command")
    received: List[JointState] = []
    node.create_subscription(JointState, "/joint_states", received.append, 10)
    while not received:
        rclpy.spin_once(node, timeout_sec=1.0)
    node.destroy_node()
    rclpy.shutdown()

    # 위치 (몇 번째) 가 아니라 이름으로 값을 찾는다
    message = received[0]
    current: Dict[str, float] = dict(zip(message.name, message.position))

    print("관절            rad        도")
    for name in COMMAND_ORDER:
        print(f"{name:14s} {current[name]:+8.4f}  {math.degrees(current[name]):+8.2f}")

    # 명령 순서로 다시 늘어놓는다. hold 는 지금 자세 그대로, moved 는 한 관절만 바꾼 것
    hold = [round(current[name], 4) for name in COMMAND_ORDER]
    moved = list(hold)
    index = COMMAND_ORDER.index(joint)
    moved[index] = round(moved[index] + delta, 4)

    print(f"\n# {joint} 만 {delta:+.4f} rad ({math.degrees(delta):+.2f}도) 움직인다")
    print(f'{COMMAND_PREFIX} "data: {moved}"')
    print("\n# 지금 읽은 자세로 되돌린다")
    print(f'{COMMAND_PREFIX} "data: {hold}"')


if __name__ == "__main__":
    main()
