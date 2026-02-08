"""
Phase 5 Week 10 - Depth 정확도 검증 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_compute_metrics():
    """
    문제 1: Depth 평가 지표 직접 계산

    아래 주어진 GT와 예측 깊이에서 AbsRel과 delta < 1.25를 계산하시오.

    TODO: abs_rel과 delta1을 직접 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: Depth 평가 지표 직접 계산")
    print("━" * 36 + "\n")

    # 5개 픽셀의 예측/GT 깊이
    gt = np.array([10.0, 20.0, 30.0, 5.0, 15.0])
    pred = np.array([9.0, 22.0, 25.0, 6.0, 14.0])

    print("  GT 깊이:   ", gt)
    print("  예측 깊이:  ", pred)
    print()

    # 1. AbsRel = mean(|pred - gt| / gt)
    abs_diff = np.abs(pred - gt)
    rel_diff = abs_diff / gt
    print("  |pred - gt|:     ", abs_diff)
    print("  |pred - gt|/gt:  ", rel_diff)

    abs_rel_computed = np.mean(rel_diff)
    print(f"  AbsRel (계산값): {abs_rel_computed:.4f}")

    # TODO: 직접 계산해보세요
    abs_rel_expected = 0.0  # 여기를 채우시오
    print(f"  AbsRel (기대값): {abs_rel_expected}")

    # 2. delta < 1.25 = mean(max(pred/gt, gt/pred) < 1.25)
    ratio = np.maximum(pred / gt, gt / pred)
    print(f"\n  max(pred/gt, gt/pred): {ratio}")
    print(f"  < 1.25 여부:           {ratio < 1.25}")

    delta1_computed = np.mean(ratio < 1.25)
    print(f"  delta < 1.25 (계산값): {delta1_computed:.4f}")

    # TODO: 직접 계산해보세요
    delta1_expected = 0.0  # 여기를 채우시오
    print(f"  delta < 1.25 (기대값): {delta1_expected}")

    # 검증
    if (abs(abs_rel_expected - abs_rel_computed) < 0.001 and
            abs(delta1_expected - delta1_computed) < 0.001):
        print("\n  정답!")
    else:
        print("\n  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_scale_alignment():
    """
    문제 2: Scale 정렬 이해

    상대적 깊이 [0.1, 0.2, 0.3, 0.5, 1.0]이
    절대 깊이 [5, 10, 15, 25, 50]에 대응할 때,
    scale과 shift를 구하시오.

    hint: gt = scale * pred + shift
          scale = 50, shift = 0 인 경우를 생각해보세요.
    """
    print("\n" + "━" * 36)
    print("문제 2: Scale 정렬 계산")
    print("━" * 36 + "\n")

    pred_rel = np.array([0.1, 0.2, 0.3, 0.5, 1.0])
    gt_abs = np.array([5.0, 10.0, 15.0, 25.0, 50.0])

    print("  상대 깊이 (pred):", pred_rel)
    print("  절대 깊이 (GT):  ", gt_abs)
    print()

    # 최소자승법
    A = np.vstack([pred_rel, np.ones_like(pred_rel)]).T
    result = np.linalg.lstsq(A, gt_abs, rcond=None)
    scale_computed, shift_computed = result[0]

    print(f"  최소자승법 결과:")
    print(f"    scale = {scale_computed:.4f}")
    print(f"    shift = {shift_computed:.4f}")

    # TODO: 직접 계산해보세요
    scale_expected = 0.0  # 여기를 채우시오
    shift_expected = 0.0  # 여기를 채우시오

    print(f"\n  직접 계산:")
    print(f"    scale = {scale_expected}")
    print(f"    shift = {shift_expected}")

    # 정렬된 깊이 확인
    aligned = scale_computed * pred_rel + shift_computed
    print(f"\n  정렬 결과: {aligned}")
    print(f"  GT:        {gt_abs}")
    print(f"  오차:      {np.abs(aligned - gt_abs)}")

    if (abs(scale_expected - scale_computed) < 1.0 and
            abs(shift_expected - shift_computed) < 1.0):
        print("\n  정답!")
    else:
        print("\n  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem3_distance_error_analysis():
    """
    문제 3: 거리별 오차 분석

    아래 데이터에서 각 거리 구간의 AbsRel을 계산하고,
    어떤 거리에서 오차가 가장 큰지 분석하시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: 거리별 오차 분석")
    print("━" * 36 + "\n")

    np.random.seed(42)

    # 시뮬레이션: 먼 거리일수록 오차가 큼
    gt_near = np.random.uniform(2, 10, 100)      # 근거리
    gt_mid = np.random.uniform(10, 30, 100)       # 중거리
    gt_far = np.random.uniform(30, 80, 100)       # 원거리

    pred_near = gt_near * (1 + np.random.normal(0, 0.03, 100))   # 3% 오차
    pred_mid = gt_mid * (1 + np.random.normal(0, 0.08, 100))     # 8% 오차
    pred_far = gt_far * (1 + np.random.normal(0, 0.20, 100))     # 20% 오차

    # 거리별 AbsRel 계산
    absrel_near = np.mean(np.abs(pred_near - gt_near) / gt_near)
    absrel_mid = np.mean(np.abs(pred_mid - gt_mid) / gt_mid)
    absrel_far = np.mean(np.abs(pred_far - gt_far) / gt_far)

    print(f"  거리별 AbsRel:")
    print(f"    근거리 (2~10m):  {absrel_near:.4f}")
    print(f"    중거리 (10~30m): {absrel_mid:.4f}")
    print(f"    원거리 (30~80m): {absrel_far:.4f}")
    print()

    print("  질문: 원거리에서 오차가 큰 이유를 3가지 이상 서술하시오.")
    print()
    print("  힌트:")
    print("    1) 이미지에서 차지하는 영역 크기")
    print("    2) 텍스처 정보의 양")
    print("    3) Monocular depth의 본질적 한계")
    print()
    print("  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 10 Quiz - Medium")
    print("━" * 40)
    problem1_compute_metrics()
    problem2_scale_alignment()
    problem3_distance_error_analysis()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
