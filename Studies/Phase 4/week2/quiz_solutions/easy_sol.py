"""
Phase 4 Week 2 - Co-fine-tuning + Action Tokenization 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) rare token 이라 원 의미 손실 최소")
    print("=" * 50 + "\n")

    print("해설:")
    print("  SentencePiece / BPE 의 vocab 정렬:")
    print("    - ID 작은 쪽: special token, 빈도 높은 sub-word ('the', 'a', '_')")
    print("    - ID 큰 쪽 : 빈도 낮은 sub-word (rare combination)")
    print()
    print("  RT-2 가 마지막 256개를 action 으로 재해석하면:")
    print("    - 원래 그 token 들이 web data 에 등장 빈도가 매우 낮음")
    print("    - embedding 이 web 학습에서 충분히 학습되지 못함")
    print("    - 새로 action 으로 의미 부여해도 web 성능 영향 미미")
    print()
    print("  반대로 vocab 의 처음 256개를 쓰면?")
    print("    - 'the', 'a', '.' 등 가장 빈도 높은 token")
    print("    - web 학습 결과 완전히 망가짐")
    print("    - catastrophic forgetting 보다도 직접적인 파괴")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: C) Web 80% + Robot 20%")
    print("=" * 50 + "\n")

    print("해설:")
    print("  RT-2 의 정확한 비율은 논문 mixture table 참고. 대략:")
    print("    - WebLI (image-caption) : 50%")
    print("    - OCR / VQA            : 30%")
    print("    - Robot trajectory     : 20%")
    print()
    print("  핵심: web data 의 absolute size 가 robot data 대비 압도적이지만,")
    print("       sample 등장 빈도 (mixture 비율) 는 8 : 2 정도로 조정.")
    print()
    print("  너무 web 쏠림 (보기 A): robot action 학습이 거의 안 됨")
    print("  너무 균형 (보기 B 50:50): web 의 다양성이 robot 쪽으로 끌려감")
    print("  너무 robot 쏠림 (보기 D): catastrophic forgetting")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) 동일한 standard next-token-prediction CE")
    print("=" * 50 + "\n")

    print("해설:")
    print("  RT-2 의 영리한 설계 중 하나:")
    print("    - Web 의 caption: image -> caption text 생성")
    print("    - Web 의 VQA   : image + Q -> A 생성")
    print("    - Robot trajectory: image + instruction -> action token 생성")
    print()
    print("  세 경우 모두 'image + text input -> text output' 의 형태.")
    print("  Loss 는 단일하게 standard next-token-prediction CE.")
    print()
    print("  [tip] 이게 RT-2 의 가장 영리한 점:")
    print("       별도 head / 별도 loss / 별도 training pipeline 이 필요 없음.")
    print("       모든 데이터가 LM 의 표준 학습 흐름으로 환원됨.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) 약 0.78 mm")
    print("=" * 50 + "\n")

    print("해설:")
    print("  계산:")
    print("    range = +0.1 - (-0.1) = 0.2 m")
    print("    step  = 0.2 / 256 = 0.00078125 m = 0.78125 mm")
    print()
    print("  이 수치의 직관:")
    print("    - cm 단위 manipulation (잡기/놓기): 충분")
    print("    - sub-mm 정밀 조립: 한계")
    print("    - 의료 수술 / 반도체 조립: 부적합")
    print()
    print("  [tip] 본 로드맵의 면접 포인트:")
    print("       'VLA 의 quantization step 0.78mm 는 자동차 부품 조립")
    print("        (tolerance ~0.1mm) 에는 부족하다' 같은 정량적 평가")
    print("       가 양산 SW 엔지니어의 차별점이다.")


if __name__ == "__main__":
    print("=" * 50)
    print("  Phase 4 Week 2 Quiz - Easy 정답")
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("\n" + "=" * 50)
