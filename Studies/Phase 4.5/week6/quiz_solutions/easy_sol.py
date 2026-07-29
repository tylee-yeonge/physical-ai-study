"""
Phase 4.5 Week 6 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) 미리 정한 값을 쓰고, 바꾸면 사실과 이유를 함께 적는다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  유의수준·단측/양측·구간 추정 방법은 week5 실습 3 에서 결과를 보기 전에")
    print("  정해 stat_method.md 에 적어 뒀다. 결과를 보고 나서 바꾸면 그 선택이")
    print("  결과에 맞춰지고, 분석이 사후 정당화가 된다.")
    print()
    print("  C 가 최악이다. 두 값으로 계산해 유리한 쪽을 고르는 것은 결과를 고른 것이다.")
    print()
    print("  다만 바꾸는 것 자체가 금지는 아니다. 바꿔야 할 이유가 생겼으면")
    print("  바꾸고, **바꾼 사실과 이유를 결과와 함께** 적는다. 감추는 것보다 낫다.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) 성립한다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Roadmap 의 성공 기준은 '성공률이 올랐다' 가 아니라")
    print("  'adaptation 파이프라인을 설계-실행하고 결과를 정량 분석했다' 다.")
    print()
    print("  소규모 데이터 LoRA 가 성공률을 못 올리는 것은 예상된 결과 중 하나이고,")
    print("  그 경우 '왜 안 올랐는가' 의 분석 자체가 산출물이 된다.")
    print()
    print("  A 가 위험한 이유: 결과가 나올 때까지 재학습을 반복하면 그것은")
    print("  분석이 아니라 결과 사냥이 된다. 그리고 예산도 없다.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) 누구나 쓸 수 있고 후보를 좁히지 못한다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  '데이터가 적어서' 류의 나열은 이 프로젝트를 하지 않은 사람도 쓸 수 있다.")
    print("  즉 수행의 증거가 되지 못한다.")
    print()
    print("  이 프로젝트가 대신 할 수 있는 진술은 배제다:")
    print("    통합 버그 아니다      <- week0 하네스 검증")
    print("    라벨 변환 손실 아니다  <- week1 round-trip")
    print("    데이터 연결 아니다    <- week2 로드 검증")
    print("    키 오연결 아니다      <- week4 값 대역 검사")
    print("    조건 누출 아니다      <- week5 무결성 검사")
    print()
    print("  '무엇이 원인인지 모른다' 와 '무엇이 원인이 아닌지는 안다' 는 다른 진술이고,")
    print("  후자가 엔지니어링 증거다.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) 하한 0, 상한은 0 이 아닌 값")
    print("=" * 50 + "\n")
    print("해설:")
    print("  0/N 은 '참 성공률이 0' 을 뜻하지 않는다. 100회 관측으로 말할 수 있는 것은")
    print("  '참 성공률이 어느 값보다 높지는 않다' 다. 즉 상한이 정보다.")
    print()
    print("  예: 0/100 의 95% 구간은 대략 [0%, 3.6%] 다.")
    print("  정규 근사로 계산하면 폭이 0 이 되어 이 정보를 잃는다 (week5 §3).")
    print()
    print("  C 가 틀린 이유: 계산이 불가능한 것이 아니라, 근사 방법이 부적절한 것이다.")
    print("  경계에서 성립하는 방법을 쓰면 정상적으로 나온다.")


def problem5_solution():
    print("\n" + "=" * 50)
    print("문제 5 정답: C) '성공률이 17%p 향상됐다'")
    print("=" * 50 + "\n")
    print("해설:")
    print("  grasped 는 부분 도달률이고 성공률(placed)이 아니다. 최종 성공은 양쪽 0 이므로")
    print("  '성공률이 향상' 은 사실과 다르다 -- 지표를 바꿔치기한 문장이다.")
    print()
    print("  A, B, D 는 모두 지표를 정확히 지칭한다. B 는 해석을 덧붙였지만")
    print("  '접근은 개선 / 최종 성공에는 이르지 못함' 으로 범위를 지켰다.")
    print()
    print("  이 미끄러짐이 흔한 이유: 부분 도달률을 도입한 목적이 '신호를 살리는 것'")
    print("  이었기 때문에, 신호가 보이면 그것을 성과로 말하고 싶어진다.")
    print("  지표 이름을 문장에 그대로 넣으면 이 실수를 막을 수 있다.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    print("=" * 50)
