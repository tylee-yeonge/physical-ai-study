"""Phase 5 Week 8 - 기초 (DINOv2 inference)"""


def p1():
    print("\n문제 1: DINOv2 patch feature 시각화 (PCA)")
    print("  A) Random pattern")
    print("  B) Spatial cluster (object 경계 명확)")
    print("  C) Semantic cluster (object identity)")
    print("  D) 흑백 image")


def p2():
    print("\n문제 2: DINOv2 ViT-L latency (RTX 4070)")
    print("  A) ~ 5 ms\n  B) ~ 28 ms\n  C) ~ 100 ms\n  D) ~ 500 ms")


def p3():
    print("\n문제 3: DINOv2 의 robot 응용")
    print("  A) Segmentation only")
    print("  B) Pose estimation only")
    print("  C) Segmentation + pose + grasp + SLAM (general feature)")
    print("  D) Text generation")


def p4():
    print("\n문제 4: DINOv2 vs SAM")
    print("  A) 같은 모델\n  B) SAM 이 더 강함\n  C) DINOv2 = general, SAM = segmentation 특화\n  D) DINOv2 가 항상 우위")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
