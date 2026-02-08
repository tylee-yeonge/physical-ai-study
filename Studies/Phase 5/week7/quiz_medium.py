"""
Quiz Medium - Week 7: Monocular Depth 이론 (Section 5.3)
3문제 - 심화 이해
"""


def problem1_teacher_student():
    print("\n" + "━" * 28)
    print("문제 1: Teacher-Student 프레임워크")
    print("━" * 28 + "\n")

    print("Depth Anything의 Teacher-Student 학습 과정을 단계별로 설명하고,")
    print("Student 모델이 Teacher보다 더 좋은 성능을 낼 수 있는 이유를 서술하세요.")
    print()
    print("  Step 1: _____")
    print("  Step 2: _____")
    print("  Step 3: _____")
    print()
    print("  Student > Teacher인 이유: _____")
    print()


def problem2_scale_invariant_loss():
    print("\n" + "━" * 28)
    print("문제 2: Scale-Invariant Loss")
    print("━" * 28 + "\n")

    print("MiDaS에서 사용하는 Scale-and-Shift Invariant Loss가")
    print("필요한 이유와 동작 원리를 설명하세요.")
    print()
    print("  다음 상황을 예시로 설명:")
    print("  - 데이터셋 A: 깊이 범위 0~80m (야외 KITTI)")
    print("  - 데이터셋 B: 깊이 범위 0~10m (실내 NYU)")
    print()
    print("  일반 L1 Loss로 혼합 학습하면 발생하는 문제: _____")
    print("  Scale-Invariant Loss가 해결하는 방법: _____")
    print()


def problem3_depth_for_slam():
    print("\n" + "━" * 28)
    print("문제 3: 깊이 추정과 SLAM의 관계")
    print("━" * 28 + "\n")

    print("Monocular SLAM에서 단안 깊이 추정 모델을 활용하는 방법과")
    print("한계를 설명하세요.")
    print()
    print("  1. Monocular SLAM의 근본적 문제점 (Scale Ambiguity)은?")
    print("     설명: _____")
    print()
    print("  2. Depth Anything의 상대 깊이를 SLAM에 활용하려면?")
    print("     방법: _____")
    print()
    print("  3. 깊이 추정 모델의 오차가 SLAM에 미치는 영향은?")
    print("     영향: _____")
    print()
    print("  4. 실제 시스템에서의 권장 접근법은?")
    print("     접근법: _____")
    print()


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 7 Quiz - Medium (Monocular Depth 이론)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_teacher_student()
    problem2_scale_invariant_loss()
    problem3_depth_for_slam()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
