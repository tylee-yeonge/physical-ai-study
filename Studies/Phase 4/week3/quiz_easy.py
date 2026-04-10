"""
Phase 6 Week 3 - KITTI 데이터셋 기초 퀴즈
"""


def problem1_label_format():
    print("\n" + "━" * 28)
    print("문제 1: KITTI 레이블 포맷")
    print("━" * 28 + "\n")

    print("질문: 다음 KITTI 레이블에서 객체의 '깊이(전방 거리)'는 몇 m인가?\n")
    print("  Car 0.00 0 -1.56 587.01 173.33 614.12 200.12 1.65 1.67 3.64 -0.65 1.71 46.70 -1.59\n")

    print("보기:")
    print("  A) -0.65 m (x 좌표)")
    print("  B) 1.71 m (y 좌표)")
    print("  C) 46.70 m (z 좌표)")
    print("  D) -1.59 (ry 값)")


def problem2_directory_structure():
    print("\n" + "━" * 28)
    print("문제 2: KITTI 디렉토리 구조")
    print("━" * 28 + "\n")

    print("질문: KITTI에서 3D bbox 레이블 파일이 위치하는 디렉토리는?\n")

    print("보기:")
    print("  A) training/image_2/")
    print("  B) training/calib/")
    print("  C) training/label_2/")
    print("  D) training/velodyne/")


def problem3_dontcare():
    print("\n" + "━" * 28)
    print("문제 3: DontCare 레이블")
    print("━" * 28 + "\n")

    print("질문: KITTI 레이블에서 'DontCare' 클래스의 역할은?\n")

    print("보기:")
    print("  A) 아직 분류되지 않은 새로운 객체를 표시")
    print("  B) 평가 시 무시할 영역을 표시 (너무 멀거나 심하게 가려진 객체)")
    print("  C) 배경(background)을 명시적으로 표시")
    print("  D) 데이터 수집 시 오류가 발생한 영역")


def problem4_difficulty():
    print("\n" + "━" * 28)
    print("문제 4: KITTI 난이도 기준")
    print("━" * 28 + "\n")

    print("질문: KITTI 3D Detection 벤치마크에서 가장 많이 사용되는")
    print("      표준 난이도 기준은?\n")

    print("보기:")
    print("  A) Easy")
    print("  B) Moderate")
    print("  C) Hard")
    print("  D) All (전체 통합)")


if __name__ == "__main__":
    print("━" * 33)
    print("  Phase 6 Week 3 Quiz - Easy")
    print("━" * 33)
    problem1_label_format()
    problem2_directory_structure()
    problem3_dontcare()
    problem4_difficulty()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
