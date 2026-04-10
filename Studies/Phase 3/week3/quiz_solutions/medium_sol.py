"""
Phase 5 Week 3 - YOLO 이론 중급 퀴즈 정답 및 해설
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: IoU 계산")
    print("━" * 36 + "\n")

    box_a = [50, 50, 200, 200]
    box_b = [100, 100, 250, 250]

    print(f"  Box A: {box_a}")
    print(f"  Box B: {box_b}")
    print()

    # 교집합
    inter_x1 = max(50, 100)   # = 100
    inter_y1 = max(50, 100)   # = 100
    inter_x2 = min(200, 250)  # = 200
    inter_y2 = min(200, 250)  # = 200
    inter_w = inter_x2 - inter_x1  # = 100
    inter_h = inter_y2 - inter_y1  # = 100
    inter_area = inter_w * inter_h  # = 10000

    print(f"  교집합:")
    print(f"    x 범위: [{inter_x1}, {inter_x2}], 폭 = {inter_w}")
    print(f"    y 범위: [{inter_y1}, {inter_y2}], 높이 = {inter_h}")
    print(f"    면적 = {inter_w} * {inter_h} = {inter_area}")
    print()

    # 각 BBox 면적
    area_a = (200 - 50) * (200 - 50)   # = 22500
    area_b = (250 - 100) * (250 - 100)  # = 22500
    print(f"  Box A 면적: {area_a}")
    print(f"  Box B 면적: {area_b}")
    print()

    # 합집합
    union_area = area_a + area_b - inter_area  # = 35000
    print(f"  합집합: {area_a} + {area_b} - {inter_area} = {union_area}")
    print()

    # IoU
    iou = inter_area / union_area  # = 10000 / 35000 ≈ 0.2857
    print(f"  IoU = {inter_area} / {union_area} = {iou:.4f}")
    print()
    print(f"  💡 정답: iou_expected = {iou:.4f}")
    print("     (약 0.2857 또는 2/7)")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: NMS 수행 과정")
    print("━" * 36 + "\n")

    print("  BBox 목록:")
    print("    BBox 0: [10,10,110,110],   conf=0.7")
    print("    BBox 1: [15,12,115,112],   conf=0.9")
    print("    BBox 2: [200,200,350,350], conf=0.8")
    print("    BBox 3: [205,195,345,340], conf=0.6")
    print("    BBox 4: [500,100,600,200], conf=0.75")
    print()

    print("  Step 1: confidence 내림차순 정렬")
    print("    순서: BBox 1(0.9) → BBox 2(0.8) → BBox 4(0.75) → BBox 0(0.7) → BBox 3(0.6)")
    print()

    print("  Step 2: BBox 1 선택 (conf=0.9)")
    print("    → BBox 0과 IoU 계산:")
    print("      교집합: [15,12]~[110,110] = 95*98 = 9310")
    print("      BBox1 면적 = 100*100 = 10000")
    print("      BBox0 면적 = 100*100 = 10000")
    print("      IoU = 9310 / (10000+10000-9310) = 9310/10690 ≈ 0.87")
    print("    → IoU > 0.5 → BBox 0 제거")
    print("    → BBox 2, 4, 3은 BBox 1과 거리가 멀어 IoU ≈ 0")
    print()

    print("  Step 3: BBox 2 선택 (conf=0.8)")
    print("    → BBox 3과 IoU 계산:")
    print("      교집합: [205,200]~[345,340] = 140*140 = 19600")
    print("      BBox2 면적 = 150*150 = 22500")
    print("      BBox3 면적 = 140*145 = 20300")
    print("      IoU = 19600 / (22500+20300-19600) = 19600/23200 ≈ 0.84")
    print("    → IoU > 0.5 → BBox 3 제거")
    print("    → BBox 4와는 IoU ≈ 0")
    print()

    print("  Step 4: BBox 4 선택 (conf=0.75)")
    print("    → 남은 BBox 없음, 종료")
    print()

    print("  최종 결과: [1, 2, 4]")
    print()
    print("  💡 정답: nms_result = [1, 2, 4]")
    print("     BBox 0은 BBox 1에 의해 제거 (같은 객체)")
    print("     BBox 3은 BBox 2에 의해 제거 (같은 객체)")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: Precision/Recall 계산")
    print("━" * 36 + "\n")

    print("  검출 결과 분석:")
    print("    Det0: IoU(GT0)=0.85 ≥ 0.5, GT0 미매칭 → TP")
    print("    Det1: IoU(GT1)=0.82 ≥ 0.5, GT1 미매칭 → TP")
    print("    Det2: 모든 GT와 IoU < 0.5                → FP")
    print("    Det3: IoU(GT2)=0.78 ≥ 0.5, GT2 미매칭 → TP")
    print("    Det4: IoU(GT0)=0.75 ≥ 0.5, 하지만 GT0은 이미 Det0에 매칭 → FP")
    print()

    print("  결과:")
    print("    TP = 3 (Det0, Det1, Det3)")
    print("    FP = 2 (Det2, Det4)")
    print("    FN = 0 (모든 GT가 매칭됨)")
    print()

    tp = 3
    fp = 2
    gt_count = 3

    precision = tp / (tp + fp)
    recall = tp / gt_count

    print(f"  Precision = TP / (TP + FP) = {tp} / ({tp}+{fp}) = {precision:.4f}")
    print(f"  Recall    = TP / GT수      = {tp} / {gt_count}     = {recall:.4f}")
    print()
    print(f"  💡 정답:")
    print(f"     final_precision = {precision:.4f}  (= 3/5 = 0.6)")
    print(f"     final_recall    = {recall:.4f}  (= 3/3 = 1.0)")
    print()
    print("  핵심 포인트:")
    print("    → Det4는 GT0과 높은 IoU를 가지지만, GT0이 이미 매칭되었으므로 FP")
    print("    → 하나의 GT는 하나의 검출에만 매칭 가능 (중복 매칭 불가)")
    print("    → Recall=1.0: 모든 GT를 찾았음")
    print("    → Precision=0.6: 5개 검출 중 3개만 맞음 (2개 오검출)")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 3 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
