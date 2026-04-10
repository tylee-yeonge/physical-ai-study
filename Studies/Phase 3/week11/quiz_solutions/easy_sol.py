"""
Phase 5 Week 11 - Detection + Depth 융합 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: A) X = (u - cx) * Z / fx")
    print("━" * 28 + "\n")

    print("해설:")
    print("  핀홀 카메라 모델의 역투영(Unprojection) 공식:")
    print()
    print("  투영 (3D → 2D):")
    print("    u = fx * X / Z + cx")
    print()
    print("  역투영 (2D → 3D):")
    print("    X = (u - cx) * Z / fx")
    print("    Y = (v - cy) * Z / fy")
    print("    Z = depth_map[v, u]")
    print()
    print("  유도 과정:")
    print("    u = fx * X / Z + cx")
    print("    u - cx = fx * X / Z")
    print("    X = (u - cx) * Z / fx")
    print()
    print("  cx를 빼는 이유:")
    print("  → cx는 주점(이미지 중심)의 오프셋")
    print("  → 주점을 기준으로 상대적 위치를 계산해야 함")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: C) 중심 20% 영역의 median")
    print("━" * 28 + "\n")

    print("해설:")
    print("  A) 전체 평균: 배경 깊이(원거리)가 포함되어 왜곡됨")
    print("  B) 중심점: 한 픽셀에 의존 → 노이즈에 매우 취약")
    print("  C) 중심 20% median: 이상치에 강건, 물체 깊이를 잘 반영")
    print("  D) 최댓값: 배경(원거리) 값을 반환할 가능성 높음")
    print()
    print("  Median이 좋은 이유:")
    print("  - BBox에는 물체 + 배경이 혼합")
    print("  - 평균은 배경(먼 거리)에 끌려감")
    print("  - Median은 중앙값이므로 이상치(배경)를 자연스럽게 무시")
    print("  - 중심 영역만 사용하면 배경 비율이 더 줄어듦")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: A) ROS: (10, -2, 1)")
    print("━" * 28 + "\n")

    print("해설:")
    print("  좌표계 변환:")
    print()
    print("  카메라:  X=오른쪽, Y=아래, Z=전방")
    print("  ROS:     X=전방,  Y=왼쪽, Z=위")
    print()
    print("  변환 규칙:")
    print("    ROS_X = Camera_Z =  10  (전방)")
    print("    ROS_Y = -Camera_X = -2  (왼쪽)")
    print("    ROS_Z = -Camera_Y =  1  (위)")
    print()
    print("  카메라 (2, -1, 10) → ROS (10, -2, 1)")
    print()
    print("  주의: 이 변환을 빠뜨리면 RViz2에서 물체가")
    print("  완전히 잘못된 위치에 표시됩니다!")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: B) YOLO → Depth → BBox Depth 추출 → 역투영")
    print("━" * 28 + "\n")

    print("해설:")
    print("  올바른 파이프라인:")
    print()
    print("  1. YOLO 검출: 이미지 → 2D BBox + 클래스")
    print("  2. Depth 추정: 이미지 → 깊이 맵 (1, 2는 병렬 가능)")
    print("  3. BBox 내 Depth 추출: BBox 영역에서 깊이 값 추출")
    print("  4. 역투영: (u, v, Z) → (X, Y, Z)")
    print()
    print("  핵심: YOLO와 Depth는 독립적이므로 병렬 실행 가능!")
    print("  하지만 BBox Depth 추출은 둘 다 완료된 후에만 가능")


def problem5_solution():
    print("\n" + "━" * 28)
    print("문제 5 정답: B) 약 55ms (max)")
    print("━" * 28 + "\n")

    print("해설:")
    print("  병렬 실행의 원리:")
    print()
    print("  순차 실행:")
    print("    YOLO ██████ (30ms)")
    print("                     Depth ████████████ (55ms)")
    print("    총: 30 + 55 = 85ms")
    print()
    print("  병렬 실행:")
    print("    YOLO  ██████ (30ms)")
    print("    Depth ████████████ (55ms)")
    print("    총: max(30, 55) = 55ms")
    print()
    print("  병렬 실행 시 총 시간 = 가장 느린 작업의 시간")
    print("  → 순차 85ms → 병렬 55ms (약 35% 개선)")
    print("  → FPS: 12 → 18")
    print()
    print("  구현 방법:")
    print("  - CUDA 스트림 분리")
    print("  - Python threading/asyncio")
    print("  - ROS2 멀티스레드 실행기")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 11 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    print("\n" + "━" * 33)
