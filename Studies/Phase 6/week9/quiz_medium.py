"""Phase 6 Week 9 - 중급"""


def p1():
    """Sim std 가 큰 경우"""
    print("\n문제 1: Sim std 가 0.5 mm (기대보다 큰)")
    print("  원인 가능성:")
    print("  A) PhysX 의 multi-thread non-determinism")
    print("  B) Random seed 변동")
    print("  C) Floating point precision")
    print("  D) 모두 가능")


def p2():
    """Real std reduction"""
    print("\n문제 2: Real std 줄이는 방법")
    print("  자작 팔의 backlash + sensor noise + temperature")
    print()
    print("  대책:")
    print("    1. Mechanical: bearing tightening, lubricant")
    print("    2. Sensor: averaging, low-pass filter")
    print("    3. Calibration: 정기 재캘리브")
    print("    4. Temperature: 안정화 후 사용")


def p3():
    """Domain randomization 의 필요"""
    print("\n문제 3: Real variability 와 Sim 학습")
    print("  LoRA fine-tune 시:")
    print("  - Sim 만 사용: Real 의 noise 가 OOD")
    print("  - Real 만 사용: limited data")
    print("  - Sim + Domain Randomization (noise 주입): 균형")
    print()
    print("  Phase 7 산출물 #4 의 한 옵션 (Domain Randomization)")


if __name__ == "__main__":
    p1(); p2(); p3()
