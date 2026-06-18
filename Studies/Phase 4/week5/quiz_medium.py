"""
Phase 4 Week 5 - OpenX-Embodiment + Fine-tuning 중급 퀴즈
"""


def problem1_step_count_estimation():
    """
    문제 1: 자작 팔 데이터 수집 계획의 step 수 추정

    Phase 7 의 산출물 #4 fine-tune 을 위한 데이터 수집 계획:
      - 3 종류 task (pick-and-place, stacking, push)
      - 각 task 당 80 demonstrations
      - 평균 episode 길이 50 step (10초 at 5Hz)

    이때:
      (a) 총 episode 수: ?
      (b) 총 step 수: ?
      (c) RTX 4090 1대 가정 시 LoRA 학습 시간 (1 step ~ 1.5초 가정):  ?

    TODO: 각 값을 계산.
    """
    print("\n" + "=" * 60)
    print("문제 1: 자작 팔 데이터 수집 + 학습 시간 추정")
    print("=" * 60 + "\n")

    n_tasks = 3
    demos_per_task = 80
    steps_per_episode = 50
    sec_per_step = 1.5  # GPU 학습 시 1 step (forward+backward) 의 시간

    # TODO
    total_episodes = 0
    total_steps = 0
    train_time_hours = 0.0

    print(f"  당신의 답:")
    print(f"    (a) total_episodes : {total_episodes}")
    print(f"    (b) total_steps    : {total_steps}")
    print(f"    (c) train_time_hr  : {train_time_hours:.2f}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem2_lora_memory_budget():
    """
    문제 2: LoRA 학습 시 GPU 메모리 budget 분석

    RTX 4070 12GB 에서 OpenVLA 7B + LoRA 학습 가능?
    아래 항목 별 메모리 추정값을 채우시오.

      - base model (int4)        : ? GB (7B * 0.5 byte * 1.2 overhead)
      - LoRA weights (fp16, ~70M) : ? GB
      - activation (batch=1)      : ~4 GB
      - gradient (LoRA, fp16)     : ~0.2 GB
      - optimizer state (Adam, ~3x of grad) : ?
      - buffer + KV cache        : ~1 GB

    TODO: 각 GB 값을 채우고 12GB 에 fit 하는지 판단.
    """
    print("\n" + "=" * 60)
    print("문제 2: LoRA 학습 시 GPU memory budget")
    print("=" * 60 + "\n")

    # TODO
    base_int4 = 0.0   # 7e9 * 0.5 / 1e9 * 1.2
    lora_fp16 = 0.0   # 70e6 * 2 / 1e9
    activation = 4.0
    gradient = 0.2
    optimizer = 0.0   # ~ 3 * gradient
    buffer = 1.0

    total = base_int4 + lora_fp16 + activation + gradient + optimizer + buffer
    fits = total < 12.0

    print(f"  당신의 추정 (GB):")
    print(f"    base (int4)  : {base_int4:.2f}")
    print(f"    LoRA (fp16)  : {lora_fp16:.2f}")
    print(f"    activation   : {activation:.2f}")
    print(f"    gradient     : {gradient:.2f}")
    print(f"    optimizer    : {optimizer:.2f}")
    print(f"    buffer       : {buffer:.2f}")
    print(f"    ---")
    print(f"    total        : {total:.2f}")
    print(f"\n  RTX 4070 12GB fit?  {fits}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem3_finetune_strategy():
    """
    문제 3: 자작 팔 fine-tune 전략 결정

    자작 6DOF 팔에 OpenVLA 를 적응시키려 한다.
    아래 시나리오에서 가장 좋은 전략은?

    상황:
      - 자작 팔 demonstrations: 240 episodes (3 task × 80)
      - GPU: RTX 4070 12GB
      - 가용 시간: 약 5 ~ 10 시간
      - 목표: pick-and-place success rate > 70%

    선택지:
    A) Full 7B fine-tune (모든 파라미터 학습)
    B) LoRA fine-tune (rank=32, attention + FFN target)
    C) Vision encoder 만 학습
    D) 무 fine-tune, zero-shot inference

    답: 'A'/'B'/'C'/'D' 중 하나 채우고 이유를 설명.
    """
    print("\n" + "=" * 60)
    print("문제 3: Fine-tune 전략 결정")
    print("=" * 60 + "\n")

    # TODO
    answer = ""
    reasoning = ""

    print(f"  당신의 답 : {answer}")
    print(f"  당신의 이유: {reasoning}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    print("  Phase 4 Week 5 Quiz - Medium")
    print("=" * 60)
    problem1_step_count_estimation()
    problem2_lora_memory_budget()
    problem3_finetune_strategy()
    print("\n" + "=" * 60)
