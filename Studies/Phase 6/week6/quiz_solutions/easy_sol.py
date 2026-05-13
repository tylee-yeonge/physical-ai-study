"""Phase 6 Week 6 - 기초 정답"""


def p1():
    print("\n정답: C) Real -> Sim 동기화")


def p2():
    print("\n정답: B) < 0.01 rad")
    print("  ~ 0.5 deg, 자작 팔 manipulation 에 적당")


def p3():
    print("\n정답: B) < 100 ms")
    print("  Real robot serial (5ms) + ROS2 (~5ms) + Sim step (33ms) + bridge (~5ms)")


def p4():
    print("\n정답: A) Sim actuator response 차이")
    print("  Real Dynamixel: 자체 control loop + PID")
    print("  Sim default drive: PID 가 다를 수 있음")
    print("  -> Sim drive 의 kp/kd 를 Real Dynamixel 에 맞춰야")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
