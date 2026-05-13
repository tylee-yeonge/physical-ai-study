"""Phase 5 Week 3 - 중급 퀴즈 정답"""


def p1():
    print("\n정답: Vision 비중")
    pct = 58 / 165 * 100
    print(f"  58 / 165 = {pct:.1f}%")


def p2():
    print("\n정답: LM 최적화 우위")
    lm_new = 165 - 95 * 0.5
    v_new = 165 - 58 * 0.5
    print(f"  LM 50% 단축    : 165 -> {lm_new:.1f} ms")
    print(f"  Vision 50% 단축: 165 -> {v_new:.1f} ms")
    print(f"  -> LM 최적화가 ~ 19 ms 더 큰 효과")
    print()
    print("  [tip] 본 phase 7 의 산출물 #4 에서:")
    print("    - speculative decoding 시도 가치 있음 (LM 의 50%+)")
    print("    - Vision 최적화는 marginal")


def p3():
    print("\n정답: 병렬화")
    save = 56 - max(28, 28)
    print(f"  순차: 28 + 28 = 56 ms")
    print(f"  병렬 (max): 28 ms")
    print(f"  Saving: {save} ms")
    print()
    print("  실제 RTX 4070 에서:")
    print("    SM 수 부족으로 완전 병렬 X")
    print("    ~ 5~10 ms 절약 실제")
    print()
    print("  총 latency 165 -> ~ 155 ms 정도 (현실)")


if __name__ == "__main__":
    p1(); p2(); p3()
