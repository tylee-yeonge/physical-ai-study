"""
Phase 4.5 Week 3 - 메모리 산수 + 학습 운영 퀴즈

서술형 문항은 답을 먼저 종이나 파일에 쓴 뒤 solution 과 대조한다.
"""


def problem1_27gb_floor():
    print("\n" + "=" * 50)
    print("문제 1: 27GB 하한의 정체 (서술형)")
    print("=" * 50 + "\n")
    print("상황: upstream 은 배치 16 에서 약 72GB, 배치를 줄이면 약 27GB 까지")
    print("      내려간다고 안내한다.\n")
    print("질문: (1) 72GB 에서 27GB 로 줄어드는 부분은 무엇인가")
    print("      (2) 27GB 에서 더는 안 내려가는 부분은 무엇인가")
    print("      (3) 이 구조가 '4070 불가' 를 어떻게 설명하는가")


def problem2_quantized_training():
    print("\n" + "=" * 50)
    print("문제 2: 4-bit 학습 경로를 쓰지 않는 이유")
    print("=" * 50 + "\n")
    print("상황: 학습 스크립트에 4-bit 양자화 학습 옵션이 있고, 켜면 12GB 안에")
    print("      들어올 여지가 생긴다. 그런데 쓰지 않기로 했다.\n")
    print("질문: Phase 4.5 의 산출물 정의에 비추어 가장 중요한 이유는?\n")
    print("보기:")
    print("  A) 4-bit 학습은 속도가 느려 예산을 넘긴다")
    print("  B) 비표준 경로라 '성공률이 안 올랐다' 의 원인 분리가 안 된다")
    print("  C) 4-bit 학습은 체크포인트를 저장할 수 없다")
    print("  D) bitsandbytes 가 RTX 4090 을 지원하지 않는다")


def problem3_local_pc_returns():
    print("\n" + "=" * 50)
    print("문제 3: 로컬 PC 를 계속 쓸 수 있게 됐다면 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 물리 접근 제약이 해소되어 로컬 PC(4070)를 언제든 쓸 수 있게 됐다.\n")
    print("질문: (1) RunPod 을 쓰는 이유 중 사라지는 것과 남는 것을 구분하라")
    print("      (2) 그 결과 이번 주 작업 중 범위가 줄어드는 항목은 무엇인가")


def problem4_loss_not_criterion():
    print("\n" + "=" * 50)
    print("문제 4: loss 가 매끄러운데도 통과가 아닌 경우")
    print("=" * 50 + "\n")
    print("상황: 학습이 완주했고 loss 곡선이 깨끗하게 내려갔다.\n")
    print("질문: 그런데도 이번 주를 통과로 볼 수 없는 상황은?\n")
    print("보기:")
    print("  A) 최종 loss 값이 목표치보다 높다")
    print("  B) 라벨이 일관되게 틀려 있어 모델이 틀린 라벨을 잘 맞춘 것이다")
    print("  C) 학습률이 기본값이었다")
    print("  D) rank 를 32 로 두고 비교 실험을 하지 않았다")


def problem5_checkpoint_tradeoff():
    print("\n" + "=" * 50)
    print("문제 5: 체크포인트 저장 방식의 트레이드오프 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 저장 방식으로 '최신만 덮어쓰기' 와 '시점별 보관' 중 하나를 고른다.\n")
    print("질문: (1) 각각의 이득과 대가")
    print("      (2) 어떤 상황에서 시점별 보관이 필요해지는가")
    print("      (3) 선택을 기록해야 하는 이유")


def problem6_merge_already_done():
    print("\n" + "=" * 50)
    print("문제 6: week4 의 범위")
    print("=" * 50 + "\n")
    print("상황: 학습 스크립트가 체크포인트를 저장할 때 LoRA 어댑터를 base 에")
    print("      머지해 저장한다는 것을 확인했다.\n")
    print("질문: 그렇다면 week4 에 남는 작업은?\n")
    print("보기:")
    print("  A) 머지 + 4-bit 양자화 + 호환성 검증")
    print("  B) 전송 + 4-bit 양자화 + 호환성 검증")
    print("  C) 재학습 + 머지 + 양자화")
    print("  D) 어댑터 병합만 (양자화는 Section 3)")


if __name__ == "__main__":
    print("=" * 50)
    problem1_27gb_floor()
    problem2_quantized_training()
    problem3_local_pc_returns()
    problem4_loss_not_criterion()
    problem5_checkpoint_tradeoff()
    problem6_merge_already_done()
    print("=" * 50)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("=" * 50)
