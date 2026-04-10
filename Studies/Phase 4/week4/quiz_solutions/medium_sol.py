"""
Phase 6 Week 4 - Monocular 3D Detection 모델 중급 퀴즈 풀이
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 풀이: sin/cos 디코딩")
    print("━" * 36 + "\n")

    predictions = [
        {"name": "Car-A", "sin": 0.0998, "cos": 0.9950},
        {"name": "Car-B", "sin": -0.7071, "cos": 0.7071},
        {"name": "Car-C", "sin": 0.0, "cos": -1.0},
    ]

    print("  과제 1: yaw 각도 복원")
    for p in predictions:
        theta = np.arctan2(p['sin'], p['cos'])
        print(f"    {p['name']}: theta = arctan2({p['sin']}, {p['cos']})")
        print(f"           = {theta:.4f} rad = {np.degrees(theta):.1f} 도")

    print(f"\n    Car-A: ~0.1 rad (~5.7도) → 거의 전방 향함")
    print(f"    Car-B: ~-0.785 rad (~-45도) → 우측 45도")
    print(f"    Car-C: ~pi rad (~180도) → 반대 방향")

    print("\n  과제 2: 직접 회귀 vs sin/cos 비교")
    gt_theta = 3.13
    pred_theta = -3.13

    direct_loss = abs(gt_theta - pred_theta)
    sincos_loss = (abs(np.sin(gt_theta) - np.sin(pred_theta)) +
                   abs(np.cos(gt_theta) - np.cos(pred_theta)))
    actual_diff = min(abs(gt_theta - pred_theta),
                      2 * np.pi - abs(gt_theta - pred_theta))

    print(f"    GT: {gt_theta} rad, Pred: {pred_theta} rad")
    print(f"    실제 각도 차이: {actual_diff:.4f} rad ({np.degrees(actual_diff):.1f}도)")
    print(f"    직접 회귀 L1: |{gt_theta} - ({pred_theta})| = {direct_loss:.2f}")
    print(f"    sin/cos L1: |sin(3.13)-sin(-3.13)| + |cos(3.13)-cos(-3.13)|")
    print(f"              = |{np.sin(gt_theta):.4f}-({np.sin(pred_theta):.4f})| + "
          f"|{np.cos(gt_theta):.4f}-({np.cos(pred_theta):.4f})|")
    print(f"              = {sincos_loss:.4f}")
    print(f"\n    → 직접 회귀: {direct_loss:.2f} (실제 차이에 비해 지나치게 큼!)")
    print(f"    → sin/cos: {sincos_loss:.4f} (실제 차이에 비례하여 적절)")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 풀이: Depth 추정 방법 비교")
    print("━" * 36 + "\n")

    gt_depths = np.array([5.0, 10.0, 30.0, 50.0])
    pred_depths = np.array([7.0, 12.0, 32.0, 52.0])

    print("  1) Direct L1 loss:")
    for gt, pred in zip(gt_depths, pred_depths):
        loss = abs(pred - gt)
        print(f"    z={gt:4.0f}m: L1 = |{pred} - {gt}| = {loss:.2f}")

    print("\n  2) Log-space L1 loss:")
    for gt, pred in zip(gt_depths, pred_depths):
        loss = abs(np.log(pred) - np.log(gt))
        print(f"    z={gt:4.0f}m: L1_log = |log({pred})-log({gt})| "
              f"= |{np.log(pred):.4f}-{np.log(gt):.4f}| = {loss:.4f}")

    print("\n  3) 가까운 vs 먼 객체의 loss 비율:")
    direct_near = abs(7.0 - 5.0)
    direct_far = abs(52.0 - 50.0)
    log_near = abs(np.log(7.0) - np.log(5.0))
    log_far = abs(np.log(52.0) - np.log(50.0))

    print(f"    Direct: near/far = {direct_near}/{direct_far} = {direct_near/direct_far:.1f}")
    print(f"    Log:    near/far = {log_near:.4f}/{log_far:.4f} = {log_near/log_far:.1f}")
    print(f"\n    → Direct: 모든 거리에서 같은 loss (1.0배)")
    print(f"    → Log: 가까운 객체의 loss가 {log_near/log_far:.1f}배 더 큼")

    print("\n  4) 자율주행에는 Log-space가 더 적합")
    print("    → 가까운 객체 = 충돌 위험 높음 → 정확도 중요")
    print("    → 먼 객체 = 여유 있음 → 상대적 정확도 낮아도 OK")
    print("    → Log-space는 가까운 객체에 더 큰 loss를 부여하여")
    print("      가까운 물체의 depth 정확도를 자연스럽게 높임")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 풀이: Heatmap → 3D 위치")
    print("━" * 36 + "\n")

    fx, fy = 721.54, 721.54
    cx, cy = 609.56, 172.85
    stride = 4

    detections = [
        {"fm_x": 100, "fm_y": 40, "offset_x": 0.3, "offset_y": -0.2,
         "depth": 15.5, "sin_ry": 0.0998, "cos_ry": 0.9950},
        {"fm_x": 200, "fm_y": 38, "offset_x": -0.1, "offset_y": 0.15,
         "depth": 35.2, "sin_ry": -0.9511, "cos_ry": 0.3090},
    ]

    for i, det in enumerate(detections):
        print(f"  객체 {i+1}:")

        # 1) Feature map → 이미지 좌표
        u = (det['fm_x'] + det['offset_x']) * stride
        v = (det['fm_y'] + det['offset_y']) * stride
        print(f"    1) 이미지 좌표:")
        print(f"       u = ({det['fm_x']} + {det['offset_x']}) * {stride} = {u:.1f}")
        print(f"       v = ({det['fm_y']} + {det['offset_y']}) * {stride} = {v:.1f}")

        # 2) 3D 좌표
        Z = det['depth']
        X = (u - cx) * Z / fx
        Y = (v - cy) * Z / fy
        print(f"    2) 3D 좌표:")
        print(f"       X = ({u:.1f} - {cx}) * {Z} / {fx} = {X:.2f}m")
        print(f"       Y = ({v:.1f} - {cy}) * {Z} / {fy} = {Y:.2f}m")
        print(f"       Z = {Z}m")

        # 3) Yaw 복원
        ry = np.arctan2(det['sin_ry'], det['cos_ry'])
        print(f"    3) Yaw:")
        print(f"       ry = arctan2({det['sin_ry']}, {det['cos_ry']})")
        print(f"          = {ry:.4f} rad = {np.degrees(ry):.1f}도")

        dist = np.sqrt(X**2 + Y**2 + Z**2)
        print(f"    직선 거리: {dist:.2f}m")
        print()


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 4 Quiz Medium - 풀이")
    print("━" * 40)

    problem1_solution()
    problem2_solution()
    problem3_solution()

    print("\n" + "━" * 40)
