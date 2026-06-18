"""Phase 6 Week 4 - 중급"""


def p1():
    """URDF Joint type 매핑"""
    print("\n문제 1: URDF -> USD joint type")
    print("  URDF: revolute, prismatic, continuous, fixed")
    print("  USD : ?")
    print()
    # TODO
    mapping = {"revolute": "", "prismatic": "", "continuous": "", "fixed": ""}
    for k in mapping:
        print(f"  {k:12s} -> 당신: {mapping[k]}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """자작 팔 임포트 디버깅"""
    print("\n문제 2: 임포트 후 robot 이 떠 있음")
    print("  원인: ?")
    print("  해결: ?")
    print()
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """Articulation drive 설정"""
    print("\n문제 3: Joint 가 set_joint_positions 호출해도 안 움직임")
    print()
    print("  원인 가능성:")
    print("  A) Drive 의 kp/kd 가 0 (drive 가 없음)")
    print("  B) DOF index 잘못")
    print("  C) Sim 이 paused")
    print("  D) 모두 가능")
    print()
    print("  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    p1(); p2(); p3()
