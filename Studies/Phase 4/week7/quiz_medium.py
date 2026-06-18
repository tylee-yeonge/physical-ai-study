"""
Phase 4 Week 7 - OpenVLA 블로그 중급 퀴즈
"""


def problem1_write_section5_table():
    """
    문제 1: Section 5 의 latency 통계 표 작성

    week 6 의 .npy 데이터 (mean=165, p95=220, p99=250 가정) 로
    블로그 Section 5 에 들어갈 표를 작성하시오.

    조건:
      - Markdown 표 형태
      - 컬럼: Metric, Value
      - Row: mean / p95 / p99 / throughput / GPU memory

    TODO: my_table 에 Markdown 표 작성.
    """
    print("\n" + "=" * 60)
    print("문제 1: Section 5 표 작성")
    print("=" * 60 + "\n")

    # TODO
    my_table = """
| Metric | Value |
|---|---|
... 여기를 채우시오
""".strip()

    print("당신의 표:")
    print(my_table)

    print("\n자가 평가:")
    print("  필요 항목: mean / p95 / p99 / throughput / GPU memory")
    print("  형태: Markdown table")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem2_write_differentiation_paragraph():
    """
    문제 2: 양산 SW 엔지니어 차별화 단락 (Section 7)

    "OpenVLA 의 5Hz throughput 이 자동차 부품 조립에 부적합한 이유" 를
    200~300 자 단락으로 작성하시오.

    조건:
      - "5Hz" / "165 ms" / "60Hz" 같은 정량 수치
      - "안전 인터록" / "hierarchical" / "fast safety policy" 중 1개 이상
      - 본인 경력 키워드 (AMR ROS 5년 / 펌웨어) 중 1개 이상

    TODO: my_para 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 2: 양산 SW 엔지니어 차별화 단락")
    print("=" * 60 + "\n")

    # TODO
    my_para = ""

    print("당신의 단락:")
    print(my_para)

    checks = {
        "200~400자": 200 <= len(my_para) <= 400,
        "정량 수치 포함": any(
            kw in my_para
            for kw in ["5Hz", "165", "60Hz", "30Hz", "ms", "Hz"]
        ),
        "구조 키워드 포함": any(
            kw in my_para
            for kw in ["안전", "인터록", "hierarchical", "fast", "safety"]
        ),
        "경력 키워드 포함": any(
            kw in my_para
            for kw in ["AMR", "ROS", "펌웨어", "양산", "5년"]
        ),
    }
    print("\n자가 평가:")
    for k, v in checks.items():
        mark = "[O]" if v else "[X]"
        print(f"  {mark} {k}")


def problem3_compare_with_rt2_blog():
    """
    문제 3: RT-2 블로그와의 차별점 한 줄

    RT-2 블로그와 OpenVLA 블로그의 가장 큰 차별점을 한 문장으로 작성.

    조건:
      - 80~150 자
      - "실측" / "RTX 4070" / "직접" 중 1개 이상
      - 두 블로그가 함께 면접관에게 던지는 메시지 한 단어 이상

    TODO: my_diff 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 3: RT-2 블로그 vs OpenVLA 블로그 한 줄 차별점")
    print("=" * 60 + "\n")

    my_diff = ""

    print("당신의 한 줄:")
    print(my_diff)

    checks = {
        "80~150자": 80 <= len(my_diff) <= 150,
        "직접/실측 키워드": any(
            kw in my_diff for kw in ["실측", "RTX 4070", "직접", "본인"]
        ),
    }
    print("\n자가 평가:")
    for k, v in checks.items():
        mark = "[O]" if v else "[X]"
        print(f"  {mark} {k}")


if __name__ == "__main__":
    print("=" * 60)
    problem1_write_section5_table()
    problem2_write_differentiation_paragraph()
    problem3_compare_with_rt2_blog()
    print("=" * 60)
