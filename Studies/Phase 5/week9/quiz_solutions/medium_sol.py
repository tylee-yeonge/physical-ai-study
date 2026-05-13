"""Phase 5 Week 9 - 중급 정답"""


def p1():
    vis = 452
    text = 30
    total = vis + text
    pct = vis / total * 100
    print(f"\n정답: vision 비중 {pct:.1f}%")
    print(f"  Vision tokens 가 prompt 의 ~94%")
    print(f"  Text 는 6% 미만")
    print()
    print("  의미: vision 정보가 압도적, text instruction 은 짧음")


def p2():
    proj = 30e6
    lora = 65e6
    total = proj + lora
    base = 7e9
    pct = total / base * 100
    print(f"\n정답: trainable params = {total/1e6:.1f}M ({pct:.2f}%)")
    print(f"  Projector  : {proj/1e6:.1f}M")
    print(f"  Llama LoRA : {lora/1e6:.1f}M")
    print(f"  Total      : {total/1e6:.1f}M")
    print(f"  Base 7B 대비 : {pct:.2f}%")
    print()
    print("  Phase 7 LoRA 학습 시:")
    print("    ~95M parameters 학습 -> GPU 메모리 적게")
    print("    학습 시간 짧음 (5~10 시간)")
    print("    Demonstrations 적어도 OK (~ 100~500)")


def p3():
    print("\n정답: 502 tokens")
    print("  Vision 452 + system 10 + instruction 20 + response 20 = 502")
    print()
    print("  Llama 2 의 max context 4096:")
    print("    OpenVLA 의 prompt 502 는 12% 만 사용")
    print("    -> 더 긴 instruction / multi-image 도 가능")
    print()
    print("  단, 매 inference 마다 462 vision tokens 처리:")
    print("    KV cache 크기 큼 (vision token 의 K, V 저장)")
    print("    Memory: 502 * 4096 * 2 * 32 layer * 2 byte = ~ 260 MB")


if __name__ == "__main__":
    p1(); p2(); p3()
