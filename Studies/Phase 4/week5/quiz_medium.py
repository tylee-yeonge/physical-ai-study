"""
Quiz Medium - Week 5: MMDetection3D 실습 (Section 6.2)
3문제 - 심화 이해
"""


def problem1_fcos3d_multitask():
    print("\n" + "━" * 28)
    print("문제 1: FCOS3D Multi-task Learning")
    print("━" * 28 + "\n")

    print("Q: FCOS3D는 하나의 네트워크에서 여러 Task를 동시에 예측합니다.")
    print("   아래 표를 완성하고, 각 Task의 Loss 가중치가 성능에 미치는")
    print("   영향을 설명하세요.\n")
    print("   ┌─────────────────┬────────────────┬──────────────┐")
    print("   │ Task            │ 예측값          │ Loss 종류    │")
    print("   ├─────────────────┼────────────────┼──────────────┤")
    print("   │ Classification  │ _____          │ _____        │")
    print("   │ 2D Offset       │ _____          │ _____        │")
    print("   │ Depth           │ _____          │ _____        │")
    print("   │ Size            │ _____          │ _____        │")
    print("   │ Rotation        │ _____          │ _____        │")
    print("   └─────────────────┴────────────────┴──────────────┘")
    print()
    print("   Loss 가중치의 영향: _____\n")


def problem2_ap3d_vs_ap2d():
    print("\n" + "━" * 28)
    print("문제 2: AP3D vs AP2D 성능 격차 분석")
    print("━" * 28 + "\n")

    print("Q: 한 Monocular 3D Detection 모델의 평가 결과가 다음과 같습니다.\n")
    print("   Car (Moderate 기준):")
    print("     AP2D (IoU 0.7): 85.2%")
    print("     AP3D (IoU 0.7): 13.8%\n")
    print("   AP2D와 AP3D의 격차가 이렇게 큰 이유를 분석하고,")
    print("   AP3D를 높이기 위해 가장 먼저 개선해야 할 부분은 무엇인지")
    print("   설명하세요.\n")
    print("   격차 원인: _____")
    print("   개선 방향: _____\n")


def problem3_config_debug():
    print("\n" + "━" * 28)
    print("문제 3: Config 디버깅")
    print("━" * 28 + "\n")

    print("Q: 아래 MMDetection3D config에서 문제점을 3가지 이상 찾고")
    print("   수정하세요.\n")
    print("   _base_ = [")
    print("       '../_base_/datasets/nuscenes-mono3d.py',  # 문제 1?")
    print("       '../_base_/models/fcos3d.py',")
    print("   ]")
    print()
    print("   model = dict(")
    print("       backbone=dict(type='ResNet', depth=101),")
    print("       bbox_head=dict(num_classes=10),           # 문제 2?")
    print("   )")
    print()
    print("   data = dict(")
    print("       samples_per_gpu=16,                       # 문제 3?")
    print("       workers_per_gpu=8,")
    print("   )")
    print()
    print("   optimizer = dict(type='SGD', lr=0.1)          # 문제 4?")
    print()
    print("   (힌트: KITTI 데이터셋으로 학습할 때를 기준으로 생각하세요)")
    print()
    print("   문제점과 수정: _____\n")


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 5 Quiz - Medium (MMDetection3D 실습)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_fcos3d_multitask()
    problem2_ap3d_vs_ap2d()
    problem3_config_debug()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
