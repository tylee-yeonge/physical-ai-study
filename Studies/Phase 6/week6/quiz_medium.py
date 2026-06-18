"""Phase 6 Week 6 - 중급"""


def p1():
    """ee 위치 오차 분석"""
    print("\n문제 1: ee 오차 20mm 발생 (기대 < 5mm)")
    print()
    print("  원인 분석:")
    print("  A) Joint 매핑 잘못 (URDF/Sim 순서)")
    print("  B) URDF 의 link length 부정확")
    print("  C) Real robot 의 joint encoder 오차")
    print("  D) 모두 가능")
    # TODO
    ans = ""
    print(f"  당신: {ans}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """ROS2 thread 와 Sim main thread"""
    print("\n문제 2: Isaac Sim 안에서 ROS2 spin")
    print("  Sim main thread: world.step() loop")
    print("  ROS2 thread    : rclpy.spin(node)")
    print()
    print("  두 thread 의 동기화:")
    print("  - Callback 에서 arm.set_joint_position_targets() 호출")
    print("  - Sim main 에서 world.step() 호출")
    print()
    print("  주의: thread safety (arm 객체 접근)")
    print("  본 phase 권장: Sim 의 single executor + ros2 spin_once 패턴")


def p3():
    """Phase 7 까지의 진행"""
    print("\n문제 3: 디지털 트윈 -> Phase 7 의 closed loop")
    print()
    print("  Phase 6 의 현재 (week 6):")
    print("    Real joint -> Sim joint (one-way)")
    print()
    print("  Phase 7 의 산출물 #4 (closed loop):")
    print("    Sim image -> vla_node -> action -> Real robot")
    print("                                |")
    print("                                v")
    print("                            Real joint -> Sim joint")
    print("    완전한 Real-to-Sim-to-Real")


if __name__ == "__main__":
    p1(); p2(); p3()
