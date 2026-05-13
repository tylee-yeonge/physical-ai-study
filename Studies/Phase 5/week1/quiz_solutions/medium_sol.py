"""
Phase 5 Week 1 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: Sequence 길이")
    print("=" * 60 + "\n")
    cases = [
        ("A", 224, 16, (224 // 16) ** 2 + 1),
        ("B", 384, 16, (384 // 16) ** 2 + 1),
        ("C", 1024, 16, (1024 // 16) ** 2 + 1),
        ("D", 224, 14, (224 // 14) ** 2 + 1),
    ]
    print(f"  {'Case':<8}{'image':<10}{'patch':<8}{'seq_len'}")
    print(f"  {'-'*8}{'-'*10}{'-'*8}{'-'*10}")
    for code, img, p, sl in cases:
        print(f"  {code:<8}{img}x{img}{'':<2}{p:<8}{sl}")
    print()
    print("  [tip] sequence 길이의 영향:")
    print("    A) 197 (표준 ViT)")
    print("    B) 577 (high resolution, OpenVLA 권장 384)")
    print("    C) 4097 (segmentation 등 dense task, FLOPS 폭증)")
    print("    D) 257 (DINOv2 의 patch 14 사용)")
    print()
    print("  OpenVLA 의 DINOv2 + SigLIP 은 384x384 / patch 14 사용 -> seq ~ 730")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: Self-attention FLOPS")
    print("=" * 60 + "\n")
    N = 197
    D = 768

    qkv = 3 * N * D * D
    attn = 2 * N * N * D
    out = N * D * D
    total = qkv + attn + out

    print(f"  N={N}, D={D}")
    print()
    print(f"  QKV projection : 3 * N * D * D = 3 * {N} * {D} * {D}")
    print(f"                 = {qkv:,}")
    print(f"  Attention      : 2 * N^2 * D = 2 * {N**2} * {D}")
    print(f"                 = {attn:,}")
    print(f"  Output proj    : N * D * D = {N} * {D} * {D}")
    print(f"                 = {out:,}")
    print(f"  ---")
    print(f"  Total          : {total:,} ≈ {total/1e9:.3f} GFLOPS")
    print()
    print(f"  ViT-B 12 layer  : {12*total:,} ≈ {12*total/1e9:.2f} GFLOPS")
    print()
    print("  [tip] FLOPS 분석:")
    print("    QKV (3*N*D*D) 가 가장 큰 항. N=197, D=768.")
    print("    attention (2*N^2*D) 는 N 작을 때 상대적으로 작음.")
    print()
    print("    sequence 가 길어지면 (예: N=4097) attention 이 dominant 됨.")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: CNN vs ViT")
    print("=" * 60 + "\n")
    items = [
        ("A", "100K 이하 데이터", "cnn", "Inductive bias 덕분에 CNN 우위"),
        ("B", "1M+ 데이터", "vit", "scale 의 ViT 우위"),
        ("C", "Vision-language modality", "vit", "ViT 의 token sequence 가 text token 과 결합 자연스러움"),
        ("D", "첫 layer global RF", "vit", "Self-attention 이 모든 token 쌍 관계"),
        ("E", "작은 입력 메모리", "cnn", "Conv 가 작은 sequence 보다 효율"),
        ("F", "Spatial feature (segmentation)", "eq", "ViT 도 patch token 으로 가능, 둘 다 OK"),
    ]
    print(f"  {'Case':<6}{'항목':<28}{'우위':<6}{'이유'}")
    print(f"  {'-'*6}{'-'*28}{'-'*6}{'-'*45}")
    for code, name, who, why in items:
        print(f"  {code:<6}{name:<28}{who:<6}{why}")
    print()
    print("  [tip] 분야 별 선택 가이드:")
    print("    - 자율주행 (실시간 + 작은 데이터셋): CNN 또는 hybrid")
    print("    - LLM 통합 / VLA: ViT")
    print("    - Segmentation / Detection: hybrid (DETR, MaskFormer)")
    print("    - Industrial defect (작은 데이터): CNN (ResNet)")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
