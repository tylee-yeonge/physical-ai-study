"""
Quiz Easy - Week 7: Monocular Depth 이론 (Section 5.3)
4문제 - 기본 개념 확인
"""


def problem1_why_monocular_depth():
    print("\n" + "━" * 28)
    print("문제 1: 왜 Monocular Depth인가?")
    print("━" * 28 + "\n")

    print("AMR(자율 이동 로봇)에서 단안 카메라 깊이 추정을 사용하는")
    print("이유를 LiDAR, 스테레오 카메라와 비교하여 2가지 이상 설명하세요.")
    print()
    print("  답: _____")
    print()


def problem2_learning_methods():
    print("\n" + "━" * 28)
    print("문제 2: 학습 방법론")
    print("━" * 28 + "\n")

    print("다음 중 올바른 설명을 모두 고르세요.")
    print()
    print("  a) Supervised 학습에는 LiDAR 등으로 측정한 GT 깊이맵이 필요하다")
    print("  b) Self-supervised 학습은 연속 프레임의 기하학적 관계를 활용한다")
    print("  c) Zero-shot 모델은 테스트 시 반드시 Fine-tuning이 필요하다")
    print("  d) Depth Anything은 라벨 없는 62M 이미지를 활용하여 일반화 성능을 높였다")
    print()
    print("  답: _____")
    print()


def problem3_relative_vs_metric():
    print("\n" + "━" * 28)
    print("문제 3: Relative vs Metric Depth")
    print("━" * 28 + "\n")

    print("다음 시나리오에서 Relative Depth와 Metric Depth 중")
    print("어떤 것이 필요한지 선택하고 이유를 설명하세요.")
    print()
    print("  시나리오 A: '앞에 장애물이 있으니 왼쪽으로 돌자'")
    print("  시나리오 B: '장애물이 정확히 2m 앞에 있으니 정지하자'")
    print("  시나리오 C: 'SLAM에서 3D 지도를 실제 스케일로 만들자'")
    print()
    print("  답: A=_____, B=_____, C=_____")
    print()


def problem4_model_selection():
    print("\n" + "━" * 28)
    print("문제 4: 모델 선택")
    print("━" * 28 + "\n")

    print("Jetson Orin Nano (8GB)에서 실시간 깊이 추정을 하려고 합니다.")
    print("다음 중 가장 적합한 모델은?")
    print()
    print("  a) DPT-Large (343M 파라미터)")
    print("  b) Depth Anything ViT-L (335M 파라미터)")
    print("  c) Depth Anything ViT-S (24.8M 파라미터)")
    print("  d) MiDaS v3.1 BEiT (345M 파라미터)")
    print()
    print("  답: _____")
    print()


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 7 Quiz - Easy (Monocular Depth 이론)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_why_monocular_depth()
    problem2_learning_methods()
    problem3_relative_vs_metric()
    problem4_model_selection()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
