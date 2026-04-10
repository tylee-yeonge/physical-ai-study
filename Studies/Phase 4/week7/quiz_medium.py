"""
Quiz Medium - Week 7: nuScenes 데이터셋 (Section 6.3)
3문제 - 심화 이해
"""


def problem1_coordinate_system():
    print("\n" + "━" * 28)
    print("문제 1: 좌표계 변환 이해")
    print("━" * 28 + "\n")

    print("Q: nuScenes에서 3D 어노테이션은 Global 좌표계로 저장됩니다.")
    print("   이를 Ego Vehicle 좌표계로 변환하려면 어떤 과정이 필요한지")
    print("   단계별로 설명하세요.\n")
    print("   Step 1: _____")
    print("   Step 2: _____")
    print("   Step 3: _____\n")
    print("   또한, Ego 좌표계에서 특정 카메라 좌표계로 변환하려면")
    print("   추가로 어떤 정보가 필요한가요?")
    print("   답: _____\n")


def problem2_multicamera_challenge():
    print("\n" + "━" * 28)
    print("문제 2: Multi-camera 도전 과제")
    print("━" * 28 + "\n")

    print("Q: nuScenes의 6대 카메라를 활용한 3D Detection에서")
    print("   발생하는 도전 과제 3가지를 설명하고,")
    print("   각각의 해결 방법을 제안하세요.\n")
    print("   도전 과제 1: _____")
    print("     해결 방법: _____\n")
    print("   도전 과제 2: _____")
    print("     해결 방법: _____\n")
    print("   도전 과제 3: _____")
    print("     해결 방법: _____\n")
    print("   (힌트: 카메라 겹침 영역, 후방 카메라 화질, 시점 간 일관성)")


def problem3_quaternion_rotation():
    print("\n" + "━" * 28)
    print("문제 3: Quaternion 회전 표현")
    print("━" * 28 + "\n")

    print("Q: nuScenes는 회전을 Quaternion [w, x, y, z]로 표현합니다.")
    print("   KITTI는 rotation_y (스칼라)로 표현합니다.\n")
    print("   1) Quaternion이 Euler 각도(yaw, pitch, roll)보다")
    print("      유리한 점을 2가지 이상 설명하세요.")
    print("      답: _____\n")
    print("   2) 다음 Quaternion은 어떤 회전을 나타내나요?")
    print("      q = [1, 0, 0, 0]")
    print("      답: _____\n")
    print("   3) 90도 yaw 회전을 Quaternion으로 표현하면?")
    print("      (힌트: q = [cos(theta/2), 0, 0, sin(theta/2)] for z축 회전)")
    print("      답: _____\n")


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 7 Quiz - Medium (nuScenes 데이터셋)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_coordinate_system()
    problem2_multicamera_challenge()
    problem3_quaternion_rotation()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
