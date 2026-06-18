"""Phase 5 Week 11 - 중급 (통합)"""


def p1():
    """Vision encoder 의 비중"""
    print("\n문제 1: OpenVLA 의 vision encoder 비중")
    total = 7.6e9
    vision = 630e6
    # TODO
    pct = 0.0
    print(f"  Vision encoder: {vision/1e6:.0f}M")
    print(f"  Total OpenVLA : {total/1e9:.1f}B")
    print(f"  당신: {pct:.2f}%")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """Phase 7 LoRA 의 GPU memory"""
    print("\n문제 2: Phase 7 LoRA 학습 시 GPU memory")
    base_int4 = 4.0
    lora_fp16 = 0.2
    activation = 4.0
    optimizer_3x = 3 * 0.2
    overhead = 1.0
    # TODO
    total = 0.0
    print(f"  당신: {total:.2f} GB")
    print(f"  RTX 4070 12GB fit: {total < 12}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """4 vision foundation model 의 OpenVLA 영향"""
    print("\n문제 3: 4 모델의 OpenVLA 직접 사용 매트릭스")
    print("  ViT     : ?")
    print("  CLIP    : ?")
    print("  DINOv2  : ?")
    print("  SigLIP  : ?")
    print()
    # TODO ('used' / 'foundation' / 'not used')
    mapping = {"ViT": "", "CLIP": "", "DINOv2": "", "SigLIP": ""}
    for k in mapping:
        print(f"  {k} -> 당신: {mapping[k]}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    p1(); p2(); p3()
