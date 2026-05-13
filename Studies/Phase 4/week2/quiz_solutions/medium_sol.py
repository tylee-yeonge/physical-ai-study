"""
Phase 4 Week 2 - Co-fine-tuning + Action Tokenization 중급 퀴즈 정답
"""
import numpy as np


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: token ID -> continuous action")
    print("=" * 60 + "\n")

    token_ids = np.array([255879, 255698, 255856, 255872, 255872, 255872, 255744])
    ACTION_TOKEN_START = 255744
    N_BIN = 256

    a_min = np.array([-0.10, -0.10, -0.10, -np.pi, -np.pi, -np.pi, 0.0])
    a_max = np.array([ 0.10,  0.10,  0.10,  np.pi,  np.pi,  np.pi, 1.0])

    bin_idx = token_ids - ACTION_TOKEN_START
    action = a_min + (bin_idx + 0.5) / N_BIN * (a_max - a_min)

    print("  Step 1: token_id -> bin_idx (subtract ACTION_TOKEN_START)")
    print(f"    {token_ids.tolist()}")
    print(f"  -> bin_idx = {bin_idx.tolist()}")
    print()
    print("  Step 2: bin_idx -> continuous action")
    print("    a = a_min + (bin + 0.5) / N_BIN * (a_max - a_min)")
    print()
    print("  Dim별 계산:")
    for i, name in enumerate(['dx', 'dy', 'dz', 'rx', 'ry', 'rz', 'gripper']):
        step = (a_max[i] - a_min[i]) / N_BIN
        print(f"    {name}: bin={bin_idx[i]:>3d} -> a = {a_min[i]:+.4f} + "
              f"({bin_idx[i]:>3d}.5)*{step:.6f} = {action[i]:+.5f}")

    print()
    print(f"  최종 action: {[round(x, 4) for x in action]}")

    print("\n  [tip] bin=128 (= 255872 - 255744) 은 거의 중심값")
    print("       -> dx = 0 m, rx = 0 rad 등 'no motion' 에 해당")
    print("       마지막 dim gripper=0 (=255744-255744) -> gripper 거의 닫힘")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: C) Web 학습 데이터에서 일정 빈도 이상 등장")
    print("=" * 60 + "\n")

    print("해설:")
    print("  각 시나리오의 위험도:")
    print()
    print("  A) sub-word 의 우연 일치 (Hello supersonic boy):")
    print("     -> 한두 sample 에서 우연히 발생")
    print("     -> 거의 무시할 수준 (noise)")
    print()
    print("  B) VQA 답에 포함:")
    print("     -> 답이 보통 짧고, action token 범위는 드물게 등장")
    print("     -> A 와 유사한 noise 수준")
    print()
    print("  C) 일정 빈도 이상 (예: batch 당 0.1% token 이 그 범위):")
    print("     -> 매 batch 마다 conflicting gradient 발생")
    print("     -> embedding 이 web 의미와 action 의미 사이에서 끌어당겨짐")
    print("     -> 학습 수렴이 느려지거나 양쪽 다 망가짐")
    print("     => 가장 위험!")
    print()
    print("  D) Robot data 의 instruction text 에 action 범위 token 등장:")
    print("     -> robot data 의 instruction 은 보통 짧고 정형화")
    print("     -> 빈도 거의 0")
    print()
    print("  [tip] 이 문제를 회피하기 위해 RT-2 는:")
    print("       - vocab 의 마지막 256개 (가장 빈도 낮은 부분) 만 선택")
    print("       - 추가로 web data preprocessing 시 그 범위 token 을 강제 제외")
    print("       - 또는 action 전용 special token 추가 (vocab 확장)")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: Combined loss = 약 3.0625")
    print("=" * 60 + "\n")

    n_web = 8
    n_vqa = 4
    n_robot = 4
    loss_web = 2.3
    loss_vqa = 3.1
    loss_robot = 4.5

    combined = (n_web * loss_web + n_vqa * loss_vqa + n_robot * loss_robot) / (
        n_web + n_vqa + n_robot
    )

    print("  공식: weighted average")
    print("    L = sum(n_i * L_i) / sum(n_i)")
    print()
    print(f"    n_web*L_web   = {n_web} * {loss_web} = {n_web * loss_web}")
    print(f"    n_vqa*L_vqa   = {n_vqa} * {loss_vqa} = {n_vqa * loss_vqa}")
    print(f"    n_robot*L_rob = {n_robot} * {loss_robot} = {n_robot * loss_robot}")
    print(f"    합           = {n_web * loss_web + n_vqa * loss_vqa + n_robot * loss_robot}")
    print(f"    /16          = {combined:.4f}")
    print()
    print("  [tip] PyTorch 의 F.cross_entropy(..., reduction='mean') 의 동작:")
    print("       - 각 token 의 loss 를 모두 평균 (단순 mean)")
    print("       - 이때 sample 간 weighting 은 sequence length 의 영향을 받음")
    print()
    print("  실제 RT-2 의 학습 코드는 sequence packing / padding 의 영향으로")
    print("  정확한 계산이 더 복잡하지만, 위 weighted average 가 직관적 근사.")


if __name__ == "__main__":
    print("=" * 60)
    print("  Phase 4 Week 2 Quiz - Medium 정답")
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "=" * 60)
