"""
Phase 5 Week 10 - Depth 정확도 검증 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: B) Z = f * b / d")
    print("━" * 28 + "\n")

    print("해설:")
    print("  스테레오 깊이 공식: Z = (f * b) / d")
    print()
    print("  f: 초점 거리 (focal length, 픽셀)")
    print("  b: 기선 거리 (baseline, 미터)")
    print("  d: 시차 (disparity, 픽셀)")
    print()
    print("  직관적 이해:")
    print("  - 시차(d)가 크다 → 물체가 가깝다 → Z가 작다")
    print("  - 시차(d)가 작다 → 물체가 멀다 → Z가 크다")
    print("  → d가 분모에 있어야 함!")
    print()
    print("  예시:")
    print("  f=721px, b=0.54m, d=50px")
    print(f"  Z = 721 * 0.54 / 50 = {721 * 0.54 / 50:.2f}m")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: B) GT 대비 평균 8% 오차")
    print("━" * 28 + "\n")

    print("해설:")
    print("  AbsRel = mean(|pred - gt| / gt)")
    print()
    print("  AbsRel = 0.08은:")
    print("  '예측 깊이가 GT 깊이 대비 평균적으로 8%만큼 차이난다'")
    print()
    print("  예시:")
    print("  GT = 10m일 때 → 평균 오차 약 0.8m")
    print("  GT = 50m일 때 → 평균 오차 약 4.0m")
    print("  → '상대적' 지표이므로 거리에 따라 절대 오차가 달라짐")
    print()
    print("  KITTI 벤치마크 기준: AbsRel < 0.06이면 우수")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: A) 96% 픽셀에서 비율 < 1.25")
    print("━" * 28 + "\n")

    print("해설:")
    print("  delta < t = mean(max(pred/gt, gt/pred) < t)")
    print()
    print("  max(pred/gt, gt/pred)의 의미:")
    print("  → 예측과 GT의 비율이 1에 가까울수록 정확")
    print("  → 1.25보다 작다 = 25% 이내 오차")
    print()
    print("  delta < 1.25 = 0.96:")
    print("  '전체 유효 픽셀의 96%에서 예측/GT 비율이 1.25 미만'")
    print("  → 즉 96%의 픽셀이 25% 이내 오차")
    print()
    print("  단계별 threshold:")
    print("  delta < 1.25:   25% 이내 오차 비율")
    print("  delta < 1.25^2: ~56% 이내 오차 비율")
    print("  delta < 1.25^3: ~95% 이내 오차 비율")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: C) 반사 표면")
    print("━" * 28 + "\n")

    print("해설:")
    print("  Depth 모델의 대표적 취약점:")
    print()
    print("  1. 반사 표면 (가장 어려움):")
    print("     - 거울: 반사된 물체의 깊이를 추정")
    print("     - 유리: 투과/반사가 혼합")
    print("     - 물웅덩이: 하늘/건물 반사")
    print("     → 실제 표면 깊이 대신 반사 물체 깊이 출력!")
    print()
    print("  2. 경계선 (번짐 현상):")
    print("     - Depth bleeding")
    print("     - 패치 경계와 물체 경계 불일치")
    print()
    print("  3. 원거리 물체:")
    print("     - 이미지에서 작은 영역")
    print("     - 텍스처 부족")
    print("     - 스케일 모호성 심화")
    print()
    print("  A, B, D는 텍스처가 풍부하여 상대적으로 정확합니다.")


def problem5_solution():
    print("\n" + "━" * 28)
    print("문제 5 정답: B) Scale + Shift 변환")
    print("━" * 28 + "\n")

    print("해설:")
    print("  Monocular Depth의 본질적 한계 = Scale Ambiguity")
    print()
    print("  Depth Anything은 상대적 깊이만 출력:")
    print("  → 'A가 B보다 가깝다'는 알지만")
    print("  → 'A는 3m, B는 5m'는 모름")
    print()
    print("  절대 깊이로 변환:")
    print("  d_abs = scale * d_rel + shift")
    print()
    print("  scale과 shift를 구하는 방법:")
    print("  1. 스테레오 카메라 GT와 비교 (최소자승법)")
    print("  2. LiDAR sparse point로 보정")
    print("  3. 알려진 물체 크기 참조 (차량 높이 등)")
    print("  4. IMU + Visual Odometry로 추정")
    print()
    print("  모델 크기(A), 정밀도(C), 해상도(D)는 스케일과 무관합니다.")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 10 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    print("\n" + "━" * 33)
