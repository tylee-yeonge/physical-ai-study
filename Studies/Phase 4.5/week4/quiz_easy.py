"""
Phase 4.5 Week 4 - 전송·양자화·검증 기초 퀴즈
"""


def problem1_scope():
    print("\n" + "=" * 50)
    print("문제 1: 이번 주 범위")
    print("=" * 50 + "\n")
    print("질문: 원래 계획의 '머지' 작업이 이번 주에서 빠진 이유는?\n")
    print("보기:")
    print("  A) 머지는 Section 3 으로 이관됐다")
    print("  B) 학습 스크립트가 체크포인트 저장 시 어댑터를 base 에 머지한다")
    print("  C) 4-bit 양자화가 머지를 대체한다")
    print("  D) LoRA 어댑터는 머지 없이 추론에 쓸 수 있다")


def problem2_vram_baseline():
    print("\n" + "=" * 50)
    print("문제 2: VRAM 을 baseline 과 대조하는 이유")
    print("=" * 50 + "\n")
    print("질문: 적재 메모리를 week0 baseline(약 4.4GB)과 비교하면")
    print("      '12GB 에 들어가는가' 외에 무엇을 알 수 있는가?\n")
    print("보기:")
    print("  A) 추론 속도가 얼마나 빠를지")
    print("  B) 양자화가 실제로 적용됐는지")
    print("  C) 학습이 수렴했는지")
    print("  D) 데이터셋 규모가 적절했는지")


def problem3_unnorm_switch():
    print("\n" + "=" * 50)
    print("문제 3: unnorm_key 전환")
    print("=" * 50 + "\n")
    print("질문: fine-tuned 모델에서 unnorm_key 를 내 데이터셋 통계로 바꾸지 않고")
    print("      옛 키(bridge_orig)를 그대로 쓰면?\n")
    print("보기:")
    print("  A) 즉시 예외가 발생해 바로 알 수 있다")
    print("  B) 오류 없이 동작하지만 남의 로봇 스케일로 역정규화된다")
    print("  C) action 이 정규화되지 않은 원값으로 나온다")
    print("  D) 모델 적재 자체가 실패한다")


def problem4_quant_variable():
    print("\n" + "=" * 50)
    print("문제 4: 양자화 조건 맞추기")
    print("=" * 50 + "\n")
    print("질문: before 를 fp16, after 를 4-bit 로 측정하면 무엇이 문제인가?\n")
    print("보기:")
    print("  A) fp16 이 4070 에 안 올라가 측정이 불가능하다")
    print("  B) 양자화 효과와 adaptation 효과가 섞여 원인 분리가 안 된다")
    print("  C) 두 조건의 추론 속도가 달라 시간이 더 걸린다")
    print("  D) 문제없다. 양자화는 성공률에 영향이 없다")


def problem5_smoke_boundary():
    print("\n" + "=" * 50)
    print("문제 5: smoke test 의 경계")
    print("=" * 50 + "\n")
    print("질문: 이번 주 smoke test 에서 성공/실패를 판정하지 않는 이유는?\n")
    print("보기:")
    print("  A) 1 episode 는 실행 시간이 짧아 결과가 부정확하다")
    print("  B) 표본 1개의 인상이 week5 의 해석을 오염시킨다")
    print("  C) success 플래그가 fine-tuned 모델에서는 동작하지 않는다")
    print("  D) 성공률은 sim 이 아니라 실기에서만 의미가 있다")


if __name__ == "__main__":
    print("=" * 50)
    problem1_scope()
    problem2_vram_baseline()
    problem3_unnorm_switch()
    problem4_quant_variable()
    problem5_smoke_boundary()
    print("=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
