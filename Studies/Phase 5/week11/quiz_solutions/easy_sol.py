"""Phase 5 Week 11 - 기초 정답"""


def p1():
    print("\n정답: B) CLIP")
    print("  OpenVLA 직접 사용:")
    print("    DINOv2 (spatial)")
    print("    SigLIP (semantic, CLIP 의 sigmoid 변종)")
    print("  CLIP 자체는 안 씀 (SigLIP 가 후속)")


def p2():
    print("\n정답: B) ~ 630M")
    print("  DINOv2 ViT-L (300M) + SigLIP ViT-L (300M) + projector (30M) = 630M")


def p3():
    print("\n정답: B) ~ 95M (~1.25%)")
    print("  Projector ~30M + Llama LoRA r=32 ~65M = 95M")
    print("  Base 7.6B 대비 ~1.25%")
    print("  -> 빠른 학습 (5-10 hr), 적은 GPU memory")


def p4():
    print("\n정답: B) ~ 452 tokens")
    print("  DINOv2 patch 14, 224 image -> 256")
    print("  SigLIP patch 16, 224 image -> 196")
    print("  Concat -> 452 vision tokens")
    print("  + ~30 text tokens = ~482 total prompt")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
