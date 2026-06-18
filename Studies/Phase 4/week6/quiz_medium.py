"""
Phase 4 Week 6 - HuggingFace + Inference 중급 퀴즈
"""
import numpy as np


def problem1_latency_to_throughput():
    """
    문제 1: latency -> throughput 변환

    아래 latency 측정 결과가 주어졌을 때:
      - mean: 165 ms
      - p95 : 220 ms

    질문:
      (a) mean throughput (Hz) = ?
      (b) p95 throughput (Hz) = ?
      (c) 5Hz 폐쇄 루프 제어 가능?
      (d) 30Hz 가능?

    TODO 값 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 1: latency -> throughput")
    print("=" * 60 + "\n")

    mean_ms = 165
    p95_ms = 220

    # TODO
    mean_hz = 0.0
    p95_hz = 0.0
    can_5hz = None  # True / False
    can_30hz = None

    print(f"  당신의 답:")
    print(f"    (a) mean throughput : {mean_hz:.2f} Hz")
    print(f"    (b) p95  throughput : {p95_hz:.2f} Hz")
    print(f"    (c) 5Hz 가능?        : {can_5hz}")
    print(f"    (d) 30Hz 가능?       : {can_30hz}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem2_vram_check():
    """
    문제 2: 본인의 측정값으로 VRAM 추정

    RTX 4070 12GB 에서 OpenVLA 4-bit nf4 로딩 후
    `torch.cuda.memory_allocated()` 가 출력한 값이 ~ 5.3 GB 였다.

    이 값에 활성화 (forward 시 추가 메모리) 가 batch=1 inference 시
    얼마나 더 필요한지 추정하시오.

    가정:
      - LM decoder generate 시 KV cache 메모리 ~ 0.5 GB
      - Vision encoder activation ~ 0.5 GB
      - 기타 buffer ~ 0.3 GB

    총 inference 시 GPU 메모리 = ? GB
    12GB 에 fit 하는 batch_size 의 최대값은? (activation 이 batch 비례한다 가정)

    TODO 값 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 2: VRAM 추정 + max batch_size")
    print("=" * 60 + "\n")

    model_loaded = 5.3
    kv_cache = 0.5
    vision_act = 0.5
    buffer = 0.3
    available = 12.0

    # TODO
    total_b1 = 0.0
    activation_b1 = 0.0
    max_batch = 0

    print(f"  당신의 답:")
    print(f"    total_b1     : {total_b1:.2f} GB")
    print(f"    activation_b1 : {activation_b1:.2f} GB")
    print(f"    max_batch    : {max_batch}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem3_warmup_outlier():
    """
    문제 3: Latency 측정 시 warm-up 의 의미

    100 회 inference 의 latency 데이터:
      [800, 250, 200, 180, 170, 165, 162, 160, 165, ..., 165]
      ^----  warm-up 효과
      ^---- 첫 1~3 측정은 outlier

    아래 중 올바른 분석 방법을 모두 고르시오.

    A) 첫 5 회를 제외하고 통계 (mean, p95) 계산
    B) 모든 데이터로 mean 계산 후 outlier 제거
    C) torch.cuda.synchronize() 를 매 measure 시 호출
    D) Throughput 계산 시 outlier 포함하면 throughput 이 과소평가됨

    답: 옳은 것을 모두 골라 'A,C,D' 같은 형태로.
    """
    print("\n" + "=" * 60)
    print("문제 3: Warm-up 처리")
    print("=" * 60 + "\n")

    # TODO
    correct_methods = ""  # 콤마로 구분 (예: "A,B" 형식)

    print(f"  당신의 답 : {correct_methods}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    problem1_latency_to_throughput()
    problem2_vram_check()
    problem3_warmup_outlier()
    print("=" * 60)
