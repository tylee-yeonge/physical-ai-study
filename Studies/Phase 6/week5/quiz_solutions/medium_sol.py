"""Phase 6 Week 5 - 중급 정답"""


def p1():
    print("\n정답: B) 매핑 표 적용 후 set")
    print()
    print("  코드 예시:")
    print("  def ros2_to_sim(ros2_msg, sim_arm, mapping):")
    print("      sim_positions = np.zeros(sim_arm.num_dof)")
    print("      for urdf_idx, sim_idx in mapping.items():")
    print("          sim_positions[sim_idx] = ros2_msg.position[urdf_idx]")
    print("      sim_arm.set_joint_position_targets(sim_positions)")
    print()
    print("  매핑 표 없이 직접 적용 시:")
    print("    joint_2 의 angle 이 gripper 에 적용 -> robot 이상 동작")


def p2():
    print("\n정답: URDF 길이 / origin 검증")
    print()
    print("  10 mm 오차 원인:")
    print("    - URDF 의 link length 가 mm 단위 오류")
    print("    - joint origin xyz 가 부정확")
    print("    - mesh 의 visual origin != 실제 link 위치")
    print()
    print("  검증 방법:")
    print("    1. 자작 팔 실측 (caliper 로 link 길이)")
    print("    2. URDF 의 <origin xyz=> 값과 비교")
    print("    3. 차이가 있으면 URDF 수정")
    print()
    print("  자작 팔 양산 SW 엔지니어로서 이 검증이 차별점")


def p3():
    print("\n정답: URDF tree 검증")
    print()
    print("  - parent/child 관계 잘못이면 movement 전파")
    print("  - joint_2 가 joint_3 의 parent 가 되어야")
    print()
    print("  검증 명령:")
    print("    $ urdf_to_graphviz my_arm.urdf")
    print("    $ open my_arm.pdf  # tree 시각화")
    print()
    print("  Tree 가 정확하면:")
    print("    base_link -> joint_1 -> link_1 -> joint_2 -> link_2 -> ...")


if __name__ == "__main__":
    p1(); p2(); p3()
