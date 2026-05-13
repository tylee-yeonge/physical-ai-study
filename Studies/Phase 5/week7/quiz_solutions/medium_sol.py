"""Phase 5 Week 7 - 중급 정답"""


def p1():
    print("\n정답: B) EMA + centering + sharpening")
    print()
    print("  Self-distillation collapse 방지 trick:")
    print("    1. Teacher = EMA of student (slow follower)")
    print("    2. Centering: teacher output 의 mean 빼기 (batch 평균 균등화)")
    print("    3. Sharpening: temperature scaling (softmax 의 sharpening)")
    print()
    print("  세 가지 함께 -> trivial collapse 방지 + meaningful learning")


def p2():
    img, patch = 224, 14
    grid = img // patch
    n = grid * grid
    print(f"\n정답: n_patches = {n} ({grid}x{grid})")
    print(f"  224 / 14 = 16")
    print(f"  16 x 16 = 256 patches")
    print(f"  + CLS = 257 (sequence length)")


def p3():
    print("\n정답: spatial vs semantic 보완")
    print()
    print("  DINOv2 patch feature PCA:")
    print("    - 같은 object 가 같은 색")
    print("    - background 와 foreground 구분")
    print("    - object boundary 명확")
    print()
    print("  CLIP / SigLIP feature:")
    print("    - 'cat' 의 patches 가 같은 색")
    print("    - object identity 강함")
    print()
    print("  OpenVLA 는 두 정보 모두 필요 -> hybrid 결합")


if __name__ == "__main__":
    p1(); p2(); p3()
