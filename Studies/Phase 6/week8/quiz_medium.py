"""Phase 6 Week 8 - 중급"""


def p1():
    """Closed loop latency 계산"""
    print("\n문제 1: Closed loop latency (Phase 7 산출물 #4)")
    inf = 165  # OpenVLA
    sim = 35   # Sim step
    real_act = 10  # Dynamixel
    # TODO
    total = 0
    print(f"  Inference + Sim + Real = {total} ms")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """Latency 의 양산 의미"""
    print("\n문제 2: 30 ms gap 의 위치 오차")
    speed = 0.1  # m/s 자작 팔
    gap_ms = 30
    # TODO
    error = 0.0
    print(f"  Error = {error*1000:.1f} mm")
    print(f"  자작 팔 cm 단위 manipulation 에 영향")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """Latency 보고서의 핵심 표"""
    print("\n문제 3: Phase 7 보고서의 latency 표 (예시)")
    print()
    print("  | Latency Component | Mean | p95 |")
    print("  |---|---|---|")
    print("  | OpenVLA inference | 165 ms | 220 |")
    print("  | Sim step          | 35 ms  | 50  |")
    print("  | Real actuator     | 8 ms   | 12  |")
    print("  | ROS2 transport    | 5 ms   | 10  |")
    print("  | **Closed loop**   | **213**| **292**|")
    print()
    print("  이 표가 Phase 7 산출물 #4 의 핵심 증거")


if __name__ == "__main__":
    p1(); p2(); p3()
