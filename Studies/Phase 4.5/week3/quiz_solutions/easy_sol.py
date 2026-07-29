"""
Phase 4.5 Week 3 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) frozen base 가중치만으로 약 15GB")
    print("=" * 50 + "\n")
    print("해설:")
    print("  항목별 크기:")
    print("    frozen base (bf16)    : 약 15GB   <- 바닥을 결정")
    print("    LoRA 어댑터 가중치     : 수십 MB")
    print("    어댑터 옵티마이저 상태 : 수백 MB")
    print("    활성값                : 배치에 비례")
    print()
    print("  LoRA 는 학습 대상을 줄여 옵티마이저·그래디언트를 줄이는 기법이지")
    print("  base 를 줄이는 기법이 아니다. 배치를 1 로 해도 15GB 는 남는다.")
    print()
    print("  A 가 흔한 오해다. 학습 대상이 어댑터뿐이라 옵티마이저 상태는 작다.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: A) 메모리를 낮추며 유효 배치 유지, 대가는 시간")
    print("=" * 50 + "\n")
    print("해설:")
    print("  batch size    -> 활성값 크기 -> 메모리를 결정")
    print("  유효 배치     -> 그래디언트의 안정성 -> 학습의 성질을 결정")
    print("  grad accumulation 은 이 둘을 분리해 준다.")
    print()
    print("  대가: 유효 배치를 유지하려면 순전파·역전파 횟수가 그만큼 늘어난다.")
    print("  즉 메모리를 시간으로 바꾸는 거래다. 24GB 에서 이 조합을 찾는 것이")
    print("  실습 3 의 일이다.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) 기본값이 사전학습급 규모다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  기본 스텝 수는 20만 규모로, 사전학습급 실험을 전제한 값이다.")
    print("  그대로 실행하면 예산 안에 끝나지 않는다.")
    print()
    print("  우리는 반대 방향으로 정한다:")
    print("    예산(시간·비용) -> probe 로 스텝당 시간 측정 -> 가능한 총 스텝 -> max_steps")
    print()
    print("  D 가 틀린 이유: 스텝 상한으로 도는 구조이므로 loss 수렴으로 멈추지 않는다.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: C) 데이터셋 통계 파일")
    print("=" * 50 + "\n")
    print("해설:")
    print("  학습은 데이터셋 통계로 action 을 정규화한다. 추론에서는 그 통계로")
    print("  역정규화해야 실제 물리량이 나온다 -- unnorm_key 가 가리키는 것이 이 통계다.")
    print()
    print("  통계 파일이 없으면 역정규화 기준이 사라져 fine-tuned 모델을 쓸 수 없다.")
    print("  week0 에서 남의 로봇 통계(bridge_orig)를 빌려 쓴 자리에")
    print("  이제 내 데이터셋 통계가 들어가야 한다.")
    print()
    print("  회수 목록: 머지 가중치 / 어댑터 원본 / 통계 파일 / 학습 설정.")


def problem5_solution():
    print("\n" + "=" * 50)
    print("문제 5 정답: B) volume 에 주기 저장 + 작업 후 pod 중지")
    print("=" * 50 + "\n")
    print("해설:")
    print("  두 가지 위험에 각각 대응한다:")
    print("    회수      -> network volume 에 주기 저장 (pod 가 사라져도 남는다)")
    print("    유휴 과금 -> 작업이 끝나면 pod 를 멈춘다 (volume 은 유지된다)")
    print()
    print("  A 가 틀린 이유: 컨테이너 내부 저장은 pod 가 회수되면 함께 사라진다.")
    print("  D 가 틀린 이유: 학습이 몇 시간 단위면 회수 확률을 무시할 수 없다.")
    print()
    print("  그리고 대응이 실제로 되는지는 실습 5 의 복구 리허설로 확인한다 --")
    print("  회수된 다음에 처음 시도하면 그때는 방법을 찾을 시간이 없다.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    print("=" * 50)
