"""Phase 6 Week 3 - 기초"""


def p1():
    print("\n문제 1: ROS2 Bridge extension 이름")
    print("  A) omni.isaac.ros2\n  B) omni.isaac.ros2_bridge\n  C) ros2_bridge\n  D) isaac_ros2")


def p2():
    print("\n문제 2: Sim time 의 가치")
    print("  A) Wall clock 보다 빠름\n  B) Deterministic 시뮬레이션 (재현 가능)\n  C) GPU 빠름\n  D) Memory 적음")


def p3():
    print("\n문제 3: Action Graph 의 역할")
    print("  A) GUI 만\n  B) Sim 의 publish/subscribe graph\n  C) Network protocol\n  D) Storage")


def p4():
    print("\n문제 4: ROS2 Bridge latency")
    print("  A) ~ 1 ms\n  B) ~ 38 ms (Sim step + bridge + DDS)\n  C) ~ 200 ms\n  D) ~ 1 s")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
