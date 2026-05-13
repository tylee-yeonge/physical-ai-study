"""
Phase 4 Week 11 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: throughput 분석")
    print("=" * 60 + "\n")
    n = 280
    duration_s = 60
    mean_ms = 175

    actual = n / duration_s
    expected = 1000 / mean_ms

    print(f"  (a) actual throughput   : {n} / {duration_s} = {actual:.2f} Hz")
    print(f"  (b) expected throughput : 1000 / {mean_ms} = {expected:.2f} Hz")
    print()
    print(f"  차이: {expected - actual:.2f} Hz ({(expected - actual)/expected*100:.0f}%)")
    print()
    print("  (c) 원인 가능성 (모두 정답):")
    print("    1. image_age_threshold 로 일부 image skip (가장 흔함)")
    print("    2. instruction 없는 시점 (callback skip)")
    print("    3. image input 자체가 ~ 5Hz 만 들어옴 (bag 의 rate)")
    print("    4. CPU/IO bottleneck (cv_bridge 변환 등)")
    print("    5. ROS executor scheduling latency")
    print()
    print("  [tip] 실제 측정 시 actual 이 expected 보다 항상 약간 작음.")
    print("       gap 이 30% 이상이면 anomaly, < 20% 면 정상.")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: status string 예시")
    print("=" * 60 + "\n")
    examples = {
        "loading": "loading",
        "ready": "ready",
        "oom": "error: oom",
        "model_fail": "error: model_load_failed",
    }
    print("  권장 패턴: '<state>' 또는 'error: <detail>'")
    print()
    for k, v in examples.items():
        print(f"  {k:12s} -> '{v}'")
    print()
    print("  [tip] string 패턴의 장점:")
    print("    - human-readable")
    print("    - log 검색 쉬움")
    print("    - downstream 모니터 노드가 substring match")
    print()
    print("  단점:")
    print("    - structured enum 처럼 type-safe 하지 않음")
    print("    - Phase 7 결정타에서는 enum-style 커스텀 msg 권장")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: 실패 시 동작 매핑")
    print("=" * 60 + "\n")
    table = [
        ("A", "cv_bridge 변환 실패", 1, "이상 input, 무시 + log"),
        ("B", "image age 초과", 1, "오래된 image 무시"),
        ("C", "VLAOOMError", 3, "empty_cache + log error"),
        ("D", "VLAOutputError (NaN)", 2, "zero action publish (safety) + warning"),
        ("E", "VLAInputError", 1, "input 자체가 잘못됨, skip"),
    ]
    for code, scenario, action_idx, why in table:
        action_name = {
            1: "log + skip",
            2: "zero action publish + warning",
            3: "empty_cache + log error",
            4: "노드 종료",
            5: "instruction reset",
        }[action_idx]
        print(f"  {code}) {scenario:25s} -> {action_idx} ({action_name})")
        print(f"      reason: {why}")
    print()
    print("  [tip] D (NaN) 만 'zero action publish' 인 이유:")
    print("    - NaN 은 inference 가 성공했지만 numerical 문제")
    print("    - 다음 frame 에서 정상 복구 가능성 높음")
    print("    - robot 이 last action 유지하기보다 zero (정지) 가 안전")
    print()
    print("  A,B,E 는 inference 자체가 무의미한 상황 -> skip 이 옳음.")
    print("  C 는 시스템 자원 문제 -> 복구 시도.")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
