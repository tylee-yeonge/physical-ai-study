"""Phase 6 Week 5 - 중급"""


def p1():
    """매핑 표 활용"""
    print("\n문제 1: ROS2 joint_state 메시지 처리")
    print("  Real robot 의 ROS2 메시지: joint_1, ..., joint_6, gripper 순")
    print("  Sim 의 dof_names: gripper, joint_1, ..., joint_6 순")
    print()
    print("  ROS2 msg 받아 Sim 에 적용 시 코드:")
    print()
    print("  A) arm.set_joint_positions(msg.position)  # 직접")
    print("  B) 매핑 표 적용 후 set_joint_positions")
    print("  C) 첫 dof 부터 순서대로")
    print()
    # TODO
    ans = ""
    print(f"  당신: {ans}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """FK 오차의 원인 추정"""
    print("\n문제 2: FK 오차 10mm (예상보다 큰)")
    print()
    print("  가능 원인:")
    print("  - URDF 의 link length 잘못")
    print("  - URDF 의 joint origin xyz 오차")
    print("  - Mesh 의 origin 잘못")
    print()
    print("  본인 자작 팔의 URDF 가 mm 단위로 정확한가?")
    print("  Hardware-Arm Stage 1 에서 측정 / 확인")


def p3():
    """단독 joint 동작 시 다른 joint 도 움직임"""
    print("\n문제 3: joint_2 set 시 joint_3 도 움직임 발생")
    print()
    print("  원인 가능성:")
    print("  - URDF 의 joint 연결 잘못 (link 가 잘못 parent)")
    print("  - Mesh 가 잘못 joint 에 부착")
    print()
    print("  검증:")
    print("    각 link 의 parent_joint 출력")
    print("    URDF tree 시각화 (rviz2 또는 urdf_to_graphviz)")


if __name__ == "__main__":
    p1(); p2(); p3()
