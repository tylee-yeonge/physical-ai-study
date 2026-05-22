"""
Phase 3 Week 3 - YOLO 이론 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_iou_calculation():
    """
    문제 1: IoU 직접 계산

    두 BBox의 IoU를 수동으로 계산하시오.
    Box A: [50, 50, 200, 200]  (x1, y1, x2, y2)
    Box B: [100, 100, 250, 250]

    TODO: iou_expected를 직접 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: IoU 직접 계산")
    print("━" * 36 + "\n")

    box_a = [50, 50, 200, 200]
    box_b = [100, 100, 250, 250]

    print(f"  Box A: {box_a}")
    print(f"  Box B: {box_b}")
    print()
    print("  힌트:")
    print("    교집합 x 범위: [max(50,100), min(200,250)] = [100, 200]")
    print("    교집합 y 범위: [max(50,100), min(200,250)] = [100, 200]")
    print("    교집합 면적 = ?")
    print("    Box A 면적 = (200-50) * (200-50) = ?")
    print("    Box B 면적 = (250-100) * (250-100) = ?")
    print("    합집합 = A + B - 교집합 = ?")
    print("    IoU = 교집합 / 합집합 = ?")

    # 프로그래밍으로 계산
    inter_x1 = max(box_a[0], box_b[0])
    inter_y1 = max(box_a[1], box_b[1])
    inter_x2 = min(box_a[2], box_b[2])
    inter_y2 = min(box_a[3], box_b[3])
    inter_area = max(0, inter_x2 - inter_x1) * max(0, inter_y2 - inter_y1)

    area_a = (box_a[2] - box_a[0]) * (box_a[3] - box_a[1])
    area_b = (box_b[2] - box_b[0]) * (box_b[3] - box_b[1])
    union_area = area_a + area_b - inter_area
    actual_iou = inter_area / union_area

    # TODO: 직접 계산해보세요
    iou_expected = 0.0  # 여기를 채우시오

    print(f"\n  프로그래밍 결과: IoU = {actual_iou:.4f}")
    print(f"  직접 계산한 값:  IoU = {iou_expected:.4f}")

    if abs(iou_expected - actual_iou) < 0.01:
        print("\n  ✅ 정답!")
    else:
        print("\n  ❌ 다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_nms_trace():
    """
    문제 2: NMS 수행 과정 추적

    아래 5개의 BBox와 confidence가 주어졌을 때,
    NMS (IoU threshold=0.5)를 적용하면 어떤 BBox가 남는지 추적하시오.

    TODO: nms_result를 예측하여 채우시오 (남는 BBox의 인덱스 리스트)
    """
    print("\n" + "━" * 36)
    print("문제 2: NMS 수행 과정 추적")
    print("━" * 36 + "\n")

    boxes = np.array([
        [10, 10, 110, 110],    # BBox 0, conf=0.7
        [15, 12, 115, 112],    # BBox 1, conf=0.9
        [200, 200, 350, 350],  # BBox 2, conf=0.8
        [205, 195, 345, 340],  # BBox 3, conf=0.6
        [500, 100, 600, 200],  # BBox 4, conf=0.75
    ])
    scores = np.array([0.7, 0.9, 0.8, 0.6, 0.75])

    print("  BBox 목록:")
    for i, (box, score) in enumerate(zip(boxes, scores)):
        print(f"    BBox {i}: {box.tolist()}, conf={score:.2f}")

    print("\n  NMS 과정 (IoU threshold=0.5):")
    print("    1단계: confidence 내림차순 정렬")
    print("    2단계: 가장 높은 conf의 BBox 선택")
    print("    3단계: 선택된 BBox와 IoU > 0.5인 BBox 제거")
    print("    4단계: 반복")

    # 실제 NMS 수행
    def nms(boxes, scores, threshold):
        order = scores.argsort()[::-1]
        keep = []
        while order.size > 0:
            i = order[0]
            keep.append(i)
            if order.size == 1:
                break
            x1 = np.maximum(boxes[i, 0], boxes[order[1:], 0])
            y1 = np.maximum(boxes[i, 1], boxes[order[1:], 1])
            x2 = np.minimum(boxes[i, 2], boxes[order[1:], 2])
            y2 = np.minimum(boxes[i, 3], boxes[order[1:], 3])
            inter = np.maximum(0, x2 - x1) * np.maximum(0, y2 - y1)
            area_i = (boxes[i, 2] - boxes[i, 0]) * (boxes[i, 3] - boxes[i, 1])
            area_j = (boxes[order[1:], 2] - boxes[order[1:], 0]) * \
                     (boxes[order[1:], 3] - boxes[order[1:], 1])
            iou = inter / (area_i + area_j - inter + 1e-7)
            remaining = np.where(iou < threshold)[0]
            order = order[remaining + 1]
        return keep

    actual_result = nms(boxes, scores, 0.5)

    # TODO: NMS 결과를 예측하세요
    nms_result = []  # 여기를 채우시오 (예: [1, 2, 4])

    print(f"\n  실제 NMS 결과: {actual_result}")
    print(f"  예측한 결과:   {nms_result}")

    if nms_result == actual_result:
        print("\n  ✅ 정답!")
    else:
        print("\n  ❌ 다시 추적해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem3_precision_recall():
    """
    문제 3: Precision/Recall 계산

    아래 검출 결과에서 IoU threshold=0.5일 때
    각 검출의 TP/FP를 판별하고, 최종 Precision과 Recall을 구하시오.

    GT 객체: 3개
    검출 결과 (conf 순): 5개

    TODO: final_precision과 final_recall을 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: Precision/Recall 계산")
    print("━" * 36 + "\n")

    print("  GT 객체 3개:")
    print("    GT0: [50, 50, 150, 150]")
    print("    GT1: [200, 200, 350, 350]")
    print("    GT2: [400, 100, 500, 250]")
    print()
    print("  검출 결과 (confidence 순):")
    print("    Det0: [55, 48, 148, 148],  conf=0.95, IoU(GT0)=0.85 → TP")
    print("    Det1: [195, 195, 345, 345],conf=0.88, IoU(GT1)=0.82 → TP")
    print("    Det2: [300, 300, 400, 400],conf=0.75, IoU(모든GT)<0.5 → FP")
    print("    Det3: [405, 105, 495, 245],conf=0.60, IoU(GT2)=0.78 → TP")
    print("    Det4: [60, 55, 145, 145],  conf=0.50, IoU(GT0)=0.75 → ?")
    print()
    print("  주의: Det4는 GT0과 높은 IoU를 가지지만,")
    print("        GT0은 이미 Det0에 매칭되었습니다!")
    print()
    print("  최종: TP=?, FP=?")
    print("  Precision = TP / (TP + FP) = ?")
    print("  Recall = TP / (GT 수) = ?")

    # TODO: 직접 계산해보세요
    final_precision = 0.0  # 여기를 채우시오
    final_recall = 0.0     # 여기를 채우시오

    # 정답
    actual_tp = 3
    actual_fp = 2
    actual_precision = actual_tp / (actual_tp + actual_fp)
    actual_recall = actual_tp / 3

    print(f"\n  실제 결과:")
    print(f"    TP={actual_tp}, FP={actual_fp}")
    print(f"    Precision={actual_precision:.4f}")
    print(f"    Recall={actual_recall:.4f}")

    print(f"\n  내 계산:")
    print(f"    Precision={final_precision:.4f}")
    print(f"    Recall={final_recall:.4f}")

    if (abs(final_precision - actual_precision) < 0.01 and
            abs(final_recall - actual_recall) < 0.01):
        print("\n  ✅ 정답!")
    else:
        print("\n  ❌ 다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 3 Quiz - Medium")
    print("━" * 40)
    problem1_iou_calculation()
    problem2_nms_trace()
    problem3_precision_recall()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
