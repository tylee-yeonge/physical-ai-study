"""Phase 5 Week 8 - 기초 정답"""


def p1():
    print("\n정답: B) Spatial cluster")
    print("  DINOv2 patch feature 의 PCA 시각화:")
    print("    - 같은 spatial region 의 patch 가 같은 색")
    print("    - object boundary 명확")
    print("    - segmentation 비슷한 효과")


def p2():
    print("\n정답: B) ~ 28 ms")
    print("  ViT-L (300M), fp16, RTX 4070: ~ 28 ms")
    print("  ViT-B (86M): ~ 12 ms")
    print("  ViT-G (1.1B): ~ 100 ms (또는 OOM)")


def p3():
    print("\n정답: C) general feature -> 다양한 응용")
    print("  DINOv2 patch feature 1024 dim 은 general:")
    print("    - downstream task 별 작은 head 추가")
    print("    - LoRA fine-tune 가능")
    print("    - SAM / DPT / DINOv2-segment 등 다양한 응용 모델")


def p4():
    print("\n정답: C) DINOv2 general, SAM segmentation 특화")
    print("  DINOv2: general feature (1024 dim per patch)")
    print("  SAM   : segmentation mask 출력 (label rich 학습)")
    print()
    print("  OpenVLA 는 DINOv2 선택:")
    print("    - general feature 가 LLM 과 결합 자연스러움")
    print("    - SAM 은 segmentation 만, LLM 결합 어려움")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
