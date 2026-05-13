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

    expected = {
        "A": (224 // 16) ** 2 + 1,  # 14*14 + 1 = 197
        "B": (384 // 16) ** 2 + 1,  # 24*24 + 1 = 577
        "C": (1024 // 16) ** 2 + 1,  # 64*64 + 1 = 4097
        "D": (224 // 14) ** 2 + 1,  # 16*16 + 1 = 257
    }

    print("  당신의 답:")
    for k, v in seq_lens.items():
        mark = "[O]" if v == expected[k] else "[X]"
        print(f"  {mark} {k}: {v}  (기대: {expected[k]})")
    print()
    print("  [tip] sequence 길이가 attention 계산량 (O(N^2)) 의 주요인.")
    print("       1024x1024 image 면 sequence 4097, attention 16M -> 매우 큼.")


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

    expected_qkv = 3 * N * D * D
    expected_attn = 2 * N * N * D
    expected_out = N * D * D
    expected_total = expected_qkv + expected_attn + expected_out

    print(f"  QKV proj : {qkv_flops:,}  (기대: {expected_qkv:,})")
    print(f"  Attention : {attn_flops:,}  (기대: {expected_attn:,})")
    print(f"  Out proj : {out_flops:,}  (기대: {expected_out:,})")
    print(f"  Total    : {qkv_flops + attn_flops + out_flops:,}  (기대: {expected_total:,})")
    print()
    print("  [tip] ViT-B 의 12 layer 전체 attention FLOPS:")
    print(f"    ~ 12 * {expected_total:,} = {12 * expected_total:,}")
    print(f"    ~ {12 * expected_total / 1e9:.2f} GFLOPS / forward pass")


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

    expected = {
        "A": "cnn",   # 작은 데이터 CNN 우위
        "B": "vit",   # 큰 데이터 ViT 우위
        "C": "vit",   # Modality 결합 자연스러움
        "D": "vit",   # 첫 layer 부터 global
        "E": "cnn",   # 작은 입력 conv 효율
        "F": "eq",    # 둘 다 가능 (ViT 도 patch token)
    }

    print("  당신의 답 vs 기대:")
    for k, v in cmp.items():
        mark = "[O]" if v == expected[k] else "[X]"
        print(f"    {mark} {k}: {v}  (기대: {expected[k]})")


if __name__ == "__main__":
    print("=" * 60)
    problem1_seq_len_calculation()
    problem2_compute_attention_flops()
    problem3_compare_cnn_vit()
    print("=" * 60)
