"""
Quiz Medium - Week 8: Depth Anything 사용 (Section 5.3)
3문제 - 심화 이해
"""


def problem1_metric_depth_conversion():
    print("\n" + "━" * 28)
    print("문제 1: Metric Depth 변환")
    print("━" * 28 + "\n")

    print("Depth Anything의 상대 깊이맵에서 절대 깊이(미터)로 변환하려 합니다.")
    print()
    print("  주어진 정보:")
    print("  - 참조점 A: 상대 깊이 0.85, 실제 거리 1.5m (가까운 의자)")
    print("  - 참조점 B: 상대 깊이 0.30, 실제 거리 6.0m (먼 벽)")
    print()
    print("  질문:")
    print("  1. 선형 변환 d_metric = alpha * d_relative + beta에서")
    print("     alpha와 beta를 구하세요.")
    print()
    print("  2. 상대 깊이가 0.60인 픽셀의 실제 거리는?")
    print()
    print("  3. 이 방법의 한계점을 2가지 이상 설명하세요.")
    print()
    print("  답: alpha=_____, beta=_____, 거리=_____m")
    print()


def problem2_preprocessing_debug():
    print("\n" + "━" * 28)
    print("문제 2: 전처리 디버깅")
    print("━" * 28 + "\n")

    print("다음 코드에서 잘못된 부분을 찾고 수정하세요.")
    print()
    print("  import torch")
    print("  import cv2")
    print("  from transformers import AutoModelForDepthEstimation")
    print()
    print("  model = AutoModelForDepthEstimation.from_pretrained(")
    print("      'LiheYoung/depth-anything-small-hf'")
    print("  )")
    print()
    print("  # 이미지 로드 (OpenCV)")
    print("  image = cv2.imread('test.jpg')  # BGR 형식")
    print()
    print("  # 전처리")
    print("  image_resized = cv2.resize(image, (518, 518))")
    print("  image_tensor = torch.from_numpy(image_resized).float()")
    print("  image_tensor = image_tensor.permute(2, 0, 1).unsqueeze(0)")
    print("  image_tensor = image_tensor / 255.0")
    print()
    print("  # 추론")
    print("  output = model(image_tensor)")
    print("  depth = output.predicted_depth")
    print()
    print("  잘못된 부분과 수정: _____")
    print()


def problem3_yolo_depth_integration():
    print("\n" + "━" * 28)
    print("문제 3: YOLO + Depth 통합 전략")
    print("━" * 28 + "\n")

    print("AMR(자율 이동 로봇)에서 YOLO와 Depth Anything을 함께 사용하여")
    print("장애물 회피 시스템을 설계하려 합니다.")
    print()
    print("  다음 질문에 답하세요:")
    print()
    print("  1. YOLO 검출 박스 내의 깊이 값을 요약하는 방법으로")
    print("     mean, median, min 중 어떤 것이 장애물 회피에 가장 적합한가요?")
    print("     이유와 함께 설명하세요.")
    print()
    print("  2. 깊이 기반 장애물 우선순위 판단 로직을 의사코드로 작성하세요.")
    print("     (가까운 장애물 > 먼 장애물)")
    print()
    print("  3. YOLO와 Depth 모델을 Jetson에서 동시에 실행할 때")
    print("     메모리/성능 관점에서 고려해야 할 점은?")
    print()
    print("  답: _____")
    print()


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 8 Quiz - Medium (Depth Anything 사용)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_metric_depth_conversion()
    problem2_preprocessing_debug()
    problem3_yolo_depth_integration()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
