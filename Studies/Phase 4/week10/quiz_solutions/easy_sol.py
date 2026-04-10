"""
Phase 6 Week 10 - BEVFormer 실습 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: D) mFPS")
    print("━" * 28 + "\n")

    print("해설:")
    print("  NDS = 1/10 * (5*mAP + (1-mATE) + (1-mASE) + (1-mAOE)")
    print("                       + (1-mAVE) + (1-mAAE))")
    print()
    print("  NDS에 포함되는 6가지 메트릭:")
    print("    1. mAP  - mean Average Precision (검출 정확도)")
    print("    2. mATE - mean Average Translation Error (위치 오차)")
    print("    3. mASE - mean Average Scale Error (크기 오차)")
    print("    4. mAOE - mean Average Orientation Error (방향 오차)")
    print("    5. mAVE - mean Average Velocity Error (속도 오차)")
    print("    6. mAAE - mean Average Attribute Error (속성 오차)")
    print()
    print("  mFPS(추론 속도)는 NDS에 포함되지 않습니다.")
    print("  NDS는 검출의 정확도와 품질만을 평가하며,")
    print("  추론 속도는 별도로 보고합니다.")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: C) BEV 중심점 거리")
    print("━" * 28 + "\n")

    print("해설:")
    print("  nuScenes mAP의 매칭 기준:")
    print("  예측 bbox와 GT bbox의 BEV 중심점 간 거리")
    print()
    print("  거리 threshold: [0.5m, 1.0m, 2.0m, 4.0m]")
    print("  각 threshold에서 AP를 구한 후 평균")
    print()
    print("  KITTI와의 차이:")
    print("    KITTI: 3D IoU ≥ 0.7 (Car) / 0.5 (Ped, Cyclist)")
    print("    nuScenes: BEV 중심 거리 ≤ threshold")
    print()
    print("  BEV 중심 거리를 사용하는 이유:")
    print("  1. 360도 카메라 환경에서 먼 거리 객체의 3D IoU가 불안정")
    print("  2. 여러 threshold로 세밀한 성능 평가 가능")
    print("  3. 구현이 단순하고 직관적")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: B) 200 x 200 x 256")
    print("━" * 28 + "\n")

    print("해설:")
    print("  BEVFormer-Base의 BEV Feature Map:")
    print("  - 높이(H): 200 셀")
    print("  - 너비(W): 200 셀")
    print("  - 채널(C): 256 차원")
    print()
    print("  물리적 의미:")
    print("  - 200 x 200 셀, 각 셀 0.5m")
    print("  - 커버 범위: 100m x 100m")
    print("  - 256차원 벡터가 각 셀의 의미 정보를 담고 있음")
    print()
    print("  BEV Feature Map의 크기는 성능과 속도의 트레이드오프:")
    print(f"  - 50x50:  2.0m/셀, 빠르지만 해상도 낮음")
    print(f"  - 100x100: 1.0m/셀, 중간")
    print(f"  - 200x200: 0.5m/셀, 정확하지만 느림")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: B) Car")
    print("━" * 28 + "\n")

    print("해설:")
    print("  Car가 가장 높은 AP를 보이는 이유:")
    print()
    print("  1. 크기: Car는 비교적 큰 객체 (4~5m)")
    print("     → 이미지에서 많은 pixel을 차지하여 Feature 풍부")
    print()
    print("  2. 데이터 양: nuScenes에서 Car가 가장 많은 학습 데이터")
    print("     → 모델이 Car 패턴을 가장 잘 학습")
    print()
    print("  3. 일관된 형태: 차량은 비교적 일정한 형태와 크기")
    print("     → 크기/방향 예측이 상대적으로 쉬움")
    print()
    print("  클래스별 AP (BEVFormer-Base 근사):")
    print("    Car:          AP ≈ 0.636")
    print("    Barrier:      AP ≈ 0.530")
    print("    Traffic Cone: AP ≈ 0.505")
    print("    Pedestrian:   AP ≈ 0.420")
    print("    Bicycle:      AP ≈ 0.226 (가장 낮음)")
    print()
    print("  Bicycle이 낮은 이유:")
    print("  - 크기가 매우 작음 → Feature 부족")
    print("  - nuScenes에서 출현 빈도가 낮음 → 학습 데이터 부족")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 10 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "━" * 33)
