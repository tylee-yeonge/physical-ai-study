"""
Phase 4.5 Week 4 - 검증 기준 + 변인 통제 퀴즈

서술형 문항은 답을 먼저 종이나 파일에 쓴 뒤 solution 과 대조한다.
"""


def problem1_four_layers():
    print("\n" + "=" * 50)
    print("문제 1: 4층 검증이 각각 배제하는 것 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 이번 주 통과 기준은 (1) OOM 없이 적재 (2) VRAM 이 baseline 과")
    print("      근사 일치 (3) 출력 대역이 학습 통계 범위 안 (4) 결정성 이다.\n")
    print("질문: 각 층이 배제하는 실패 원인을 하나씩 짝지어 설명하고,")
    print("      어느 층이 없으면 '조용히 틀린 모델' 을 통과시키게 되는지 답하라.")


def problem2_same_output_two_keys():
    print("\n" + "=" * 50)
    print("문제 2: 두 키의 출력이 같을 때")
    print("=" * 50 + "\n")
    print("상황: 같은 이미지에 unnorm_key 를 'bridge_orig' 와 내 데이터셋 이름으로")
    print("      각각 주고 추론했더니 출력이 완전히 동일했다. 예외는 없다.\n")
    print("질문: 가장 그럴듯한 원인은?\n")
    print("보기:")
    print("  A) 두 데이터셋의 통계가 우연히 같다")
    print("  B) 키가 실제로 적용되지 않았다 (주입 위치·구조 문제)")
    print("  C) 4-bit 양자화가 통계 차이를 지웠다")
    print("  D) 모델이 학습으로 정규화를 내재화해 키가 무의미해졌다")


def problem3_vram_larger():
    print("\n" + "=" * 50)
    print("문제 3: VRAM 이 baseline 보다 크게 나올 때 (서술형)")
    print("=" * 50 + "\n")
    print("상황: fine-tuned 모델 적재 후 메모리가 약 14GB 로 나왔다.")
    print("      week0 baseline 은 약 4.4GB 였다. OOM 은 나지 않았다.\n")
    print("질문: (1) 가장 먼저 의심할 것과 그 근거")
    print("      (2) 이 상태로 week5 측정을 진행하면 무엇이 문제가 되는가")


def problem4_version_mismatch():
    print("\n" + "=" * 50)
    print("문제 4: 로컬에서 체크포인트가 안 열릴 때")
    print("=" * 50 + "\n")
    print("상황: 학습 컨테이너와 로컬 venv 의 라이브러리 조합이 다르고,")
    print("      로컬에서 체크포인트 로드가 실패한다.\n")
    print("질문: '로컬 조합을 학습 환경에 맞춰 올린다' 를 기본 선택으로 두면")
    print("      안 되는 이유는?\n")
    print("보기:")
    print("  A) 로컬 GPU 가 새 버전을 지원하지 않는다")
    print("  B) 그 조합 위에서 재현되는 기존 실측(Block 1-3, week0 baseline)이 깨진다")
    print("  C) 라이선스상 버전을 바꿀 수 없다")
    print("  D) 업그레이드는 되돌릴 수 없다")


def problem5_what_may_change():
    print("\n" + "=" * 50)
    print("문제 5: week5 에서 바뀌어도 되는 것 (서술형)")
    print("=" * 50 + "\n")
    print("상황: week5 는 zero-shot 과 fine-tuned 를 동일 조건 N회 비교한다.\n")
    print("질문: (1) 두 측정 사이에 바뀌어도 되는 것은 정확히 몇 개이며 무엇인가")
    print("      (2) 고정해야 하는 항목을 5개 이상 들라")
    print("      (3) 고정이 깨지면 Phase 4.5 의 어떤 주장이 성립하지 않는가")


def problem6_smoke_success():
    print("\n" + "=" * 50)
    print("문제 6: smoke test 에서 성공이 나왔다면")
    print("=" * 50 + "\n")
    print("상황: 1 episode smoke test 에서 큐브를 집는 데 성공했다.\n")
    print("질문: 이때 취해야 할 태도는?\n")
    print("보기:")
    print("  A) fine-tuning 이 효과가 있었다고 판단하고 week5 를 간소화한다")
    print("  B) 루프가 동작한다는 것만 기록하고 성공률 판단은 week5 로 넘긴다")
    print("  C) 같은 seed 로 몇 번 더 돌려 성공률을 추정한다")
    print("  D) zero-shot 도 같은 seed 로 1회 돌려 즉시 비교한다")


if __name__ == "__main__":
    print("=" * 50)
    problem1_four_layers()
    problem2_same_output_two_keys()
    problem3_vram_larger()
    problem4_version_mismatch()
    problem5_what_may_change()
    problem6_smoke_success()
    print("=" * 50)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("=" * 50)
