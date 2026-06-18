"""
Phase 4 Week 10 - 중급 퀴즈
"""


def problem1_directory_structure():
    """
    문제 1: ament_python 패키지의 표준 디렉토리 구조

    아래 위치에 들어가는 파일/디렉토리 이름을 채우시오:

    vla_node/
      [A]               # package metadata
      [B]               # build entry point + executables
      [C]               # build config
      [D]/              # ROS marker (빈 디렉토리)
      [E]/              # 실제 Python 코드
        __init__.py
        vla_inference_node.py

    TODO: A~E 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 1: ament_python 디렉토리 구조")
    print("=" * 60 + "\n")

    # TODO
    answers = {
        "A (package metadata)": "",
        "B (build entry point)": "",
        "C (build config)": "",
        "D (ROS marker dir)": "",
        "E (실제 코드 dir)": "",
    }
    print("  당신의 답:")
    for k, v in answers.items():
        print(f"    {k}: '{v}'")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem2_lifecycle_skeleton():
    """
    문제 2: Callback 의 진입점 순서

    image_callback 안에서 다음 단계의 순서:
      A) action publish
      B) image age check
      C) preprocess (bgr -> rgb)
      D) inference (predict)
      E) instruction 비어있으면 skip

    TODO: 순서를 채우시오 (콤마로 구분, 예: "A,B,C,D,E" 형식)
    """
    print("\n" + "=" * 60)
    print("문제 2: image_callback 의 단계 순서")
    print("=" * 60 + "\n")

    # TODO
    order = ""

    print(f"  당신의 답 : {order}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem3_qos_match():
    """
    문제 3: subscribe / publish 의 QoS 매칭

    아래 시나리오 별로 reliability / depth 권장값:

      Topic                | reliability | depth
      /camera/image_raw   | ?            | ?
      /vla/instruction   | ?            | ?
      /vla/action         | ?            | ?

    TODO 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 3: QoS 결정")
    print("=" * 60 + "\n")

    # TODO
    qos = {
        "image_raw": ("", 0),
        "instruction": ("", 0),
        "action": ("", 0),
    }
    print("  당신의 QoS:")
    for k, v in qos.items():
        print(f"    {k}: {v}")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    problem1_directory_structure()
    problem2_lifecycle_skeleton()
    problem3_qos_match()
    print("=" * 60)
