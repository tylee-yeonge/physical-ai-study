"""
Phase 4 Week 11 - 중급 퀴즈
"""


def problem1_compute_throughput():
    """
    문제 1: dry-run 결과 분석

    1분 dry-run 결과:
      - inference 총 횟수: 280
      - mean latency: 175 ms
      - p95 latency:  240 ms
      - 0 fail

    질문:
      (a) actual throughput (Hz)?
      (b) 기대 throughput (1000/mean)?
      (c) 두 차이의 원인 한 가지?

    TODO 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 1: dry-run throughput 분석")
    print("=" * 60 + "\n")

    n = 280
    duration_s = 60
    mean_ms = 175

    actual_hz = 0.0  # TODO
    expected_hz = 0.0  # TODO

    print(f"  당신의 답:")
    print(f"    (a) actual_hz   : {actual_hz:.2f}")
    print(f"    (b) expected_hz : {expected_hz:.2f}")
    print(f"    (c) 차이의 원인 : ?")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem2_design_status_msg():
    """
    문제 2: status topic 의 구체 설계

    /vla/status 토픽으로 노드의 현재 상태를 publish 하려고 한다.

    아래 4 가지 상태에 대해 적절한 string 을 작성:
      - 모델 로딩 중
      - inference ready
      - OOM 발생
      - 모델 fail

    TODO statuses 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 2: /vla/status 설계")
    print("=" * 60 + "\n")

    # TODO
    statuses = {
        "loading": "",
        "ready": "",
        "oom": "",
        "model_fail": "",
    }
    print("  당신의 답:")
    for k, v in statuses.items():
        print(f"    {k}: '{v}'")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem3_failure_handling_table():
    """
    문제 3: 실패 시 노드 동작 매핑

    아래 실패 종류에 대해 노드의 적절한 동작:

      A) cv_bridge 변환 실패
      B) image_age 초과
      C) VLAOOMError
      D) VLAOutputError (NaN)
      E) VLAInputError

    동작 옵션:
      1) log + skip (action publish 안 함)
      2) zero action publish + warning log
      3) torch.cuda.empty_cache + log error
      4) 노드 종료
      5) instruction reset

    TODO 매핑.
    """
    print("\n" + "=" * 60)
    print("문제 3: 실패 시 동작 매핑")
    print("=" * 60 + "\n")

    # TODO
    mapping = {"A": 0, "B": 0, "C": 0, "D": 0, "E": 0}

    print("  당신의 답:")
    for k, v in mapping.items():
        print(f"    {k}: {v}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    problem1_compute_throughput()
    problem2_design_status_msg()
    problem3_failure_handling_table()
    print("=" * 60)
