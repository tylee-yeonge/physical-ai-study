"""Phase 5 Week 9 - 기초 (OpenVLA backbone 종합)"""


def p1():
    print("\n문제 1: DINOv2 + SigLIP fusion 방식")
    print("  A) 가중 평균")
    print("  B) Sequence concat -> projector")
    print("  C) Cross-attention")
    print("  D) Element-wise add")


def p2():
    print("\n문제 2: Projector 의 역할")
    print("  A) Vision -> action 직접 변환")
    print("  B) Vision dim 1024 -> Llama dim 4096 변환")
    print("  C) Image -> token 변환")
    print("  D) Token -> text 변환")


def p3():
    print("\n문제 3: LoRA fine-tune 시 학습되는 부분")
    print("  A) DINOv2 + SigLIP")
    print("  B) Llama 만")
    print("  C) Projector + Llama (LoRA), vision encoder frozen")
    print("  D) 모든 부분")


def p4():
    print("\n문제 4: 224 vs 384 image input")
    print("  A) 둘 다 같은 속도")
    print("  B) 224 가 빠름, 384 가 fine task 정확")
    print("  C) 384 가 항상 우위")
    print("  D) 224 만 가능")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
