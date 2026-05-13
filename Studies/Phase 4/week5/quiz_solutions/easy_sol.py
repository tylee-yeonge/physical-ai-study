"""
Phase 4 Week 5 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) 970K episodes, 22 embodiments")
    print("=" * 50 + "\n")
    print("해설:")
    print("  OpenX-Embodiment (Google + DeepMind + 21 institutions, 2023):")
    print("    - 60개 dataset 통합")
    print("    - ~ 970K episodes")
    print("    - 22 embodiments")
    print("  RT-1 dataset (130K, 1 embodiment) 의 약 7배 규모.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: A) observation, action, reward, is_*")
    print("=" * 50 + "\n")
    print("해설:")
    print("  RLDS step 의 표준 필드:")
    print("    - observation : 이미지 / 명령 / state 등")
    print("    - action      : 이 step 에서 취한 행동")
    print("    - reward      : 이 step 의 보상")
    print("    - is_first    : episode 의 첫 step?")
    print("    - is_last     : episode 의 마지막 step?")
    print("    - is_terminal : terminal state (성공 / 실패)?")
    print()
    print("  observation 안에는 image / natural_language_instruction / "
          "state 등이 들어감.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) WidowX 250 (6-DoF, Dynamixel 기반)")
    print("=" * 50 + "\n")
    print("해설:")
    print("  자작 팔 (Dynamixel XM430 6DOF) 와 가장 유사한 특성:")
    print("    - DoF: 6 (같음)")
    print("    - actuator: Dynamixel 기반 (같음, WidowX 도 Dynamixel)")
    print("    - 크기/페이로드: 비슷한 sub-meter scale")
    print()
    print("  Bridge / Bridge V2 dataset (~ 50K episodes) 가 WidowX 사용 -> ")
    print("  Phase 7 의 LoRA fine-tune 시 가장 유용한 prior.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) q/k/v/o_proj + gate/up/down_proj")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Llama 2 의 transformer block 의 linear layer:")
    print("    Attention: q_proj, k_proj, v_proj, o_proj")
    print("    FFN     : gate_proj, up_proj, down_proj (SwiGLU 구조)")
    print()
    print("  OpenVLA + LoRA 권장 설정:")
    print("    target_modules = 7 개 module 전부")
    print("    rank = 32, alpha = 64")
    print("    -> 표현력 + 메모리 효율 균형")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
