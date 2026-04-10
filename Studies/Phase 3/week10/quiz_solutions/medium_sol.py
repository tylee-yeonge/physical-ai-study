"""
Phase 5 Week 10 - Depth 정확도 검증 중급 퀴즈 정답 및 해설
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: AbsRel = 0.0933, delta < 1.25 = 0.80")
    print("━" * 36 + "\n")

    gt = np.array([10.0, 20.0, 30.0, 5.0, 15.0])
    pred = np.array([9.0, 22.0, 25.0, 6.0, 14.0])

    print("해설:")
    print(f"  GT:   {gt}")
    print(f"  Pred: {pred}")
    print()

    # AbsRel
    abs_diff = np.abs(pred - gt)
    rel_diff = abs_diff / gt

    print("  AbsRel 계산:")
    for i in range(len(gt)):
        print(f"    |{pred[i]} - {gt[i]}| / {gt[i]} = {abs_diff[i]} / {gt[i]} = {rel_diff[i]:.4f}")

    abs_rel = np.mean(rel_diff)
    print(f"  평균 = ({' + '.join(f'{r:.4f}' for r in rel_diff)}) / 5")
    print(f"       = {np.sum(rel_diff):.4f} / 5 = {abs_rel:.4f}")

    print()

    # delta < 1.25
    ratio = np.maximum(pred / gt, gt / pred)
    print("  delta < 1.25 계산:")
    for i in range(len(gt)):
        r = ratio[i]
        check = "< 1.25" if r < 1.25 else ">= 1.25"
        print(f"    max({pred[i]}/{gt[i]}, {gt[i]}/{pred[i]}) = {r:.4f} {check}")

    delta1 = np.mean(ratio < 1.25)
    count = np.sum(ratio < 1.25)
    print(f"  1.25 미만 개수: {count} / 5 = {delta1:.4f}")

    print()
    print(f"  정답: AbsRel = {abs_rel:.4f}, delta < 1.25 = {delta1:.2f}")
    print()
    print("  30m → 25m 예측이 가장 큰 오차 (AbsRel = 0.1667)")
    print("  이처럼 먼 물체에서 깊이 추정이 부정확해지는 경향이 있습니다.")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: scale = 50.0, shift = 0.0")
    print("━" * 36 + "\n")

    pred_rel = np.array([0.1, 0.2, 0.3, 0.5, 1.0])
    gt_abs = np.array([5.0, 10.0, 15.0, 25.0, 50.0])

    print("해설:")
    print(f"  상대 깊이: {pred_rel}")
    print(f"  절대 깊이: {gt_abs}")
    print()
    print("  관찰: gt = 50 * pred")
    print("  → 모든 gt/pred = 50.0")

    for i in range(len(pred_rel)):
        print(f"    {gt_abs[i]} / {pred_rel[i]} = {gt_abs[i] / pred_rel[i]:.1f}")

    print()
    print("  따라서: scale = 50.0, shift = 0.0")
    print("  d_abs = 50.0 * d_rel + 0.0")
    print()

    # 최소자승법 검증
    A = np.vstack([pred_rel, np.ones_like(pred_rel)]).T
    result = np.linalg.lstsq(A, gt_abs, rcond=None)
    scale, shift = result[0]
    print(f"  최소자승법 검증: scale = {scale:.4f}, shift = {shift:.4f}")

    print()
    print("  참고 - 실제 데이터에서는:")
    print("  - 완벽한 선형 관계가 아닐 수 있음")
    print("  - shift가 0이 아닌 경우도 많음 (모델의 systematic bias)")
    print("  - 최소자승법이 최적의 scale/shift를 찾아줌")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: 원거리 오차가 큰 이유")
    print("━" * 36 + "\n")

    print("해설:")
    print("  원거리(30~80m)에서 오차가 큰 이유:")
    print()
    print("  1. 이미지 해상도 한계")
    print("     → 30m 거리의 물체는 이미지에서 매우 작은 영역 차지")
    print("     → 적은 픽셀 = 적은 정보 = 부정확한 추정")
    print()
    print("  2. 텍스처 정보 부족")
    print("     → 원거리 물체는 디테일이 사라짐")
    print("     → 모델이 깊이 단서를 찾기 어려움")
    print()
    print("  3. Monocular Depth의 본질적 한계 (Scale Ambiguity)")
    print("     → 단안 카메라로는 절대 스케일을 알 수 없음")
    print("     → 원거리일수록 이 모호성이 심해짐")
    print("     → 근거리: 많은 시각적 단서 (크기, 그림자 등)")
    print("     → 원거리: 대기 투시 외에 단서 부족")
    print()
    print("  4. 학습 데이터 편향")
    print("     → 대부분의 학습 데이터에서 원거리 GT가 부정확")
    print("     → 스테레오/LiDAR도 원거리에서는 노이즈가 큼")
    print()
    print("  5. 깊이 범위의 비대칭성")
    print("     → 0~10m: 범위 10m")
    print("     → 40~80m: 범위 40m")
    print("     → 같은 상대 오차라도 절대 오차가 4배 큼")
    print()
    print("  SLAM에서의 대처:")
    print("  → 깊이 사용 범위를 제한 (예: 0.5~30m)")
    print("  → 원거리는 VO/VIO로 보정")
    print("  → 깊이 신뢰도(confidence)가 낮은 영역 필터링")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 10 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
