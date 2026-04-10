"""
Phase 6 Week 10 - BEVFormer 실습 중급 퀴즈 정답 및 해설
"""
import numpy as np


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: NDS 계산")
    print("━" * 36 + "\n")

    mAP = 0.416
    mATE = 0.673
    mASE = 0.274
    mAOE = 0.372
    mAVE = 0.394
    mAAE = 0.198

    # 단계별 계산
    step1 = 5 * mAP
    step2_ATE = 1 - mATE
    step2_ASE = 1 - mASE
    step2_AOE = 1 - mAOE
    step2_AVE = 1 - mAVE
    step2_AAE = 1 - mAAE
    tp_sum = step2_ATE + step2_ASE + step2_AOE + step2_AVE + step2_AAE
    nds = (step1 + tp_sum) / 10.0

    print("  단계별 계산:")
    print(f"    5 * mAP = 5 * {mAP} = {step1:.3f}")
    print()
    print(f"    1 - mATE = 1 - {mATE} = {step2_ATE:.3f}")
    print(f"    1 - mASE = 1 - {mASE} = {step2_ASE:.3f}")
    print(f"    1 - mAOE = 1 - {mAOE} = {step2_AOE:.3f}")
    print(f"    1 - mAVE = 1 - {mAVE} = {step2_AVE:.3f}")
    print(f"    1 - mAAE = 1 - {mAAE} = {step2_AAE:.3f}")
    print(f"    TP 합 = {tp_sum:.3f}")
    print()
    print(f"    NDS = ({step1:.3f} + {tp_sum:.3f}) / 10")
    print(f"        = {step1 + tp_sum:.3f} / 10")
    print(f"        = {nds:.4f}")
    print()
    print(f"  정답: NDS = {nds:.4f} (약 0.517)")
    print()
    print("  해석:")
    print("  - mAP 기여분: 5*0.416/10 = 0.208 (전체의 40%)")
    print("  - TP 기여분: 3.089/10 = 0.309 (전체의 60%)")
    print("  - TP 메트릭 중 mASE(크기)가 가장 좋고, mATE(위치)가 가장 나쁨")


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: mAP threshold 분석")
    print("━" * 36 + "\n")

    ap_05 = 0.35
    ap_10 = 0.55
    ap_20 = 0.72
    ap_40 = 0.80

    car_ap = (ap_05 + ap_10 + ap_20 + ap_40) / 4.0

    print(f"  Car AP = ({ap_05} + {ap_10} + {ap_20} + {ap_40}) / 4")
    print(f"        = {ap_05 + ap_10 + ap_20 + ap_40} / 4")
    print(f"        = {car_ap:.4f}")
    print()
    print(f"  정답: Car AP = {car_ap:.4f} (약 0.605)")
    print()
    print("  AP@0.5m이 AP@4.0m보다 낮은 이유:")
    print()
    print("  threshold = 0.5m:")
    print("    예측 중심이 GT 중심에서 0.5m 이내여야 TP")
    print("    → 매우 엄격: 약간의 위치 오차도 FP로 판정")
    print("    → AP가 낮음 (0.35)")
    print()
    print("  threshold = 4.0m:")
    print("    예측 중심이 GT 중심에서 4.0m 이내면 TP")
    print("    → 느슨: 상당한 오차도 TP로 인정")
    print("    → AP가 높음 (0.80)")
    print()
    print("  시사점:")
    print("  - 0.5m threshold에서의 AP는 '정밀한 위치 추정' 능력을 반영")
    print("  - 4.0m threshold에서의 AP는 '대략적인 검출' 능력을 반영")
    print("  - 둘의 차이가 크면 '검출은 하지만 위치가 부정확'한 것")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: TP/FP/FN 분석")
    print("━" * 36 + "\n")

    # GT
    gts = [
        {'name': 'Car A', 'center': np.array([10.0, 2.0])},
        {'name': 'Car B', 'center': np.array([25.0, -3.0])},
        {'name': 'Ped C', 'center': np.array([8.0, 5.0])},
    ]

    preds = [
        {'name': 'Pred 1', 'center': np.array([10.3, 1.8]), 'score': 0.92},
        {'name': 'Pred 2', 'center': np.array([26.5, -2.5]), 'score': 0.75},
        {'name': 'Pred 3', 'center': np.array([8.2, 5.3]), 'score': 0.81},
        {'name': 'Pred 4', 'center': np.array([15.0, 0.0]), 'score': 0.45},
    ]

    threshold = 2.0

    print("  매칭 분석 (threshold=2.0m):")
    print()

    # Pred 1 ↔ Car A
    d1a = np.linalg.norm(preds[0]['center'] - gts[0]['center'])
    print(f"  Pred 1 ↔ Car A: {d1a:.2f}m → {'TP' if d1a <= threshold else 'X'}")

    # Pred 2 ↔ Car B
    d2b = np.linalg.norm(preds[1]['center'] - gts[1]['center'])
    print(f"  Pred 2 ↔ Car B: {d2b:.2f}m → {'TP' if d2b <= threshold else 'X'}")

    # Pred 3 ↔ Ped C
    d3c = np.linalg.norm(preds[2]['center'] - gts[2]['center'])
    print(f"  Pred 3 ↔ Ped C: {d3c:.2f}m → {'TP' if d3c <= threshold else 'X'}")

    # Pred 4
    d4_min = min(
        np.linalg.norm(preds[3]['center'] - gt['center']) for gt in gts
    )
    print(f"  Pred 4 ↔ 가장 가까운 GT: {d4_min:.2f}m → {'TP' if d4_min <= threshold else 'FP'}")

    print()
    print("  결과:")
    print(f"    Pred 1 → Car A:  {d1a:.2f}m ≤ 2.0m → TP")
    print(f"    Pred 2 → Car B:  {d2b:.2f}m ≤ 2.0m → TP")
    print(f"    Pred 3 → Ped C:  {d3c:.2f}m ≤ 2.0m → TP")
    print(f"    Pred 4 → 없음:   {d4_min:.2f}m > 2.0m → FP")
    print()
    print("  정답: TP=3, FP=1, FN=0")
    print()
    print("  Precision = TP/(TP+FP) = 3/4 = 0.75")
    print("  Recall    = TP/(TP+FN) = 3/3 = 1.00")
    print()
    print("  해석:")
    print("  - 모든 GT 객체를 찾았지만 (Recall=1.0)")
    print("  - 하나의 오탐(Pred 4)이 있음 (Precision=0.75)")
    print("  - Pred 4는 가장 가까운 GT와도 5m 이상 떨어져 있어 FP")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 10 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
