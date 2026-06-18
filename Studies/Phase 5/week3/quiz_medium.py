"""Phase 5 Week 3 - 중급 퀴즈"""


def p1():
    """OpenVLA breakdown 비중 계산"""
    print("\n문제 1: Latency 비중")
    print("  총 165 ms, vision = 58 ms")
    # TODO
    vision_pct = 0.0
    print(f"  당신: {vision_pct:.1f}%")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """LM 최적화 vs Vision 최적화의 효과 비교"""
    print("\n문제 2: 어떤 최적화가 더 큰 효과?")
    print("  LM speculative decoding -> LM 50% 단축")
    print("  Vision pruning         -> Vision 50% 단축")
    # TODO: 효과 계산
    new_lm = 0
    new_vision = 0
    print(f"  당신:    LM 후 {new_lm}, Vision 후 {new_vision}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """병렬화 효과 계산"""
    print("\n문제 3: DINOv2 + SigLIP 병렬화")
    seq = 28 + 28  # = 56
    par = max(28, 28)  # = 28
    # TODO: 효과
    saving = 0
    print(f"  순차: {seq} ms")
    print(f"  병렬 이론: {par} ms")
    print(f"  Saving: {saving} ms")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    p1(); p2(); p3()
