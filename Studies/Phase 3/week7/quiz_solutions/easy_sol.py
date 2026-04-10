"""
Solutions - Easy Quiz (Week 7: Monocular Depth 이론)
"""


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 7 Quiz - Easy 정답")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    print("Q1. 단안 깊이 추정을 사용하는 이유:")
    print("    1. 비용: LiDAR는 $200~$10,000+, 카메라는 $30")
    print("       이미 장착된 카메라를 활용하면 추가 비용 $0")
    print("    2. 경량/저전력: 카메라 30g/0.5W vs LiDAR 1kg+/15W")
    print("       소형 AMR에 적합")
    print("    3. 밀집 깊이맵: LiDAR는 포인트가 희소하지만")
    print("       단안 깊이는 모든 픽셀에 대해 깊이 추정 가능")
    print("    4. 스테레오 대비: 캘리브레이션 불필요, 카메라 1대로 충분\n")

    print("Q2. 정답: a, b, d")
    print("    a) 맞음: Supervised 학습에는 GT 깊이맵이 필요")
    print("    b) 맞음: Self-supervised는 연속 프레임의 기하학적 일관성 활용")
    print("    c) 틀림: Zero-shot은 Fine-tuning 없이 바로 사용 가능")
    print("       (이것이 Zero-shot의 핵심 장점)")
    print("    d) 맞음: Depth Anything은 62M 비라벨 데이터로 일반화 향상\n")

    print("Q3. 정답:")
    print("    A = Relative Depth (상대 깊이)")
    print("      이유: 장애물이 어느 방향에 있는지만 판단하면 됨")
    print("            '왼쪽이 오른쪽보다 비어있다' = 상대적 비교로 충분")
    print()
    print("    B = Metric Depth (절대 깊이)")
    print("      이유: '정확히 2m'라는 절대 거리 판단이 필요")
    print("            상대 깊이로는 실제 미터 거리를 알 수 없음")
    print()
    print("    C = Metric Depth (절대 깊이)")
    print("      이유: SLAM 3D 지도는 실제 스케일이 필요")
    print("            상대 깊이만으로는 스케일이 모호함(Scale Ambiguity)\n")

    print("Q4. 정답: c) Depth Anything ViT-S (24.8M 파라미터)")
    print("    이유:")
    print("    - 파라미터가 가장 적어 Jetson 메모리에 적합 (8GB 내)")
    print("    - TensorRT FP16 변환 시 ~50ms 추론 (15-20 FPS)")
    print("    - DPT-Large(343M), ViT-L(335M)은 메모리 부족/실시간 불가")
    print("    - MiDaS v3.1(345M)도 마찬가지로 너무 큼")
    print("    - ViT-S는 작지만 Zero-shot 성능이 우수 (62M 데이터 학습 효과)")

    print("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
