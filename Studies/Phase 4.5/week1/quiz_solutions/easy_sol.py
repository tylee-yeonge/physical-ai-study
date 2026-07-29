"""
Phase 4.5 Week 1 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) 실패를 학습하게 된다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  지도학습의 라벨은 '모델이 따라야 할 값' 이다.")
    print("  week0 baseline 에서 zero-shot 성공률이 바닥이라고 이미 측정했으므로,")
    print("  그 출력을 정답으로 쓰면 실패하는 궤적을 정답으로 굳히게 된다.")
    print()
    print("  채택된 대안은 week0 실습 5 의 기성 해법(motion planning / scripted)이다.")
    print("  상한 대조에서 성공률이 높다는 것도 그때 이미 확인했다.")
    print("  -> 하네스 검증의 부산물이 데이터 생성기다.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) 모델의 출력 규약과 같아야 한다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  모델은 학습에서 본 형식으로 출력하도록 배운다.")
    print("  expert 가 만든 값을 그 형식 그대로 저장하면, 모델은 자기가 낼 수 없는")
    print("  표현을 정답으로 배우게 된다.")
    print()
    print("  그래서 week0 과 week1 의 변환 방향이 반대다:")
    print("    week0: OpenVLA 출력 -> ManiSkill action   (정변환)")
    print("    week1: expert action -> OpenVLA 표현       (역변환)")
    print("  계약 표는 하나이고 양방향으로 쓴다. 두 곳에 따로 쓰면 반드시 어긋난다.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) adaptation 효과와 암기 효과")
    print("=" * 50 + "\n")
    print("해설:")
    print("  eval seed 는 초기 큐브 위치와 목표를 결정한다.")
    print("  그 seed 로 학습하면 fine-tuned 모델은 자기가 학습한 배치에서 평가받는다.")
    print("  성공률이 올라가도 '적응했다' 와 '외웠다' 를 구분할 수 없다.")
    print()
    print("  Phase 4.5 의 성공 기준이 before/after 정량 비교이므로,")
    print("  이 구분 불가는 산출물 전체를 무효로 만든다.")
    print("  -> week0 이 쓴 seed 목록은 eval 전용으로 예약된 상태다.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: C) 학습 제외 + 통계로는 남긴다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  학습에서 빼는 이유: expert 의 실패 궤적은 정답이 아니다.")
    print("  통계로 남기는 이유: expert 성공률은 데이터 품질의 지표이고,")
    print("  나중에 'LoRA 가 안 올랐다' 의 원인 후보를 좁힐 때 쓰인다.")
    print("  (expert 성공률이 애초에 낮았다면 라벨 자체가 얇았다는 뜻이다)")
    print()
    print("  D 가 위험한 이유: 라벨을 조작하면 학습은 돌지만 결과 해석이 불가능해진다.")


def problem5_solution():
    print("\n" + "=" * 50)
    print("문제 5 정답: C) 오염 검사와 재현을 코드로 하기 위해")
    print("=" * 50 + "\n")
    print("해설:")
    print("  seed 를 데이터에 박아 두면 두 가지가 가능해진다:")
    print("    1) eval seed 오염 검사 -- 파일에서 seed 를 읽어 교집합을 확인")
    print("    2) 특정 episode 재현 -- 같은 seed 로 reset 하면 같은 초기 배치")
    print()
    print("  메타 파일에만 적어 두면 파일이 섞이거나 일부만 옮겨질 때 대응이 끊긴다.")
    print("  검사를 사람 눈이 아니라 코드가 하도록 만드는 것이 요점이다.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    print("=" * 50)
