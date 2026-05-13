"""Phase 6 Week 8 - 기초 (Latency)"""


def p1():
    print("\n문제 1: Sim step latency 기대")
    print("  A) <5 ms\n  B) ~ 33-50 ms\n  C) ~ 200 ms\n  D) ~ 1 s")


def p2():
    print("\n문제 2: Dynamixel XM430 response time")
    print("  A) <1 ms\n  B) ~ 5-10 ms\n  C) ~ 100 ms\n  D) ~ 1 s")


def p3():
    print("\n문제 3: Latency 측정의 표준")
    print("  A) Python time.time() 만\n  B) ROS2 header.stamp\n  C) GPU clock\n  D) 무관")


def p4():
    print("\n문제 4: 본 phase 의 latency measurement 4 가지")
    print("  A) Inference / Sim step / Real actuator / Closed loop")
    print("  B) GPU / CPU / RAM / Disk")
    print("  C) Latency 만")
    print("  D) Throughput 만")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
