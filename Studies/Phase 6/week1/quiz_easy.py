"""
Phase 6 Week 1 - 3D Detection 개념 기초 퀴즈
"""


def problem1_3d_bbox_params():
    print("\n" + "━" * 28)
    print("문제 1: 3D BBox 파라미터")
    print("━" * 28 + "\n")

    print("질문: 3D Bounding Box를 표현하는 7개 파라미터 [x, y, z, l, w, h, theta] 중")
    print("      2D Bounding Box [x, y, w, h]에는 없는 파라미터는?\n")

    print("보기:")
    print("  A) x, y (위치)")
    print("  B) w, h (크기)")
    print("  C) z, l, theta (깊이, 길이, 회전)")
    print("  D) x, y, z (3D 위치)")


def problem2_depth_ambiguity():
    print("\n" + "━" * 28)
    print("문제 2: Depth Ambiguity")
    print("━" * 28 + "\n")

    print("질문: 단안 카메라에서 'Depth Ambiguity(깊이 모호성)'란 무엇인가?\n")

    print("보기:")
    print("  A) 카메라 렌즈에 의한 왜곡으로 깊이가 왜곡되는 현상")
    print("  B) 하나의 픽셀이 무한한 3D 점에 대응하여 깊이를 특정할 수 없는 현상")
    print("  C) LiDAR 없이는 절대로 깊이를 추정할 수 없다는 원리")
    print("  D) 카메라 해상도가 낮아서 깊이 정보가 손실되는 현상")


def problem3_detection_methods():
    print("\n" + "━" * 28)
    print("문제 3: 3D Detection 방법론")
    print("━" * 28 + "\n")

    print("질문: 다음 중 Camera 기반 3D Detection의 장점으로 올바른 것은?\n")

    print("보기:")
    print("  A) LiDAR보다 정확한 거리 측정이 가능하다")
    print("  B) 센서 비용이 저렴하고, 색상/텍스처 정보를 활용할 수 있다")
    print("  C) 어두운 환경에서도 LiDAR보다 성능이 좋다")
    print("  D) 날씨(비, 안개)의 영향을 전혀 받지 않는다")


def problem4_3d_iou():
    print("\n" + "━" * 28)
    print("문제 4: 3D IoU 평가 기준")
    print("━" * 28 + "\n")

    print("질문: KITTI 3D Detection 평가에서 Car 클래스의 IoU threshold는 0.7이고")
    print("      Pedestrian은 0.5인 이유로 가장 적절한 것은?\n")

    print("보기:")
    print("  A) Car가 Pedestrian보다 검출이 더 쉽기 때문")
    print("  B) Pedestrian은 크기가 작아서 같은 위치 오차에도 IoU가 더 크게 떨어지기 때문")
    print("  C) Car 데이터가 Pedestrian 데이터보다 더 많기 때문")
    print("  D) KITTI 데이터셋 제작자의 임의적인 결정이기 때문")


if __name__ == "__main__":
    print("━" * 33)
    print("  Phase 6 Week 1 Quiz - Easy")
    print("━" * 33)
    problem1_3d_bbox_params()
    problem2_depth_ambiguity()
    problem3_detection_methods()
    problem4_3d_iou()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
