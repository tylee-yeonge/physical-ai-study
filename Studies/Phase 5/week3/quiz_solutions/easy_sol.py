"""Phase 5 Week 3 - 기초 퀴즈 정답"""


def p1():
    print("\n정답: C) ~ 600M")
    print("  ViT-L 300M * 2 (DINOv2 + SigLIP) = 600M")
    print("  Llama 2 7B 의 ~ 8.5%")


def p2():
    print("\n정답: C) LM decoder generate")
    print("  Latency breakdown:")
    print("    Vision (DINOv2+SigLIP) : 58 ms (35%)")
    print("    LM decoder generate    : 95 ms (58%) <- 최대")
    print("    기타                   : 12 ms (7%)")


def p3():
    print("\n정답: B) Sequence concat + projector")
    print("  DINOv2 patches + SigLIP patches -> concat along sequence")
    print("  -> MLP projector -> Llama hidden dim (4096)")
    print()
    print("  Cross-attention / 가중평균 등은 더 복잡 -> OpenVLA 안 씀.")


def p4():
    print("\n정답: B) OpenVLA vision = ViT-L 응용")
    print("  OpenVLA vision encoder:")
    print("    DINOv2 = ViT-L (patch 14, self-supervised)")
    print("    SigLIP = ViT-L (patch 16, image-text contrastive)")
    print("  -> ViT 의 모든 개념 (patch + attention + pos) 그대로 적용")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
