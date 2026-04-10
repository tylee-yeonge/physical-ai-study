"""
Quiz Medium - Week 6: 성능 분석 및 개선 (Section 6.2)
3문제 - 심화 이해
"""


def problem1_ablation_analysis():
    print("\n" + "━" * 28)
    print("문제 1: Ablation Study 해석")
    print("━" * 28 + "\n")

    print("Q: 다음 Ablation Study 결과를 분석하세요.\n")
    print("   ┌──────────────────────────┬──────────────┐")
    print("   │ 실험                      │ AP3D (%)     │")
    print("   ├──────────────────────────┼──────────────┤")
    print("   │ Baseline                 │ 12.5         │")
    print("   │ + Multi-scale FPN        │ 14.0         │")
    print("   │ + Data Augmentation      │ 13.8         │")
    print("   │ + Multi-scale + Augment  │ 15.2         │")
    print("   │ + 3D NMS                 │ 13.1         │")
    print("   │ All combined             │ 15.8         │")
    print("   └──────────────────────────┴──────────────┘\n")
    print("   1) 가장 기여도가 높은 단일 기법은 무엇인가?")
    print("   2) Multi-scale과 Augmentation을 함께 사용하면")
    print("      개별 기여도의 합(1.5+1.3=2.8)보다 높은 2.7이 나왔습니다.")
    print("      이것이 의미하는 바는?")
    print("   3) 3D NMS 단독 효과가 작은 이유는?\n")
    print("   답: _____\n")


def problem2_depth_improvement():
    print("\n" + "━" * 28)
    print("문제 2: Depth 추정 개선 전략")
    print("━" * 28 + "\n")

    print("Q: Monocular 3D Detection에서 Depth 추정을 개선하기 위한")
    print("   3가지 전략을 설명하고, 각각의 장단점을 비교하세요.\n")
    print("   전략 1 (Direct Regression): _____")
    print("     장점: _____")
    print("     단점: _____\n")
    print("   전략 2 (Depth Bin Classification): _____")
    print("     장점: _____")
    print("     단점: _____\n")
    print("   전략 3 (기하학적 제약 활용): _____")
    print("     장점: _____")
    print("     단점: _____\n")


def problem3_performance_diagnosis():
    print("\n" + "━" * 28)
    print("문제 3: 성능 진단 시나리오")
    print("━" * 28 + "\n")

    print("Q: 모델의 거리별 성능이 다음과 같을 때, 각 구간의 주요 실패 원인을")
    print("   분석하고 개선 방법을 제안하세요.\n")
    print("   ┌──────────┬──────────┬──────────────────────┐")
    print("   │ 거리 구간 │ AP3D (%) │ 주요 실패 원인 (추론) │")
    print("   ├──────────┼──────────┼──────────────────────┤")
    print("   │ 0-10m    │ 20.5     │ _____                │")
    print("   │ 10-20m   │ 28.3     │ (가장 좋은 구간)      │")
    print("   │ 20-30m   │ 12.1     │ _____                │")
    print("   │ 30-40m   │ 4.2      │ _____                │")
    print("   │ 40m+     │ 0.8      │ _____                │")
    print("   └──────────┴──────────┴──────────────────────┘\n")
    print("   힌트: 0-10m에서 성능이 10-20m보다 낮은 이유는 무엇일까요?")
    print("   답: _____\n")


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 6 Quiz - Medium (성능 분석 및 개선)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_ablation_analysis()
    problem2_depth_improvement()
    problem3_performance_diagnosis()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
