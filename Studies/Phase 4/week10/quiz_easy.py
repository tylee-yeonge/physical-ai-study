"""
Phase 4 Week 10 - ROS2 패키지 골격 기초 퀴즈
"""


def problem1_pkg_type():
    print("\n" + "=" * 50)
    print("문제 1: vla_node 패키지의 build_type")
    print("=" * 50 + "\n")
    print("질문: Python 으로 ROS2 노드를 작성할 때 ros2 pkg create 의 build_type?\n")
    print("보기:")
    print("  A) ament_cmake")
    print("  B) ament_python")
    print("  C) cmake")
    print("  D) cargo")


def problem2_entry_point():
    print("\n" + "=" * 50)
    print("문제 2: ros2 run 의 entry_point")
    print("=" * 50 + "\n")
    print("질문: `ros2 run vla_node vla_inference_node` 가 동작하려면?\n")
    print("보기:")
    print("  A) setup.py 의 entry_points/console_scripts 에 등록")
    print("  B) CMakeLists.txt 의 add_executable")
    print("  C) package.xml 의 <executable> tag")
    print("  D) bash 의 alias")


def problem3_qos_for_image():
    print("\n" + "=" * 50)
    print("문제 3: image subscriber QoS")
    print("=" * 50 + "\n")
    print("질문: 30Hz 카메라 image 를 subscribe 할 때 적절한 QoS 는?\n")
    print("보기:")
    print("  A) RELIABLE + depth=100")
    print("  B) BEST_EFFORT + depth=1")
    print("  C) RELIABLE + depth=1")
    print("  D) BEST_EFFORT + depth=100")


def problem4_param_declare():
    print("\n" + "=" * 50)
    print("문제 4: ROS2 parameter 의 표준 패턴")
    print("=" * 50 + "\n")
    print("질문: ROS2 node 에서 hyper-parameter (model_id 등) 를 받는 표준 방식?\n")
    print("보기:")
    print("  A) os.environ 환경 변수")
    print("  B) self.declare_parameter() + self.get_parameter()")
    print("  C) argparse")
    print("  D) 코드에 하드코드")


if __name__ == "__main__":
    print("=" * 50)
    problem1_pkg_type()
    problem2_entry_point()
    problem3_qos_for_image()
    problem4_param_declare()
    print("=" * 50)
