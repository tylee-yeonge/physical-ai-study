"""기초"""


def p1():
    print("\n문제 1: 파이프라인 순서")
    print("  A) vla -> motor 직접\n  B) vla -> ik -> safety -> motor\n  C) motor -> vla\n  D) safety only")


def p2():
    print("\n문제 2: Total latency 추정")
    print("  A) ~ 50 ms\n  B) ~ 180 ms\n  C) ~ 1 s\n  D) ~ 10 s")


def p3():
    print("\n문제 3: 첫 Real 실행 시 주의")
    print("  A) 빠르게\n  B) low speed + e-stop 위치\n  C) 무인\n  D) 야간")


def p4():
    print("\n문제 4: 본 phase 의 양산 SW 차별화 증거")
    print("  A) 학습 시간\n  B) VLA latency + 안전 메커니즘\n  C) 모델 크기\n  D) 데이터 수")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
