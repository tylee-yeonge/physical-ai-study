"""Phase 5 Week 9 - 중급 (OpenVLA backbone)"""


def p1():
    """Vision tokens 와 Llama context 비율"""
    print("\n문제 1: Llama context 의 vision 비중")
    vis_tokens = 256 + 196  # 452
    text_tokens = 30  # 평균 instruction
    total = vis_tokens + text_tokens

    # TODO: vision 비중
    vis_pct = 0.0
    print(f"  Vision: {vis_tokens}, Text: {text_tokens}, Total: {total}")
    print(f"  당신: {vis_pct:.1f}%")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """LoRA 학습 시 trainable params 계산"""
    print("\n문제 2: LoRA fine-tune 의 trainable params")
    projector = 30e6  # 30M
    llama_lora_r32 = 65e6  # ~ 65M
    # TODO
    total = 0.0
    print(f"  당신: {total/1e6:.1f}M")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


def p3():
    """OpenVLA inference 시 prompt 길이"""
    print("\n문제 3: OpenVLA inference 의 prompt 길이")
    print("  Vision tokens (224 input)  : 452")
    print("  System prompt              : 10")
    print("  User instruction           : 20")
    print("  Reserve for response       : 20")
    # TODO
    total = 0
    print(f"  당신: {total}")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    p1(); p2(); p3()
