"""Phase 5 Week 6 - 중급 정답"""


def p1():
    total = 30 + 5 + 1
    print(f"\n정답: CLIP ROS2 latency = {total} ms")
    print(f"  inference 30 + cv_bridge 5 + publish 1 = {total} ms")
    print(f"  Throughput: 27 Hz")
    print(f"  -> 30 Hz 카메라 image 와 동일 속도 가능")


def p2():
    total = 5.3 + 1.2 + 1.0
    print(f"\n정답: OpenVLA + CLIP VRAM = {total} GB")
    print(f"  OpenVLA int4: 5.3")
    print(f"  CLIP fp16:    1.2")
    print(f"  overhead:     1.0")
    print(f"  Total:        {total} GB")
    print(f"  RTX 4070 12GB fit: True (3.5 GB 여유)")


def p3():
    print("\n정답: B) split-screen 60s 유지")
    print("  영상 길이 늘이지 말 것:")
    print("    - 면접관의 attention span 제한")
    print("    - 60 초가 sweet spot")
    print()
    print("  split-screen 으로:")
    print("    좌측: CLIP top class")
    print("    우측: OpenVLA action")
    print("    한 화면에 두 정보 동시")


if __name__ == "__main__":
    p1(); p2(); p3()
