"""
Phase 5 Week 2 - 기초 퀴즈 정답
"""


def p1():
    print("\n문제 1 정답: B) ~ 30 ms\n")
    print("  ViT-B (86M)  : ~ 10 ms")
    print("  ViT-L (300M) : ~ 30 ms (OpenVLA backbone)")
    print("  ViT-H (632M) : ~ 60 ms")


def p2():
    print("\n문제 2 정답: B) 회전 / scale 변화\n")
    print("  ViT 의 약점:")
    print("    1. Out-of-distribution")
    print("    2. Augmentation 없이 회전 / scale (translation invariance 없음)")
    print("    3. Adversarial example")
    print("  Data augmentation (RandomRotation, RandomResizedCrop 등) 필수.")


def p3():
    print("\n문제 3 정답: B) ~ 60 ms (35%)\n")
    print("  OpenVLA 165 ms breakdown:")
    print("    - Vision (DINOv2 + SigLIP, 2*ViT-L) : ~ 60 ms")
    print("    - LM decoder generate                : ~ 95 ms (가장 큼)")
    print("    - 기타 (preprocess, postprocess)     : ~ 10 ms")
    print()
    print("  LM decoder 가 가장 큰 비중. vision 은 35% 차지.")


def p4():
    print("\n문제 4 정답: B) Sequence 짧음 -> 빠름 + 정확도 약간 손실\n")
    print("  patch_size 32 (vs 16):")
    print("    - seq_len: 50 (vs 197) -> attention FLOPS 16x 감소")
    print("    - latency: ~3x 빠름")
    print("    - 정확도: 약 1~2%p 감소 (fine spatial 정보 손실)")
    print()
    print("  OpenVLA 는 patch_size 14 (DINOv2 표준) 사용:")
    print("    - 16 보다 약간 더 fine spatial 정보")
    print("    - sequence 길이 차이 미미 (196 vs 256)")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
