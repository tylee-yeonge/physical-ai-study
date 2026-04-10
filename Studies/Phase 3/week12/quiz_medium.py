"""
Phase 5 Week 12 - 최종 데모 & 포트폴리오 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_amdahl_law():
    """
    문제 1: Amdahl의 법칙으로 최적화 효과 계산

    전체 파이프라인 시간 분포:
      이미지 캡처:   5ms
      YOLO 전처리:   3ms
      YOLO 추론:    30ms  → 병렬화 가능
      YOLO 후처리:   2ms
      Depth 전처리:  3ms
      Depth 추론:   55ms  → 병렬화 가능
      Depth 후처리:  2ms
      융합 + 역투영:  2ms
      시각화:        3ms
      ─────────────────
      합계:        105ms

    TODO:
    1. YOLO와 Depth를 병렬 실행하면 전체 시간은?
    2. 추가로 Depth를 2배 빠르게(27.5ms) 최적화하면 전체 시간은?
    3. 각 경우의 FPS는?
    """
    print("\n" + "━" * 36)
    print("문제 1: Amdahl 법칙 최적화 계산")
    print("━" * 36 + "\n")

    # 각 단계별 시간 (ms)
    stages = {
        '이미지 캡처':    5,
        'YOLO 전처리':    3,
        'YOLO 추론':     30,
        'YOLO 후처리':    2,
        'Depth 전처리':   3,
        'Depth 추론':    55,
        'Depth 후처리':   2,
        '융합+역투영':     2,
        '시각화':         3,
    }

    total = sum(stages.values())
    print(f"  순차 실행 총 시간: {total}ms ({1000/total:.1f} FPS)")
    print()

    print("  과제:")
    print("  1) YOLO 추론과 Depth 추론을 병렬 실행하면 총 시간은?")
    print("  2) 추가로 Depth 추론을 2배 빠르게(27.5ms) 하면 총 시간은?")
    print("  3) 각 경우의 FPS를 계산하시오.")
    print()

    # TODO: 학생이 계산
    print("  병렬 실행 시 총 시간: ___ ms (___ FPS)")
    print("  Depth 2x 최적화 시:  ___ ms (___ FPS)")


def problem2_tradeoff_analysis():
    """
    문제 2: 해상도-정확도-속도 트레이드오프

    3가지 설정의 성능 데이터가 주어졌을 때,
    AMR 자율주행 용도에 가장 적합한 설정을 선택하시오.

    요구사항:
    - FPS >= 10 (안전을 위한 실시간성)
    - Depth AbsRel <= 0.15 (2m 이내 오차)
    - GPU 메모리 <= 6GB (Jetson 8GB 기준)
    """
    print("\n" + "━" * 36)
    print("문제 2: 트레이드오프 분석")
    print("━" * 36 + "\n")

    configs = [
        {"name": "Config-A", "resolution": "640x480",
         "fps": 8.5, "absrel": 0.06, "gpu_mem": 5.2},
        {"name": "Config-B", "resolution": "384x512",
         "fps": 15.4, "absrel": 0.09, "gpu_mem": 3.8},
        {"name": "Config-C", "resolution": "256x320",
         "fps": 25.0, "absrel": 0.22, "gpu_mem": 2.1},
    ]

    print(f"  {'설정':>10} | {'해상도':>10} | {'FPS':>6} | {'AbsRel':>8} | {'GPU(GB)':>8}")
    print("  " + "─" * 55)
    for c in configs:
        print(f"  {c['name']:>10} | {c['resolution']:>10} | "
              f"{c['fps']:>6.1f} | {c['absrel']:>8.2f} | {c['gpu_mem']:>8.1f}")

    print("\n  요구사항:")
    print("    - FPS >= 10")
    print("    - AbsRel <= 0.15")
    print("    - GPU 메모리 <= 6GB")
    print()

    print("  과제:")
    print("  1) 각 설정이 요구사항을 만족하는지 판단하시오.")
    print("  2) AMR 자율주행에 가장 적합한 설정은?")
    print("  3) Config-A를 만족시키려면 어떤 최적화가 필요한가?")


def problem3_performance_report():
    """
    문제 3: 최종 성능 리포트 작성

    아래 측정 데이터로 Phase 5 최종 리포트를 완성하시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: 성능 리포트 완성")
    print("━" * 36 + "\n")

    # 측정 데이터
    yolo_times = [28.5, 31.2, 29.8, 30.5, 32.1, 29.0, 30.8, 31.5, 28.9, 30.2]
    depth_times = [52.3, 55.8, 54.1, 53.5, 56.2, 51.9, 55.0, 54.8, 53.2, 55.5]
    fusion_times = [1.8, 2.1, 1.9, 2.3, 2.0, 1.7, 2.2, 1.9, 2.1, 2.0]

    print("  10프레임 측정 데이터 (ms):")
    print(f"    YOLO:   {yolo_times}")
    print(f"    Depth:  {depth_times}")
    print(f"    Fusion: {fusion_times}")
    print()

    print("  과제:")
    print("  1) 각 단계의 평균/표준편차를 구하시오.")
    print("  2) 순차 실행 시 평균 총 시간과 FPS는?")
    print("  3) 병렬 실행(YOLO||Depth) 시 평균 총 시간과 FPS는?")
    print("  4) 어떤 단계의 분산이 가장 크고, 그 원인은?")
    print()

    # TODO: 학생이 계산
    yolo_avg = np.mean(yolo_times)
    depth_avg = np.mean(depth_times)
    fusion_avg = np.mean(fusion_times)

    print(f"  힌트: YOLO 평균 = {yolo_avg:.1f}ms")
    print(f"        Depth 평균 = {depth_avg:.1f}ms")
    print(f"        Fusion 평균 = {fusion_avg:.1f}ms")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 12 Quiz - Medium (최종 데모)")
    print("━" * 40)

    problem1_amdahl_law()
    problem2_tradeoff_analysis()
    problem3_performance_report()

    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
