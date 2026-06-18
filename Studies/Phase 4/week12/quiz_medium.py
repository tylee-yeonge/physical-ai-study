"""
Phase 4 Week 12 - 중급 퀴즈
"""


def problem1_video_section_priority():
    """
    문제 1: 1분 영상 의 section 별 분량 결정

    아래 5 가지 section 에 각 몇 초를 배분할지 결정 (총 60 초).

    A) 인트로 (Title + 본인 정보)
    B) System 구조 (rqt_graph 등)
    C) 실시간 동작 (Rerun)
    D) 결과 + 한계
    E) Next (Phase 7 예고)

    TODO: 시간 배분 (초).
    """
    print("\n" + "=" * 60)
    print("문제 1: 1분 영상 section 별 분량")
    print("=" * 60 + "\n")

    # TODO
    section_secs = {"A": 0, "B": 0, "C": 0, "D": 0, "E": 0}

    print("  당신의 답:")
    your_total = sum(section_secs.values())
    for k, v in section_secs.items():
        print(f"  {k}: {v}s")
    print(f"  Total: {your_total}s")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def problem2_rerun_log_design():
    """
    문제 2: Rerun 의 entity path 설계

    아래 5 가지 정보를 Rerun 에 log 할 때 적절한 entity path:
      A) camera image
      B) action linear x/y/z
      C) action angular x/y/z
      D) gripper position
      E) latency

    TODO 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 2: Rerun entity path 설계")
    print("=" * 60 + "\n")

    # TODO
    paths = {"A": "", "B": "", "C": "", "D": "", "E": ""}

    print("  당신의 답:")
    for k, v in paths.items():
        print(f"  {k}: '{v}'")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")
    print()
    print("  [tip] entity path 의 원칙:")
    print("    - 슬래시로 계층 분리")
    print("    - 의미 그룹별 prefix (vla/action/*, vla/gripper, etc.)")
    print("    - 짧고 검색 쉽게")


def problem3_packaging_files():
    """
    문제 3: Portfolio/02_VLA_demo/ 의 파일 list

    `physical-ai-study/Portfolio/02_VLA_demo/` 에 들어가야 할 파일을
    모두 고르시오.

    A) vla_demo.mp4 (1분 영상)
    B) README.md (산출물 설명)
    C) vla_inference/ (Python wrapper)
    D) vla_node/ (ROS2 패키지)
    E) bag/ (1분 dry-run bag 압축)
    F) latency_data.csv (측정 데이터)
    G) blog_links.md (RT-2 + OpenVLA 블로그 URL)
    H) huggingface_credentials.txt (HuggingFace token)

    TODO: 포함되어야 할 항목을 모두 답.
    """
    print("\n" + "=" * 60)
    print("문제 3: Portfolio 패키징 파일")
    print("=" * 60 + "\n")

    # TODO
    included = ""  # 포함할 항목 코드를 콤마로 구분

    print(f"  당신의 답 : {included}")
    print()
    print("  힌트: credential / token 류 파일은 절대 패키징에 포함하지 말 것.")
    print("        token 은 환경 변수 또는 .env (gitignore) 로 관리.")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("=" * 60)
    problem1_video_section_priority()
    problem2_rerun_log_design()
    problem3_packaging_files()
    print("=" * 60)
