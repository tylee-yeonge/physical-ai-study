"""
Phase 4.5 Week 5 - 변인 통제 + 구간 추정 퀴즈

서술형 문항은 답을 먼저 종이나 파일에 쓴 뒤 solution 과 대조한다.
"""


def problem1_n_decision():
    print("\n" + "=" * 50)
    print("문제 1: N 을 올리는 판단 (서술형)")
    print("=" * 50 + "\n")
    print("상황: week0 은 N=20 으로 baseline 을 측정했다. 추론 1회는 약 300 ms 이고")
    print("      step cap 은 100 이다.\n")
    print("질문: (1) N=100 으로 올릴 때 두 모델 총 실행 시간을 계산하라")
    print("      (2) N 을 올리면 어떤 추가 작업이 필요해지는가. 그 이유는")
    print("      (3) 그 추가 비용을 감수할 만한 근거는")


def problem2_leaked_variable():
    print("\n" + "=" * 50)
    print("문제 2: 무결성 검사에서 발견된 차이")
    print("=" * 50 + "\n")
    print("상황: 두 결과 파일의 메타를 비교했더니 다음 항목이 다르다.")
    print("        model, unnorm_key, step_cap\n")
    print("질문: 이때 취해야 할 조치는?\n")
    print("보기:")
    print("  A) step_cap 차이는 작으므로 결과에 주석만 달고 진행한다")
    print("  B) 두 측정을 같은 step_cap 으로 다시 실행한다")
    print("  C) 더 긴 step_cap 쪽 결과를 짧은 쪽 기준으로 잘라 맞춘다")
    print("  D) step_cap 이 큰 쪽이 유리하므로 그쪽만 다시 측정한다")


def problem3_discordant_pairs():
    print("\n" + "=" * 50)
    print("문제 3: 불일치 쌍 읽기 (서술형)")
    print("=" * 50 + "\n")
    print("상황: N=100 에서 짝지은 결과가 다음과 같다.")
    print("        둘 다 성공: 3    zero만 성공: 2    fine만 성공: 9    둘 다 실패: 86\n")
    print("질문: (1) 집계 성공률을 각각 계산하라")
    print("      (2) 차이에 대한 정보를 주는 관측 수는 몇 개인가")
    print("      (3) 이 표를 독립 두 집단으로 보는 것보다 짝지어 보는 것이")
    print("          유리한 이유를 설명하라")


def problem4_zero_upper_bound():
    print("\n" + "=" * 50)
    print("문제 4: 0/100 의 보고 문장")
    print("=" * 50 + "\n")
    print("상황: zero-shot 이 0/100, fine-tuned 가 12/100 이다.\n")
    print("질문: 가장 적절한 보고 문장은?\n")
    print("보기:")
    print("  A) 'zero-shot 0%, fine-tuned 12% 로 12%p 향상'")
    print("  B) 'zero-shot 0% (± 0%), fine-tuned 12% (± 6%)'")
    print("  C) 'zero-shot 0/100 (95% 구간 상한 약 X%), fine-tuned 12/100 (구간 Y-Z%)'")
    print("  D) 'zero-shot 은 측정 불가, fine-tuned 만 12%'")


def problem5_all_zero():
    print("\n" + "=" * 50)
    print("문제 5: 양쪽 모두 최종 성공 0 일 때 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 최종 성공률이 zero-shot 0/100, fine-tuned 0/100 이다.\n")
    print("질문: (1) 이 시점에 무엇을 보는가")
    print("      (2) 그 기준을 이번 주에 조정하면 안 되는 이유")
    print("      (3) 여기서 나올 수 있는 week6 의 진술 형태를 하나 예로 들라")


def problem6_week0_reproduction():
    print("\n" + "=" * 50)
    print("문제 6: week0 재현 검사의 의미 (서술형)")
    print("=" * 50 + "\n")
    print("상황: zero-shot 을 N=100 으로 재측정한 뒤, week0 이 쓴 20개 seed 에")
    print("      해당하는 줄만 뽑아 week0 결과와 비교했다.\n")
    print("질문: (1) 두 결과가 일치해야 하는 이유")
    print("      (2) 일치하지 않으면 무엇을 의심하는가 (후보 3개 이상)")


if __name__ == "__main__":
    print("=" * 50)
    problem1_n_decision()
    problem2_leaked_variable()
    problem3_discordant_pairs()
    problem4_zero_upper_bound()
    problem5_all_zero()
    problem6_week0_reproduction()
    print("=" * 50)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("=" * 50)
