"""
Phase 4.5 Week 1 - 데이터 수집 기초 퀴즈
"""


def problem1_expert_source():
    print("\n" + "=" * 50)
    print("문제 1: 정답 action 을 만드는 주체")
    print("=" * 50 + "\n")
    print("질문: LoRA 학습 데이터의 정답 action 을 zero-shot OpenVLA 의 출력으로")
    print("      쓰면 안 되는 이유는?\n")
    print("보기:")
    print("  A) 추론 속도가 느려 수집 시간이 오래 걸린다")
    print("  B) week0 에서 성공률이 바닥이라고 측정했으므로 실패를 학습하게 된다")
    print("  C) OpenVLA 의 출력은 정규화돼 있어 저장할 수 없다")
    print("  D) 라이선스상 자기 출력을 학습에 쓸 수 없다")


def problem2_direction():
    print("\n" + "=" * 50)
    print("문제 2: 변환의 방향")
    print("=" * 50 + "\n")
    print("질문: 학습 데이터의 action 을 OpenVLA 표현으로 바꿔 저장하는 이유는?\n")
    print("보기:")
    print("  A) OpenVLA 표현이 저장 용량이 더 작다")
    print("  B) 정답은 모델이 내놓아야 하는 값이므로 모델의 출력 규약과 같아야 한다")
    print("  C) ManiSkill 표현은 버전마다 바뀌어 보존성이 없다")
    print("  D) RLDS 포맷이 OpenVLA 표현만 허용한다")


def problem3_seed_split():
    print("\n" + "=" * 50)
    print("문제 3: eval seed 예약")
    print("=" * 50 + "\n")
    print("질문: week0 이 baseline 측정에 쓴 seed 로 학습 데이터를 모으면")
    print("      무엇을 구분할 수 없게 되는가?\n")
    print("보기:")
    print("  A) 학습 손실과 검증 손실")
    print("  B) adaptation 의 효과와 학습한 초기 배치를 암기한 효과")
    print("  C) expert 의 성공률과 모델의 성공률")
    print("  D) 위치 델타와 회전 델타")


def problem4_failed_episodes():
    print("\n" + "=" * 50)
    print("문제 4: 실패한 expert episode 의 처리")
    print("=" * 50 + "\n")
    print("질문: expert 가 실패한 episode 를 어떻게 다루는가?\n")
    print("보기:")
    print("  A) 학습에 넣고 통계에서도 제외한다")
    print("  B) 학습에 넣지 않고 통계에서도 제외한다")
    print("  C) 학습에 넣지 않되 expert 성공률 통계로는 남긴다")
    print("  D) 성공으로 라벨을 바꿔 학습에 넣는다")


def problem5_seed_in_file():
    print("\n" + "=" * 50)
    print("문제 5: 데이터 파일에 seed 를 박아 두는 이유")
    print("=" * 50 + "\n")
    print("질문: 각 episode 파일에 seed 를 함께 저장하는 이유로 가장 적절한 것은?\n")
    print("보기:")
    print("  A) seed 가 없으면 npz 파일을 열 수 없다")
    print("  B) 학습 시 seed 를 입력으로 함께 넣어야 한다")
    print("  C) 나중에 eval seed 오염 검사와 episode 재현을 코드로 할 수 있다")
    print("  D) seed 로 파일 순서를 정렬해야 학습이 수렴한다")


if __name__ == "__main__":
    print("=" * 50)
    problem1_expert_source()
    problem2_direction()
    problem3_seed_split()
    problem4_failed_episodes()
    problem5_seed_in_file()
    print("=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
