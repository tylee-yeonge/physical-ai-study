"""Phase 6 Week 12 - 중급"""


def p1():
    """Sim/Real gap 의 dominant factor"""
    print("\n문제 1: 4 gap 중 dominant factor (DR 전)")
    print("  Latency gap : -27 ms (작음)")
    print("  반복성 gap   : EE std Sim 0.05 vs Real 2.3 mm")
    print("  Force gap   : Friction 5x")
    print("  Image gap   : noise variance 27x")
    print()
    print("  가장 큰 gap?")
    # TODO
    ans = ""
    expected = "Image"
    print(f"  당신: {ans} (기대: {expected})")


def p2():
    """Phase 7 의 우선 작업"""
    print("\n문제 2: Phase 7 진입 시 우선 작업")
    print()
    print("  Phase 6 의 dominant gap 따라:")
    print("    Image gap 크다 -> Phase 7 의 첫 작업: Domain Randomization 강화")
    print("    Force gap 크다 -> Phase 7 의 첫 작업: Friction tuning")
    print()
    print("  본 phase 의 dominant: Image (noise)")
    print("  -> Phase 7 week 1~2 의 DR 정교화 우선")


def p3():
    """분기 재평가 #2 의 결정 매트릭스"""
    print("\n문제 3: 분기 재평가 #2 의 4 시나리오")
    print()
    print("  A) Phase 6 정상 + 신 VLA 모델 X -> 실지원 지속 (2027.02 개시)")
    print("  B) Phase 6 정상 + 신 모델 등장 -> 모델 갱신 + 실지원 지속")
    print("  C) Phase 6 지연 -> Phase 7 압축 또는 fallback")
    print("  D) 시장 정체 -> fallback 2028.03")
    print()
    print("  본 phase 정상 (4 gap 측정 완료) 가정 시:")
    print("    A 또는 B 가능")


if __name__ == "__main__":
    p1(); p2(); p3()
