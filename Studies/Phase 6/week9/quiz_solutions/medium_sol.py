"""Phase 6 Week 9 - 중급 정답"""


def p1():
    print("\n정답: D) 모두 가능")
    print("  Isaac Sim PhysX 의 non-determinism:")
    print("    - multi-thread scheduling")
    print("    - GPU asynchronous ops")
    print("    - floating point reduction order")
    print()
    print("  완전 deterministic 만들려면:")
    print("    - PhysX deterministic mode")
    print("    - Fixed thread count")
    print("    - Same seed for any RNG")


def p2():
    print("\n정답: 대책 4가지")
    print("  Hardware (Stage 2): backlash 줄이기")
    print("  Software: low-pass filter on sensor read")
    print("  Calibration: 정기 재캘리브 (자작 팔 양산 SW 의 교훈)")
    print("  Temperature: 5~10분 warm-up 후 사용")


def p3():
    print("\n정답: Sim + Domain Randomization")
    print()
    print("  Sim 안에서 의도적 noise 주입:")
    print("    - Random lighting")
    print("    - Random color/texture")
    print("    - Random initial pose")
    print("    - Random gravity")
    print()
    print("  결과: Sim 학습된 model 이 Real 의 noise 에 robust")
    print("  Phase 7 의 산출물 #4 의 한 option")


if __name__ == "__main__":
    p1(); p2(); p3()
