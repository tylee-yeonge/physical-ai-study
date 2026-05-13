"""Phase 5 Week 10 - 기초 정답"""


def p1():
    print("\n정답: B) Loss function 차이")
    print("  Architecture 는 거의 동일 (ViT + Text Transformer)")
    print("  Loss 가 핵심:")
    print("    CLIP: softmax InfoNCE")
    print("    SigLIP: sigmoid pairwise BCE")


def p2():
    print("\n정답: A) batch 의존성 적음")
    print("  Sigmoid pairwise 의 효과:")
    print("    - 각 pair (i, j) 가 독립")
    print("    - batch 안의 다른 sample 영향 없음")
    print("    - small batch (1K) 에서도 학습 가능")
    print()
    print("  CLIP 의 softmax 는 negative 가 많을수록 강함 (4K+ 권장)")


def p3():
    print("\n정답: B) Semantic 정보")
    print("  SigLIP image-text contrastive -> semantic 강함")
    print("  DINOv2 self-supervised -> spatial 강함")
    print("  두 정보 hybrid 가 OpenVLA backbone")


def p4():
    print("\n정답: B) sum 1 이 아닐 수 있음")
    print("  Sigmoid: 각 logit 을 [0,1] 로 변환 (independent)")
    print("  Softmax: 전체 sum=1 (probability distribution)")
    print()
    print("  SigLIP 의 의미:")
    print("    'image i 가 text j 와 match 할 확률' (다른 pair 와 무관)")
    print("    -> binary classification 처럼 사용")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
