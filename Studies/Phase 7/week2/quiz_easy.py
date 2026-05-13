"""Phase 7 Week 2 - 기초"""


def p1():
    print("\n문제 1: ee_delta -> joint 변환")
    print("  A) FK\n  B) IK (Inverse Kinematics)\n  C) Random\n  D) Identity")


def p2():
    print("\n문제 2: LoRA fine-tuned 의 unnormalize_key")
    print("  A) bridge_orig (zero-shot)\n  B) 자작 팔 자체 통계 (custom)\n  C) 무관\n  D) RT-1")


def p3():
    print("\n문제 3: Output topic")
    print("  A) /vla/action (Twist)\n  B) /joint_command (JointState)\n  C) 둘 다\n  D) 없음")


def p4():
    print("\n문제 4: Phase 4 와 차이")
    print("  A) 같음\n  B) Fine-tuned + joint output + 자작 팔\n  C) 모델만 다름\n  D) Topic 만 다름")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
