"""
Phase 5 Week 3 - YOLO 이론 기초 퀴즈
"""


def problem1_anchor_free():
    print("\n" + "━" * 28)
    print("문제 1: Anchor-Free 방식")
    print("━" * 28 + "\n")

    print("질문: YOLOv8 / YOLO11이 Anchor-Free 방식을 채택한 주된 이유는?\n")
    print("보기:")
    print("  A) GPU 메모리를 절약하기 위해")
    print("  B) Anchor 설계/튜닝 부담을 없애고 더 유연한 BBox 예측을 위해")
    print("  C) 이미지 해상도를 높이기 위해")
    print("  D) 클래스 수를 늘리기 위해")


def problem2_yolo11_structure():
    print("\n" + "━" * 28)
    print("문제 2: YOLO11 구조")
    print("━" * 28 + "\n")

    print("질문: YOLO11의 Backbone-Neck-Head 각각의 역할로 올바른 것은?\n")
    print("보기:")
    print("  A) Backbone: 특징 융합, Neck: 특징 추출, Head: 예측")
    print("  B) Backbone: 특징 추출, Neck: 특징 융합, Head: BBox+Class 예측")
    print("  C) Backbone: 예측, Neck: 특징 추출, Head: 특징 융합")
    print("  D) Backbone: 데이터 전처리, Neck: 후처리, Head: 학습")


def problem3_ciou():
    print("\n" + "━" * 28)
    print("문제 3: CIoU Loss")
    print("━" * 28 + "\n")

    print("질문: CIoU Loss가 기본 IoU Loss보다 나은 점은?\n")
    print("보기:")
    print("  A) 계산이 더 빠르다")
    print("  B) BBox가 겹치지 않아도 중심 거리와 종횡비를 고려하여 gradient를 제공한다")
    print("  C) 클래스 분류도 함께 수행한다")
    print("  D) GPU 없이도 동작한다")


def problem4_map():
    print("\n" + "━" * 28)
    print("문제 4: mAP 평가 지표")
    print("━" * 28 + "\n")

    print("질문: mAP@0.5:0.95가 mAP@0.5보다 더 엄격한 지표인 이유는?\n")
    print("보기:")
    print("  A) 더 많은 이미지를 평가하기 때문")
    print("  B) IoU 임계값 0.5~0.95 범위에서 평균하여 정확한 위치 예측을 요구하기 때문")
    print("  C) FPS를 함께 고려하기 때문")
    print("  D) 클래스 수가 더 많기 때문")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 3 Quiz - Easy")
    print("━" * 33)
    problem1_anchor_free()
    problem2_yolo11_structure()
    problem3_ciou()
    problem4_map()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
