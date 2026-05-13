"""
Phase 5 Week 1 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) 196 patches (= 14*14)")
    print("=" * 50 + "\n")
    print("해설:")
    print("  224 / 16 = 14 (한 변의 patch 개수)")
    print("  14 * 14 = 196 (총 patch 개수)")
    print("  +1 (CLS token) = 197 (sequence 길이)")
    print()
    print("  C 답 (197) 은 patch 가 아닌 sequence 길이.")
    print("  D 답 (768) 은 patch 한 개의 dim (16*16*3 = 768).")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) 16x16 patch + Linear + pos + CLS")
    print("=" * 50 + "\n")
    print("해설:")
    print("  ViT 의 Image -> token 변환 흐름:")
    print("    1. 16x16 patch 분할 (224x224 -> 14x14=196 patch)")
    print("    2. Linear projection (16*16*3 = 768 -> embed_dim D)")
    print("    3. Position embedding 더하기")
    print("    4. [CLS] token prepend")
    print()
    print("  Conv2d 로 구현 시:")
    print("    nn.Conv2d(3, 768, kernel_size=16, stride=16)")
    print("    -> patch + linear projection 한 번에")
    print()
    print("  픽셀 그대로 sequence (C) 는 너무 길음 (224^2=50000).")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: C) O(N^2)")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Self-attention 의 계산:")
    print("    Attention(Q, K, V) = softmax(QK^T / sqrt(d)) V")
    print("    Q, K, V 는 모두 (N, D)")
    print("    QK^T = (N, D) * (D, N) = (N, N) -> 메모리 O(N^2)")
    print()
    print("  Token 수 N 이 polynomial 의 핵심:")
    print("    - ViT-B 224x224 : N=197, N^2=38809")
    print("    - 1024x1024     : N=4097, N^2=16M (1000x 늘어남)")
    print()
    print("  이 한계를 극복하는 방법들:")
    print("    - Sparse attention")
    print("    - Linear attention (Performer 등)")
    print("    - Sliding window (Swin Transformer)")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: C) 약 300M")
    print("=" * 50 + "\n")
    print("해설:")
    print("  ViT variant 의 parameter 수:")
    print("    ViT-Ti (Tiny) :  5.7M")
    print("    ViT-S (Small) :  22M  <- A 답")
    print("    ViT-B (Base)  :  86M  <- B 답")
    print("    ViT-L (Large) : 307M  <- 정답 (OpenVLA backbone)")
    print("    ViT-H (Huge)  : 632M")
    print("    ViT-22B       :  22B  (PaLI-X 의 일부)")
    print()
    print("  OpenVLA 의 vision encoder:")
    print("    DINOv2 ViT-L : ~ 300M")
    print("    SigLIP ViT-L : ~ 300M")
    print("    Total vision : ~ 600M")
    print()
    print("  + Llama 2 7B = total ~ 7.6B")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
