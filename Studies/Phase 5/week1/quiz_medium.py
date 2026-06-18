"""
Phase 5 Week 1 - ViT 중급 퀴즈
"""


def problem1_seq_len_calculation():
    """
    문제 1: 다양한 image size + patch size 의 sequence 길이

    각 조합에서 sequence 길이를 계산 (CLS 포함).

      A) 224x224, patch 16
      B) 384x384, patch 16
      C) 1024x1024, patch 16
      D) 224x224, patch 14 (DINOv2 사용)

    TODO 값 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 1: Sequence 길이 계산")
    print("=" * 60 + "\n")

    # TODO
    seq_lens = {"A": 0, "B": 0, "C": 0, "D": 0}

    print("  당신의 답:")
    for k, v in seq_lens.items():
        print(f"  {k}: {v}")
    print()
    print("  [tip] sequence 길이가 attention 계산량 (O(N^2)) 의 주요인.")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem2_compute_attention_flops():
    """
    문제 2: Self-attention 의 FLOPS 추정

    ViT-B (D=768, H=12, N=197) 의 한 layer 의 self-attention 의 대략적 FLOPS:

    Self-attention FLOPS:
      - QKV projection: 3 * N * D * D
      - Attention: 2 * N^2 * D
      - Output projection: N * D * D

    TODO: 각 component + total 계산
    """
    print("\n" + "=" * 60)
    print("문제 2: Self-attention FLOPS 추정")
    print("=" * 60 + "\n")

    N = 197
    D = 768

    # TODO
    qkv_flops = 0
    attn_flops = 0
    out_flops = 0

    print(f"  QKV proj : {qkv_flops:,}")
    print(f"  Attention : {attn_flops:,}")
    print(f"  Out proj : {out_flops:,}")
    print(f"  Total    : {qkv_flops + attn_flops + out_flops:,}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem3_compare_cnn_vit():
    """
    문제 3: CNN vs ViT 비교

    아래 항목 별로 CNN 우위 / ViT 우위 / 동등 분류:

      A) 100K 이하 데이터로 학습
      B) 1M+ 데이터로 학습
      C) Vision-language modality 결합
      D) Receptive field 첫 layer 부터 global
      E) GPU 메모리 효율 (작은 입력)
      F) Spatial feature 추출 (segmentation 응용)

    TODO: 'cnn' / 'vit' / 'eq' 로 채우시오.
    """
    print("\n" + "=" * 60)
    print("문제 3: CNN vs ViT 비교")
    print("=" * 60 + "\n")

    # TODO
    cmp = {"A": "", "B": "", "C": "", "D": "", "E": "", "F": ""}

    print("  당신의 답:")
    for k, v in cmp.items():
        print(f"    {k}: {v}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    problem1_seq_len_calculation()
    problem2_compute_attention_flops()
    problem3_compare_cnn_vit()
    print("=" * 60)
