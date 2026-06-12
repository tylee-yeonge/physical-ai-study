"""
Phase 4 Week 6 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: A) bitsandbytes")
    print("=" * 50 + "\n")
    print("해설:")
    print("  bitsandbytes: HuggingFace transformers 와 통합된 4-bit/8-bit quantization")
    print("    - load_in_4bit=True 만으로 자동 변환")
    print("    - nf4 (normalized float 4): 표준 분포 가중치에 최적화")
    print("    - double_quant: quantization constant 까지 quantize (메모리 더 절약)")
    print()
    print("  GPTQ / AWQ: 별도 도구로 모델을 사전 변환 (post-training quant)")
    print("              일반적으로 inference 속도는 더 빠르지만 setup 복잡")
    print()
    print("  본 로드맵: bitsandbytes 가 가장 빠른 진입.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: C) model.predict_action() (custom method)")
    print("=" * 50 + "\n")
    print("해설:")
    print("  OpenVLA 는 VLA 전용 inference API 를 model 에 추가:")
    print("    predict_action(input_ids, pixel_values, unnorm_key, do_sample)")
    print()
    print("  내부 흐름:")
    print("    1. vision encoder forward")
    print("    2. LM decoder generate (action token sequence)")
    print("    3. action token de-tokenize")
    print("    4. unnorm_key 의 통계로 de-normalize")
    print("    5. 7-DoF action ndarray 반환")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) OpenX-Embodiment dataset 의 action norm 통계")
    print("=" * 50 + "\n")
    print("해설:")
    print("  학습 시 OpenVLA 는 각 embodiment 의 action 을 normalize:")
    print("    - WidowX의 action 범위 != Franka 의 범위")
    print("    - 각 dataset 별 mean/std 통계 사용")
    print()
    print("  Inference 시 모델은 normalized action 을 출력:")
    print("    -> unnorm_key 로 어떤 통계로 복원할지 지정")
    print("    -> 'bridge_orig' = WidowX 통계 사용")
    print()
    print("  자작 팔 (Phase 7) 의 경우:")
    print("    - 사전 fine-tune 전: 가까운 embodiment (예: bridge_orig)")
    print("    - LoRA fine-tune 후: 자체 통계 사용")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: C) LM decoder generate")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Latency 분해 (RTX 4070, 4-bit):")
    print("    - Image preprocess (CPU)     : ~ 10~30 ms")
    print("    - Vision encoder forward     : ~ 20~40 ms")
    print("    - LM decoder generate         : ~ 60~120 ms  <- 가장 큼")
    print("    - Action de-tokenize          : ~ 1 ms")
    print()
    print("  왜 LM decoder 가 가장 큰가:")
    print("    - 7~11 token autoregressive generation")
    print("    - 매 token 마다 전체 forward pass")
    print("    - prompt context 길이 (수백 token) 와 무관하게 매번 처리")
    print()
    print("  Latency 줄이는 방법:")
    print("    - speculative decoding (small model 로 후보 토큰 미리 생성)")
    print("    - parallel action token decoding (action 7개 한 번에)")
    print("    - smaller backbone (Llama 1B, etc.)")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
