"""
Phase 4 Week 14 - README 작성 기초 퀴즈
"""


def problem1_readme_first():
    print("\n" + "=" * 50)
    print("문제 1: README 의 첫 30초 노출 항목")
    print("=" * 50 + "\n")
    print("질문: 면접관이 README 첫 30초에 보는 항목은?\n")
    print("보기:")
    print("  A) License section")
    print("  B) Title + One-liner + Demo 영상")
    print("  C) Citation")
    print("  D) Architecture diagram")


def problem2_readme_section_count():
    print("\n" + "=" * 50)
    print("문제 2: README 의 권장 section 수")
    print("=" * 50 + "\n")
    print("질문: 본 phase 권장 README 의 section 개수는?\n")
    print("보기:")
    print("  A) 3 개 (간결)")
    print("  B) 8 ~ 10 개 (균형)")
    print("  C) 20 개 (자세함)")
    print("  D) 1 개 (one-page)")


def problem3_verify_step():
    print("\n" + "=" * 50)
    print("문제 3: 각 step 의 검증 명령")
    print("=" * 50 + "\n")
    print("질문: README 의 환경 세팅 step 마다 검증 명령을 두는 이유?\n")
    print("보기:")
    print("  A) 사용자가 그 step 이 성공했는지 즉시 확인 -> 디버깅 쉬움")
    print("  B) 페이지가 길어 보임")
    print("  C) SEO 효과")
    print("  D) 권장사항이지만 필수 아님")


def problem4_license_priority():
    print("\n" + "=" * 50)
    print("문제 4: License section 의 가장 중요한 내용")
    print("=" * 50 + "\n")
    print("질문: OpenVLA 기반 프로젝트의 License section 에서 가장 중요한 한 줄?\n")
    print("보기:")
    print("  A) MIT License")
    print("  B) Llama 2 license (OpenVLA weights, commercial use 시 확인)")
    print("  C) Apache 2.0")
    print("  D) BSD")


if __name__ == "__main__":
    print("=" * 50)
    problem1_readme_first()
    problem2_readme_section_count()
    problem3_verify_step()
    problem4_license_priority()
    print("=" * 50)
