"""Phase 5 Week 11 - 중급 정답"""


def p1():
    pct = 630e6 / 7.6e9 * 100
    print(f"\n정답: vision 비중 {pct:.2f}%")
    print(f"  Vision encoder: 630M (~8.3% of 7.6B)")
    print(f"  Llama 가 대부분 (~92%)")
    print()
    print("  하지만 latency 는 vision 35%, LM 58%")
    print("  -> param 비중 != latency 비중")
    print("  Llama 가 autoregressive 라 token 별 forward 가 누적")


def p2():
    base = 4.0
    lora = 0.2
    act = 4.0
    opt = 0.6
    over = 1.0
    total = base + lora + act + opt + over
    print(f"\n정답: {total:.2f} GB")
    print(f"  Base int4: 4.0")
    print(f"  LoRA fp16: 0.2")
    print(f"  Activation: 4.0")
    print(f"  Optimizer 3x: 0.6")
    print(f"  Overhead: 1.0")
    print(f"  Total: {total:.2f} GB")
    print(f"  RTX 4070 12GB fit: True (~ 2 GB 여유)")


def p3():
    print("\n정답:")
    print("  ViT    : foundation (모든 vision foundation 모델의 토대)")
    print("  CLIP   : not used (OpenVLA 는 SigLIP 사용, CLIP 의 후속)")
    print("  DINOv2 : used (spatial encoder)")
    print("  SigLIP : used (semantic encoder)")


if __name__ == "__main__":
    p1(); p2(); p3()
