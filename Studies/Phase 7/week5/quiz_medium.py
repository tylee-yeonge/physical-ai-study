"""중급"""


def p1():
    print("\n문제 1: 충돌 감지 threshold")
    print("  너무 낮음 (0.1 Nm) : false positive 빈번")
    print("  너무 높음 (5 Nm)   : 충돌 못 잡음")
    print("  자작 팔 권장: 1~2 Nm (XM430 max 의 ~50%)")


def p2():
    print("\n문제 2: e-stop 의 ROS2 통합")
    print("  /estop topic publish")
    print("  모든 노드가 subscribe -> emergency state")
    print("  motor controller 가 disable")


def p3():
    print("\n문제 3: 양산 SW 의 안전 시스템")
    print("  플러그-앤-플레이 안전 시스템:")
    print("  - e-stop button (필수)")
    print("  - 충돌 감지 (필수)")
    print("  - 위치/속도/토크 한계 (필수)")
    print("  - 통신 timeout (필수)")
    print("  - watchdog (필수)")


if __name__ == "__main__":
    p1(); p2(); p3()
