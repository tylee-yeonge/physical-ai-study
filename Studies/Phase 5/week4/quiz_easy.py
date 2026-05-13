"""Phase 5 Week 4 - CLIP 기초 퀴즈"""


def p1():
    print("\n문제 1: CLIP 의 핵심 아이디어")
    print("  A) Image classifier 의 새 architecture")
    print("  B) Image + text 를 공통 vector space 로 매핑 (contrastive)")
    print("  C) Text generator")
    print("  D) Object detector")


def p2():
    print("\n문제 2: CLIP 학습 데이터")
    print("  A) ImageNet 1.28M")
    print("  B) ImageNet-21k 14M")
    print("  C) WIT (Web Image-Text) 400M")
    print("  D) MS COCO 100K")


def p3():
    print("\n문제 3: CLIP 의 Open-vocabulary 의미")
    print("  A) 1000 classes 이상 학습")
    print("  B) 임의 텍스트 prompt 로 zero-shot 분류 가능")
    print("  C) Multi-lingual 지원")
    print("  D) 새 class 학습 시 재학습 필요")


def p4():
    print("\n문제 4: CLIP-L/14 의 parameter")
    print("  A) ~ 150M\n  B) ~ 300M\n  C) ~ 428M\n  D) ~ 1B")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
