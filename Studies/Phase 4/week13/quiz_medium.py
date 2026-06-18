"""
Phase 4 Week 13 - 중급 퀴즈: 퇴고
"""


def problem1_write_cross_link_section():
    """
    문제 1: cross-link section 작성

    RT-2 블로그 의 끝부분 (Section 8: 다음) 에 들어갈 cross-link section
    한 단락 (50~100자) + link 마크다운 작성.

    TODO: my_section 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 1: cross-link section 작성")
    print("=" * 60 + "\n")

    # TODO
    my_section = ""

    print("당신의 cross-link section:")
    print(my_section)
    print()
    checks = {
        "다음 글 link 포함": "다음 글" in my_section or "https" in my_section,
        "50~200자": 50 <= len(my_section) <= 200,
        "OpenVLA 언급": "OpenVLA" in my_section,
    }
    for k, v in checks.items():
        mark = "[O]" if v else "[X]"
        print(f"  {mark} {k}")


def problem2_seo_title():
    """
    문제 2: 제목의 SEO 최적화

    아래 후보 제목 중 SEO 와 면접관 진입 효과가 가장 큰 것은?

    A) "RT-2"
    B) "RT-2 정독 노트"
    C) "RT-2 정독 노트: VLM 이 어떻게 로봇 행동을 생성하는가"
    D) "Robotics Transformer 2 Paper Reading"
    """
    print("\n" + "=" * 60)
    print("문제 2: 제목 SEO 최적화")
    print("=" * 60 + "\n")

    answer = ""  # TODO

    print(f"  당신의 답: {answer}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem3_metric_consistency():
    """
    문제 3: 정량 표현 일관성

    아래 표현 중 권장 / 비권장:

    a) "165 ms" (공백)
    b) "165ms" (붙임)
    c) "약 165ms"
    d) "165milliseconds"
    e) "0.165 초"

    SI 단위 + 영어 + 정확한 형식이 권장:
    - 권장 표기 1개를 선택.
    - 그리고 본 블로그 안에서 이 표기를 일관되게.
    """
    print("\n" + "=" * 60)
    print("문제 3: 정량 표현 일관성")
    print("=" * 60 + "\n")

    # TODO
    preferred = ""

    print(f"  당신의 권장 표기: {preferred}")
    print()
    print("  [tip] 핵심은 어느 표기를 고르든 글 전체에서 일관되게 쓰는 것.")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    problem1_write_cross_link_section()
    problem2_seo_title()
    problem3_metric_consistency()
    print("=" * 60)
