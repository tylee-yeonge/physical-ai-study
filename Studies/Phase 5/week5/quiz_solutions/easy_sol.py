"""Phase 5 Week 5 - 기초 정답"""


def p1():
    print("\n정답: B) 'a photo of a cat'")
    print("  CLIP 학습 데이터 (WIT alt-text) 의 caption 분포에 가까움")


def p2():
    print("\n정답: B) 70~80%")
    print("  ImageNet zero-shot: ~76% (CLIP-L/14)")
    print("  Fine-grained 약함 (40-50%)")


def p3():
    print("\n정답: B) 불가능")
    print("  CLIP 은 image-level classification 만")
    print("  Detection 은 GroundingDINO / OWL-ViT 등 CLIP feature 활용 모델 필요")


def p4():
    print("\n정답: A) 일반 객체 80%+")
    print("  컵 / 마우스 / 키보드 등 학습 데이터에 많은 객체 80%+")
    print("  자작 팔 자체는 unusual 형태라 confidence 낮을 수 있음")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
