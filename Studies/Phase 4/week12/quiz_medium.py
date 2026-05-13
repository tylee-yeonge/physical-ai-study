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

    expected = {"A": 10, "B": 15, "C": 20, "D": 10, "E": 5}
    total_expected = sum(expected.values())

    print("  당신의 답:")
    your_total = sum(section_secs.values())
    for k, v in section_secs.items():
        mark = "[O]" if abs(v - expected[k]) <= 3 else "[X]"
        print(f"  {mark} {k}: {v}s  (기대: {expected[k]}s)")
    print(f"  Total: {your_total}s (기대: {total_expected}s)")


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

    expected = {
        "A": "camera/image",
        "B": "vla/action/linear_{x,y,z}",
        "C": "vla/action/angular_{x,y,z}",
        "D": "vla/gripper",
        "E": "vla/latency_ms",
    }

    print("  당신의 답:")
    for k, v in paths.items():
        print(f"  {k}: '{v}'")
        print(f"      (기대: '{expected[k]}')")
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
    included = ""  # 예: "A,B,C"

    expected = "A,B,C,D,E,F,G"  # H 는 보안상 절대 포함하면 안 됨!

    print(f"  당신의 답 : {included}")
    print(f"  기대 답   : {expected}")
    print()
    print("  주의: H (huggingface_credentials.txt) 는 절대 포함하면 안 됨!")
    print("        token 은 환경 변수 또는 .env (gitignore) 로 관리.")


if __name__ == "__main__":
    print("=" * 60)
    problem1_video_section_priority()
    problem2_rerun_log_design()
    problem3_packaging_files()
    print("=" * 60)
