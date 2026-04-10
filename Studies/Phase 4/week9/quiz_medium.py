"""
Phase 6 Week 9 - BEVFormer 이해 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_bev_query_parameters():
    """
    문제 1: BEV Query 파라미터 수 계산

    BEVFormer에서 BEV Queries와 Position Embedding의
    총 학습 가능한 파라미터 수를 계산하시오.

    - BEV Query: (bev_h * bev_w) x embed_dim
    - BEV Position: (bev_h * bev_w) x embed_dim
    - bev_h = 200, bev_w = 200, embed_dim = 256

    TODO: total_params를 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: BEV Query 파라미터 수 계산")
    print("━" * 36 + "\n")

    bev_h = 200
    bev_w = 200
    embed_dim = 256

    print(f"  BEV Grid: {bev_h} x {bev_w}")
    print(f"  Embedding 차원: {embed_dim}")
    print(f"  BEV Queries: ({bev_h}*{bev_w}) x {embed_dim}")
    print(f"  BEV Position: ({bev_h}*{bev_w}) x {embed_dim}")
    print()

    # TODO: 직접 계산하시오
    total_params = 0  # 여기를 채우시오

    # 실제 계산
    actual_query = bev_h * bev_w * embed_dim
    actual_pos = bev_h * bev_w * embed_dim
    actual_total = actual_query + actual_pos

    print(f"  계산한 파라미터 수: {total_params:,}")
    print(f"  실제 파라미터 수: {actual_total:,}")

    if total_params == actual_total:
        print("\n  정답!")
    else:
        print("\n  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_reference_point_projection():
    """
    문제 2: Reference Point 투영 계산

    BEV 위치 (x=5.0, y=20.0) (월드 좌표, 단위: m)에서
    높이 z=0.0의 3D 점을 정면 카메라에 투영하시오.

    카메라 파라미터:
    - K = [[800, 0, 800], [0, 800, 450], [0, 0, 1]]
    - 외부 파라미터: 단위행렬 (카메라 = 월드 좌표, 높이 1.5m)
      R = I, t = [0, -1.5, 0]

    투영 공식: [u, v, 1]^T = K * [x, y-1.5, z]^T / z_cam

    TODO: projected_u, projected_v를 계산하시오.
    """
    print("\n" + "━" * 36)
    print("문제 2: Reference Point 투영 계산")
    print("━" * 36 + "\n")

    # 3D 점 (BEV x=5, y=20, z=0)
    # 카메라 좌표계에서: x_cam=5, y_cam=0-1.5=-1.5, z_cam=20
    point_3d = np.array([5.0, 0.0 - 1.5, 20.0])

    K = np.array([
        [800, 0, 800],
        [0, 800, 450],
        [0, 0, 1]
    ], dtype=float)

    print(f"  3D 점 (카메라 좌표): {point_3d}")
    print(f"  K 행렬:")
    print(f"    fx=800, fy=800, cx=800, cy=450")
    print()
    print(f"  투영 공식:")
    print(f"    u = fx * x_cam / z_cam + cx")
    print(f"    v = fy * y_cam / z_cam + cy")
    print()

    # TODO: 직접 계산하시오
    projected_u = 0.0  # 여기를 채우시오
    projected_v = 0.0  # 여기를 채우시오

    # 실제 계산
    projected = K @ point_3d
    actual_u = projected[0] / projected[2]
    actual_v = projected[1] / projected[2]

    print(f"  계산한 결과: u={projected_u:.1f}, v={projected_v:.1f}")
    print(f"  실제 결과:   u={actual_u:.1f}, v={actual_v:.1f}")

    if abs(projected_u - actual_u) < 1.0 and abs(projected_v - actual_v) < 1.0:
        print("\n  정답!")
    else:
        print("\n  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")
        print(f"  힌트: u = 800 * 5.0 / 20.0 + 800 = ?")
        print(f"  힌트: v = 800 * (-1.5) / 20.0 + 450 = ?")


def problem3_ego_motion_warp():
    """
    문제 3: Ego-motion 보상 좌표 계산

    이전 프레임에서 BEV 위치 (x=3.0, y=10.0)에 객체가 있었다.
    ego 차량이 전방으로 2.0m 이동했다면 (dx=0, dy=2.0, dtheta=0),
    현재 프레임의 BEV 좌표계에서 이 객체의 보정된 위치는?

    규칙:
    - 차량이 전방 2m 이동 → 정적 객체는 상대적으로 2m 뒤로 이동
    - x_new = x_old - dx
    - y_new = y_old - dy

    TODO: corrected_x, corrected_y를 계산하시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: Ego-motion 보상 좌표 계산")
    print("━" * 36 + "\n")

    # 이전 프레임의 객체 위치
    prev_x, prev_y = 3.0, 10.0
    # ego-motion
    dx, dy, dtheta = 0.0, 2.0, 0.0

    print(f"  이전 위치: ({prev_x}, {prev_y})")
    print(f"  Ego-motion: dx={dx}, dy={dy}, dtheta={dtheta}")
    print()
    print(f"  차량이 전방 {dy}m 이동 → 정적 객체는 상대적으로 {dy}m 뒤로")
    print()

    # TODO: 직접 계산하시오
    corrected_x = 0.0  # 여기를 채우시오
    corrected_y = 0.0  # 여기를 채우시오

    # 실제 계산
    actual_x = prev_x - dx
    actual_y = prev_y - dy

    print(f"  계산한 보정 위치: ({corrected_x}, {corrected_y})")
    print(f"  실제 보정 위치:   ({actual_x}, {actual_y})")

    if abs(corrected_x - actual_x) < 0.1 and abs(corrected_y - actual_y) < 0.1:
        print("\n  정답!")
    else:
        print("\n  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 9 Quiz - Medium")
    print("━" * 40)
    problem1_bev_query_parameters()
    problem2_reference_point_projection()
    problem3_ego_motion_warp()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
