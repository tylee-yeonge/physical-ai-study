"""
Phase 4 Week 16 - 중급 퀴즈
"""


def problem1_phase4_lessons():
    """
    문제 1: Phase 4 의 lessons learned 작성

    Phase 4 의 16주를 돌아보며 다음에 적용할 구체적 action 3 개를 작성.

    조건:
      - 추상적 표현 (예: "더 잘 하자") 안 됨
      - 구체적 행동 (예: "다음 phase 에서는 ___ 를 ___ 시점에 ___ 하기")

    TODO: my_lessons 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 1: Phase 4 lessons learned")
    print("=" * 60 + "\n")

    # TODO
    my_lessons = [
        "",  # action 1
        "",  # action 2
        "",  # action 3
    ]

    examples = [
        "다음 phase 의 블로그는 outline 먼저 마감 후 본문 작성 (Phase 4 에서 본문부터 시작해 outline 부재로 재작성 발생)",
        "다음 phase 의 영상은 자막 / 인트로 / 아웃트로를 별도 주에 배치 (Phase 4 에서 합쳐서 시간 부족)",
        "다음 phase 의 ROS2 통합은 vla_inference wrapper 패턴 그대로 재사용 (3주 절약)",
    ]

    print("  당신의 lessons:")
    for i, l in enumerate(my_lessons, 1):
        print(f"    {i}) {l}")

    print("\n  예시 (구체적 action 패턴):")
    for ex in examples:
        print(f"    - {ex}")


def problem2_quarterly_decision():
    """
    문제 2: 2026.11 분기 재평가의 판단 매트릭스

    아래 4 가지 시그널 조합에 대해 결정:

      시그널 1: 정찰 지원 응답률
        a) 30%+ (많음)
        b) < 10% (적음)

      시그널 2: OpenVLA 후속 모델 (π0 / Helix / GR00T)
        a) 등장 (한 세대 차이)
        b) 미등장 (OpenVLA 표준 유지)

    네 가지 조합에 대한 결정:
      A) (지원 응답 많음, 모델 신모델 미등장): ?
      B) (지원 응답 많음, 신모델 등장): ?
      C) (지원 응답 적음, 신모델 미등장): ?
      D) (지원 응답 적음, 신모델 등장): ?

    옵션:
      1) 원안 고수 (2027.07 본격 지원)
      2) 본격 지원 앞당김 (2027.03)
      3) fallback (2028.03) + Jetson 옵션
      4) 모델 갱신 후 원안 고수
      5) 모델 갱신 + fallback

    TODO: 각 조합의 결정.
    """
    print("\n" + "=" * 60)
    print("문제 2: 분기 재평가 의사결정 매트릭스")
    print("=" * 60 + "\n")

    # TODO
    decisions = {"A": 0, "B": 0, "C": 0, "D": 0}

    expected = {
        "A": 2,  # 지원 응답 많음 + 신모델 X -> 앞당김
        "B": 4,  # 지원 응답 많음 + 신모델 등장 -> 모델 갱신 + 원안
        "C": 3,  # 지원 응답 적음 + 신모델 X -> fallback
        "D": 5,  # 지원 응답 적음 + 신모델 등장 -> 모델 갱신 + fallback
    }

    print("  당신의 결정:")
    for k, v in decisions.items():
        mark = "[O]" if v == expected[k] else "[X]"
        print(f"    {mark} {k}: {v}  (기대: {expected[k]})")


def problem3_phase5_prep():
    """
    문제 3: Phase 5 진입 직전 점검

    Phase 5 (Foundation Model: ViT / CLIP / DINOv2 / SigLIP) 진입 전에
    Phase 4 에서 이미 다룬 사전 지식 중 활용 가능한 것을 모두 고르시오.

    A) Transformer 기본 (decoder)
    B) HuggingFace transformers 라이브러리
    C) DINOv2 / SigLIP 의 hybrid vision encoder 의미 (OpenVLA 정독)
    D) 4-bit quantization (bitsandbytes)
    E) ROS2 노드 작성
    F) ros2 bag 사용

    TODO: 활용 가능한 것을 모두 'A,B,...' 형태로.
    """
    print("\n" + "=" * 60)
    print("문제 3: Phase 5 에서 활용 가능한 Phase 4 자산")
    print("=" * 60 + "\n")

    # TODO
    reusable = ""

    expected = "A,B,C,D"  # E, F 는 Phase 5 에서 직접 안 씀

    print(f"  당신의 답 : {reusable}")
    print(f"  기대 답   : {expected}")
    print()
    print("  분석:")
    print("    A) Transformer 기본 - Phase 5 ViT 의 직접 토대")
    print("    B) HuggingFace transformers - Phase 5 그대로 사용")
    print("    C) DINOv2 / SigLIP - Phase 5 의 핵심 모델")
    print("    D) 4-bit quantization - Phase 5 의 큰 모델에도 적용")
    print("    E) ROS2 노드 - Phase 5 는 동작 원리 수준, ROS2 안 씀")
    print("    F) ros2 bag - Phase 5 안 씀")


if __name__ == "__main__":
    print("=" * 60)
    problem1_phase4_lessons()
    problem2_quarterly_decision()
    problem3_phase5_prep()
    print("=" * 60)
