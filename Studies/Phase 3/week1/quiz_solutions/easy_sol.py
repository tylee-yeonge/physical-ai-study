"""
Phase 3 Week 1 - PyTorch 기초 재정비 기초 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 정답: A) torch.Size([4, 3072])")
    print("━" * 28 + "\n")

    print("해설:")
    print("  x = torch.randn(4, 3, 32, 32)")
    print("  → 원래 shape: [4, 3, 32, 32]")
    print("  → 총 요소 수: 4 * 3 * 32 * 32 = 12,288")
    print()
    print("  y = x.view(4, -1)")
    print("  → 첫 번째 차원은 4로 유지")
    print("  → -1은 나머지를 자동 계산: 12288 / 4 = 3072")
    print("  → 결과: torch.Size([4, 3072])")
    print()
    print("  💡 view(-1)은 'flatten'과 같은 효과입니다.")
    print("     배치 차원(B=4)은 유지하고 나머지를 1차원으로 펴는 것은")
    print("     CNN의 출력을 FC 레이어에 연결할 때 자주 사용됩니다.")

    import torch
    x = torch.randn(4, 3, 32, 32)
    y = x.view(4, -1)
    print(f"\n  검증: {y.shape}")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 정답: B) tensor(27.0)")
    print("━" * 28 + "\n")

    print("해설:")
    print("  y = x^3")
    print("  dy/dx = 3x^2")
    print("  x = 3.0일 때: 3 * (3.0)^2 = 3 * 9 = 27.0")
    print()
    print("  💡 autograd는 chain rule을 자동 적용합니다.")
    print("     backward()를 호출하면 computational graph를 역순으로")
    print("     따라가며 각 텐서의 gradient를 계산합니다.")

    import torch
    x = torch.tensor(3.0, requires_grad=True)
    y = x ** 3
    y.backward()
    print(f"\n  검증: x.grad = {x.grad}")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 정답: C) 390회")
    print("━" * 28 + "\n")

    print("해설:")
    print("  총 데이터: 50,000장")
    print("  batch_size: 128")
    print("  50,000 / 128 = 390.625")
    print()
    print("  drop_last=True이므로:")
    print("  → 마지막 불완전 배치(0.625 * 128 = 80장)를 버림")
    print("  → 정확히 390번의 iteration")
    print()
    print("  drop_last=False라면:")
    print("  → 391번 (마지막 배치는 80장)")
    print()
    print("  💡 drop_last=True를 사용하는 이유:")
    print("     - BatchNorm은 배치 크기가 너무 작으면 불안정해질 수 있음")
    print("     - 학습 시 모든 배치의 크기를 동일하게 유지하기 위해")

    print(f"\n  검증: 50000 // 128 = {50000 // 128}")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 정답: B) BatchNorm과 Dropout이 학습 모드로 동작")
    print("━" * 28 + "\n")

    print("해설:")
    print("  model.eval()을 호출하지 않으면:")
    print()
    print("  1. BatchNorm:")
    print("     - train 모드: 현재 배치의 mean/var 사용")
    print("     - eval 모드: 학습 중 누적된 running_mean/running_var 사용")
    print("     → 추론 시 배치마다 다른 결과가 나올 수 있음!")
    print()
    print("  2. Dropout:")
    print("     - train 모드: 랜덤하게 뉴런 비활성화")
    print("     - eval 모드: 모든 뉴런 활성화")
    print("     → 추론 시 매번 다른 결과가 나옴!")
    print()
    print("  💡 추론 시에는 반드시 model.eval() + torch.no_grad()를 함께 사용합니다:")
    print("     model.eval()")
    print("     with torch.no_grad():")
    print("         output = model(input)")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 1 Quiz - Easy 정답")
    print("━" * 33)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "━" * 33)
