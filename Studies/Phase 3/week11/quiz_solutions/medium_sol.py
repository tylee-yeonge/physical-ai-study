"""
Phase 5 Week 11 - Detection + Depth 융합 중급 퀴즈 정답 및 해설
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: X = 1.6m, Y = -0.8m")
    print("━" * 36 + "\n")

    fx, fy = 500.0, 500.0
    cx, cy = 320.0, 240.0
    u, v = 400.0, 200.0
    Z = 10.0

    X = (u - cx) * Z / fx
    Y = (v - cy) * Z / fy

    print("해설:")
    print(f"  X = (u - cx) * Z / fx")
    print(f"    = ({u} - {cx}) * {Z} / {fx}")
    print(f"    = {u - cx} * {Z} / {fx}")
    print(f"    = {X:.2f}m (오른쪽으로 {X:.1f}m)")
    print()
    print(f"  Y = (v - cy) * Z / fy")
    print(f"    = ({v} - {cy}) * {Z} / {fy}")
    print(f"    = {v - cy} * {Z} / {fy}")
    print(f"    = {Y:.2f}m (위로 {abs(Y):.1f}m, Y가 음수 = 위)")
    print()
    print(f"  Z = {Z:.2f}m (전방)")
    print()

    dist = np.sqrt(X**2 + Y**2 + Z**2)
    print(f"  직선 거리 = sqrt({X:.2f}^2 + {Y:.2f}^2 + {Z:.2f}^2)")
    print(f"           = sqrt({X**2:.2f} + {Y**2:.2f} + {Z**2:.2f})")
    print(f"           = {dist:.2f}m")
    print()
    print("  물리적 의미:")
    print(f"  → 물체는 카메라 기준으로 전방 {Z}m, 오른쪽 {X}m, 위 {abs(Y)}m에 위치")
    print(f"  → 직선 거리는 {dist:.2f}m (전방 거리 {Z}m보다 약간 멀음)")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: 방법 3 (중심 median) = 10.1m")
    print("━" * 36 + "\n")

    depth_roi = np.array([
        [50.0, 50.0, 10.2, 10.1, 50.0],
        [50.0, 10.3, 10.0,  9.8, 50.0],
        [10.5, 10.1,  9.9, 10.2, 10.4],
        [50.0, 10.2, 10.3, 10.0, 50.0],
        [50.0, 50.0, 10.1, 50.0, 50.0],
    ])

    mean_val = np.mean(depth_roi)
    median_val = np.median(depth_roi)
    center = depth_roi[1:4, 1:4]
    center_median = np.median(center)
    min_val = np.min(depth_roi)

    print("해설:")
    print(f"  방법 1 (전체 평균): {mean_val:.2f}m")
    print(f"    → 50m(배경) 값들이 평균을 크게 끌어올림!")
    print(f"    → 실제 물체 깊이(~10m)와 큰 차이")
    print()
    print(f"  방법 2 (전체 median): {median_val:.2f}m")
    print(f"    → 전체 25개 값 중 중앙값")
    print(f"    → 배경이 절반 미만이면 괜찮지만 불안정")
    print()
    print(f"  방법 3 (중심 3x3 median): {center_median:.2f}m")
    print(f"    → 가장 정확! 물체가 중심에 있으므로")
    print(f"    → 중심 영역만 사용하면 배경 영향 최소화")
    print()
    print(f"  방법 4 (최솟값): {min_val:.2f}m")
    print(f"    → 가장 가까운 부분이지만 노이즈에 민감")
    print()
    print("  결론:")
    print("  - 평균은 배경(이상치)에 매우 취약")
    print("  - Median은 이상치에 강건 (중앙값 특성)")
    print("  - 중심 영역만 사용하면 배경 비율 감소")
    print("  - 권장: 중심 20% 영역의 median")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: person (5.16m)")
    print("━" * 36 + "\n")

    fx, fy, cx, cy = 500.0, 500.0, 320.0, 240.0

    detections = [
        {'class': 'car',    'center': (200, 250), 'depth': 15.0},
        {'class': 'person', 'center': (400, 300), 'depth': 5.0},
        {'class': 'car',    'center': (550, 220), 'depth': 30.0},
    ]

    print("해설:")
    results = []
    for i, det in enumerate(detections):
        u, v = det['center']
        Z = det['depth']
        X = (u - cx) * Z / fx
        Y = (v - cy) * Z / fy
        dist = np.sqrt(X**2 + Y**2 + Z**2)
        results.append(dist)

        print(f"\n  물체 {i+1} ({det['class']}):")
        print(f"    X = ({u} - {cx}) * {Z} / {fx} = {X:.2f}m")
        print(f"    Y = ({v} - {cy}) * {Z} / {fy} = {Y:.2f}m")
        print(f"    Z = {Z:.2f}m")
        print(f"    거리 = sqrt({X:.2f}^2 + {Y:.2f}^2 + {Z:.2f}^2) = {dist:.2f}m")

    nearest_idx = np.argmin(results)
    print(f"\n  가장 가까운 물체: {detections[nearest_idx]['class']} ({results[nearest_idx]:.2f}m)")
    print()
    print("  주의:")
    print("  - 'depth'(Z값)만으로는 정확한 거리를 알 수 없음")
    print("  - X, Y 성분도 고려한 유클리드 거리가 실제 거리")
    print("  - 하지만 대부분의 경우 Z >> X, Y이므로 Z만으로도 근사 가능")
    print("  - 이 예제에서도 Z=5m인 person이 가장 가까움")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 11 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
