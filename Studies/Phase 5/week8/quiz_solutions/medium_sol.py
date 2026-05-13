"""Phase 5 Week 8 - 중급 정답"""


def p1():
    print("\n정답: PCA 3 dim ~ 30~50% variance")
    print("  1024 dim 의 PCA 후 3 dim:")
    print("    - 시각화 목적으로 충분")
    print("    - 정확한 정보는 손실 (top 3 component 만)")
    print()
    print("  [tip] 더 정확한 분석:")
    print("    - top 10~20 PCA 로 클러스터링")
    print("    - t-SNE / UMAP 로 2D embedding")


def p2():
    seq = 28 + 28
    par = max(28, 28)
    print(f"\n정답: 순차 {seq}, 병렬 {par}")
    print(f"  Saving = {seq - par} ms (이론)")
    print(f"  실제 4070: ~ 5~10 ms 절약 (SM 부족)")


def p3():
    print("\n정답: B) VRAM + latency 부담")
    print("  ViT-G/14 (1.1B):")
    print("    fp16 VRAM: ~ 2.2 GB")
    print("    latency  : ~ 100 ms")
    print()
    print("  OpenVLA hardware budget (RTX 4070):")
    print("    Llama 7B int4   : ~ 4 GB")
    print("    DINOv2 ViT-L    : ~ 1.2 GB")
    print("    SigLIP ViT-L    : ~ 1.2 GB")
    print("    activation      : ~ 4 GB")
    print("    -> total ~ 10.5 GB (4070 12GB 에 fit)")
    print()
    print("  ViT-G 대체 시:")
    print("    + 4.4 GB (2개 ViT-G) -> 14.9 GB -> OOM")
    print("    latency ~ 250 ms (vs 165 ms)")
    print("    -> hardware budget 초과")


if __name__ == "__main__":
    p1(); p2(); p3()
