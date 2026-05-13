"""
Phase 5 Week 12 - 최종 데모 & 포트폴리오 기초 퀴즈 풀이
"""


def problem1_solution():
    print("\n" + "━" * 28)
    print("문제 1 풀이: 파이프라인 병목")
    print("━" * 28 + "\n")

    print("  정답: B) Depth 추론 (55ms)\n")
    print("  전체 시간 분포:")
    print("    YOLO 추론:    30ms (28.6%)")
    print("    Depth 추론:   55ms (52.4%)  ← 병목!")
    print("    융합+역투영:    2ms ( 1.9%)")
    print()
    print("  → Depth 추론이 전체의 절반 이상을 차지")
    print("  → 이 부분을 최적화해야 전체 FPS 향상")
    print("  → 해결: 모델 경량화, 해상도↓, INT8, 병렬 실행")


def problem2_solution():
    print("\n" + "━" * 28)
    print("문제 2 풀이: 최적화 전략")
    print("━" * 28 + "\n")

    print("  정답: B) 캘리브레이션 데이터 (대표 입력 이미지)\n")
    print("  INT8 양자화 과정:")
    print("    1. 대표적인 입력 이미지 100~1000장 준비")
    print("    2. TensorRT가 각 레이어의 값 범위 분석")
    print("    3. FP32/FP16 → INT8 매핑 테이블 생성")
    print("    4. 캘리브레이션 없으면 정확도 심각하게 하락")
    print()
    print("  명령어:")
    print("    trtexec --onnx=model.onnx --int8 --calib=cal_data/")
    print()
    print("  주의:")
    print("    → 캘리브레이션 데이터는 실제 운용 환경과 유사해야 함")
    print("    → 변환 후 반드시 정확도 재검증 필요")


def problem3_solution():
    print("\n" + "━" * 28)
    print("문제 3 풀이: 병렬 실행 효과")
    print("━" * 28 + "\n")

    print("  정답: B) 55ms (둘 중 긴 쪽)\n")
    print("  병렬 실행 원리:")
    print("    YOLO(30ms)  ━━━━━━━━━━━━━━━|")
    print("    Depth(55ms) ━━━━━━━━━━━━━━━━━━━━━━━━━|")
    print("                                          └ 55ms 후 완료")
    print()
    print("  → 두 작업이 동시 실행되므로 max(30, 55) = 55ms")
    print("  → 순차(85ms)보다 30ms 절약 → 약 1.55x 속도 향상")
    print()
    print("  구현 방법:")
    print("    1. CUDA 스트림 분리 (C++ 레벨)")
    print("    2. ThreadPoolExecutor (Python)")
    print("    3. asyncio + 비동기 CUDA 호출")


def problem4_solution():
    print("\n" + "━" * 28)
    print("문제 4 풀이: 포트폴리오 핵심")
    print("━" * 28 + "\n")

    print("  정답: B) 데모 영상 + 정량적 성능 수치\n")
    print("  포트폴리오 임팩트 순서:")
    print("    1위: 데모 영상 (실제 동작을 눈으로 확인)")
    print("    2위: 성능 수치 (FPS, mAP, AbsRel 등)")
    print("    3위: 시스템 아키텍처 다이어그램")
    print("    4위: 깔끔한 코드 구조")
    print()
    print("  → 코드 줄 수, 라이브러리 개수는 중요하지 않음")
    print("  → 문서 분량보다 실제 결과가 중요")
    print("  → '무엇을 만들었고, 얼마나 잘 되는가'가 핵심")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 12 Quiz Easy - 풀이")
    print("━" * 33)

    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()

    print("\n" + "━" * 33)
