"""
Phase 4 Week 8 - 중급 퀴즈: inference wrapper 설계 + 에러 처리
"""


def problem1_class_responsibilities():
    """
    문제 1: VLAInference class 의 책임 분리

    아래 동작 중 VLAInference class 의 책임은? (multi-answer)
      A) 모델 로드
      B) image preprocess (cv2 -> PIL)
      C) predict_action 호출
      D) NaN 검출
      E) ROS2 topic 발행
      F) Stress test 실행

    답: 책임에 해당하는 것을 모두 골라 'A,B,...' 형태.
    """
    print("\n" + "=" * 60)
    print("문제 1: VLAInference class 의 책임 범위")
    print("=" * 60 + "\n")

    # TODO
    answers = ""

    # A: 모델 로드 - O
    # B: image preprocess - X (preprocess.py 가 담당, 별도 모듈)
    # C: predict_action - O
    # D: NaN 검출 - O
    # E: ROS2 topic - X (ROS2 노드의 책임)
    # F: Stress test - X (테스트 스크립트의 책임)
    expected = sorted(["A", "C", "D"])

    print(f"  당신의 답 : {sorted(answers.replace(' ', '').split(',')) if answers else '(empty)'}")
    print(f"  기대 답   : {expected}")
    print()
    print("  자가 평가:")
    print("    A 모델 로드      : O (VLAInference 의 일)")
    print("    B image preprocess: X (preprocess.py 가 따로)")
    print("    C predict_action : O")
    print("    D NaN 검출       : O (output validation)")
    print("    E ROS2 topic 발행: X (ROS2 노드)")
    print("    F Stress test    : X (test script)")


def problem2_exception_mapping():
    """
    문제 2: Exception 클래스 매핑

    아래 시나리오 - exception 매핑:
      Scenario                              | Exception
      --------------------------------------|--------------------
      1) model file 손상                     | A) VLAModelError
      2) image 가 numpy array 가 아님       | B) VLAInputError
      3) inference 결과에 NaN               | C) VLAOutputError
      4) CUDA OOM                            | D) VLAOOMError
      5) 알 수 없는 에러                     | E) VLAInferenceError (base)

    TODO: mapping 딕셔너리를 채우시오.
    """
    print("\n" + "=" * 60)
    print("문제 2: Exception 클래스 매핑")
    print("=" * 60 + "\n")

    # TODO
    mapping = {
        1: "",  # model file 손상
        2: "",  # image type error
        3: "",  # NaN 결과
        4: "",  # OOM
        5: "",  # base
    }

    expected = {
        1: "VLAModelError",
        2: "VLAInputError",
        3: "VLAOutputError",
        4: "VLAOOMError",
        5: "VLAInferenceError",
    }

    print(f"  당신의 답:")
    for k, v in mapping.items():
        match = v == expected[k]
        mark = "[O]" if match else "[X]"
        print(f"    {mark} {k}: {v}  (기대: {expected[k]})")


def problem3_pipeline_order():
    """
    문제 3: predict() 의 단계 순서

    아래 단계를 올바른 순서로 나열:
      A) prompt 생성
      B) input validation
      C) model.predict_action 호출
      D) output validation (NaN / shape)
      E) processor 로 tensor 변환

    TODO: 순서를 채우시오 (예: "B,A,E,C,D")
    """
    print("\n" + "=" * 60)
    print("문제 3: predict() 의 단계 순서")
    print("=" * 60 + "\n")

    # TODO
    order = ""  # 예: "B,A,E,C,D"

    expected = "B,A,E,C,D"

    print(f"  당신의 답 : {order}")
    print(f"  기대 답   : {expected}")
    print()
    print("  정답 흐름:")
    print("    1. B (input validation)        - 잘못된 입력은 일찍 catch")
    print("    2. A (prompt 생성)              - instruction + 'In: ...' template")
    print("    3. E (processor tensor 변환)    - HuggingFace processor")
    print("    4. C (model.predict_action)     - 실제 inference")
    print("    5. D (output validation)        - NaN / shape 검증")


if __name__ == "__main__":
    print("=" * 60)
    problem1_class_responsibilities()
    problem2_exception_mapping()
    problem3_pipeline_order()
    print("=" * 60)
