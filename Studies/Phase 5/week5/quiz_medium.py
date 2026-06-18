"""Phase 5 Week 5 - 중급 퀴즈"""


def p1():
    """Prompt engineering 의 정량 효과"""
    print("\n문제 1: prompt 변화의 정확도 영향")
    print("  prompt A: 'cat' -> 정확도 약 60%")
    print("  prompt B: 'a photo of a cat' -> 정확도 약 75%")
    print("  prompt C: 'an HD photo of a small cat sitting' -> 약 70%")
    print()
    # TODO: 가장 좋은 prompt
    best = ""
    print(f"  당신의 답: {best}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """OpenVLA inference 시 image 의 classification 정확도와 robot 성공률 관계"""
    print("\n문제 2: zero-shot 인식 - robot 성공률 관계")
    print("  CLIP zero-shot 의 객체 인식 70%")
    print("  -> 'pick up the red can' 명령의 robot 성공률은?")
    print()
    print("  A) 70% (그대로)")
    print("  B) 70% 이상 (action 만 잘 되면 인식 충분)")
    print("  C) 70% 이하 (인식 + action 모두 통과해야 성공)")
    print("  D) 관계 없음")
    print()
    # TODO
    answer = ""
    print(f"  당신의 답: {answer}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """자작 팔 환경 prompt 설계"""
    print("\n문제 3: 자작 팔 환경 zero-shot 의 표준 prompt 설계")
    print("  자작 팔 환경 객체: red cup, blue mouse, keyboard, monitor, ...")
    print()
    # TODO: 좋은 prompt 5개 설계
    prompts = []

    examples = [
        "a photo of a red cup",
        "a photo of a blue computer mouse",
        "a photo of a keyboard on a desk",
        "a photo of a robot arm",
        "a photo of a desk with objects",
    ]
    print("  예시:")
    for p in examples:
        print(f"    '{p}'")


if __name__ == "__main__":
    p1(); p2(); p3()
