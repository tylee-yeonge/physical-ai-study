"""Phase 5 Week 7 - 기초 정답"""


def p1():
    print("\n정답: B) Self-supervised")
    print("  Self-distillation (DINO) + iBOT (masked image modeling)")
    print("  label 전혀 없이 학습")


def p2():
    print("\n정답: B) Spatial / geometric")
    print("  Patch feature 가 spatial cluster 형성")
    print("  CLIP / SigLIP 와 보완적 (semantic vs spatial)")


def p3():
    print("\n정답: B) 14")
    print("  DINOv2 의 표준 patch_size 14 (CLIP 16, 표준 ViT 16)")
    print("  spatial resolution 약간 더 fine")


def p4():
    print("\n정답: C) LVD-142M")
    print("  142M curated images (label 없음)")
    print("  ImageNet 의 ~ 100x")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
