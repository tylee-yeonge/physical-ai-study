"""
Phase 4 Week 12 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) multi-modal 한 화면")
    print("=" * 50 + "\n")
    print("해설:")
    print("  rqt_image_view: image 만 보여줌")
    print("  rqt_plot      : scalar 만 보여줌")
    print("  -> 각각 별도 창, 시간 동기화 어려움")
    print()
    print("  Rerun:")
    print("    - image, scalar, text, 3D arrow, point cloud 한 화면")
    print("    - 시간 축 따라 모두 동기화")
    print("    - 영상 녹화 시 한 번의 화면 캡처로 끝")
    print()
    print("  [tip] 본 phase 의 영상은 Rerun 화면 그대로 녹화.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) 1 분")
    print("=" * 50 + "\n")
    print("해설:")
    print("  영상 길이 별 trade-off:")
    print("    10~20s (A): 깊이 부족, 'screenshot 같음'")
    print("    1 분  (B): 핵심 흐름 + 본인 차별화 메시지 모두 담음")
    print("    5~10분 (C): 면접관이 다 안 봄, '튜토리얼 같음'")
    print()
    print("  1분의 권장 구조:")
    print("    Intro 10s + System 15s + 실시간 20s + 결과 10s + Next 5s = 60s")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) 한국어")
    print("=" * 50 + "\n")
    print("해설:")
    print("  본 로드맵의 1순위 타깃 회사:")
    print("    - 마음AI (한국 양산 자율주행 회사)")
    print("    - 카카오모빌리티 VLA 팀")
    print("    - 신생 휴머노이드 스타트업 (대부분 한국 기반)")
    print()
    print("  한국 면접관이 한국어 자막 영상 + 한국어 블로그 보면 신뢰감 ↑.")
    print()
    print("  글로벌 확장 (영어 자막 추가) 도 가능, 단 1순위 한국어.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) < 50 MB")
    print("=" * 50 + "\n")
    print("해설:")
    print("  GitHub file size:")
    print("    - 100 MB: hard limit (push 거부)")
    print("    - 50 MB:  권장 한계 (LFS 없이)")
    print("    - >50MB : Git LFS 사용 권장")
    print()
    print("  1분 영상 (1280x720, 30fps, H.264) 의 일반적 크기:")
    print("    - bitrate ~5 Mbps -> 약 40 MB")
    print("    - bitrate ~2 Mbps -> 약 16 MB")
    print()
    print("  본 phase 권장: bitrate 2~3 Mbps, 30 MB 이하.")
    print("  GitHub 에 직접 push 후 README 의 video tag 로 embed.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
