"""Phase 5 Week 10 - 기초 (SigLIP)"""


def p1():
    print("\n문제 1: SigLIP 의 CLIP 대비 핵심 차이")
    print("  A) 다른 architecture\n  B) Loss function (sigmoid vs softmax)\n  C) 더 큰 모델\n  D) 다른 학습 데이터")


def p2():
    print("\n문제 2: Sigmoid loss 의 장점")
    print("  A) Batch 의존성 적음 (작은 batch OK)")
    print("  B) Architecture 더 단순")
    print("  C) GPU 더 빠름")
    print("  D) Inference 더 빠름")


def p3():
    print("\n문제 3: OpenVLA 가 SigLIP 사용하는 이유")
    print("  A) DINOv2 보다 정확")
    print("  B) Semantic 정보 (어떤 객체)")
    print("  C) Faster inference")
    print("  D) 작은 모델")


def p4():
    print("\n문제 4: SigLIP probs 의 sum")
    print("  A) 항상 1 (softmax)")
    print("  B) sum 은 1 이 아닐 수 있음 (sigmoid 독립)")
    print("  C) 항상 0")
    print("  D) 무한")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
