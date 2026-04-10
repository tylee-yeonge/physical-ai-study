"""
Phase 6 Week 1 - 3D Detection 개념 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_compute_box_corners():
    """
    문제 1: 3D BBox Corners 계산

    중심이 (0, 0, 10), 크기가 l=4.0, w=2.0, h=1.5인
    3D Bounding Box의 8개 꼭짓점 좌표를 구하시오.
    (yaw = 0, 회전 없음)

    TODO: corners 배열의 첫 번째 꼭짓점(corner 0) 좌표를 채우시오.
          corner 0 = (w/2, 0, l/2) + center
    """
    print("\n" + "━" * 36)
    print("문제 1: 3D BBox Corners 계산")
    print("━" * 36 + "\n")

    x, y, z = 0.0, 0.0, 10.0  # 중심 좌표
    l, w, h = 4.0, 2.0, 1.5   # 크기 (length, width, height)
    theta = 0.0                 # yaw (회전 없음)

    # 8개 꼭짓점 (회전 전, 중심 기준)
    # 순서: 바닥면 4개 -> 윗면 4개
    # KITTI 좌표계: x(오른쪽), y(아래쪽), z(전방)
    x_corners = [ w/2,  w/2, -w/2, -w/2,  w/2,  w/2, -w/2, -w/2]
    y_corners = [  0,     0,    0,    0,   -h,   -h,   -h,   -h ]
    z_corners = [ l/2, -l/2, -l/2,  l/2,  l/2, -l/2, -l/2,  l/2]

    corners = np.array([x_corners, y_corners, z_corners])  # (3, 8)
    corners[0, :] += x
    corners[1, :] += y
    corners[2, :] += z

    print(f"  중심: ({x}, {y}, {z})")
    print(f"  크기: l={l}, w={w}, h={h}")
    print(f"  회전: theta={theta}")
    print()
    print("  계산된 8개 꼭짓점:")
    for i in range(8):
        print(f"    Corner {i}: ({corners[0, i]:>6.1f}, {corners[1, i]:>6.1f}, {corners[2, i]:>6.1f})")

    # TODO: corner 0의 좌표를 직접 계산하여 채우시오
    expected_corner0 = np.array([0.0, 0.0, 0.0])  # 여기를 채우시오

    print(f"\n  직접 계산한 corner 0: {expected_corner0}")

    actual_corner0 = corners[:, 0]
    if np.allclose(expected_corner0, actual_corner0, atol=0.01):
        print("  정답!")
    else:
        print(f"  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")
        print(f"  힌트: corner 0 = (x + w/2, y + 0, z + l/2)")


def problem2_box_volume_iou():
    """
    문제 2: 3D Box 부피와 IoU 계산

    두 3D 박스(축 정렬, 회전 없음)의 IoU를 계산하시오.

    Box A: 중심 (0, 0, 10), 크기 l=4, w=2, h=1.5
    Box B: 중심 (1, 0, 11), 크기 l=4, w=2, h=1.5

    TODO: 각 축의 겹침(overlap)을 계산하고, 3D IoU를 구하시오.
    """
    print("\n" + "━" * 36)
    print("문제 2: 3D Box 부피와 IoU 계산")
    print("━" * 36 + "\n")

    # Box A
    a_center = np.array([0.0, 0.0, 10.0])
    a_size = np.array([4.0, 2.0, 1.5])  # l, w, h

    # Box B
    b_center = np.array([1.0, 0.0, 11.0])
    b_size = np.array([4.0, 2.0, 1.5])

    print(f"  Box A: 중심={a_center}, 크기(l,w,h)={a_size}")
    print(f"  Box B: 중심={b_center}, 크기(l,w,h)={b_size}")

    # 각 축 범위 (x: width, y: height, z: length)
    # x축: center_x +/- w/2
    a_x_min, a_x_max = a_center[0] - a_size[1]/2, a_center[0] + a_size[1]/2
    b_x_min, b_x_max = b_center[0] - b_size[1]/2, b_center[0] + b_size[1]/2

    # y축: center_y - h ~ center_y (KITTI: y가 아래로 양수)
    a_y_min, a_y_max = a_center[1] - a_size[2], a_center[1]
    b_y_min, b_y_max = b_center[1] - b_size[2], b_center[1]

    # z축: center_z +/- l/2
    a_z_min, a_z_max = a_center[2] - a_size[0]/2, a_center[2] + a_size[0]/2
    b_z_min, b_z_max = b_center[2] - b_size[0]/2, b_center[2] + b_size[0]/2

    print(f"\n  Box A 범위: x=[{a_x_min}, {a_x_max}], y=[{a_y_min}, {a_y_max}], z=[{a_z_min}, {a_z_max}]")
    print(f"  Box B 범위: x=[{b_x_min}, {b_x_max}], y=[{b_y_min}, {b_y_max}], z=[{b_z_min}, {b_z_max}]")

    # 겹침 계산
    overlap_x = max(0, min(a_x_max, b_x_max) - max(a_x_min, b_x_min))
    overlap_y = max(0, min(a_y_max, b_y_max) - max(a_y_min, b_y_min))
    overlap_z = max(0, min(a_z_max, b_z_max) - max(a_z_min, b_z_min))

    intersection = overlap_x * overlap_y * overlap_z
    vol_a = a_size[0] * a_size[1] * a_size[2]
    vol_b = b_size[0] * b_size[1] * b_size[2]
    union = vol_a + vol_b - intersection
    iou = intersection / union if union > 0 else 0

    print(f"\n  힌트:")
    print(f"    x축 겹침: min({a_x_max}, {b_x_max}) - max({a_x_min}, {b_x_min}) = ?")
    print(f"    y축 겹침: min({a_y_max}, {b_y_max}) - max({a_y_min}, {b_y_min}) = ?")
    print(f"    z축 겹침: min({a_z_max}, {b_z_max}) - max({a_z_min}, {b_z_min}) = ?")

    # TODO: IoU를 직접 계산하여 채우시오
    expected_iou = 0.0  # 여기를 채우시오

    print(f"\n  직접 계산한 IoU: {expected_iou}")
    print(f"  실제 IoU: {iou:.4f}")

    if abs(expected_iou - iou) < 0.01:
        print("  정답!")
    else:
        print("  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem3_rotation_effect():
    """
    문제 3: Yaw 회전이 Corner에 미치는 영향

    중심 (0, 0, 10), 크기 l=4, w=2, h=1.5의 3D Box가 있을 때,
    theta = pi/2 (90도 회전)를 적용하면 corner 0의 좌표가 어떻게 바뀌는가?

    회전 행렬 (y축 회전):
    R = [[cos(theta), 0, sin(theta)],
         [0,          1, 0         ],
         [-sin(theta), 0, cos(theta)]]

    TODO: 회전 후 corner 0의 좌표를 계산하시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: Yaw 회전 효과 계산")
    print("━" * 36 + "\n")

    x, y, z = 0.0, 0.0, 10.0
    l, w, h = 4.0, 2.0, 1.5
    theta = np.pi / 2  # 90도

    c = np.cos(theta)
    s = np.sin(theta)
    R = np.array([
        [ c, 0, s],
        [ 0, 1, 0],
        [-s, 0, c]
    ])

    # corner 0 (회전 전, 중심 기준): (w/2, 0, l/2) = (1.0, 0, 2.0)
    corner0_before = np.array([w/2, 0, l/2])

    print(f"  회전 전 corner 0 (중심 기준): {corner0_before}")
    print(f"  회전 각도: theta = pi/2 (90도)")
    print(f"\n  회전 행렬 R:")
    print(f"    [[ {c:>6.3f}, 0, {s:>6.3f}],")
    print(f"     [ 0,       1, 0      ],")
    print(f"     [{-s:>6.3f}, 0, {c:>6.3f}]]")

    # 회전 적용
    corner0_after_local = R @ corner0_before
    corner0_after = corner0_after_local + np.array([x, y, z])

    print(f"\n  R @ corner0 = ?")
    print(f"  힌트: cos(pi/2) = 0, sin(pi/2) = 1")
    print(f"  R @ [1, 0, 2] = [{c}*1 + {s}*2, 0, {-s}*1 + {c}*2]")

    # TODO: 회전 후 corner 0 좌표를 채우시오 (전역 좌표)
    expected_corner0 = np.array([0.0, 0.0, 0.0])  # 여기를 채우시오

    print(f"\n  직접 계산한 corner 0 (전역): {expected_corner0}")
    print(f"  실제 corner 0 (전역): [{corner0_after[0]:.3f}, {corner0_after[1]:.3f}, {corner0_after[2]:.3f}]")

    if np.allclose(expected_corner0, corner0_after, atol=0.1):
        print("  정답!")
    else:
        print("  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("━" * 40)
    print("  Phase 6 Week 1 Quiz - Medium")
    print("━" * 40)
    problem1_compute_box_corners()
    problem2_box_volume_iou()
    problem3_rotation_effect()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
