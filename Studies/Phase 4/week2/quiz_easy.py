"""
Phase 6 Week 2 - 좌표계 이해 기초 퀴즈
"""


def problem1_camera_coordinate():
    print("\n" + "━" * 28)
    print("문제 1: Camera 좌표계")
    print("━" * 28 + "\n")

    print("질문: KITTI Camera 좌표계에서 '전방'은 어느 축의 양의 방향인가?\n")

    print("보기:")
    print("  A) +X (오른쪽)")
    print("  B) +Y (아래쪽)")
    print("  C) +Z (전방)")
    print("  D) -Z (후방)")


def problem2_kitti_label_order():
    print("\n" + "━" * 28)
    print("문제 2: KITTI 레이블 순서")
    print("━" * 28 + "\n")

    print("질문: KITTI 3D 레이블에서 크기 정보의 순서는?\n")
    print("  레이블 예시:")
    print("  Car 0.0 0 -1.56 587.01 173.33 614.12 200.12 1.65 1.67 3.64 ...\n")

    print("보기:")
    print("  A) [l, w, h] (length, width, height)")
    print("  B) [h, w, l] (height, width, length)")
    print("  C) [w, h, l] (width, height, length)")
    print("  D) [h, l, w] (height, length, width)")


def problem3_projection_division():
    print("\n" + "━" * 28)
    print("문제 3: 투영 시 Z 나누기")
    print("━" * 28 + "\n")

    print("질문: 3D 점을 P2 행렬로 이미지에 투영할 때,")
    print("      결과 벡터를 z 성분으로 나누는 이유는?\n")

    print("보기:")
    print("  A) 좌표계를 정규화하기 위해")
    print("  B) 원근 투영(perspective projection)을 적용하기 위해")
    print("  C) 단위를 미터에서 픽셀로 변환하기 위해")
    print("  D) 카메라 왜곡을 보정하기 위해")


def problem4_lidar_vs_camera():
    print("\n" + "━" * 28)
    print("문제 4: LiDAR vs Camera 좌표계")
    print("━" * 28 + "\n")

    print("질문: LiDAR 좌표계에서 '위쪽'은 +Z 방향이고,")
    print("      Camera 좌표계에서 '위쪽'은 -Y 방향이다.")
    print("      두 좌표계 사이의 변환에 사용하는 KITTI 행렬은?\n")

    print("보기:")
    print("  A) P2 (투영 행렬)")
    print("  B) R0_rect (스테레오 정류 행렬)")
    print("  C) Tr_velo_to_cam (LiDAR-Camera 변환 행렬)")
    print("  D) K (내부 파라미터 행렬)")


if __name__ == "__main__":
    print("━" * 33)
    print("  Phase 6 Week 2 Quiz - Easy")
    print("━" * 33)
    problem1_camera_coordinate()
    problem2_kitti_label_order()
    problem3_projection_division()
    problem4_lidar_vs_camera()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
