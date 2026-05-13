"""
Phase 4 Week 4 - OpenVLA 1회독 + Architecture 기초 퀴즈
"""


def problem1_backbone():
    print("\n" + "=" * 50)
    print("문제 1: OpenVLA 의 LM backbone")
    print("=" * 50 + "\n")

    print("질문: OpenVLA 의 LM backbone 으로 가장 가까운 것은?\n")
    print("보기:")
    print("  A) PaLI-X 5B / 55B")
    print("  B) Llama 2 7B")
    print("  C) GPT-3.5")
    print("  D) Mistral 8x7B")


def problem2_vision_encoder():
    print("\n" + "=" * 50)
    print("문제 2: Hybrid vision encoder")
    print("=" * 50 + "\n")

    print("질문: OpenVLA 가 결합하는 두 vision encoder 의 역할?\n")
    print("보기:")
    print("  A) DINOv2 (spatial) + SigLIP (semantic)")
    print("  B) CLIP (semantic) + ViT (spatial)")
    print("  C) DINOv2 (semantic) + SigLIP (spatial)")
    print("  D) ResNet (low-level) + ViT (high-level)")


def problem3_dataset():
    print("\n" + "=" * 50)
    print("문제 3: OpenVLA 학습 데이터")
    print("=" * 50 + "\n")

    print("질문: OpenVLA 의 학습에 사용된 robot dataset 은?\n")
    print("보기:")
    print("  A) RT-1 dataset (130K episodes, single embodiment)")
    print("  B) OpenX-Embodiment (970K episodes, 22 embodiments)")
    print("  C) Bridge Dataset only")
    print("  D) DROID dataset only")


def problem4_lora():
    print("\n" + "=" * 50)
    print("문제 4: LoRA 의 의미")
    print("=" * 50 + "\n")

    print("질문: OpenVLA 에서 LoRA fine-tuning 이 중요한 가장 큰 이유는?\n")
    print("보기:")
    print("  A) 7B 모델 전체 fine-tuning 보다 메모리 / 시간 부담이 적어")
    print("     새 robot 에 빠르게 적응 가능")
    print("  B) Inference latency 가 낮아짐")
    print("  C) 모델 정확도가 항상 올라감")
    print("  D) Quantization 과 동일한 효과")


if __name__ == "__main__":
    print("=" * 50)
    print("  Phase 4 Week 4 Quiz - Easy")
    print("  OpenVLA 1회독 + Architecture")
    print("=" * 50)
    problem1_backbone()
    problem2_vision_encoder()
    problem3_dataset()
    problem4_lora()
    print("\n" + "=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
