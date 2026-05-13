"""Phase 7 Week 2 - 중급"""


def p1():
    print("\n문제 1: IK 라이브러리 선택")
    print("  옵션: PyKDL, ikpy, MoveIt, ikfast")
    print("  본 phase 권장: ikpy (Python, 가볍고 자작 팔 호환)")


def p2():
    print("\n문제 2: LoRA fine-tune 후 latency 변화")
    print("  zero-shot: 165 ms")
    print("  fine-tuned: 165 ms (LoRA 가 weight 만 추가, latency 영향 적음)")


def p3():
    print("\n문제 3: success rate 측정 방법")
    print("  recorded test data (validation set):")
    print("    - episode 의 final ee 가 target 의 5cm 이내 -> success")
    print("    - 50 trial * 3 task 평가")
    print("    - 목표 > 70%")


if __name__ == "__main__":
    p1(); p2(); p3()
