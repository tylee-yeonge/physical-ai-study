"""
Phase 5 Week 1 - ViT 기초 퀴즈
"""


def problem1_patch_count():
    print("\n" + "=" * 50)
    print("문제 1: ViT 의 patch 개수")
    print("=" * 50 + "\n")
    print("질문: 224x224 image, 16x16 patch_size 일 때 patch 개수는?\n")
    print("보기:")
    print("  A) 14 (= 224/16)")
    print("  B) 196 (= 14*14)")
    print("  C) 197 (CLS 포함)")
    print("  D) 768 (patch dim)")


def problem2_image_to_token():
    print("\n" + "=" * 50)
    print("문제 2: Image -> token 변환의 정확한 과정")
    print("=" * 50 + "\n")
    print("질문: ViT 가 224x224 image 를 token sequence 로 변환할 때?\n")
    print("보기:")
    print("  A) Conv2d 로 feature map 추출")
    print("  B) 16x16 patch 분할 -> Linear projection -> + position embed + CLS")
    print("  C) 픽셀 그대로 sequence (224*224 token)")
    print("  D) 16x16 patch 분할 후 그대로 token")


def problem3_self_attention_complexity():
    print("\n" + "=" * 50)
    print("문제 3: Self-attention 의 시간 복잡도")
    print("=" * 50 + "\n")
    print("질문: ViT 의 self-attention 의 시간 복잡도는?\n")
    print("보기:")
    print("  A) O(N) (token 수에 비례)")
    print("  B) O(N log N)")
    print("  C) O(N^2) (token 쌍의 수)")
    print("  D) O(N^3)")


def problem4_vit_l_params():
    print("\n" + "=" * 50)
    print("문제 4: ViT-L 의 parameter 수")
    print("=" * 50 + "\n")
    print("질문: OpenVLA 의 DINOv2 / SigLIP backbone 인 ViT-L 의 parameter 수?\n")
    print("보기:")
    print("  A) 약 22M")
    print("  B) 약 86M")
    print("  C) 약 300M (300M)")
    print("  D) 약 700M")


if __name__ == "__main__":
    print("=" * 50)
    problem1_patch_count()
    problem2_image_to_token()
    problem3_self_attention_complexity()
    problem4_vit_l_params()
    print("=" * 50)
