"""
Phase 4 Week 7 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) 실측 데이터 포함")
    print("=" * 50 + "\n")
    print("해설:")
    print("  RT-2 블로그 (week 3): 논문 정독만, latency 추정")
    print("  OpenVLA 블로그 (week 7): 본인 RTX 4070 4-bit 직접 측정한 latency")
    print()
    print("  이 차별점이 면접관에게 가장 큰 신뢰감 차이:")
    print("    - RT-2 블로그   : '잘 정리했네'")
    print("    - OpenVLA 블로그: '직접 돌려봤구나, 시간을 들여 검증했구나'")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) Section 5 실측 결과 표")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Section 5 (결과 / 사례) 가 실측 데이터의 자리.")
    print("    - 5-1: OpenVLA 논문의 평가 결과 (인용)")
    print("    - 5-2: 본인 측정 결과 (latency 표)")
    print("    - 5-3: 그 수치의 의미 (5Hz 가 어떤 작업까지 가능한가)")
    print()
    print("  5-2 가 본 블로그의 핵심. RT-2 블로그에는 없는 부분.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) cm 단위 pick-and-place")
    print("=" * 50 + "\n")
    print("해설:")
    print("  5~6 Hz = 매 200ms 마다 새 action.")
    print()
    print("  가능한 작업:")
    print("    - cm 단위 manipulation (10 cm/s = 2 cm/step)")
    print("    - pick-and-place (정적 환경)")
    print("    - 명령 기반 navigation")
    print()
    print("  불가능한 작업:")
    print("    - 60Hz 실시간 폐쇄 루프 (catching, juggling)")
    print("    - mm 단위 정밀 조립 (자동차 부품 tolerance ~0.1mm)")
    print("    - 실시간 force / impedance control")
    print()
    print("  -> hierarchical 구조 (slow VLA + fast safety) 필수.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) 1순위 회사 채용 담당자 + 면접관")
    print("=" * 50 + "\n")
    print("해설:")
    print("  본 로드맵에서 블로그의 역할:")
    print("    - 산출물 #2 의 1/3 (블로그 2편)")
    print("    - 면접관의 진입점")
    print("    - 본인의 차별화 메시지 전달")
    print()
    print("  1순위 회사 (마음AI WoRV / 카카오모빌리티 VLA / 휴머노이드 스타트업) 의")
    print("  채용 담당자가 후보자 이름을 구글링했을 때 가장 먼저 나와야 할 글.")
    print()
    print("  따라서:")
    print("    - 한국어 우선 (Velog)")
    print("    - VLA / OpenVLA 키워드 노출")
    print("    - 양산 SW 엔지니어 톤")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
