"""
Phase 6 Week 8 - BEV 개념 이해 중급 퀴즈 풀이
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 풀이: BEV 그리드 설계")
    print("━" * 36 + "\n")

    # 커버 범위
    forward = 50   # 전방 50m
    backward = 10  # 후방 10m
    left = 25      # 좌 25m
    right = 25     # 우 25m
    cell_size = 0.5  # 셀 크기 0.5m

    # 1) 그리드 크기
    y_range = forward + backward  # 60m
    x_range = left + right        # 50m
    H = int(y_range / cell_size)  # 120 셀
    W = int(x_range / cell_size)  # 100 셀

    print(f"  1) BEV 그리드 크기:")
    print(f"     Y 범위: {forward} + {backward} = {y_range}m")
    print(f"     X 범위: {left} + {right} = {x_range}m")
    print(f"     H = {y_range} / {cell_size} = {H}")
    print(f"     W = {x_range} / {cell_size} = {W}")
    print(f"     → BEV 그리드: {H} x {W}")
    print()

    # 2) 총 셀 수
    total_cells = H * W
    print(f"  2) 총 셀 수: {H} x {W} = {total_cells:,}")
    print()

    # 3) 메모리 계산
    C = 256  # Feature dimension
    memory_bytes = total_cells * C * 4  # FP32 = 4 bytes
    memory_mb = memory_bytes / (1024 * 1024)
    print(f"  3) BEV Feature 메모리 (FP32):")
    print(f"     {H} x {W} x {C} x 4 bytes")
    print(f"     = {total_cells:,} x {C} x 4")
    print(f"     = {memory_bytes:,} bytes")
    print(f"     = {memory_mb:.1f} MB")
    print()

    # 4) 셀 크기 0.25m로 줄이면
    cell_small = 0.25
    H_small = int(y_range / cell_small)
    W_small = int(x_range / cell_small)
    total_small = H_small * W_small
    ratio = total_small / total_cells

    print(f"  4) 셀 크기 {cell_small}m로 줄이면:")
    print(f"     H = {H_small}, W = {W_small}")
    print(f"     총 셀 수: {total_small:,}")
    print(f"     메모리 증가: {total_small:,} / {total_cells:,} = {ratio:.0f}배")
    print(f"     메모리: {memory_mb * ratio:.1f} MB")
    print(f"     → 해상도 2배 → 셀 수 4배 → 메모리 4배!")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 풀이: IPM 좌표 변환")
    print("━" * 36 + "\n")

    fx, fy = 721.5, 721.5
    cx, cy = 609.6, 172.9
    cam_height = 1.65
    img_w, img_h = 1242, 375

    ground_points = [
        {"name": "A", "X": 0.0, "Z": 10.0},
        {"name": "B", "X": 3.0, "Z": 20.0},
        {"name": "C", "X": -5.0, "Z": 40.0},
    ]

    print("  1) 카메라 좌표계 Y값:")
    print("     카메라는 지면에서 1.65m 위에 있음")
    print("     지면 위의 점은 카메라 아래 → Y = cam_height = 1.65m")
    print("     (카메라 좌표계에서 Y축은 아래 방향)")
    print()

    print("  2) 이미지 좌표 투영:")
    for pt in ground_points:
        X = pt['X']
        Y = cam_height  # 지면 점의 카메라 Y 좌표
        Z = pt['Z']

        u = fx * X / Z + cx
        v = fy * Y / Z + cy

        in_image = (0 <= u < img_w) and (0 <= v < img_h)
        status = "이미지 내" if in_image else "이미지 밖!"

        print(f"     점 {pt['name']} (X={X}, Y={Y}, Z={Z}):")
        print(f"       u = {fx}*{X}/{Z} + {cx} = {u:.1f}")
        print(f"       v = {fy}*{Y}/{Z} + {cy} = {v:.1f}")
        print(f"       → ({u:.1f}, {v:.1f}) [{status}]")
        print()

    print("  3) 점 C의 이미지 좌표:")
    X_c, Z_c = -5.0, 40.0
    u_c = fx * X_c / Z_c + cx
    v_c = fy * cam_height / Z_c + cy
    in_c = (0 <= u_c < img_w) and (0 <= v_c < img_h)
    print(f"     u = {u_c:.1f}, v = {v_c:.1f}")
    print(f"     이미지 범위: 0~{img_w}, 0~{img_h}")
    print(f"     → {'이미지 내에 있음' if in_c else '이미지 밖에 있음!'}")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 풀이: Depth 분포 → BEV")
    print("━" * 36 + "\n")

    print("  1) 각 픽셀의 BEV 영향:")
    print()
    print("     픽셀 A (가까운 차량, 명확한 depth):")
    print("       → Feature가 10m 부근 한 곳에 집중")
    print("       → BEV에서 해당 위치에 강한 신호")
    print("       → 3D Detection에 유리")
    print()
    print("     픽셀 B (먼 차량, 불확실한 depth):")
    print("       → Feature가 26~38m 넓은 범위에 분산")
    print("       → BEV에서 여러 셀에 약한 신호")
    print("       → 검출 정확도 저하")
    print()
    print("     픽셀 C (하늘, depth 없음):")
    print("       → Feature가 모든 depth에 균일하게 분산")
    print("       → BEV 전체에 노이즈로 작용")
    print()

    print("  2) 하늘 픽셀의 영향:")
    print("     → 의미 없는 Feature가 BEV 전체에 퍼짐")
    print("     → SNR (Signal-to-Noise Ratio) 저하")
    print("     → 해결: 하늘 영역 마스킹, sky segmentation")
    print()

    print("  3) Depth 확신도와 BEV:")
    print("     확신도 높음 (peaked) → Feature 집중 → BEV에서 명확한 신호")
    print("     확신도 낮음 (flat)   → Feature 분산 → BEV에서 약한 신호")
    print("     → Depth 예측의 '첨도(sharpness)'가 BEV 품질을 결정!")
    print()

    print("  4) Depth GT supervision이 중요한 이유:")
    print("     → Depth 분포가 정확해야 Feature가 올바른 위치에 배치")
    print("     → LiDAR 등으로 Depth GT를 제공하면 학습 수렴 빠름")
    print("     → Depth supervision 없으면 BEV Feature 품질 저하")
    print("     → BEVDet 등은 explicit depth supervision 사용")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 8 Quiz Medium - 풀이")
    print("━" * 40)

    problem1_solution()
    problem2_solution()
    problem3_solution()

    print("\n" + "━" * 40)
