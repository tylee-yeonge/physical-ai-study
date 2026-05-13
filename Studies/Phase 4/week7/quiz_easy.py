"""
Phase 4 Week 7 - OpenVLA 블로그 1편 작성 기초 퀴즈
"""


def problem1_differentiation():
    print("\n" + "=" * 50)
    print("문제 1: RT-2 vs OpenVLA 블로그 차별점")
    print("=" * 50 + "\n")
    print("질문: OpenVLA 블로그가 RT-2 블로그 (week 3) 와 가장 다른 점?\n")
    print("보기:")
    print("  A) 분량이 더 길다")
    print("  B) 본인이 직접 RTX 4070 에서 inference 한 실측 데이터 포함")
    print("  C) 영어로 쓰인다")
    print("  D) 다이어그램이 더 많다")


def problem2_data_for_blog():
    print("\n" + "=" * 50)
    print("문제 2: week 6 의 latency 데이터 활용")
    print("=" * 50 + "\n")
    print("질문: week 6 에서 저장한 `openvla_latency_4070_int4.npy` 데이터는")
    print("      OpenVLA 블로그의 어디에 인용되는가?\n")
    print("보기:")
    print("  A) Section 1 한 줄 요약")
    print("  B) Section 5 실측 결과 표")
    print("  C) Section 8 Reference")
    print("  D) Section 2 배경")


def problem3_throughput_at_4070():
    print("\n" + "=" * 50)
    print("문제 3: 5~6Hz 로 가능한 작업")
    print("=" * 50 + "\n")
    print("질문: OpenVLA 의 5~6Hz throughput 으로 무리없이 가능한 작업은?\n")
    print("보기:")
    print("  A) 60Hz 폐쇄 루프 동적 제어 (catching, juggling)")
    print("  B) cm 단위 pick-and-place (10cm/s 이동)")
    print("  C) mm 단위 자동차 부품 조립")
    print("  D) 실시간 force control")


def problem4_blog_target_audience():
    print("\n" + "=" * 50)
    print("문제 4: 블로그의 1순위 독자")
    print("=" * 50 + "\n")
    print("질문: 본 로드맵에서 블로그의 1순위 독자는?\n")
    print("보기:")
    print("  A) 같은 분야 박사 / 연구자")
    print("  B) 1순위 타깃 회사의 채용 담당자 + 면접관")
    print("  C) 일반 대중 (테크 블로그 독자)")
    print("  D) 본인 (학습 기록용)")


if __name__ == "__main__":
    print("=" * 50)
    problem1_differentiation()
    problem2_data_for_blog()
    problem3_throughput_at_4070()
    problem4_blog_target_audience()
    print("=" * 50)
