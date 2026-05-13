"""
Solutions - Easy Quiz (Week 7: nuScenes 데이터셋)
"""


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 7 Quiz - Easy 정답")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    print("Q1. nuScenes 핵심 특징:")
    print("    카메라 수:    6대")
    print("    장면 수:      1000개")
    print("    클래스 수:    23개 (벤치마크 평가는 10개)")
    print("    커버리지:     360도")
    print("    Key Frame:   2Hz (초당 2프레임)\n")

    print("Q2. 정답: a), c), d)")
    print("    a) 맞음: KITTI는 전방 단안 카메라 1대,")
    print("       nuScenes는 6대로 360도 커버")
    print("    b) 틀림: KITTI가 3개 클래스, nuScenes가 23개 클래스")
    print("       (순서가 반대)")
    print("    c) 맞음: nuScenes 어노테이션에는 객체의 속도")
    print("       (vx, vy) 정보가 포함됨")
    print("    d) 맞음: KITTI는 AP3D (3D IoU 기반),")
    print("       nuScenes는 NDS (종합 점수)")
    print("    e) 틀림: Mini 데이터셋은 약 10GB,")
    print("       Full 데이터셋이 약 300GB\n")

    print("Q3. 데이터 구조 관계:")
    print("    scene        -> sample (첫 Key Frame)")
    print("    sample       -> sample_data (센서 데이터)")
    print("    sample       -> sample_annotation (3D bbox)")
    print("    sample_annotation -> instance (같은 객체 추적)")
    print("    설명:")
    print("    - scene은 하나의 주행 장면 (20초)")
    print("    - sample은 Key Frame (2Hz로 어노테이션된 시점)")
    print("    - sample_data는 각 센서의 실제 파일(이미지, LiDAR)")
    print("    - sample_annotation은 3D bounding box 정보")
    print("    - instance는 프레임 간 같은 객체를 연결하는 ID\n")

    print("Q4. NDS 구성 요소:")
    print("    1. mAP (mean Average Precision) - 검출 정확도")
    print("    2. ATE (Average Translation Error) - 위치 오차")
    print("    3. ASE (Average Scale Error) - 크기 오차")
    print("    4. AOE (Average Orientation Error) - 방향 오차")
    print("    5. AVE (Average Velocity Error) - 속도 오차")
    print("    6. AAE (Average Attribute Error) - 속성 오차")
    print()
    print("    NDS가 AP3D보다 종합적인 이유:")
    print("    AP3D는 3D IoU 기반 검출 정확도만 평가하지만,")
    print("    NDS는 검출 정확도 외에 위치/크기/방향/속도/속성의")
    print("    품질까지 종합적으로 평가합니다. 자율주행에서는")
    print("    정확한 속도 추정과 방향 추정이 안전과 직결되므로")
    print("    이런 종합 지표가 더 실용적입니다.")

    print("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
