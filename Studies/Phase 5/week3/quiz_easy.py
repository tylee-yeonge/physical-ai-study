"""Phase 5 Week 3 - 기초 퀴즈"""


def p1():
    print("\n문제 1: OpenVLA vision encoder 총 params")
    print("  A) ~ 100M\n  B) ~ 300M\n  C) ~ 600M (ViT-L * 2)\n  D) ~ 1B")


def p2():
    print("\n문제 2: OpenVLA 의 가장 큰 latency component")
    print("  A) Image preprocess\n  B) DINOv2 forward\n  C) LM decoder generate\n  D) Action de-tokenize")


def p3():
    print("\n문제 3: 두 vision encoder fusion 방식")
    print("  A) 가중 평균\n  B) Sequence concat + MLP projector\n  C) Skip-connection\n  D) Cross-attention")


def p4():
    print("\n문제 4: ViT 지식과 OpenVLA")
    print("  A) ViT 와 OpenVLA 는 무관")
    print("  B) OpenVLA vision = ViT-L 의 응용 (patch + attention + pos)")
    print("  C) OpenVLA 는 CNN 기반")
    print("  D) ViT 는 OpenVLA 보다 큰 모델")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
