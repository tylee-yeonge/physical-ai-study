"""Phase 5 Week 10 - 중급 (SigLIP)"""


def p1():
    """Sigmoid loss formula"""
    print("\n문제 1: Sigmoid pairwise loss")
    print("  For pair (i, j):")
    print("    target = 1 if i==j else 0")
    print("    loss = BCE(sigmoid(sim_ij), target)")
    print()
    print("  CLIP InfoNCE softmax:")
    print("    loss = -log(exp(sim_ii) / sum_k exp(sim_ik))")
    print()
    print("  핵심 차이:")
    print("  A) Sigmoid loss 가 batch 의 다른 sample 에 의존")
    print("  B) Sigmoid loss 가 각 pair 독립, softmax 가 batch 통합")
    print("  C) 차이 없음")
    # TODO
    ans = ""
    expected = "B"
    print(f"  당신: {ans} (기대: {expected})")


def p2():
    """SigLIP vs CLIP 정확도"""
    print("\n문제 2: SigLIP vs CLIP")
    print("  ImageNet zero-shot:")
    print("    CLIP-L/14    : 75.5%")
    print("    SigLIP-L/16  : 76.0%")
    print()
    # TODO: 정확도 차이
    diff = 0.0
    expected = 0.5
    print(f"  당신: {diff}% (기대 ~ {expected})")
    print()
    print("  [tip] 정확도 차이는 작지만 (~0.5%p):")
    print("    - SigLIP 의 학습 효율이 훨씬 좋음")
    print("    - 같은 compute 로 더 큰 모델 가능")
    print("    - OpenVLA 가 SigLIP 선택 이유")


def p3():
    """SigLIP-SO400M 의 의미"""
    print("\n문제 3: SigLIP-SO400M")
    print("  Shape-Optimized SigLIP, ~400M params")
    print("  OpenVLA-7B-SigLIP 의 vision encoder 옵션")
    print()
    # TODO: latency 추정
    lat = 0
    expected = 40
    print(f"  RTX 4070 fp16 latency 추정: {lat} ms (기대 ~ {expected})")


if __name__ == "__main__":
    p1(); p2(); p3()
