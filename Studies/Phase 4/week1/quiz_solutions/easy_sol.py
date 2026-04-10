"""
Phase 6 Week 1 - 3D Detection 개념 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: C) z, l, theta")
    print("━" * 28 + "\n")

    print("해설:")
    print("  2D BBox: [x, y, w, h] → 이미지 평면 위의 위치와 크기")
    print("  3D BBox: [x, y, z, l, w, h, theta]")
    print()
    print("  2D에 없고 3D에만 있는 파라미터:")
    print("    - z (깊이): 카메라로부터의 거리")
    print("    - l (길이): 깊이 방향(z축)의 크기")
    print("    - theta (yaw): 수평면에서의 회전 각도")
    print()
    print("  x, y는 의미가 다릅니다:")
    print("    - 2D: 이미지 좌표 (픽셀)")
    print("    - 3D: 3D 공간 좌표 (미터)")
    print()
    print("  핵심: 3D Detection은 2D에 '깊이(z)', '깊이 방향 크기(l)',")
    print("        '회전(theta)'을 추가로 예측해야 합니다.")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: B) 하나의 픽셀이 무한한 3D 점에 대응")
    print("━" * 28 + "\n")

    print("해설:")
    print("  핀홀 카메라 모델에서:")
    print("    u = fx * X/Z + cx")
    print("    v = fy * Y/Z + cy")
    print()
    print("  역투영 시:")
    print("    X = (u - cx) * Z / fx")
    print("    Y = (v - cy) * Z / fy")
    print()
    print("  Z를 모르면 X, Y도 결정할 수 없습니다.")
    print("  하나의 픽셀 (u, v)에 대해 Z값에 따라")
    print("  무한히 많은 (X, Y, Z) 조합이 가능합니다.")
    print()
    print("  이것이 Camera 기반 3D Detection이 어려운 핵심 이유이며,")
    print("  네트워크는 시각적 단서(크기, 원근, 그림자 등)로")
    print("  깊이를 '추정'해야 합니다.")
    print()
    print("  A) 렌즈 왜곡은 Depth Ambiguity와 다른 문제입니다.")
    print("  C) 딥러닝으로 깊이를 '추정'할 수 있으므로 틀렸습니다.")
    print("  D) 해상도와 Depth Ambiguity는 관련이 없습니다.")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: B) 센서 비용 저렴, 색상/텍스처 활용")
    print("━" * 28 + "\n")

    print("해설:")
    print("  Camera 기반 3D Detection의 장점:")
    print("    1. 센서 비용: 카메라 $50~500 vs LiDAR $10,000~75,000")
    print("    2. 풍부한 정보: 색상, 텍스처, 의미 정보 활용 가능")
    print("    3. 해상도: 수백만 픽셀 (LiDAR는 수만~수십만 포인트)")
    print("    4. Tesla 등 Pure Vision 접근의 산업 트렌드")
    print()
    print("  틀린 보기 해설:")
    print("  A) LiDAR가 직접 거리를 측정하므로 더 정확합니다.")
    print("  C) 어두운 환경에서는 카메라가 불리합니다.")
    print("  D) 비, 안개는 카메라에도 영향을 줍니다.")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: B) Pedestrian은 크기가 작아서 IoU가 더 크게 떨어짐")
    print("━" * 28 + "\n")

    print("해설:")
    print("  3D IoU는 두 직육면체의 겹치는 부피 비율입니다.")
    print()
    print("  예: 위치가 0.5m 어긋났을 때:")
    print("    Car (4.5 x 1.8 x 1.5m): IoU에 미치는 영향이 작음")
    print("    Pedestrian (0.6 x 0.6 x 1.7m): IoU가 급격히 떨어짐")
    print()
    print("  구체적 예시:")
    print("    Car 부피: 4.5 * 1.8 * 1.5 = 12.15 m^3")
    print("    Pedestrian 부피: 0.6 * 0.6 * 1.7 = 0.612 m^3")
    print()
    print("    0.5m 오차가 있으면:")
    print("    - Car: 겹침 부피가 크게 유지됨 (IoU ~0.5+ 가능)")
    print("    - Pedestrian: 거의 안 겹침 (IoU ~0.1 이하)")
    print()
    print("  따라서 공정한 평가를 위해:")
    print("    Car: IoU >= 0.7 (큰 물체 → 높은 기준)")
    print("    Pedestrian: IoU >= 0.5 (작은 물체 → 낮은 기준)")


if __name__ == "__main__":
    print("━" * 33)
    print("  Phase 6 Week 1 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "━" * 33)
