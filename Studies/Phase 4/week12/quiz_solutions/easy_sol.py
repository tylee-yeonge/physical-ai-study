"""
Phase 6 Week 12 - 블로그 & 영상 기초 퀴즈 풀이
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 풀이: 블로그 구조")
    print("━" * 28 + "\n")

    print("  정답: B) 시각적 결과 이미지 + 핵심 코드 발췌 + 배운 점 정리\n")
    print("  좋은 기술 블로그의 핵심:")
    print("    1. 시각적 결과: 독자가 코드를 실행하지 않아도 결과를 볼 수 있음")
    print("    2. 핵심 코드: 전체 코드가 아닌 핵심 로직만 발췌")
    print("    3. 배운 점: 실패 경험 포함, 독자에게 실질적 도움")
    print()
    print("  오답 분석:")
    print("    A) 전체 코드를 넣으면 글이 장황해지고 가독성 저하")
    print("    C) 긴 글보다는 간결하고 핵심적인 글이 효과적")
    print("    D) 학술 논문 형식은 블로그 독자에게 부적절")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 풀이: Demo 영상 길이")
    print("━" * 28 + "\n")

    print("  정답: B) 5~7분 (핵심 내용 전달 + 집중력 유지)\n")
    print("  5~7분이 적정인 이유:")
    print("    - 5분 미만: 기술적 내용을 충분히 전달하기 어려움")
    print("    - 5~7분: 인트로(30초) + KITTI(2분) + BEV(2분)")
    print("              + 인사이트(1분) + 마무리(30초)")
    print("    - 10분 초과: 시청자 집중력 급격히 감소")
    print()
    print("  참고: YouTube 데이터에 따르면")
    print("    평균 시청 유지율은 5분 이후 급격히 하락합니다.")
    print("    기술 Demo는 핵심만 간결하게 보여주는 것이 효과적입니다.")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 풀이: LinkedIn 포스팅")
    print("━" * 28 + "\n")

    print("  정답: B) 피드에서 첫 2~3줄만 표시되므로, 클릭을 유도해야 하기 때문\n")
    print("  LinkedIn 피드 동작 방식:")
    print("    1. 피드에서는 포스팅의 첫 2~3줄만 보임")
    print("    2. '더 보기'를 눌러야 전체를 볼 수 있음")
    print("    3. 첫 줄이 흥미롭지 않으면 스크롤해서 넘어감")
    print()
    print("  효과적인 첫 줄(Hook) 예시:")
    print("    ✓ '카메라 한 대로 3D 물체 검출이 가능할까?'")
    print("    ✓ 'AP3D 15%를 달성하기까지의 도전기'")
    print("    ✗ '3D Detection 프로젝트를 완료했습니다.' (밋밋함)")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 풀이: 면접 답변 프레임워크")
    print("━" * 28 + "\n")

    print("  정답: B) Situation, Task, Action, Result\n")
    print("  STAR 프레임워크:")
    print("    S (Situation): 프로젝트 배경, 문제 상황")
    print("    T (Task):      내가 맡은 역할, 구체적 목표")
    print("    A (Action):    실제로 취한 행동, 기술적 결정")
    print("    R (Result):    정량적 결과 + 배운 점")
    print()
    print("  예시:")
    print("    S: KITTI에서 Monocular 3D Detection 프로젝트를 진행했습니다.")
    print("    T: FCOS3D로 AP3D 15% 이상 달성이 목표였습니다.")
    print("    A: Multi-scale training 적용, Depth loss weight 튜닝,")
    print("       원거리 객체 오류 분석을 수행했습니다.")
    print("    R: AP3D(Moderate) 13.87% 달성, Depth 추정이")
    print("       핵심 병목임을 파악했습니다.")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 12 Quiz Easy - 풀이")
    print("━" * 33)

    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()

    print("\n" + "━" * 33)
