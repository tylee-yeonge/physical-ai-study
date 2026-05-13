"""Phase 6 Week 2 - 기초 정답"""


def p1():
    print("\n정답: B) numpy [x, y, z]")
    print("  Isaac Sim 의 좌표는 numpy 벡터")
    print("  X (right), Y (forward), Z (up) - 보통")


def p2():
    print("\n정답: A) /World/cube")
    print("  USD path = Linux file path 비슷")
    print("  / 시작, slash 로 hierarchy")


def p3():
    print("\n정답: A) color, metallic, roughness")
    print("  OmniPBR (Physically-Based Rendering):")
    print("    color: base albedo")
    print("    metallic: 0 (non-metal) ~ 1 (metal)")
    print("    roughness: 0 (mirror) ~ 1 (rough)")


def p4():
    print("\n정답: B) Lumens")
    print("  Indoor 1000~5000")
    print("  자연광 outdoor 10000+")
    print("  Sim/Real 시각 gap 의 첫 원인")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
