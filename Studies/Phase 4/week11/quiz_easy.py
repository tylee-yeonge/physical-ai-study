"""
Phase 6 Week 11 - 코드 및 문서 정리 기초 퀴즈
"""


def problem1_readme_essentials():
    print("\n" + "━" * 28)
    print("문제 1: README 필수 요소")
    print("━" * 28 + "\n")

    print("질문: 포트폴리오용 GitHub README에 반드시 포함해야 할")
    print("      요소로 가장 적절하지 않은 것은?\n")

    print("보기:")
    print("  A) 프로젝트 설명과 결과 이미지")
    print("  B) 설치 방법과 실행 예시")
    print("  C) 개발 일지와 일일 작업 로그")
    print("  D) 사용한 방법론과 참고 논문")


def problem2_gitignore():
    print("\n" + "━" * 28)
    print("문제 2: .gitignore 설정")
    print("━" * 28 + "\n")

    print("질문: Git 저장소에 포함하지 말아야 할 파일은?\n")

    print("보기:")
    print("  A) requirements.txt")
    print("  B) src/models/fcos3d_wrapper.py")
    print("  C) data/kitti/training/image_2/000000.png (대용량 데이터)")
    print("  D) README.md")


def problem3_docstring():
    print("\n" + "━" * 28)
    print("문제 3: Docstring 작성")
    print("━" * 28 + "\n")

    print("질문: 좋은 Docstring에 포함되어야 할 항목은?\n")

    print("보기:")
    print("  A) 함수의 Git 커밋 해시")
    print("  B) Args(인자 설명), Returns(반환값 설명), 간단한 사용 예시")
    print("  C) 함수를 작성한 날짜와 작성자 이름")
    print("  D) 함수의 실행 시간 벤치마크")


def problem4_commit_message():
    print("\n" + "━" * 28)
    print("문제 4: 커밋 메시지 규칙")
    print("━" * 28 + "\n")

    print("질문: 다음 중 가장 좋은 커밋 메시지는?\n")

    print("보기:")
    print("  A) 'update'")
    print("  B) 'fix bug'")
    print("  C) 'feat(visualization): 3D bbox BEV 시각화 함수 추가'")
    print("  D) '2024-01-15 작업'")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 11 Quiz - Easy")
    print("━" * 33)
    problem1_readme_essentials()
    problem2_gitignore()
    problem3_docstring()
    problem4_commit_message()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
