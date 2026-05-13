"""Phase 6 Week 6 - 기초"""


def p1():
    print("\n문제 1: 디지털 트윈의 정의")
    print("  A) Sim 만\n  B) Real 만\n  C) Real -> Sim 동기화\n  D) Sim -> Real 동기화")


def p2():
    print("\n문제 2: 매칭 정확도 기준 (joint)")
    print("  A) < 0.001 rad\n  B) < 0.01 rad (~0.5 deg)\n  C) < 0.1 rad\n  D) < 1 rad")


def p3():
    print("\n문제 3: Real -> Sim latency 기대")
    print("  A) < 10 ms\n  B) < 100 ms\n  C) < 1 s\n  D) < 10 s")


def p4():
    print("\n문제 4: 디지털 트윈의 한계 원인")
    print("  A) Sim actuator response != Real")
    print("  B) Network 통신")
    print("  C) GPU 부족")
    print("  D) URDF 잘못")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
