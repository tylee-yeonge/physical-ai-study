"""Phase 6 Week 1 - 중급"""


def p1():
    """RTX 4070 hardware budget"""
    print("\n문제 1: VRAM 사용 추정")
    sim = 4.0  # Isaac Sim 단순 scene
    openvla = 5.3
    ros2_bridge = 0.5

    # TODO: total
    total_sim_only = 0.0
    total_with_vla = 0.0
    print(f"  Sim only         : {total_sim_only:.1f} GB")
    print(f"  + OpenVLA + Bridge: {total_with_vla:.1f} GB")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """conda vs Omniverse Launcher"""
    print("\n문제 2: 본 phase 권장 설치 방법")
    print("  A) Omniverse Launcher (GUI only)")
    print("  B) Conda + pip (출장지 ssh 호환)")
    print("  C) Docker only")
    print("  D) Manual build")
    # TODO
    ans = ""
    print(f"  당신: {ans}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """USD vs URDF"""
    print("\n문제 3: USD 와 URDF 의 관계")
    print("  URDF: ROS 의 robot description")
    print("  USD : NVIDIA / Pixar 의 scene description")
    print()
    print("  Isaac Sim 에서:")
    print("    URDF 임포트 -> USD 로 변환")
    print("    Isaac Sim 의 native 포맷은 USD")
    print()
    print("  자작 팔 URDF 가 어떻게 USD 로 변환되는가:")
    print("    Isaac Sim 의 URDF Importer (week 4 의 핵심)")


if __name__ == "__main__":
    p1(); p2(); p3()
