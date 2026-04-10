"""
Phase 6 Week 3 - KITTI 데이터셋 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: C) 46.70 m (z 좌표)")
    print("━" * 28 + "\n")

    print("해설:")
    print("  KITTI 레이블 필드 순서:")
    print("  Car 0.00 0 -1.56 587.01 173.33 614.12 200.12 1.65 1.67 3.64 -0.65 1.71 46.70 -1.59")
    print("   0    1  2   3    4      5      6      7     8    9    10   11    12   13    14")
    print()
    print("  필드 11: x = -0.65 (오른쪽/왼쪽 위치)")
    print("  필드 12: y = 1.71 (위/아래 위치, 바닥면)")
    print("  필드 13: z = 46.70 (전방 거리 = 깊이)")
    print("  필드 14: ry = -1.59 (yaw 회전)")
    print()
    print("  z 좌표가 Camera 좌표계에서의 '전방 거리'입니다.")
    print("  이 차량은 카메라로부터 전방 46.7m 거리에 있습니다.")
    print()
    print("  주의: KITTI에서 크기는 [h, w, l] 순서 (필드 8, 9, 10)")
    print("        위치는 [x, y, z] 순서 (필드 11, 12, 13)")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: C) training/label_2/")
    print("━" * 28 + "\n")

    print("해설:")
    print("  KITTI 디렉토리 구조:")
    print("    training/image_2/  → 좌측 컬러 카메라 '이미지'")
    print("    training/calib/    → '캘리브레이션' 파일 (P2, R0, Tr)")
    print("    training/label_2/  → '레이블' 파일 (3D bbox 포함)")
    print("    training/velodyne/ → LiDAR '포인트 클라우드'")
    print()
    print("  번호 의미:")
    print("    '2'는 좌측 컬러 카메라를 의미합니다.")
    print("    label_2 = 좌측 컬러 카메라(image_2) 기준의 레이블")
    print("    따라서 3D 좌표는 P2 투영 행렬에 대응합니다.")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: B) 평가 시 무시할 영역")
    print("━" * 28 + "\n")

    print("해설:")
    print("  DontCare 레이블의 역할:")
    print("    - 너무 먼 객체 (작아서 정확한 레이블링 어려움)")
    print("    - 심하게 가려진 객체 (3D 정보 불확실)")
    print("    - 분류가 모호한 객체")
    print()
    print("  평가 시 동작:")
    print("    - DontCare 영역 내 검출 = 무시 (TP도 FP도 아님)")
    print("    - DontCare 영역 밖 잘못된 검출 = FP")
    print()
    print("  코드에서 처리:")
    print("    objects = [o for o in objects if o['class'] != 'DontCare']")
    print()
    print("  DontCare 예시 레이블:")
    print("    DontCare -1 -1 -10 527 174 547 186 -1 -1 -1 -1000 -1000 -1000 -10")
    print("    → 3D 정보가 모두 -1, -1000으로 '의미 없음'을 표시")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: B) Moderate")
    print("━" * 28 + "\n")

    print("해설:")
    print("  KITTI 3D Detection 벤치마크의 표준은 'Moderate'입니다.")
    print()
    print("  이유:")
    print("    Easy: 조건이 너무 좋아서 대부분의 모델이 높은 점수")
    print("           → 모델 간 차이가 크지 않음")
    print("    Hard: 조건이 너무 가혹해서 모든 모델이 낮은 점수")
    print("           → 노이즈가 많고 불안정")
    print("    Moderate: 실제 주행 환경에 가장 가까운 조건")
    print("               → 모델의 실질적 성능을 잘 반영")
    print()
    print("  따라서 대부분의 논문에서:")
    print("    'AP3D (Moderate)'를 주요 성능 지표로 보고합니다.")
    print()
    print("  예: FCOS3D: 12% (Moderate), MonoDETR: 25% (Moderate)")


if __name__ == "__main__":
    print("━" * 33)
    print("  Phase 6 Week 3 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "━" * 33)
