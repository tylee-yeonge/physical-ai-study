"""
Phase 5 Week 2 - CV 라이브러리 기초 퀴즈 풀이
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 풀이: Albumentations BBox")
    print("━" * 28 + "\n")

    print("정답: B) bbox_params를 Compose에 전달\n")
    print("  A.Compose([...], bbox_params=A.BboxParams(")
    print("      format='yolo',")
    print("      label_fields=['class_labels']")
    print("  ))\n")
    print("  → Augmentation 적용 시 BBox도 자동 변환")
    print("  → format: 'pascal_voc', 'yolo', 'coco', 'albumentations'")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 풀이: W&B 실험 관리")
    print("━" * 28 + "\n")

    print("정답: B) 학습 메트릭을 서버에 기록하여 시각화\n")
    print("  wandb.log({'loss': 0.5, 'mAP': 0.7})")
    print("  → W&B 서버에 실시간으로 전송")
    print("  → 웹 대시보드에서 그래프 확인 가능")
    print("  → 여러 실험 비교 가능")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 풀이: timm 모델")
    print("━" * 28 + "\n")

    print("정답: B) 중간 Feature map들의 리스트\n")
    print("  features_only=True:")
    print("    → Classification head 제거")
    print("    → 각 stage의 Feature map 반환")
    print("    → Detection backbone으로 활용 가능\n")
    print("  예: ResNet-50 features_only=True, out_indices=[1,2,3,4]")
    print("    → [stride4, stride8, stride16, stride32] 4개 Feature map")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 풀이: Augmentation 전략")
    print("━" * 28 + "\n")

    print("정답: C) 데이터 다양성을 높여 과적합 방지\n")
    print("  Augmentation의 효과:")
    print("    → 동일 이미지를 다양하게 변형")
    print("    → 모델이 더 robust한 특징 학습")
    print("    → 실제 환경의 다양성 모사 (밝기, 각도 등)")
    print("    → 적은 데이터로도 좋은 성능")


if __name__ == "__main__":
    print("━" * 33)
    print("Week 2 Quiz Easy - 풀이")
    print("━" * 33)

    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()

    print("\n" + "━" * 33)
