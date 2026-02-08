"""
Phase 5 Week 4 - YOLOv8 학습 기초 퀴즈
"""


def problem1_yolo_format():
    print("\n" + "━" * 28)
    print("문제 1: YOLO 라벨 포맷")
    print("━" * 28 + "\n")

    print("질문: YOLO 라벨 파일에서 '0 0.5 0.4 0.3 0.6'의 의미는?\n")
    print("보기:")
    print("  A) class=0, x_min=0.5, y_min=0.4, x_max=0.3, y_max=0.6")
    print("  B) class=0, x_center=0.5, y_center=0.4, width=0.3, height=0.6")
    print("  C) class=0, confidence=0.5, IoU=0.4, precision=0.3, recall=0.6")
    print("  D) class=0, batch=0.5, epoch=0.4, lr=0.3, loss=0.6")


def problem2_best_vs_last():
    print("\n" + "━" * 28)
    print("문제 2: best.pt vs last.pt")
    print("━" * 28 + "\n")

    print("질문: 학습 후 생성되는 best.pt와 last.pt의 차이는?\n")
    print("보기:")
    print("  A) best.pt는 학습 시작 시, last.pt는 끝날 때의 가중치")
    print("  B) best.pt는 가장 빠른 추론, last.pt는 가장 느린 추론의 가중치")
    print("  C) best.pt는 검증 mAP가 가장 높은 에폭, last.pt는 마지막 에폭의 가중치")
    print("  D) best.pt와 last.pt는 동일한 파일")


def problem3_mosaic():
    print("\n" + "━" * 28)
    print("문제 3: Mosaic Augmentation")
    print("━" * 28 + "\n")

    print("질문: Mosaic Augmentation이 마지막 10 에폭에서")
    print("      자동으로 비활성화되는 이유는?\n")
    print("보기:")
    print("  A) GPU 메모리를 절약하기 위해")
    print("  B) 최종 에폭에서는 원본 이미지로 학습하여 성능을 안정화하기 위해")
    print("  C) 학습 속도를 높이기 위해")
    print("  D) 라벨이 손상되는 것을 방지하기 위해")


def problem4_overfitting():
    print("\n" + "━" * 28)
    print("문제 4: 과적합 판단")
    print("━" * 28 + "\n")

    print("질문: 학습 중 아래와 같은 상황이 발생했습니다.")
    print("      과적합인지 판단하고, 해결 방법을 선택하세요.\n")
    print("  Epoch 30: train_loss=0.5, val_mAP=0.55")
    print("  Epoch 60: train_loss=0.1, val_mAP=0.48")
    print("  Epoch 90: train_loss=0.02, val_mAP=0.42\n")
    print("보기:")
    print("  A) 미적합: 에폭을 더 늘려야 한다")
    print("  B) 과적합: train_loss는 계속 감소하지만 val_mAP는 감소하므로,")
    print("     Augmentation 강화 또는 Early Stopping 적용")
    print("  C) 정상: val_mAP는 자연스럽게 감소한다")
    print("  D) 발산: 학습률이 너무 커서 발생하는 현상")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 4 Quiz - Easy")
    print("━" * 33)
    problem1_yolo_format()
    problem2_best_vs_last()
    problem3_mosaic()
    problem4_overfitting()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
