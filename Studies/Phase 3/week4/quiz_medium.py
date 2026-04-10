"""
Phase 5 Week 4 - YOLOv8 학습 중급 퀴즈
코드를 직접 실행하고 결과를 확인하세요.
"""
import numpy as np


def problem1_label_conversion():
    """
    문제 1: BBox 포맷 변환

    이미지 크기: 1920 x 1080 (WxH)
    BBox (Pascal VOC): [384, 216, 960, 756]
    = [x_min, y_min, x_max, y_max]

    이 BBox를 YOLO 포맷으로 변환하시오.
    YOLO: [class_id, x_center/W, y_center/H, width/W, height/H]

    TODO: yolo_label을 계산하여 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 1: BBox 포맷 변환")
    print("━" * 36 + "\n")

    W, H = 1920, 1080
    x_min, y_min, x_max, y_max = 384, 216, 960, 756
    class_id = 0  # person

    print(f"  이미지 크기: {W} x {H}")
    print(f"  Pascal VOC: [{x_min}, {y_min}, {x_max}, {y_max}]")
    print()
    print("  힌트:")
    print(f"    x_center = ({x_min} + {x_max}) / 2 / {W} = ?")
    print(f"    y_center = ({y_min} + {y_max}) / 2 / {H} = ?")
    print(f"    width    = ({x_max} - {x_min}) / {W} = ?")
    print(f"    height   = ({y_max} - {y_min}) / {H} = ?")

    # 정답 계산
    actual_xc = (x_min + x_max) / 2 / W
    actual_yc = (y_min + y_max) / 2 / H
    actual_w = (x_max - x_min) / W
    actual_h = (y_max - y_min) / H

    # TODO: 직접 계산해보세요
    yolo_xc = 0.0   # 여기를 채우시오
    yolo_yc = 0.0   # 여기를 채우시오
    yolo_w = 0.0    # 여기를 채우시오
    yolo_h = 0.0    # 여기를 채우시오

    print(f"\n  정답: {class_id} {actual_xc:.4f} {actual_yc:.4f} "
          f"{actual_w:.4f} {actual_h:.4f}")
    print(f"  입력: {class_id} {yolo_xc:.4f} {yolo_yc:.4f} "
          f"{yolo_w:.4f} {yolo_h:.4f}")

    if (abs(yolo_xc - actual_xc) < 0.01 and
            abs(yolo_yc - actual_yc) < 0.01 and
            abs(yolo_w - actual_w) < 0.01 and
            abs(yolo_h - actual_h) < 0.01):
        print("\n  ✅ 정답!")
    else:
        print("\n  ❌ 다시 계산해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem2_confusion_matrix():
    """
    문제 2: Confusion Matrix 분석

    아래 혼동 행렬을 분석하여 질문에 답하시오.

                 예측: person  car  bicycle  background
    실제: person    70     5     3       22
          car        2    60     1       37
          bicycle    4     0    45       51
          background 8     3     2        -

    TODO: 각 질문에 답하시오.
    """
    print("\n" + "━" * 36)
    print("문제 2: Confusion Matrix 분석")
    print("━" * 36 + "\n")

    print("  Confusion Matrix:")
    print(f"  {'':16s} 예측: person  car  bicycle  background")
    print(f"  실제: person     70     5     3       22")
    print(f"        car         2    60     1       37")
    print(f"        bicycle     4     0    45       51")
    print(f"        background  8     3     2        -")
    print()

    print("  질문 1: person 클래스의 Recall은?")
    print("    Recall = TP / (TP + FN)")
    print("    TP = person→person = 70")
    print("    FN = person→(다른 클래스 + background) = 5 + 3 + 22 = 30")
    print("    Recall = ?")
    print()

    print("  질문 2: 가장 미검출(FN)이 많은 클래스는?")
    print("    각 클래스의 background 열 값 비교")
    print()

    print("  질문 3: False Positive가 가장 많은 클래스는?")
    print("    background 행에서 각 클래스 열 값 비교")
    print()

    # TODO: 직접 계산해보세요
    person_recall = 0.0      # 여기를 채우시오
    most_fn_class = ""       # 여기를 채우시오 ("person", "car", "bicycle")
    most_fp_class = ""       # 여기를 채우시오

    # 정답
    actual_person_recall = 70 / (70 + 5 + 3 + 22)
    actual_most_fn = "bicycle"   # background 열: 22, 37, 51 → bicycle
    actual_most_fp = "person"    # background 행: 8, 3, 2 → person

    print(f"  정답:")
    print(f"    person Recall = 70 / 100 = {actual_person_recall:.2f}")
    print(f"    가장 미검출 많은 클래스: {actual_most_fn} (51개)")
    print(f"    가장 오검출 많은 클래스: {actual_most_fp} (8개)")

    print(f"\n  내 답:")
    print(f"    person Recall = {person_recall:.2f}")
    print(f"    가장 미검출 많은 클래스: {most_fn_class}")
    print(f"    가장 오검출 많은 클래스: {most_fp_class}")

    if (abs(person_recall - actual_person_recall) < 0.01 and
            most_fn_class == actual_most_fn and
            most_fp_class == actual_most_fp):
        print("\n  ✅ 정답!")
    else:
        print("\n  ❌ 다시 분석해보세요. 정답은 quiz_solutions/medium_sol.py 참고")


def problem3_hyperparameter_decision():
    """
    문제 3: Hyperparameter 의사결정

    아래 실험 결과를 보고 다음 실험에서 바꿀 설정을 결정하시오.

    TODO: next_experiment 딕셔너리를 채우시오.
    """
    print("\n" + "━" * 36)
    print("문제 3: Hyperparameter 의사결정")
    print("━" * 36 + "\n")

    experiments = [
        {"name": "Baseline", "model": "yolov8n", "imgsz": 640,
         "lr0": 0.01, "epochs": 50, "augmentation": "default",
         "map50": 0.48, "map50_95": 0.32, "train_loss": 0.8,
         "status": "val_mAP 정체"},
        {"name": "Exp-A", "model": "yolov8s", "imgsz": 640,
         "lr0": 0.01, "epochs": 50, "augmentation": "default",
         "map50": 0.55, "map50_95": 0.38, "train_loss": 0.5,
         "status": "개선됨"},
        {"name": "Exp-B", "model": "yolov8s", "imgsz": 640,
         "lr0": 0.01, "epochs": 100, "augmentation": "default",
         "map50": 0.57, "map50_95": 0.39, "train_loss": 0.15,
         "status": "과적합 의심"},
    ]

    print(f"  {'실험':10s} │ {'모델':8s} │ {'imgsz':5s} │ {'lr0':6s} │ "
          f"{'epochs':6s} │ {'mAP50':6s} │ {'mAP50-95':8s} │ {'t_loss':6s} │ {'상태':12s}")
    print("  " + "─" * 90)
    for exp in experiments:
        print(f"  {exp['name']:10s} │ {exp['model']:8s} │ {exp['imgsz']:5d} │ "
              f"{exp['lr0']:6.3f} │ {exp['epochs']:6d} │ {exp['map50']:6.2f} │ "
              f"{exp['map50_95']:8.2f} │ {exp['train_loss']:6.2f} │ {exp['status']:12s}")

    print("\n  분석:")
    print("    - Baseline → Exp-A: 모델 크기 증가 → 성능 향상")
    print("    - Exp-A → Exp-B: 에폭 증가 → 약간 향상, 하지만 train_loss 매우 낮음")
    print("    - Exp-B는 과적합 의심 (train_loss=0.15, map50_95=0.39)")
    print()

    print("  질문: 다음 실험(Exp-C)의 설정은?")
    print("    힌트: 과적합 해결 + 추가 성능 향상을 위해")
    print("    고려사항: Augmentation 강화? imgsz 증가? Early stopping?")
    print()

    # TODO: 다음 실험 설정을 결정하세요
    next_experiment = {
        "model": "",        # 여기를 채우시오
        "imgsz": 0,         # 여기를 채우시오
        "lr0": 0.0,         # 여기를 채우시오
        "augmentation": "", # 여기를 채우시오 ("default", "heavy", "light")
        "reason": "",       # 여기를 채우시오 (선택 이유)
    }

    print(f"  내 선택:")
    for key, value in next_experiment.items():
        print(f"    {key}: {value}")

    print("\n  모범 답안은 quiz_solutions/medium_sol.py 참고")
    print("  (정답이 하나가 아닌 주관식 문제입니다)")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 4 Quiz - Medium")
    print("━" * 40)
    problem1_label_conversion()
    problem2_confusion_matrix()
    problem3_hyperparameter_decision()
    print("\n" + "━" * 40)
    print("정답은 quiz_solutions/medium_sol.py 참고")
    print("━" * 40)
