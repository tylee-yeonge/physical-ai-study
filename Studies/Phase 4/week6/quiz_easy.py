"""
Quiz Easy - Week 6: 성능 분석 및 개선 (Section 6.2)
4문제 - 기본 개념 확인
"""


def problem1_error_types():
    print("\n" + "━" * 28)
    print("문제 1: 오류 유형 분류")
    print("━" * 28 + "\n")

    print("Q: 3D Detection에서 발생하는 주요 오류 유형 4가지를 나열하고,")
    print("   각각이 성능(AP3D)에 미치는 영향을 간단히 설명하세요.\n")
    print("   1. _____ : _____")
    print("   2. _____ : _____")
    print("   3. _____ : _____")
    print("   4. _____ : _____\n")


def problem2_depth_error_impact():
    print("\n" + "━" * 28)
    print("문제 2: Depth 오류와 3D IoU")
    print("━" * 28 + "\n")

    print("Q: Monocular 3D Detection에서 Depth 오차가 3D IoU에 미치는")
    print("   영향에 대한 설명 중 올바른 것을 모두 고르세요.\n")
    print("   a) Depth 1m 오차면 Car의 3D IoU가 0.7 이하로 떨어질 수 있다")
    print("   b) 원거리(40m+)에서 Depth 오차는 보통 1m 미만이다")
    print("   c) AP2D가 85%라도 AP3D는 15% 이하일 수 있다")
    print("   d) Depth 오차는 2D bbox 정확도와 무관하다")
    print()
    print("   답: _____\n")


def problem3_augmentation_flip():
    print("\n" + "━" * 28)
    print("문제 3: Random Flip과 3D 레이블")
    print("━" * 28 + "\n")

    print("Q: 이미지를 좌우 반전(Random Flip)할 때, 3D 레이블에서")
    print("   반드시 함께 변환해야 하는 값은 무엇인가요?\n")
    print("   다음 중 변환이 필요한 항목을 모두 고르세요.")
    print("   a) 3D 중심 좌표의 x값")
    print("   b) 3D 중심 좌표의 z값 (깊이)")
    print("   c) rotation_y (yaw 회전각)")
    print("   d) 3D 크기 (l, w, h)")
    print()
    print("   답: _____\n")


def problem4_nms_comparison():
    print("\n" + "━" * 28)
    print("문제 4: 2D NMS vs 3D NMS")
    print("━" * 28 + "\n")

    print("Q: 다음 시나리오에서 2D NMS와 BEV NMS의 결과 차이를 설명하세요.\n")
    print("   이미지에서 두 차량이 겹쳐 보이지만:")
    print("     Car A: z=10m, score=0.92")
    print("     Car B: z=25m, score=0.85")
    print("   두 차량의 2D bbox가 IoU 0.6으로 겹칩니다.\n")
    print("   2D NMS (threshold=0.5) 결과: _____")
    print("   BEV NMS (threshold=0.25) 결과: _____")
    print("   이유: _____\n")


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 6 Quiz - Easy (성능 분석 및 개선)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_error_types()
    problem2_depth_error_impact()
    problem3_augmentation_flip()
    problem4_nms_comparison()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
