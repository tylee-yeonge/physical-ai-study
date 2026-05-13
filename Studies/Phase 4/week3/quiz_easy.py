"""
Phase 4 Week 3 - RT-2 블로그 1편 작성 기초 퀴즈

이번 주는 글쓰기 주차라 코드 보다는 '좋은 블로그의 기준' 을 확인하는 4문제.
"""


def problem1_blog_section_priority():
    print("\n" + "=" * 50)
    print("문제 1: 면접관이 가장 먼저 보는 section")
    print("=" * 50 + "\n")

    print("질문: 면접관이 30초 안에 블로그의 가치를 판단할 때")
    print("      가장 먼저 보는 section 은?\n")

    print("보기:")
    print("  A) Section 4 (자세한 동작 / 수식)")
    print("  B) Section 1 (한 줄 요약) + Section 3 (한 페이지 요약)")
    print("  C) Section 8 (Reference)")
    print("  D) Section 5 (실험 결과)")


def problem2_blog_quality():
    print("\n" + "=" * 50)
    print("문제 2: 좋은 블로그의 가장 큰 특징")
    print("=" * 50 + "\n")

    print("질문: 좋은 기술 블로그와 나쁜 기술 블로그의")
    print("      가장 큰 구분 기준은?\n")

    print("보기:")
    print("  A) 분량 (길수록 좋다)")
    print("  B) 다이어그램 수")
    print("  C) 한계 / 비판 section 의 유무와 깊이")
    print("  D) 영어 vs 한국어")


def problem3_blog_length():
    print("\n" + "=" * 50)
    print("문제 3: 본 로드맵의 권장 블로그 분량")
    print("=" * 50 + "\n")

    print("질문: 본 로드맵에서 권장하는 한국어 블로그 1편의 분량은?\n")

    print("보기:")
    print("  A) 1000 자 이하 (짧고 강력)")
    print("  B) 3000 ~ 4000 자 (적정)")
    print("  C) 7000 ~ 8000 자 (자세함)")
    print("  D) 분량은 중요하지 않음")


def problem4_differentiation():
    print("\n" + "=" * 50)
    print("문제 4: 양산 SW 엔지니어의 차별화 메시지")
    print("=" * 50 + "\n")

    print("질문: RT-2 블로그에서 '박사/연구자' 블로그와 차별화될 수 있는")
    print("      가장 효과적인 section 의 내용은?\n")

    print("보기:")
    print("  A) RT-2 의 수식을 더 자세히")
    print("  B) PaLI-X 의 architecture 더 자세히")
    print("  C) 'latency 200ms 가 양산 시점에 무엇을 의미하는가' 같은")
    print("     실무 비용 관점의 해석")
    print("  D) RT-1 의 자세한 비교")


if __name__ == "__main__":
    print("=" * 50)
    print("  Phase 4 Week 3 Quiz - Easy")
    print("  RT-2 블로그 1편 작성")
    print("=" * 50)
    problem1_blog_section_priority()
    problem2_blog_quality()
    problem3_blog_length()
    problem4_differentiation()
    print("\n" + "=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
