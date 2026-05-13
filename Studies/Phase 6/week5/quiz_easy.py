"""Phase 6 Week 5 - 기초"""


def p1():
    print("\n문제 1: dof_names 순서 vs URDF 순서")
    print("  A) 항상 같음\n  B) 다를 수 있음 (매핑 표 필요)\n  C) 알파벳 순\n  D) 무작위")


def p2():
    print("\n문제 2: FK 검증 기준")
    print("  A) < 1 mm\n  B) < 1 cm\n  C) < 10 cm\n  D) 1 m")


def p3():
    print("\n문제 3: 단독 joint 동작 검증의 의의")
    print("  A) GPU 부하 측정\n  B) URDF 매핑 + 시각 정확성\n  C) Latency\n  D) Memory")


def p4():
    print("\n문제 4: Joint limit 가 무한 (지속 회전)")
    print("  A) Revolute\n  B) Continuous\n  C) Prismatic\n  D) Fixed")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
