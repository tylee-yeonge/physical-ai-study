"""중급"""


def p1():
    print("\n문제 1: 안전 인터록의 양산 의미")
    print("  - 모터 손상 방지")
    print("  - 사람 안전 (e-stop)")
    print("  - 양산 시 99% 의 OOM / NaN / outlier action 차단")


def p2():
    print("\n문제 2: 인터록의 오버헤드 vs Total latency")
    print("  OpenVLA 165 + IK 5 + Safety 1 + Motor 8 = 179 ms")
    print("  Safety overhead = 1/179 = 0.5% (negligible)")


def p3():
    print("\n문제 3: Hierarchical 구조의 의미")
    print("  Slow VLA (165 ms) + Fast Safety (1 ms)")
    print("  -> 양산 시 safety policy 가 매 ms 마다 동작 가능")
    print("  -> 사람 안전 + 모터 손상 방지")


if __name__ == "__main__":
    p1(); p2(); p3()
