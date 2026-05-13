"""Phase 6 Week 4 - 중급 정답"""


def p1():
    print("\n정답: URDF -> USD joint type")
    print("  revolute    -> RevoluteJoint (with limit)")
    print("  prismatic  -> PrismaticJoint")
    print("  continuous -> RevoluteJoint (no limit)")
    print("  fixed       -> FixedJoint (보통 merge 옵션 시 사라짐)")


def p2():
    print("\n정답: fix_base=True 필수")
    print("  자작 팔 (6DOF) 의 base 가 떠 있으면:")
    print("    중력으로 떨어짐 -> joint 동작 안 함")
    print()
    print("  Mobile robot (예: AMR) 인 경우 fix_base=False")
    print("  Manipulator 는 항상 fix_base=True")


def p3():
    print("\n정답: D) 모두 가능")
    print()
    print("  Joint 동작 안 함의 trouble:")
    print("    1. Drive kp/kd 가 0 -> set_kp / set_kd")
    print("    2. DOF index 잘못 -> dof_names 로 확인")
    print("    3. Sim 이 paused -> world.play()")
    print()
    print("  Set 방법 2가지:")
    print("    set_joint_position_targets : drive 사용 (PID)")
    print("    set_joint_position         : 직접 set (kinematic)")
    print()
    print("  Phase 7 의 closed loop 에서는 set_joint_position_targets 사용")
    print("  (Real robot 의 controller 와 같은 동작)")


if __name__ == "__main__":
    p1(); p2(); p3()
