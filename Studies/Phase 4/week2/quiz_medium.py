"""
Phase 4 Week 2 - Co-fine-tuning + Action Tokenization 중급 퀴즈
"""
import numpy as np


def problem1_recover_action_from_token():
    """
    문제 1: token ID sequence 로부터 action 복원

    어떤 inference 결과로 다음 token ID sequence 가 나왔다:
      [255879, 255698, 255856, 255872, 255872, 255872, 255744]

    각 차원의 범위는:
      dx, dy, dz : [-0.1, +0.1] m
      rx, ry, rz : [-pi, +pi]   rad
      gripper   : [0, 1]

    VOCAB_SIZE = 256000, ACTION_TOKEN_START = 255744 일 때
    실제 7-DoF action 값을 복원하시오.

    TODO: continuous_action 리스트를 채우시오.
    """
    print("\n" + "=" * 60)
    print("문제 1: token ID -> continuous action 복원")
    print("=" * 60 + "\n")

    token_ids = [255879, 255698, 255856, 255872, 255872, 255872, 255744]
    ACTION_TOKEN_START = 255744
    N_BIN = 256

    a_min = np.array([-0.10, -0.10, -0.10, -np.pi, -np.pi, -np.pi, 0.0])
    a_max = np.array([ 0.10,  0.10,  0.10,  np.pi,  np.pi,  np.pi, 1.0])

    # TODO: 직접 계산해보세요. 각 dim 에 대해
    # 1. bin = token_id - ACTION_TOKEN_START
    # 2. action = a_min + (bin + 0.5) / N_BIN * (a_max - a_min)
    continuous_action = [0.0] * 7  # 여기를 채우시오

    # 검증
    bin_idx = np.array(token_ids) - ACTION_TOKEN_START
    expected = a_min + (bin_idx + 0.5) / N_BIN * (a_max - a_min)

    print(f"  token_ids : {token_ids}")
    print(f"  bin index : {bin_idx.tolist()}")
    print(f"  당신의 답 : {[round(x, 4) for x in continuous_action]}")
    print(f"  기대 답   : {[round(x, 4) for x in expected]}")

    if all(abs(c - e) < 1e-4 for c, e in zip(continuous_action, expected)):
        print("\n  [O] 정답!")
    else:
        print("\n  [X] 다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_vocab_overlap():
    """
    문제 2: Vocab overlap 문제

    RT-2 가 vocab 의 마지막 256개를 action 으로 재사용한다.
    만약 web 학습 데이터에서 그 256개 token 이 자주 등장하면 무슨 문제가 생기는가?

    아래 시나리오 4 개 중 가장 큰 문제가 되는 것을 고르시오.

    A) Web caption "Hello supersonic boy" 가 action_bin_5 token 으로
       tokenize 되는 경우 (sub-word 우연 일치)
    B) Web VQA "What is 256?" 의 답에 마지막 vocab 의 token 이 포함
    C) Web 학습 데이터에서 마지막 vocab 의 token 들이 일정 빈도 이상
       등장 (예: 매 batch 마다 평균 0.1% 의 token 이 그 범위)
    D) Robot data 의 instruction text 가 action 범위 token 을 우연히 포함

    답: 답 후보 emerg_problem 변수에 'A'/'B'/'C'/'D' 중 하나로 채우시오.
    """
    print("\n" + "=" * 60)
    print("문제 2: Vocab overlap 의 위험")
    print("=" * 60 + "\n")

    # TODO
    emerg_problem = ""  # 여기를 채우시오

    expected = "C"

    print(f"  당신의 답 : {emerg_problem}")
    print(f"  기대 답   : {expected}")

    if emerg_problem == expected:
        print("\n  [O] 정답!")
    else:
        print("\n  [X] 다시 생각해보세요. 정답은 quiz_solutions/medium_sol.py 참고")

    print("\n  힌트: '평소에 자주 나타나는가' 가 핵심.")
    print("        web data 에서 그 token 이 일정 빈도 이상 등장하면")
    print("        그 embedding 이 양쪽 의미로 끌어당겨져 학습 충돌.")


def problem3_compute_combined_loss():
    """
    문제 3: Combined loss 의 가중 평균 계산

    한 학습 step 의 mini-batch 가 다음과 같다:
      - WebLI sample 8개, 평균 loss 2.3
      - VQA sample  4개, 평균 loss 3.1
      - Robot sample 4개, 평균 loss 4.5

    이때 PyTorch 의 표준 cross-entropy (reduction='mean') 가 계산하는
    'batch 전체 평균 loss' 는 얼마인가?
    (각 sample 의 sequence length 는 모두 동일하다고 가정)

    TODO: combined_loss 를 직접 계산하시오.
    """
    print("\n" + "=" * 60)
    print("문제 3: Combined loss 계산")
    print("=" * 60 + "\n")

    # TODO: 직접 계산
    combined_loss = 0.0  # 여기를 채우시오

    expected = (8 * 2.3 + 4 * 3.1 + 4 * 4.5) / (8 + 4 + 4)
    print(f"  Web sample (8): loss=2.3")
    print(f"  VQA sample (4): loss=3.1")
    print(f"  Robot (4)     : loss=4.5")
    print()
    print(f"  당신의 답 : {combined_loss:.4f}")
    print(f"  기대 답   : {expected:.4f}")

    if abs(combined_loss - expected) < 1e-4:
        print("\n  [O] 정답!")
    else:
        print("\n  [X] 다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    print("  Phase 4 Week 2 Quiz - Medium")
    print("  Co-fine-tuning + Action Tokenization 의 수치 감각")
    print("=" * 60)
    problem1_recover_action_from_token()
    problem2_vocab_overlap()
    problem3_compute_combined_loss()
    print("\n" + "=" * 60)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("=" * 60)
