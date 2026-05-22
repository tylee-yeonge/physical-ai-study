"""
Phase 3 Week 3 - YOLO 이론 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: B) Anchor 설계/튜닝 부담을 없애고 더 유연한 BBox 예측을 위해")
    print("━" * 28 + "\n")

    print("해설:")
    print("  Anchor 기반 (YOLOv1~v7):")
    print("    → 미리 정의한 Anchor Box의 오프셋을 예측")
    print("    → Anchor 크기/비율을 K-means로 결정해야 함")
    print("    → 데이터셋마다 최적 Anchor가 달라짐")
    print()
    print("  Anchor-Free (YOLOv8 ~ YOLO11):")
    print("    → 각 셀에서 직접 BBox 좌표를 예측")
    print("    → left, top, right, bottom 거리를 직접 예측")
    print("    → Anchor 설계 불필요, 더 유연한 예측 가능")
    print()
    print("  TIP: Anchor-Free는 FCOS, CenterNet 등에서 먼저 제안되었고,")
    print("     YOLOv8이 YOLO 계열에 도입, YOLO11도 동일 방식을 계승합니다.")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: B) Backbone: 특징 추출, Neck: 특징 융합, Head: BBox+Class 예측")
    print("━" * 28 + "\n")

    print("해설:")
    print("  Backbone (CSPDarknet + C3k2 + SPPF + C2PSA):")
    print("    → 입력 이미지에서 Multi-Scale Feature Map 추출")
    print("    → P3(80x80), P4(40x40), P5(20x20) 출력")
    print("    → YOLO11: C2f를 C3k2로 교체, 마지막에 C2PSA(Attention) 추가")
    print()
    print("  Neck (PANet = FPN + PAN):")
    print("    → Top-Down(FPN): 고수준 의미 정보를 저해상도→고해상도로 전파")
    print("    → Bottom-Up(PAN): 위치 정보를 고해상도→저해상도로 전파")
    print("    → Multi-Scale Feature 융합")
    print()
    print("  Head (Decoupled Head):")
    print("    → BBox Branch: BBox 좌표 예측 (DFL)")
    print("    → Cls Branch: 클래스 확률 예측 (BCE)")
    print("    → Objectness는 제거됨 (v8부터)")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: B) BBox가 겹치지 않아도 중심 거리와 종횡비를 고려하여 gradient를 제공한다")
    print("━" * 28 + "\n")

    print("해설:")
    print("  IoU Loss:")
    print("    → 교집합/합집합으로 계산")
    print("    → 문제: BBox가 겹치지 않으면 IoU=0, gradient=0")
    print("    → 학습 초기에 BBox가 먼 경우 학습 불가")
    print()
    print("  CIoU Loss = IoU - d²/c² - αv")
    print("    → d²/c²: 중심점 거리 (BBox가 떨어져 있어도 gradient 존재)")
    print("    → αv: 종횡비 일관성 (가로세로 비율도 맞추도록)")
    print()
    print("  진화 순서: IoU → GIoU → DIoU → CIoU")
    print("    각 단계마다 더 풍부한 정보를 gradient에 포함")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: B) IoU 임계값 0.5~0.95 범위에서 평균하여 정확한 위치 예측을 요구하기 때문")
    print("━" * 28 + "\n")

    print("해설:")
    print("  mAP@0.5:")
    print("    → IoU ≥ 0.5이면 TP로 인정")
    print("    → 위치가 대략 맞으면 OK")
    print("    → Pascal VOC에서 사용")
    print()
    print("  mAP@0.5:0.95:")
    print("    → IoU 0.5, 0.55, 0.60, ..., 0.95 총 10개 임계값")
    print("    → 각 임계값에서 AP를 계산하고 평균")
    print("    → IoU 0.9 이상은 매우 정확한 위치를 요구")
    print("    → COCO 공식 지표 (더 엄격)")
    print()
    print("  💡 mAP@0.5:0.95가 높으려면:")
    print("     BBox 위치가 매우 정확해야 합니다.")
    print("     따라서 모델의 실질적인 검출 능력을 더 잘 반영합니다.")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 3 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "━" * 33)
