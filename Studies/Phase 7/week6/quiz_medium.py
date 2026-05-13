"""중급"""


def p1():
    print("\n문제 1: Latency breakdown")
    print("  Inference: 165")
    print("  IK: 5")
    print("  Safety: 1")
    print("  Motor write: 8")
    print("  Total: 179 ms")
    print("  Throughput: ~ 5.5 Hz")


def p2():
    print("\n문제 2: 안전 violation 시 동작")
    print("  Safety check fail -> action 발행 안 함")
    print("  -> motor 가 last position 유지")
    print("  -> robot 정지 (안전한 상태)")


def p3():
    print("\n문제 3: 첫 Real 실행의 점검 항목")
    print("  - Joint home position 설정")
    print("  - Velocity 50% 제한")
    print("  - e-stop 사용자 손이 닿는 위치")
    print("  - 충돌 감지 verbose log")
    print("  - 카메라 view 확인 (사람 인식)")


if __name__ == "__main__":
    p1(); p2(); p3()
