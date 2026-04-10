"""
Phase 6 Week 8 - BEV 개념 이해 기초 퀴즈
"""


def problem1_bev_definition():
    print("\n" + "━" * 28)
    print("문제 1: BEV 정의")
    print("━" * 28 + "\n")

    print("질문: Bird's Eye View(BEV)의 정의로 가장 적절한 것은?\n")

    print("보기:")
    print("  A) 카메라 전방 시점 그대로의 2D 이미지")
    print("  B) 3D 공간을 위에서 X-Y 평면으로 투영한 2D 표현")
    print("  C) LiDAR 포인트 클라우드를 그대로 시각화한 것")
    print("  D) 카메라 후방 시점의 이미지")


def problem2_ipm_limitation():
    print("\n" + "━" * 28)
    print("문제 2: IPM 한계")
    print("━" * 28 + "\n")

    print("질문: IPM(Inverse Perspective Mapping)의 핵심 가정과 한계는?\n")

    print("보기:")
    print("  A) GPU가 필요하며 학습에 시간이 오래 걸림")
    print("  B) 지면이 평평하다고 가정하므로, 경사로와 3D 객체에서 왜곡 발생")
    print("  C) 6대 카메라가 반드시 필요함")
    print("  D) Depth 네트워크를 별도로 학습해야 함")


def problem3_lift_splat():
    print("\n" + "━" * 28)
    print("문제 3: Lift-Splat-Shoot")
    print("━" * 28 + "\n")

    print("질문: Lift-Splat-Shoot에서 'Lift' 단계의 역할은?\n")

    print("보기:")
    print("  A) BEV Feature에서 3D bbox를 검출")
    print("  B) 각 픽셀에 Depth 분포를 예측하여 2D Feature를 3D로 확장")
    print("  C) 이미지를 Homography로 변환")
    print("  D) Temporal 정보를 이전 프레임에서 가져옴")


def problem4_bevformer_advantage():
    print("\n" + "━" * 28)
    print("문제 4: BEVFormer 장점")
    print("━" * 28 + "\n")

    print("질문: BEVFormer가 Lift-Splat-Shoot 대비 가지는 핵심 장점은?\n")

    print("보기:")
    print("  A) 연산량이 훨씬 적음")
    print("  B) Depth를 명시적으로 예측하지 않아도 되며, Temporal 정보를 자연스럽게 활용")
    print("  C) 평면 가정을 사용하여 정확도를 높임")
    print("  D) 학습 데이터가 적어도 동작함")


if __name__ == "__main__":
    print("━" * 33)
    print("  Phase 6 Week 8 Quiz - Easy")
    print("━" * 33)

    problem1_bev_definition()
    problem2_ipm_limitation()
    problem3_lift_splat()
    problem4_bevformer_advantage()

    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
