"""
Phase 5 Week 2 - ViT inference 중급 퀴즈
"""


def problem1():
    """
    문제 1: ViT-B/L 의 throughput 계산
    Mean latency: ViT-B 10ms, ViT-L 30ms
    Throughput (Hz)?
    """
    print("\n문제 1: throughput 계산\n")

    # TODO
    thr_b = 0.0
    thr_l = 0.0

    expected = (100.0, 33.3)
    print(f"  ViT-B: {thr_b} Hz (기대 {expected[0]})")
    print(f"  ViT-L: {thr_l} Hz (기대 {expected[1]})")


def problem2():
    """
    문제 2: VRAM 추정
    ViT-L: 300M params, fp16, batch=1
    Params memory + activation = ?
    """
    print("\n문제 2: ViT-L VRAM 추정 (fp16)\n")

    n_params = 300e6
    bytes_per_param = 2  # fp16
    activation_factor = 1.5  # activation 추가 약 50%

    # TODO
    vram_gb = 0.0

    expected = (n_params * bytes_per_param / 1e9) * activation_factor
    print(f"  당신: {vram_gb:.2f} GB")
    print(f"  기대: {expected:.2f} GB")


def problem3():
    """
    문제 3: ViT 의 input resolution scale 영향
    224x224 -> 384x384 image 변경 시 seq_len + FLOPS 변화?
    """
    print("\n문제 3: input resolution scale\n")

    # 224x224 / 16 -> 14*14 + 1 = 197
    # 384x384 / 16 -> 24*24 + 1 = 577

    seq_224 = 197
    seq_384 = 577

    # TODO: FLOPS 비율
    attn_flops_ratio = 0.0  # (577^2) / (197^2)

    expected = (577 ** 2) / (197 ** 2)
    print(f"  Seq len  : 197 -> 577 ({577/197:.2f}x)")
    print(f"  Attn FLOPS ratio: {attn_flops_ratio:.2f}  (기대: {expected:.2f}x)")
    print(f"  -> attention 이 ~ 8.6x 늘어남, latency 도 비슷한 비율")


if __name__ == "__main__":
    problem1(); problem2(); problem3()
