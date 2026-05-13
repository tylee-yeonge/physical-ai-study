"""
Phase 4 Week 1 - RT-2 Architecture / Action Tokenization 중급 퀴즈 정답
"""
import numpy as np


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: Quantization step 계산")
    print("=" * 60 + "\n")

    N_BIN = 256

    step_dx = (0.10 - (-0.10)) / N_BIN
    step_rx = (np.pi - (-np.pi)) / N_BIN
    step_grip = (1.0 - 0.0) / N_BIN

    print("  공식: step = (max - min) / N_BIN")
    print(f"  N_BIN = {N_BIN}")
    print()
    print(f"  dx  : ( 0.10 - (-0.10) ) / 256 = {step_dx:.6f} m = {step_dx*1000:.4f} mm")
    print(f"  rx  : ( pi   - (-pi)   ) / 256 = {step_rx:.6f} rad = {np.degrees(step_rx):.4f} deg")
    print(f"  grip: ( 1.0  -  0.0    ) / 256 = {step_grip:.6f}")

    print("\n  [tip] 양자화 오차의 직관:")
    print("    - dx step ~ 0.78 mm: 컵 잡기 (cm 단위) 에는 충분, 정밀 조립 (sub-mm) 은 한계")
    print("    - rx step ~ 1.4 deg: 대부분 OK, mechanical assembly 같은 정밀 회전은 한계")
    print("    - grip step ~ 0.004: 잡기/놓기에는 충분, 미세 force control 은 불가")
    print()
    print("  이 수치들이 RT-2 의 'fine motion 한계' 의 정량적 근거.")
    print("  Phase 7 의 산출물 #4 에서 'VLA latency / quantization 의 양산 비용'")
    print("  으로 면접 포인트가 된다.")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: 한 frame 의 output token 길이")
    print("=" * 60 + "\n")

    action_tokens = 7
    special_tokens = 2
    fps = 5

    tokens_per_frame = action_tokens + special_tokens  # = 9
    tokens_per_second = tokens_per_frame * fps  # = 45

    print(f"  tokens_per_frame = {action_tokens} + {special_tokens} = {tokens_per_frame}")
    print(f"  tokens_per_second = {tokens_per_frame} * {fps} = {tokens_per_second}")
    print()
    print("  비교:")
    print("    - RTX 4070 에서 일반 LLM (Llama 7B fp16) 의 token/s : ~ 100 token/s")
    print("    - PaLI-X 5B 의 token/s 도 비슷 (~50~100, 양자화 시 더 빠름)")
    print(f"    - RT-2 가 필요로 하는 속도 : {tokens_per_second} token/s")
    print()
    print("  결론: 5Hz 는 RTX 4070 으로 *간신히* 가능한 수치이다.")
    print("       단, prompt processing time (input 196+N image tokens) 이 추가되어")
    print("       실제 latency 는 200~300ms 가 일반적.")
    print()
    print("  [tip] 30Hz 실시간 제어를 원한다면:")
    print("       - quantization (4-bit 등) → 2x 속도")
    print("       - speculative decoding → 1.5~2x 속도")
    print("       - smaller backbone → 모델 변경")
    print("       - parallel decoding of action tokens → action 7 개를 한 번에")
    print()
    print("  본 로드맵의 Phase 7 산출물 #4 에서 'latency 측정' 의 핵심 근거.")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: Emergent capability 사례 분류")
    print("=" * 60 + "\n")

    print("  정답: A, C, D")
    print()
    print("  분석:")
    print("    A) 'pick up the green can'")
    print("       -> 학습 데이터에 색상 명시 없음")
    print("       -> VLM 의 'green' 인식 능력이 transfer 됨")
    print("       => Emergent")
    print()
    print("    B) 'pick up the can'")
    print("       -> 정확히 학습된 명령")
    print("       -> 학습 분포 내")
    print("       => Not emergent (정상 동작)")
    print()
    print("    C) 'move toward the dirty cup'")
    print("       -> 'dirty' 라는 형용사 의미 추론 필요")
    print("       -> VLM 의 VQA 능력이 transfer 됨")
    print("       => Emergent")
    print()
    print("    D) 'pick up the leftmost object'")
    print("       -> 공간적 추론 (relative position)")
    print("       -> 학습 데이터엔 명시적 spatial reasoning 없음")
    print("       => Emergent")
    print()
    print("    E) 'pick up the apple' (apple 이 robot data 에 있는 경우)")
    print("       -> 학습 분포 내")
    print("       => Not emergent")
    print()
    print("  [tip] 'web knowledge transfer 가 필요했는가' 가 판별 기준.")
    print("       - 색상/속성/공간/추론/상식 등이 동원되면 emergent")
    print("       - 학습 데이터에 정확히 같은 형태가 있으면 not emergent")


if __name__ == "__main__":
    print("=" * 60)
    print("  Phase 4 Week 1 Quiz - Medium 정답")
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "=" * 60)
