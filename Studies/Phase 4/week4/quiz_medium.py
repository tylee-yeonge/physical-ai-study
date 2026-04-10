"""
Phase 6 Week 4 - Monocular 3D Detection 모델 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_sincos_decode():
    """
    문제 1: sin/cos 인코딩과 디코딩

    3개의 객체에 대해 네트워크가 예측한 (sin, cos) 값이 있습니다.
    각 객체의 yaw 각도(theta)를 복원하시오.

    또한, "직접 회귀"와 "sin/cos 인코딩"의 loss 차이를 계산하시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: sin/cos 디코딩 및 비교")
    print("━" * 36 + "\n")

    predictions = [
        {"name": "Car-A", "sin": 0.0998, "cos": 0.9950},
        {"name": "Car-B", "sin": -0.7071, "cos": 0.7071},
        {"name": "Car-C", "sin": 0.0, "cos": -1.0},
    ]

    print("  네트워크 예측값:")
    for p in predictions:
        print(f"    {p['name']}: sin={p['sin']:.4f}, cos={p['cos']:.4f}")

    print("\n  과제 1: 각 객체의 yaw (theta)를 라디안과 도(degree)로 구하시오.")
    print("          공식: theta = arctan2(sin, cos)\n")

    # TODO: 학생이 계산
    for p in predictions:
        print(f"    {p['name']}: theta = ___ rad = ___ 도")

    print("\n  과제 2: GT theta = 3.13 rad일 때,")
    print("          pred theta = -3.13 rad로 예측했다면:")
    print("    직접 회귀 L1 loss = |3.13 - (-3.13)| = ___")
    print("    sin/cos L1 loss = |sin(3.13)-sin(-3.13)| + |cos(3.13)-cos(-3.13)| = ___")
    print("    (실제 각도 차이는 불과 0.02 rad입니다)")


def problem2_depth_log_comparison():
    """
    문제 2: Depth 추정 - Direct vs Log-space 비교

    두 가지 방법으로 depth loss를 계산하고 비교하시오.
    """
    print("\n" + "━" * 36)
    print("문제 2: Depth 추정 방법 비교")
    print("━" * 36 + "\n")

    gt_depths = np.array([5.0, 10.0, 30.0, 50.0])
    pred_depths = np.array([7.0, 12.0, 32.0, 52.0])  # 각각 2m 오차

    print("  GT depths:   ", gt_depths)
    print("  Pred depths: ", pred_depths)
    print("  절대 오차:      모두 2.0m\n")

    print("  과제:")
    print("  1) Direct L1 loss를 각 객체에 대해 계산하시오.")
    print("     L1 = |pred - gt|")
    print()
    print("  2) Log-space L1 loss를 각 객체에 대해 계산하시오.")
    print("     L1_log = |log(pred) - log(gt)|")
    print()
    print("  3) 두 방법에서 가까운 객체(5m)와 먼 객체(50m)의")
    print("     loss 비율을 비교하시오.")
    print()
    print("  4) 자율주행 관점에서 어떤 방법이 더 적합한가? 이유는?")

    # 힌트
    print("\n  힌트:")
    for gt, pred in zip(gt_depths, pred_depths):
        direct_loss = abs(pred - gt)
        log_loss = abs(np.log(pred) - np.log(gt))
        print(f"    z={gt:4.0f}m: Direct L1={direct_loss:.2f}, "
              f"Log L1={log_loss:.4f}")


def problem3_3d_position_from_heatmap():
    """
    문제 3: Heatmap에서 3D 위치 추정 (SMOKE 스타일)

    Feature map에서 검출된 중심점과 회귀된 속성으로
    3D bbox의 카메라 좌표를 계산하시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: Heatmap → 3D 위치 계산")
    print("━" * 36 + "\n")

    # 카메라 파라미터 (KITTI 전형적)
    fx, fy = 721.54, 721.54
    cx, cy = 609.56, 172.85
    stride = 4  # feature map 다운샘플 비율

    # Feature map에서 검출된 중심점 (SMOKE 출력)
    detections = [
        {"fm_x": 100, "fm_y": 40, "offset_x": 0.3, "offset_y": -0.2,
         "depth": 15.5, "h": 1.52, "w": 1.63, "l": 3.88,
         "sin_ry": 0.0998, "cos_ry": 0.9950},
        {"fm_x": 200, "fm_y": 38, "offset_x": -0.1, "offset_y": 0.15,
         "depth": 35.2, "h": 1.50, "w": 1.60, "l": 4.10,
         "sin_ry": -0.9511, "cos_ry": 0.3090},
    ]

    print(f"  카메라 파라미터: fx={fx}, fy={fy}, cx={cx}, cy={cy}")
    print(f"  Feature map stride: {stride}")
    print()

    for i, det in enumerate(detections):
        print(f"  객체 {i+1}:")
        print(f"    Feature map 위치: ({det['fm_x']}, {det['fm_y']})")
        print(f"    Offset: ({det['offset_x']:.1f}, {det['offset_y']:.1f})")
        print(f"    Depth: {det['depth']:.1f}m")
        print(f"    Size: h={det['h']}, w={det['w']}, l={det['l']}")
        print(f"    Rotation: sin={det['sin_ry']:.4f}, cos={det['cos_ry']:.4f}")
        print()

    print("  과제:")
    print("  1) Feature map 좌표를 이미지 좌표로 변환하시오.")
    print("     u = (fm_x + offset_x) * stride")
    print("     v = (fm_y + offset_y) * stride")
    print()
    print("  2) 이미지 좌표와 depth로 3D 좌표를 구하시오.")
    print("     X = (u - cx) * Z / fx")
    print("     Y = (v - cy) * Z / fy")
    print("     Z = depth")
    print()
    print("  3) sin/cos에서 yaw 각도를 복원하시오.")
    print("     ry = arctan2(sin_ry, cos_ry)")


if __name__ == "__main__":
    print("━" * 40)
    print("  Phase 6 Week 4 Quiz - Medium")
    print("━" * 40)

    problem1_sincos_decode()
    problem2_depth_log_comparison()
    problem3_3d_position_from_heatmap()

    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
