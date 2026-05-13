"""
Phase 4 Week 7 - 중급 퀴즈 정답 예시
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답 예시: Section 5 표")
    print("=" * 60 + "\n")

    example = """
| Metric | Value |
|---|---|
| mean latency | 165 ms |
| p95 latency  | 220 ms |
| p99 latency  | 250 ms |
| Throughput (mean) | 6.0 Hz |
| Throughput (p95)  | 4.5 Hz |
| Model GPU memory  | 5.3 GB |
| Peak GPU memory   | 6.5 GB |
""".strip()

    print(example)
    print()
    print("  [tip] 표의 핵심은 mean / p95 / p99 의 분리 표시.")
    print("       p99 가 너무 크면 worst-case 가 양산에 문제.")
    print("       p95 throughput 까지 보고 robot 제어 frequency 결정해야 함.")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답 예시: 양산 SW 엔지니어 차별화 단락")
    print("=" * 60 + "\n")

    example = """
RTX 4070 에서 OpenVLA 의 mean latency 가 165 ms (6 Hz) 라는 사실은,
9년의 AMR ROS 양산 SW 경험에서 봤을 때 자동차 부품 조립 같은 정밀
작업에 직접 사용할 수 없는 수치다. 자동차 양산 라인은 60 Hz 폐쇄 루프
제어를 기본으로 하며, action 명령과 모터 응답 사이의 16 ms 마진
안에서 안전 인터록이 동작해야 한다. OpenVLA 의 165 ms 는 그 마진의
10 배 이상. 따라서 OpenVLA 같은 'slow brain' 은 반드시 자체 안전
루프 (fast safety policy, joint-level PD + 토크 한계) 와 결합된
hierarchical 구조 안에서만 양산 통합이 가능하다.
""".strip()

    print(f"({len(example)}자)")
    print(example)
    print()
    print("  [tip] 좋은 단락의 구조:")
    print("    1) 사실 (165ms, 6Hz)")
    print("    2) 본인 경력의 관점 (9년 AMR ROS)")
    print("    3) 양산 비교 기준 (60Hz, 16ms 마진)")
    print("    4) 결론 (hierarchical 구조 필수)")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답 예시: RT-2 vs OpenVLA 블로그 한 줄 차별점")
    print("=" * 60 + "\n")

    examples = [
        "RT-2 블로그가 학술적 흐름의 이해라면, OpenVLA 블로그는 본인 RTX 4070 에서 직접 측정한 165 ms latency 의 양산 의미 해석이다.",
        "두 블로그를 함께 보면, RT-2 정독으로 학술적 흐름을 알고 OpenVLA 실측으로 양산 비용을 직접 검증한 엔지니어임이 드러난다.",
    ]

    for i, ex in enumerate(examples, 1):
        print(f"  ({len(ex)}자) {i}) {ex}")

    print()
    print("  [tip] 한 줄 차별점은 LinkedIn / 이력서 / 면접 자기소개의 hook 으로 재활용 가능.")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
