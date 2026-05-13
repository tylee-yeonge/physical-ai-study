"""
Phase 4 Week 14 - 중급 퀴즈: README 작성
"""


def problem1_section_order():
    """
    문제 1: README section 의 표준 순서

    아래 section 들을 표준 순서로 나열:
      A) License
      B) Quick Start
      C) Title + One-liner
      D) Demo 영상
      E) Architecture
      F) Installation
      G) Usage
      H) Results
      I) Troubleshooting

    TODO: 순서 (예: "C,D,B,...")
    """
    print("\n" + "=" * 60)
    print("문제 1: README section 표준 순서")
    print("=" * 60 + "\n")

    # TODO
    order = ""

    expected = "C,D,B,E,F,G,H,I,A"

    print(f"  당신의 답 : {order}")
    print(f"  기대 답   : {expected}")


def problem2_troubleshooting_table():
    """
    문제 2: 트러블슈팅 표 작성

    OpenVLA + ROS2 환경에서 자주 발생할 5 가지 문제와 해결책을 작성.

    형식: 한 row 마다 (증상, 원인, 해결)

    TODO: troubleshooting 채우기 (5개 row).
    """
    print("\n" + "=" * 60)
    print("문제 2: 트러블슈팅 표 작성")
    print("=" * 60 + "\n")

    # TODO
    troubleshooting = [
        # ("증상", "원인", "해결"),
    ]

    expected_keywords = [
        ["OOM", "out of memory", "메모리"],
        ["flash_attn", "import"],
        ["bag", "image"],
        ["unnormalize", "action"],
        ["colcon", "build", "source"],
    ]

    print("  당신의 표:")
    for i, row in enumerate(troubleshooting):
        print(f"    {i+1}) {row}")

    print("\n  자가 평가 (각 row 가 관련 키워드 포함 여부):")
    for i, kw_list in enumerate(expected_keywords):
        found = any(
            any(kw.lower() in str(row).lower() for kw in kw_list)
            for row in troubleshooting
        )
        mark = "[O]" if found else "[X]"
        print(f"    {mark} row {i+1}: should mention {kw_list}")


def problem3_quickstart_time():
    """
    문제 3: Quick Start 의 권장 시간

    아래 시나리오에서 Quick Start 가 걸리는 시간:
      - git clone
      - apt install (ROS2)
      - conda create + python deps
      - pip install -e vla_inference
      - colcon build
      - OpenVLA download
      - 첫 inference

    각 단계의 시간 + 총 시간을 추정.

    TODO: 시간 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 3: Quick Start 시간 추정")
    print("=" * 60 + "\n")

    # TODO (분 단위)
    times_min = {
        "git clone": 0,
        "ROS2 apt install": 0,
        "conda + pip": 0,
        "colcon build": 0,
        "OpenVLA download (15GB)": 0,
        "첫 inference": 0,
    }

    expected = {
        "git clone": 1,
        "ROS2 apt install": 10,
        "conda + pip": 5,
        "colcon build": 2,
        "OpenVLA download (15GB)": 15,
        "첫 inference": 1,
    }

    print("  당신의 추정 (분):")
    your_total = 0
    for k, v in times_min.items():
        your_total += v
        print(f"    {k}: {v} (기대 ~ {expected[k]})")
    print(f"  Total: {your_total} 분 (기대 ~ {sum(expected.values())} 분)")
    print()
    print("  [tip] 총 30~40분 안에 첫 inference 까지 가는 게 Quick Start 의 표준.")
    print("       이보다 길면 'Quick' 의미 잃음 -> docker / pre-downloaded 검토.")


if __name__ == "__main__":
    print("=" * 60)
    problem1_section_order()
    problem2_troubleshooting_table()
    problem3_quickstart_time()
    print("=" * 60)
