"""Phase 5 Week 4 - 중급 퀴즈 정답"""


def p1():
    print("\n정답: InfoNCE 직관")
    print("  각 row 의 softmax 후 diag log prob 계산")
    print("  학습 진행 -> diag 값 만 커지고 off-diag 줄음")
    print("  -> 'image i 는 text i 와만 가까움' 학습")


def p2():
    print("\n정답: negative 수")
    print("  batch 32  -> 31 negative")
    print("  batch 4096 -> 4095 negative")
    print()
    print("  CLIP 의 batch 32K-65K 사용 (TPU 학습)")
    print("  negative 가 많을수록 'hard negative' 학습 강해짐")


def p3():
    print("\n정답: OpenCLIP")
    print("  OpenCLIP (LAION):")
    print("    - LAION-5B dataset (5억 image-text)")
    print("    - open weights")
    print("    - 다양한 ViT 크기 / patch size")
    print("    - SigLIP 의 사전 단계 (Google 이 OpenCLIP 형식에서 sigmoid 변경)")


if __name__ == "__main__":
    p1(); p2(); p3()
