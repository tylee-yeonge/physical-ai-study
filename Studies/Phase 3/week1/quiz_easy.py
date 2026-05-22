"""
Phase 3 Week 1 - PyTorch 기초 재정비 기초 퀴즈
"""


def problem1_tensor_shape():
    print("\n" + "━" * 28)
    print("문제 1: Tensor Shape 변환")
    print("━" * 28 + "\n")

    print("질문: 다음 코드의 출력 shape은?\n")
    print("  import torch")
    print("  x = torch.randn(4, 3, 32, 32)  # [B, C, H, W]")
    print("  y = x.view(4, -1)")
    print("  print(y.shape)\n")

    print("보기:")
    print("  A) torch.Size([4, 3072])")
    print("  B) torch.Size([4, 96, 32])")
    print("  C) torch.Size([12, 1024])")
    print("  D) torch.Size([4, 32, 96])")


def problem2_autograd():
    print("\n" + "━" * 28)
    print("문제 2: autograd 동작")
    print("━" * 28 + "\n")

    print("질문: 다음 코드에서 x.grad의 값은?\n")
    print("  x = torch.tensor(3.0, requires_grad=True)")
    print("  y = x ** 3")
    print("  y.backward()")
    print("  print(x.grad)\n")

    print("보기:")
    print("  A) tensor(9.0)")
    print("  B) tensor(27.0)")
    print("  C) tensor(3.0)")
    print("  D) tensor(6.0)")


def problem3_dataloader():
    print("\n" + "━" * 28)
    print("문제 3: DataLoader 이해")
    print("━" * 28 + "\n")

    print("질문: CIFAR-10 학습 데이터(50,000장)를 batch_size=128,")
    print("      drop_last=True로 DataLoader를 만들면,")
    print("      1 epoch에 몇 번의 iteration이 발생하는가?\n")

    print("보기:")
    print("  A) 390회 (50000 / 128 = 390.625, 올림)")
    print("  B) 391회 (50000 / 128, 올림)")
    print("  C) 390회 (50000 / 128 = 390.625, 마지막 배치 버림)")
    print("  D) 128회")


def problem4_train_eval():
    print("\n" + "━" * 28)
    print("문제 4: train() vs eval() 모드")
    print("━" * 28 + "\n")

    print("질문: model.eval()을 호출하지 않고 추론하면 어떤 문제가 발생하는가?\n")

    print("보기:")
    print("  A) GPU에서 실행이 안 된다")
    print("  B) BatchNorm과 Dropout이 학습 모드로 동작하여 추론 결과가 불안정해진다")
    print("  C) 에러가 발생하여 코드가 실행되지 않는다")
    print("  D) 메모리 부족(OOM)이 발생한다")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 1 Quiz - Easy")
    print("━" * 33)
    problem1_tensor_shape()
    problem2_autograd()
    problem3_dataloader()
    problem4_train_eval()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
