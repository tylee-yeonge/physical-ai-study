"""중급"""


def p1():
    print("\n문제 1: 영상의 핵심 메시지 한 줄")
    print()
    print("  '9년 AMR ROS 양산 SW 엔지니어가 자작 6DOF 팔에 OpenVLA")
    print("   fine-tune 으로 Real-to-Sim-to-Real 통합 + 정량 측정.'")


def p2():
    print("\n문제 2: 영상의 segment 별 시간 분배")
    print("  Intro 15s + System 30s + Real 45s + Sim mirror 30s")
    print("  + 결과 30s + 한계 20s + Next 10s = 180s (3분)")


def p3():
    print("\n문제 3: 면접관에게 가장 강력한 1초")
    print("  Real 자작 팔이 'pick up the red cup' 명령을 성공하는 frame")
    print("  + Sim 의 동일 동작 mirror")
    print("  -> 한 frame 에 모든 메시지 (VLA + Real + Sim + 양산)")


if __name__ == "__main__":
    p1(); p2(); p3()
