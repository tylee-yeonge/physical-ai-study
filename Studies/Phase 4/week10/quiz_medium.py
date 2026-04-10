"""
Phase 6 Week 10 - BEVFormer 실습 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_nds_calculation():
    """
    문제 1: NDS 직접 계산

    아래 모델의 NDS를 계산하시오.

    BEVFormer-Base 결과:
      mAP  = 0.416
      mATE = 0.673
      mASE = 0.274
      mAOE = 0.372
      mAVE = 0.394
      mAAE = 0.198

    NDS = 1/10 * (5*mAP + (1-mATE) + (1-mASE) + (1-mAOE) + (1-mAVE) + (1-mAAE))

    TODO: nds_calculated를 직접 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: NDS 직접 계산")
    print("━" * 36 + "\n")

    mAP = 0.416
    mATE = 0.673
    mASE = 0.274
    mAOE = 0.372
    mAVE = 0.394
    mAAE = 0.198

    print(f"  mAP  = {mAP}")
    print(f"  mATE = {mATE}")
    print(f"  mASE = {mASE}")
    print(f"  mAOE = {mAOE}")
    print(f"  mAVE = {mAVE}")
    print(f"  mAAE = {mAAE}")
    print()
    print("  NDS = 1/10 * (5*mAP + (1-mATE) + (1-mASE) + (1-mAOE) + (1-mAVE) + (1-mAAE))")
    print()

    # TODO: 직접 계산하시오
    nds_calculated = 0.0  # 여기를 채우시오

    # 실제 계산
    tp_sum = (1 - mATE) + (1 - mASE) + (1 - mAOE) + (1 - mAVE) + (1 - mAAE)
    actual_nds = (5 * mAP + tp_sum) / 10.0

    print(f"  계산한 NDS: {nds_calculated:.4f}")
    print(f"  실제 NDS:   {actual_nds:.4f}")

    if abs(nds_calculated - actual_nds) < 0.01:
        print("\n  정답!")
    else:
        print("\n  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")
        print(f"  힌트: 5*mAP = {5*mAP:.3f}")
        print(f"  힌트: TP 합 = {tp_sum:.3f}")


def problem2_map_threshold():
    """
    문제 2: mAP 거리 threshold 분석

    nuScenes mAP는 4개 거리 threshold (0.5, 1.0, 2.0, 4.0 m)의
    평균 AP로 계산된다.

    특정 모델의 Car 클래스 AP가 다음과 같을 때:
      AP@0.5m = 0.35
      AP@1.0m = 0.55
      AP@2.0m = 0.72
      AP@4.0m = 0.80

    Car의 평균 AP를 계산하시오.

    또한, AP@0.5m이 AP@4.0m보다 낮은 이유를 생각해보시오.

    TODO: car_ap를 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 2: mAP 거리 threshold 분석")
    print("━" * 36 + "\n")

    ap_05 = 0.35
    ap_10 = 0.55
    ap_20 = 0.72
    ap_40 = 0.80

    print(f"  Car 클래스 AP:")
    print(f"    AP@0.5m = {ap_05}")
    print(f"    AP@1.0m = {ap_10}")
    print(f"    AP@2.0m = {ap_20}")
    print(f"    AP@4.0m = {ap_40}")
    print()

    # TODO: 직접 계산하시오
    car_ap = 0.0  # 여기를 채우시오

    actual_ap = (ap_05 + ap_10 + ap_20 + ap_40) / 4.0

    print(f"  계산한 Car AP: {car_ap:.4f}")
    print(f"  실제 Car AP:   {actual_ap:.4f}")

    if abs(car_ap - actual_ap) < 0.01:
        print("\n  정답!")
    else:
        print("\n  다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")

    print()
    print("  생각해볼 질문:")
    print("  AP@0.5m이 AP@4.0m보다 낮은 이유는?")
    print("  (힌트: threshold가 엄격할수록 TP 매칭이 어려움)")


def problem3_detection_analysis():
    """
    문제 3: 검출 결과 분석

    아래 BEV 검출 결과에서 각 객체의 검출 상태를 판단하시오.

    GT (Ground Truth):
      Car A: center=(10, 2), 거리=10.2m
      Car B: center=(25, -3), 거리=25.2m
      Pedestrian C: center=(8, 5), 거리=9.4m

    Predictions:
      Pred 1: center=(10.3, 1.8), class=car, score=0.92
      Pred 2: center=(26.5, -2.5), class=car, score=0.75
      Pred 3: center=(8.2, 5.3), class=pedestrian, score=0.81
      Pred 4: center=(15, 0), class=car, score=0.45

    매칭 기준: BEV 중심 거리 2.0m 이내이면 TP

    질문: TP, FP, FN 각각 몇 개인가?

    TODO: tp, fp, fn을 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: 검출 결과 분석")
    print("━" * 36 + "\n")

    # GT
    gts = [
        {'name': 'Car A', 'center': np.array([10.0, 2.0])},
        {'name': 'Car B', 'center': np.array([25.0, -3.0])},
        {'name': 'Ped C', 'center': np.array([8.0, 5.0])},
    ]

    # Predictions
    preds = [
        {'name': 'Pred 1', 'center': np.array([10.3, 1.8]), 'class': 'car', 'score': 0.92},
        {'name': 'Pred 2', 'center': np.array([26.5, -2.5]), 'class': 'car', 'score': 0.75},
        {'name': 'Pred 3', 'center': np.array([8.2, 5.3]), 'class': 'ped', 'score': 0.81},
        {'name': 'Pred 4', 'center': np.array([15.0, 0.0]), 'class': 'car', 'score': 0.45},
    ]

    threshold = 2.0

    print("  GT:")
    for gt in gts:
        print(f"    {gt['name']}: center={gt['center']}")
    print()
    print("  Predictions:")
    for pred in preds:
        print(f"    {pred['name']}: center={pred['center']}, score={pred['score']}")
    print()

    # 거리 계산
    print(f"  매칭 거리 (threshold={threshold}m):")
    for pred in preds:
        for gt in gts:
            dist = np.linalg.norm(pred['center'] - gt['center'])
            match = "TP" if dist <= threshold else "  "
            print(f"    {pred['name']} ↔ {gt['name']}: {dist:.2f}m {match}")
        print()

    # TODO: 직접 분석하시오
    tp = 0  # 여기를 채우시오
    fp = 0  # 여기를 채우시오
    fn = 0  # 여기를 채우시오

    print(f"  분석 결과: TP={tp}, FP={fp}, FN={fn}")
    print(f"  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 10 Quiz - Medium")
    print("━" * 40)
    problem1_nds_calculation()
    problem2_map_threshold()
    problem3_detection_analysis()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
