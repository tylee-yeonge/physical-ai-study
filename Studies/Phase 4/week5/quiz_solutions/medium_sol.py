"""
Phase 4 Week 5 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: 데이터 수집 + 학습 시간")
    print("=" * 60 + "\n")

    n_tasks = 3
    demos_per_task = 80
    steps_per_episode = 50
    sec_per_step = 1.5

    total_eps = n_tasks * demos_per_task
    total_steps = total_eps * steps_per_episode
    train_time_hr = total_steps * sec_per_step / 3600

    print(f"  (a) total_episodes = {n_tasks} * {demos_per_task} = {total_eps}")
    print(f"  (b) total_steps    = {total_eps} * {steps_per_episode} = {total_steps:,}")
    print(f"  (c) train_time    = {total_steps} * {sec_per_step}s / 3600 = {train_time_hr:.2f} hr")
    print()
    print("  [tip] 실제 LoRA 학습은:")
    print("       - 1 epoch ~ 36000 step / batch_size 1 = ~ 9 hr (위 계산)")
    print("       - 1~3 epoch 권장 (overfit 주의)")
    print("       - RTX 4090 기준 8~24 시간 학습")
    print("       - RTX 4070 으로는 1.5~2배 더 걸림")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: GPU memory budget")
    print("=" * 60 + "\n")

    base_int4 = 7e9 * 0.5 * 1.2 / 1e9
    lora_fp16 = 70e6 * 2 / 1e9
    activation = 4.0
    gradient = 0.2
    optimizer = 3 * gradient
    buffer = 1.0
    total = base_int4 + lora_fp16 + activation + gradient + optimizer + buffer

    print(f"  base (int4)    : {base_int4:.2f} GB")
    print(f"  LoRA (fp16)    : {lora_fp16:.2f} GB")
    print(f"  activation     : {activation:.2f} GB")
    print(f"  gradient       : {gradient:.2f} GB")
    print(f"  optimizer (3x) : {optimizer:.2f} GB")
    print(f"  buffer + cache : {buffer:.2f} GB")
    print(f"  ---")
    print(f"  total          : {total:.2f} GB")
    print()
    print(f"  RTX 4070 (12GB) fit?  {total < 12}")
    print()
    print("  [tip] 위 budget 은 batch_size=1 기준.")
    print("       batch_size 2 이상이면 activation 2x 증가 -> OOM")
    print("       gradient_accumulation 8 로 effective batch 8 달성.")
    print()
    print("  optimizer state 가 3x gradient 인 이유:")
    print("       Adam: m, v, fp32 copy = ~ 3 * gradient size")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: B) LoRA fine-tune")
    print("=" * 60 + "\n")

    print("해설:")
    print("  각 옵션 분석:")
    print()
    print("    A) Full 7B fine-tune")
    print("       - 7B parameter 학습 = ~ 50 GB+ VRAM 필요")
    print("       - RTX 4070 12GB 로는 불가능")
    print("       => 부적합")
    print()
    print("    B) LoRA fine-tune (rank=32, all linear)  <- 정답")
    print("       - 학습 파라미터 ~ 70M (1%)")
    print("       - VRAM ~ 10 GB (4070 에 fit)")
    print("       - 학습 시간 5~10 시간")
    print("       - 240 episode 면 충분히 학습 가능")
    print("       => 최적")
    print()
    print("    C) Vision encoder 만 학습")
    print("       - DINOv2 / SigLIP 는 이미 충분한 vision feature")
    print("       - 학습 시 action 생성은 그대로")
    print("       - target task adaptation 효과 미미")
    print("       => 부적합")
    print()
    print("    D) Zero-shot inference")
    print("       - 자작 팔이 OpenX 의 22 embodiment 중 하나가 아님")
    print("       - 환경 / 카메라 setup 도 다름")
    print("       - success rate ~ 0~20% 예상")
    print("       => 70% 목표 달성 불가")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
