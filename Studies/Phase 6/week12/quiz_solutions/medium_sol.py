"""Phase 6 Week 12 - 중급 정답"""


def p1():
    print("\n정답: Image gap 이 dominant")
    print("  Noise variance: 27x 차이 (DR 전)")
    print("  -> Phase 7 의 첫 작업: DR 정교화 우선")


def p2():
    print("\n정답: Phase 7 의 우선 작업 = Image DR")
    print()
    print("  Phase 6 의 dominant gap 에 따라 Phase 7 의 첫 작업 결정")
    print("  Image: DR 정교화 (Domain Randomization)")
    print("  Force: Friction tuning")
    print()
    print("  본 phase: Image dominant -> Phase 7 week 1~2 DR 강화")


def p3():
    print("\n정답: 분기 재평가 #2 매트릭스")
    print()
    print("  Best case (A): Phase 6 정상 + 신 모델 X -> 원안 (2027.07)")
    print("  Updated (B): 신 모델 등장 -> 모델 갱신 + 원안")
    print("  Compressed (C): Phase 6 지연 -> Phase 7 4주 압축")
    print("  Fallback (D): 시장 정체 -> 2028.03 + Jetson 옵션")


if __name__ == "__main__":
    p1(); p2(); p3()
