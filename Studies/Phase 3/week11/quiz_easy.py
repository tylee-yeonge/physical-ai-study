"""
Phase 5 Week 11 - Detection + Depth 융합 기초 퀴즈
"""


def problem1_unprojection_formula():
    print("\n" + "━" * 28)
    print("문제 1: 역투영 공식")
    print("━" * 28 + "\n")

    print("질문: 픽셀 좌표 (u, v)와 깊이 Z로부터 3D 좌표 X를 구하는 공식은?\n")
    print("  fx: x축 초점 거리 (픽셀)")
    print("  cx: x축 주점 (principal point)\n")

    print("보기:")
    print("  A) X = (u - cx) * Z / fx")
    print("  B) X = u * Z / fx")
    print("  C) X = (u - fx) * Z / cx")
    print("  D) X = fx * Z / (u - cx)")


def problem2_depth_extraction():
    print("\n" + "━" * 28)
    print("문제 2: BBox Depth 추출 전략")
    print("━" * 28 + "\n")

    print("질문: BBox에서 물체의 깊이를 추출할 때 가장 권장되는 방법은?\n")

    print("보기:")
    print("  A) BBox 전체 영역의 평균 깊이")
    print("  B) BBox 중심점의 깊이 값")
    print("  C) BBox 중심 20% 영역의 중앙값(median)")
    print("  D) BBox 내 최댓값")


def problem3_coordinate_transform():
    print("\n" + "━" * 28)
    print("문제 3: 좌표계 변환")
    print("━" * 28 + "\n")

    print("질문: 카메라 좌표계에서 (X=2, Y=-1, Z=10)인 점이")
    print("      ROS 좌표계(REP 103)에서의 좌표는?\n")
    print("  카메라: X(오른쪽), Y(아래), Z(전방)")
    print("  ROS:    X(전방), Y(왼쪽), Z(위)\n")

    print("보기:")
    print("  A) ROS: (10, -2, 1)")
    print("  B) ROS: (2, -1, 10)")
    print("  C) ROS: (10, 2, -1)")
    print("  D) ROS: (-1, 2, 10)")


def problem4_pipeline_order():
    print("\n" + "━" * 28)
    print("문제 4: 파이프라인 순서")
    print("━" * 28 + "\n")

    print("질문: Detection + Depth 융합 파이프라인의 올바른 순서는?\n")

    print("보기:")
    print("  A) 역투영 → YOLO 검출 → Depth 추출 → Depth 추정")
    print("  B) YOLO 검출 → Depth 추정 → BBox 내 Depth 추출 → 역투영")
    print("  C) Depth 추정 → 역투영 → YOLO 검출 → 결과 합치기")
    print("  D) YOLO 검출 → 역투영 → Depth 추정 → 결과 합치기")


def problem5_parallel_processing():
    print("\n" + "━" * 28)
    print("문제 5: 병렬 처리 효과")
    print("━" * 28 + "\n")

    print("질문: YOLO(30ms)와 Depth(55ms)를 GPU에서 병렬 실행하면")
    print("      총 처리 시간은 약 얼마인가?\n")

    print("보기:")
    print("  A) 약 85ms (합산)")
    print("  B) 약 55ms (max)")
    print("  C) 약 42.5ms (평균)")
    print("  D) 약 15ms (절반)")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 11 Quiz - Easy")
    print("━" * 33)
    problem1_unprojection_formula()
    problem2_depth_extraction()
    problem3_coordinate_transform()
    problem4_pipeline_order()
    problem5_parallel_processing()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
