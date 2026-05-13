"""
Phase 5 Week 4 - YOLO11 학습 중급 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: BBox 포맷 변환")
    print("━" * 36 + "\n")

    W, H = 1920, 1080
    x_min, y_min, x_max, y_max = 384, 216, 960, 756

    print(f"  이미지 크기: {W} x {H}")
    print(f"  Pascal VOC: [{x_min}, {y_min}, {x_max}, {y_max}]")
    print()

    # 계산 과정
    x_center = (x_min + x_max) / 2
    y_center = (y_min + y_max) / 2
    width = x_max - x_min
    height = y_max - y_min

    print("  Step 1: 픽셀 좌표 계산")
    print(f"    x_center = ({x_min} + {x_max}) / 2 = {x_center}")
    print(f"    y_center = ({y_min} + {y_max}) / 2 = {y_center}")
    print(f"    width    = {x_max} - {x_min} = {width}")
    print(f"    height   = {y_max} - {y_min} = {height}")
    print()

    yolo_xc = x_center / W
    yolo_yc = y_center / H
    yolo_w = width / W
    yolo_h = height / H

    print("  Step 2: 정규화 (0~1)")
    print(f"    x_center = {x_center} / {W} = {yolo_xc:.4f}")
    print(f"    y_center = {y_center} / {H} = {yolo_yc:.4f}")
    print(f"    width    = {width} / {W} = {yolo_w:.4f}")
    print(f"    height   = {height} / {H} = {yolo_h:.4f}")
    print()

    print(f"  💡 YOLO 라벨: 0 {yolo_xc:.4f} {yolo_yc:.4f} {yolo_w:.4f} {yolo_h:.4f}")
    print()

    # 역변환 검증
    print("  검증 (YOLO → Pascal VOC 역변환):")
    recovered_x_min = int((yolo_xc - yolo_w / 2) * W)
    recovered_y_min = int((yolo_yc - yolo_h / 2) * H)
    recovered_x_max = int((yolo_xc + yolo_w / 2) * W)
    recovered_y_max = int((yolo_yc + yolo_h / 2) * H)
    print(f"    역변환: [{recovered_x_min}, {recovered_y_min}, "
          f"{recovered_x_max}, {recovered_y_max}]")
    print(f"    원본:   [{x_min}, {y_min}, {x_max}, {y_max}]")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: Confusion Matrix 분석")
    print("━" * 36 + "\n")

    print("  Confusion Matrix:")
    print(f"  {'':16s} 예측: person  car  bicycle  background")
    print(f"  실제: person     70     5     3       22")
    print(f"        car         2    60     1       37")
    print(f"        bicycle     4     0    45       51")
    print(f"        background  8     3     2        -")
    print()

    # 질문 1: person Recall
    person_tp = 70
    person_total = 70 + 5 + 3 + 22  # = 100
    person_recall = person_tp / person_total

    print(f"  질문 1: person Recall")
    print(f"    TP = 70 (person→person)")
    print(f"    FN = 5(→car) + 3(→bicycle) + 22(→background) = 30")
    print(f"    총 GT = 70 + 30 = {person_total}")
    print(f"    Recall = 70 / {person_total} = {person_recall:.2f}")
    print()

    # 질문 2: 가장 미검출이 많은 클래스
    print(f"  질문 2: 가장 미검출(FN)이 많은 클래스")
    print(f"    person  → background: 22개 (미검출)")
    print(f"    car     → background: 37개 (미검출)")
    print(f"    bicycle → background: 51개 (미검출)")
    print(f"    → bicycle이 가장 미검출이 많음 (51개)")
    print(f"    → 원인: 자전거 학습 데이터 부족 또는 크기가 작아 놓침")
    print()

    # 질문 3: 가장 오검출이 많은 클래스
    print(f"  질문 3: False Positive가 가장 많은 클래스")
    print(f"    background 행 (실제로 객체가 아닌데 검출한 경우):")
    print(f"    → person으로 오검출:  8건")
    print(f"    → car로 오검출:       3건")
    print(f"    → bicycle로 오검출:   2건")
    print(f"    → person이 가장 FP가 많음 (8건)")
    print(f"    → 원인: 사람과 유사한 배경(포스터, 마네킹)을 오검출")
    print()

    # 개선 제안
    print(f"  개선 제안:")
    print(f"    1. bicycle: 학습 데이터 추가 (특히 작은 자전거)")
    print(f"    2. person FP: Hard Negative Mining")
    print(f"       (사람과 유사한 배경 이미지를 추가 학습)")
    print(f"    3. car: 부분 가림(occlusion) 데이터 추가")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: Hyperparameter 의사결정")
    print("━" * 36 + "\n")

    print("  실험 결과 분석:")
    print("    Baseline(yolo11n) → Exp-A(yolo11s): 모델 크기 ↑ → 성능 ↑ (효과적)")
    print("    Exp-A → Exp-B: 에폭 ↑ → 약간 성능 ↑, 하지만 과적합 의심")
    print("    Exp-B: train_loss=0.15 (매우 낮음), map50_95=0.39 (정체)")
    print()

    print("  모범 답안 (Exp-C):")
    print()

    print("  방안 1: 과적합 해결 + Augmentation 강화")
    print("    {")
    print('      "model": "yolo11s",')
    print('      "imgsz": 640,')
    print('      "lr0": 0.01,')
    print('      "augmentation": "heavy",')
    print('      "reason": "Exp-B의 과적합을 Augmentation 강화로 해결"')
    print("    }")
    print()

    print("  방안 2: 이미지 크기 증가")
    print("    {")
    print('      "model": "yolo11s",')
    print('      "imgsz": 960,')
    print('      "lr0": 0.01,')
    print('      "augmentation": "heavy",')
    print('      "reason": "더 큰 이미지로 작은 객체 검출 개선"')
    print("    }")
    print()

    print("  방안 3: 모델 크기 추가 증가")
    print("    {")
    print('      "model": "yolo11m",')
    print('      "imgsz": 640,')
    print('      "lr0": 0.01,')
    print('      "augmentation": "heavy",')
    print('      "reason": "더 큰 모델로 표현력 증가 + Augmentation으로 과적합 방지"')
    print("    }")
    print()

    print("  💡 핵심 원칙:")
    print("    1. 한 번에 하나의 변수만 변경 (통제 실험)")
    print("    2. 과적합이 보이면 먼저 정규화(Augmentation) 강화")
    print("    3. 정규화 후에도 성능 정체면 모델 크기/이미지 크기 증가")
    print("    4. 항상 val_mAP를 기준으로 판단 (train_loss 아님)")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 4 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
