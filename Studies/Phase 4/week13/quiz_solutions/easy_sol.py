"""
Phase 4 Week 13 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: A) SEO + 면접관 양쪽 다 읽을 확률 ↑")
    print("=" * 50 + "\n")
    print("해설:")
    print("  cross-link 의 효과:")
    print("    1. SEO: 서로 link 된 글은 'site authority' 가 강화")
    print("    2. UX: 면접관이 한 글 읽고 다음 글로 자연스럽게")
    print("    3. 'series' 인식: 두 글이 한 작품으로 보임")
    print()
    print("  본 phase 의 1순위 회사 면접관은 한국어 검색:")
    print("    'OpenVLA RTX 4070' -> 본인 블로그 노출")
    print("    -> 두 글 모두 읽고 '심도 있는 후보' 인상")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) Mermaid theme + 노드 모양 + direction")
    print("=" * 50 + "\n")
    print("해설:")
    print("  다이어그램 통일의 가장 큰 효과:")
    print("    - 두 글이 한 시리즈로 인식")
    print("    - 면접관에게 '꼼꼼한 사람' 인상")
    print()
    print("  통일 우선순위:")
    print("    1. Mermaid theme (default / forest / dark)")
    print("    2. direction (LR 또는 TB 한 가지)")
    print("    3. 노드 모양 (rectangle 기본)")
    print("    4. 색상 / 강조 (특정 노드만)")
    print()
    print("  중요도 낮음: 폰트 크기 / 정확한 노드 size")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: A) 제목")
    print("=" * 50 + "\n")
    print("해설:")
    print("  SEO 영향력 순위:")
    print("    1. 제목 (가장 큼): 검색 결과의 첫 줄")
    print("    2. URL slug: 검색엔진의 분석 단위")
    print("    3. 태그: 카테고리 / 관련글")
    print("    4. 첫 단락 (description): 검색 결과의 snippet")
    print("    5. 본문 길이: 내용 깊이의 지표")
    print()
    print("  좋은 제목의 구조:")
    print("    '<모델/주제명>: <메시지 요약>' 또는 '<숫자> + <메시지>'")
    print()
    print("  예: 'RT-2 정독 노트: VLM 이 어떻게 로봇 행동을 생성하는가'")
    print("     'OpenVLA 정독 + RTX 4070 실측: 5Hz 가 양산에서 의미하는 것'")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) RT-2 학술적 / OpenVLA 실측")
    print("=" * 50 + "\n")
    print("해설:")
    print("  두 블로그가 함께 던지는 메시지:")
    print("    'VLA 의 학술적 흐름 (RT-2) -> open-source 검증 (OpenVLA)")
    print("     -> 양산 SW 관점 (latency 165ms, 6Hz 의 의미)'")
    print()
    print("  RT-2 만 있으면: 학술적이지만 'paper 만 본 사람'")
    print("  OpenVLA 만 있으면: 실용적이지만 '맥락 모르는 사람'")
    print("  두 글 함께: '맥락 + 실용 + 양산 해석' 의 완전한 그림")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
