"""기초"""


def p1():
    print("\n문제 1: 안전 인터록 의 언어")
    print("  A) Python\n  B) C++ (1-2 ms 빠름, 양산 표준)\n  C) Rust\n  D) Go")


def p2():
    print("\n문제 2: 한계 종류")
    print("  A) Position only\n  B) Position / Velocity / Torque\n  C) Velocity only\n  D) None")


def p3():
    print("\n문제 3: Torque 한계")
    print("  A) 모터 max 의 80%\n  B) 무한\n  C) 모터 max 의 200%\n  D) 0")


def p4():
    print("\n문제 4: 인터록 오버헤드")
    print("  A) ~ 100 ms\n  B) ~ 1 ms (C++)\n  C) ~ 1 s\n  D) 무관")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
