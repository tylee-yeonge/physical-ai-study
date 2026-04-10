"""
Solutions - Easy Quiz (Week 6: 성능 분석 및 개선)
"""


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 6 Quiz - Easy 정답")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    print("Q1. 오류 유형 4가지:")
    print("    1. Depth 오류: 깊이 추정이 부정확하여 3D IoU가 급감")
    print("       -> AP3D에 가장 큰 영향 (1m 오차만으로 IoU < 0.7)")
    print("    2. Orientation 오류: 회전각(ry) 추정이 부정확")
    print("       -> 큰 각도 오차(30도+) 시 3D IoU 20% 이상 감소")
    print("    3. Occlusion 오류: 가려진 객체를 검출하지 못함")
    print("       -> Recall 감소, 특히 Hard 난이도에서 심각")
    print("    4. Truncation 오류: 이미지 경계에서 잘린 객체 검출 실패")
    print("       -> 근거리(0-10m) 성능 저하의 주요 원인\n")

    print("Q2. 정답: a), c)")
    print("    a) 맞음: Depth 1m 오차는 Car 크기(4.5m) 대비 상당하여")
    print("       3D IoU가 0.7 이하로 떨어질 수 있음")
    print("    b) 틀림: 원거리(40m+)에서는 Depth 오차가 보통 5m 이상")
    print("    c) 맞음: 2D에서는 잘 맞지만 Depth 오류로 3D IoU가 낮으면")
    print("       AP3D는 크게 낮아짐 (AP2D 85% vs AP3D 13% 가능)")
    print("    d) 틀림: 2D bbox가 정확하더라도 Depth가 틀리면")
    print("       3D IoU가 낮아지므로, 2D와 3D 정확도는 독립적이지 않음\n")

    print("Q3. 정답: a), c)")
    print("    a) x값: 좌우 반전이므로 x_new = -x (부호 반전)")
    print("    b) z값: 깊이는 좌우 반전과 무관하므로 변환 불필요")
    print("    c) rotation_y: 좌우 반전 시 ry_new = -ry (부호 반전)")
    print("    d) 크기(l,w,h): 크기는 좌우 반전과 무관하므로 변환 불필요")
    print("    핵심: x와 ry만 부호를 반전해야 합니다!\n")

    print("Q4. 정답:")
    print("    2D NMS: Car A만 유지 (Car B 제거)")
    print("      -> 2D IoU=0.6 > threshold=0.5이므로 낮은 score의 B 제거")
    print("    BEV NMS: Car A, Car B 모두 유지")
    print("      -> BEV(x-z 평면)에서 z=10m과 z=25m은 전혀 겹치지 않음")
    print("      -> BEV IoU=0 < threshold=0.25이므로 두 검출 모두 유지")
    print("    이유: 2D NMS는 이미지 평면만 고려하므로 깊이가 다른 객체를")
    print("    잘못 제거할 수 있지만, BEV NMS는 3D 공간을 고려하므로")
    print("    깊이가 다른 객체를 올바르게 보존합니다.")

    print("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
