"""Phase 6 Week 3 - 기초 정답"""


def p1():
    print("\n정답: B) omni.isaac.ros2_bridge")
    print("  Omniverse extension 의 표준 이름.")


def p2():
    print("\n정답: B) Deterministic")
    print("  Sim time = simulation 의 자체 시계")
    print("  같은 input 으로 다시 실행 시 같은 output")
    print("  benchmark / regression 의 핵심.")


def p3():
    print("\n정답: B) Sim publish/subscribe graph")
    print("  Action Graph:")
    print("    - GUI editor 로 시각적 작성")
    print("    - 또는 Python API")
    print("    - 모든 ROS2 통합 (publish, subscribe, action) 의 단위")


def p4():
    print("\n정답: B) ~ 38 ms")
    print("  Sim step 33 + bridge 1 + DDS 5 = 38 ms")
    print("  Real robot 의 ~ 10 ms 보다 느림")
    print("  -> Phase 6 week 8 의 latency gap 측정의 토대")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
