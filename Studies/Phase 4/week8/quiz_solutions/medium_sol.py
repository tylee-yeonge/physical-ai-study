"""
Phase 4 Week 8 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: VLAInference class 의 책임 - A, C, D")
    print("=" * 60 + "\n")
    print("  A) 모델 로드          - O (이 class 의 일)")
    print("  B) image preprocess   - X (preprocess.py 가 따로 담당)")
    print("  C) predict_action 호출 - O")
    print("  D) NaN 검출           - O (output validation)")
    print("  E) ROS2 topic 발행    - X (ROS2 노드의 책임)")
    print("  F) Stress test 실행   - X (테스트 스크립트)")
    print()
    print("  [tip] Single Responsibility 의 원칙:")
    print("    - VLAInference  : 모델 로드 + predict + validate")
    print("    - preprocess.py : image 변환")
    print("    - ROS2 노드     : msg subscribe/publish, callback, lifecycle")
    print("    - test script   : 다양한 입력으로 stress / regression")
    print()
    print("  이렇게 책임 분리하면:")
    print("    - 각 모듈을 독립적으로 unit test 가능")
    print("    - ROS2 통합 시 노드는 VLAInference 만 import")
    print("    - 다른 framework (예: gradio, FastAPI) 로 wrap 도 쉬움")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: Exception 매핑")
    print("=" * 60 + "\n")
    mapping = {
        1: ("model file 손상", "VLAModelError"),
        2: ("image type / mode 잘못", "VLAInputError"),
        3: ("NaN 결과", "VLAOutputError"),
        4: ("CUDA OOM", "VLAOOMError"),
        5: ("알 수 없는", "VLAInferenceError (base)"),
    }
    for k, (scenario, exc) in mapping.items():
        print(f"  {k}) {scenario:25s} -> {exc}")

    print()
    print("  [tip] Exception hierarchy 설계의 의의:")
    print("    - ROS2 노드에서 `except VLAOOMError` 로 OOM 만 잡아 fallback")
    print("    - `except VLAInferenceError` 로 모든 VLA 에러를 한 번에 잡기")
    print("    - error class 가 명확해야 빠른 디버깅")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: predict() 단계 순서 - B,A,E,C,D")
    print("=" * 60 + "\n")

    steps = [
        ("B", "input validation", "잘못된 입력은 빨리 catch (fail fast)"),
        ("A", "prompt 생성", "'In: What action ...?' template"),
        ("E", "processor 로 tensor 변환", "HuggingFace AutoProcessor"),
        ("C", "model.predict_action", "실제 inference 호출"),
        ("D", "output validation", "NaN / shape (7,) 확인"),
    ]

    for i, (k, name, why) in enumerate(steps, 1):
        print(f"  {i}. ({k}) {name}")
        print(f"     -> {why}")

    print()
    print("  [tip] 이 순서가 양산 SW 의 표준:")
    print("    'validate input -> transform -> compute -> validate output'")
    print("    어느 단계의 에러도 명확히 구분 가능.")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
