"""
Phase 6 Week 4 - Monocular 3D Detection 모델 기초 퀴즈 풀이
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 풀이: SMOKE 핵심 아이디어")
    print("━" * 28 + "\n")

    print("  정답: B) Heatmap으로 중심점 검출 → Z 회귀 → X,Y 카메라 모델 계산\n")
    print("  SMOKE의 3D 위치 결정 과정:")
    print("    1. Backbone(DLA-34) → Feature map")
    print("    2. Heatmap head → 객체 중심 (u, v) 검출")
    print("    3. Regression head → 깊이 Z 예측")
    print("    4. 카메라 내부 파라미터로 X, Y 계산:")
    print("       X = (u - cx) * Z / fx")
    print("       Y = (v - cy) * Z / fy")
    print()
    print("  핵심: Z(깊이)만 정확히 예측하면 X, Y는 자동 결정!")
    print("  → CenterNet 기반 Keypoint 방식의 장점")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 풀이: FCOS3D FPN 할당")
    print("━" * 28 + "\n")

    print("  정답: C) 객체의 깊이 (depth)\n")
    print("  FCOS (2D): 2D bbox 크기로 FPN 레벨 할당")
    print("    작은 객체 → P3 (고해상도)")
    print("    큰 객체 → P7 (저해상도)")
    print()
    print("  FCOS3D: 깊이(depth)로 FPN 레벨 할당")
    print("    가까운 객체 → P3 (이미지에서 크게 보임)")
    print("    먼 객체 → P7 (이미지에서 작게 보임)")
    print()
    print("  이유: 3D 환경에서 멀리 있는 객체가 이미지에서")
    print("  작게 보이므로, 깊이 기반 할당이 자연스러움")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 풀이: sin/cos 인코딩")
    print("━" * 28 + "\n")

    print("  정답: B) 각도의 주기성(0도=360도) 문제를 해결하기 위해\n")
    print("  문제 상황:")
    print("    GT theta = 0.01 rad")
    print("    Pred theta = 6.27 rad (= 2*pi - 0.01)")
    print("    직접 회귀: L1 = |0.01 - 6.27| = 6.26 (매우 큼!)")
    print("    실제 각도 차이: 0.02 rad (거의 같은 방향)")
    print()
    print("  sin/cos 해결:")
    print("    theta=0.01: sin=0.01, cos=1.00")
    print("    theta=6.27: sin=-0.01, cos=1.00")
    print("    L1 = |0.01-(-0.01)| + |1.00-1.00| = 0.02 (적절!)")
    print()
    print("  복원: theta = arctan2(sin, cos)")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 풀이: Depth 추정의 어려움")
    print("━" * 28 + "\n")

    print("  정답: C) Depth Ambiguity (깊이의 근본적 모호성)\n")
    print("  Depth Ambiguity란:")
    print("    - 하나의 2D 이미지 → 무한한 3D 해석 가능")
    print("    - 작은 가까운 물체 vs 큰 먼 물체가 같아 보임")
    print()
    print("  추가적인 어려움:")
    print("    - 깊이의 비선형성: u = fx*X/Z (Z에 역비례)")
    print("    - 학습 데이터 불균형: 가까운 객체 >> 먼 객체")
    print("    - 시각적 단서 한계: 크기, 원근법, 그림자 등")
    print()
    print("  → LiDAR/Stereo 없이 단안 카메라만으로는")
    print("    깊이 정보를 추론해야 하는 본질적 한계")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 4 Quiz Easy - 풀이")
    print("━" * 33)

    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()

    print("\n" + "━" * 33)
