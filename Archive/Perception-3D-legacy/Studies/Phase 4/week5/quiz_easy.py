"""
Quiz Easy - Week 5: MMDetection3D 실습 (Section 6.2)
4문제 - 기본 개념 확인
"""


def problem1_openmim_role():
    print("\n" + "━" * 28)
    print("문제 1: openmim의 역할")
    print("━" * 28 + "\n")

    print("Q: MMDetection3D 환경 세팅에서 openmim의 역할은 무엇인가요?")
    print("   다음 중 올바른 설명을 모두 고르세요.\n")
    print("   a) OpenMMLab 패키지 간 버전 호환성을 자동으로 관리해주는 도구")
    print("   b) PyTorch와 CUDA 버전을 자동으로 설치해주는 도구")
    print("   c) mim install mmcv-full 명령으로 호환 가능한 mmcv를 설치")
    print("   d) 모델 학습 속도를 2배 향상시키는 가속 도구")
    print()
    print("   답: _____\n")


def problem2_kitti_ap3d_difficulty():
    print("\n" + "━" * 28)
    print("문제 2: KITTI AP3D 난이도")
    print("━" * 28 + "\n")

    print("Q: KITTI AP3D 평가에서 Easy / Moderate / Hard 난이도의")
    print("   구분 기준을 각각 간단히 설명하세요.\n")
    print("   Easy:     _____")
    print("   Moderate: _____")
    print("   Hard:     _____")
    print()
    print("   그리고 논문에서 주로 보고하는 기준 난이도는 무엇인가요?")
    print("   답: _____\n")


def problem3_iou_threshold():
    print("\n" + "━" * 28)
    print("문제 3: IoU Threshold 차이")
    print("━" * 28 + "\n")

    print("Q: KITTI에서 Car의 IoU threshold는 0.7이고,")
    print("   Pedestrian의 IoU threshold는 0.5입니다.")
    print("   왜 Car에 더 높은 기준을 적용하나요?\n")
    print("   a) Car가 Pedestrian보다 속도가 빨라서")
    print("   b) Car는 크기가 커서 같은 오차에도 3D IoU가 더 높게 나오기 때문")
    print("   c) Car 데이터가 더 많아서")
    print("   d) Pedestrian은 검출이 더 어려워서 기준을 낮춰야 하므로")
    print()
    print("   답: _____\n")


def problem4_config_system():
    print("\n" + "━" * 28)
    print("문제 4: Config 시스템 이해")
    print("━" * 28 + "\n")

    print("Q: MMDetection3D의 Config 파일에서 _base_ 키워드의 역할을")
    print("   설명하고, 아래 예시에서 최종 학습률(lr)은 얼마인지 답하세요.\n")
    print("   # _base_ config (base_config.py)")
    print("   optimizer = dict(type='SGD', lr=0.01, momentum=0.9)")
    print()
    print("   # 하위 config (my_config.py)")
    print("   _base_ = ['base_config.py']")
    print("   optimizer = dict(lr=0.001)")
    print()
    print("   _base_의 역할: _____")
    print("   최종 lr 값:   _____\n")


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 5 Quiz - Easy (MMDetection3D 실습)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_openmim_role()
    problem2_kitti_ap3d_difficulty()
    problem3_iou_threshold()
    problem4_config_system()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
