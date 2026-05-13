"""Phase 7 Week 1 - 중급"""


def p1():
    print("\n문제 1: trainable params 계산")
    print("  projector (~30M) + Llama LoRA r=32 (~65M)")
    total = 30 + 65
    print(f"  Total: ~{total}M")


def p2():
    print("\n문제 2: 학습 step 수")
    n_ep = 300; steps_per_ep = 50; epochs = 2
    total = n_ep * steps_per_ep * epochs
    print(f"  300 ep * 50 step * 2 epoch = {total} steps")


def p3():
    print("\n문제 3: success rate 목표")
    print("  Baseline (zero-shot): 0-20%")
    print("  After LoRA: > 70%")
    print("  -> Phase 7 의 핵심 지표")


if __name__ == "__main__":
    p1(); p2(); p3()
