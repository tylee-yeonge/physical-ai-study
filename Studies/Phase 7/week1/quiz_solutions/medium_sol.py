"""Phase 7 Week 1 - 중급 정답"""


def p1():
    print("\n정답: ~95M")
    print("  projector 30M + Llama LoRA 65M")


def p2():
    print("\n정답: 30000 steps")
    print("  300 ep * 50 step * 2 epoch = 30K")
    print("  RTX 4070 1 step ~ 1초 -> 8 시간")


def p3():
    print("\n정답: > 70% success rate")
    print("  Phase 7 의 결정타 영상에 인용")
    print("  Baseline (zero-shot) 과의 대비가 핵심")


if __name__ == "__main__":
    p1(); p2(); p3()
