"""
Phase 4.5 Week 5 - eval harness 기초 퀴즈
"""


def problem1_single_script():
    print("\n" + "=" * 50)
    print("문제 1: 스크립트를 하나로 두는 이유")
    print("=" * 50 + "\n")
    print("질문: zero-shot 용과 fine-tuned 용 스크립트를 복사해 두 개 만들지 않고")
    print("      모델을 인자로 받는 이유는?\n")
    print("보기:")
    print("  A) 파일 수가 적어 관리가 편하다")
    print("  B) 두 측정이 같은 상수와 같은 코드에서 나온다는 보장이 된다")
    print("  C) 인자로 받으면 실행 속도가 빠르다")
    print("  D) 복사본은 git 이 추적하지 못한다")


def problem2_zero_ci():
    print("\n" + "=" * 50)
    print("문제 2: 0/20 의 신뢰구간")
    print("=" * 50 + "\n")
    print("질문: 성공률 0/20 을 정규 근사로 구간 추정하면?\n")
    print("보기:")
    print("  A) 구간 폭이 0 이 되어 보고할 수 없는 값이 나온다")
    print("  B) 구간이 음수 영역까지 넓어져 해석이 어렵다")
    print("  C) 계산 자체가 불가능해 예외가 난다")
    print("  D) 정상적으로 넓은 구간이 나온다")


def problem3_paired():
    print("\n" + "=" * 50)
    print("문제 3: 짝지은 관측에서 정보를 주는 seed")
    print("=" * 50 + "\n")
    print("질문: 같은 seed 목록으로 두 모델을 측정했다. 두 모델의 차이에 대해")
    print("      정보를 주는 seed 는?\n")
    print("보기:")
    print("  A) 둘 다 성공한 seed")
    print("  B) 둘 다 실패한 seed")
    print("  C) 한쪽만 성공한 seed")
    print("  D) 모든 seed 가 같은 정보량을 준다")


def problem4_per_seed_save():
    print("\n" + "=" * 50)
    print("문제 4: seed 단위 저장")
    print("=" * 50 + "\n")
    print("질문: 성공 횟수만 저장하고 seed 별 결과를 버리면 무엇이 불가능해지는가?\n")
    print("보기:")
    print("  A) 성공률 계산")
    print("  B) 짝지은 비교")
    print("  C) 신뢰구간 계산")
    print("  D) 실행 시간 산정")


def problem5_method_first():
    print("\n" + "=" * 50)
    print("문제 5: 방법을 먼저 정하는 이유")
    print("=" * 50 + "\n")
    print("질문: 통계 방법 확정(실습 3)을 실행(실습 4-5)보다 앞에 두는 이유는?\n")
    print("보기:")
    print("  A) 방법을 정해야 스크립트가 돌아간다")
    print("  B) 결과를 본 뒤에 고르면 방법이 결과에 맞춰진다")
    print("  C) 실행 중에는 문서를 쓸 수 없다")
    print("  D) 통계 라이브러리를 미리 설치해야 한다")


if __name__ == "__main__":
    print("=" * 50)
    problem1_single_script()
    problem2_zero_ci()
    problem3_paired()
    problem4_per_seed_save()
    problem5_method_first()
    print("=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
