"""
Phase 4 Week 3 - RT-2 블로그 1편 작성 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) Section 1 (한 줄 요약) + Section 3 (한 페이지)")
    print("=" * 50 + "\n")

    print("해설:")
    print("  면접관의 블로그 평가 순서:")
    print()
    print("    0~30초: Section 1 (한 줄 요약) + 첫 단락")
    print("           -> 글의 주제 / 가치 / 깊이를 빠르게 판단")
    print()
    print("    30초~3분: Section 3 (한 페이지 요약) + 다이어그램")
    print("           -> 글의 구조 / 핵심 / 시각 자료 평가")
    print()
    print("    3분~10분: Section 4 (자세한 동작) / Section 6 (한계)")
    print("           -> 깊이와 정직함 평가")
    print()
    print("  [tip] Section 1, 3 에 정성을 들이면 면접관이 글을 끝까지 읽을 확률 ↑.")
    print("       Section 4 부터 정성을 들여도 면접관이 거기까지 안 가면 무용.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: C) 한계 / 비판 section 의 유무와 깊이")
    print("=" * 50 + "\n")

    print("해설:")
    print("  나쁜 블로그의 공통 패턴:")
    print("    - '혁신적' / '획기적' 같은 과장")
    print("    - 모델의 한계를 언급 안 함")
    print("    - abstract 의 거의 직역")
    print()
    print("  좋은 블로그의 공통 패턴:")
    print("    - 정직한 한계 5가지")
    print("    - 본인 해석 한 줄 추가")
    print("    - 다른 모델과의 비교")
    print()
    print("  면접관 관점:")
    print("    - '단점을 정직하게 보는 능력' = '비판적 사고' 의 척도")
    print("    - 양산 환경의 SW 엔지니어에게 필수 능력")
    print("    - 모든 결정에는 trade-off 가 있다는 사실을 아는가")
    print()
    print("  [tip] RT-2 의 한계 5 가지 (week 1 README):")
    print("       latency / closed-source / quantization / single-arm / VRAM")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) 3000 ~ 4000 자")
    print("=" * 50 + "\n")

    print("해설:")
    print("  분량 별 trade-off:")
    print()
    print("    1000 자 이하 (A):")
    print("      - 깊이 부족, '훑어본 글' 느낌")
    print("      - 면접관: '이 사람이 이해한 깊이가 의심된다'")
    print()
    print("    3000 ~ 4000 자 (B): <- 권장")
    print("      - 8 section 표준 구조에 적정")
    print("      - 면접관이 끝까지 읽는 한계")
    print("      - 본인 해석 + 수치 + 다이어그램의 균형")
    print()
    print("    7000 자 이상 (C):")
    print("      - 면접관이 다 안 읽음")
    print("      - 핵심이 흐려짐")
    print("      - 차라리 글 2 편으로 나누는 게 나음")
    print()
    print("  [tip] 표 / 다이어그램은 글자 수에 포함 안 됨.")
    print("       표 / 다이어그램으로 본문 대체 가능하면 적극 활용.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: C) latency 의 실무 비용 관점 해석")
    print("=" * 50 + "\n")

    print("해설:")
    print("  박사 / 연구자가 잘 쓰는 영역:")
    print("    - 수식 / architecture 분석")
    print("    - 다른 모델과의 학술적 비교")
    print("    - 후속 연구 방향")
    print()
    print("  양산 SW 엔지니어가 잘 쓰는 영역:")
    print("    - latency 가 실제 제품에 미치는 영향")
    print("    - 안전 인터록 설계")
    print("    - 양산 비용 (GPU VRAM / 메모리 / 전력)")
    print("    - hardware 부착 / 캘리브레이션 / 환경")
    print()
    print("  RT-2 블로그에서 차별화하려면:")
    print("    'latency 200ms 는 30Hz 실시간 제어 불가, 즉 안전 인터록과 함께")
    print("     별도의 low-frequency policy 와 결합되어야 양산 시점에 사용 가능' 같은")
    print("    '실무 인과 관계' 가 박사 블로그에는 거의 없다.")
    print()
    print("  [tip] 이 한 문단이 본인의 경력 (AMR ROS 실무 5년 + 펌웨어 2.5년) 의")
    print("       유일한 차별화 메시지가 된다. 정성껏 작성.")


if __name__ == "__main__":
    print("=" * 50)
    print("  Phase 4 Week 3 Quiz - Easy 정답")
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "=" * 50)
