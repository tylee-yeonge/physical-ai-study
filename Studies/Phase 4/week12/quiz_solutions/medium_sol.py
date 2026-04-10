"""
Phase 6 Week 12 - 블로그 & 영상 중급 퀴즈 풀이
"""


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 풀이: 블로그 아웃라인 작성")
    print("━" * 36 + "\n")

    print("  제목:")
    print("  '카메라 한 대로 3D 물체를 잡는다?")
    print("   — KITTI Monocular 3D Detection 도전기'")
    print()

    outline = [
        {
            "section": "1. 도입 (왜 이 글을 읽어야 하는가)",
            "content": "2D Detection의 한계 → 3D Detection의 필요성 제시",
            "length": "200~300자",
        },
        {
            "section": "2. 배경 지식 (3D Detection 기초)",
            "content": "3D BBox 파라미터 7개, Monocular vs LiDAR 비교",
            "length": "300~400자",
        },
        {
            "section": "3. KITTI 데이터셋 소개",
            "content": "데이터 구조, 레이블 형식, 좌표계 설명",
            "length": "300~400자",
        },
        {
            "section": "4. FCOS3D로 학습하기",
            "content": "모델 선택 이유, 학습 설정, 핵심 코드 스니펫",
            "length": "400~500자",
        },
        {
            "section": "5. 결과 및 분석",
            "content": "AP3D 결과 표, 시각화 이미지, 오류 분석",
            "length": "300~400자",
        },
        {
            "section": "6. 배운 점 & 다음 단계",
            "content": "핵심 인사이트 3가지, 다음 글 예고",
            "length": "200~300자",
        },
    ]

    for item in outline:
        print(f"  {item['section']}")
        print(f"    내용: {item['content']}")
        print(f"    분량: {item['length']}")
        print()

    print("  시각 자료 목록:")
    visuals = [
        "2D vs 3D Detection 비교 다이어그램",
        "3D BBox 파라미터 시각화",
        "KITTI 데이터 예시 이미지",
        "KITTI 좌표계 다이어그램",
        "학습 Loss 커브 그래프",
        "3D Detection 결과 시각화 (성공 + 실패 사례)",
        "AP3D 결과 표",
    ]
    for v in visuals:
        print(f"    - {v}")

    print()
    print("  총 분량: 약 1,800~2,300자 + 이미지 7장")
    print("  예상 읽기 시간: 7~10분")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 풀이: 영상 시간 배분 설계")
    print("━" * 36 + "\n")

    sections = [
        {
            "name": "인트로",
            "seconds": 30,
            "screen": "프로젝트 제목 + 결과 이미지 몽타주",
            "message": "프로젝트 한 줄 소개, 영상 목차 안내",
        },
        {
            "name": "문제 정의",
            "seconds": 30,
            "screen": "2D vs 3D Detection 비교 슬라이드",
            "message": "왜 3D Detection이 필요한가",
        },
        {
            "name": "KITTI 3D Detection",
            "seconds": 120,
            "screen": "KITTI 데이터 → 터미널(학습) → 결과 시각화",
            "message": "데이터셋 소개, 학습 과정, AP3D 13.87% 결과",
        },
        {
            "name": "nuScenes & BEV",
            "seconds": 90,
            "screen": "6대 카메라 → BEV 시각화 → Feature Map",
            "message": "Multi-view 카메라, BEV 변환, NDS 0.517 결과",
        },
        {
            "name": "기술적 인사이트",
            "seconds": 60,
            "screen": "핵심 포인트 3가지 슬라이드",
            "message": "Depth 모호성, BEV 장점, 개선 방향",
        },
        {
            "name": "마무리",
            "seconds": 30,
            "screen": "GitHub 링크 + QR 코드",
            "message": "코드 공개, 피드백 요청, 감사 인사",
        },
    ]

    total = 0
    for i, s in enumerate(sections, 1):
        total += s['seconds']
        mins = s['seconds'] // 60
        secs = s['seconds'] % 60
        time_str = f"{mins}분 {secs}초" if mins > 0 else f"{secs}초"

        print(f"  섹션 {i}: {s['name']} ({time_str})")
        print(f"    화면: {s['screen']}")
        print(f"    메시지: {s['message']}")
        print()

    print(f"  합계: {total}초 ({total // 60}분)")
    print()

    print("  시간 배분 비율:")
    for s in sections:
        ratio = s['seconds'] / total * 100
        bar = "█" * int(ratio / 2)
        print(f"    {s['name']:12s}: {bar} {ratio:.0f}%")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 풀이: 면접 답변 작성")
    print("━" * 36 + "\n")

    answers = [
        {
            "question": "3D Detection 프로젝트에서 가장 어려웠던 점은?",
            "S": "KITTI 데이터셋으로 카메라 기반 Monocular 3D Detection을 구현하는 프로젝트를 진행했습니다.",
            "T": "FCOS3D 모델로 AP3D(Moderate) 15% 이상을 달성하는 것이 목표였습니다.",
            "A": "가장 큰 어려움은 Depth 추정의 모호성이었습니다. "
                 "작은 차량이 가까이 있는 것과 큰 차량이 멀리 있는 것이 "
                 "이미지에서 비슷하게 보여서, Depth 오차가 3D 위치 오차로 직결되었습니다. "
                 "이를 해결하기 위해 Depth loss weight를 조정하고, "
                 "log-space depth regression을 적용했습니다.",
            "R": "AP3D(Moderate) 13.87%를 달성했고, "
                 "원거리(>30m) 객체의 Depth 오차가 핵심 병목임을 파악했습니다. "
                 "이 경험으로 BEV 기반 Multi-view 접근의 필요성을 이해하게 되었습니다.",
        },
        {
            "question": "BEV 표현의 장점을 3가지 설명하세요.",
            "S": "nuScenes 데이터셋에서 BEVFormer를 활용한 Multi-view 3D Detection을 실습했습니다.",
            "T": "BEV 표현이 기존 Camera view 기반 접근 대비 어떤 장점이 있는지 분석하는 것이 목표였습니다.",
            "A": "BEV의 핵심 장점 3가지를 분석했습니다. "
                 "첫째, Multi-camera 융합이 자연스럽습니다. 6대 카메라의 특징을 하나의 BEV 공간에서 합칠 수 있습니다. "
                 "둘째, Occlusion에 강건합니다. 위에서 보는 시점이므로 가려진 부분도 표현 가능합니다. "
                 "셋째, 경로 계획과 직접 연결됩니다. BEV 공간은 주행 경로와 같은 좌표계입니다.",
            "R": "BEVFormer는 NDS 0.517을 달성했고, 이는 Monocular 대비 "
                 "Multi-view BEV 접근이 성능과 실용성 면에서 우월함을 보여줍니다.",
        },
        {
            "question": "이 프로젝트 경험을 실무에 어떻게 적용하시겠습니까?",
            "S": "6개월간 KITTI와 nuScenes에서 Camera 기반 3D Detection을 학습했습니다.",
            "T": "이 경험을 AMR이나 자율주행 시스템 개발에 적용하는 방법을 고민했습니다.",
            "A": "실무 적용 시 3가지를 고려하겠습니다. "
                 "첫째, Jetson 등 임베디드 환경에서의 TensorRT 최적화로 실시간성을 확보합니다. "
                 "둘째, 카메라 캘리브레이션과 좌표계 변환을 정확히 구현합니다. "
                 "셋째, Depth 추정의 불확실성을 고려한 안전 마진을 설계합니다.",
            "R": "이 프로젝트를 통해 3D Detection의 전체 파이프라인(데이터→학습→평가→최적화)을 "
                 "경험했고, 특히 Depth 추정과 BEV 변환의 실무적 중요성을 체감했습니다.",
        },
    ]

    for i, a in enumerate(answers, 1):
        print(f"  Q{i}. {a['question']}")
        print(f"  ─────────────────────────────")
        print(f"    S: {a['S']}")
        print(f"    T: {a['T']}")
        print(f"    A: {a['A']}")
        print(f"    R: {a['R']}")
        print()

    print("  답변 작성 팁:")
    print("    1. 구체적 수치를 반드시 포함 (AP3D, NDS, 거리 등)")
    print("    2. 어려웠던 점 → 해결 과정이 가장 인상적")
    print("    3. 각 답변을 소리 내어 2분 이내에 말할 수 있어야 함")
    print("    4. 질문에 직접적으로 답한 후 보충 설명 추가")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 12 Quiz Medium - 풀이")
    print("━" * 40)

    problem1_solution()
    problem2_solution()
    problem3_solution()

    print("\n" + "━" * 40)
