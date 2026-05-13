"""
Phase 4 Week 9 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) Twist + Float64")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Twist (6-DoF velocity / delta):")
    print("    linear.x, linear.y, linear.z   - 3D translation")
    print("    angular.x, angular.y, angular.z - 3D rotation")
    print()
    print("  Gripper (1-DoF): Float64 별도 topic")
    print()
    print("  왜 Pose (C) 안 쓰는가:")
    print("    - OpenVLA action 은 delta (변화량) 인데 Pose 는 absolute")
    print("    - quaternion 변환 추가 필요 (복잡도 증가)")
    print()
    print("  왜 custom (D) 안 쓰는가 (minimal demo 단계):")
    print("    - custom msg package 빌드 / 의존성 추가")
    print("    - minimal demo 는 표준 msg 로 충분")
    print()
    print("  Phase 7 결정타에서는 custom vla_msgs/Action 으로 전환:")
    print("    - 7-DoF + metadata + latency 한 번에")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: A) BGR8 -> RGB 변환 필요")
    print("=" * 50 + "\n")
    print("해설:")
    print("  OpenCV: BGR 기본 (cv2.imread 등의 결과)")
    print("  HuggingFace / OpenVLA / PIL: RGB 기본")
    print()
    print("  변환 누락 시:")
    print("    - red can 이 blue can 으로 인식")
    print("    - 'pick up the red can' 명령에서 학습된 적 없는 색 분포")
    print("    - action 이 noisy")
    print()
    print("  표준 pipeline:")
    print("    cv_bridge.imgmsg_to_cv2(msg, 'bgr8')   # ROS -> BGR")
    print("    cv2.cvtColor(img, cv2.COLOR_BGR2RGB)   # BGR -> RGB")
    print("    PIL.Image.fromarray(rgb)               # RGB -> PIL")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) best_effort, depth=1")
    print("=" * 50 + "\n")
    print("해설:")
    print("  /camera/image_raw 의 특징:")
    print("    - 30 Hz 로 빠르게 새 image 도착")
    print("    - 지난 image 는 의미 없음 (이미 새 게 와 있음)")
    print("    - drop 허용 (network 부하 시 자연스러운 해결)")
    print()
    print("  best_effort + depth=1:")
    print("    - 손실 허용으로 network 부담 적음")
    print("    - 큐 1 개 = 항상 최신 image 만 메모리에")
    print()
    print("  reliable + depth=100 (A) 의 문제:")
    print("    - 모든 frame 보존하려고 메모리 누적")
    print("    - inference 가 느리면 큐가 쌓이고 OOM")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: A) action.header.stamp - image.header.stamp")
    print("=" * 50 + "\n")
    print("해설:")
    print("  ROS2 의 모든 msg 에 header.stamp 가 있음 (ROS time):")
    print("    image_msg.header.stamp = image 가 카메라에서 publish 된 시점")
    print("    action_msg.header.stamp = action 이 발행된 시점")
    print()
    print("  inference latency = 두 시점의 차이")
    print()
    print("  System clock (B) 의 문제:")
    print("    - 노드들이 다른 host 일 수 있음")
    print("    - clock skew 영향")
    print("    - ROS2 는 자체 clock 동기화 (rclpy.clock)")
    print()
    print("  GPU clock (C):")
    print("    - inference 자체 시간만 측정 가능")
    print("    - image 도착부터 action 발행까지의 end-to-end 못 봄")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
