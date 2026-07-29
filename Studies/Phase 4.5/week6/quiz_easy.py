"""
Phase 4.5 Week 6 - 분석·보고 기초 퀴즈
"""


def problem1_method_locked():
    print("\n" + "=" * 50)
    print("문제 1: 결과가 아쉬울 때")
    print("=" * 50 + "\n")
    print("질문: 구간이 겹쳐 판정이 안 나왔다. 이때 유의수준을 0.05 에서 0.1 로")
    print("      바꾸는 것에 대한 옳은 태도는?\n")
    print("보기:")
    print("  A) 탐색적 연구이므로 0.1 이 더 적절하다. 바꿔서 보고한다")
    print("  B) week5 에서 미리 정한 값을 쓴다. 바꾸려면 바꾼 사실과 이유를 함께 적는다")
    print("  C) 두 값으로 모두 계산해 유리한 쪽을 결론으로 쓴다")
    print("  D) N 을 줄여 다시 계산한다")


def problem2_success_criterion():
    print("\n" + "=" * 50)
    print("문제 2: Phase 4.5 의 성공 기준")
    print("=" * 50 + "\n")
    print("질문: fine-tuned 성공률이 zero-shot 과 같았다. 이때 Phase 4.5 는?\n")
    print("보기:")
    print("  A) 실패다. 다시 학습해야 한다")
    print("  B) 성공 기준이 상승이 아니라 설계-실행-정량 분석이므로 성립한다")
    print("  C) 판정을 보류하고 Phase 6 으로 넘긴다")
    print("  D) task 를 바꿔 다시 측정해야 성립한다")


def problem3_exclusion():
    print("\n" + "=" * 50)
    print("문제 3: 원인 분석의 형태")
    print("=" * 50 + "\n")
    print("질문: '데이터가 적어서 / 분포가 달라서 / 학습 설정 때문에' 를 나열하는 것이")
    print("      분석으로 부족한 이유는?\n")
    print("보기:")
    print("  A) 항목 수가 너무 적다")
    print("  B) 누구나 쓸 수 있고 후보를 좁히지 못한다")
    print("  C) 통계적 근거가 없다")
    print("  D) 블로그 분량이 부족해진다")


def problem4_zero_report():
    print("\n" + "=" * 50)
    print("문제 4: 0/N 단계의 구간")
    print("=" * 50 + "\n")
    print("질문: 어떤 단계의 도달 수가 0/100 이었다. 올바른 구간 보고는?\n")
    print("보기:")
    print("  A) 하한 0, 상한 0")
    print("  B) 하한 0, 상한은 0 이 아닌 값")
    print("  C) 구간을 계산할 수 없어 생략")
    print("  D) 하한과 상한 모두 음수 가능")


def problem5_partial_claim():
    print("\n" + "=" * 50)
    print("문제 5: 부분 도달률이 개선됐을 때")
    print("=" * 50 + "\n")
    print("질문: grasped 단계가 4/100 에서 21/100 으로 늘었고 최종 성공은 양쪽 0 이다.")
    print("      쓰면 안 되는 문장은?\n")
    print("보기:")
    print("  A) 'grasped 도달률이 4/100 에서 21/100 으로 이동했다'")
    print("  B) '파악까지의 접근은 개선됐으나 최종 성공에는 이르지 못했다'")
    print("  C) 'fine-tuning 으로 성공률이 17%p 향상됐다'")
    print("  D) '최종 성공은 양쪽 모두 0/100 이었다'")


if __name__ == "__main__":
    print("=" * 50)
    problem1_method_locked()
    problem2_success_criterion()
    problem3_exclusion()
    problem4_zero_report()
    problem5_partial_claim()
    print("=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
