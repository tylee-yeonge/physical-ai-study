"""Phase 6 Week 3 - 중급"""


def p1():
    """Sim time 사용 시 ROS2 노드 설정"""
    print("\n문제 1: Sim time 사용 시 ROS2 노드의 parameter")
    print("  /vla_node 가 Sim 의 image 를 받으려면?")
    print()
    print("  A) use_sim_time:=true 설정 + Bridge 가 /clock publish")
    print("  B) ROS2 launch file 수정 만")
    print("  C) DDS QoS 만 변경")
    print("  D) 안 됨")
    # TODO
    ans = ""
    expected = "A"
    print(f"  당신: {ans} (기대 {expected})")


def p2():
    """ROS2 Bridge 의 동시 publish 부하"""
    print("\n문제 2: Bridge 의 부하 추정")
    print("  publish list:")
    print("    /image_raw   : 640x480 RGB @ 30 FPS = 27 MB/s")
    print("    /joint_states: 7 floats * 30 FPS    = 1 KB/s")
    print("    /tf          : 10 transforms * 30   = 5 KB/s")
    # TODO
    total = 0.0
    expected = 27 + 0.001 + 0.005  # ~ 27 MB/s
    print(f"  Total ~ {total} MB/s (기대 ~{expected})")
    print()
    print("  -> image 가 압도적 (99%+)")


def p3():
    """Phase 4 vla_node 와 통합 방법"""
    print("\n문제 3: Sim image -> vla_node 흐름")
    print()
    print("  Isaac Sim (/camera/image_raw publish)")
    print("       |")
    print("       v")
    print("  vla_node (image subscribe -> inference)")
    print("       |")
    print("       v")
    print("  /vla/action publish")
    print()
    print("  Phase 7 산출물 #4 의 토대.")
    print("  vla_node 코드 변경 X (image source 만 swap).")


if __name__ == "__main__":
    p1(); p2(); p3()
