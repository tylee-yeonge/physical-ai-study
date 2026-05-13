"""
Phase 4 Week 10 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: ament_python 구조")
    print("=" * 60 + "\n")
    print("  vla_node/")
    print("    package.xml        # A: 의존성, license, maintainer")
    print("    setup.py           # B: entry_points + install_requires")
    print("    setup.cfg          # C: build/develop 옵션")
    print("    resource/          # D: ament index marker")
    print("      vla_node         # 빈 파일 (패키지 이름)")
    print("    vla_node/          # E: 실제 Python 코드")
    print("      __init__.py")
    print("      vla_inference_node.py")
    print("    test/")
    print("      test_*.py")
    print()
    print("  [tip] 'vla_node' 가 두 번 나오는 이유:")
    print("    바깥 vla_node/ = 패키지 디렉토리")
    print("    안쪽 vla_node/ = Python module 디렉토리 (import 시 사용)")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: callback 순서 E,B,C,D,A")
    print("=" * 60 + "\n")
    steps = [
        ("E", "instruction 없으면 skip", "fail fast - 의미 없는 inference 방지"),
        ("B", "image age check", "오래된 image 로 inference 하면 outdated"),
        ("C", "preprocess BGR->RGB", "OpenVLA 입력 형식"),
        ("D", "inference predict", "VLAInference.predict()"),
        ("A", "action publish", "결과 발행"),
    ]
    for i, (k, name, why) in enumerate(steps, 1):
        print(f"  {i}. ({k}) {name}")
        print(f"     -> {why}")
    print()
    print("  [tip] 이 순서가 양산 SW 표준 (validate -> transform -> compute -> emit)")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: QoS")
    print("=" * 60 + "\n")
    qos = [
        ("image_raw", "BEST_EFFORT", 1, "drop 허용, 최신만"),
        ("instruction", "RELIABLE + transient_local", 1, "latch, 새 join 노드도 받음"),
        ("action", "RELIABLE", 10, "robot 제어, 모두 보존"),
    ]
    print(f"  {'Topic':<14}{'reliability':<28}{'depth':<6}{'reason'}")
    print(f"  {'-'*14}{'-'*28}{'-'*6}{'-'*40}")
    for q in qos:
        print(f"  {q[0]:<14}{q[1]:<28}{q[2]:<6}{q[3]}")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
