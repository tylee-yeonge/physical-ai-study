"""
Phase 3 Week 1 - PyTorch 기초 재정비 중급 퀴즈 정답 및 해설
"""
import torch
import torch.nn as nn


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: Gradient 계산")
    print("━" * 36 + "\n")

    print("  f(x, y) = x^2 * y + y^3")
    print()
    print("  편미분:")
    print("    df/dx = 2xy")
    print("    df/dy = x^2 + 3y^2")
    print()
    print("  (x, y) = (2, 3) 대입:")
    print("    df/dx = 2 * 2 * 3 = 12.0")
    print("    df/dy = 2^2 + 3 * 3^2 = 4 + 27 = 31.0")

    # 검증
    x = torch.tensor(2.0, requires_grad=True)
    y = torch.tensor(3.0, requires_grad=True)
    f = x ** 2 * y + y ** 3
    f.backward()

    print(f"\n  PyTorch 검증:")
    print(f"    df/dx = {x.grad.item()} (기대: 12.0)")
    print(f"    df/dy = {y.grad.item()} (기대: 31.0)")

    print("\n  💡 chain rule과 편미분의 관계:")
    print("     autograd는 computational graph의 각 노드에서")
    print("     chain rule을 적용하여 gradient를 역전파합니다.")
    print("     x^2 * y에서 x에 대한 미분: d(x^2)/dx * y = 2xy")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: 파라미터 수 계산")
    print("━" * 36 + "\n")

    # Conv2d 파라미터: out_ch * (in_ch * kH * kW + 1)  (bias 포함)
    conv1 = 16 * (3 * 3 * 3 + 1)      # = 16 * 28 = 448
    conv2 = 32 * (16 * 3 * 3 + 1)     # = 32 * 145 = 4640
    fc = 10 * (32 * 8 * 8 + 1)        # = 10 * 2049 = 20490
    total = conv1 + conv2 + fc          # = 25578

    print(f"  Conv1: 16 * (3 * 3 * 3 + 1) = 16 * 28 = {conv1}")
    print(f"  Conv2: 32 * (16 * 3 * 3 + 1) = 32 * 145 = {conv2}")
    print(f"  FC:    10 * (32 * 8 * 8 + 1) = 10 * 2049 = {fc}")
    print(f"  총합:  {conv1} + {conv2} + {fc} = {total}")

    # 검증
    model = nn.Sequential(
        nn.Conv2d(3, 16, kernel_size=3, padding=1),
        nn.ReLU(),
        nn.MaxPool2d(2),
        nn.Conv2d(16, 32, kernel_size=3, padding=1),
        nn.ReLU(),
        nn.MaxPool2d(2),
        nn.Flatten(),
        nn.Linear(32 * 8 * 8, 10),
    )
    actual = sum(p.numel() for p in model.parameters() if p.requires_grad)
    print(f"\n  PyTorch 검증: {actual}")

    print("\n  💡 파라미터 계산 공식:")
    print("     Conv2d(in, out, k): out * (in * k * k + 1)")
    print("     Linear(in, out):    out * (in + 1)")
    print("     +1은 bias에 해당합니다.")
    print()
    print("  💡 ReLU, MaxPool2d, Flatten은 학습 가능한 파라미터가 없습니다.")

    # 각 레이어별 파라미터 출력
    print("\n  레이어별 파라미터:")
    for name, param in model.named_parameters():
        print(f"    {name}: {param.shape} → {param.numel()}")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: 학습률과 수렴")
    print("━" * 36 + "\n")

    print("  f(x) = (x - 5)^2, 최솟값: x = 5.0")
    print("  df/dx = 2(x - 5)")
    print("  업데이트: x_new = x - lr * 2(x - 5)")
    print()

    print("  lr=0.01 (너무 작은 학습률):")
    print("    → 매 스텝마다 조금씩만 이동")
    print("    → 20스텝 후에도 x ≈ 1.7 (5.0에 도달 못함)")
    print("    → '수렴은 하지만 매우 느림'")
    print()

    print("  lr=0.1 (적절한 학습률):")
    print("    → 적절한 속도로 이동")
    print("    → 약 10스텝 내에 x ≈ 5.0에 수렴")
    print("    → '적절한 학습률'")
    print()

    print("  lr=1.5 (너무 큰 학습률):")
    print("    → 업데이트가 너무 커서 최솟값을 지나침")
    print("    → x가 5.0 주변을 진동하거나 발산")
    print("    → 수렴 조건: lr < 1/L (L은 Lipschitz 상수)")
    print("    → f''(x) = 2이므로 lr < 1.0이어야 수렴")
    print("    → lr=1.5 > 1.0이므로 발산!")

    # 실제 실험
    print("\n  실험 결과:")
    for lr in [0.01, 0.1, 1.5]:
        x = torch.tensor(0.0, requires_grad=True)
        for _ in range(20):
            f = (x - 5) ** 2
            f.backward()
            with torch.no_grad():
                x -= lr * x.grad
            x.grad.zero_()
        print(f"    lr={lr:.2f}: x_final={x.item():>12.4f}, "
              f"f(x)={(x.item()-5)**2:>12.4f}")

    print("\n  💡 핵심 교훈:")
    print("     - 학습률이 너무 작으면 수렴이 느리고")
    print("     - 학습률이 너무 크면 발산할 수 있습니다.")
    print("     - 실제로는 lr scheduler를 사용하여 학습 초기에는 크게,")
    print("       후기에는 작게 조절합니다.")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 1 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
