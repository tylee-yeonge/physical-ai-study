"""
Phase 6 Week 1 - 3D Detection 개념 중급 퀴즈 정답 및 해설
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: 3D BBox Corners 계산")
    print("━" * 36 + "\n")

    x, y, z = 0.0, 0.0, 10.0
    l, w, h = 4.0, 2.0, 1.5

    print("  중심: (0, 0, 10), 크기: l=4, w=2, h=1.5, theta=0")
    print()
    print("  Corner 0은 (w/2, 0, l/2) + center:")
    print(f"    x: 0 + {w}/2 = {x + w/2}")
    print(f"    y: 0 + 0 = {y + 0}")
    print(f"    z: 10 + {l}/2 = {z + l/2}")
    print()
    print(f"  정답: corner 0 = ({x + w/2}, {y}, {z + l/2})")
    print(f"       즉, (1.0, 0.0, 12.0)")
    print()

    # 전체 8개 꼭짓점 검증
    x_corners = [ w/2,  w/2, -w/2, -w/2,  w/2,  w/2, -w/2, -w/2]
    y_corners = [  0,     0,    0,    0,   -h,   -h,   -h,   -h ]
    z_corners = [ l/2, -l/2, -l/2,  l/2,  l/2, -l/2, -l/2,  l/2]

    print("  전체 8개 꼭짓점 (전역 좌표):")
    for i in range(8):
        cx = x_corners[i] + x
        cy = y_corners[i] + y
        cz = z_corners[i] + z
        print(f"    Corner {i}: ({cx:>5.1f}, {cy:>5.1f}, {cz:>5.1f})")

    print()
    print("  핵심:")
    print("    - 바닥면 (0-3): y = 0 (중심의 y와 같음)")
    print("    - 윗면 (4-7): y = -h (높이만큼 위로)")
    print("    - KITTI에서 y축은 아래가 양수이므로 윗면이 -h")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: 3D Box 부피와 IoU")
    print("━" * 36 + "\n")

    # Box A
    a_center = np.array([0.0, 0.0, 10.0])
    a_size = np.array([4.0, 2.0, 1.5])  # l, w, h

    # Box B
    b_center = np.array([1.0, 0.0, 11.0])
    b_size = np.array([4.0, 2.0, 1.5])

    # x축 겹침 (width 방향)
    a_x_min, a_x_max = 0.0 - 1.0, 0.0 + 1.0  # [-1, 1]
    b_x_min, b_x_max = 1.0 - 1.0, 1.0 + 1.0  # [0, 2]
    overlap_x = min(1.0, 2.0) - max(-1.0, 0.0)  # 1.0 - 0.0 = 1.0

    # y축 겹침 (height 방향)
    a_y_min, a_y_max = 0.0 - 1.5, 0.0  # [-1.5, 0]
    b_y_min, b_y_max = 0.0 - 1.5, 0.0  # [-1.5, 0]
    overlap_y = min(0.0, 0.0) - max(-1.5, -1.5)  # 0.0 - (-1.5) = 1.5

    # z축 겹침 (length 방향)
    a_z_min, a_z_max = 10.0 - 2.0, 10.0 + 2.0  # [8, 12]
    b_z_min, b_z_max = 11.0 - 2.0, 11.0 + 2.0  # [9, 13]
    overlap_z = min(12.0, 13.0) - max(8.0, 9.0)  # 12.0 - 9.0 = 3.0

    intersection = overlap_x * overlap_y * overlap_z
    vol_a = 4.0 * 2.0 * 1.5  # 12.0
    vol_b = 4.0 * 2.0 * 1.5  # 12.0
    union = vol_a + vol_b - intersection
    iou = intersection / union

    print("  단계별 계산:")
    print()
    print(f"  1. x축 (width) 겹침:")
    print(f"     A: [{a_x_min}, {a_x_max}], B: [{b_x_min}, {b_x_max}]")
    print(f"     overlap_x = min(1, 2) - max(-1, 0) = 1.0 - 0.0 = {overlap_x}")
    print()
    print(f"  2. y축 (height) 겹침:")
    print(f"     A: [{a_y_min}, {a_y_max}], B: [{b_y_min}, {b_y_max}]")
    print(f"     overlap_y = min(0, 0) - max(-1.5, -1.5) = 0 - (-1.5) = {overlap_y}")
    print()
    print(f"  3. z축 (length) 겹침:")
    print(f"     A: [{a_z_min}, {a_z_max}], B: [{b_z_min}, {b_z_max}]")
    print(f"     overlap_z = min(12, 13) - max(8, 9) = 12 - 9 = {overlap_z}")
    print()
    print(f"  4. Intersection = {overlap_x} * {overlap_y} * {overlap_z} = {intersection}")
    print(f"  5. Vol_A = {vol_a}, Vol_B = {vol_b}")
    print(f"  6. Union = {vol_a} + {vol_b} - {intersection} = {union}")
    print(f"  7. IoU = {intersection} / {union} = {iou:.4f}")
    print()
    print(f"  정답: IoU = {iou:.4f}")
    print()
    print(f"  KITTI 기준 (Car IoU >= 0.7): {'TP' if iou >= 0.7 else 'FP'}")
    print(f"  -> 1m의 x, z 오차만으로도 IoU가 0.7 미만이 됩니다!")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: Yaw 회전 효과")
    print("━" * 36 + "\n")

    theta = np.pi / 2
    c = np.cos(theta)  # ~0
    s = np.sin(theta)  # ~1

    print(f"  회전 각도: theta = pi/2")
    print(f"  cos(pi/2) = {c:.6f} (약 0)")
    print(f"  sin(pi/2) = {s:.6f} (약 1)")
    print()
    print("  회전 전 corner 0 (중심 기준): (1.0, 0, 2.0)")
    print()
    print("  R @ [1, 0, 2]:")
    print(f"    x' = cos(pi/2)*1 + sin(pi/2)*2 = 0*1 + 1*2 = 2.0")
    print(f"    y' = 0 (y축 회전이므로 y 불변)")
    print(f"    z' = -sin(pi/2)*1 + cos(pi/2)*2 = -1*1 + 0*2 = -1.0")
    print()
    print("  회전 후 (중심 기준): (2.0, 0, -1.0)")
    print("  전역 좌표: (2.0 + 0, 0 + 0, -1.0 + 10) = (2.0, 0, 9.0)")
    print()
    print("  정답: corner 0 (전역) = (2.0, 0.0, 9.0)")
    print()

    # 검증
    R = np.array([
        [ c, 0, s],
        [ 0, 1, 0],
        [-s, 0, c]
    ])
    corner0 = np.array([1.0, 0, 2.0])
    rotated = R @ corner0
    result = rotated + np.array([0, 0, 10])
    print(f"  검증: {result}")
    print()
    print("  의미: 90도 회전하면 length 방향과 width 방향이 바뀝니다.")
    print("  원래 전방을 향하던 차가 이제 옆을 향하게 됩니다.")
    print("  이것이 yaw(heading) 각도가 자율주행에서 중요한 이유입니다.")


if __name__ == "__main__":
    print("━" * 40)
    print("  Phase 6 Week 1 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
