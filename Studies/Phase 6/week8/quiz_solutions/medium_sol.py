"""Phase 6 Week 8 - 중급 정답"""


def p1():
    print("\n정답: 210 ms")
    print("  Inference 165 + Sim 35 + Real 10 = 210 ms")
    print("  (Real-to-Sim-to-Real closed loop)")
    print()
    print("  Throughput: ~ 5 Hz")
    print("  '30 Hz 실시간 제어 불가, 양산 시 hierarchical 필요'")


def p2():
    print("\n정답: 3 mm error")
    print("  0.1 m/s * 0.03 s = 3 mm")
    print()
    print("  자작 팔 manipulation 영향:")
    print("    cm 단위 pick-and-place: 3 mm 무시 가능")
    print("    mm 단위 정밀 작업: 영향 큼")
    print()
    print("  Phase 7 산출물 #4 의 면접 포인트")


def p3():
    print("\n정답: Phase 7 보고서 표")
    print("  본 표가 산출물 #4 의 정량 증거")
    print("  면접관에게 'latency 의 양산 의미' 직접 어필")


if __name__ == "__main__":
    p1(); p2(); p3()
