"""
Phase 4 Week 12 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: 1분 영상 section 분량")
    print("=" * 60 + "\n")
    sections = [
        ("A", "Intro (Title + 본인)", 10),
        ("B", "System 구조", 15),
        ("C", "실시간 동작 (Rerun)", 20),
        ("D", "결과 + 한계", 10),
        ("E", "Next (Phase 7)", 5),
    ]
    print(f"  {'Section':<25}{'sec'}")
    print(f"  {'-'*25}{'-'*5}")
    for k, name, s in sections:
        print(f"  {k}) {name:<22}{s:>4}")
    print(f"  {'-'*25}{'-'*5}")
    print(f"  {'Total':<25}{60:>4}")
    print()
    print("  [tip] 'C' (실시간 동작) 가 가장 큰 분량 (1/3) - 핵심 흐름이라.")
    print("       Intro / Next 는 짧게 (각 5~10초).")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: Rerun entity path 설계")
    print("=" * 60 + "\n")
    paths = [
        ("A", "camera image", "camera/image"),
        ("B", "action linear", "vla/action/linear_x, _y, _z (각각)"),
        ("C", "action angular", "vla/action/angular_x, _y, _z"),
        ("D", "gripper", "vla/gripper"),
        ("E", "latency", "vla/latency_ms"),
    ]
    for k, name, path in paths:
        print(f"  {k}) {name:<20} -> {path}")
    print()
    print("  [tip] entity path 의 권장:")
    print("    1) 슬래시 (/) 로 hierarchy 분리")
    print("    2) 같은 의미 그룹은 같은 prefix")
    print("    3) Rerun UI 에서 tree 형태로 보임 -> path 가 곧 organization")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: Portfolio 포함/제외")
    print("=" * 60 + "\n")
    print("  포함 (A-G):")
    print("    A) vla_demo.mp4")
    print("    B) README.md")
    print("    C) vla_inference/ 패키지")
    print("    D) vla_node/ 패키지")
    print("    E) bag/ (압축, dry-run 결과)")
    print("    F) latency_data.csv")
    print("    G) blog_links.md")
    print()
    print("  포함 안 함:")
    print("    H) huggingface_credentials.txt - 보안!!!")
    print()
    print("  [security tip] 절대 commit 하지 말아야 할 것:")
    print("    - HuggingFace token")
    print("    - API keys (OpenAI / Google / etc.)")
    print("    - .env 파일")
    print("    - personal credentials")
    print("    - 사적 사진 (얼굴 등)")
    print()
    print("  표준 .gitignore:")
    print("    *.env")
    print("    credentials.json")
    print("    secrets/")
    print("    .git/")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
