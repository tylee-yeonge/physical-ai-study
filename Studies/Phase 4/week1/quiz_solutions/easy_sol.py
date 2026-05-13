"""
Phase 4 Week 1 - RT-2 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: C) PaLI-X 또는 PaLM-E (5B/55B VLM)")
    print("=" * 50 + "\n")

    print("해설:")
    print("  RT-2 는 두 가지 backbone variant 로 학습되었다:")
    print("    1. RT-2-PaLI-X (5B / 55B)")
    print("    2. RT-2-PaLM-E (12B / 562B)")
    print()
    print("  두 모델 모두 web-scale 로 사전학습된 large VLM 이다.")
    print("  RT-1 (35M parameter, from scratch) 와의 가장 큰 차이.")
    print()
    print("  [tip] BERT (text only), CLIP (image-text contrastive 만), GPT-3.5 (text only)")
    print("       는 모두 VLM 으로서의 generation 능력이 없거나 부족하여")
    print("       RT-2 의 backbone 으로 부적합하다.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) VLM vocab 의 마지막 256 개 토큰을 action bin 으로 재사용")
    print("=" * 50 + "\n")

    print("해설:")
    print("  RT-2 의 가장 영리한 설계 결정:")
    print()
    print("  - VLM 은 'next token prediction' 만 한다.")
    print("  - Action 도 token 으로 표현하면 별도의 action head 불필요.")
    print("  - VLM 의 vocab 마지막 256 개를 action discrete bin 으로 재해석.")
    print()
    print("  결과: VLM 의 표준 generation 흐름이 그대로 action 생성 흐름이 됨.")
    print("  훈련 시에도 단순히 next-token-prediction loss 로 일관됨.")
    print()
    print("  [tip] 이 디자인이 'co-fine-tuning' 을 가능케 한다.")
    print("       web data (caption / VQA) 와 robot data 가 동일한 loss 로 학습 가능.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: C) Robot data 만 학습하면 catastrophic forgetting 발생")
    print("=" * 50 + "\n")

    print("해설:")
    print("  Robot data (130k episodes from RT-1) 는 web data (수십억 image-text pair)")
    print("  대비 압도적으로 적다.")
    print()
    print("  Robot data 만 fine-tune 하면:")
    print("    - VLM 이 robot 명령에 과적합")
    print("    - 'red can' 같은 색상 이해, 'almost-empty' 같은 의미 추론이 사라짐")
    print("    - Emergent capability 완전 소실")
    print()
    print("  Co-fine-tuning (web : robot = 8 : 2 정도) 으로:")
    print("    - VLM 의 일반 visual reasoning 능력 보존")
    print("    - Action 생성 능력 추가 습득")
    print()
    print("  [tip] 이 비율은 Phase 5 의 'instruction tuning' 의 일반 패턴과도 같다.")
    print("       원래 능력을 유지하면서 새로운 능력을 더할 때 쓰는 표준 기법.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) 학습 데이터에 없는 의미 추론 명령을 수행")
    print("=" * 50 + "\n")

    print("해설:")
    print("  Emergent capability 는 '학습 데이터에 명시적으로 없는 것을 수행' 하는 능력.")
    print()
    print("  RT-2 의 대표 사례 4 가지 (논문 Sec 5):")
    print("    1. Symbol understanding: 'arrow' 를 가리키며 가르킨 방향 인식")
    print("    2. Reasoning: 'pick up the almost-empty can' (VQA 적 표현)")
    print("    3. Human recognition: 'pick up Taylor Swift' (사람 인식)")
    print("    4. Math: '5 - 3' 적힌 cue 카드를 보고 2 위치로 이동")
    print()
    print("  보기 A 는 '학습된 명령을 정확히 수행' 이므로 emergent 가 아니다.")
    print("  보기 C 는 RT-2 가 RT-1 보다 느리다는 사실 (latency ~200ms)")
    print("       와 모순이며 emergent 와 무관.")
    print("  보기 D 는 RT-2 의 명시적 한계 중 하나 (single embodiment) 이므로 거짓.")
    print()
    print("  [tip] week 3 의 RT-2 블로그에서 emergent 사례 4 개를 한 줄씩 인용할 것.")


if __name__ == "__main__":
    print("=" * 50)
    print("  Phase 4 Week 1 Quiz - Easy 정답")
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "=" * 50)
