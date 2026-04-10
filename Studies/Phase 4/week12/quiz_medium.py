"""
Phase 6 Week 12 - 블로그 & 영상 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""


def problem1_blog_outline():
    """
    문제 1: 블로그 포스팅 아웃라인 작성

    "KITTI 3D Detection 입문기" 블로그의 아웃라인을 작성하시오.
    각 섹션의 핵심 내용과 예상 분량을 포함하시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: 블로그 아웃라인 작성")
    print("━" * 36 + "\n")

    print("  주제: 'KITTI 3D Detection 입문기'\n")

    print("  과제:")
    print("  1) 블로그 제목을 작성하시오 (눈길을 끄는 제목)")
    print("  2) 5개 이상의 섹션을 구성하시오")
    print("  3) 각 섹션의 핵심 내용을 1~2줄로 요약하시오")
    print("  4) 포함할 시각 자료 목록을 나열하시오")
    print()

    print("  예시 (채워 넣으세요):")
    print("  ─────────────────────────────")
    print("  제목: _______________")
    print()
    print("  1. 도입: _______________")
    print("  2. 배경 지식: _______________")
    print("  3. 구현 과정: _______________")
    print("  4. 결과 분석: _______________")
    print("  5. 마무리: _______________")
    print()
    print("  시각 자료:")
    print("  - _______________")
    print("  - _______________")
    print("  - _______________")


def problem2_video_time_allocation():
    """
    문제 2: 영상 시간 배분 설계

    6분짜리 Demo 영상의 시간 배분을 설계하시오.
    각 섹션의 시간, 화면 구성, 핵심 메시지를 포함하시오.
    """
    print("\n" + "━" * 36)
    print("문제 2: 영상 시간 배분 설계")
    print("━" * 36 + "\n")

    print("  총 길이: 6분\n")

    print("  과제:")
    print("  1) 5개 이상의 섹션으로 나누시오")
    print("  2) 각 섹션의 시간(초)을 배분하시오 (합계 = 360초)")
    print("  3) 각 섹션의 화면 구성을 설명하시오")
    print("  4) 각 섹션의 핵심 메시지를 작성하시오")
    print()

    total_seconds = 360
    print(f"  총 배분 가능 시간: {total_seconds}초 ({total_seconds // 60}분)\n")

    print("  예시 (채워 넣으세요):")
    print("  ─────────────────────────────")
    print("  섹션 1: 인트로")
    print("    시간: ___ 초")
    print("    화면: _______________")
    print("    메시지: _______________")
    print()
    print("  섹션 2: _______________")
    print("    시간: ___ 초")
    print("    화면: _______________")
    print("    메시지: _______________")
    print()
    print("  ...")
    print()
    print(f"  합계: ___ / {total_seconds} 초")


def problem3_interview_prep():
    """
    문제 3: 면접 답변 작성

    아래 3개 면접 질문에 대해 STAR 프레임워크로 답변을 작성하시오.
    각 답변은 2분 이내로 말할 수 있는 분량이어야 한다.
    """
    print("\n" + "━" * 36)
    print("문제 3: 면접 답변 작성")
    print("━" * 36 + "\n")

    questions = [
        "3D Detection 프로젝트에서 가장 어려웠던 점은?",
        "BEV 표현의 장점을 3가지 설명하세요.",
        "이 프로젝트 경험을 실무에 어떻게 적용하시겠습니까?",
    ]

    for i, q in enumerate(questions, 1):
        print(f"  Q{i}. {q}\n")
        print(f"  STAR 답변:")
        print(f"    S (상황): _______________")
        print(f"    T (과제): _______________")
        print(f"    A (행동): _______________")
        print(f"    R (결과): _______________")
        print()

    print("  기준:")
    print("  - 각 답변 200~300자 이내")
    print("  - 구체적인 수치 포함 (AP3D, NDS 등)")
    print("  - 소리 내어 2분 이내에 말할 수 있어야 함")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 12 Quiz - Medium")
    print("━" * 40)
    problem1_blog_outline()
    problem2_video_time_allocation()
    problem3_interview_prep()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
