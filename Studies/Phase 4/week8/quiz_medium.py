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

    print(f"  당신의 답 : {sorted(answers.replace(' ', '').split(',')) if answers else '(empty)'}")
    print("  정답/해설은 quiz_solutions/medium_sol.py 참고")


def problem2_exception_mapping():
    """
    문제 2: Exception 클래스 매핑

    아래 시나리오 - exception 매핑:
    시나리오:
      1) model file 손상
      2) image 가 numpy array 가 아님
      3) inference 결과에 NaN
      4) CUDA OOM
      5) 알 수 없는 에러

    Exception 클래스 후보 (순서 무관):
      VLAOutputError, VLAOOMError, VLAModelError,
      VLAInferenceError (base), VLAInputError

    TODO: mapping 딕셔너리에 각 번호에 맞는 Exception 클래스명을 채우시오.
    """
    print("\n" + "=" * 60)
    print("문제 2: Exception 클래스 매핑")
    print("=" * 60 + "\n")

    # TODO
    mapping = {
        1: "",  # 시나리오 1
        2: "",  # 시나리오 2
        3: "",  # 시나리오 3
        4: "",  # 시나리오 4
        5: "",  # 시나리오 5
    }

    print("  당신의 답:")
    for k, v in mapping.items():
        print(f"    {k}: {v}")
    print("  정답/해설은 quiz_solutions/medium_sol.py 참고")


def problem3_pipeline_order():
    """
    문제 3: predict() 의 단계 순서

    아래 단계를 올바른 순서로 나열:
      A) prompt 생성
      B) input validation
      C) model.predict_action 호출
      D) output validation (NaN / shape)
      E) processor 로 tensor 변환

    TODO: 순서를 채우시오 (콤마로 구분, 예: "A,B,C,D,E" 형식)
    """
    print("\n" + "=" * 60)
    print("문제 3: predict() 의 단계 순서")
    print("=" * 60 + "\n")

    # TODO
    order = ""  # 콤마로 구분된 순서 (예: "A,B,C,D,E" 형식)

    print(f"  당신의 답 : {order}")
    print("  정답/해설은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    problem1_class_responsibilities()
    problem2_exception_mapping()
    problem3_pipeline_order()
    print("=" * 60)
