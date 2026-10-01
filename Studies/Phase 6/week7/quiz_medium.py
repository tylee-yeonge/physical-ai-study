"""Phase 6 Week 7 - 중급"""


def p1():
    """Camera viewpoint 매칭"""
    print("\n문제 1: Real <-> Sim camera viewpoint 매칭")
    print("  Real 전체 뷰 카메라는 팔로워의 대각선 앞쪽 위에서 약 45도로 내려다본다 (위치는 줄자로 잰다)")
    print("  Sim Camera 도 같은 위치 + orientation 필요")
    print()
    print("  매칭 검증:")
    print("  - 같은 image 위치에 자작 팔의 base 가 보임")
    print("  - perspective 같음 (FOV)")
    print()
    print("  Trial-and-error: 자작 팔의 알려진 landmark 가 같은 픽셀 위치에")


def p2():
    """FOV 매칭"""
    print("\n문제 2: 전체 뷰 카메라의 HFOV 를 재서 Sim focal 로 바꾸기")
    print("  렌즈 앞 D = 0.60 m 에 줄자를 놓았더니 화면 가로에 L = 0.92 m 가 들어왔다")
    print("  Sim 카메라의 aperture (get_horizontal_aperture) 는 20.955 mm 였다")
    print()
    print("  (1) HFOV 는 몇 도인가?")
    print("  (2) 같은 화각이 되는 focal length 는 몇 mm 인가?")
    print("  (3) 사양서의 '대각선 화각 90도' 를 그대로 HFOV 로 쓰면 왜 틀리는가?")


def p3():
    """Sim 의 image 품질 trade-off"""
    print("\n문제 3: Sim 의 rendering 품질 vs Latency")
    print("  RayTracedLighting: 사실적, 느림 (~50ms per frame)")
    print("  PathTracing       : 가장 사실적, 매우 느림 (~수초)")
    print("  RasterizedRendering: 빠름 (~5ms), 사실성 떨어짐")
    print()
    print("  본 phase 권장: RayTracedLighting (latency 와 사실성 균형)")


if __name__ == "__main__":
    p1(); p2(); p3()
