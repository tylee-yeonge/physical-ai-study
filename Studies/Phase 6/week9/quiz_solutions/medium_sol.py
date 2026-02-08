"""
Phase 6 Week 9 - BEVFormer 이해 중급 퀴즈 정답 및 해설
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: BEV Query 파라미터 수")
    print("━" * 36 + "\n")

    bev_h = 200
    bev_w = 200
    embed_dim = 256

    query_params = bev_h * bev_w * embed_dim
    pos_params = bev_h * bev_w * embed_dim
    total = query_params + pos_params

    print(f"  BEV Queries: {bev_h} x {bev_w} x {embed_dim} = {query_params:,}")
    print(f"  BEV Position: {bev_h} x {bev_w} x {embed_dim} = {pos_params:,}")
    print(f"  총합: {query_params:,} + {pos_params:,} = {total:,}")
    print()
    print(f"  정답: {total:,} (약 {total/1e6:.1f}M)")
    print()
    print("  참고:")
    print(f"  - nn.Embedding({bev_h * bev_w}, {embed_dim})의 파라미터")
    print(f"  - {bev_h*bev_w} = {bev_h*bev_w:,}개의 학습 가능한 벡터")
    print(f"  - 각 벡터의 차원: {embed_dim}")
    print()
    print("  BEV Queries는 학습 초기에 랜덤으로 초기화되고,")
    print("  학습을 통해 각 BEV 위치에 적합한 질문(query)을 학습합니다.")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: Reference Point 투영")
    print("━" * 36 + "\n")

    K = np.array([
        [800, 0, 800],
        [0, 800, 450],
        [0, 0, 1]
    ], dtype=float)

    point_cam = np.array([5.0, -1.5, 20.0])

    print("  카메라 좌표계의 3D 점: (5.0, -1.5, 20.0)")
    print("    x_cam = 5.0 (오른쪽)")
    print("    y_cam = -1.5 (위쪽, 카메라 높이 보정)")
    print("    z_cam = 20.0 (전방)")
    print()
    print("  투영 공식:")
    print("    u = fx * x_cam / z_cam + cx")
    print("    v = fy * y_cam / z_cam + cy")
    print()
    print("  계산:")
    u = 800 * 5.0 / 20.0 + 800
    v = 800 * (-1.5) / 20.0 + 450
    print(f"    u = 800 * 5.0 / 20.0 + 800 = {800 * 5.0 / 20.0} + 800 = {u}")
    print(f"    v = 800 * (-1.5) / 20.0 + 450 = {800 * (-1.5) / 20.0} + 450 = {v}")
    print()
    print(f"  정답: u = {u:.1f}, v = {v:.1f}")
    print()

    # 행렬 곱으로 검증
    projected = K @ point_cam
    u_verify = projected[0] / projected[2]
    v_verify = projected[1] / projected[2]
    print(f"  행렬 곱 검증: u = {u_verify:.1f}, v = {v_verify:.1f}")
    print()
    print("  이 점은 이미지 우측 상단에 투영됩니다.")
    print("  (cx=800이 이미지 중앙이므로, u=1000은 오른쪽)")
    print("  (cy=450이 이미지 중앙이므로, v=390은 위쪽)")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: Ego-motion 보상 좌표")
    print("━" * 36 + "\n")

    prev_x, prev_y = 3.0, 10.0
    dx, dy = 0.0, 2.0

    corrected_x = prev_x - dx
    corrected_y = prev_y - dy

    print(f"  이전 위치: ({prev_x}, {prev_y})")
    print(f"  Ego-motion: dx={dx}, dy={dy}")
    print()
    print("  보상 원리:")
    print("  차량이 전방(+y) 2m 이동 → 정적 객체는 상대적으로 2m 뒤(-y)로 이동")
    print()
    print("  계산:")
    print(f"    x_new = x_old - dx = {prev_x} - {dx} = {corrected_x}")
    print(f"    y_new = y_old - dy = {prev_y} - {dy} = {corrected_y}")
    print()
    print(f"  정답: ({corrected_x}, {corrected_y})")
    print()
    print("  직관적 이해:")
    print("  이전 프레임에서 전방 10m에 있던 건물은,")
    print("  차량이 2m 전진하면 전방 8m에 있게 됩니다.")
    print("  이것이 ego-motion 보상의 핵심입니다.")
    print()
    print("  회전이 있는 경우:")
    print("    R = [[cos(theta), -sin(theta)],")
    print("         [sin(theta),  cos(theta)]]")
    print("    [x_new, y_new] = R^T @ [x_old - dx, y_old - dy]")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 9 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
