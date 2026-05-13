"""
Phase 4 Week 10 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) ament_python")
    print("=" * 50 + "\n")
    print("해설:")
    print("  ROS2 의 두 가지 주요 build_type:")
    print("    ament_cmake  : C++ 패키지 (rclcpp)")
    print("    ament_python : Python 패키지 (rclpy)")
    print()
    print("  본 로드맵의 vla_node 는 Python (rclpy) -> ament_python.")
    print("  Phase 7 의 안전 인터록은 C++ (latency 최소화) -> ament_cmake.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: A) setup.py 의 entry_points")
    print("=" * 50 + "\n")
    print("해설:")
    print("  ament_python 의 entry_points 패턴:")
    print()
    print("    setup(")
    print("        entry_points={")
    print("            'console_scripts': [")
    print("                'vla_inference_node = vla_node.vla_inference_node:main',")
    print("            ],")
    print("        },")
    print("    )")
    print()
    print("  형식: '<executable_name> = <package>.<module>:<function>'")
    print("  -> `ros2 run vla_node vla_inference_node` 가 vla_node.vla_inference_node.main 호출")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) BEST_EFFORT + depth=1")
    print("=" * 50 + "\n")
    print("해설:")
    print("  30Hz image 의 특성:")
    print("    - 자주 새 image 도착 (matter 33ms)")
    print("    - 옛 image 는 곧 stale")
    print("    - network 부담 큼")
    print()
    print("  BEST_EFFORT + depth=1:")
    print("    - drop 허용 (network 끊김 시 자연스러운 처리)")
    print("    - 큐 1개 = 항상 최신만")
    print()
    print("  RELIABLE + depth=100 의 함정:")
    print("    - inference 가 느리면 큐 누적")
    print("    - 메모리 누수 + 처리 지연 누적")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) declare_parameter + get_parameter")
    print("=" * 50 + "\n")
    print("해설:")
    print("  ROS2 parameter 시스템:")
    print()
    print("    self.declare_parameter('model_id', 'openvla/openvla-7b')")
    print("    model_id = self.get_parameter('model_id').get_parameter_value().string_value")
    print()
    print("  장점:")
    print("    - launch file 에서 외부 주입")
    print("    - 동적 변경 가능 (ros2 param set)")
    print("    - 다른 노드가 조회 가능")
    print("    - rqt_reconfigure GUI 지원")
    print()
    print("  os.environ (A): runtime 동적 변경 불가, 노드 간 공유 어려움")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
