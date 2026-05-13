"""Phase 5 Week 5 - 기초 퀴즈"""


def p1():
    print("\n문제 1: CLIP 표준 prompt 패턴")
    print("  A) 'cat' (단어만)")
    print("  B) 'a photo of a cat' (표준)")
    print("  C) 'cat photograph high-quality professional'")
    print("  D) 분류 클래스 이름 그대로")


def p2():
    print("\n문제 2: CLIP zero-shot 일반 객체 정확도")
    print("  A) ~ 95%\n  B) 70~80%\n  C) 50~60%\n  D) 20~30%")


def p3():
    print("\n문제 3: Open-vocab detection 의 CLIP 단독 가능 여부")
    print("  A) 가능 (CLIP 만으로)")
    print("  B) 불가능 (별도 detection 모델 필요)")
    print("  C) 일부만 가능")
    print("  D) detection 은 CLIP 의 주 용도")


def p4():
    print("\n문제 4: 자작 팔 환경에서 CLIP 의 zero-shot 정확도 예상")
    print("  A) 일반 객체 (컵, 마우스): 80%+")
    print("  B) 모든 객체: 95%+")
    print("  C) 모든 객체: < 30%")
    print("  D) 자작 팔 자체: 95%+")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
