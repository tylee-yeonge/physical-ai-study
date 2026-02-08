"""
Solutions - Easy Quiz (Week 5: MMDetection3D 실습)
"""


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 5 Quiz - Easy 정답")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    print("Q1. 정답: a), c)")
    print("    openmim은 OpenMMLab 패키지 간의 버전 호환성을 자동으로")
    print("    관리해주는 패키지 매니저입니다.")
    print("    - mim install mmcv-full 명령으로 현재 PyTorch/CUDA와")
    print("      호환되는 mmcv 버전을 자동으로 찾아 설치합니다.")
    print("    - b)는 틀림: PyTorch는 별도로 직접 설치해야 합니다.")
    print("    - d)는 틀림: openmim은 패키지 관리 도구이지 가속 도구가 아닙니다.\n")

    print("Q2. 정답:")
    print("    Easy:     크고, 완전히 보이는 (가려지지 않은) 객체")
    print("    Moderate: 중간 크기, 부분적으로 가려진 객체")
    print("    Hard:     작고, 많이 가려지거나 잘린 객체")
    print("    주요 보고 기준: Moderate")
    print("    - 대부분의 3D Detection 논문에서 Moderate AP3D를")
    print("      주요 벤치마크 지표로 사용합니다.\n")

    print("Q3. 정답: b)")
    print("    Car는 크기가 크기 때문에(약 4.5m x 1.8m x 1.5m)")
    print("    같은 위치 오차에도 3D IoU가 상대적으로 높게 나옵니다.")
    print("    Pedestrian은 크기가 작아서(약 0.6m x 0.6m x 1.7m)")
    print("    0.5m 위치 오차만으로도 3D IoU가 크게 떨어집니다.")
    print("    공정한 평가를 위해 작은 객체에 더 낮은 threshold를 적용합니다.\n")

    print("Q4. 정답:")
    print("    _base_ 역할: 기본 설정을 상속받는 키워드입니다.")
    print("    base_config.py의 모든 설정을 가져온 후,")
    print("    하위 config에서 dict()로 특정 값만 오버라이드할 수 있습니다.")
    print("    최종 lr 값: 0.001")
    print("    - base_config에서 lr=0.01이 설정되지만,")
    print("      my_config에서 optimizer = dict(lr=0.001)로 오버라이드합니다.")
    print("    - type='SGD'와 momentum=0.9는 base에서 상속됩니다.")

    print("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
