"""
Phase 4 Week 4 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: OpenVLA VRAM 추정")
    print("=" * 60 + "\n")

    n = 7e9
    ovh = 1.2

    print("  공식: VRAM = n_params * byte_per_param * overhead")
    print()
    print(f"  fp32 : 7B * 4   * 1.2 = {n*4*ovh/1e9:>5.2f} GB")
    print(f"  fp16 : 7B * 2   * 1.2 = {n*2*ovh/1e9:>5.2f} GB")
    print(f"  int8 : 7B * 1   * 1.2 = {n*1*ovh/1e9:>5.2f} GB")
    print(f"  int4 : 7B * 0.5 * 1.2 = {n*0.5*ovh/1e9:>5.2f} GB")
    print()
    print("  RTX 4070 (12 GB) 에 fit:")
    print(f"    fp32 ({n*4*ovh/1e9:.1f}GB) : 불가")
    print(f"    fp16 ({n*2*ovh/1e9:.1f}GB) : 불가 (간신히 안 됨)")
    print(f"    int8 ({n*1*ovh/1e9:.1f}GB) : 가능")
    print(f"    int4 ({n*0.5*ovh/1e9:.1f}GB) : 가능 (여유)")
    print()
    print("  본 로드맵 권장: int4 (bitsandbytes nf4)")
    print("    - 메모리 여유로 vision encoder + projector 도 함께 GPU 에")
    print("    - 정확도 손실 미미 (논문 ablation 기준 ~ 1~2%p)")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: LoRA 학습 가능 파라미터")
    print("=" * 60 + "\n")

    rank = 16
    in_size = 4096
    out_size = 4096
    n_layers = 388

    lora = rank * (in_size + out_size) * n_layers
    total = 7e9
    pct = lora / total * 100

    print(f"  한 LoRA layer: rank * (in + out)")
    print(f"               = 16 * (4096 + 4096) = {16 * (4096 + 4096):,}")
    print()
    print(f"  전체:  layer 당 * n_layers")
    print(f"      = {16 * (4096 + 4096):,} * {n_layers}")
    print(f"      = {lora:,}")
    print()
    print(f"  7B 모델 대비 비율: {pct:.2f}%")
    print()
    print("  [tip] 실제 OpenVLA + LoRA:")
    print("    - rank 16~32, alpha 32~64 사용")
    print("    - 학습 파라미터 ~ 50~100M")
    print("    - 7B 모델의 1% 미만으로 새 robot 적응 가능")
    print("    - GPU memory: int4 base + fp16 LoRA = ~ 6 GB")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: RT-2 vs OpenVLA 비교")
    print("=" * 60 + "\n")

    table = [
        ("LM size", "PaLI-X 5B / 55B", "Llama 2 7B"),
        ("Vision encoder", "ViT (PaLI-X 내부)", "DINOv2 + SigLIP hybrid"),
        ("Dataset (robot)", "RT-1 (130K, 1 embodiment)", "OpenX (970K, 22 embodiments)"),
        ("Open-source", "No (closed)", "Yes (weight + code + data)"),
        ("Training mode", "Co-fine-tune (web + robot)", "Robot-only fine-tune"),
        ("Latency", "~ 200ms", "~ 100~150ms"),
        ("VRAM (4-bit)", "~ 14 GB+ (55B)", "~ 4~5 GB"),
        ("LoRA", "공개되지 않음", "지원, 권장"),
    ]

    print(f"  {'항목':<18}{'RT-2':<32}{'OpenVLA'}")
    print(f"  {'-'*18}{'-'*32}{'-'*30}")
    for row in table:
        print(f"  {row[0]:<18}{row[1]:<32}{row[2]}")

    print("\n  [tip] 본 표를 week 7 (OpenVLA 블로그) 의 핵심 단락으로 인용.")


if __name__ == "__main__":
    print("=" * 60)
    print("  Phase 4 Week 4 Quiz - Medium 정답")
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "=" * 60)
