"""
Quiz Easy - Week 7: nuScenes 데이터셋 (Section 6.3)
4문제 - 기본 개념 확인
"""


def problem1_nuscenes_features():
    print("\n" + "━" * 28)
    print("문제 1: nuScenes 핵심 특징")
    print("━" * 28 + "\n")

    print("Q: nuScenes 데이터셋의 핵심 특징을 빈칸에 채우세요.\n")
    print("   카메라 수:    _____대")
    print("   장면 수:      _____개")
    print("   클래스 수:    _____개")
    print("   커버리지:     _____도")
    print("   Key Frame:   _____Hz\n")


def problem2_kitti_vs_nuscenes():
    print("\n" + "━" * 28)
    print("문제 2: KITTI vs nuScenes 비교")
    print("━" * 28 + "\n")

    print("Q: KITTI와 nuScenes의 차이점을 설명하세요.")
    print("   다음 중 올바른 설명을 모두 고르세요.\n")
    print("   a) KITTI는 전방 1대 카메라, nuScenes는 6대 카메라를 사용한다")
    print("   b) KITTI는 23개 클래스, nuScenes는 3개 클래스이다")
    print("   c) nuScenes는 객체 속도(velocity) 정보도 포함한다")
    print("   d) KITTI의 평가 지표는 AP3D, nuScenes는 NDS이다")
    print("   e) nuScenes Mini 데이터셋은 약 300GB이다")
    print()
    print("   답: _____\n")


def problem3_data_structure():
    print("\n" + "━" * 28)
    print("문제 3: 데이터 구조 이해")
    print("━" * 28 + "\n")

    print("Q: nuScenes의 관계형 데이터 구조에서")
    print("   다음 관계를 올바르게 연결하세요.\n")
    print("   ┌───────────────────┐     ┌──────────────────────┐")
    print("   │ scene             │ ──→ │ _____ (첫 Key Frame) │")
    print("   │ sample            │ ──→ │ _____ (센서 데이터)   │")
    print("   │ sample            │ ──→ │ _____ (3D bbox)      │")
    print("   │ sample_annotation │ ──→ │ _____ (같은 객체 추적)│")
    print("   └───────────────────┘     └──────────────────────┘\n")
    print("   보기: instance, sample_data, sample, sample_annotation")
    print()
    print("   답: _____\n")


def problem4_nds_metric():
    print("\n" + "━" * 28)
    print("문제 4: NDS 평가 지표")
    print("━" * 28 + "\n")

    print("Q: nuScenes Detection Score (NDS)는 어떤 메트릭들의 조합인가요?")
    print("   NDS를 구성하는 6가지 요소를 나열하세요.\n")
    print("   1. _____ (검출 정확도)")
    print("   2. _____ (위치 오차)")
    print("   3. _____ (크기 오차)")
    print("   4. _____ (방향 오차)")
    print("   5. _____ (속도 오차)")
    print("   6. _____ (속성 오차)")
    print()
    print("   NDS가 AP3D보다 종합적인 이유: _____\n")


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 7 Quiz - Easy (nuScenes 데이터셋)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_nuscenes_features()
    problem2_kitti_vs_nuscenes()
    problem3_data_structure()
    problem4_nds_metric()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
