"""
Phase 4 Week 2 - Co-fine-tuning + Action tokenization 기초 퀴즈
"""


def problem1_vocab_position():
    print("\n" + "=" * 50)
    print("문제 1: 왜 vocab 의 마지막 256개를 쓰는가")
    print("=" * 50 + "\n")

    print("질문: RT-2 가 action bin 으로 vocab 의 마지막 256개를")
    print("      재사용하는 가장 큰 이유는?\n")

    print("보기:")
    print("  A) GPU 메모리 최적화 (마지막 token 이 SRAM 에 가까움)")
    print("  B) 그 token 들은 web 학습 데이터에서 가장 빈도가 낮은")
    print("     'rare token' 이라, 원 의미를 덮어써도 web 성능 영향이 최소")
    print("  C) Tokenizer 의 구현 상 마지막만 추가 가능")
    print("  D) 마지막 token 들이 의미가 가장 명확함")


def problem2_data_mixture():
    print("\n" + "=" * 50)
    print("문제 2: Co-fine-tuning 의 데이터 mixture")
    print("=" * 50 + "\n")

    print("질문: RT-2 의 학습 mini-batch 에 들어가는 데이터 비율로")
    print("      가장 가까운 것은?\n")

    print("보기:")
    print("  A) Web 100% (robot data 는 별도 phase 로 학습)")
    print("  B) Web 50% + Robot 50% (균형)")
    print("  C) Web 80% + Robot 20% (web 이 다수)")
    print("  D) Web 20% + Robot 80% (robot 이 다수)")


def problem3_loss_uniformity():
    print("\n" + "=" * 50)
    print("문제 3: Web sample 과 Robot sample 의 loss 형태")
    print("=" * 50 + "\n")

    print("질문: RT-2 학습 시 web sample 과 robot sample 의 loss 는?\n")

    print("보기:")
    print("  A) 서로 다른 loss (web 은 contrastive, robot 은 imitation)")
    print("  B) 동일한 standard next-token-prediction cross-entropy")
    print("  C) Web 은 reconstruction, robot 은 cross-entropy")
    print("  D) 별도 모델 head 에 별도 loss")


def problem4_action_quantization():
    print("\n" + "=" * 50)
    print("문제 4: Action Quantization step 의 직관")
    print("=" * 50 + "\n")

    print("질문: action dx 의 범위가 [-0.1, +0.1] m 이고 256 bin 으로")
    print("      양자화될 때, RT-2 가 표현 가능한 가장 작은 dx 변화는?\n")

    print("보기:")
    print("  A) 약 0.04 mm")
    print("  B) 약 0.78 mm")
    print("  C) 약 1.0 mm")
    print("  D) 약 7.8 mm")


if __name__ == "__main__":
    print("=" * 50)
    print("  Phase 4 Week 2 Quiz - Easy")
    print("  Co-fine-tuning + Action Tokenization")
    print("=" * 50)
    problem1_vocab_position()
    problem2_data_mixture()
    problem3_loss_uniformity()
    problem4_action_quantization()
    print("\n" + "=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
