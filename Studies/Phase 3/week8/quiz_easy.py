"""
Quiz Easy - Week 8: Depth Anything 사용 (Section 5.3)
4문제 - 기본 개념 확인
"""


def problem1_huggingface_pipeline():
    print("\n" + "━" * 28)
    print("문제 1: HuggingFace Pipeline")
    print("━" * 28 + "\n")

    print("Depth Anything 모델을 HuggingFace Pipeline으로")
    print("추론하는 코드의 빈칸을 채우세요.")
    print()
    print("  from transformers import _______")
    print()
    print("  depth_pipe = _______(")
    print("      task='_______',")
    print("      model='LiheYoung/depth-anything-small-hf'")
    print("  )")
    print()
    print("  result = depth_pipe('image.jpg')")
    print("  depth_map = result['_______']")
    print()
    print("  답: _____")
    print()


def problem2_depth_map_interpretation():
    print("\n" + "━" * 28)
    print("문제 2: 깊이맵 해석")
    print("━" * 28 + "\n")

    print("Depth Anything의 출력 깊이맵에서 다음 설명 중 올바른 것을 모두 고르세요.")
    print()
    print("  a) 깊이맵의 큰 값은 가까운 물체를 나타낸다")
    print("  b) 깊이맵의 출력은 항상 미터 단위이다")
    print("  c) 깊이맵은 단일 채널 (H x W) 형태이다")
    print("  d) 깊이맵의 값 범위는 항상 0~255이다")
    print("  e) 같은 이미지라도 모델 크기에 따라 깊이맵이 다를 수 있다")
    print()
    print("  답: _____")
    print()


def problem3_colormap_selection():
    print("\n" + "━" * 28)
    print("문제 3: 컬러맵 선택")
    print("━" * 28 + "\n")

    print("깊이맵 시각화에 자주 사용되는 컬러맵과 그 특징을 연결하세요.")
    print()
    print("  컬러맵:")
    print("  1. magma")
    print("  2. turbo")
    print("  3. viridis")
    print()
    print("  특징:")
    print("  A. 파랑->초록->빨강, 직관적 (빨강=가까움)")
    print("  B. 보라->초록->노랑, 색각 이상 친화적")
    print("  C. 어두운 보라->밝은 노랑, 논문에서 가장 많이 사용")
    print()
    print("  답: 1=__, 2=__, 3=__")
    print()


def problem4_model_selection():
    print("\n" + "━" * 28)
    print("문제 4: 모델 크기 선택")
    print("━" * 28 + "\n")

    print("Depth Anything의 3가지 크기 중 다음 상황에 적합한 모델을 고르세요.")
    print()
    print("  상황 A: Jetson Orin Nano에서 실시간 (15+ FPS)")
    print("  상황 B: 서버에서 최고 품질 깊이맵 생성")
    print("  상황 C: 데스크톱 GPU에서 속도와 품질 균형")
    print()
    print("  선택지: ViT-S (24.8M) / ViT-B (97.5M) / ViT-L (335.3M)")
    print()
    print("  답: A=_____, B=_____, C=_____")
    print()


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 8 Quiz - Easy (Depth Anything 사용)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    problem1_huggingface_pipeline()
    problem2_depth_map_interpretation()
    problem3_colormap_selection()
    problem4_model_selection()

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
