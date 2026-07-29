"""
Phase 4.5 Week 1 - 표현 정합 + 분포 논증 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) 정보를 잃고 있다 -> 역변환으로 복귀")
    print("=" * 50 + "\n")
    print("해설:")
    print("  약한 기준만 통과한다는 것은 '궤적이 달라도 task 는 됐다' 는 뜻이다.")
    print("  PickCube 는 목표 반경이 있어 궤적이 어긋나도 성공할 여지가 있으므로,")
    print("  success 통과는 변환 정합의 증거가 되지 못한다.")
    print()
    print("  0.02 m 오차는 그리퍼 개구 여유 수준의 크기다. 이 정도가 라벨에 계통으로")
    print("  섞이면 모델은 '조금씩 틀린 정답' 을 학습한다.")
    print("  -> 나중에 '성공률이 안 올랐다' 의 원인 후보가 하나 늘어난다.")
    print("     지금 잡는 것이 학습 사이클을 한 번 더 도는 것보다 훨씬 싸다.")
    print()
    print("  먼저 의심할 것: 회전 표현(오일러각 vs 축-각)과 기준 프레임(base vs EEF).")
    print("  위치 오차만 크고 방향이 맞는 경우라면 스케일(계약 표 1번 행)을 본다.")
    print()
    print("  C 가 최악이다. 기준을 결과에 맞춰 늘리면 검증이 아니라 통과 의식이 된다.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 해설: TCP pose 궤적의 차분을 쓴다")
    print("=" * 50 + "\n")
    print("답의 뼈대:")
    print("  방법: 매 스텝의 end-effector(TCP) pose 를 기록하고, 연속한 두 pose 의")
    print("        차이를 계산한다. 위치는 좌표 차분, 회전은 상대 회전으로 구한다.")
    print("        그 결과를 계약 표가 정한 표현·단위로 옮기면 EEF delta 라벨이 된다.")
    print()
    print("  제어 모드에 무관한 이유: 관절 명령이든 EEF 명령이든, 실행되고 나면")
    print("  결과는 '팔이 실제로 어디에 있었는가' 로 남는다. TCP pose 궤적은 그 결과이므로")
    print("  명령의 형식과 분리돼 있다. 즉 expert 의 표현을 번역하는 대신")
    print("  expert 가 만든 '결과' 를 관측해 라벨을 만드는 것이다.")
    print()
    print("  주의: 스텝 t 의 라벨은 t -> t+1 의 변화다. 인덱스를 하나 밀면")
    print("  모든 라벨이 한 스텝 어긋나고, 이 오류는 round-trip 에서 잡힌다.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 해설: 층마다 답이 다르다")
    print("=" * 50 + "\n")
    print("답의 뼈대:")
    print("  (1) embodiment 층: OpenVLA 는 여러 로봇의 대규모 데이터로 사전학습됐고,")
    print("      Franka 계열 팔은 공개 데이터에 흔하다. 따라서 '이 팔을 본 적 없다' 는")
    print("      주장은 확인 없이 하면 틀릴 가능성이 높다. 데이터셋 목록을 실제로 열어")
    print("      확인한 뒤 '겹칠 수 있다' 로 적는 것이 정확하다.")
    print()
    print("  (2) adaptation 의 의미는 시각 도메인 층에서 나온다. 사전학습은 실로봇")
    print("      영상 중심이고, sim 렌더 화면·씬·조명은 그와 다르다. 같은 팔이라도")
    print("      '이 화면에서 이 팔을 어떻게 움직이는가' 는 학습된 적이 없다.")
    print()
    print("  이 정밀도가 필요한 이유: 면접이나 리뷰에서 '그럼 이미 아는 로봇 아닌가' 라는")
    print("  질문이 그대로 나온다. 층을 나눠 두면 그 질문에 한 문장으로 답할 수 있다.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) week0 의 하네스 검증 통과")
    print("=" * 50 + "\n")
    print("해설:")
    print("  성공률 0% 를 만드는 원인은 두 가지다 -- 도메인 갭과 통합 버그.")
    print("  검증 없이 낮은 성공률을 '미학습 분포의 증거' 로 쓰면,")
    print("  실제로는 변환 버그였던 것을 분포 차이로 오독하게 된다.")
    print()
    print("  그래서 week0 의 순서 제약(검증 -> 측정)이 week1 의 논증까지 지탱한다.")
    print("  distribution_check.md 의 간접 증거 행에 이 의존을 명시해 둔다.")
    print()
    print("  A 가 틀린 이유: N 을 늘리면 신뢰구간이 좁아지지만 원인 구분은 안 된다.")
    print("  두 원인이 같은 숫자를 만드는 문제는 표본 수로 풀리지 않는다.")


def problem5_solution():
    print("\n" + "=" * 50)
    print("문제 5 해설: 상태 분포가 expert 궤적 주변으로 좁다")
    print("=" * 50 + "\n")
    print("답의 뼈대:")
    print("  (1) 한계: 모델은 expert 가 지나간 상태만 보고 배운다.")
    print("      실패·이탈 상태에 대한 정답이 데이터에 없다.")
    print()
    print("  (2) 양상: 추론 중 한 번 궤도에서 벗어나면 학습에서 본 적 없는 상태에")
    print("      놓이고, 그 상태의 정답을 모르므로 오차가 누적된다.")
    print("      '초반은 그럴싸한데 중간부터 무너진다' 로 나타나는 것이 전형이다.")
    print()
    print("  (3) 이번 Phase 의 대응: 해결하지 않고 명시한다.")
    print("      - 실패 episode 를 통계로 남겨 expert 품질을 기록")
    print("      - 한계를 미리 적어 두고, before/after 해석에서 원인 후보로 사용")
    print("      (정책의 이탈 상태를 다시 라벨링하는 계열의 방법은 별도 과제다)")


def problem6_solution():
    print("\n" + "=" * 50)
    print("문제 6 정답: C) 성분 추출 또는 스케일링 대상 오류")
    print("=" * 50 + "\n")
    print("해설:")
    print("  gripper 는 연속 물리량이 아니라 개폐 신호에 가깝다.")
    print("  따라서 라벨 분포는 두 값 근처에 몰려야 정상이다.")
    print()
    print("  넓게 퍼졌다면 후보는 둘:")
    print("    1) expert action 에서 gripper 성분의 인덱스를 잘못 뽑았다")
    print("       (다른 관절 값이 들어왔다)")
    print("    2) 정규화/스케일링을 gripper 차원에도 적용했다")
    print("       -- week0 실습 4 에서 확인한 차원별 마스크가 이것을 막는 장치다")
    print()
    print("  A, B, D 는 분포의 '모양' 을 설명하지 못한다. 데이터가 적으면 히스토그램이")
    print("  거칠어지지만 두 값 근처에 몰리는 성질 자체는 유지된다.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    problem6_solution()
    print("=" * 50)
