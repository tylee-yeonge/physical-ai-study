"""
Phase 4 Week 14 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) Title + One-liner + Demo 영상")
    print("=" * 50 + "\n")
    print("해설:")
    print("  면접관의 30초 노출 우선순위:")
    print("    1. Title - '이 프로젝트가 무엇인가' 첫 인식")
    print("    2. One-liner (한 줄 요약) - 가치 판단")
    print("    3. Demo 영상 (있으면) - 가장 강력한 첫 인상")
    print()
    print("  Demo 영상 / GIF 가 위쪽에 있으면 면접관 click 후 머무름.")
    print("  License (A) / Citation (C) 같은 메타는 끝부분에.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) 8 ~ 10 개")
    print("=" * 50 + "\n")
    print("해설:")
    print("  표준 README section:")
    print("    1. Title + One-liner")
    print("    2. Demo")
    print("    3. Quick Start")
    print("    4. Architecture")
    print("    5. Installation")
    print("    6. Usage")
    print("    7. Results")
    print("    8. Troubleshooting")
    print("    9. Limitations")
    print("   10. License")
    print()
    print("  너무 적으면 (A): 정보 부족")
    print("  너무 많으면 (C): 스크롤 부담, 진입점 흐릿함")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: A) 즉시 확인 + 디버깅 쉬움")
    print("=" * 50 + "\n")
    print("해설:")
    print("  검증 명령의 가치:")
    print("    1. 사용자가 그 step 의 성공 여부를 즉시 확인")
    print("    2. 실패 시 그 step 만 다시 (앞 step 영향 X)")
    print("    3. README 자체가 'self-validating'")
    print()
    print("  예시:")
    print("    Step: ROS2 설치")
    print("    검증: `ros2 --version`  -> 'humble' 이 출력되면 OK")
    print()
    print("    Step: pip install torch")
    print("    검증: `python -c 'import torch; print(torch.cuda.is_available())'`")
    print("    expected: True")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) Llama 2 license (OpenVLA weights)")
    print("=" * 50 + "\n")
    print("해설:")
    print("  OpenVLA 의 license 구조:")
    print("    - 코드 (training / inference script): MIT")
    print("    - Model weight (Llama 2 기반): Llama 2 license")
    print()
    print("  Llama 2 license 의 주의:")
    print("    - 비상업적 사용: 자유")
    print("    - 상업적 사용: Meta 의 별도 license 필요할 수 있음 (월간 활성 사용자 700M 이상)")
    print("    - 본 phase 의 minimal demo 는 비상업적 -> 문제 없음")
    print()
    print("  면접관 (기업) 입장에서 가장 중요한 정보:")
    print("    '이 모델을 우리 제품에 그대로 쓸 수 있는가'")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
