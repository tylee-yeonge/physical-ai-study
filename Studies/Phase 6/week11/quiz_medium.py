"""Phase 6 Week 11 - 중급"""


def p1():
    """DR 의 randomization 범위 선택"""
    print("\n문제 1: DR randomization 범위 결정")
    print("  너무 좁음: Real 의 actual range cover 못 함")
    print("  너무 넓음: Sim 학습 시 noise 너무 큼 -> model 불안정")
    print()
    print("  표준 방법:")
    print("  A) Real 의 실측 분포 측정 + Sim 의 범위 ~ 1.5x")
    print("  B) 임의 결정")
    print("  C) 가장 넓게")
    print()
    # TODO
    ans = ""
    print(f"  당신: {ans}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """Histogram distance threshold"""
    print("\n문제 2: hist distance 의 'gap closed' 기준")
    print("  Default Sim vs Real: 145")
    print("  After DR: ?")
    print()
    print("  Phase 7 의 충분 기준 (대략):")
    print("  - hist distance < 50 -> good")
    print("  - hist distance < 30 -> excellent")


def p3():
    """Phase 7 의 robust action 검증"""
    print("\n문제 3: Sim/Real 의 OpenVLA action 비교")
    print()
    print("  Sim image -> OpenVLA action_sim")
    print("  Real image -> OpenVLA action_real")
    print()
    print("  action difference 측정:")
    print("    L2 distance of [dx, dy, dz, rx, ry, rz, gripper]")
    print()
    print("  기준:")
    print("    < 0.01 (norm) -> robust")
    print("    > 0.05 -> Sim/Real gap 큼, 추가 randomization 필요")


if __name__ == "__main__":
    p1(); p2(); p3()
