"""
Phase 4 Week 6 - HuggingFace + Inference 기초 퀴즈
"""


def problem1_quantization():
    print("\n" + "=" * 50)
    print("문제 1: 4-bit quantization 의 라이브러리")
    print("=" * 50 + "\n")
    print("질문: OpenVLA 의 4-bit nf4 quantization 에 가장 자주 쓰이는 라이브러리?\n")
    print("보기:")
    print("  A) bitsandbytes")
    print("  B) GPTQ")
    print("  C) AWQ")
    print("  D) torch.quantization")


def problem2_predict_action():
    print("\n" + "=" * 50)
    print("문제 2: OpenVLA 의 inference API")
    print("=" * 50 + "\n")
    print("질문: OpenVLA 에서 action 추론 시 호출하는 method 는?\n")
    print("보기:")
    print("  A) model.forward()")
    print("  B) model.generate()")
    print("  C) model.predict_action() (custom method)")
    print("  D) model.predict()")


def problem3_unnorm_key():
    print("\n" + "=" * 50)
    print("문제 3: unnorm_key 의 의미")
    print("=" * 50 + "\n")
    print("질문: predict_action(..., unnorm_key='bridge_orig') 의 의미는?\n")
    print("보기:")
    print("  A) 모델 weights 의 normalization key")
    print("  B) OpenX-Embodiment 의 어떤 dataset 의 action normalization 통계로")
    print("     de-normalize 할지 지정")
    print("  C) 이미지 preprocess 의 mean/std 지정")
    print("  D) tokenizer 의 vocab 지정")


def problem4_latency_component():
    print("\n" + "=" * 50)
    print("문제 4: Latency 의 가장 큰 component")
    print("=" * 50 + "\n")
    print("질문: OpenVLA inference 의 한 frame latency (~150 ms) 에서")
    print("      가장 큰 비중을 차지하는 component 는?\n")
    print("보기:")
    print("  A) Image preprocess (resize, normalize)")
    print("  B) Vision encoder forward (DINOv2 + SigLIP)")
    print("  C) LM decoder generate (token 단위 autoregressive)")
    print("  D) Action de-tokenize")


if __name__ == "__main__":
    print("=" * 50)
    problem1_quantization()
    problem2_predict_action()
    problem3_unnorm_key()
    problem4_latency_component()
    print("=" * 50)
    print("정답은 quiz_solutions/easy_sol.py 참고")
    print("=" * 50)
