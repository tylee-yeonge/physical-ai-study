"""
Phase 4 Week 16 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: lessons learned 예시")
    print("=" * 60 + "\n")
    examples = [
        "Phase 5 의 블로그는 outline 마감 후 본문 작성 (Phase 4 의 본문부터 시작이 재작성 비용 컸음)",
        "Phase 5 의 영상 (있다면) 은 자막 / 인트로 / 아웃트로를 별도 주에 배치",
        "Phase 5 의 모델 (ViT 등) inference 코드는 vla_inference wrapper 패턴 재사용",
        "다음 phase 의 LoRA 학습 (Phase 7) 은 0.5% 의 epoch 수 사전 검증 후 본 학습",
        "다음 phase 의 ROS2 노드는 vla_inference_node.py 의 callback 패턴 재사용",
    ]
    print("  좋은 lessons 의 패턴:")
    print("    '<다음 phase 에서> <구체 동작> <기대 효과>'")
    print()
    for i, ex in enumerate(examples, 1):
        print(f"    {i}) {ex}")
    print()
    print("  [tip] 추상적 표현 피하기:")
    print("    X) '더 잘 하자', '시간 관리 잘 하자', '집중하자'")
    print("    O) '___ 를 ___ 시점에 ___ 하기' (구체)")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: 분기 재평가 매트릭스")
    print("=" * 60 + "\n")
    decisions = [
        ("A", "지원 응답 많음, 신모델 X", 2, "본격 지원 앞당김 (2027.03)"),
        ("B", "지원 응답 많음, 신모델 등장", 4, "모델 갱신 + 원안 (2027.07)"),
        ("C", "지원 응답 적음, 신모델 X", 3, "fallback (2028.03) + Jetson"),
        ("D", "지원 응답 적음, 신모델 등장", 5, "모델 갱신 + fallback"),
    ]
    print(f"  {'시나리오':<6}{'시그널':<30}{'옵션':<6}{'결정'}")
    print(f"  {'-'*6}{'-'*30}{'-'*6}{'-'*45}")
    for code, sig, opt, decision in decisions:
        print(f"  {code:<6}{sig:<30}{opt:<6}{decision}")
    print()
    print("  [tip] '신모델 등장' 시:")
    print("    - 항상 모델 갱신 (OpenVLA -> π0 / Helix / GR00T)")
    print("    - 학습 시점 / fallback 여부는 시장 시그널 따라")
    print()
    print("  '응답 적음 + 신모델 X' 가 가장 위험:")
    print("    - 시장이 정체 또는 본인 산출물이 약함")
    print("    - fallback + Jetson 옵션으로 산출물 추가")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: Phase 5 재활용 가능 - A, B, C, D")
    print("=" * 60 + "\n")
    items = [
        ("A", "Transformer 기본", "재활용", "Phase 5 의 ViT 의 직접 토대"),
        ("B", "HuggingFace transformers", "재활용", "Phase 5 모든 모델 동일 라이브러리"),
        ("C", "DINOv2 / SigLIP 의미", "재활용", "Phase 5 의 핵심 모델"),
        ("D", "4-bit quantization", "재활용", "큰 모델 (ViT-L, CLIP-L) 에도 적용"),
        ("E", "ROS2 노드 작성", "재활용 X", "Phase 5 는 동작 원리 수준, ROS2 안 씀"),
        ("F", "ros2 bag", "재활용 X", "Phase 5 안 씀"),
    ]
    print(f"  {'Item':<6}{'기술':<25}{'Phase 5 재활용':<13}{'이유'}")
    print(f"  {'-'*6}{'-'*25}{'-'*13}{'-'*45}")
    for code, tech, reuse, why in items:
        print(f"  {code:<6}{tech:<25}{reuse:<13}{why}")
    print()
    print("  [tip] Phase 4 -> Phase 5 의 자산 활용:")
    print("    - 같은 conda env 그대로 사용 가능")
    print("    - 같은 HuggingFace cache 그대로")
    print("    - 4-bit quantization 패턴 그대로")
    print()
    print("  Phase 7 에서는 다시 ROS2 + ros2 bag 재활용 (E, F).")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
