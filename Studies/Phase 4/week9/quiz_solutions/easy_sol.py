"""
Phase 6 Week 9 - BEVFormer 이해 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: B) 100m x 100m")
    print("━" * 28 + "\n")

    print("해설:")
    print("  BEV Query Grid: 200 x 200 셀")
    print("  셀 크기: 0.5m x 0.5m")
    print()
    print("  물리적 범위 = 그리드 크기 x 셀 크기")
    print("  = 200 x 0.5m = 100m")
    print("  → 100m x 100m")
    print()
    print("  Ego 차량이 중앙에 위치하므로:")
    print("  x 방향: -50m ~ +50m")
    print("  y 방향: -50m ~ +50m")
    print()
    print("  이는 nuScenes 데이터셋의 검출 범위(50m 반경)와 일치합니다.")
    print("  도로 위 대부분의 객체를 포함하기에 충분한 범위입니다.")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: D) 24개")
    print("━" * 28 + "\n")

    print("해설:")
    print("  BEV Query 하나에 대해:")
    print("  - 높이 수: 4개 (예: z = -5m, -3m, -1m, 1m)")
    print("  - 카메라 수: 6대")
    print()
    print("  총 Reference Points = 높이 수 x 카메라 수")
    print("  = 4 x 6 = 24개")
    print()
    print("  단, 이 중 실제로 이미지 범위 내에 있는 점만 사용합니다.")
    print("  예를 들어, 뒤쪽 BEV Query의 Reference Point는")
    print("  FRONT 카메라에 투영되지 않을 수 있습니다.")
    print()
    print("  전체 BEV에 대해서는:")
    print(f"  200 x 200 x 4 x 6 = {200*200*4*6:,}개의 Reference Points")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: B) 좌표 어긋남")
    print("━" * 28 + "\n")

    print("해설:")
    print("  Ego-motion 보상이 필요한 이유:")
    print()
    print("  차량이 이동하면, 동일한 BEV 위치 (i, j)가")
    print("  이전 프레임과 현재 프레임에서 서로 다른 실세계 좌표를 가리킵니다.")
    print()
    print("  예시:")
    print("  - t-1 시점: BEV (100, 100) = 월드 좌표 (0, 0)")
    print("  - 차량이 전방 2m 이동")
    print("  - t 시점: BEV (100, 100) = 월드 좌표 (0, 2)")
    print()
    print("  보상 없이 합치면:")
    print("  → 정적 객체의 위치가 이중으로 보임")
    print("  → 속도 추정이 부정확")
    print()
    print("  Ego-motion으로 이전 BEV를 현재 좌표계에 맞춰 정렬해야")
    print("  올바른 시간 정보 융합이 가능합니다.")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: B) Depth 예측 불필요 + Temporal")
    print("━" * 28 + "\n")

    print("해설:")
    print("  BEVFormer vs BEVDet 핵심 차이:")
    print()
    print("  BEVDet (Lift-Splat-Shoot):")
    print("  - 각 픽셀에 대해 명시적으로 Depth를 예측해야 함")
    print("  - Depth 예측 정확도가 전체 성능에 직접 영향")
    print("  - Temporal 정보 활용이 기본 구조에 포함되지 않음")
    print()
    print("  BEVFormer:")
    print("  - Attention 메커니즘으로 암묵적(implicit) 3D 정보 학습")
    print("  - 카메라 파라미터와 Reference Points를 통한 기하학적 투영")
    print("  - Temporal Self-Attention으로 이전 프레임 정보 자연스럽게 활용")
    print("  - 가려진 객체 복구, 속도 추정에 큰 도움")
    print()
    print("  비교 결과 (nuScenes val):")
    print("  - BEVDet:    NDS 0.392")
    print("  - BEVFormer: NDS 0.517 (+ Temporal)")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 9 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "━" * 33)
