"""
Phase 5 Week 2 - CV 라이브러리 중급 퀴즈 풀이
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 풀이: BBox 변환 계산")
    print("━" * 28 + "\n")

    W, H = 640, 480
    x_min, y_min, x_max, y_max = 100, 200, 300, 400

    print(f"  원본 (Pascal VOC): [{x_min}, {y_min}, {x_max}, {y_max}]")
    print(f"  이미지 크기: {W}x{H}\n")

    # 1. Pascal VOC → YOLO
    x_center = (x_min + x_max) / 2 / W
    y_center = (y_min + y_max) / 2 / H
    width = (x_max - x_min) / W
    height = (y_max - y_min) / H

    print("  1) YOLO 변환:")
    print(f"     x_center = ({x_min}+{x_max})/2/{W} = {x_center:.4f}")
    print(f"     y_center = ({y_min}+{y_max})/2/{H} = {y_center:.4f}")
    print(f"     width = ({x_max}-{x_min})/{W} = {width:.4f}")
    print(f"     height = ({y_max}-{y_min})/{H} = {height:.4f}")
    print(f"     YOLO: [{x_center:.4f}, {y_center:.4f}, {width:.4f}, {height:.4f}]\n")

    # 2. HorizontalFlip
    x_center_flip = 1.0 - x_center
    print("  2) HorizontalFlip 후:")
    print(f"     x_center_flip = 1 - {x_center:.4f} = {x_center_flip:.4f}")
    print(f"     (y_center, width, height는 변하지 않음)")
    print(f"     YOLO: [{x_center_flip:.4f}, {y_center:.4f}, {width:.4f}, {height:.4f}]\n")

    # 3. Resize(320, 320)
    scale_x = 320 / W
    scale_y = 320 / H
    new_xmin = int(x_min * scale_x)
    new_ymin = int(y_min * scale_y)
    new_xmax = int(x_max * scale_x)
    new_ymax = int(y_max * scale_y)

    print("  3) Resize(320, 320) 후:")
    print(f"     scale_x = 320/{W} = {scale_x:.4f}")
    print(f"     scale_y = 320/{H} = {scale_y:.4f}")
    print(f"     VOC: [{new_xmin}, {new_ymin}, {new_xmax}, {new_ymax}]")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 풀이: Feature Map 크기 계산")
    print("━" * 28 + "\n")

    input_size = 640

    strides = [4, 8, 16, 32]
    channels = [256, 512, 1024, 2048]

    print(f"  입력: [1, 3, {input_size}, {input_size}]\n")
    for i, (s, c) in enumerate(zip(strides, channels)):
        h = input_size // s
        print(f"  Layer{i+1} (stride {s:2d}): [1, {c:4d}, {h:3d}, {h:3d}]")

    print("\n  공식: output_size = input_size / stride")
    print("  → stride가 클수록 해상도 낮고 채널 많음")
    print("  → Multi-scale detection에 활용 (FPN)")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 풀이: W&B 실험 비교")
    print("━" * 28 + "\n")

    print("  1. 가장 좋은 실험: exp-B")
    print("     → val_mAP 0.52 (가장 높음)")
    print("     → train_loss 0.3 (적절한 수준)")
    print("     → 과적합 없이 좋은 일반화\n")

    print("  2. exp-C의 문제: 과적합(Overfitting)")
    print("     → train_loss 0.15 (매우 낮음)")
    print("     → val_mAP 0.38 (오히려 낮음)")
    print("     → 학습 데이터는 잘 맞추지만 검증에서 실패")
    print("     → 원인: batch_size 16이 작아서 불안정할 수 있음\n")

    print("  3. 다음 실험 제안:")
    print("     → exp-B 기반으로 lr을 약간 조정 (0.0005~0.002)")
    print("     → 또는 Augmentation 강도 조절")
    print("     → exp-C의 과적합은 regularization 추가로 해결")


if __name__ == "__main__":
    print("━" * 33)
    print("Week 2 Quiz Medium - 풀이")
    print("━" * 33)

    problem1_solution()
    problem2_solution()
    problem3_solution()

    print("\n" + "━" * 33)
