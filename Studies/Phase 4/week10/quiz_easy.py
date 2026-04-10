"""
Phase 6 Week 10 - BEVFormer 실습 기초 퀴즈
"""


def problem1_nds_components():
    print("\n" + "━" * 28)
    print("문제 1: NDS 구성 요소")
    print("━" * 28 + "\n")

    print("질문: NDS(nuScenes Detection Score)에 포함되지 않는")
    print("      메트릭은?\n")

    print("보기:")
    print("  A) mAP (mean Average Precision)")
    print("  B) mATE (mean Average Translation Error)")
    print("  C) mAVE (mean Average Velocity Error)")
    print("  D) mFPS (mean Frames Per Second)")


def problem2_nuscenes_matching():
    print("\n" + "━" * 28)
    print("문제 2: nuScenes 매칭 기준")
    print("━" * 28 + "\n")

    print("질문: nuScenes mAP에서 예측과 GT를 매칭하는 기준은?\n")

    print("보기:")
    print("  A) 3D IoU (3D 겹침 비율)")
    print("  B) 2D IoU (이미지 상의 겹침)")
    print("  C) BEV 중심점 거리 (여러 threshold)")
    print("  D) 코사인 유사도")


def problem3_bev_feature_shape():
    print("\n" + "━" * 28)
    print("문제 3: BEV Feature Map Shape")
    print("━" * 28 + "\n")

    print("질문: BEVFormer-Base의 BEV Feature Map 크기는?\n")

    print("보기:")
    print("  A) 100 x 100 x 128")
    print("  B) 200 x 200 x 256")
    print("  C) 400 x 400 x 512")
    print("  D) 50 x 50 x 64")


def problem4_class_performance():
    print("\n" + "━" * 28)
    print("문제 4: 클래스별 성능 차이")
    print("━" * 28 + "\n")

    print("질문: BEVFormer에서 일반적으로 가장 높은 AP를 보이는")
    print("      클래스와 그 이유는?\n")

    print("보기:")
    print("  A) Pedestrian - 움직임이 예측 가능하므로")
    print("  B) Car - 크기가 크고 학습 데이터가 가장 많으므로")
    print("  C) Traffic Cone - 형태가 단순하므로")
    print("  D) Bicycle - 도로에서 눈에 띄므로")


if __name__ == "__main__":
    print("━" * 33)
    print("  Week 10 Quiz - Easy")
    print("━" * 33)
    problem1_nds_components()
    problem2_nuscenes_matching()
    problem3_bev_feature_shape()
    problem4_class_performance()
    print("\n" + "━" * 33)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━" * 33)
