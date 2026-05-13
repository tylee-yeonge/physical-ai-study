"""
Phase 4 Week 15 - 중급 퀴즈
"""


def problem1_estimate_bitrate():
    """
    문제 1: 영상 크기 추정

    1분 (60초) 영상, 1920x1080, 30fps, H.264.
    bitrate 2 Mbps 일 때 파일 크기 추정?

    공식: file_size = bitrate * duration / 8 (bits -> bytes)

    TODO: file_size_mb 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 1: 영상 파일 크기 추정")
    print("=" * 60 + "\n")

    bitrate_mbps = 2  # Mbps (video)
    audio_mbps = 0.128  # 128 kbps = 0.128 Mbps
    duration_s = 60

    # TODO
    file_size_mb = 0.0

    expected = (bitrate_mbps + audio_mbps) * duration_s / 8

    print(f"  당신의 답: {file_size_mb:.2f} MB")
    print(f"  기대 답  : {expected:.2f} MB")
    print()
    print("  계산:")
    print(f"    total bitrate = video + audio = {bitrate_mbps + audio_mbps:.3f} Mbps")
    print(f"    file size = {bitrate_mbps + audio_mbps:.3f} Mbps * {duration_s}s / 8 = {expected:.2f} MB")


def problem2_subtitle_segments():
    """
    문제 2: 자막 segment 의 timing 설계

    60초 영상, 다음 8 가지 정보를 자막에 표시해야 한다:
    a) 인트로 메시지
    b) 본인 정보
    c) System 구조 1
    d) System 구조 2
    e) 실시간 동작 메시지
    f) Latency 수치
    g) 양산 의미
    h) 다음 산출물

    각 자막의 시작 / 끝 시간을 설계 (15자 한국어 기준, 2초+ 유지).

    TODO: segments 시간 채우기.
    """
    print("\n" + "=" * 60)
    print("문제 2: 자막 timing 설계")
    print("=" * 60 + "\n")

    # TODO
    segments = [
        ("a", "인트로", 0, 0),       # start, end
        ("b", "본인", 0, 0),
        ("c", "System 1", 0, 0),
        ("d", "System 2", 0, 0),
        ("e", "실시간", 0, 0),
        ("f", "Latency", 0, 0),
        ("g", "양산 의미", 0, 0),
        ("h", "다음", 0, 0),
    ]

    expected = [
        ("a", "인트로", 0, 5),
        ("b", "본인", 5, 10),
        ("c", "System 1", 10, 18),
        ("d", "System 2", 18, 25),
        ("e", "실시간", 25, 45),
        ("f", "Latency", 45, 52),
        ("g", "양산 의미", 52, 56),
        ("h", "다음", 56, 60),
    ]

    print("  당신의 segment 들:")
    for code, name, s, e in segments:
        dur = e - s
        print(f"    {code}) {name:<10} {s:>3}s -> {e:>3}s ({dur}s)")
    print("\n  기대 답:")
    for code, name, s, e in expected:
        dur = e - s
        print(f"    {code}) {name:<10} {s:>3}s -> {e:>3}s ({dur}s)")


def problem3_thumbnail_design():
    """
    문제 3: thumbnail 의 핵심 단어 3 개

    thumbnail 의 큰 폰트 (60pt+) 로 표시할 핵심 단어 3 개를 선택.

    아래 후보에서 우선순위:
      A) OpenVLA
      B) ROS2
      C) RTX 4070
      D) 4-bit nf4
      E) 165 ms / 6 Hz
      F) AMR / 펌웨어
      G) Vision-Language-Action
      H) PaLI-X

    TODO: top 3 를 우선순위 순으로.
    """
    print("\n" + "=" * 60)
    print("문제 3: thumbnail 핵심 단어 top 3")
    print("=" * 60 + "\n")

    # TODO (우선순위 순)
    top3 = ["", "", ""]

    expected = ["A", "B", "E"]  # OpenVLA, ROS2, latency

    print(f"  당신의 답 : {top3}")
    print(f"  기대 답  : {expected}")
    print()
    print("  이유:")
    print("    A) OpenVLA  - 가장 중요한 모델명")
    print("    B) ROS2     - 본인의 차별화 (양산 SW)")
    print("    E) 165ms/6Hz - '직접 측정' 의 증거")
    print()
    print("  G) Vision-Language-Action 은 길어서 thumbnail 에 안 들어감.")
    print("  F) AMR 은 본인 정보 (작은 폰트로 별도).")


if __name__ == "__main__":
    print("=" * 60)
    problem1_estimate_bitrate()
    problem2_subtitle_segments()
    problem3_thumbnail_design()
    print("=" * 60)
