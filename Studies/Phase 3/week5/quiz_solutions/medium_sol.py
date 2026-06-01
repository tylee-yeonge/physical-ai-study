"""
Solutions - Medium Quiz (Week 5: ONNX 변환)
"""

def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 5 Quiz - Medium 정답")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    print("Q1. ONNX Runtime이 PyTorch보다 빠른 이유:")
    print("    1. 그래프 최적화: Conv+BN+ReLU → FusedConvBNRelu (연산 합침)")
    print("    2. 상수 폴딩: 컴파일 시 계산 가능한 것은 미리 계산")
    print("    3. 메모리 최적화: 불필요한 중간 텐서 제거")
    print("    4. 커널 튜닝: 하드웨어에 최적화된 연산 커널 자동 선택")
    print("    5. 불필요한 Python 오버헤드 없음 (C++ 백엔드)\n")

    print("Q2. INT8 캘리브레이션이 필요한 이유:")
    print("    FP32의 넓은 범위(-3.4e38~3.4e38)를 INT8(-128~127)로 매핑하려면")
    print("    각 레이어의 실제 값 분포를 알아야 최적의 스케일을 결정할 수 있음")
    print()
    print("    캘리브레이션 과정:")
    print("    Step 1: 대표 이미지 100-500장 준비")
    print("    Step 2: FP32 모델로 추론하며 각 레이어의 출력 범위(min/max) 수집")
    print("    Step 3: Histogram 기반으로 최적의 scale, zero_point 계산")
    print("    Step 4: scale과 zero_point를 사용하여 가중치를 INT8로 변환")
    print("    Step 5: 변환된 모델의 정확도 검증\n")

    print("Q3. 잘못된 부분:")
    print("    1. model.eval() 호출이 빠짐 → export 전에 추가해야 함")
    print("       (BatchNorm, Dropout이 학습 모드에서는 다르게 동작)")
    print("    2. input_names/output_names가 없음")
    print("       (dynamic_axes의 키 'input'이 매칭될 이름이 없어 무시됨)")
    print("    3. opset_version=7 → 12 이상 권장")
    print("       (Opset 7은 너무 오래됨, 지원 연산 부족)")
    print()
    print("    수정된 코드:")
    print("    model.eval()  # 반드시 eval 모드!")
    print("    torch.onnx.export(")
    print("        model, dummy, 'model.onnx',")
    print("        opset_version=12,           # 12 이상 권장")
    print("        input_names=['input'],      # dynamic_axes 키와 일치시킴")
    print("        output_names=['output'],")
    print("        dynamic_axes={'input': {0: 'batch', 2: 'height', 3: 'width'}}")
    print("    )")

    print("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
