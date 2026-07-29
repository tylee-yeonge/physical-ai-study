"""
Phase 4.5 Week 3 - LoRA 학습 환경 기초 퀴즈
"""


def problem1_memory_floor():
    print("\n" + "=" * 50)
    print("문제 1: 12GB 로 LoRA 가 안 되는 주된 이유")
    print("=" * 50 + "\n")
    print("질문: OpenVLA 7B LoRA 를 RTX 4070 12GB 에서 못 돌리는 주된 원인은?\n")
    print("보기:")
    print("  A) 어댑터의 옵티마이저 상태가 12GB 를 넘는다")
    print("  B) frozen base 가중치(bf16)만으로 약 15GB 라 배치를 줄여도 남는다")
    print("  C) 활성값이 배치 1 에서도 12GB 를 넘는다")
    print("  D) LoRA 는 구조상 단일 GPU 학습을 지원하지 않는다")


def problem2_grad_accum():
    print("\n" + "=" * 50)
    print("문제 2: grad accumulation 의 역할")
    print("=" * 50 + "\n")
    print("질문: batch size 를 낮추고 grad accumulation 을 올리면 무엇을 얻는가?\n")
    print("보기:")
    print("  A) 메모리를 낮추면서 유효 배치를 유지한다. 대가는 시간이다")
    print("  B) 메모리와 시간을 동시에 줄인다")
    print("  C) 유효 배치를 줄여 학습을 안정화한다")
    print("  D) base 가중치의 크기를 줄인다")


def problem3_max_steps():
    print("\n" + "=" * 50)
    print("문제 3: max_steps 를 반드시 지정하는 이유")
    print("=" * 50 + "\n")
    print("질문: 학습 스크립트의 max_steps 를 지정하지 않으면?\n")
    print("보기:")
    print("  A) 데이터를 1 epoch 만 돌고 자동 종료된다")
    print("  B) 기본값이 사전학습급 규모라 예산 안에 끝나지 않는다")
    print("  C) 오류로 즉시 중단된다")
    print("  D) loss 가 수렴하면 자동으로 멈춘다")


def problem4_stats_file():
    print("\n" + "=" * 50)
    print("문제 4: 회수 목록에서 빠뜨리면 안 되는 것")
    print("=" * 50 + "\n")
    print("질문: 학습 산출물 중 이것이 없으면 fine-tuned 모델을 추론에 쓸 수 없다.")
    print("      무엇인가?\n")
    print("보기:")
    print("  A) 학습 로그")
    print("  B) processor 설정")
    print("  C) 데이터셋 통계 파일")
    print("  D) VRAM 기록 csv")


def problem5_idle_billing():
    print("\n" + "=" * 50)
    print("문제 5: Community Cloud 사용 시 주의")
    print("=" * 50 + "\n")
    print("질문: 인스턴스 회수와 유휴 과금에 대한 대응으로 옳은 조합은?\n")
    print("보기:")
    print("  A) 체크포인트를 컨테이너 내부에 저장하고 pod 를 계속 켜 둔다")
    print("  B) 체크포인트를 network volume 에 주기 저장하고 작업 후 pod 를 멈춘다")
    print("  C) 체크포인트를 로컬로 매 스텝 전송하고 pod 를 계속 켜 둔다")
    print("  D) 회수는 드물므로 체크포인트 없이 한 번에 돌린다")


if __name__ == "__main__":
    print("=" * 50)
    problem1_memory_floor()
    problem2_grad_accum()
    problem3_max_steps()
    problem4_stats_file()
    problem5_idle_billing()
    print("=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
