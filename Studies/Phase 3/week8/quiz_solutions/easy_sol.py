"""
Solutions - Easy Quiz (Week 8: Depth Anything 사용)
"""


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("Week 8 Quiz - Easy 정답")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")

    print("Q1. HuggingFace Pipeline 코드 완성:")
    print()
    print("  from transformers import pipeline")
    print()
    print("  depth_pipe = pipeline(")
    print("      task='depth-estimation',")
    print("      model='LiheYoung/depth-anything-small-hf'")
    print("  )")
    print()
    print("  result = depth_pipe('image.jpg')")
    print("  depth_map = result['depth']")
    print()
    print("  빈칸:")
    print("  1번: pipeline (함수 이름)")
    print("  2번: pipeline (호출)")
    print("  3번: depth-estimation (태스크)")
    print("  4번: depth (결과 키)\n")

    print("Q2. 정답: a, c, e")
    print("    a) 맞음: Depth Anything 기본 출력에서 큰 값 = 가까운 물체")
    print("       (disparity와 유사한 표현)")
    print("    b) 틀림: 기본 출력은 상대 깊이이며, 미터 단위가 아님")
    print("       (Metric 모델을 별도 사용해야 미터 단위)")
    print("    c) 맞음: 깊이맵은 (H, W) 형태의 단일 채널")
    print("    d) 틀림: 모델의 raw 출력은 임의 범위")
    print("       (Pipeline 후처리에서 0~255로 변환하는 경우도 있음)")
    print("    e) 맞음: ViT-S, ViT-B, ViT-L 모델마다 예측이 다를 수 있음")
    print("       (큰 모델이 더 정밀하고 세밀한 경계 표현)\n")

    print("Q3. 정답: 1=C, 2=A, 3=B")
    print("    1. magma = C (어두운 보라 -> 밝은 노랑, 논문 표준)")
    print("    2. turbo = A (파랑 -> 초록 -> 빨강, 직관적)")
    print("    3. viridis = B (보라 -> 초록 -> 노랑, 색각 이상 친화적)\n")

    print("Q4. 정답:")
    print("    A = ViT-S (24.8M)")
    print("      이유: Jetson 메모리/연산 제한에서 유일하게 실시간 가능")
    print("            TensorRT FP16으로 ~50ms (15-20 FPS)")
    print()
    print("    B = ViT-L (335.3M)")
    print("      이유: 서버급 GPU에서 최고 품질 깊이맵 생성")
    print("            파라미터가 많아 세밀한 경계/깊이 표현 가능")
    print()
    print("    C = ViT-B (97.5M)")
    print("      이유: 데스크톱 GPU에서 ~25ms (40 FPS) 수준")
    print("            ViT-S보다 품질 좋고, ViT-L보다 빠름")

    print("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")


if __name__ == "__main__":
    main()
