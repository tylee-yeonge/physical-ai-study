"""Phase 5 Week 3 - 중급 퀴즈"""


def p1():
    """OpenVLA breakdown 비중 계산"""
    print("\n문제 1: Latency 비중")
    print("  총 165 ms, vision = 58 ms")
    # TODO
    vision_pct = 0.0
    expected = 58 / 165 * 100
    print(f"  당신: {vision_pct:.1f}% (기대 {expected:.1f}%)")


def p2():
    """LM 최적화 vs Vision 최적화의 효과 비교"""
    print("\n문제 2: 어떤 최적화가 더 큰 효과?")
    print("  LM speculative decoding -> LM 50% 단축")
    print("  Vision pruning         -> Vision 50% 단축")
    # TODO: 효과 계산
    new_lm = 0
    new_vision = 0
    expected_lm = 165 - 95 * 0.5  # 117.5
    expected_vision = 165 - 58 * 0.5  # 136
    print(f"  당신:    LM 후 {new_lm}, Vision 후 {new_vision}")
    print(f"  기대:    LM 후 {expected_lm}, Vision 후 {expected_vision}")
    print("  -> LM 최적화 효과가 ~ 2배 큼 (95 vs 58)")


def p3():
    """병렬화 효과 계산"""
    print("\n문제 3: DINOv2 + SigLIP 병렬화")
    seq = 28 + 28  # = 56
    par = max(28, 28)  # = 28
    # TODO: 효과
    saving = 0
    print(f"  순차: {seq} ms")
    print(f"  병렬 이론: {par} ms")
    print(f"  Saving: {saving} ms (기대 {seq - par})")
    print(f"  Total latency: 165 -> ~ 137 ms (이론), ~ 155 ms (실제 4070)")


if __name__ == "__main__":
    p1(); p2(); p3()
