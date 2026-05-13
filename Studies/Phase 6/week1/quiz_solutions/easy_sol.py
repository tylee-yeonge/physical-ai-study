"""Phase 6 Week 1 - 기초 정답"""


def p1():
    print("\n정답: B) RTX 30 series")
    print("  Isaac Sim 4.x 최소: RTX 30 series (Ampere)")
    print("  본 phase: RTX 4070 (Ada Lovelace, 권장)")


def p2():
    print("\n정답: B) USD = file, Stage = in-memory")
    print("  USD: 3D scene file (.usd / .usda)")
    print("  Stage: 실행 시 메모리 안의 representation")
    print("  Prim: stage 안의 unit (object)")


def p3():
    print("\n정답: B) ssh 원격 실행")
    print("  Headless mode = GUI 없음")
    print("  -> X server 불필요")
    print("  -> ssh -X 안 써도 됨")
    print("  -> 출장지 / cloud / Docker 호환")


def p4():
    print("\n정답: A) Sim 데이터 -> ROS2 topic publish")
    print("  Bridge 가 변환:")
    print("    Sim 의 joint_state -> /joint_states")
    print("    Sim 의 camera -> /camera/image_raw")
    print("    Sim 의 transform -> /tf")
    print()
    print("  Real robot 과 같은 인터페이스 -> 코드 swap 가능")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
