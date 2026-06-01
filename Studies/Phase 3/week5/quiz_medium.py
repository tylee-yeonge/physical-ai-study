"""
Quiz Medium - Week 5: ONNX 변환 (Section 5.2)
3문제 - 심화 이해
"""

def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 5 Quiz - Medium (ONNX 변환)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    print("Q1. ONNX Runtime의 그래프 최적화가 PyTorch보다 빠른 이유를")
    print("    3가지 이상 설명하세요.")
    print("   설명:\n")

    print("Q2. INT8 양자화에서 '캘리브레이션'이 필요한 이유와")
    print("    캘리브레이션 과정을 단계별로 설명하세요.")
    print("   설명:\n")

    print("Q3. 다음 코드에서 잘못된 부분을 찾고 수정하세요.")
    print("""
    import torch

    model = MyModel()
    dummy = torch.randn(1, 3, 640, 640)

    torch.onnx.export(
        model, dummy, "model.onnx",
        opset_version=7,
        dynamic_axes={'input': {0: 'batch', 2: 'height', 3: 'width'}}
    )
    """)
    print("   잘못된 부분과 수정: _____\n")

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
