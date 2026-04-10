"""
Solutions - Easy Quiz (Week 5: ONNX 변환)
"""

def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 5 Quiz - Easy 정답")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    print("Q1. Open Neural Network Exchange")
    print("    핵심 목적: 프레임워크 독립적인 모델 포맷으로")
    print("    PyTorch, TensorFlow 등 다양한 프레임워크에서 학습한 모델을")
    print("    하나의 표준 포맷으로 변환하여 어디서든 추론 가능하게 함\n")

    print("Q2. b) Opset 12")
    print("    이유: 안정적이고 TensorRT, ONNX Runtime 등")
    print("    주요 런타임과 호환성이 가장 좋음\n")

    print("Q3. Static Shape [1, 3, 640, 640]")
    print("    이유:")
    print("    1. 카메라 해상도가 고정 (640x480)")
    print("    2. 배치 크기 1 (실시간 단일 프레임)")
    print("    3. TensorRT 최적화가 Static에서 극대화")
    print("    4. 메모리 사전 할당으로 지연 시간 감소\n")

    print("Q4. FP32 → FP16 양자화 장점:")
    print("    1. 모델 크기 50% 감소 (메모리 절약)")
    print("    2. 추론 속도 ~2배 향상")
    print("    3. 정확도 손실 거의 없음 (< 0.5% mAP)")
    print("    4. 캘리브레이션 불필요 (단순 타입 캐스팅)")

    print("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
