"""
Phase 3 Week 1 - PyTorch 기초 재정비 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import torch
import torch.nn as nn
import numpy as np


def problem1_gradient_computation():
    """
    문제 1: Gradient 직접 계산 및 검증

    아래 함수 f(x, y) = x^2 * y + y^3 에 대해
    (x, y) = (2, 3) 에서의 편미분을 구하시오.

    df/dx = ?, df/dy = ?

    TODO: expected_dx, expected_dy를 직접 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: Gradient 직접 계산 및 검증")
    print("━" * 36 + "\n")

    x = torch.tensor(2.0, requires_grad=True)
    y = torch.tensor(3.0, requires_grad=True)

    # f(x, y) = x^2 * y + y^3
    f = x ** 2 * y + y ** 3
    f.backward()

    print(f"  f(x, y) = x^2 * y + y^3")
    print(f"  (x, y) = (2.0, 3.0)")
    print(f"  f = {f.item()}")
    print(f"\n  PyTorch가 계산한 gradient:")
    print(f"    df/dx = {x.grad.item()}")
    print(f"    df/dy = {y.grad.item()}")

    # TODO: 수학적으로 직접 계산해보세요
    # df/dx = 2xy = ?
    # df/dy = x^2 + 3y^2 = ?
    expected_dx = 0.0  # 여기를 채우시오
    expected_dy = 0.0  # 여기를 채우시오

    print(f"\n  직접 계산한 gradient:")
    print(f"    df/dx = {expected_dx}")
    print(f"    df/dy = {expected_dy}")

    if expected_dx == x.grad.item() and expected_dy == y.grad.item():
        print("\n  ✅ 정답!")
    else:
        print("\n  ❌ 다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_custom_model():
    """
    문제 2: 모델 파라미터 수 계산

    아래 모델의 총 학습 가능한 파라미터 수를 계산하시오.
    Conv2d의 파라미터: out_ch * (in_ch * kernel_h * kernel_w + 1)
    Linear의 파라미터: out_features * (in_features + 1)

    TODO: total_params_expected를 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 2: 모델 파라미터 수 계산")
    print("━" * 36 + "\n")

    model = nn.Sequential(
        nn.Conv2d(3, 16, kernel_size=3, padding=1),   # Conv1
        nn.ReLU(),
        nn.MaxPool2d(2),                                # 32x32 → 16x16
        nn.Conv2d(16, 32, kernel_size=3, padding=1),   # Conv2
        nn.ReLU(),
        nn.MaxPool2d(2),                                # 16x16 → 8x8
        nn.Flatten(),
        nn.Linear(32 * 8 * 8, 10),                     # FC
    )

    # PyTorch로 실제 파라미터 수 계산
    actual_params = sum(p.numel() for p in model.parameters() if p.requires_grad)

    print("  모델 구조:")
    print("    Conv2d(3→16, 3x3)  + ReLU + MaxPool2d")
    print("    Conv2d(16→32, 3x3) + ReLU + MaxPool2d")
    print("    Flatten + Linear(2048→10)")
    print(f"\n  입력: [B, 3, 32, 32]")
    print(f"\n  힌트:")
    print(f"    Conv1: 16 * (3 * 3 * 3 + 1) = ?")
    print(f"    Conv2: 32 * (16 * 3 * 3 + 1) = ?")
    print(f"    FC:    10 * (2048 + 1) = ?")

    # TODO: 직접 계산해보세요
    total_params_expected = 0  # 여기를 채우시오

    print(f"\n  실제 파라미터 수: {actual_params}")
    print(f"  계산한 파라미터 수: {total_params_expected}")

    if total_params_expected == actual_params:
        print("\n  ✅ 정답!")
    else:
        print("\n  ❌ 다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem3_learning_rate_effect():
    """
    문제 3: 학습률(Learning Rate)이 수렴에 미치는 영향

    간단한 최적화 문제: f(x) = (x - 5)^2 의 최솟값을 gradient descent로 찾기
    초기값 x = 0, 세 가지 학습률로 실험한다.

    질문: 어떤 학습률이 가장 빠르게 수렴하며,
          어떤 학습률은 발산하는가?
    """
    print("\n" + "━" * 36)
    print("문제 3: 학습률과 수렴 실험")
    print("━" * 36 + "\n")

    learning_rates = [0.01, 0.1, 1.5]
    n_steps = 20

    print(f"  목적함수: f(x) = (x - 5)^2,  최솟값은 x = 5.0")
    print(f"  초기값: x = 0.0")
    print(f"  Steps: {n_steps}\n")

    for lr in learning_rates:
        x = torch.tensor(0.0, requires_grad=True)

        values = []
        for step in range(n_steps):
            f = (x - 5) ** 2
            f.backward()
            values.append(x.item())

            with torch.no_grad():
                x -= lr * x.grad
            x.grad.zero_()

        final_x = x.item()
        converged = abs(final_x - 5.0) < 0.1

        print(f"  lr={lr:.2f}: x_final={final_x:>10.4f}, "
              f"{'수렴 ✅' if converged else '발산/미수렴 ❌'}")
        print(f"         경로: {' → '.join(f'{v:.2f}' for v in values[:5])} → ...")

    print("\n  💡 질문: lr=0.01, 0.1, 1.5 각각에 대해 왜 이런 결과가 나왔는지 설명하시오.")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 1 Quiz - Medium")
    print("━" * 40)
    problem1_gradient_computation()
    problem2_custom_model()
    problem3_learning_rate_effect()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
