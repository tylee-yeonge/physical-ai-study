"""중급"""


def p1():
    print("\n문제 1: Throughput 계산")
    total = 184
    hz = 1000 / total
    print(f"  Total 184 ms -> {hz:.2f} Hz")
    print("  5.4 Hz (자작 팔 cm-level 적당)")


def p2():
    print("\n문제 2: latency 의 30Hz 가능 여부")
    print("  목표 cycle: 33 ms (30 Hz)")
    print("  실제: 184 ms")
    print("  30 Hz 불가 - 184 ms / 33 = ~ 5.6x 차이")
    print("  -> hierarchical (slow VLA + fast safety) 필수")


def p3():
    print("\n문제 3: 양산 적용 시")
    print("  '5.4 Hz VLA + 100 Hz safety' 의 hierarchical 가능?")
    print("  - VLA: 184 ms 마다 새 action")
    print("  - safety: 10 ms 마다 monitor (인터록 / 충돌 감지)")
    print("  - motor: 1 kHz 의 자체 control loop")
    print("  -> 양산 가능 패턴")


if __name__ == "__main__":
    p1(); p2(); p3()
