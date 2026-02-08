"""
Phase 6 Week 8 - BEV 개념 이해 기초 퀴즈 풀이
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 풀이: BEV 정의")
    print("━" * 28 + "\n")

    print("  정답: B) 3D 공간을 위에서 X-Y 평면으로 투영한 2D 표현\n")
    print("  BEV (Bird's Eye View):")
    print("    - '새가 위에서 내려다본 시점'")
    print("    - 3D 세계를 위에서 X-Y 평면으로 투영")
    print("    - 높이(Z) 정보는 합산(sum) 또는 평균")
    print()
    print("  BEV의 핵심 가치:")
    print("    1. 다중 카메라 융합의 자연스러운 공간")
    print("    2. Occlusion 문제 해결")
    print("    3. 경로 계획과 직접 연결 가능")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 풀이: IPM 한계")
    print("━" * 28 + "\n")

    print("  정답: B) 지면이 평평하다고 가정 → 경사로/3D 객체에서 왜곡\n")
    print("  IPM의 핵심 가정: y = 0 (지면 평면)")
    print()
    print("  이 가정이 깨지는 경우:")
    print("    1. 경사로/언덕: 지면이 기울어져 BEV 왜곡")
    print("    2. 차량 등 3D 객체: 높이가 있어 윗부분이 늘어남")
    print("    3. 높이 정보 완전 손실: 신호등, 표지판 위치 부정확")
    print()
    print("  IPM은 학습이 필요 없고 빠르지만,")
    print("  실제 환경에서는 한계가 명확합니다.")
    print("  → 차선 검출, 주차장 등 제한된 환경에서 사용")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 풀이: Lift-Splat-Shoot")
    print("━" * 28 + "\n")

    print("  정답: B) 각 픽셀에 Depth 분포를 예측하여 2D → 3D 확장\n")
    print("  Lift-Splat-Shoot 3단계:")
    print()
    print("  1. Lift (들어올리기):")
    print("     - 각 픽셀에 대해 Depth 분포 예측")
    print("     - 2D Feature를 3D Feature Volume으로 확장")
    print("     - feature × depth_prob → 3D 공간에 배치")
    print()
    print("  2. Splat (펼치기):")
    print("     - 3D Feature Volume을 BEV 그리드에 투영")
    print("     - Z축 합산 (sum pooling)")
    print()
    print("  3. Shoot (쏘기):")
    print("     - BEV Feature에서 Detection/Segmentation 수행")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 풀이: BEVFormer 장점")
    print("━" * 28 + "\n")

    print("  정답: B) Depth 명시적 예측 불필요 + Temporal 자연스러운 활용\n")
    print("  BEVFormer vs Lift-Splat-Shoot:")
    print()
    print("  1. Depth 처리 방식:")
    print("     Lift-Splat: Depth 분포를 명시적으로 예측 → 오차 전파 위험")
    print("     BEVFormer: BEV Query → 카메라 투영 → Attention → 암묵적 학습")
    print()
    print("  2. Temporal 정보:")
    print("     Lift-Splat: 기본적으로 단일 프레임 (확장 가능)")
    print("     BEVFormer: Temporal Self-Attention 내장")
    print("       → 이전 프레임 BEV를 ego-motion 정렬 후 활용")
    print("       → 가려진 객체 복구, 속도 추정, 안정성 향상")
    print()
    print("  성능 비교:")
    print("    Lift-Splat (BEVDet): NDS ~0.39")
    print("    BEVFormer:           NDS ~0.52 (+13!)")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 8 Quiz Easy - 풀이")
    print("━" * 33)

    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()

    print("\n" + "━" * 33)
