"""
Phase 4 Week 9 - I/O Spec + msg 변환 기초 퀴즈
"""


def problem1_minimal_msg():
    print("\n" + "=" * 50)
    print("문제 1: 7-DoF action 의 minimal demo msg")
    print("=" * 50 + "\n")
    print("질문: minimal demo 단계에서 OpenVLA 7-DoF action 발행에")
    print("      가장 적합한 표준 msg 조합은?\n")
    print("보기:")
    print("  A) sensor_msgs/JointState 하나로 전부")
    print("  B) geometry_msgs/Twist (6-DoF) + std_msgs/Float64 (gripper)")
    print("  C) geometry_msgs/Pose (3 + 4 quat) - 1개")
    print("  D) custom vla_msgs/Action 필수")


def problem2_cv_bridge_encoding():
    print("\n" + "=" * 50)
    print("문제 2: cv_bridge encoding")
    print("=" * 50 + "\n")
    print("질문: OpenCV 의 표준 색공간 + OpenVLA 입력 색공간 매핑은?\n")
    print("보기:")
    print("  A) OpenCV BGR8, OpenVLA RGB → cvtColor 로 BGR2RGB 필요")
    print("  B) OpenCV RGB8, OpenVLA RGB → 변환 불필요")
    print("  C) OpenCV YUV, OpenVLA RGB → 변환 필요")
    print("  D) OpenCV BGR8, OpenVLA BGR → 변환 불필요")


def problem3_qos_image():
    print("\n" + "=" * 50)
    print("문제 3: image topic 의 적절한 QoS")
    print("=" * 50 + "\n")
    print("질문: /camera/image_raw 에 적합한 QoS 설정은?\n")
    print("보기:")
    print("  A) reliable, depth=100 (모든 frame 보존)")
    print("  B) best_effort, depth=1 (drop 허용, 최신 1개만)")
    print("  C) transient_local, depth=10 (latch)")
    print("  D) reliable, depth=10")


def problem4_inference_latency():
    print("\n" + "=" * 50)
    print("문제 4: inference latency 측정 방법")
    print("=" * 50 + "\n")
    print("질문: ROS2 환경에서 OpenVLA inference latency 를 정확히")
    print("      측정하는 방법은?\n")
    print("보기:")
    print("  A) action.header.stamp - image.header.stamp")
    print("  B) system clock (time.time()) 호출 차이")
    print("  C) GPU clock 측정")
    print("  D) 매 1초마다 평균만 측정")


if __name__ == "__main__":
    print("=" * 50)
    problem1_minimal_msg()
    problem2_cv_bridge_encoding()
    problem3_qos_image()
    problem4_inference_latency()
    print("=" * 50)
