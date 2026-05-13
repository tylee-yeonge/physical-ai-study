"""Phase 6 Week 5 - 기초 정답"""


def p1():
    print("\n정답: B) 다를 수 있음")
    print("  Isaac Sim 은 알파벳 또는 USD 순서로 dof_names 정렬")
    print("  URDF 와 다를 수 있어 매핑 표 필수")


def p2():
    print("\n정답: A) < 1 mm")
    print("  자작 팔 (cm 단위 manipulation) 기준")
    print("  더 정밀한 작업 (자동차 부품) 은 < 0.1 mm 필요")


def p3():
    print("\n정답: B) URDF 매핑 + 시각 정확성")
    print("  각 joint 가 의도된 axis 로 움직이는가")
    print("  Mesh 가 의도된 link 에 부착되었는가")


def p4():
    print("\n정답: B) Continuous")
    print("  URDF type:")
    print("    revolute  : limit 있음")
    print("    continuous: limit 없음 (지속 회전)")
    print("    prismatic : 선형")
    print("    fixed     : 고정")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
