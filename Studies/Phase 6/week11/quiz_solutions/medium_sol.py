"""Phase 6 Week 11 - 중급 정답"""


def p1():
    print("\n정답: A) Real 분포 + 1.5x")
    print()
    print("  표준 DR 권장:")
    print("    1. Real environment 의 light intensity 측정 (예: 1000~4000 lumens)")
    print("    2. Sim 의 random range: 700~5500 (1.5x)")
    print()
    print("  너무 좁으면 Real 의 outlier 대응 못 함")
    print("  너무 넓으면 Sim 학습 시 noise 너무 큼")


def p2():
    print("\n정답: hist distance threshold")
    print("  Default ~ 145")
    print("  After DR ~ 30~50 (good)")
    print()
    print("  Phase 7 의 산출물 #4 의 보고서:")
    print("    - hist distance before DR / after DR 두 값 인용")
    print("    - '4x 가까워짐' 같은 정량 표현")


def p3():
    print("\n정답: action L2 distance < 0.01")
    print()
    print("  Robust 기준:")
    print("    Sim action ~ Real action -> robust (양산 가능)")
    print("    Sim action != Real action -> Sim/Real 차이 큼")
    print()
    print("  Domain Randomization 의 효과 측정:")
    print("    No DR: action diff > 0.05 (불안정)")
    print("    With DR: action diff < 0.02 (robust)")


if __name__ == "__main__":
    p1(); p2(); p3()
