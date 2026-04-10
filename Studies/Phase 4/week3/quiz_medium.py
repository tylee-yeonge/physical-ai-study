"""
Phase 6 Week 3 - KITTI 데이터셋 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_parse_label():
    """
    문제 1: KITTI 레이블 파싱 및 해석

    아래 KITTI 레이블을 파싱하여 각 객체의 정보를 추출하시오.
    Car 클래스만 필터링하고, 가장 가까운 차의 거리(z)를 구하시오.

    TODO: closest_z 값을 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: KITTI 레이블 파싱")
    print("━" * 36 + "\n")

    label_text = """Car 0.00 0 -1.56 587.01 173.33 614.12 200.12 1.65 1.67 3.64 -0.65 1.71 46.70 -1.59
Pedestrian 0.00 0 -0.20 712.40 143.00 810.73 307.92 1.89 0.48 0.88 1.84 1.47 8.41 0.01
Car 0.00 0 1.85 387.63 181.54 423.81 203.12 1.52 1.64 3.88 -2.01 1.74 22.17 1.58
Cyclist 0.00 0 -1.65 548.00 171.33 572.40 194.42 1.75 0.50 1.95 -2.60 1.55 18.61 -1.63
Car 0.20 1 0.50 100.00 180.00 250.00 230.00 1.48 1.70 4.20 5.30 1.68 35.50 0.30"""

    print("  레이블:")
    for line in label_text.strip().split('\n'):
        parts = line.split()
        print(f"    {parts[0]:12s} z={float(parts[13]):>6.1f}m")

    # Car만 필터링
    cars = []
    for line in label_text.strip().split('\n'):
        parts = line.split()
        if parts[0] == 'Car':
            z = float(parts[13])
            cars.append(z)
            print(f"\n  Car 발견: z = {z}m")

    actual_closest = min(cars)
    print(f"\n  Car 거리 목록: {cars}")
    print(f"  힌트: 가장 작은 z 값을 찾으세요.")

    # TODO: 가장 가까운 Car의 z 값을 채우시오
    closest_z = 0.0  # 여기를 채우시오

    print(f"\n  직접 찾은 가장 가까운 Car의 z: {closest_z}m")
    print(f"  실제 가장 가까운 Car의 z: {actual_closest}m")

    if abs(closest_z - actual_closest) < 0.01:
        print("  정답!")
    else:
        print("  다시 확인해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_3d_bbox_projection():
    """
    문제 2: 3D BBox의 2D 투영 크기

    같은 크기의 Car (h=1.5, w=1.8, l=4.5)가 z=10m와 z=30m에 있을 때,
    이미지에 투영된 2D bbox의 대략적인 폭(pixel)을 비교하시오.

    간단한 근사: 투영 폭 ~ fx * w / z
    fx = 720

    TODO: width_10m과 width_30m을 계산하시오.
    """
    print("\n" + "━" * 36)
    print("문제 2: 거리에 따른 투영 크기 비교")
    print("━" * 36 + "\n")

    fx = 720.0
    w = 1.8  # 차 폭 (미터)

    z_near = 10.0
    z_far = 30.0

    actual_width_near = fx * w / z_near
    actual_width_far = fx * w / z_far

    print(f"  Car 폭: w = {w}m")
    print(f"  초점 거리: fx = {fx}")
    print()
    print(f"  근사 공식: 투영 폭 (pixels) ~ fx * w / z")
    print()
    print(f"  z = {z_near}m: 투영 폭 = {fx} * {w} / {z_near} = ?")
    print(f"  z = {z_far}m: 투영 폭 = {fx} * {w} / {z_far} = ?")

    # TODO: 각 거리에서의 투영 폭을 계산하시오
    width_10m = 0.0  # 여기를 채우시오
    width_30m = 0.0  # 여기를 채우시오

    print(f"\n  직접 계산:")
    print(f"    z=10m 투영 폭: {width_10m:.1f} pixels")
    print(f"    z=30m 투영 폭: {width_30m:.1f} pixels")
    print(f"\n  실제 계산:")
    print(f"    z=10m 투영 폭: {actual_width_near:.1f} pixels")
    print(f"    z=30m 투영 폭: {actual_width_far:.1f} pixels")
    print(f"    비율: {actual_width_near:.1f} / {actual_width_far:.1f} = {actual_width_near/actual_width_far:.1f}배")

    if abs(width_10m - actual_width_near) < 1 and abs(width_30m - actual_width_far) < 1:
        print("  정답!")
    else:
        print("  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem3_alpha_vs_ry():
    """
    문제 3: alpha와 ry의 관계

    alpha = ry - arctan2(x, z)

    객체 위치: x=5.0, z=20.0, ry=0.0 (전방을 향함)
    이 객체의 alpha 값을 계산하시오.

    TODO: expected_alpha를 계산하시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: alpha vs ry 계산")
    print("━" * 36 + "\n")

    x = 5.0
    z = 20.0
    ry = 0.0

    actual_alpha = ry - np.arctan2(x, z)

    print(f"  객체 위치: x={x}, z={z}")
    print(f"  ry = {ry} (전방을 향함)")
    print()
    print(f"  공식: alpha = ry - arctan2(x, z)")
    print(f"       alpha = {ry} - arctan2({x}, {z})")
    print(f"       alpha = {ry} - arctan({x}/{z})")
    print(f"       alpha = {ry} - arctan({x/z})")
    print()
    print(f"  힌트: arctan(0.25) = {np.arctan(0.25):.4f} 라디안")
    print(f"        = {np.degrees(np.arctan(0.25)):.2f}도")

    # TODO: alpha 값을 계산하시오
    expected_alpha = 0.0  # 여기를 채우시오

    print(f"\n  직접 계산한 alpha: {expected_alpha:.4f}")
    print(f"  실제 alpha: {actual_alpha:.4f}")

    if abs(expected_alpha - actual_alpha) < 0.01:
        print("  정답!")
    else:
        print("  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")
        print(f"\n  의미:")
        print(f"    ry=0이므로 차는 전방(+Z)을 향합니다.")
        print(f"    하지만 카메라에서 보면 오른쪽에 있으므로 (x=5)")
        print(f"    관측 각도(alpha)는 음수입니다 (왼쪽으로 돌아간 것처럼 보임)")


if __name__ == "__main__":
    print("━" * 40)
    print("  Phase 6 Week 3 Quiz - Medium")
    print("━" * 40)
    problem1_parse_label()
    problem2_3d_bbox_projection()
    problem3_alpha_vs_ry()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
