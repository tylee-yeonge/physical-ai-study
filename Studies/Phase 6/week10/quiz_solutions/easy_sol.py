"""Phase 6 Week 10 - 기초 정답"""


def p1():
    print("\n정답: B) PRESENT_CURRENT")
    print("  Dynamixel 내부 current 측정 (ADC) -> torque 계산")
    print("  별도 force sensor 불필요")


def p2():
    print("\n정답: B) get_applied_joint_efforts")
    print("  Articulation 의 method. Nm 단위.")


def p3():
    print("\n정답: A) Friction / inertia / gear ratio")
    print("  Sim 의 default friction 0 -> Real 보다 효율적")
    print("  Inertia 가 URDF 와 정확하지 않으면 mismatch")
    print("  Dynamixel 의 internal gear ratio (XM430: 353:1) 모사 필요")


def p4():
    print("\n정답: B) 조립 / 그립 성공률")
    print("  Force-sensitive task:")
    print("    - Bolt tightening (정확한 force)")
    print("    - Pick-and-place (over-grip 방지)")
    print("    - Assembly (insertion force)")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
