"""Phase 5 Week 4 - CLIP 중급 퀴즈"""


def p1():
    """InfoNCE loss 계산
    batch=4, sim_matrix 4x4 의 diagonal=correct
    """
    print("\n문제 1: InfoNCE loss 직관")
    print("  sim_matrix (B=4, image_i vs text_j):")
    print("    [3.0, 0.5, 0.3, 0.2]   <- image 0, correct=3.0")
    print("    [0.4, 3.5, 0.6, 0.1]   <- image 1, correct=3.5")
    print("    [0.2, 0.5, 3.2, 0.3]   <- image 2, correct=3.2")
    print("    [0.3, 0.4, 0.5, 3.0]   <- image 3, correct=3.0")
    print()
    print("  Loss = -mean log( exp(diag) / sum_row exp )")
    print("  각 row 마다 softmax 후 diag 의 log prob 평균.")
    print()
    print("  -> 학습 진행 시 loss 가 0 으로 수렴 (perfect alignment)")


def p2():
    """batch size 의 영향"""
    print("\n문제 2: batch size 의 영향")
    print("  small batch (32): negative example 적음 -> 학습 신호 약함")
    print("  large batch (4096+): negative 많음 -> 강한 학습")
    print()
    # TODO
    n_neg_32 = 0  # batch - 1
    n_neg_4096 = 0  # batch - 1
    print(f"  배치 32 의 negative: {n_neg_32}")
    print(f"  배치 4096 의 negative: {n_neg_4096}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """OpenAI CLIP vs OpenCLIP"""
    print("\n문제 3: CLIP 의 후속 (OpenCLIP 등)")
    print("  OpenAI CLIP: closed weights")
    print("  OpenCLIP: open implementation + LAION 데이터 학습")
    print("  -> OpenCLIP ViT-L 가 OpenVLA 의 SigLIP 의 사전 단계")


if __name__ == "__main__":
    p1(); p2(); p3()
