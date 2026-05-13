"""
Phase 5 Week 10 - Depth 정확도 검증 기초 퀴즈
"""


def problem1_stereo_depth_formula():
    print("\n" + "━" * 28)
    print("문제 1: 스테레오 깊이 공식")
    print("━" * 28 + "\n")

    print("질문: 스테레오 카메라에서 깊이(Z)를 구하는 공식은?\n")
    print("  f: 초점 거리 (픽셀)")
    print("  b: 기선 거리 (미터)")
    print("  d: 시차 (disparity, 픽셀)\n")

    print("보기:")
    print("  A) Z = f * d / b")
    print("  B) Z = f * b / d")
    print("  C) Z = d * b / f")
    print("  D) Z = b / (f * d)")


def problem2_absrel_meaning():
    print("\n" + "━" * 28)
    print("문제 2: AbsRel 지표의 의미")
    print("━" * 28 + "\n")

    print("질문: AbsRel = 0.08의 의미로 가장 적절한 것은?\n")

    print("보기:")
    print("  A) 예측 깊이가 평균적으로 0.08미터 틀림")
    print("  B) 예측 깊이가 GT 대비 평균 8% 오차")
    print("  C) 80%의 픽셀이 정확함")
    print("  D) RMSE가 0.08임")


def problem3_delta_threshold():
    print("\n" + "━" * 28)
    print("문제 3: delta 임계값 지표")
    print("━" * 28 + "\n")

    print("질문: delta < 1.25 = 0.96의 의미는?\n")

    print("보기:")
    print("  A) 96%의 픽셀에서 예측/GT 비율이 1.25 미만")
    print("  B) 예측 깊이의 96%가 1.25미터 이내")
    print("  C) RMSE가 1.25 미만인 비율이 96%")
    print("  D) 96%의 픽셀이 25% 이내 오차")


def problem4_depth_weakness():
    print("\n" + "━" * 28)
    print("문제 4: Depth 모델의 취약점")
    print("━" * 28 + "\n")

    print("질문: 단안(monocular) Depth 추정 모델이 가장 어려워하는 상황은?\n")

    print("보기:")
    print("  A) 텍스처가 풍부한 실내 장면")
    print("  B) 잘 조명된 도로 위의 차량")
    print("  C) 반사 표면 (유리, 거울, 물웅덩이)")
    print("  D) 일반적인 건물 외벽")


def problem5_scale_ambiguity():
    print("\n" + "━" * 28)
    print("문제 5: 스케일 모호성")
    print("━" * 28 + "\n")

    print("질문: Depth Anything이 출력하는 '상대적 깊이'를")
    print("      '절대적 깊이'로 변환하려면 무엇이 필요한가?\n")

    print("보기:")
    print("  A) 더 큰 모델 (ViT-L)을 사용하면 자동으로 절대 깊이가 됨")
    print("  B) Scale과 Shift를 구해서 d_abs = scale * d_rel + shift로 변환")
    print("  C) FP32로 추론하면 절대 깊이가 됨")
    print("  D) 해상도를 높이면 절대 깊이가 됨")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 10 Quiz - Easy")
    print("━" * 33)
    problem1_stereo_depth_formula()
    problem2_absrel_meaning()
    problem3_delta_threshold()
    problem4_depth_weakness()
    problem5_scale_ambiguity()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
