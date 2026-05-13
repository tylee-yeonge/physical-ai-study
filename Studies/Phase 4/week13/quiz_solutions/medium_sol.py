"""
Phase 4 Week 13 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: cross-link section 예시")
    print("=" * 60 + "\n")

    example = """
이 글에서는 RT-2 의 학술적 흐름과 한계를 정리했습니다. 다음 글에서는
RT-2 의 open-source 버전 OpenVLA 를 본인 RTX 4070 환경에서 직접
inference 해 본 결과를 다룹니다. 165 ms latency 와 6 Hz throughput
의 양산 의미까지 함께 정리.

[다음 글: OpenVLA + RTX 4070 실측 - 5Hz 가 양산에서 의미하는 것](https://velog.io/...)
""".strip()

    print(example)
    print(f"\n  ({len(example)}자)")
    print()
    print("  [tip] cross-link section 의 구조:")
    print("    1) 이 글의 요약 (1문장)")
    print("    2) 다음 글의 주제 (1문장)")
    print("    3) 핵심 수치/메시지 미리 보기 (1문장)")
    print("    4) 명확한 link")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: C")
    print("=" * 60 + "\n")
    print("  C) 'RT-2 정독 노트: VLM 이 어떻게 로봇 행동을 생성하는가'")
    print()
    print("  분석:")
    print("    A) 'RT-2' - 너무 짧음, 맥락 없음")
    print("    B) 'RT-2 정독 노트' - 부족, 핵심 메시지 없음")
    print("    C) '<주제>: <메시지>' - SEO + 진입점 모두 우수")
    print("    D) 영어 + 분량 길음 - 한국 검색에 약함")
    print()
    print("  좋은 제목의 조건:")
    print("    - 핵심 단어 (RT-2 / OpenVLA / VLA) 포함")
    print("    - 한 줄로 '이 글이 무엇에 대한가' 명확")
    print("    - 검색 가능한 단어 + 매력적인 메시지")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: a 또는 b (일관성이 핵심)")
    print("=" * 60 + "\n")
    print("  권장 표기:")
    print("    a) '165 ms' (공백, SI 표준)")
    print("    b) '165ms' (붙임, 일반적)")
    print()
    print("  비권장:")
    print("    c) '약 165ms' - '약' 이 빠진 표현이 더 자신감 있음")
    print("    d) '165milliseconds' - 너무 김")
    print("    e) '0.165 초' - ms 표기가 일반적")
    print()
    print("  [tip] 본 phase 권장: 'b (165ms)' - 한국어 글 일반 관례.")
    print("       단 한 글 안에서 일관성이 가장 중요.")
    print()
    print("  글 안에서 일관성:")
    print("    - 모든 latency: 'ms' 로")
    print("    - 모든 메모리: 'GB' 로")
    print("    - 모든 frequency: 'Hz' 로")
    print("    - 모든 시간: 's' 또는 '초' 한 가지로")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
