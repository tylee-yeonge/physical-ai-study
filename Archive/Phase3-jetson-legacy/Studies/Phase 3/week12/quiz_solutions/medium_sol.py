"""
Phase 5 Week 12 - 최종 데모 & 포트폴리오 중급 퀴즈 풀이
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 풀이: Amdahl 법칙 최적화")
    print("━" * 36 + "\n")

    # 원래 순차 실행
    sequential = {
        '이미지 캡처':   5,
        'YOLO 전처리':   3,
        'YOLO 추론':    30,
        'YOLO 후처리':   2,
        'Depth 전처리':  3,
        'Depth 추론':   55,
        'Depth 후처리':  2,
        '융합+역투영':    2,
        '시각화':        3,
    }
    total_seq = sum(sequential.values())
    print(f"  원래 순차 실행: {total_seq}ms ({1000/total_seq:.1f} FPS)\n")

    # 1) 병렬 실행: YOLO와 Depth를 동시에
    # 병렬 구간: max(YOLO 전처리+추론+후처리, Depth 전처리+추론+후처리)
    yolo_total = 3 + 30 + 2  # 35ms
    depth_total = 3 + 55 + 2  # 60ms
    parallel_section = max(yolo_total, depth_total)  # 60ms
    non_parallel = 5 + 2 + 3  # 캡처 + 융합 + 시각화 = 10ms
    total_parallel = non_parallel + parallel_section

    print("  1) YOLO||Depth 병렬 실행:")
    print(f"     YOLO 구간: {yolo_total}ms (전처리+추론+후처리)")
    print(f"     Depth 구간: {depth_total}ms (전처리+추론+후처리)")
    print(f"     병렬 소요: max({yolo_total}, {depth_total}) = {parallel_section}ms")
    print(f"     비병렬 구간: 캡처(5) + 융합(2) + 시각화(3) = {non_parallel}ms")
    print(f"     총 시간: {total_parallel}ms ({1000/total_parallel:.1f} FPS)\n")

    # 2) Depth 2x 최적화
    depth_optimized = 3 + 27.5 + 2  # 32.5ms
    yolo_total_f = float(yolo_total)
    parallel_opt = max(yolo_total_f, depth_optimized)
    total_opt = non_parallel + parallel_opt

    print("  2) 추가로 Depth 2x 빠르게 (55→27.5ms):")
    print(f"     Depth 구간: {depth_optimized}ms")
    print(f"     병렬 소요: max({yolo_total}, {depth_optimized}) = {parallel_opt}ms")
    print(f"     총 시간: {total_opt}ms ({1000/total_opt:.1f} FPS)\n")

    # 비교
    speedup1 = total_seq / total_parallel
    speedup2 = total_seq / total_opt
    print("  비교:")
    print(f"     순차:          {total_seq}ms → {1000/total_seq:.1f} FPS")
    print(f"     병렬:          {total_parallel}ms → {1000/total_parallel:.1f} FPS ({speedup1:.2f}x)")
    print(f"     병렬+Depth최적: {total_opt}ms → {1000/total_opt:.1f} FPS ({speedup2:.2f}x)")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 풀이: 트레이드오프 분석")
    print("━" * 36 + "\n")

    configs = [
        {"name": "Config-A", "fps": 8.5, "absrel": 0.06, "gpu": 5.2},
        {"name": "Config-B", "fps": 15.4, "absrel": 0.09, "gpu": 3.8},
        {"name": "Config-C", "fps": 25.0, "absrel": 0.22, "gpu": 2.1},
    ]

    print("  1) 요구사항 충족 여부:")
    print(f"     {'설정':>10} | {'FPS>=10':>8} | {'AbsRel<=0.15':>13} | {'GPU<=6GB':>9} | 결과")
    print("     " + "─" * 55)

    for c in configs:
        fps_ok = c['fps'] >= 10
        abs_ok = c['absrel'] <= 0.15
        gpu_ok = c['gpu'] <= 6.0
        all_ok = fps_ok and abs_ok and gpu_ok

        print(f"     {c['name']:>10} | "
              f"{'PASS' if fps_ok else 'FAIL':>8} | "
              f"{'PASS' if abs_ok else 'FAIL':>13} | "
              f"{'PASS' if gpu_ok else 'FAIL':>9} | "
              f"{'PASS' if all_ok else 'FAIL'}")

    print()
    print("  2) 최적 설정: Config-B")
    print("     → FPS 15.4 (실시간 충분)")
    print("     → AbsRel 0.09 (높은 정확도)")
    print("     → GPU 3.8GB (메모리 여유)")
    print("     → 모든 요구사항 만족하는 유일한 설정")
    print()
    print("  3) Config-A 개선 방안:")
    print("     → FPS 8.5 < 10 이 문제")
    print("     → 방법 1: 병렬 실행 (YOLO || Depth)")
    print("     → 방법 2: Depth 모델 경량화 (ViT-B → ViT-S)")
    print("     → 방법 3: 입력 해상도 줄이기 (640→512)")
    print("     → 방법 4: 프레임 스킵 (2프레임마다 Depth 실행)")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 풀이: 성능 리포트 완성")
    print("━" * 36 + "\n")

    yolo_times = [28.5, 31.2, 29.8, 30.5, 32.1, 29.0, 30.8, 31.5, 28.9, 30.2]
    depth_times = [52.3, 55.8, 54.1, 53.5, 56.2, 51.9, 55.0, 54.8, 53.2, 55.5]
    fusion_times = [1.8, 2.1, 1.9, 2.3, 2.0, 1.7, 2.2, 1.9, 2.1, 2.0]

    yolo_avg = np.mean(yolo_times)
    yolo_std = np.std(yolo_times)
    depth_avg = np.mean(depth_times)
    depth_std = np.std(depth_times)
    fusion_avg = np.mean(fusion_times)
    fusion_std = np.std(fusion_times)

    print("  1) 각 단계 통계:")
    print(f"     YOLO:   평균 {yolo_avg:.1f}ms, 표준편차 {yolo_std:.2f}ms")
    print(f"     Depth:  평균 {depth_avg:.1f}ms, 표준편차 {depth_std:.2f}ms")
    print(f"     Fusion: 평균 {fusion_avg:.1f}ms, 표준편차 {fusion_std:.2f}ms")
    print()

    # 순차 실행
    seq_total = yolo_avg + depth_avg + fusion_avg
    seq_fps = 1000.0 / seq_total
    print(f"  2) 순차 실행:")
    print(f"     총 시간 = {yolo_avg:.1f} + {depth_avg:.1f} + {fusion_avg:.1f} = {seq_total:.1f}ms")
    print(f"     FPS = 1000 / {seq_total:.1f} = {seq_fps:.1f}")
    print()

    # 병렬 실행
    parallel_times = []
    for y, d, f in zip(yolo_times, depth_times, fusion_times):
        t = max(y, d) + f
        parallel_times.append(t)
    par_avg = np.mean(parallel_times)
    par_fps = 1000.0 / par_avg

    print(f"  3) 병렬 실행 (YOLO||Depth):")
    print(f"     각 프레임: max(YOLO, Depth) + Fusion")
    print(f"     평균 총 시간 = {par_avg:.1f}ms")
    print(f"     FPS = 1000 / {par_avg:.1f} = {par_fps:.1f}")
    print()

    print(f"  4) 분산 분석:")
    print(f"     YOLO  분산: {yolo_std**2:.2f}")
    print(f"     Depth 분산: {depth_std**2:.2f}")
    print(f"     Fusion 분산: {fusion_std**2:.2f}")
    print()
    print(f"     → Depth 추론의 분산이 가장 큼")
    print(f"     → 원인: ViT 모델의 attention 연산이")
    print(f"       입력 이미지 내용에 따라 실행 시간이 변동")
    print(f"     → GPU 스케줄링, 메모리 대역폭도 영향")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 12 Quiz Medium - 풀이")
    print("━" * 40)

    problem1_solution()
    problem2_solution()
    problem3_solution()

    print("\n" + "━" * 40)
