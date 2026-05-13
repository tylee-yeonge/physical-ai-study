"""
Phase 4 Week 4 - OpenVLA 1회독 + Architecture 중급 퀴즈
"""


def problem1_vram_calculation():
    """
    문제 1: OpenVLA 의 VRAM 요구량 계산

    OpenVLA 7B 모델의 VRAM 을 추정하시오.
    가정:
      - 7B parameter
      - dtype 별 byte: fp32=4, fp16=2, int8=1, int4=0.5
      - inference 시 KV cache 등 overhead 약 20% 추가

    각 dtype 별 VRAM 을 계산하고, RTX 4070 12GB 에 fit 가능한 dtype 을 찾으시오.

    TODO: vram_fp16, vram_int8, vram_int4 를 계산하고 fits_on_4070 채우시오.
    """
    print("\n" + "=" * 60)
    print("문제 1: OpenVLA VRAM 추정")
    print("=" * 60 + "\n")

    n_params = 7e9  # 7B
    overhead = 1.2  # 20% overhead

    # TODO: 각 dtype 의 VRAM 계산 (GB 단위)
    vram_fp32 = 0.0  # 7B * 4 byte * 1.2 / 1e9
    vram_fp16 = 0.0  # 7B * 2 byte * 1.2 / 1e9
    vram_int8 = 0.0  # 7B * 1 byte * 1.2 / 1e9
    vram_int4 = 0.0  # 7B * 0.5 byte * 1.2 / 1e9

    # TODO: 4070 (12GB) 에 fit 가능한 dtype 만 True
    fits_on_4070 = {
        "fp32": False,
        "fp16": False,
        "int8": False,
        "int4": False,
    }

    expected_fp32 = (n_params * 4 * overhead) / 1e9
    expected_fp16 = (n_params * 2 * overhead) / 1e9
    expected_int8 = (n_params * 1 * overhead) / 1e9
    expected_int4 = (n_params * 0.5 * overhead) / 1e9

    print(f"  당신의 답:")
    print(f"    fp32 : {vram_fp32:.2f} GB (기대: {expected_fp32:.2f})")
    print(f"    fp16 : {vram_fp16:.2f} GB (기대: {expected_fp16:.2f})")
    print(f"    int8 : {vram_int8:.2f} GB (기대: {expected_int8:.2f})")
    print(f"    int4 : {vram_int4:.2f} GB (기대: {expected_int4:.2f})")
    print()
    print(f"  RTX 4070 (12GB) 에 fit 가능?")
    print(f"    {fits_on_4070}")
    print(f"    기대: fp32=False, fp16=False, int8=True, int4=True")


def problem2_lora_params():
    """
    문제 2: LoRA 의 학습 가능 파라미터 수 계산

    Llama 2 7B 모델에 LoRA 를 적용. 가정:
      - 모든 Linear layer 에 LoRA 적용 (Wq, Wk, Wv, Wo, Wgate, Wup, Wdown)
      - 7B 모델의 Linear layer 의 총 파라미터 수 ~ 6.5B (대략)
      - LoRA rank r = 16
      - LoRA alpha = 32
      - Linear layer 의 in/out 평균 = 4096

    LoRA 학습 가능 파라미터 수를 계산하시오.
    LoRA 한 layer 의 학습 파라미터 수: r * (in + out) = 16 * 8192 = 131072
    layer 개수 약 ~ 6.5B / (4096 * 4096) = 약 388 개 Linear layer 가정

    TODO: lora_params 계산.
    """
    print("\n" + "=" * 60)
    print("문제 2: LoRA 학습 가능 파라미터 수")
    print("=" * 60 + "\n")

    n_linear_layers = 388
    in_size = 4096
    out_size = 4096
    rank = 16

    # TODO
    lora_params = 0  # rank * (in + out) * n_linear_layers

    expected = rank * (in_size + out_size) * n_linear_layers
    total = 7e9

    print(f"  Linear layer 수    : {n_linear_layers}")
    print(f"  Layer 당 in/out    : {in_size}/{out_size}")
    print(f"  LoRA rank          : {rank}")
    print()
    print(f"  당신의 답          : {lora_params:,}")
    print(f"  기대값             : {expected:,}")
    print()
    print(f"  전체 7B 모델 대비  : {lora_params/total*100:.2f}%")
    print(f"  기대 비율          : {expected/total*100:.2f}%")
    print()
    print("  -> LoRA 는 전체 파라미터의 약 1% 만 학습.")
    print("     이게 빠른 fine-tuning 의 핵심.")


def problem3_compare_table():
    """
    문제 3: RT-2 vs OpenVLA 비교 표 채우기

    아래 표의 빈칸을 채우시오 (week 4 README 7장 참고).

    | 항목 | RT-2 | OpenVLA |
    |---|---|---|
    | LM size | A1 | A2 |
    | Vision encoder | B1 | B2 |
    | Dataset (robot) | C1 | C2 |
    | Open-source | D1 | D2 |
    """
    print("\n" + "=" * 60)
    print("문제 3: RT-2 vs OpenVLA 비교 표")
    print("=" * 60 + "\n")

    # TODO
    answers = {
        "A1 (RT-2 LM size)": "",
        "A2 (OpenVLA LM size)": "",
        "B1 (RT-2 vision)": "",
        "B2 (OpenVLA vision)": "",
        "C1 (RT-2 robot data)": "",
        "C2 (OpenVLA robot data)": "",
        "D1 (RT-2 open)": "",
        "D2 (OpenVLA open)": "",
    }

    expected = {
        "A1 (RT-2 LM size)": "PaLI-X 5B / 55B",
        "A2 (OpenVLA LM size)": "Llama 2 7B",
        "B1 (RT-2 vision)": "ViT (PaLI-X 내부)",
        "B2 (OpenVLA vision)": "DINOv2 + SigLIP hybrid",
        "C1 (RT-2 robot data)": "RT-1 dataset (130K)",
        "C2 (OpenVLA robot data)": "OpenX-Embodiment (970K)",
        "D1 (RT-2 open)": "No (closed)",
        "D2 (OpenVLA open)": "Yes (weight+code+data)",
    }

    print("  당신의 답 vs 기대:")
    correct = 0
    for k, v in answers.items():
        match = v.strip().lower() == expected[k].strip().lower()
        if match:
            correct += 1
        mark = "[O]" if match else "[X]"
        print(f"  {mark} {k}")
        print(f"      당신: '{v}'")
        print(f"      기대: '{expected[k]}'")

    print(f"\n  점수: {correct}/{len(answers)}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    print("  Phase 4 Week 4 Quiz - Medium")
    print("=" * 60)
    problem1_vram_calculation()
    problem2_lora_params()
    problem3_compare_table()
    print("\n" + "=" * 60)
