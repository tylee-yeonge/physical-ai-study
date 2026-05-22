"""
Phase 3 Week 2 - CV 라이브러리 기초 퀴즈
"""


def problem1_albumentations():
    print("\n" + "━" * 28)
    print("문제 1: Albumentations BBox")
    print("━" * 28 + "\n")

    print("질문: Albumentations에서 HorizontalFlip을 적용할 때,")
    print("      Detection BBox도 자동으로 변환하려면 필요한 것은?\n")
    print("보기:")
    print("  A) 별도 코드로 BBox를 수동 변환")
    print("  B) bbox_params를 Compose에 전달")
    print("  C) BBox를 이미지에 직접 그린 후 변환")
    print("  D) Detection에는 Augmentation 사용 불가")


def problem2_wandb():
    print("\n" + "━" * 28)
    print("문제 2: W&B 실험 관리")
    print("━" * 28 + "\n")

    print("질문: W&B에서 wandb.log()의 역할은?\n")
    print("보기:")
    print("  A) 모델 가중치를 저장")
    print("  B) 학습 메트릭을 서버에 기록하여 시각화")
    print("  C) 데이터셋을 다운로드")
    print("  D) GPU 메모리를 해제")


def problem3_timm():
    print("\n" + "━" * 28)
    print("문제 3: timm 모델")
    print("━" * 28 + "\n")

    print("질문: timm.create_model('resnet50', features_only=True)의 출력은?\n")
    print("보기:")
    print("  A) 최종 Classification logits (1000 클래스)")
    print("  B) 중간 Feature map들의 리스트")
    print("  C) 이미지 재구성 결과")
    print("  D) 학습된 가중치 딕셔너리")


def problem4_augmentation_strategy():
    print("\n" + "━" * 28)
    print("문제 4: Augmentation 전략")
    print("━" * 28 + "\n")

    print("질문: Detection 모델 학습에서 Augmentation이 중요한 이유는?\n")
    print("보기:")
    print("  A) 학습 속도를 빠르게 함")
    print("  B) GPU 메모리 사용을 줄임")
    print("  C) 데이터 다양성을 높여 과적합 방지")
    print("  D) 모델 크기를 줄임")


if __name__ == "__main__":
    print("━" * 33)
    print("Week 2 Quiz - Easy (CV 라이브러리)")
    print("━" * 33)

    problem1_albumentations()
    problem2_wandb()
    problem3_timm()
    problem4_augmentation_strategy()

    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
