"""
Phase 5 Week 11 - Detection + Depth 융합 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_unprojection_calculation():
    """
    문제 1: 역투영 직접 계산

    카메라 파라미터:
      fx = 500, fy = 500, cx = 320, cy = 240

    YOLO가 검출한 물체:
      BBox 중심: (u=400, v=200)
      Depth: Z = 10.0m

    TODO: 이 물체의 3D 좌표 (X, Y, Z)를 계산하시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: 역투영 직접 계산")
    print("━" * 36 + "\n")

    fx, fy = 500.0, 500.0
    cx, cy = 320.0, 240.0
    u, v = 400.0, 200.0
    Z = 10.0

    print(f"  카메라 파라미터: fx={fx}, fy={fy}, cx={cx}, cy={cy}")
    print(f"  픽셀 좌표: (u={u}, v={v})")
    print(f"  깊이: Z = {Z}m")
    print()

    # 역투영 공식
    X = (u - cx) * Z / fx
    Y = (v - cy) * Z / fy

    print(f"  계산:")
    print(f"    X = (u - cx) * Z / fx = ({u} - {cx}) * {Z} / {fx} = {X:.2f}m")
    print(f"    Y = (v - cy) * Z / fy = ({v} - {cy}) * {Z} / {fy} = {Y:.2f}m")
    print(f"    Z = {Z:.2f}m")
    print()

    # TODO: 직접 계산한 값을 채우시오
    expected_X = 0.0  # 여기를 채우시오
    expected_Y = 0.0  # 여기를 채우시오

    print(f"  직접 계산한 값: X={expected_X}, Y={expected_Y}")

    distance = np.sqrt(X**2 + Y**2 + Z**2)
    print(f"\n  물체까지 직선 거리: {distance:.2f}m")

    if abs(expected_X - X) < 0.1 and abs(expected_Y - Y) < 0.1:
        print("\n  정답!")
    else:
        print("\n  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_robust_depth_comparison():
    """
    문제 2: Depth 추출 전략 비교

    BBox 내 깊이 값이 아래와 같을 때,
    3가지 전략의 결과를 비교하시오.

    BBox 내 깊이 분포 (5x5 영역):
    물체(10m 부근) + 배경(50m)이 섞여 있음
    """
    print("\n" + "━" * 36)
    print("문제 2: Depth 추출 전략 비교")
    print("━" * 36 + "\n")

    # BBox 내 깊이 (물체 10m + 배경 50m이 혼합)
    depth_roi = np.array([
        [50.0, 50.0, 10.2, 10.1, 50.0],
        [50.0, 10.3, 10.0,  9.8, 50.0],
        [10.5, 10.1,  9.9, 10.2, 10.4],
        [50.0, 10.2, 10.3, 10.0, 50.0],
        [50.0, 50.0, 10.1, 50.0, 50.0],
    ])

    print("  BBox 내 깊이 맵 (미터):")
    for row in depth_roi:
        print("    " + "  ".join(f"{v:5.1f}" for v in row))

    print()

    # 방법 1: 전체 평균
    mean_depth = np.mean(depth_roi)
    print(f"  방법 1 (전체 평균): {mean_depth:.2f}m")

    # 방법 2: 중앙값
    median_depth = np.median(depth_roi)
    print(f"  방법 2 (중앙값):    {median_depth:.2f}m")

    # 방법 3: 중심 20% 영역의 중앙값
    center = depth_roi[1:4, 1:4]  # 중심 3x3
    center_median = np.median(center)
    print(f"  방법 3 (중심 median): {center_median:.2f}m")

    # 방법 4: 최솟값
    min_depth = np.min(depth_roi)
    print(f"  방법 4 (최솟값):    {min_depth:.2f}m")

    print()
    print("  질문: 물체의 실제 깊이가 약 10m일 때, 어떤 방법이 가장 정확한가?")
    print("  그리고 방법 1(평균)이 부정확한 이유는?")
    print()
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def problem3_multi_object_3d():
    """
    문제 3: 다중 물체 3D 위치 추정

    3개의 검출 결과로부터 3D 좌표를 구하고,
    가장 가까운 물체를 찾으시오.

    카메라: fx=500, fy=500, cx=320, cy=240
    """
    print("\n" + "━" * 36)
    print("문제 3: 다중 물체 3D 위치")
    print("━" * 36 + "\n")

    fx, fy, cx, cy = 500.0, 500.0, 320.0, 240.0

    detections = [
        {'class': 'car',    'center': (200, 250), 'depth': 15.0},
        {'class': 'person', 'center': (400, 300), 'depth': 5.0},
        {'class': 'car',    'center': (550, 220), 'depth': 30.0},
    ]

    print(f"  카메라: fx={fx}, fy={fy}, cx={cx}, cy={cy}")
    print(f"  검출 수: {len(detections)}")
    print()

    results = []
    for i, det in enumerate(detections):
        u, v = det['center']
        Z = det['depth']
        X = (u - cx) * Z / fx
        Y = (v - cy) * Z / fy
        dist = np.sqrt(X**2 + Y**2 + Z**2)

        results.append({
            'class': det['class'],
            'position': (X, Y, Z),
            'distance': dist
        })

        print(f"  물체 {i+1} ({det['class']}):")
        print(f"    픽셀: ({u}, {v}), Depth: {Z}m")
        print(f"    3D: ({X:.2f}, {Y:.2f}, {Z:.2f})m")
        print(f"    직선 거리: {dist:.2f}m")
        print()

    # 가장 가까운 물체 찾기
    nearest = min(results, key=lambda x: x['distance'])
    print(f"  질문: 가장 가까운 물체는?")
    print(f"  (직접 확인하고, quiz_solutions/medium_sol.py와 비교하세요)")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 11 Quiz - Medium")
    print("━" * 40)
    problem1_unprojection_calculation()
    problem2_robust_depth_comparison()
    problem3_multi_object_3d()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
