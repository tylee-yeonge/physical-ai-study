"""
Phase 6 Week 4 - Monocular 3D Detection 모델 기초 퀴즈
"""


def problem1_smoke_keypoint():
    print("\n" + "━" * 28)
    print("문제 1: SMOKE 핵심 아이디어")
    print("━" * 28 + "\n")

    print("질문: SMOKE 모델에서 객체의 3D 위치(X, Y, Z)를 결정하는 과정은?\n")

    print("보기:")
    print("  A) Anchor box를 3D로 확장하여 매칭")
    print("  B) Heatmap으로 중심점(u,v) 검출 → Z 회귀 → X,Y는 카메라 모델로 계산")
    print("  C) Depth map을 먼저 추정한 후 BBox 영역의 depth 추출")
    print("  D) 3D 점구름을 입력으로 받아 직접 3D box 회귀")


def problem2_fcos3d_fpn():
    print("\n" + "━" * 28)
    print("문제 2: FCOS3D FPN 할당")
    print("━" * 28 + "\n")

    print("질문: FCOS3D에서 FPN 레벨 할당 기준은?\n")
    print("  (기존 FCOS는 2D bbox '크기'로 할당)\n")

    print("보기:")
    print("  A) 2D bbox의 크기 (면적)")
    print("  B) 객체의 클래스 (Car, Pedestrian 등)")
    print("  C) 객체의 깊이 (depth)")
    print("  D) 객체의 회전 각도 (yaw)")


def problem3_sincos_encoding():
    print("\n" + "━" * 28)
    print("문제 3: sin/cos 인코딩")
    print("━" * 28 + "\n")

    print("질문: 3D 회전각(yaw)을 sin/cos로 인코딩하는 이유는?\n")

    print("보기:")
    print("  A) 계산 속도를 높이기 위해")
    print("  B) 각도의 주기성(0도=360도) 문제를 해결하기 위해")
    print("  C) GPU 메모리를 줄이기 위해")
    print("  D) 3D bbox 크기를 함께 인코딩하기 위해")


def problem4_depth_difficulty():
    print("\n" + "━" * 28)
    print("문제 4: Depth 추정의 어려움")
    print("━" * 28 + "\n")

    print("질문: Monocular 3D Detection에서 depth 추정이")
    print("      가장 어려운 근본적인 이유는?\n")

    print("보기:")
    print("  A) GPU 메모리가 부족해서")
    print("  B) 학습 데이터가 적어서")
    print("  C) 단안 이미지에서 깊이가 근본적으로 모호(Depth Ambiguity)하므로")
    print("  D) Loss function 설계가 어려워서")


if __name__ == "__main__":
    print("━" * 33)
    print("  Phase 6 Week 4 Quiz - Easy")
    print("━" * 33)

    problem1_smoke_keypoint()
    problem2_fcos3d_fpn()
    problem3_sincos_encoding()
    problem4_depth_difficulty()

    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
