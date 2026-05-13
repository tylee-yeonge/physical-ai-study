"""Phase 5 Week 7 - 중급 (DINOv2)"""


def p1():
    """Self-distillation 의 collapse 방지"""
    print("\n문제 1: Self-distillation 의 collapse 방지")
    print("  Naive 한 self-distillation 은 collapse 발생:")
    print("    -> 모든 image 가 같은 representation (trivial 해결책)")
    print()
    print("  DINOv2 의 방지 방법:")
    print("  A) Teacher 의 weight 를 random 초기화")
    print("  B) Teacher 의 EMA + centering + sharpening")
    print("  C) batch normalization 강화")
    print("  D) Dropout 사용")
    # TODO
    answer = ""
    expected = "B"
    print(f"  당신: {answer}, 기대: {expected}")


def p2():
    """DINOv2 ViT-L 의 spatial resolution"""
    print("\n문제 2: DINOv2 ViT-L 의 spatial resolution")
    img = 224
    patch = 14
    grid = img // patch
    seq = grid * grid + 1  # CLS

    # TODO
    n_patches = 0
    spatial_grid = 0
    expected_n = grid * grid
    expected_g = grid

    print(f"  당신: n_patches={n_patches}, grid={spatial_grid}x{spatial_grid}")
    print(f"  기대: n_patches={expected_n}, grid={expected_g}x{expected_g}")


def p3():
    """DINOv2 vs CLIP 의 feature 차이"""
    print("\n문제 3: DINOv2 vs CLIP feature")
    print("  같은 image 의 patch feature:")
    print()
    print("  DINOv2: 비슷한 spatial region 의 patch 끼리 close (PCA 결과)")
    print("  CLIP  : 같은 semantic class 의 patch 끼리 close")
    print()
    print("  Robot manipulation 에 필요:")
    print("    'pick up the red cup' = semantic ('red cup') + spatial ('어디')")
    print("    -> 두 정보 모두 필요 -> DINOv2 + SigLIP hybrid")


if __name__ == "__main__":
    p1(); p2(); p3()
