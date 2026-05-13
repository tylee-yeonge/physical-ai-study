"""중급"""


def p1():
    print("\n문제 1: Sim/Real success rate 비교")
    print("  Sim: ~ 80%")
    print("  Real: ~ 65%")
    print("  Gap: ~ 15%")
    print("  Domain Randomization 으로 줄임 (Phase 6 week 11)")


def p2():
    print("\n문제 2: 영상의 가장 중요한 segment")
    print("  '같은 instruction 으로 Sim 과 Real 이 동시에 동작'")
    print("  -> 면접관에게 가장 강력한 증거 (Real-to-Sim-to-Real)")


def p3():
    print("\n문제 3: Trial 의 다양성")
    print("  3 trials 권장:")
    print("    1. Standard (red cup)")
    print("    2. 다른 색상 (blue object)")
    print("    3. Cluttered (여러 물체)")
    print("  -> 다양한 generalization 보임")


if __name__ == "__main__":
    p1(); p2(); p3()
