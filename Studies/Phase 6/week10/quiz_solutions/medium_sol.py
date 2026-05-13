"""Phase 6 Week 10 - 중급 정답"""


def p1():
    cur = 200
    torque = cur * 4.1
    print(f"\n정답: {torque:.1f} mNm = {torque/1000:.3f} Nm")
    print()
    print("  Signed conversion (XM430 의 PRESENT_CURRENT):")
    print("    if cur > 32767: cur -= 65536")
    print("    torque = cur * 4.1 mNm")
    print()
    print("  최대 torque (XM430): ~ 4 Nm")


def p2():
    print("\n정답: Sim friction 조정")
    print()
    print("  PhysX SDK 접근:")
    print("    joint.set_friction_coefficient(0.05)")
    print()
    print("  또는 USD 의 PhysicsJointAPI:")
    print("    joint.GetFrictionAttr().Set(0.05)")
    print()
    print("  Real 의 friction 측정 + Sim 에 적용 -> Sim2Real 핵심")


def p3():
    print("\n정답: friction randomization")
    print()
    print("  Phase 7 산출물 #4 의 한 option:")
    print("    - 매 episode reset 시 friction 변경")
    print("    - 학습된 model 의 generalization 강화")
    print("    - Real 의 unknown friction (사용 마모) 에도 robust")
    print()
    print("  Domain Randomization 의 표준 기법 중 하나")


if __name__ == "__main__":
    p1(); p2(); p3()
