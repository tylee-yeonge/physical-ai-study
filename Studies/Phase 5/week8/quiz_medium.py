"""Phase 5 Week 8 - 중급 (DINOv2)"""


def p1():
    """DINOv2 feature 차원과 PCA"""
    print("\n문제 1: DINOv2 patch feature PCA 출력")
    print("  DINOv2 ViT-L: 1024 dim per patch")
    print("  PCA 후 3 dim 시각화")
    print()
    # TODO: 정보 손실량
    pca3_pct = 0  # PCA 3 dim 이 explain 하는 variance %
    print(f"  당신: {pca3_pct}%")
    print()
    print("  [tip] 시각화에는 PCA 3 dim 으로 충분, 정확한 분석은 더 많은 dim 필요.")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """OpenVLA 의 DINOv2 + SigLIP latency 측정"""
    print("\n문제 2: OpenVLA vision encoder latency")
    dino = 28
    sig = 28
    # TODO
    seq_total = 0
    par_total = 0
    print(f"  순차: {seq_total} ms")
    print(f"  병렬: {par_total} ms")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """DINOv2 ViT-G 가 OpenVLA 에 안 쓰이는 이유"""
    print("\n문제 3: 왜 ViT-G/14 (1.1B) 안 쓰는가")
    print("  Options:")
    print("    A) accuracy 가 낮아서")
    print("    B) VRAM + latency 부담 (1.1B fp16 = 2.2GB, latency ~ 100ms)")
    print("    C) 라이선스 문제")
    print("    D) 학습 데이터 부족")
    print()
    # TODO
    answer = ""
    print(f"  당신: {answer}")
    print()
    print("  [tip] OpenVLA 의 hardware budget:")
    print("    - 7B Llama (~ 14GB fp16, 4GB int4)")
    print("    - vision encoder (~ 2GB)")
    print("    - 합쳐서 4070 12GB 에 fit 해야")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    p1(); p2(); p3()
