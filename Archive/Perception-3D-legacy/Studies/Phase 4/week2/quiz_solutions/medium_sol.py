"""
Phase 6 Week 2 - 좌표계 이해 중급 퀴즈 정답 및 해설
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: 3D -> 2D 투영")
    print("━" * 36 + "\n")

    P2 = np.array([
        [720,   0, 600, 0],
        [  0, 720, 180, 0],
        [  0,   0,   1, 0]
    ])
    pt_3d = np.array([2.0, 1.0, 10.0, 1.0])

    print("  단계별 계산:")
    print()
    print("  P2 @ [2, 1, 10, 1]^T:")
    print(f"    u*z = 720*2 + 0*1 + 600*10 + 0*1 = 1440 + 0 + 6000 + 0 = 7440")
    print(f"    v*z = 0*2 + 720*1 + 180*10 + 0*1 = 0 + 720 + 1800 + 0 = 2520")
    print(f"    z   = 0*2 + 0*1 + 1*10 + 0*1 = 10")
    print()
    print(f"  정규화 (z로 나누기):")
    print(f"    u = 7440 / 10 = 744.0")
    print(f"    v = 2520 / 10 = 252.0")
    print()
    print(f"  정답: u = 744.0, v = 252.0")

    # 검증
    result = P2 @ pt_3d
    u = result[0] / result[2]
    v = result[1] / result[2]
    print(f"\n  검증: u={u:.1f}, v={v:.1f}")
    print()
    print("  의미:")
    print("    3D 점 (2, 1, 10)은 전방 10m, 오른쪽 2m, 아래 1m에 있고,")
    print("    이미지에서는 (744, 252) 픽셀에 나타납니다.")
    print("    cx=600보다 오른쪽 (x>0이므로), cy=180보다 아래 (y>0이므로)")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: KITTI BBox Corner 투영")
    print("━" * 36 + "\n")

    h, w, l = 1.5, 1.8, 4.5
    x, y, z = 0.0, 1.65, 10.0

    print("  Corner 0 (3D) 계산:")
    print(f"    중심 기준: (l/2, 0, w/2) = ({l/2}, 0, {w/2})")
    print(f"    ry=0이므로 회전 없음")
    print(f"    전역: ({x}+{l/2}, {y}+0, {z}+{w/2}) = ({x+l/2}, {y}, {z+w/2})")
    print()
    print(f"  정답: corner 0 (3D) = ({x+l/2}, {y}, {z+w/2}) = (2.25, 1.65, 10.9)")

    P2 = np.array([
        [720,   0, 600, 0],
        [  0, 720, 180, 0],
        [  0,   0,   1, 0]
    ])

    corner0 = np.array([2.25, 1.65, 10.9, 1.0])
    pt_2d = P2 @ corner0

    u = pt_2d[0] / pt_2d[2]
    v = pt_2d[1] / pt_2d[2]

    print()
    print("  Corner 0 (2D) 계산:")
    print(f"    P2 @ [2.25, 1.65, 10.9, 1]^T:")
    print(f"    u*z = 720*2.25 + 600*10.9 = 1620 + 6540 = 8160")
    print(f"    v*z = 720*1.65 + 180*10.9 = 1188 + 1962 = 3150")
    print(f"    z   = 10.9")
    print()
    print(f"    u = 8160 / 10.9 = {u:.1f}")
    print(f"    v = 3150 / 10.9 = {v:.1f}")
    print()
    print(f"  정답: corner 0 (2D) = (u={u:.1f}, v={v:.1f})")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: BEV 거리 계산")
    print("━" * 36 + "\n")

    car_a = np.array([2.0, 1.65, 15.0])
    car_b = np.array([-3.0, 1.65, 25.0])

    dx = car_a[0] - car_b[0]  # 2 - (-3) = 5
    dz = car_a[2] - car_b[2]  # 15 - 25 = -10

    dist = np.sqrt(dx**2 + dz**2)

    print("  BEV 거리는 X-Z 평면의 유클리드 거리입니다.")
    print("  (높이 Y는 무시)")
    print()
    print(f"  dx = {car_a[0]} - ({car_b[0]}) = {dx}")
    print(f"  dz = {car_a[2]} - {car_b[2]} = {dz}")
    print()
    print(f"  distance = sqrt({dx}^2 + {dz}^2)")
    print(f"           = sqrt({dx**2} + {dz**2})")
    print(f"           = sqrt({dx**2 + dz**2})")
    print(f"           = {dist:.2f} m")
    print()
    print(f"  정답: {dist:.2f} m (약 11.18m)")
    print()
    print("  의미:")
    print("    두 차 사이의 BEV 거리는 약 11.18m입니다.")
    print("    X 방향 5m + Z 방향 10m의 대각선 거리이며,")
    print("    자율주행에서 경로 계획 시 이 거리를 사용합니다.")
    print()
    print("  참고: 3D 거리와 BEV 거리의 차이")
    print(f"    3D 거리 = sqrt({dx}^2 + 0^2 + {dz}^2) = {dist:.2f} m")
    print(f"    (두 차의 y가 같으므로 이 경우에는 BEV와 3D 거리가 같음)")


if __name__ == "__main__":
    print("━" * 40)
    print("  Phase 6 Week 2 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
