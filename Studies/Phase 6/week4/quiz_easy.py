"""Phase 6 Week 4 - 기초 (URDF import)"""


def p1():
    print("\n문제 1: URDF Importer module")
    print("  A) omni.isaac.urdf\n  B) omni.importer.urdf\n  C) urdf_import\n  D) ros2_urdf")


def p2():
    print("\n문제 2: 6DOF 자작 팔 의 DOF 수 (gripper 포함)")
    print("  A) 6\n  B) 7 (6 + gripper)\n  C) 12\n  D) 5")


def p3():
    print("\n문제 3: fix_base=True 의 의미")
    print("  A) Robot 이 떠다님\n  B) Base 가 world 에 고정\n  C) Mesh 가 안 보임\n  D) Inertia 가 0")


def p4():
    print("\n문제 4: Articulation 의 핵심 method")
    print("  A) get_joint_positions / set_joint_positions")
    print("  B) get_xy")
    print("  C) connect / disconnect")
    print("  D) start / stop")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
