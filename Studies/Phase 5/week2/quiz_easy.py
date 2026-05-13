"""
Phase 5 Week 2 - ViT inference 기초 퀴즈
"""


def problem1():
    print("\n" + "=" * 50)
    print("문제 1: ViT-L latency (RTX 4070 fp16)")
    print("=" * 50 + "\n")
    print("질문: ViT-L (300M) 의 inference latency 예상치?\n")
    print("  A) ~ 1 ms\n  B) ~ 30 ms\n  C) ~ 200 ms\n  D) ~ 1 s")


def problem2():
    print("\n" + "=" * 50)
    print("문제 2: ViT 의 robustness 한계")
    print("=" * 50 + "\n")
    print("질문: ViT 가 약한 영역은?\n")
    print("  A) Natural image 분류")
    print("  B) Augmentation 없이 회전 / scale 변화")
    print("  C) 큰 배치 학습")
    print("  D) GPU 가속")


def problem3():
    print("\n" + "=" * 50)
    print("문제 3: OpenVLA vision encoder 차지 비중")
    print("=" * 50 + "\n")
    print("질문: OpenVLA 의 165 ms 전체 latency 중 vision encoder 가 차지하는 비중?\n")
    print("  A) ~ 5 ms (3%)\n  B) ~ 60 ms (35%)\n  C) ~ 100 ms (60%)\n  D) ~ 150 ms (90%)")


def problem4():
    print("\n" + "=" * 50)
    print("문제 4: patch_size trade-off")
    print("=" * 50 + "\n")
    print("질문: patch_size 32 (vs 16) 의 효과?\n")
    print("  A) Sequence 더 길어 더 정확")
    print("  B) Sequence 짧음 -> 빠름 + 정확도 약간 손실")
    print("  C) 학습 시간 증가")
    print("  D) 변화 없음")


if __name__ == "__main__":
    problem1(); problem2(); problem3(); problem4()
