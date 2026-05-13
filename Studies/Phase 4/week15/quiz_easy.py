"""
Phase 4 Week 15 - 영상 마감 기초 퀴즈
"""


def problem1_intro_length():
    print("\n" + "=" * 50)
    print("문제 1: 인트로 권장 길이")
    print("=" * 50 + "\n")
    print("질문: 1분 영상의 인트로 권장 길이?\n")
    print("보기:")
    print("  A) 1초 (최소)")
    print("  B) 3~5초")
    print("  C) 10~15초")
    print("  D) 20초 이상")


def problem2_subtitle_chars():
    print("\n" + "=" * 50)
    print("문제 2: 자막 한 줄 글자 수")
    print("=" * 50 + "\n")
    print("질문: 영상 자막의 한국어 한 줄 권장 글자 수?\n")
    print("보기:")
    print("  A) 5자 (매우 짧게)")
    print("  B) 15자 (가독성 + 정보량 균형)")
    print("  C) 30자")
    print("  D) 50자")


def problem3_thumbnail_size():
    print("\n" + "=" * 50)
    print("문제 3: thumbnail 표준 크기")
    print("=" * 50 + "\n")
    print("질문: YouTube / LinkedIn 영상 thumbnail 의 표준 해상도?\n")
    print("보기:")
    print("  A) 320x240")
    print("  B) 1280x720 (16:9)")
    print("  C) 1920x1080")
    print("  D) 800x600")


def problem4_gif_size():
    print("\n" + "=" * 50)
    print("문제 4: README 의 demo.gif 권장 크기")
    print("=" * 50 + "\n")
    print("질문: README 의 demo.gif 의 권장 파일 크기?\n")
    print("보기:")
    print("  A) < 100 KB (매우 작게)")
    print("  B) < 5 MB (적정)")
    print("  C) < 50 MB")
    print("  D) GIF 보다 mp4 만 사용")


if __name__ == "__main__":
    print("=" * 50)
    problem1_intro_length()
    problem2_subtitle_chars()
    problem3_thumbnail_size()
    problem4_gif_size()
    print("=" * 50)
