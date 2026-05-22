"""
Phase 3 Week 4 - YOLO11 학습 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: B) class=0, x_center=0.5, y_center=0.4, width=0.3, height=0.6")
    print("━" * 28 + "\n")

    print("해설:")
    print("  YOLO 라벨 포맷: class_id x_center y_center width height")
    print()
    print("  각 값의 의미:")
    print("    0   → 클래스 ID (정수)")
    print("    0.5 → BBox 중심의 x 좌표 (이미지 폭으로 정규화, 0~1)")
    print("    0.4 → BBox 중심의 y 좌표 (이미지 높이로 정규화, 0~1)")
    print("    0.3 → BBox의 폭 (정규화)")
    print("    0.6 → BBox의 높이 (정규화)")
    print()
    print("  💡 모든 좌표가 0~1로 정규화된 이유:")
    print("     이미지 크기에 상관없이 동일한 라벨 사용 가능")
    print("     640x480에서 작성한 라벨을 1280x960에서도 사용 가능")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: C) best.pt는 검증 mAP가 가장 높은 에폭, last.pt는 마지막 에폭의 가중치")
    print("━" * 28 + "\n")

    print("해설:")
    print("  best.pt:")
    print("    → 학습 중 검증 mAP가 가장 높았던 에폭의 가중치")
    print("    → Early Stopping의 기준이 됨")
    print("    → 배포(deploy)에 사용하는 모델")
    print()
    print("  last.pt:")
    print("    → 마지막 에폭의 가중치")
    print("    → 학습을 이어서(resume) 할 때 사용")
    print("    → 과적합된 상태일 수 있음")
    print()
    print("  💡 실전 팁:")
    print("     - 추론/배포: 항상 best.pt 사용")
    print("     - 학습 재개: last.pt + resume=True")
    print("     - 모델 공유: best.pt를 전달")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: B) 최종 에폭에서는 원본 이미지로 학습하여 성능을 안정화하기 위해")
    print("━" * 28 + "\n")

    print("해설:")
    print("  Mosaic Augmentation:")
    print("    → 4장의 이미지를 하나로 합침")
    print("    → 작은 객체 학습 + 다양한 문맥 제공")
    print()
    print("  마지막 10 에폭에서 비활성화하는 이유:")
    print("    1. Mosaic 이미지는 실제 입력과 다른 분포")
    print("    2. 마지막 단계에서 실제 이미지에 적응시킴")
    print("    3. 최종 BBox 정확도를 높임")
    print("    4. 이를 close_mosaic=10으로 설정 (기본값)")
    print()
    print("  💡 비유: 시험 전 마지막 정리")
    print("     학습 중 = 다양한 변형으로 공부 (일반화)")
    print("     마지막 = 실제 시험지 형식에 맞춰 정리 (안정화)")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: B) 과적합")
    print("━" * 28 + "\n")

    print("해설:")
    print("  학습 과정:")
    print("    Epoch 30: train_loss=0.5,  val_mAP=0.55  ← 정상")
    print("    Epoch 60: train_loss=0.1,  val_mAP=0.48  ← val_mAP 감소!")
    print("    Epoch 90: train_loss=0.02, val_mAP=0.42  ← 더 감소!")
    print()
    print("  과적합 증거:")
    print("    → train_loss는 계속 감소 (학습 데이터를 잘 맞춤)")
    print("    → val_mAP는 감소 (새로운 데이터에 일반화 실패)")
    print("    → train_loss와 val_mAP의 격차가 커짐")
    print()
    print("  해결 방법:")
    print("    1. Early Stopping (patience=20): Epoch ~30에서 중단")
    print("    2. Augmentation 강화: mosaic, mixup, 회전, 스케일")
    print("    3. 데이터 추가: 학습 데이터 다양성 확보")
    print("    4. 모델 축소: yolo11s → yolo11n")
    print("    5. Weight Decay 증가: 0.0005 → 0.001")
    print()
    print("  💡 과적합 vs 미적합:")
    print("     과적합: train 잘함 + val 못함 → 정규화 필요")
    print("     미적합: train 못함 + val 못함 → 모델 확대 필요")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 4 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "━" * 33)
