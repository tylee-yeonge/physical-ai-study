"""
Phase 4.5 Week 5 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) 같은 상수·같은 코드에서 나온다는 보장")
    print("=" * 50 + "\n")
    print("해설:")
    print("  이 주차의 실패 형태는 하나다 -- 모르는 사이에 변인이 하나 더 바뀌는 것.")
    print("  스크립트를 복사하면 전처리, 리사이즈, 4-bit 설정, step cap 중 하나가")
    print("  한쪽에서만 수정될 여지가 생긴다. 그리고 그 차이는 결과를 보고 나서는")
    print("  발견하기 어렵다.")
    print()
    print("  모델을 인자로 받으면 '바뀐 것은 인자뿐' 이 코드 구조로 보장된다.")
    print("  무결성 검사(실습 5-1)에서 메타 차이가 model/unnorm_key 둘뿐인지")
    print("  확인하는 것도 같은 목적이다.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: A) 구간 폭이 0 이 된다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  정규 근사의 표준오차는 p(1-p)/N 의 제곱근에 비례한다.")
    print("  p=0 이면 이 값이 0 이므로 구간이 한 점으로 붕괴한다.")
    print()
    print("  '0% ± 0%' 는 20회 관측으로 참 성공률이 정확히 0 이라고 주장하는 것이고,")
    print("  명백히 과한 주장이다. p=1 에서도 같은 문제가 생긴다.")
    print()
    print("  우리 상황에서 before 쪽이 0/N 일 가능성이 높으므로(week0 §7)")
    print("  이 문제를 우회할 수 없다. 경계에서도 성립하는 구간 추정을 써야 한다.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: C) 한쪽만 성공한 seed")
    print("=" * 50 + "\n")
    print("해설:")
    print("  같은 seed 는 같은 초기 배치를 뜻한다. 그 배치에서")
    print("    둘 다 성공 -> 두 모델이 같은 결과. 차이에 대한 정보 없음")
    print("    둘 다 실패 -> 같음. 정보 없음")
    print("    한쪽만 성공 -> 이 배치에서 두 모델이 갈렸다. 정보 있음")
    print()
    print("  즉 판단의 근거는 불일치 쌍의 개수와 방향이다.")
    print("  개선 방향 불일치가 악화 방향보다 뚜렷이 많으면 개선의 근거가 된다.")
    print()
    print("  이 정보를 쓰려면 seed 별로 결과를 저장해야 한다 -- 집계값만 남기면")
    print("  어느 seed 에서 갈렸는지가 사라진다.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) 짝지은 비교")
    print("=" * 50 + "\n")
    print("해설:")
    print("  성공률과 신뢰구간은 성공 횟수와 N 만 있으면 계산된다.")
    print("  계산할 수 없게 되는 것은 짝지은 비교다 -- 어느 seed 에서 두 모델이")
    print("  갈렸는지를 알아야 불일치 쌍을 셀 수 있다.")
    print()
    print("  짝지음을 잃으면 독립 두 집단 비교로 후퇴하게 되고,")
    print("  task 난이도 편차가 잡음으로 들어와 검정력이 떨어진다.")
    print()
    print("  seed 단위 저장에는 두 번째 이유도 있다 -- N=100 이면 실행이 한 시간")
    print("  단위이므로, 중단됐을 때 그때까지의 결과가 남아야 한다.")


def problem5_solution():
    print("\n" + "=" * 50)
    print("문제 5 정답: B) 방법이 결과에 맞춰진다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  결과를 먼저 보면, 원하는 결론이 나오는 방법을 (의도하지 않아도) 고르게 된다.")
    print("  구간 추정 방법, 유의 수준, 어느 지표를 주 지표로 볼지 -- 모두")
    print("  결과를 알고 나면 선택이 오염된다.")
    print()
    print("  그래서 실습 3 이 실습 4-5 보다 앞에 있고, 보고 문장의 틀까지")
    print("  미리 만들어 둔다 (숫자만 나중에 채운다).")
    print()
    print("  같은 이유로 week5 는 집계를 하지 않고 원시 결과만 넘긴다.")
    print("  해석은 week6 의 일이다.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    print("=" * 50)
