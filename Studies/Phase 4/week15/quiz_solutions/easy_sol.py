"""
Phase 4 Week 15 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) 3~5초")
    print("=" * 50 + "\n")
    print("해설:")
    print("  인트로의 trade-off:")
    print("    너무 짧음 (A 1초): 정보 부족, '갑작스러움'")
    print("    적정 (B 3~5초): 제목 + 본인 정보 표시 가능")
    print("    너무 김 (C 10~15초): 본문 도달 전 면접관이 떠남")
    print()
    print("  3~5초 안에:")
    print("    - 제목 (1초)")
    print("    - 본인 정보 (2초)")
    print("    - fade out (1초)")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) 15자")
    print("=" * 50 + "\n")
    print("해설:")
    print("  자막 한 줄 글자 수 기준:")
    print("    너무 짧음 (A 5자): 정보 부족, 너무 자주 바뀜")
    print("    적정 (B 15자): 1~2초 안에 읽기 + 의미 전달")
    print("    너무 김 (C 30자): 못 읽음, 화면 가림")
    print()
    print("  Netflix 자막 기준: 한국어 15자/줄, 2줄 (30자) 까지.")
    print()
    print("  본 phase 권장: 1줄 15자 최대.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) 1280x720 (16:9)")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Thumbnail 표준:")
    print("    YouTube: 1280x720 (HD), 16:9 비율, < 2MB")
    print("    LinkedIn: 1200x627 추천 (가깝게)")
    print("    Twitter: 1200x675")
    print()
    print("  대부분 1280x720 한 가지로 호환 가능 (자동 crop).")
    print()
    print("  1920x1080 (C) 는 더 크지만 thumbnail 으로 너무 큼.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) < 5 MB")
    print("=" * 50 + "\n")
    print("해설:")
    print("  GIF 의 trade-off:")
    print("    너무 작음 (A < 100KB): 화질 또는 길이 부족")
    print("    적정 (B < 5MB): 5~10초, 화질 적당")
    print("    너무 큼 (C < 50MB): GitHub 로딩 느림")
    print()
    print("  최적화 팁:")
    print("    - fps 10~15 (영상의 30 보다 낮춤)")
    print("    - width 640 정도 (1920 -> 640)")
    print("    - palette 사용 (ffmpeg 의 palettegen)")
    print("    - 길이 5~10초")
    print()
    print("  GIF 대신 mp4 도 README 의 video tag 로 가능.")
    print("  단 GIF 가 더 자동 재생 / 호환성 좋음.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
