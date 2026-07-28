"""
Phase 4.5 Week 0 - action 변환 계약 + 진단 논리 퀴즈

서술형 문항은 답을 먼저 종이나 파일에 쓴 뒤 solution 과 대조한다.
"""


def problem1_remaining_suspect():
    print("\n" + "=" * 50)
    print("문제 1: 상한 대조를 통과한 뒤 남는 용의자")
    print("=" * 50 + "\n")
    print("상황:")
    print("  - 기성 해법으로 상한 대조 -> 18/20 성공")
    print("  - 같은 env / 같은 step cap / 같은 seed 목록으로 OpenVLA -> 0/20")
    print("  - reached 도 0/20\n")
    print("질문: 이 시점에 아직 배제되지 않은 용의자와, 다음에 확인할 것은?\n")
    print("보기:")
    print("  A) env 설정 문제. control_mode 를 바꿔 다시 측정한다")
    print("  B) 성공 판정 문제. success 임계값을 완화한다")
    print("  C) action 변환 레이어. 단위/프레임/부호 계약 표로 돌아간다")
    print("  D) 도메인 갭. 정상 결과이므로 baseline 으로 기록한다")


def problem2_rotation_silent_bug():
    print("\n" + "=" * 50)
    print("문제 2: 회전 표현 오매핑이 조용히 지나가는 이유 (서술형)")
    print("=" * 50 + "\n")
    print("상황: OpenVLA 출력의 회전 3차원을 ManiSkill 에 그대로 넣었다.")
    print("      팔은 부드럽게 움직이고 예외도 나지 않는다.\n")
    print("질문: 그래도 회전 매핑이 틀렸을 수 있는 이유를 설명하라.")
    print("      (힌트: 두 표현이 언제 서로 비슷해지는가)")


def problem3_scale_mismatch():
    print("\n" + "=" * 50)
    print("문제 3: 범위 규약 불일치의 결과")
    print("=" * 50 + "\n")
    print("상황:")
    print("  - env.action_space.low  = [-1, -1, -1, -1, -1, -1, -1]")
    print("  - env.action_space.high = [ 1,  1,  1,  1,  1,  1,  1]")
    print("  - OpenVLA 의 un-normalize 된 위치 델타는 대략 0.01 대역\n")
    print("질문: 변환 없이 그대로 env.step() 에 넣으면 어떤 일이 생기는가?\n")
    print("보기:")
    print("  A) 값이 범위를 벗어나 예외가 발생한다")
    print("  B) 의도한 이동량보다 훨씬 작게 움직여 거의 정지한 것처럼 보인다")
    print("  C) 의도한 이동량보다 훨씬 크게 움직여 팔이 튄다")
    print("  D) 스케일이 자동 보정되어 정상 동작한다")


def problem4_norm_mask():
    print("\n" + "=" * 50)
    print("문제 4: 통계에 담긴 차원별 마스크 (서술형)")
    print("=" * 50 + "\n")
    print("상황: unnorm_key 의 action 통계를 열어 보니 분위수 배열과 함께")
    print("      길이 7 의 boolean 배열이 들어 있고, 마지막 원소만 다른 값이다.\n")
    print("질문: (1) 그 boolean 배열의 역할은 무엇인가")
    print("      (2) 마지막 차원이 다르게 취급되는 이유는 무엇인가")
    print("      (3) 이것을 놓치면 로봇 동작에 어떤 증상이 나타나는가")


def problem5_floor_response():
    print("\n" + "=" * 50)
    print("문제 5: 부분 도달률까지 0 일 때의 대응")
    print("=" * 50 + "\n")
    print("상황: 하네스 검증은 통과했고, 변환 계약 표도 출처와 함께 채웠다.")
    print("      그런데 reached 조차 0/20 이다.\n")
    print("질문: Roadmap 이 지시하는 다음 조정 대상은?\n")
    print("보기:")
    print("  A) task 를 더 쉬운 변형으로 바꿔 zero-shot 성공률을 올린다")
    print("  B) N 을 20 에서 50 으로 늘려 신호를 찾는다")
    print("  C) 환경의 embodiment / 카메라 규약 정합을 손본다")
    print("  D) step cap 을 100 에서 300 으로 늘린다")


def problem6_env_conflict():
    print("\n" + "=" * 50)
    print("문제 6: venv 통합 판단 (서술형)")
    print("=" * 50 + "\n")
    print("상황: .venv-vla 에서 `pip install --dry-run mani_skill` 을 실행했더니")
    print("      would install 목록에 transformers 와 timm 이 다른 버전으로 나온다.\n")
    print("질문: (1) 합치면 안 되는 이유를 리포의 어떤 자산을 근거로 설명하는가")
    print("      (2) 합치지 않을 경우 실습 6 의 구조는 어떻게 바뀌는가")


if __name__ == "__main__":
    print("=" * 50)
    problem1_remaining_suspect()
    problem2_rotation_silent_bug()
    problem3_scale_mismatch()
    problem4_norm_mask()
    problem5_floor_response()
    problem6_env_conflict()
    print("=" * 50)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("=" * 50)
