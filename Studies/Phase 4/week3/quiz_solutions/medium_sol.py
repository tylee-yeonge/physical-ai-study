"""
Phase 6 Week 3 - KITTI 데이터셋 중급 퀴즈 정답 및 해설
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: 레이블 파싱")
    print("━" * 36 + "\n")

    print("  레이블에서 Car 클래스만 추출:")
    print("    Car 1: z = 46.70m")
    print("    Car 2: z = 22.17m")
    print("    Car 3: z = 35.50m")
    print()
    print("  가장 가까운 Car의 z = min(46.70, 22.17, 35.50) = 22.17m")
    print()
    print("  정답: closest_z = 22.17")
    print()
    print("  핵심 포인트:")
    print("    1. 레이블 파싱 시 class 필드(첫 번째)로 필터링")
    print("    2. z 좌표(13번째 필드)가 '전방 거리'")
    print("    3. 가장 가까운 = z값이 가장 작은 것")
    print()
    print("  실무에서:")
    print("    가장 가까운 객체가 충돌 위험이 가장 높으므로")
    print("    자율주행에서는 이 정보가 매우 중요합니다.")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: 거리에 따른 투영 크기")
    print("━" * 36 + "\n")

    fx = 720.0
    w = 1.8

    width_10 = fx * w / 10.0
    width_30 = fx * w / 30.0

    print(f"  투영 폭 = fx * w / z")
    print()
    print(f"  z = 10m:")
    print(f"    폭 = {fx} * {w} / 10 = {fx * w} / 10 = {width_10:.1f} pixels")
    print()
    print(f"  z = 30m:")
    print(f"    폭 = {fx} * {w} / 30 = {fx * w} / 30 = {width_30:.1f} pixels")
    print()
    print(f"  정답: width_10m = {width_10:.1f}, width_30m = {width_30:.1f}")
    print()
    print(f"  비율: {width_10:.1f} / {width_30:.1f} = {width_10/width_30:.1f}배")
    print()
    print("  의미:")
    print("    거리가 3배 멀어지면 이미지에서 크기가 3배 작아집니다.")
    print("    이것이 '원근 투영'의 핵심 성질입니다.")
    print()
    print("  Monocular 3D Detection과의 연결:")
    print("    이미지에서 큰 bbox = 가까이 있을 가능성 높음")
    print("    이미지에서 작은 bbox = 멀리 있을 가능성 높음")
    print("    → 네트워크는 이 '크기 단서'를 활용하여 depth를 추정합니다.")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: alpha vs ry")
    print("━" * 36 + "\n")

    x = 5.0
    z = 20.0
    ry = 0.0

    alpha = ry - np.arctan2(x, z)

    print(f"  alpha = ry - arctan2(x, z)")
    print(f"        = {ry} - arctan2({x}, {z})")
    print(f"        = {ry} - arctan({x}/{z})")
    print(f"        = {ry} - arctan({x/z})")
    print(f"        = {ry} - {np.arctan2(x, z):.4f}")
    print(f"        = {alpha:.4f} 라디안")
    print(f"        = {np.degrees(alpha):.2f}도")
    print()
    print(f"  정답: alpha = {alpha:.4f} (약 -0.2450)")
    print()
    print("  의미:")
    print(f"    ry = 0: 차량이 전방(+Z)을 향하고 있음 (전역 방향)")
    print(f"    alpha = {alpha:.4f}: 카메라에서 보면 약간 왼쪽을 향하는 것처럼 보임")
    print()
    print("    이유: 차가 오른쪽(x=5)에 있기 때문에")
    print("    카메라 시점에서는 차의 왼쪽 면이 더 많이 보입니다.")
    print("    이 시점 차이를 alpha가 반영합니다.")
    print()
    print("  ry vs alpha 비교:")
    print("    ry: 전역 방향 (카메라 위치에 무관)")
    print("    alpha: 관측 방향 (카메라에서 바라본 상대적 방향)")
    print("    3D bbox 계산에는 ry를 사용합니다!")


if __name__ == "__main__":
    print("━" * 40)
    print("  Phase 6 Week 3 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
