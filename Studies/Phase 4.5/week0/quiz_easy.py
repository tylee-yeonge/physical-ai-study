"""
Phase 4.5 Week 0 - sim 구축 + 하네스 검증 기초 퀴즈
"""


def problem1_render_backend():
    print("\n" + "=" * 50)
    print("문제 1: SAPIEN 의 렌더 백엔드")
    print("=" * 50 + "\n")
    print("질문: ManiSkill 설치 후 렌더가 검은 화면으로 나올 때")
    print("      가장 먼저 점검해야 하는 것은?\n")
    print("보기:")
    print("  A) OpenGL 버전과 GLX 확장")
    print("  B) Vulkan 로더와 nvidia ICD 파일")
    print("  C) X11 DISPLAY 환경변수")
    print("  D) CUDA 버전과 cuDNN")


def problem2_order():
    print("\n" + "=" * 50)
    print("문제 2: 하네스 검증을 측정보다 먼저 하는 이유")
    print("=" * 50 + "\n")
    print("질문: zero-shot 성공률을 재기 전에 하네스를 검증하는 이유는?\n")
    print("보기:")
    print("  A) 검증을 먼저 하면 측정 시간이 줄어든다")
    print("  B) 성공률 0% 가 도메인 갭인지 통합 버그인지 구분하기 위해")
    print("  C) 검증 없이 측정하면 GPU 메모리가 부족해진다")
    print("  D) 논문 재현 규약이 검증을 먼저 요구한다")


def problem3_success_definition():
    print("\n" + "=" * 50)
    print("문제 3: success 판정 정의의 확인 방법")
    print("=" * 50 + "\n")
    print("질문: PickCube 의 success 임계값을 확정하는 올바른 방법은?\n")
    print("보기:")
    print("  A) 공식 문서에 적힌 수치를 그대로 인용한다")
    print("  B) 논문의 실험 절에 적힌 값을 쓴다")
    print("  C) 설치된 버전의 판정 함수 소스코드를 직접 읽는다")
    print("  D) 임계값을 직접 정해 새로 정의한다")


def problem4_seed_list():
    print("\n" + "=" * 50)
    print("문제 4: seed 목록을 기록하는 이유")
    print("=" * 50 + "\n")
    print("질문: baseline 측정에서 사용한 seed 목록을 남겨야 하는 이유는?\n")
    print("보기:")
    print("  A) seed 를 남기면 성공률의 신뢰구간이 좁아진다")
    print("  B) fine-tuned 측정에서 같은 목록을 써야 변인이 모델 하나로 고정된다")
    print("  C) seed 가 없으면 sim 이 초기 배치를 무작위화하지 못한다")
    print("  D) 재현 시 GPU 메모리 사용량을 맞추기 위해")


def problem5_final_metric():
    print("\n" + "=" * 50)
    print("문제 5: 부분 도달률을 함께 재는 이유")
    print("=" * 50 + "\n")
    print("질문: 최종 성공률 외에 reached / grasped / lifted 도 기록하는 이유는?\n")
    print("보기:")
    print("  A) 최종 성공률의 계산 오류를 교차 검증하기 위해")
    print("  B) 최종 성공률이 0 에 붙어도 before/after 신호가 남게 하기 위해")
    print("  C) sim 이 요구하는 표준 보고 항목이기 때문")
    print("  D) 단계별로 재면 episode 수를 줄일 수 있기 때문")


if __name__ == "__main__":
    print("=" * 50)
    problem1_render_backend()
    problem2_order()
    problem3_success_definition()
    problem4_seed_list()
    problem5_final_metric()
    print("=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
