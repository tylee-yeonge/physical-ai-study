"""중급"""


def p1():
    print("\n문제 1: Sim vs Real success rate gap")
    print("  Sim 80% - Real 65% = 15% gap")
    print("  주 원인: image noise (DR 부족), force friction")


def p2():
    print("\n문제 2: 산출물 #4 의 정량 메시지")
    print("  '자작 6DOF 팔에 OpenVLA fine-tune 후:'")
    print("  '- Real success rate 65%'")
    print("  '- Closed loop latency 184 ms'")
    print("  '- 안전 인터록 오버헤드 1 ms'")
    print("  '- Sim/Real gap 4가지 정량 측정'")
    print()
    print("  이게 양산 SW 차별화 (\"VLA latency / 안전 / 비용\") 의 직접 증거")


def p3():
    print("\n문제 3: Phase 7 의 본 phase 종합")
    print("  - LoRA fine-tune 완료")
    print("  - ROS2 통합 + 안전 인터록")
    print("  - Sim/Real gap 정량")
    print("  - 산출물 #4 영상의 핵심 데이터")


if __name__ == "__main__":
    p1(); p2(); p3()
