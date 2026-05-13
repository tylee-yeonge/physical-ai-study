"""Phase 6 Week 10 - 기초"""


def p1():
    print("\n문제 1: Dynamixel torque 측정")
    print("  A) PRESENT_POSITION\n  B) PRESENT_CURRENT (current -> torque)\n  C) Direct read torque\n  D) Force sensor 추가 필요")


def p2():
    print("\n문제 2: Sim 의 effort method")
    print("  A) get_joint_positions\n  B) get_applied_joint_efforts\n  C) get_velocity\n  D) get_world_pose")


def p3():
    print("\n문제 3: Sim/Real torque gap 의 원인")
    print("  A) Friction / inertia / gear ratio")
    print("  B) Network")
    print("  C) GPU memory")
    print("  D) Python")


def p4():
    print("\n문제 4: Force feedback 의 양산 의미")
    print("  A) GPU 가속\n  B) 조립 / 그립 의 성공률\n  C) Network 안정\n  D) memory 효율")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
