"""
Phase 4.5 Week 1 - 표현 정합 + 분포 논증 퀴즈

서술형 문항은 답을 먼저 종이나 파일에 쓴 뒤 solution 과 대조한다.
"""


def problem1_roundtrip_verdict():
    print("\n" + "=" * 50)
    print("문제 1: round-trip 결과의 해석")
    print("=" * 50 + "\n")
    print("상황:")
    print("  - 강한 기준(스텝별 pose 오차 < 1e-3): 실패 (max 오차 0.02 m)")
    print("  - 약한 기준(재생 episode 도 success): 통과\n")
    print("질문: 이 조합의 의미와 다음 행동은?\n")
    print("보기:")
    print("  A) task 는 달성했으므로 변환은 충분하다. 본 수집으로 진행한다")
    print("  B) 정보를 잃고 있다. 회전 표현·프레임을 의심해 역변환으로 복귀한다")
    print("  C) 강한 기준의 허용 오차가 너무 엄격하다. 0.05 로 올리고 통과 처리한다")
    print("  D) env 의 성공 판정이 느슨하다. success 임계값을 조인다")


def problem2_joint_to_eef():
    print("\n" + "=" * 50)
    print("문제 2: 관절 공간 expert 에서 EEF delta 만들기 (서술형)")
    print("=" * 50 + "\n")
    print("상황: expert(motion planning) 는 관절 위치 명령으로 팔을 움직인다.")
    print("      그런데 학습 라벨은 EEF delta 표현이어야 한다.\n")
    print("질문: expert 의 원본 action 을 직접 변환하지 않고도")
    print("      EEF delta 를 얻는 방법과, 그 방법이 제어 모드에 무관한 이유를 설명하라.")


def problem3_embodiment_layer():
    print("\n" + "=" * 50)
    print("문제 3: 미학습 분포 논증의 embodiment 층 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 미학습 분포 점검을 '겹치지 않는다' 한 문장으로 쓰려 한다.\n")
    print("질문: (1) embodiment 층에서 '겹칠 수 있다' 로 써야 하는 이유")
    print("      (2) 그런데도 adaptation 이 의미를 갖는 근거는 어느 층에서 나오는가")


def problem4_indirect_evidence():
    print("\n" + "=" * 50)
    print("문제 4: zero-shot 성적을 분포 증거로 쓰는 조건")
    print("=" * 50 + "\n")
    print("질문: 'zero-shot 성공률이 낮으니 미학습 분포다' 라고 주장하려면")
    print("      어떤 선행 조건이 필요한가?\n")
    print("보기:")
    print("  A) N 이 50 이상이어야 한다")
    print("  B) week0 의 하네스 검증이 통과했어야 한다")
    print("  C) 같은 task 를 다른 sim 에서도 측정했어야 한다")
    print("  D) 사전학습 데이터셋 목록을 전부 다운로드해 봤어야 한다")


def problem5_expert_only_limit():
    print("\n" + "=" * 50)
    print("문제 5: expert 궤적만 학습한 모델의 실패 양상 (서술형)")
    print("=" * 50 + "\n")
    print("상황: 성공한 expert episode 만 모아 LoRA 를 학습시켰다.\n")
    print("질문: (1) 이 데이터의 상태 분포가 가진 구조적 한계는 무엇인가")
    print("      (2) 추론 중 그 한계가 어떤 양상으로 드러나는가")
    print("      (3) 이번 Phase 에서 이것을 해결하지 않는다면 무엇으로 대신하는가")


def problem6_gripper_histogram():
    print("\n" + "=" * 50)
    print("문제 6: 히스토그램에서 드러나는 변환 오류")
    print("=" * 50 + "\n")
    print("상황: 수집 데이터의 action 히스토그램을 보니 gripper 차원(dim6)의 값이")
    print("      두 값 근처가 아니라 넓은 구간에 고르게 퍼져 있다.\n")
    print("질문: 가장 먼저 의심할 것은?\n")
    print("보기:")
    print("  A) 학습 데이터가 부족해 분포가 안 잡혔다")
    print("  B) expert 성공률이 낮아 궤적이 불안정하다")
    print("  C) gripper 성분을 잘못 뽑았거나 정규화 대상에서 제외해야 할 차원을 스케일링했다")
    print("  D) 히스토그램 bin 수가 너무 많다")


if __name__ == "__main__":
    print("=" * 50)
    problem1_roundtrip_verdict()
    problem2_joint_to_eef()
    problem3_embodiment_layer()
    problem4_indirect_evidence()
    problem5_expert_only_limit()
    problem6_gripper_histogram()
    print("=" * 50)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("=" * 50)
