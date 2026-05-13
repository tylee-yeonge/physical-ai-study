"""
Phase 5 Week 12 - 최종 데모 & 포트폴리오 기초 퀴즈
"""


def problem1_bottleneck():
    print("\n" + "━" * 28)
    print("문제 1: 파이프라인 병목")
    print("━" * 28 + "\n")

    print("질문: YOLO(30ms) + Depth(55ms) + 융합(2ms) 순차 실행 시")
    print("      전체 파이프라인의 병목은?\n")

    print("보기:")
    print("  A) YOLO 추론 (30ms)")
    print("  B) Depth 추론 (55ms)")
    print("  C) 융합 + 역투영 (2ms)")
    print("  D) 전처리 + 후처리")


def problem2_optimization():
    print("\n" + "━" * 28)
    print("문제 2: 최적화 전략")
    print("━" * 28 + "\n")

    print("질문: TensorRT FP16 → INT8 변환 시 반드시 필요한 것은?\n")

    print("보기:")
    print("  A) 더 큰 GPU 메모리")
    print("  B) 캘리브레이션 데이터 (대표 입력 이미지)")
    print("  C) 새로운 ONNX 모델")
    print("  D) CUDA 재설치")


def problem3_parallel():
    print("\n" + "━" * 28)
    print("문제 3: 병렬 실행 효과")
    print("━" * 28 + "\n")

    print("질문: YOLO(30ms)와 Depth(55ms)를 CUDA 스트림으로 병렬 실행하면")
    print("      두 추론의 총 소요 시간은 약?\n")

    print("보기:")
    print("  A) 85ms (합산)")
    print("  B) 55ms (둘 중 긴 쪽)")
    print("  C) 42.5ms (평균)")
    print("  D) 27.5ms (절반)")


def problem4_portfolio():
    print("\n" + "━" * 28)
    print("문제 4: 포트폴리오 핵심 요소")
    print("━" * 28 + "\n")

    print("질문: Perception 포트폴리오에서 가장 임팩트가 큰 요소는?\n")

    print("보기:")
    print("  A) 코드 줄 수가 많은 것")
    print("  B) 데모 영상 + 정량적 성능 수치")
    print("  C) 사용한 라이브러리 개수")
    print("  D) 이론 설명 문서 분량")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 12 Quiz - Easy (최종 데모)")
    print("━" * 33)

    problem1_bottleneck()
    problem2_optimization()
    problem3_parallel()
    problem4_portfolio()

    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
