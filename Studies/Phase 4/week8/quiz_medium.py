"""
Phase 6 Week 8 - BEV 개념 이해 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_bev_grid_calculation():
    """
    문제 1: BEV 그리드 설계

    자율주행 시스템의 BEV 그리드를 설계하시오.

    요구사항:
    - 커버 범위: 전방 50m, 후방 10m, 좌우 각 25m
    - 셀 크기: 0.5m x 0.5m
    """
    print("\n" + "━" * 36)
    print("문제 1: BEV 그리드 설계")
    print("━" * 36 + "\n")

    print("  요구사항:")
    print("    커버 범위: 전방 50m, 후방 10m, 좌우 25m")
    print("    셀 크기: 0.5m x 0.5m")
    print()

    print("  과제:")
    print("  1) BEV 그리드의 크기(H x W)는? (셀 단위)")
    print("  2) 총 셀 수는?")
    print("  3) Feature dim = 256일 때 BEV Feature의 메모리는? (FP32)")
    print("  4) 셀 크기를 0.25m로 줄이면 메모리가 몇 배 증가하는가?")
    print()

    # TODO: 학생이 계산
    print("  BEV 그리드 크기: ___ x ___")
    print("  총 셀 수: ___")
    print("  메모리 (FP32): ___ MB")
    print("  0.25m 셀 시 메모리 증가: ___배")


def problem2_ipm_projection():
    """
    문제 2: IPM 좌표 변환 계산

    카메라 파라미터가 주어졌을 때, 지면 위의 3D 점을
    이미지에 투영하시오.

    카메라: fx=721.5, fy=721.5, cx=609.6, cy=172.9
    카메라 높이: 1.65m (지면에서)
    카메라 pitch: 0 (수평)
    """
    print("\n" + "━" * 36)
    print("문제 2: IPM 좌표 변환")
    print("━" * 36 + "\n")

    fx, fy = 721.5, 721.5
    cx, cy = 609.6, 172.9
    cam_height = 1.65

    print(f"  카메라: fx={fx}, fy={fy}, cx={cx}, cy={cy}")
    print(f"  카메라 높이: {cam_height}m")
    print()

    # 지면 위의 3D 점 (카메라 좌표계: X=오른쪽, Y=아래, Z=전방)
    ground_points = [
        {"name": "A", "X": 0.0, "Z": 10.0},    # 정면 10m
        {"name": "B", "X": 3.0, "Z": 20.0},     # 오른쪽 3m, 전방 20m
        {"name": "C", "X": -5.0, "Z": 40.0},    # 왼쪽 5m, 전방 40m
    ]

    print("  지면 위의 3D 점 (Y = cam_height, 지면에 있음):")
    for pt in ground_points:
        print(f"    점 {pt['name']}: X={pt['X']:.1f}m, Z={pt['Z']:.1f}m")
    print()

    print("  과제:")
    print("  1) 각 점의 카메라 좌표계 Y값은? (힌트: 카메라가 지면 위에 있음)")
    print("  2) 각 점을 이미지 좌표 (u, v)로 투영하시오.")
    print("     u = fx * X/Z + cx")
    print("     v = fy * Y/Z + cy")
    print("  3) 점 C는 이미지 밖에 있는가? (이미지 크기: 1242 x 375)")


def problem3_depth_distribution():
    """
    문제 3: Lift-Splat의 Depth 분포 분석

    3개 픽셀의 Depth 분포가 주어졌을 때,
    BEV Feature에 대한 영향을 분석하시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: Depth 분포 → BEV 영향")
    print("━" * 36 + "\n")

    depth_bins = np.arange(2, 62, 2)  # 2m ~ 60m, 2m 간격 (30 bins)

    # 픽셀 A: 가까운 차량 (명확한 depth)
    dist_a = np.zeros(30)
    dist_a[4] = 0.8   # 10m
    dist_a[3] = 0.1   # 8m
    dist_a[5] = 0.1   # 12m

    # 픽셀 B: 먼 차량 (불확실한 depth)
    dist_b = np.zeros(30)
    for i in range(12, 20):
        dist_b[i] = np.exp(-0.3 * (i - 16)**2)
    dist_b /= dist_b.sum()

    # 픽셀 C: 하늘 (depth 없음)
    dist_c = np.ones(30) / 30

    print("  Depth bins: 2m ~ 60m (2m 간격, 30 bins)")
    print()
    print("  픽셀 A (가까운 차량):")
    peak_a = depth_bins[np.argmax(dist_a)]
    print(f"    피크: {peak_a}m, 확률: {max(dist_a):.1f}")
    print()
    print("  픽셀 B (먼 차량):")
    peak_b = depth_bins[np.argmax(dist_b)]
    print(f"    피크: {peak_b}m, 확률: {max(dist_b):.2f}")
    print()
    print("  픽셀 C (하늘):")
    print(f"    균일 분포, 각 bin 확률: {1/30:.4f}")
    print()

    print("  과제:")
    print("  1) 각 픽셀의 Feature가 BEV에서 어떻게 분포되는가?")
    print("     (집중 vs 분산)")
    print("  2) 하늘 픽셀(C)이 BEV에 미치는 영향은?")
    print("  3) Depth 분포의 '확신도'가 높을수록 BEV에 미치는 영향은?")
    print("  4) Depth GT를 사용한 감독(supervision)이 중요한 이유는?")


if __name__ == "__main__":
    print("━" * 40)
    print("  Phase 6 Week 8 Quiz - Medium")
    print("━" * 40)

    problem1_bev_grid_calculation()
    problem2_ipm_projection()
    problem3_depth_distribution()

    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
