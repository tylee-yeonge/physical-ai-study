"""Phase 6 Week 4 - 기초 정답"""


def p1():
    print("\n정답: B) omni.importer.urdf")


def p2():
    print("\n정답: B) 7 (6 + gripper)")
    print("  자작 팔 Hardware-Arm Stage 2:")
    print("    6 revolute (XM430 모터 6개)")
    print("    + 1 gripper (revolute 또는 prismatic)")


def p3():
    print("\n정답: B) Base 가 world 에 고정")
    print("  fix_base=True 가 거의 모든 manipulator 에 필수")
    print("  False 면 robot 이 중력으로 떨어짐")


def p4():
    print("\n정답: A) get/set_joint_positions")
    print("  + set_joint_position_targets (drive 사용)")
    print("  + get_joint_velocities")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
