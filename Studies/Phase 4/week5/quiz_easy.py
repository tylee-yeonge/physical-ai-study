"""
Phase 4 Week 5 - OpenX-Embodiment + Fine-tuning 기초 퀴즈
"""


def problem1_openx_size():
    print("\n" + "=" * 50)
    print("문제 1: OpenX-Embodiment 의 규모")
    print("=" * 50 + "\n")
    print("질문: OpenX-Embodiment 의 episode 수와 embodiment 수는?\n")
    print("보기:")
    print("  A) 130K episodes, 1 embodiment")
    print("  B) 970K episodes, 22 embodiments")
    print("  C) 5M episodes, 50 embodiments")
    print("  D) 50K episodes, 5 embodiments")


def problem2_rlds_schema():
    print("\n" + "=" * 50)
    print("문제 2: RLDS schema 의 핵심 필드")
    print("=" * 50 + "\n")
    print("질문: RLDS (RL Dataset Schema) 의 한 step 에 반드시 들어가는 필드는?\n")
    print("보기:")
    print("  A) observation, action, reward, is_first/is_last/is_terminal")
    print("  B) image, label")
    print("  C) state, next_state, action")
    print("  D) input, output")


def problem3_self_arm_embodiment():
    print("\n" + "=" * 50)
    print("문제 3: 자작 6DOF 팔과 가장 가까운 OpenX embodiment")
    print("=" * 50 + "\n")
    print("질문: 자작 6DOF 팔 (Dynamixel XM430) 와 가장 가까운")
    print("      OpenX-Embodiment 의 embodiment 는?\n")
    print("보기:")
    print("  A) Franka Emika Panda (7-DoF, 자체 actuator)")
    print("  B) WidowX 250 (6-DoF, Dynamixel 기반)")
    print("  C) Sawyer (7-DoF Rethink)")
    print("  D) Boston Dynamics Spot (legged)")


def problem4_lora_target():
    print("\n" + "=" * 50)
    print("문제 4: LoRA target_modules 의 의미")
    print("=" * 50 + "\n")
    print("질문: OpenVLA + LoRA fine-tune 시 target_modules 에 들어가는")
    print("      대표적인 module 이름은?\n")
    print("보기:")
    print("  A) conv1, conv2, fc")
    print("  B) q_proj, k_proj, v_proj, o_proj, gate_proj, up_proj, down_proj")
    print("  C) encoder, decoder")
    print("  D) input_layer, output_layer")


def problem5_action_representation():
    print("\n" + "=" * 50)
    print("문제 5: OpenVLA 가 출력하는 action 의 표현")
    print("=" * 50 + "\n")
    print("질문: OpenVLA 는 연속 EE-delta action 을 어떻게 출력하는가?\n")
    print("보기:")
    print("  A) 6개 관절각 (joint angle) 을 float 로 직접 회귀")
    print("  B) 차원별 256 bin 으로 discretize 해 Llama vocabulary 토큰으로 출력")
    print("  C) 이미지 segmentation mask 로 출력")
    print("  D) 자연어 문장으로 동작을 서술")


if __name__ == "__main__":
    print("=" * 50)
    print("  Phase 4 Week 5 Quiz - Easy")
    print("=" * 50)
    problem1_openx_size()
    problem2_rlds_schema()
    problem3_self_arm_embodiment()
    problem4_lora_target()
    problem5_action_representation()
    print("\n" + "=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
