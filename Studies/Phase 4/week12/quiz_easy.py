"""
Phase 4 Week 12 - Rerun + 영상 기초 퀴즈
"""


def problem1_rerun_value():
    print("\n" + "=" * 50)
    print("문제 1: Rerun 의 가치")
    print("=" * 50 + "\n")
    print("질문: 본 phase 에서 Rerun 이 rqt_image_view 보다 유리한 이유?\n")
    print("보기:")
    print("  A) 더 빠른 image display")
    print("  B) 시간 동기화된 multi-modal (image + action + latency) 한 화면")
    print("  C) GPU 사용 안 함")
    print("  D) C++ 으로 작성됨")


def problem2_video_length():
    print("\n" + "=" * 50)
    print("문제 2: 데모 영상의 권장 길이")
    print("=" * 50 + "\n")
    print("질문: 본 phase 의 데모 영상의 권장 길이?\n")
    print("보기:")
    print("  A) 10~20 초 (짧고 강력)")
    print("  B) 1 분 (적정)")
    print("  C) 5~10 분 (자세함)")
    print("  D) 20~30 분 (튜토리얼)")


def problem3_subtitle_lang():
    print("\n" + "=" * 50)
    print("문제 3: 자막 언어")
    print("=" * 50 + "\n")
    print("질문: 본 로드맵의 1순위 타깃을 고려한 영상 자막 언어 우선순위?\n")
    print("보기:")
    print("  A) 영어 (글로벌)")
    print("  B) 한국어 (1순위 회사 한국 기반)")
    print("  C) 일본어")
    print("  D) 중국어")


def problem4_github_size():
    print("\n" + "=" * 50)
    print("문제 4: GitHub 영상 파일 크기 제한")
    print("=" * 50 + "\n")
    print("질문: GitHub 에 영상 파일 직접 호스팅 시 권장 크기?\n")
    print("보기:")
    print("  A) < 5 MB (작게)")
    print("  B) < 50 MB (권장, file size limit 100MB 내)")
    print("  C) < 500 MB")
    print("  D) GitHub 호스팅 불가")


if __name__ == "__main__":
    print("=" * 50)
    problem1_rerun_value()
    problem2_video_length()
    problem3_subtitle_lang()
    problem4_github_size()
    print("=" * 50)
