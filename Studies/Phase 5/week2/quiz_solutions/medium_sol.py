"""
Phase 5 Week 2 - 중급 퀴즈 정답
"""


def p1():
    print("\n문제 1 정답: throughput")
    print(f"  ViT-B: 1000/10  = 100   Hz")
    print(f"  ViT-L: 1000/30  =  33.3 Hz")
    print()
    print("  [tip] 1 FPS = 30Hz 자율주행 perception 에는:")
    print("    - ViT-B: 충분히 가능 (33ms cycle 대비 10ms)")
    print("    - ViT-L: 간신히 가능 (33ms 대비 30ms)")
    print("    - ViT-H: 60ms > 33ms -> 30Hz 불가")


def p2():
    print("\n문제 2 정답: ViT-L VRAM")
    n = 300e6
    bytes_pp = 2
    act = 1.5
    vram = n * bytes_pp / 1e9 * act
    print(f"  300M * 2 byte = 600 MB (model)")
    print(f"  * 1.5 (activation 추가) = {vram:.2f} GB")
    print()
    print("  실제 RTX 4070 기준:")
    print("    - fp16 모델: ~ 0.6 GB")
    print("    - activation: ~ 0.3 GB")
    print("    - total: ~ 1.2 GB (batch=1)")


def p3():
    print("\n문제 3 정답: input scale 영향")
    ratio = (577 ** 2) / (197 ** 2)
    print(f"  Seq: 197 -> 577 ({577/197:.2f}x)")
    print(f"  Attention FLOPS: 197^2 -> 577^2")
    print(f"  Ratio = (577/197)^2 = {ratio:.2f}x")
    print()
    print(f"  의미: 384x384 image -> attention FLOPS ~ 8.6x 증가")
    print(f"        latency 도 비슷한 비율 (실제 5~7x, FFN 은 sequence 에 linear)")
    print()
    print("  [tip] OpenVLA 의 384x384 사용 이유:")
    print("        높은 해상도 -> 더 정확한 spatial reasoning")
    print("        latency cost 감수 (~ 6x)")


if __name__ == "__main__":
    p1(); p2(); p3()
