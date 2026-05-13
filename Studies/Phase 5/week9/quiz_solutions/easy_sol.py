"""Phase 5 Week 9 - 기초 정답"""


def p1():
    print("\n정답: B) Sequence concat -> projector")
    print("  DINOv2 256 tokens + SigLIP 196 tokens -> 452 tokens")
    print("  Projector (MLP) 가 vision dim 1024 -> Llama dim 4096")
    print("  단순한 fusion 방식. cross-attention 같은 복잡한 방식 X.")


def p2():
    print("\n정답: B) Vision dim -> Llama dim")
    print("  Projector:")
    print("    nn.Linear(1024, 4096) -> nn.GELU() -> nn.Linear(4096, 4096)")
    print("    ~ 30M params")
    print("  Vision encoder 의 출력을 Llama 가 처리 가능한 형태로.")


def p3():
    print("\n정답: C) Projector + Llama LoRA, vision frozen")
    print("  LoRA fine-tune (Phase 7):")
    print("    DINOv2 / SigLIP : frozen (pre-trained)")
    print("    Projector       : 학습 (~30M)")
    print("    Llama LoRA r=32 : ~65M")
    print("    Total trainable : ~95M (전체 8B 의 ~1.2%)")


def p4():
    print("\n정답: B) 224 빠름, 384 정확")
    print("  224 input:")
    print("    vision tokens 452")
    print("    latency ~ 28 ms (DINOv2 ViT-L)")
    print("    일반 manipulation OK")
    print()
    print("  384 input:")
    print("    vision tokens ~ 1150")
    print("    latency ~ 60 ms (attention FLOPS 6x)")
    print("    fine task (mm 단위) 정확도 향상")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
