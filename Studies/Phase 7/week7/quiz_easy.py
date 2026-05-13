"""기초"""


def p1():
    print("\n문제 1: Latency measurement 의 시작점")
    print("  A) Image header.stamp\n  B) inference 시작\n  C) Random\n  D) 무관")


def p2():
    print("\n문제 2: Total end-to-end 기대")
    print("  A) ~ 50 ms\n  B) ~ 184 ms\n  C) ~ 1 s\n  D) 10 s")


def p3():
    print("\n문제 3: Dominant component")
    print("  A) Image capture\n  B) OpenVLA inference\n  C) IK\n  D) Motor")


def p4():
    print("\n문제 4: Sim vs Real latency")
    print("  A) Sim 이 항상 빠름")
    print("  B) Real 이 약간 빠름 (Sim 의 step 35 ms 추가)")
    print("  C) 같음")
    print("  D) 무관")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
