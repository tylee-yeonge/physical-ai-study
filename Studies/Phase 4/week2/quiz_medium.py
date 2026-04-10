"""
Phase 6 Week 2 - 좌표계 이해 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_projection():
    """
    문제 1: 3D -> 2D 투영 계산

    Camera 좌표계의 3D 점 (2.0, 1.0, 10.0)을
    P2 행렬로 이미지에 투영했을 때의 (u, v) 좌표를 구하시오.

    P2 = [[720,   0, 600, 0],
          [  0, 720, 180, 0],
          [  0,   0,   1, 0]]

    TODO: u, v 값을 직접 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: 3D -> 2D 투영 계산")
    print("━" * 36 + "\n")

    P2 = np.array([
        [720,   0, 600, 0],
        [  0, 720, 180, 0],
        [  0,   0,   1, 0]
    ])

    pt_3d = np.array([2.0, 1.0, 10.0, 1.0])  # 동차 좌표

    # 투영
    pt_2d_hom = P2 @ pt_3d  # [u*z, v*z, z]

    print(f"  P2 @ [2, 1, 10, 1]^T = [{pt_2d_hom[0]:.1f}, {pt_2d_hom[1]:.1f}, {pt_2d_hom[2]:.1f}]")
    print()
    print(f"  힌트:")
    print(f"    u*z = 720*2 + 0*1 + 600*10 + 0 = ?")
    print(f"    v*z = 0*2 + 720*1 + 180*10 + 0 = ?")
    print(f"    z   = 0*2 + 0*1 + 1*10 + 0 = ?")
    print()
    print(f"    u = u*z / z = ?")
    print(f"    v = v*z / z = ?")

    # 정규화
    u = pt_2d_hom[0] / pt_2d_hom[2]
    v = pt_2d_hom[1] / pt_2d_hom[2]

    # TODO: 직접 계산한 값을 채우시오
    expected_u = 0.0  # 여기를 채우시오
    expected_v = 0.0  # 여기를 채우시오

    print(f"\n  직접 계산: u={expected_u}, v={expected_v}")
    print(f"  실제 결과: u={u:.1f}, v={v:.1f}")

    if abs(expected_u - u) < 0.1 and abs(expected_v - v) < 0.1:
        print("  정답!")
    else:
        print("  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_corners_and_projection():
    """
    문제 2: KITTI 3D BBox Corners 및 투영

    KITTI 레이블: h=1.5, w=1.8, l=4.5, x=0, y=1.65, z=10, ry=0
    P2 = [[720, 0, 600, 0], [0, 720, 180, 0], [0, 0, 1, 0]]

    Corner 0 (바닥면 전면 오른쪽)의 3D 좌표를 구하고,
    이를 이미지에 투영한 (u, v)를 계산하시오.

    TODO: corner0_3d와 corner0_2d를 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 2: KITTI BBox Corner 투영")
    print("━" * 36 + "\n")

    h, w, l = 1.5, 1.8, 4.5
    x, y, z = 0.0, 1.65, 10.0
    ry = 0.0

    P2 = np.array([
        [720,   0, 600, 0],
        [  0, 720, 180, 0],
        [  0,   0,   1, 0]
    ])

    print(f"  KITTI 레이블: h={h}, w={w}, l={l}, x={x}, y={y}, z={z}, ry={ry}")
    print()
    print(f"  Corner 0 (중심 기준, 회전 전):")
    print(f"    x_c = l/2 = {l/2}")
    print(f"    y_c = 0")
    print(f"    z_c = w/2 = {w/2}")
    print()
    print(f"  ry=0이므로 회전 없음.")
    print(f"  전역 좌표 = (x + l/2, y + 0, z + w/2)")
    print(f"             = ({x} + {l/2}, {y} + 0, {z} + {w/2})")

    # 실제 corner 0 계산
    corner0_actual = np.array([x + l/2, y, z + w/2])

    # 투영
    pt_hom = np.append(corner0_actual, 1.0)
    pt_2d = P2 @ pt_hom
    u_actual = pt_2d[0] / pt_2d[2]
    v_actual = pt_2d[1] / pt_2d[2]

    # TODO: 직접 계산
    corner0_3d_expected = np.array([0.0, 0.0, 0.0])  # 여기를 채우시오
    corner0_u_expected = 0.0  # 여기를 채우시오
    corner0_v_expected = 0.0  # 여기를 채우시오

    print(f"\n  직접 계산한 corner 0 (3D): {corner0_3d_expected}")
    print(f"  실제 corner 0 (3D): {corner0_actual}")
    print(f"\n  직접 계산한 corner 0 (2D): u={corner0_u_expected}, v={corner0_v_expected}")
    print(f"  실제 corner 0 (2D): u={u_actual:.1f}, v={v_actual:.1f}")

    if (np.allclose(corner0_3d_expected, corner0_actual, atol=0.01) and
        abs(corner0_u_expected - u_actual) < 0.5 and
        abs(corner0_v_expected - v_actual) < 0.5):
        print("  정답!")
    else:
        print("  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem3_bev_distance():
    """
    문제 3: BEV에서 두 객체 사이의 거리

    Camera 좌표계에서 두 객체의 위치:
      Car A: (x=2.0, y=1.65, z=15.0)
      Car B: (x=-3.0, y=1.65, z=25.0)

    BEV(X-Z 평면)에서 두 객체 사이의 유클리드 거리를 구하시오.
    (높이 y는 무시)

    TODO: distance 값을 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: BEV 거리 계산")
    print("━" * 36 + "\n")

    car_a = np.array([2.0, 1.65, 15.0])
    car_b = np.array([-3.0, 1.65, 25.0])

    print(f"  Car A 위치: x={car_a[0]}, y={car_a[1]}, z={car_a[2]}")
    print(f"  Car B 위치: x={car_b[0]}, y={car_b[1]}, z={car_b[2]}")
    print()
    print(f"  BEV 거리 = sqrt((x_A - x_B)^2 + (z_A - z_B)^2)")
    print(f"           = sqrt(({car_a[0]} - {car_b[0]})^2 + ({car_a[2]} - {car_b[2]})^2)")
    print(f"           = sqrt(({car_a[0] - car_b[0]})^2 + ({car_a[2] - car_b[2]})^2)")

    actual_dist = np.sqrt((car_a[0] - car_b[0])**2 + (car_a[2] - car_b[2])**2)

    # TODO: 거리를 직접 계산하여 채우시오
    expected_distance = 0.0  # 여기를 채우시오

    print(f"\n  직접 계산한 거리: {expected_distance:.2f} m")
    print(f"  실제 거리: {actual_dist:.2f} m")

    if abs(expected_distance - actual_dist) < 0.1:
        print("  정답!")
    else:
        print("  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")
        print(f"  힌트: sqrt(25 + 100) = sqrt(125) = ?")


if __name__ == "__main__":
    print("━" * 40)
    print("  Phase 6 Week 2 Quiz - Medium")
    print("━" * 40)
    problem1_projection()
    problem2_corners_and_projection()
    problem3_bev_distance()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
