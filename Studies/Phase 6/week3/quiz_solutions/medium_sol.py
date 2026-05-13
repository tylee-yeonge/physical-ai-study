"""Phase 6 Week 3 - 중급 정답"""


def p1():
    print("\n정답: A) use_sim_time:=true + /clock publish")
    print()
    print("  ROS2 launch:")
    print("    Node(package='vla_node', executable='vla_inference_node',")
    print("         parameters=[{'use_sim_time': True}])")
    print()
    print("  Bridge 의 /clock topic:")
    print("    Sim 의 elapsed time publish")
    print("    ROS2 의 모든 use_sim_time:=true 노드가 이걸 따라감")


def p2():
    print("\n정답: ~ 27 MB/s")
    print("  /image_raw 가 99%+")
    print()
    print("  Bridge 부하 최적화:")
    print("    - image resolution 줄임 (640 -> 224)")
    print("    - FPS 줄임 (30 -> 10)")
    print("    - compressed image (jpg) 권장")
    print("    - depth 만 publish, RGB 생략 (필요 시)")


def p3():
    print("\n정답: Sim image -> vla_node 흐름")
    print()
    print("  핵심: vla_node 의 image_callback 은 source 와 무관")
    print("  -> Sim 이든 Real 이든 동일하게 작동")
    print()
    print("  통합 흐름:")
    print("    Sim publish /camera/image_raw")
    print("    vla_node subscribe -> inference -> publish /vla/action")
    print("    joint_command_node subscribe -> publish /joint_command")
    print("    Sim subscribe /joint_command -> articulation control")
    print()
    print("  closed loop! Phase 7 산출물 #4 핵심.")


if __name__ == "__main__":
    p1(); p2(); p3()
