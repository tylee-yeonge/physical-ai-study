"""Phase 6 Week 10 - 중급"""


def p1():
    """Torque conversion"""
    print("\n문제 1: Dynamixel current -> torque")
    print("  Read PRESENT_CURRENT: 200 (uint16)")
    print("  torque_constant: 4.1 mNm/mA")
    print()
    # TODO
    cur = 200
    torque_mNm = cur * 4.1
    print(f"  Torque = {torque_mNm:.1f} mNm = {torque_mNm/1000:.3f} Nm")
    print()
    print("  부호 처리: cur > 32767 일 때 cur -= 65536")


def p2():
    """Friction parameter 조정"""
    print("\n문제 2: Sim friction 조정 방법")
    print("  Isaac Sim 의 PhysX joint friction:")
    print()
    print("  set_articulation_property(prim_path, 'friction', 0.05)")
    print()
    print("  Real 의 friction 측정 -> Sim 에 적용")
    print("  Sim/Real gap 줄임")


def p3():
    """Domain randomization with friction"""
    print("\n문제 3: friction 의 domain randomization")
    print()
    print("  Sim 안에서 매 episode 마다 다른 friction:")
    print("    friction ~ uniform(0.01, 0.1)")
    print()
    print("  학습된 model 이 다양한 friction 에 robust")
    print("  Real 의 unknown friction 에도 대응 가능")


if __name__ == "__main__":
    p1(); p2(); p3()
