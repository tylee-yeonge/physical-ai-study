"""
Phase 4.5 Week 6 - 주장 범위 + 배제 분석 퀴즈

서술형 문항은 답을 먼저 종이나 파일에 쓴 뒤 solution 과 대조한다.
"""


def problem1_exclusion_table():
    print("\n" + "=" * 50)
    print("문제 1: 배제 표를 채울 수 없는 행 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 배제된 후보 표를 채우다가 '라벨 변환 손실' 행의 근거를 못 찾았다.")
    print("      week1 round-trip 검증에서 강한 기준이 실패하고 약한 기준만")
    print("      통과한 상태로 진행했기 때문이다.\n")
    print("질문: (1) 이 행을 어떻게 처리해야 하는가")
    print("      (2) 그 처리가 분석의 정직성에 왜 중요한가")
    print("      (3) 이 상황이 결론 문장에 미치는 영향")


def problem2_discordant_zero():
    print("\n" + "=" * 50)
    print("문제 2: 불일치 쌍이 0 개일 때")
    print("=" * 50 + "\n")
    print("상황: 어떤 단계에서 두 모델의 결과가 모든 seed 에서 동일했다.")
    print("      (둘 다 성공 3, 둘 다 실패 97, 불일치 0)\n")
    print("질문: 올바른 진술은?\n")
    print("보기:")
    print("  A) 두 모델은 이 단계에서 차이가 없다")
    print("  B) 이 N 으로는 차이에 대한 정보가 없다")
    print("  C) 짝지은 비교가 불가능하므로 독립 비교로 전환한다")
    print("  D) 표본이 오염됐으므로 재측정한다")


def problem3_overclaim():
    print("\n" + "=" * 50)
    print("문제 3: 과잉 주장 판별 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 아래 문장들을 블로그 초고에 썼다.")
    print("  (a) 'sim 에서 12/100 을 달성해 실로봇 배포 가능성을 확인했다'")
    print("  (b) 'LoRA 는 소규모 sim 데이터로도 효과가 있다'")
    print("  (c) 'zero-shot 0/100 (상한 3.6%), fine-tuned 12/100 (6.4-20.0%)")
    print("       으로 구간이 겹치지 않았다'")
    print("  (d) 'PickCube 단일 task 에서 관측된 결과이며 다른 task 로의 일반화는")
    print("       확인하지 않았다'\n")
    print("질문: 각 문장이 과잉 주장인지 판정하고, 과잉인 것은 고쳐 쓰라.")


def problem4_blog_structure():
    print("\n" + "=" * 50)
    print("문제 4: 블로그를 배제 중심으로 쓰는 이유")
    print("=" * 50 + "\n")
    print("질문: 성공률 숫자를 중심에 두지 않고 '무엇을 배제했는가' 를 축으로")
    print("      구성하는 가장 큰 이점은?\n")
    print("보기:")
    print("  A) 분량을 늘리기 쉽다")
    print("  B) 결과가 어느 쪽으로 나와도 같은 구조로 쓸 수 있다")
    print("  C) 수치를 적지 않아도 되므로 검증 부담이 줄어든다")
    print("  D) 검색 유입에 유리하다")


def problem5_next_phase():
    print("\n" + "=" * 50)
    print("문제 5: 배제 표가 Phase 7 에서 쓰이는 방식 (서술형)")
    print("=" * 50 + "\n")
    print("상황: Phase 7 에서 같은 파이프라인을 자작 팔의 real 데이터로 확장한다.\n")
    print("질문: (1) 이번 Phase 에서 배제한 후보들이 real 로 옮기면 어떻게 되는가")
    print("      (2) 그래서 배제 표가 Phase 7 에서 어떤 문서로 기능하는가")
    print("      (3) '남은 후보' 표의 세 번째 칸(확인하려면 필요한 것)이 있는 이유")


def problem6_incomplete_checklist():
    print("\n" + "=" * 50)
    print("문제 6: 닫히지 않은 완료 항목")
    print("=" * 50 + "\n")
    print("상황: Roadmap 완료 체크리스트 중 한 항목의 근거를 만들지 못했다.")
    print("      Phase 4.5 를 종료하려는 시점이다.\n")
    print("질문: 올바른 처리는?\n")
    print("보기:")
    print("  A) 다른 항목들이 통과했으므로 체크하고 종료한다")
    print("  B) 체크하지 않고 미완 사유를 적어 남긴다")
    print("  C) 항목 자체를 체크리스트에서 삭제한다")
    print("  D) 해당 항목을 Phase 6 으로 옮겨 적는다")


if __name__ == "__main__":
    print("=" * 50)
    problem1_exclusion_table()
    problem2_discordant_zero()
    problem3_overclaim()
    problem4_blog_structure()
    problem5_next_phase()
    problem6_incomplete_checklist()
    print("=" * 50)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("=" * 50)
