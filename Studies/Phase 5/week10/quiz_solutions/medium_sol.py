"""Phase 5 Week 10 - 중급 정답"""


def p1():
    print("\n정답: B) sigmoid 독립, softmax batch 통합")
    print()
    print("  Sigmoid pairwise:")
    print("    각 (i, j) 의 sim 을 binary classification")
    print("    -> 다른 pair 와 무관 -> batch 독립")
    print()
    print("  Softmax InfoNCE:")
    print("    한 row 의 모든 (i, k) 가 softmax denominator 공유")
    print("    -> batch 안의 negative 가 많을수록 hard")


def p2():
    diff = 76.0 - 75.5
    print(f"\n정답: 약 {diff}%p 차이")
    print("  정확도 차이는 미미 (~0.5%p)")
    print("  하지만 효율 차이는 큼:")
    print("    - 같은 정확도를 더 적은 compute 로")
    print("    - 또는 같은 compute 로 더 큰 모델 학습")


def p3():
    print("\n정답: ~40 ms")
    print()
    print("  SigLIP-SO400M 추정 (RTX 4070 fp16):")
    print("    ~ 400M params -> 1.5x ViT-L (300M)")
    print("    ViT-L latency ~ 28 ms")
    print("    SO400M latency ~ 40 ms")
    print()
    print("  OpenVLA-7B-SigLIP 의 옵션 (보다 큰 vision):")
    print("    - 정확도 향상 (~ 2~3%p)")
    print("    - latency 증가 (28 -> 40 ms)")
    print("    - OpenVLA 표준 base 는 SigLIP-L 사용")


if __name__ == "__main__":
    p1(); p2(); p3()
