"""
Phase 4 Week 6 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: latency -> throughput")
    print("=" * 60 + "\n")
    mean_hz = 1000 / 165
    p95_hz = 1000 / 220

    print(f"  (a) mean throughput : 1000 / 165 = {mean_hz:.2f} Hz")
    print(f"  (b) p95  throughput : 1000 / 220 = {p95_hz:.2f} Hz")
    print()
    print(f"  (c) 5Hz 가능?  {mean_hz >= 5} (mean = {mean_hz:.2f} > 5)")
    print(f"  (d) 30Hz 가능? {mean_hz >= 30} (mean = {mean_hz:.2f} << 30)")
    print()
    print("  [tip] '5Hz 가능' 의 의미:")
    print("       OpenVLA 가 매 200ms 마다 action 생성 가능.")
    print("       robot 의 low-level controller 가 200ms 사이를 보간하면")
    print("       smooth motion 가능. (Phase 7 의 hierarchical 구조)")
    print()
    print("  '30Hz 불가' 의 의미:")
    print("       OpenVLA 만으로 30Hz 폐쇄 루프 제어 불가능.")
    print("       반드시 fast safety policy (joint-level PD controller 등)")
    print("       와 함께 hierarchical 구조 필요.")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: VRAM 추정")
    print("=" * 60 + "\n")
    model_loaded = 5.3
    kv_cache = 0.5
    vision_act = 0.5
    buffer = 0.3
    available = 12.0

    total_b1 = model_loaded + kv_cache + vision_act + buffer
    activation_b1 = kv_cache + vision_act
    max_batch = int((available - model_loaded - buffer) / activation_b1)

    print(f"  total_b1     : {model_loaded} + {kv_cache} + {vision_act} + {buffer}")
    print(f"               = {total_b1:.2f} GB")
    print()
    print(f"  activation_b1 (batch 변동 부분): {kv_cache} + {vision_act} = {activation_b1:.2f} GB")
    print()
    print(f"  max_batch    : (12 - 5.3 - 0.3) / 1.0 = {max_batch}")
    print()
    print("  [tip] 실제 OpenVLA inference 는 보통 batch_size=1.")
    print("       여러 robot 의 동시 inference 가 필요한 경우 max_batch=6 정도까지 가능.")
    print("       단 KV cache 의 sequence length 가 길어지면 batch 줄여야 할 수 있음.")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: A, C, D")
    print("=" * 60 + "\n")
    print("  A) 첫 5 회 제외 - O")
    print("     warm-up 동안 CUDA kernel JIT compile, model weights GPU 로드 등")
    print("     첫 1~3 measure 는 outlier (보통 2~10배 느림)")
    print()
    print("  B) 모든 데이터로 mean 후 outlier 제거 - X (덜 권장)")
    print("     mean 자체가 outlier 의 영향 받음 -> warm-up 부터 제외하는 게 더 깔끔")
    print()
    print("  C) torch.cuda.synchronize() - O")
    print("     CUDA 는 비동기 실행. python 의 time.time() 측정 직전 sync 필요.")
    print("     sync 없으면 kernel 이 다 안 끝났는데 시간 측정 -> 짧게 측정됨")
    print()
    print("  D) outlier 포함 mean -> throughput 과소평가 - O")
    print("     mean latency 가 outlier 로 늘어남 -> 1/mean 이 줄어듦")
    print()
    print("  [tip] 표준 latency 측정 패턴:")
    print("       1. warm-up 5~10 회")
    print("       2. 100 회 측정 (매번 cuda.synchronize)")
    print("       3. mean / median / p95 / p99 보고")
    print("       4. throughput = 1000 / mean_ms")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
