"""
Phase 4.5 Week 2 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) 원인 후보에 '내 로더' 가 추가되는 것을 막는다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Phase 4.5 의 산출물은 '성공률 상승' 이 아니라 before/after 정량 분석이다.")
    print("  결과가 안 나왔을 때 '왜 안 나왔나' 를 설명할 수 있어야 하고,")
    print("  그러려면 원인 후보가 적어야 한다.")
    print()
    print("  자작 로더는 검증되지 않은 변인을 하나 더 만든다.")
    print("  사전학습과 논문의 파인튜닝이 모두 통과한 경로를 쓰는 것이 변인 축소다.")
    print("  같은 이유로 week0 에서 하네스 검증을 먼저 했다 -- 같은 사고방식이다.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: C) mixtures.py")
    print("=" * 50 + "\n")
    print("해설:")
    print("  세 파일의 역할 구분:")
    print("    configs.py    : 무엇이 어디 있는가 (관측 키, state/action 인코딩)")
    print("    transforms.py : 그것을 표준 형태로 어떻게 옮기는가")
    print("    mixtures.py   : 무엇을 학습에 쓸 것인가 (조합과 가중치)")
    print()
    print("  단일 데이터셋이어도 mixture 로 등록해야 학습 스크립트가 인자로 받는다.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) 원시 물리 단위 값")
    print("=" * 50 + "\n")
    print("해설:")
    print("  학습 파이프라인이 데이터셋을 훑어 통계를 계산하고, 학습 중 그 통계로")
    print("  action 을 정규화한다. 즉 정규화는 파이프라인의 몫이다.")
    print()
    print("  미리 [-1, 1] 로 맞춰 저장하면 정규화가 두 번 걸린다(이중 정규화).")
    print("  증상: 예외 없음, loss 는 내려감, 결과만 조용히 틀림.")
    print("  -> 그래서 로드 검증에서 정규화 후 범위를 눈으로 확인하는 절차가 있다.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: C) 오일러각 (RPY)")
    print("=" * 50 + "\n")
    print("해설:")
    print("  EEF 위치 인코딩의 벡터 구성은")
    print("    EEF delta XYZ (3) + RPY (3) + gripper (1) = 7")
    print("  로 정의돼 있다. 즉 회전 표현은 협상 대상이 아니라 이미 정해진 규약이다.")
    print()
    print("  week0 계약 표에서 '오일러각인가 축-각인가' 로 열어 두었던 행이")
    print("  여기서 닫힌다. 축-각으로 라벨을 만들었다면 재생성해야 한다.")
    print("  (delta 가 작으면 두 표현이 근사적으로 비슷해 조용히 틀린다)")


def problem5_solution():
    print("\n" + "=" * 50)
    print("문제 5 정답: B) 옛 통계 캐시로 정규화된다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  통계는 한 번 계산한 뒤 파일로 캐시되고, 캐시가 있으면 다시 계산하지 않는다.")
    print("  데이터를 바꿨는데 캐시가 남아 있으면 옛 분포 기준으로 정규화된다.")
    print()
    print("  이 함정은 라벨을 고친 직후에 정확히 발생한다.")
    print("  (예: 회전 표현을 RPY 로 고쳐 재생성한 다음)")
    print("  절차에 '캐시 삭제 확인' 을 넣고, 데이터셋 버전을 올리는 것도 방법이다.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    print("=" * 50)
