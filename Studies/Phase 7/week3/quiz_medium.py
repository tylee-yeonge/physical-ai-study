"""Phase 7 Week 3 - 중급"""


def p1():
    print("\n문제 1: Sim success rate vs Real")
    print("  Sim ~ 80% (LoRA fine-tuned)")
    print("  Real ~ 60-70% (Sim2Real gap)")
    print("  Domain Randomization 으로 Gap 줄임")


def p2():
    print("\n문제 2: Sim 의 latency vs Real")
    print("  Sim closed loop: ~ 35 ms step + inference")
    print("  Real closed loop: ~ 8 ms motor + inference")
    print("  Sim 이 약간 느림")


def p3():
    print("\n문제 3: Sim 만으로 산출물 #4?")
    print("  안 됨. 'Sim only 산출물은 박사도 만든다'")
    print("  Real 자작 팔이 차별점 (양산 SW 엔지니어의 차별화)")


if __name__ == "__main__":
    p1(); p2(); p3()
