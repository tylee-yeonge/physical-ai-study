"""
Phase 4.5 Week 4 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) base 가 이미 로컬 캐시에 있다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  학습 스크립트는 체크포인트 저장 때 어댑터를 base 에 머지해 15GB 로 남긴다.")
    print("  그 파일을 내리는 대신 같은 합(base + 어댑터)을 로컬에서 다시 한다 --")
    print("  base 15GB 는 로컬 HuggingFace 캐시에 있고 어댑터는 462MB 로 회수했다.")
    print()
    print("  단 재머지는 학습과 같은 버전 조합(openvla-train:v2)에서 한다.")
    print("  다른 조합에서 머지하면 '학습이 저장했을 모델과 같다' 는 근거가 약해진다.")
    print("  같은 모델이 만들어졌는지는 회수한 색인과의 대조로 확인한다 (실습 1).")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) 양자화가 실제로 적용됐는지")
    print("=" * 50 + "\n")
    print("해설:")
    print("  fine-tuned 모델은 base 와 구조가 같고 가중치 값만 다르다.")
    print("  따라서 4-bit 적재 메모리도 거의 같아야 한다.")
    print()
    print("    baseline 과 근사 일치 -> 정상")
    print("    크게 크다            -> 양자화 설정이 안 걸렸을 가능성 (약 15GB 쪽)")
    print("    크게 작다            -> 모델이 일부만 로드됐을 가능성")
    print()
    print("  즉 이 수치는 용량 확인용이 아니라 양자화 적용 여부의 판정 지표다.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) 오류 없이 남의 로봇 스케일로 역정규화된다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  unnorm_key 는 '어느 로봇의 통계로 되돌릴지' 고르는 스위치다.")
    print("  옛 키가 모델에 남아 있으면 그 키로 역정규화가 정상 수행된다.")
    print("  예외도 없고 값도 그럴싸하지만 스케일이 전부 다르다.")
    print()
    print("  키가 아예 없으면 예외가 나서 바로 알 수 있다 -- 그 경우가 오히려 안전하다.")
    print("  위험한 것은 조용히 동작하는 경우다.")
    print()
    print("  판정 방법: 두 키로 각각 추론해 대역을 비교하고, 내 키의 출력이")
    print("  학습 데이터 통계 범위 안인지 확인한다 (실습 3).")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) 양자화 효과와 adaptation 효과가 섞인다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  before/after 비교의 전제는 '변인이 모델 하나' 다.")
    print("  정밀도가 다르면 성공률 차이에 양자화 손실이 섞여 들어가고,")
    print("  '올랐다/안 올랐다' 의 원인을 adaptation 으로 귀속할 수 없다.")
    print()
    print("  week0 baseline 을 4-bit 로 측정해 둔 것이 여기서 값을 한다.")
    print("  이번 주 할 일은 조건이 실제로 같은지 확인하는 것이다 --")
    print("  같은 nf4, 같은 double quant 여부, 같은 compute dtype.")
    print()
    print("  A 도 사실이긴 하다 (fp16 7B 는 약 15GB 로 4070 에 안 올라간다).")
    print("  다만 문제의 핵심은 변인 통제다.")


def problem5_solution():
    print("\n" + "=" * 50)
    print("문제 5 정답: B) 표본 1개의 인상이 week5 해석을 오염시킨다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  smoke test 의 목적은 '모델이 루프에 꽂히는가' 하나다.")
    print("  1 episode 성공/실패는 성공률에 대해 아무것도 말해 주지 않는다.")
    print("  (week0 에서 N=20 의 신뢰구간이 약 +-22%p 라고 이미 적어 뒀다)")
    print()
    print("  그런데 사람은 첫 결과의 인상을 버리지 못한다. '됐다' 를 본 뒤에는")
    print("  week5 의 낮은 성공률을 우연으로 해석하고, '안 됐다' 를 본 뒤에는")
    print("  높은 성공률을 의심한다. 경계를 지키는 것이 해석을 지키는 것이다.")
    print()
    print("  week4 는 동작 확인, week5 는 측정. 역할을 섞지 않는다.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    print("=" * 50)
