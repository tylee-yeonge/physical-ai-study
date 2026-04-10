"""
Phase 5 Week 2 - CV 라이브러리 중급 퀴즈
"""
import numpy as np


def problem1_bbox_transform():
    print("\n" + "━" * 28)
    print("문제 1: BBox 변환 계산")
    print("━" * 28 + "\n")

    print("이미지 크기: 640x480 (WxH)")
    print("원본 BBox (Pascal VOC): [100, 200, 300, 400]")
    print("  = [x_min, y_min, x_max, y_max]\n")

    print("과제:")
    print("  1. Pascal VOC → YOLO 포맷으로 변환하세요.")
    print("     YOLO: [x_center/W, y_center/H, width/W, height/H]\n")
    print("  2. HorizontalFlip 적용 후 YOLO BBox는?\n")
    print("  3. Resize(320, 320) 후 Pascal VOC BBox는?\n")

    # TODO: 학생이 구현
    print("  YOLO: [_____, _____, _____, _____]")
    print("  Flip 후 YOLO: [_____, _____, _____, _____]")
    print("  Resize 후 VOC: [_____, _____, _____, _____]")


def problem2_feature_extraction():
    print("\n" + "━" * 28)
    print("문제 2: Feature Map 크기 계산")
    print("━" * 28 + "\n")

    print("ResNet-50의 Feature Pyramid에서:")
    print("  입력: [1, 3, 640, 640]")
    print("  Layer1 (stride 4):  [1, 256, ?, ?]")
    print("  Layer2 (stride 8):  [1, 512, ?, ?]")
    print("  Layer3 (stride 16): [1, 1024, ?, ?]")
    print("  Layer4 (stride 32): [1, 2048, ?, ?]\n")

    print("과제: 각 Layer의 공간 해상도(H, W)를 구하세요.\n")

    # TODO: 학생이 구현
    print("  Layer1: [1, 256, ___, ___]")
    print("  Layer2: [1, 512, ___, ___]")
    print("  Layer3: [1, 1024, ___, ___]")
    print("  Layer4: [1, 2048, ___, ___]")


def problem3_wandb_config():
    print("\n" + "━" * 28)
    print("문제 3: W&B 실험 비교")
    print("━" * 28 + "\n")

    print("3개의 실험 결과가 있습니다:\n")

    experiments = [
        {"name": "exp-A", "lr": 0.01, "batch": 16,
         "aug": "basic", "val_mAP": 0.45, "train_loss": 0.8},
        {"name": "exp-B", "lr": 0.001, "batch": 32,
         "aug": "heavy", "val_mAP": 0.52, "train_loss": 0.3},
        {"name": "exp-C", "lr": 0.001, "batch": 16,
         "aug": "heavy", "val_mAP": 0.38, "train_loss": 0.15},
    ]

    print(f"  {'실험':8s} │ {'lr':8s} │ {'batch':6s} │ {'aug':6s} │ {'val_mAP':8s} │ {'train_loss':10s}")
    print("  " + "─" * 60)
    for exp in experiments:
        print(f"  {exp['name']:8s} │ {exp['lr']:8.3f} │ {exp['batch']:6d} │ {exp['aug']:6s} │ {exp['val_mAP']:8.2f} │ {exp['train_loss']:10.2f}")

    print("\n과제:")
    print("  1. 가장 좋은 실험은? 그 이유는?")
    print("  2. exp-C의 문제점은? (train_loss vs val_mAP)")
    print("  3. 다음 실험에서 바꿀 하이퍼파라미터는?")


if __name__ == "__main__":
    print("━" * 33)
    print("Week 2 Quiz - Medium (CV 라이브러리)")
    print("━" * 33)

    problem1_bbox_transform()
    problem2_feature_extraction()
    problem3_wandb_config()

    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 33)
