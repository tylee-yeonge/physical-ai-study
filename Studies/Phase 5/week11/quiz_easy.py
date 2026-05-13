"""Phase 5 Week 11 - 기초 (통합)"""


def p1():
    print("\n문제 1: 4 vision foundation model 중 OpenVLA 가 직접 안 쓰는 것")
    print("  A) ViT\n  B) CLIP\n  C) DINOv2\n  D) SigLIP")


def p2():
    print("\n문제 2: OpenVLA vision encoder 총 params")
    print("  A) ~ 300M\n  B) ~ 630M (DINOv2 300 + SigLIP 300 + projector 30)\n  C) ~ 1.5B\n  D) ~ 7B")


def p3():
    print("\n문제 3: Phase 7 LoRA fine-tune trainable params")
    print("  A) 7B (전체)")
    print("  B) ~ 95M (projector + Llama LoRA, ~1.25%)")
    print("  C) ~ 1B")
    print("  D) ~ 10M")


def p4():
    print("\n문제 4: OpenVLA inference 의 vision token 수 (224 image)")
    print("  A) ~ 197 (CLS + 196)")
    print("  B) ~ 452 (DINOv2 256 + SigLIP 196)")
    print("  C) ~ 50")
    print("  D) ~ 4096")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
