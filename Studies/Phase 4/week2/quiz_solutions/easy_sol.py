"""
Phase 6 Week 2 - 좌표계 이해 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: C) +Z (전방)")
    print("━" * 28 + "\n")

    print("해설:")
    print("  KITTI Camera 좌표계:")
    print("    +X: 오른쪽")
    print("    +Y: 아래쪽 (주의!)")
    print("    +Z: 전방 (카메라가 바라보는 방향)")
    print()
    print("  이미지 좌표계와의 관계:")
    print("    이미지 u축 = Camera X축 (오른쪽)")
    print("    이미지 v축 = Camera Y축 (아래쪽)")
    print("    깊이 방향 = Camera Z축 (전방)")
    print()
    print("  비교:")
    print("    Camera: 전방=+Z, 위=−Y")
    print("    LiDAR:  전방=+X, 위=+Z")
    print("    → 축 방향이 다르므로 변환 행렬이 필요!")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: B) [h, w, l]")
    print("━" * 28 + "\n")

    print("해설:")
    print("  KITTI 레이블 포맷:")
    print("  Car 0.0 0 -1.56 587.01 173.33 614.12 200.12 1.65 1.67 3.64 ...")
    print("                                                 │    │    │")
    print("                                                 h    w    l")
    print()
    print("  KITTI는 [height, width, length] 순서를 사용합니다.")
    print("  이것은 KITTI 고유의 규약이며, 다른 데이터셋과 다를 수 있습니다:")
    print()
    print("    KITTI:    [h, w, l]  (height, width, length)")
    print("    nuScenes: [w, l, h]  (width, length, height)")
    print("    Waymo:    [l, w, h]  (length, width, height)")
    print()
    print("  데이터셋을 다룰 때 반드시 순서를 확인해야 합니다!")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: B) 원근 투영 적용")
    print("━" * 28 + "\n")

    print("해설:")
    print("  P2 @ [X, Y, Z, 1]^T = [u*Z, v*Z, Z]^T")
    print()
    print("  Z로 나누면: (u, v) = (u*Z/Z, v*Z/Z)")
    print()
    print("  이것이 원근 투영(perspective projection)입니다:")
    print("    u = fx * X/Z + cx")
    print("    v = fy * Y/Z + cy")
    print()
    print("  Z로 나누는 효과:")
    print("    - 가까운 물체(Z 작음): X/Z가 커짐 -> 이미지에서 크게 보임")
    print("    - 먼 물체(Z 큼): X/Z가 작아짐 -> 이미지에서 작게 보임")
    print()
    print("  이것이 '원근감(perspective)'을 만드는 원리입니다.")
    print("  핀홀 카메라 모델(Phase 2)의 핵심 개념과 동일합니다.")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: C) Tr_velo_to_cam")
    print("━" * 28 + "\n")

    print("해설:")
    print("  KITTI 캘리브레이션 행렬:")
    print()
    print("  P2: Camera 좌표 -> 이미지 좌표 (투영)")
    print("    3x4 행렬, 내부 파라미터 + 스테레오 보정 포함")
    print()
    print("  R0_rect: 스테레오 정류(rectification) 회전")
    print("    카메라 이미지를 스테레오 정합에 맞게 보정")
    print()
    print("  Tr_velo_to_cam: LiDAR -> Camera 좌표 변환")
    print("    3x4 행렬 (회전 3x3 + 이동 3x1)")
    print("    이것이 LiDAR와 Camera 좌표계를 연결합니다.")
    print()
    print("  전체 파이프라인:")
    print("    LiDAR 좌표 -> Tr_velo_to_cam -> R0_rect -> Camera 좌표 -> P2 -> 이미지")


if __name__ == "__main__":
    print("━" * 33)
    print("  Phase 6 Week 2 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "━" * 33)
