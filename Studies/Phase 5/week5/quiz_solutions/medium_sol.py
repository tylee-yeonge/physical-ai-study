"""Phase 5 Week 5 - 중급 정답"""


def p1():
    print("\n정답: B")
    print("  prompt A 'cat' -> 60%: 단어만, 학습 분포 불일치")
    print("  prompt B 'a photo of a cat' -> 75%: 표준, 최선")
    print("  prompt C 'HD photo of a small cat sitting' -> 70%: 너무 specific")
    print()
    print("  [tip] 'a photo of a X' 가 sweet spot")


def p2():
    print("\n정답: C) 70% 이하")
    print("  Robot 성공률 = P(인식 성공) * P(action 성공 | 인식 성공)")
    print("  = 0.70 * P(action)")
    print()
    print("  P(action) 도 보통 70~80% 정도라:")
    print("  Total ~ 49~56% (zero-shot OpenVLA)")
    print()
    print("  LoRA fine-tune 시 P(action) ↑ -> total 70%+")


def p3():
    print("\n정답: prompt 예시")
    print("  표준 패턴:")
    print("    'a photo of a {object}' (가장 표준)")
    print("    'a photo of a {color} {object}'")
    print("    'a photo of a {object} on a desk' (context 추가)")
    print()
    print("  자작 팔 환경 권장:")
    print("    'a photo of a red cup'")
    print("    'a photo of a blue computer mouse'")
    print("    'a photo of a keyboard on a desk'")
    print("    'a photo of a robot arm'")
    print("    'a photo of a desktop computer'")


if __name__ == "__main__":
    p1(); p2(); p3()
