"""
Phase 4 Week 4 - OpenVLA 1회독 + Architecture 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) Llama 2 7B")
    print("=" * 50 + "\n")
    print("해설:")
    print("  RT-2  : PaLI-X 5B / 55B (Google, closed)")
    print("  OpenVLA: Llama 2 7B (Meta, open)")
    print()
    print("  Llama 2 선택 이유:")
    print("    1. open-source (commercial use 가능)")
    print("    2. 7B 크기로 4-bit 양자화 시 단일 GPU (8GB+) 에 fit")
    print("    3. 다른 LLM 생태계 (HuggingFace, vLLM 등) 와 호환")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: A) DINOv2 (spatial) + SigLIP (semantic)")
    print("=" * 50 + "\n")
    print("해설:")
    print("  DINOv2  : self-supervised, spatial / geometric 정보 강함")
    print("           (객체가 '어디' 있는가)")
    print("  SigLIP  : image-text contrastive, semantic 정보 강함")
    print("           (객체가 '무엇' 인가)")
    print()
    print("  Robot manipulation 은 둘 다 필요:")
    print("    'pick up the green can' = '집어라' (action) + 'green can' (semantic)")
    print("                            + 위치 (spatial)")
    print()
    print("  Single encoder 보다 hybrid 가 일관되게 우수 (논문 ablation 결과).")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) OpenX-Embodiment (970K episodes, 22 embodiments)")
    print("=" * 50 + "\n")
    print("해설:")
    print("  RT-1 dataset (130K, 1 embodiment) <- RT-2 가 사용")
    print("  OpenX-Embodiment (970K, 22 embodiments) <- OpenVLA 가 사용")
    print()
    print("  OpenX-Embodiment 의 의미:")
    print("    1. 22 embodiments 다양성 -> generalization 강화")
    print("    2. 970K episodes -> 데이터 양 7배")
    print("    3. 60 개 dataset 통합 (RT-1 + Bridge + Berkeley Cable 등)")
    print()
    print("  자작 6DOF 팔 (Phase 7) 에 LoRA fine-tune 시:")
    print("    - 22 embodiments 중 유사한 것 (예: Franka, Bridge) 의 표현 활용")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: A) 메모리 / 시간 부담 적어 빠른 적응 가능")
    print("=" * 50 + "\n")
    print("해설:")
    print("  LoRA (Low-Rank Adaptation):")
    print("    - 원 모델 가중치 W 를 고정")
    print("    - 작은 추가 matrix BA (rank r) 만 학습")
    print("    - 학습 파라미터 수 ~ 1% (약 65M for 7B)")
    print()
    print("  OpenVLA + LoRA 의 의미:")
    print("    - 새 robot 의 50~500 demonstrations 만으로 적응")
    print("    - GPU 메모리 8GB 정도면 학습 가능")
    print("    - 학습 시간 1~10 시간")
    print()
    print("  Phase 7 산출물 #4 (자작 6DOF 팔) 에서 핵심:")
    print("    teleop 으로 ~100 demonstrations 수집 -> LoRA fine-tune")


if __name__ == "__main__":
    print("=" * 50)
    print("  Phase 4 Week 4 Quiz - Easy 정답")
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "=" * 50)
