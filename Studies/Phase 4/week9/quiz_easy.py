"""
Phase 6 Week 9 - BEVFormer 이해 기초 퀴즈
"""


def problem1_bev_coverage():
    print("\n" + "━" * 28)
    print("문제 1: BEV Query 커버 범위")
    print("━" * 28 + "\n")

    print("질문: BEV Query Grid가 200x200이고 셀 크기가 0.5m일 때,")
    print("      커버하는 물리적 범위는?\n")

    print("보기:")
    print("  A) 50m x 50m")
    print("  B) 100m x 100m")
    print("  C) 200m x 200m")
    print("  D) 400m x 400m")


def problem2_spatial_cross_attention():
    print("\n" + "━" * 28)
    print("문제 2: Spatial Cross-Attention")
    print("━" * 28 + "\n")

    print("질문: Spatial Cross-Attention에서 BEV Query 하나에 대해")
    print("      4개 높이, 6대 카메라를 사용하면 총 몇 개의")
    print("      Reference Point가 생성되는가?\n")

    print("보기:")
    print("  A) 4개")
    print("  B) 6개")
    print("  C) 10개")
    print("  D) 24개")


def problem3_temporal_attention():
    print("\n" + "━" * 28)
    print("문제 3: Temporal Self-Attention")
    print("━" * 28 + "\n")

    print("질문: Temporal Self-Attention에서 ego-motion 보상이 필요한")
    print("      가장 핵심적인 이유는?\n")

    print("보기:")
    print("  A) GPU 메모리를 절약하기 위해")
    print("  B) 차량 이동으로 인해 이전 BEV와 현재 BEV의 좌표가 어긋나기 때문에")
    print("  C) 이미지 해상도를 높이기 위해")
    print("  D) Backbone의 학습 속도를 높이기 위해")


def problem4_bevformer_advantage():
    print("\n" + "━" * 28)
    print("문제 4: BEVFormer vs BEVDet")
    print("━" * 28 + "\n")

    print("질문: BEVFormer가 BEVDet(Lift-Splat-Shoot 기반)에 비해")
    print("      가지는 핵심적인 장점은?\n")

    print("보기:")
    print("  A) 더 적은 카메라를 사용할 수 있다")
    print("  B) 명시적 Depth 예측이 필요 없고, Temporal 정보를 활용할 수 있다")
    print("  C) LiDAR 데이터를 함께 사용할 수 있다")
    print("  D) 더 적은 GPU 메모리를 사용한다")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 9 Quiz - Easy")
    print("━" * 33)
    problem1_bev_coverage()
    problem2_spatial_cross_attention()
    problem3_temporal_attention()
    problem4_bevformer_advantage()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
