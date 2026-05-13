"""Phase 5 Week 7 - 기초 퀴즈 (DINOv2)"""


def p1():
    print("\n문제 1: DINOv2 의 학습 방식")
    print("  A) Supervised (label 있음)")
    print("  B) Self-supervised (label 없음, self-distillation + iBOT)")
    print("  C) Contrastive image-text")
    print("  D) Generative")


def p2():
    print("\n문제 2: DINOv2 의 강점")
    print("  A) Semantic 정보\n  B) Spatial / geometric 정보\n  C) Text generation\n  D) Action prediction")


def p3():
    print("\n문제 3: DINOv2 patch_size")
    print("  A) 8\n  B) 14\n  C) 16\n  D) 32")


def p4():
    print("\n문제 4: DINOv2 학습 데이터")
    print("  A) ImageNet 1.28M\n  B) WIT 400M\n  C) LVD-142M (label 없음)\n  D) JFT-300M")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
