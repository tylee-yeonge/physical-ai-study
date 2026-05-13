"""Phase 6 Week 7 - 중급"""


def p1():
    """Camera viewpoint 매칭"""
    print("\n문제 1: Real <-> Sim camera viewpoint 매칭")
    print("  Real ELP 가 자작 팔의 앞 0.5m, 0.4m 높이")
    print("  Sim Camera 도 같은 위치 + orientation 필요")
    print()
    print("  매칭 검증:")
    print("  - 같은 image 위치에 자작 팔의 base 가 보임")
    print("  - perspective 같음 (FOV)")
    print()
    print("  Trial-and-error: 자작 팔의 알려진 landmark 가 같은 픽셀 위치에")


def p2():
    """FOV 매칭"""
    print("\n문제 2: ELP 의 FOV vs Sim FOV")
    import math
    elp_sensor = 5.86  # ELP CMOS 약 5.86 mm
    elp_focal = 3.6  # 대략
    real_fov = math.degrees(2 * math.atan(elp_sensor / (2 * elp_focal)))
    print(f"  ELP estimated FOV: {real_fov:.1f} deg")
    print()
    print("  Sim 의 focal_length 조정:")
    print("    sensor_w 36mm (default) 가정")
    print(f"    focal = 36 / (2 * tan({real_fov/2:.1f}deg))")


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
