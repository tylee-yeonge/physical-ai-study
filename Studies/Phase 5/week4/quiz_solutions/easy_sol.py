"""Phase 5 Week 4 - 기초 퀴즈 정답"""


def p1():
    print("\n정답: B) Image + text 공통 vector space (contrastive)")
    print("  핵심: image_vec 와 text_vec 가 같은 의미면 가까이")
    print("  -> InfoNCE loss")


def p2():
    print("\n정답: C) WIT 400M")
    print("  WIT = OpenAI 의 Web Image-Text dataset")
    print("  alt-text 자동 수집")
    print("  ImageNet 보다 ~ 300x 큼")


def p3():
    print("\n정답: B) 임의 prompt 로 zero-shot")
    print("  기존 분류 모델: 학습된 1000 classes 만")
    print("  CLIP: 임의 텍스트 'a photo of <X>' 만 만들면 분류 가능")
    print("  -> 새 class 추가 시 재학습 불필요")


def p4():
    print("\n정답: C) ~ 428M")
    print("  ViT-L (300M) + text Transformer (~128M) = 428M")
    print("  OpenAI CLIP 의 최대 모델 (ViT-L/14)")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
