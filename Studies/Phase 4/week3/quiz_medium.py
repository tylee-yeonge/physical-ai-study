"""
Phase 4 Week 3 - RT-2 블로그 1편 작성 중급 퀴즈

글쓰기 주차이므로 코드 대신 '핵심 단락 글쓰기' 3문제.
직접 본인 단어로 작성한 뒤 quiz_solutions/medium_sol.py 의 예시와 비교.
"""


def problem1_write_one_liner():
    """
    문제 1: RT-2 의 한 줄 요약 작성

    아래 조건을 만족하는 한 줄 요약을 작성하시오.

    조건:
      1) 50 자 이하 (공백 포함)
      2) "VLM" 또는 "Vision-Language Model" 포함
      3) "robot" 또는 "로봇" 포함
      4) "co-fine-tune" / "transfer" 등 핵심 기법 1개 이상

    TODO: my_one_liner 에 본인의 한 줄을 작성하시오.
    """
    print("\n" + "=" * 60)
    print("문제 1: RT-2 한 줄 요약")
    print("=" * 60 + "\n")

    # TODO
    my_one_liner = ""  # 여기에 본인의 한 줄

    print(f"  당신의 한 줄: '{my_one_liner}'")
    print(f"  글자 수     : {len(my_one_liner)}")

    # 자체 평가
    checks = {
        "<=50자": len(my_one_liner) <= 50 and len(my_one_liner) > 0,
        "VLM 또는 Vision-Language Model 포함": (
            "VLM" in my_one_liner or "Vision-Language" in my_one_liner
        ),
        "robot 또는 로봇 포함": (
            "robot" in my_one_liner.lower() or "로봇" in my_one_liner
        ),
        "핵심 기법 포함": any(
            kw in my_one_liner.lower()
            for kw in ["co-fine", "transfer", "토큰", "action token", "fine-tune"]
        ),
    }

    print("\n  자가 점검:")
    for k, v in checks.items():
        mark = "[O]" if v else "[X]"
        print(f"    {mark} {k}")

    if all(checks.values()):
        print("\n  [O] 모든 조건 만족!")
    else:
        print("\n  [X] 다시 작성해보세요. 예시는 quiz_solutions/medium_sol.py 참고")


def problem2_write_limitation_paragraph():
    """
    문제 2: 한계 단락 1개 작성

    "RT-2 의 inference latency 가 양산 SW 엔지니어에게 어떤 의미인가" 를
    200~300 자의 단락 1개로 작성하시오.

    조건:
      1) "200ms" 또는 비슷한 수치 포함
      2) "30Hz" / "실시간" / "안전 인터록" 중 1개 이상 언급
      3) 단순 "느리다" 가 아닌 '왜 양산에 문제인가' 의 인과 관계

    TODO: my_paragraph 에 본인의 단락을 작성.
    """
    print("\n" + "=" * 60)
    print("문제 2: RT-2 inference latency 한계 단락")
    print("=" * 60 + "\n")

    # TODO
    my_paragraph = """
    여기에 본인의 200~300자 단락을 작성하시오.
    """.strip()

    print(f"  당신의 단락 ({len(my_paragraph)}자):")
    print(f"  ---")
    print(f"  {my_paragraph}")
    print(f"  ---")

    checks = {
        "200~400자 (대략)": 200 <= len(my_paragraph) <= 400,
        "200ms 또는 수치 포함": any(
            kw in my_paragraph
            for kw in ["200ms", "200 ms", "200밀리", "0.2초", "5Hz"]
        ),
        "30Hz/실시간/안전 중 1개": any(
            kw in my_paragraph for kw in ["30Hz", "30 Hz", "실시간", "안전", "인터록"]
        ),
    }

    print("\n  자가 점검:")
    for k, v in checks.items():
        mark = "[O]" if v else "[X]"
        print(f"    {mark} {k}")

    if all(checks.values()):
        print("\n  [O] 모든 조건 만족! 예시 단락도 quiz_solutions/medium_sol.py 참고")
    else:
        print("\n  [X] 다시 작성해보세요. 예시는 quiz_solutions/medium_sol.py 참고")


def problem3_write_differentiation_sentence():
    """
    문제 3: 양산 SW 엔지니어의 차별화 한 문장

    RT-2 블로그 Section 7 에 들어갈 '본인의 차별화 메시지' 를 한 문장으로 작성.
    이 문장은 면접관에게 "이 블로그를 쓴 사람이 왜 다른가" 를 보여주는 핵심.

    조건:
      1) 80~150 자
      2) "양산" / "9년" / "AMR ROS" / "펌웨어" / "자동차 R&D 보조" 중 1개 이상
      3) RT-2 또는 VLA 의 한계 / 비용을 본인 강점으로 연결

    TODO: my_sentence 에 본인의 한 문장을 작성.
    """
    print("\n" + "=" * 60)
    print("문제 3: 양산 SW 엔지니어의 차별화 한 문장")
    print("=" * 60 + "\n")

    # TODO
    my_sentence = ""  # 여기에 본인의 한 문장

    print(f"  당신의 문장 ({len(my_sentence)}자):")
    print(f"  ---")
    print(f"  {my_sentence}")
    print(f"  ---")

    checks = {
        "80~150자": 80 <= len(my_sentence) <= 150,
        "본인 경력 키워드 1개": any(
            kw in my_sentence
            for kw in ["양산", "9년", "AMR", "ROS", "펌웨어", "자동차"]
        ),
        "RT-2/VLA 한계 연결": any(
            kw in my_sentence
            for kw in ["latency", "200ms", "quantization", "안전", "비용", "한계"]
        ),
    }

    print("\n  자가 점검:")
    for k, v in checks.items():
        mark = "[O]" if v else "[X]"
        print(f"    {mark} {k}")

    if all(checks.values()):
        print("\n  [O] 모든 조건 만족!")
    else:
        print("\n  [X] 다시 작성. 예시는 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    print("  Phase 4 Week 3 Quiz - Medium")
    print("  RT-2 블로그 작성: 글쓰기 3문제")
    print("=" * 60)
    problem1_write_one_liner()
    problem2_write_limitation_paragraph()
    problem3_write_differentiation_sentence()
    print("\n" + "=" * 60)
    print("정답 예시는 quiz_solutions/medium_sol.py 참고")
    print("=" * 60)
