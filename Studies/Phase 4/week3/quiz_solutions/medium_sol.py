"""
Phase 4 Week 3 - RT-2 블로그 글쓰기 중급 퀴즈 정답 (예시)
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답 예시: RT-2 한 줄 요약")
    print("=" * 60 + "\n")

    examples = [
        "Web 으로 학습된 VLM 을 robot action 토큰까지 co-fine-tune 한 모델.",
        "VLM 의 web 지식이 robot 행동으로 transfer 되는 첫 대규모 증명.",
        "Pre-trained VLM + action token 의 co-fine-tune 으로 emergent 로봇 능력.",
    ]

    print("  예시 (50자 이하):")
    for i, ex in enumerate(examples, 1):
        print(f"    {i}) ({len(ex):>2}자) '{ex}'")

    print("\n  [tip] 한 줄 요약 작성 시 체크:")
    print("       - 모델의 본질 (what) + 핵심 기법 (how) + 영향 (why) 의 균형")
    print("       - 'RT-2 는 ___ 인 모델' 의 빈칸 채우기 식으로 시작")
    print("       - 50자 안에 한국어로 모두 담아내는 게 어렵지만 면접 시 빠른 임팩트")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답 예시: latency 한계 단락")
    print("=" * 60 + "\n")

    example = """
RT-2 는 5B/55B VLM 을 backbone 으로 쓰기 때문에 inference latency 가
약 200ms (5Hz) 수준이다. 양산 SW 관점에서 이는 두 가지 문제를 만든다.
첫째, 일반적 로봇 제어가 요구하는 30Hz 의 실시간 폐쇄 루프 제어가
불가능하다. 둘째, latency 가 큰 만큼 명령과 동작 사이의 시간 격차가
커져 안전 인터록 (위치/속도/토크 한계) 의 reaction time 이 짧아지고,
충돌 회피의 마진이 줄어든다. 즉 양산에서는 RT-2 같은 'slow VLA' 와
별도의 'fast safety policy' 를 함께 두는 hierarchical 구조가
필수가 된다.
""".strip()

    print(f"  예시 ({len(example)}자):")
    print(f"  ---")
    print(f"  {example}")
    print(f"  ---")

    print("\n  [tip] 좋은 단락의 패턴:")
    print("       1) 사실 (200ms latency)")
    print("       2) 영향 1 (실시간 30Hz 불가)")
    print("       3) 영향 2 (안전 인터록 marginal time 짧음)")
    print("       4) 결론 (hierarchical 구조 필요)")
    print()
    print("       'fact -> impact -> conclusion' 의 3 단계가 좋은 단락의 골격.")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답 예시: 양산 SW 엔지니어 차별화 한 문장")
    print("=" * 60 + "\n")

    examples = [
        "AMR ROS 양산 실무 5년 경험에서 봤을 때, RT-2 의 200ms latency 와 "
        "0.78mm quantization step 은 안전 인터록과 hierarchical "
        "structure 없이는 양산 시점에 그대로 사용할 수 없다.",

        "펌웨어 (상용차 클러치 반자동화) 와 AMR ROS 경력 관점에서, VLA 의 closed-source "
        "weight 와 큰 GPU 의존성은 단순한 기술적 결함이 아니라 양산 비용 "
        "구조의 본질적 변화를 의미한다.",

        "AMR 양산 ROS 5년 동안 노드의 5ms latency 와 싸워온 입장에서, "
        "RT-2 의 200ms 는 '느린 brain + 빠른 body' hierarchical 구조 "
        "설계가 필수임을 명확히 보여준다.",
    ]

    print("  예시 (80~150자):")
    for i, ex in enumerate(examples, 1):
        print(f"    {i}) ({len(ex):>3}자)")
        print(f"       '{ex}'")
        print()

    print("  [tip] 차별화 한 문장의 패턴:")
    print("       1) 본인 경력 (AMR ROS 5년 / 펌웨어)")
    print("       2) 그 경력에서 보이는 RT-2 의 한계")
    print("       3) 이 한계가 양산에서 의미하는 구조적 결론")
    print()
    print("       이 한 문장이 본인의 경력이 응축된 면접 hook.")
    print("       정성껏 다듬을 가치가 있다.")


if __name__ == "__main__":
    print("=" * 60)
    print("  Phase 4 Week 3 Quiz - Medium 정답 예시")
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "=" * 60)
