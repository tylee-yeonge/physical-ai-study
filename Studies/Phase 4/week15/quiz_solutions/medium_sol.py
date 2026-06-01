"""
Phase 4 Week 15 - 중급 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 60)
    print("문제 1 정답: 영상 파일 크기")
    print("=" * 60 + "\n")
    bitrate = 2 + 0.128
    duration = 60
    size = bitrate * duration / 8

    print(f"  bitrate (video + audio) = 2 + 0.128 = {bitrate:.3f} Mbps")
    print(f"  duration = {duration} s")
    print(f"  size = {bitrate:.3f} Mbps * {duration} s / 8 bit/byte")
    print(f"       = {size:.2f} MB")
    print()
    print("  [tip] 실제로 H.264 의 가변 bitrate (VBR) 라 ~ 10~20% 차이.")
    print("       Mean ~ 16 MB, max ~ 20 MB 정도.")
    print()
    print("  GitHub < 30 MB 권장 안에 들어옴 -> 직접 호스팅 OK.")


def problem2_solution():
    print("\n" + "=" * 60)
    print("문제 2 정답: 자막 timing")
    print("=" * 60 + "\n")
    segments = [
        ("a", "인트로",       0,  5,  "5s - 인트로 카드"),
        ("b", "본인 정보",    5, 10,  "5s - AMR ROS 5년"),
        ("c", "System 1",   10, 18,  "8s - ros2 node 다이어그램"),
        ("d", "System 2",   18, 25,  "7s - 자세한 흐름"),
        ("e", "실시간 동작", 25, 45, "20s - Rerun 화면, 가장 긴 segment"),
        ("f", "Latency",   45, 52,  "7s - 수치 표"),
        ("g", "양산 의미",  52, 56,  "4s - hierarchical 메시지"),
        ("h", "다음 산출물", 56, 60,  "4s - Phase 7 예고"),
    ]
    for code, name, s, e, note in segments:
        print(f"  {code}) {name:<13} {s:>3}s -> {e:>3}s | {note}")
    print()
    print(f"  Total: 60 s")
    print()
    print("  [tip] 'e' (실시간 동작) 가 20s 로 가장 긴 이유:")
    print("       이게 영상의 핵심. 충분한 시간을 들여 demo 보여주기.")


def problem3_solution():
    print("\n" + "=" * 60)
    print("문제 3 정답: thumbnail top 3 - A, B, E")
    print("=" * 60 + "\n")
    print("  1. A) OpenVLA - 가장 중요한 모델명")
    print("  2. B) ROS2 - 본인의 차별화 (양산 SW 통합)")
    print("  3. E) 165 ms / 6 Hz - '직접 측정' 의 정량 증거")
    print()
    print("  thumbnail 의 황금 패턴: '모델 + 기술 + 수치'")
    print("    -> 면접관이 0.5초 안에 '실측 데이터' 인식")
    print()
    print("  후보 분석:")
    print("    A) OpenVLA   - thumbnail top: '뭐 하는 글?'")
    print("    B) ROS2      - '실로봇 통합' 임을 즉시 인식")
    print("    C) RTX 4070  - 너무 specific, top 자리에는 부적합")
    print("    D) 4-bit nf4 - 기술적, 작은 폰트로 sub")
    print("    E) 165ms/6Hz - '실측' 의 가장 강한 증거")
    print("    F) AMR/펌웨어 - 본인 정보, 작은 폰트로 별도")
    print("    G) Vision-Language-Action - 너무 김")
    print("    H) PaLI-X    - RT-2 의 backbone, OpenVLA 와 무관")


if __name__ == "__main__":
    print("=" * 60)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("=" * 60)
