"""Phase 5 Week 6 - 중급 퀴즈"""


def p1():
    """CLIP 의 ROS2 통합 latency 추정"""
    print("\n문제 1: CLIP ROS2 노드의 latency")
    print("  CLIP-L/14 inference ~ 30ms (RTX 4070)")
    print("  + cv_bridge 변환 ~ 5ms")
    print("  + ROS topic publish ~ 1ms")
    # TODO
    total = 0
    print(f"  당신: {total} ms")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """OpenVLA + CLIP 동시 실행 시 VRAM"""
    print("\n문제 2: OpenVLA + CLIP 동시 실행 VRAM")
    openvla = 5.3  # int4 inference
    clip = 1.2     # CLIP-L
    overhead = 1.0
    # TODO
    total = 0.0
    print(f"  당신: {total:.2f} GB")
    print(f"  RTX 4070 12GB 에 fit?: {total < 12.0}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """Phase 4 demo 영상 보강 시 시간 추가"""
    print("\n문제 3: Phase 4 영상 보강")
    print("  원본 영상 60초")
    print("  CLIP 시연 추가 시 영상 길어짐 (~ 5초)")
    print()
    print("  방법 A: 별도 5초 추가 -> 65초")
    print("  방법 B: 0:25-0:35 부분 split-screen -> 60초 유지")
    print()
    print("  본 phase 권장: B (split-screen, 영상 길이 유지)")


if __name__ == "__main__":
    p1(); p2(); p3()
