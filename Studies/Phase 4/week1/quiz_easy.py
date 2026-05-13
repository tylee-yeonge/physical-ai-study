"""
Phase 4 Week 1 - RT-2 1회독 + Architecture 기초 퀴즈

논문 1회독 후 핵심 개념이 정확하게 잡혔는지 확인하는 4문제.
"""


def problem1_rt2_backbone():
    print("\n" + "=" * 50)
    print("문제 1: RT-2 의 backbone")
    print("=" * 50 + "\n")

    print("질문: RT-2 가 backbone 으로 사용한 모델은?\n")

    print("보기:")
    print("  A) BERT-large")
    print("  B) CLIP ViT-L/14")
    print("  C) PaLI-X 또는 PaLM-E (5B/55B VLM)")
    print("  D) GPT-3.5")


def problem2_action_tokenization():
    print("\n" + "=" * 50)
    print("문제 2: Action Tokenization")
    print("=" * 50 + "\n")

    print("질문: RT-2 는 robot action 을 어떻게 표현하는가?\n")

    print("보기:")
    print("  A) 별도의 action head (MLP) 가 7-DoF continuous 값을 직접 출력")
    print("  B) VLM vocabulary 의 마지막 256 개 토큰을 action discrete bin 으로 재사용")
    print("  C) 7 개의 별도 classification head 를 추가")
    print("  D) action 을 image 로 변환하여 vision token 으로 출력")


def problem3_cofinetuning():
    print("\n" + "=" * 50)
    print("문제 3: Co-fine-tuning 의 목적")
    print("=" * 50 + "\n")

    print("질문: RT-2 가 robot data 만으로 fine-tune 하지 않고")
    print("      web data 와 함께 co-fine-tune 하는 가장 큰 이유는?\n")

    print("보기:")
    print("  A) robot data 가 부족하기 때문")
    print("  B) Web data 만 학습하면 robot 이 동작하지 않음")
    print("  C) Robot data 만 학습하면 VLM 의 web knowledge 가 사라짐 (catastrophic forgetting)")
    print("  D) GPU 메모리 절약을 위해")


def problem4_emergent_capability():
    print("\n" + "=" * 50)
    print("문제 4: Emergent Capability")
    print("=" * 50 + "\n")

    print("질문: RT-2 의 emergent capability 의 예시로 가장 적절한 것은?\n")

    print("보기:")
    print("  A) 학습 데이터에 있는 'pick up the can' 명령을 99% 정확도로 수행")
    print("  B) 학습 데이터에 없는 'pick up the almost-empty can' 같은")
    print("     의미 추론이 필요한 명령을 수행")
    print("  C) 추론 속도가 RT-1 보다 빠름")
    print("  D) 다른 로봇 hardware 에서도 그대로 동작")


if __name__ == "__main__":
    print("=" * 50)
    print("  Phase 4 Week 1 Quiz - Easy")
    print("  RT-2 정독 + Architecture")
    print("=" * 50)
    problem1_rt2_backbone()
    problem2_action_tokenization()
    problem3_cofinetuning()
    problem4_emergent_capability()
    print("\n" + "=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
