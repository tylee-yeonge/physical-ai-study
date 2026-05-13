"""
Phase 4 Week 1 - RT-2 Architecture / Action Tokenization 중급 퀴즈

논문의 수치/계산이 손에 잡혔는지 확인하는 3문제.
"""
import numpy as np


def problem1_quantization_step():
    """
    문제 1: Action Quantization step 계산

    RT-2 는 7-DoF action 의 각 차원을 256 bin 으로 양자화한다.
    각 차원의 범위가 아래와 같을 때, quantization step 을 계산하시오.

    - dx, dy, dz : [-0.10, 0.10] m
    - rx, ry, rz : [-pi,  pi]   rad
    - gripper   : [ 0.00, 1.00]

    TODO: step_dx, step_rx, step_grip 를 직접 계산해 보세요.
    """
    print("\n" + "=" * 60)
    print("문제 1: Quantization step 계산")
    print("=" * 60 + "\n")

    N_BIN = 256

    # TODO: 직접 계산
    step_dx = 0.0  # dx 의 step (단위: m)
    step_rx = 0.0  # rx 의 step (단위: rad)
    step_grip = 0.0  # gripper 의 step

    # 검증용
    expected_dx = (0.10 - (-0.10)) / N_BIN
    expected_rx = (np.pi - (-np.pi)) / N_BIN
    expected_grip = 1.0 / N_BIN

    print(f"  계산한 step_dx   : {step_dx:.6f} m = {step_dx*1000:.4f} mm")
    print(f"  계산한 step_rx   : {step_rx:.6f} rad = {np.degrees(step_rx):.4f} deg")
    print(f"  계산한 step_grip : {step_grip:.6f}")
    print(f"\n  기대 step_dx   : {expected_dx:.6f} m = {expected_dx*1000:.4f} mm")
    print(f"  기대 step_rx   : {expected_rx:.6f} rad = {np.degrees(expected_rx):.4f} deg")
    print(f"  기대 step_grip : {expected_grip:.6f}")

    if (
        abs(step_dx - expected_dx) < 1e-9
        and abs(step_rx - expected_rx) < 1e-9
        and abs(step_grip - expected_grip) < 1e-9
    ):
        print("\n  [O] 정답!")
    else:
        print("\n  [X] 다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_output_token_length():
    """
    문제 2: 한 frame 의 output token 길이

    RT-2 는 매 frame 마다 다음을 output 으로 생성한다:
    - action 7 개 (각 1 token, 총 7 token)
    - episode termination 등 special token 1 ~ 4 개

    이때, 1초에 5 frame 을 처리하려면 (5Hz) decoder 가 1초에 생성해야 하는
    총 token 수를 계산하시오. special token 은 평균 2 개로 가정.

    추가: 이 token 수와 일반적인 LLM (예: GPT-3.5) 의 generation speed
    (~100 token/s on RTX 4070) 를 비교했을 때, 5Hz 가 가능한가?

    TODO: tokens_per_frame, tokens_per_second 를 채우시오.
    """
    print("\n" + "=" * 60)
    print("문제 2: 한 frame 의 output token 길이 계산")
    print("=" * 60 + "\n")

    action_tokens = 7
    special_tokens = 2
    fps = 5

    # TODO: 직접 계산
    tokens_per_frame = 0
    tokens_per_second = 0

    print(f"  Action tokens per frame   : {action_tokens}")
    print(f"  Special tokens per frame  : {special_tokens}")
    print(f"  Tokens per frame          : {tokens_per_frame} (기대: {action_tokens + special_tokens})")
    print(f"  Frames per second         : {fps}")
    print(f"  Tokens per second         : {tokens_per_second} (기대: {(action_tokens + special_tokens) * fps})")

    print("\n  질문: 일반 LLM 이 RTX 4070 에서 약 100 token/s 라고 할 때,")
    print(f"        RT-2 의 {tokens_per_second} token/s 는 가능한 수치인가?")
    print("  -> 답은 quiz_solutions/medium_sol.py 참고")


def problem3_emergent_capability_classification():
    """
    문제 3: Emergent capability 사례 분류

    아래 5 가지 명령 중 "RT-1 (small transformer, robot data only) 으로는
    수행 불가능 / RT-2 만 가능" 한 emergent capability 사례를 모두 고르시오.

    A) "pick up the green can"   (학습 데이터에 색상 명시 없음)
    B) "pick up the can"          (정확히 학습된 명령)
    C) "move toward the dirty cup" ('dirty' 라는 의미 추론 필요)
    D) "pick up the leftmost object"  (공간 추론 필요, 학습엔 없음)
    E) "pick up the apple"        (apple 이 robot data 에 있는 경우)

    답: emergent capability 사례를 모두 골라 emergent_cases 리스트에 채우시오.
    """
    print("\n" + "=" * 60)
    print("문제 3: Emergent capability 사례 분류")
    print("=" * 60 + "\n")

    # TODO: emergent capability 인 사례를 모두 채우시오
    emergent_cases = []  # 예: ["A", "C"]

    expected = sorted(["A", "C", "D"])
    print(f"  당신의 답: {sorted(emergent_cases)}")
    print(f"  기대 답  : {expected}")

    if sorted(emergent_cases) == expected:
        print("\n  [O] 정답!")
    else:
        print("\n  [X] 다시 생각해보세요. 정답은 quiz_solutions/medium_sol.py 참고")

    print("\n  힌트: emergent capability 는 'web knowledge transfer' 가 필요한 사례.")
    print("        학습 데이터에 정확히 같은 형태가 있으면 emergent 가 아니다.")


if __name__ == "__main__":
    print("=" * 60)
    print("  Phase 4 Week 1 Quiz - Medium")
    print("  RT-2 Architecture / Action Tokenization 의 수치 감각")
    print("=" * 60)
    problem1_quantization_step()
    problem2_output_token_length()
    problem3_emergent_capability_classification()
    print("\n" + "=" * 60)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("=" * 60)
