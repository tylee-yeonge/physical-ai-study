"""
Phase 4.5 Week 2 - 스키마 정합 + 정규화 계약 퀴즈

서술형 문항은 답을 먼저 종이나 파일에 쓴 뒤 solution 과 대조한다.
"""


def problem1_double_normalization():
    print("\n" + "=" * 50)
    print("문제 1: 로드 검증에서 본 좁은 범위")
    print("=" * 50 + "\n")
    print("상황: 배치를 꺼내 정규화 후 action 을 보니 앞 6차원의 범위가")
    print("      대략 [-0.01, +0.01] 이다. 예외는 없고 학습도 돈다.\n")
    print("질문: 가장 먼저 의심할 것은?\n")
    print("보기:")
    print("  A) 데이터가 적어 분포가 좁다. episode 를 더 모은다")
    print("  B) 라벨을 미리 정규화해 저장했다 (이중 정규화)")
    print("  C) 학습률이 높아 값이 눌렸다")
    print("  D) 이미지 해상도가 작아 action 이 작게 나왔다")


def problem2_gripper_sign():
    print("\n" + "=" * 50)
    print("문제 2: gripper 부호가 틀렸을 때 (서술형)")
    print("=" * 50 + "\n")
    print("상황: week1 에서 gripper 부호 규약을 반대로 저장했다고 가정한다.\n")
    print("질문: (1) 정규화 단계가 이 오류를 보정해 주는가. 그 이유는 무엇인가")
    print("      (2) 이 오류는 학습·추론에서 어떤 양상으로 드러나는가")
    print("      (3) 그래서 이번 주에 마지막으로 점검해야 하는 지점은 어디인가")


def problem3_missing_registration():
    print("\n" + "=" * 50)
    print("문제 3: 등록을 하나 빠뜨렸을 때 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 데이터는 RLDS 로 잘 변환됐다.\n")
    print("질문: 등록 3파일 중 하나만 빠졌을 때 각각 어떤 방식으로 막히는지")
    print("      파일별로 구분해 설명하라. (무엇을 못 찾는가 / 무엇을 모르는가)")


def problem4_encoding_wrong():
    print("\n" + "=" * 50)
    print("문제 4: action 인코딩 오등록의 파급")
    print("=" * 50 + "\n")
    print("상황: configs.py 에 action 인코딩을 잘못 적었다.\n")
    print("질문: 이때 함께 어긋나는 것은?\n")
    print("보기:")
    print("  A) 이미지 해상도 검사")
    print("  B) 정규화 마스크 (gripper 제외 여부)")
    print("  C) instruction 토크나이즈 방식")
    print("  D) episode 경계 판정")


def problem5_axis_angle_label():
    print("\n" + "=" * 50)
    print("문제 5: 라벨을 축-각으로 만들었다면")
    print("=" * 50 + "\n")
    print("상황: week1 에서 회전 라벨을 축-각으로 만들었고, 지금 실습 1 에서")
    print("      학습 인코딩이 RPY 를 요구한다는 것을 확인했다.\n")
    print("질문: 지금 취해야 하는 조치와 그 이유는?\n")
    print("보기:")
    print("  A) 그대로 진행한다. 델타가 작아 두 표현은 근사적으로 같다")
    print("  B) 학습 후 성공률이 낮으면 그때 돌아와 고친다")
    print("  C) week1 라벨을 RPY 로 재생성하고 RLDS 도 다시 빌드한다")
    print("  D) configs.py 의 인코딩을 축-각 계열로 바꿔 맞춘다")


def problem6_patch_record():
    print("\n" + "=" * 50)
    print("문제 6: 등록 변경을 패치로 남기는 이유 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 등록 3파일 수정은 upstream 리포에 대한 로컬 변경이다.\n")
    print("질문: (1) 이 변경을 패치 파일과 기준 커밋 해시로 남겨야 하는 이유")
    print("      (2) RLDS 데이터셋 본체는 커밋하지 않는데 이 기록은 남기는 이유의 차이")


if __name__ == "__main__":
    print("=" * 50)
    problem1_double_normalization()
    problem2_gripper_sign()
    problem3_missing_registration()
    problem4_encoding_wrong()
    problem5_axis_angle_label()
    problem6_patch_record()
    print("=" * 50)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("=" * 50)
