"""
Phase 4.5 Week 2 - 학습 포맷 변환 기초 퀴즈
"""


def problem1_why_rlds():
    print("\n" + "=" * 50)
    print("문제 1: 검증된 데이터 경로를 쓰는 이유")
    print("=" * 50 + "\n")
    print("질문: 데이터로더를 자작하지 않고 upstream 이 쓰는 형식을 그대로 따르는")
    print("      가장 중요한 이유는?\n")
    print("보기:")
    print("  A) 자작 로더는 속도가 느려 학습 시간이 늘어난다")
    print("  B) 결과가 안 나올 때 원인 후보에 '내 로더' 가 추가되는 것을 막는다")
    print("  C) 라이선스상 자체 로더를 쓸 수 없다")
    print("  D) 자작 로더는 GPU 메모리를 더 쓴다")


def problem2_three_files():
    print("\n" + "=" * 50)
    print("문제 2: 등록 3파일의 역할")
    print("=" * 50 + "\n")
    print("질문: '학습에 어떤 데이터셋 조합을 쓸 것인가' 를 지정하는 파일은?\n")
    print("보기:")
    print("  A) configs.py (데이터셋 설정)")
    print("  B) transforms.py (표준화 변환)")
    print("  C) mixtures.py (mixture 정의)")
    print("  D) finetune.py (학습 스크립트)")


def problem3_label_unit():
    print("\n" + "=" * 50)
    print("문제 3: 라벨을 저장하는 단위")
    print("=" * 50 + "\n")
    print("질문: RLDS 에 저장하는 action 라벨의 값은 어떤 상태여야 하는가?\n")
    print("보기:")
    print("  A) [-1, 1] 로 미리 정규화한 값")
    print("  B) 원시 물리 단위 (미터·라디안) 값")
    print("  C) 0-255 로 스케일한 정수 값")
    print("  D) 사전학습 통계로 역정규화한 값")


def problem4_rotation_fixed():
    print("\n" + "=" * 50)
    print("문제 4: 회전 표현의 확정")
    print("=" * 50 + "\n")
    print("질문: 7차원 EEF 위치 인코딩을 쓸 때 회전 3차원의 표현은?\n")
    print("보기:")
    print("  A) 쿼터니언의 앞 3개 성분")
    print("  B) 축-각 (회전 벡터)")
    print("  C) 오일러각 (RPY)")
    print("  D) R6 표현의 앞 3개 성분")


def problem5_stats_cache():
    print("\n" + "=" * 50)
    print("문제 5: 통계 캐시")
    print("=" * 50 + "\n")
    print("질문: 라벨을 고쳐 데이터셋을 재생성했다. 이때 특히 주의할 것은?\n")
    print("보기:")
    print("  A) 학습률을 낮춰야 한다")
    print("  B) 옛 통계 캐시가 남아 있으면 그것으로 정규화된다")
    print("  C) episode 수가 줄어들 수 있다")
    print("  D) 이미지 인코딩 형식이 바뀐다")


if __name__ == "__main__":
    print("=" * 50)
    problem1_why_rlds()
    problem2_three_files()
    problem3_label_unit()
    problem4_rotation_fixed()
    problem5_stats_cache()
    print("=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
