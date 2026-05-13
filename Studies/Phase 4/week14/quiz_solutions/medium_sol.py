"""
Phase 4 Week 14 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: section 순서 C,D,B,E,F,G,H,I,A")
    print("=" * 60 + "\n")
    order = [
        ("C", "Title + One-liner", "첫 인상"),
        ("D", "Demo 영상", "가장 강력한 첫 인상"),
        ("B", "Quick Start", "5분 안에 실행 - 진입점"),
        ("E", "Architecture", "구조 이해"),
        ("F", "Installation", "자세한 환경 세팅"),
        ("G", "Usage", "실행 방법"),
        ("H", "Results", "성능 지표"),
        ("I", "Troubleshooting", "자주 발생 + 해결"),
        ("A", "License", "법적 정보"),
    ]
    for i, (k, name, why) in enumerate(order, 1):
        print(f"  {i:2d}. ({k}) {name:<20} - {why}")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: 트러블슈팅 표 예시")
    print("=" * 60 + "\n")
    rows = [
        ("CUDA out of memory", "다른 GPU process", "nvidia-smi 후 kill"),
        ("flash_attn ImportError", "flash-attn 미설치", "attn_implementation='eager'"),
        ("Image topic 없음", "bag 재생 안 함", "ros2 bag play /path/to/bag"),
        ("inference 가 0 action", "unnormalize_key 잘못", "parameter 확인"),
        ("colcon build fail", "ROS2 source 누락", "source /opt/ros/humble/setup.bash"),
    ]
    print(f"  {'증상':<25}{'원인':<22}{'해결'}")
    print(f"  {'-'*25}{'-'*22}{'-'*40}")
    for s, c, r in rows:
        print(f"  {s:<25}{c:<22}{r}")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: Quick Start 시간")
    print("=" * 60 + "\n")
    times = [
        ("git clone", 1),
        ("ROS2 apt install", 10),
        ("conda + pip", 5),
        ("colcon build", 2),
        ("OpenVLA download (15GB)", 15),
        ("첫 inference", 1),
    ]
    total = 0
    for k, m in times:
        total += m
        print(f"  {k:<35} {m:>3} 분")
    print(f"  {'-'*40} {'-'*5}")
    print(f"  {'Total':<35} {total:>3} 분")
    print()
    print("  [tip] 30~40 분이 'Quick Start' 의 합리적 상한.")
    print("       그 이상이면 Docker / pre-downloaded model 권장.")
    print()
    print("  ROS2 apt install (10분) + OpenVLA download (15분) 이 가장 큼.")
    print("  Docker image 미리 빌드해 놓으면 5분 안으로 단축 가능.")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
