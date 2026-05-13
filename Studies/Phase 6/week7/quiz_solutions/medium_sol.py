"""Phase 6 Week 7 - 중급 정답"""
import math


def p1():
    print("\n정답: Camera viewpoint 매칭")
    print("  Trial-and-error: 자작 팔의 base 가 같은 픽셀 위치에")
    print("  + landmark (지정된 point) 가 같은 픽셀 매칭")


def p2():
    elp_sensor = 5.86
    elp_focal = 3.6
    fov = math.degrees(2 * math.atan(elp_sensor / (2 * elp_focal)))
    sim_focal = 36 / (2 * math.tan(math.radians(fov/2)))
    print(f"\n정답: FOV ~ {fov:.1f} deg")
    print(f"  Sim focal length (sensor 36mm 가정): {sim_focal:.1f} mm")


def p3():
    print("\n정답: RayTracedLighting 권장")
    print("  Phase 6 의 디지털 트윈은 실시간 (~ 30 FPS) 필요")
    print("  RayTracedLighting 가 사실성 + 속도 균형")
    print("  PathTracing 은 영상 capture 용 (1 frame ~ 수초)")


if __name__ == "__main__":
    p1(); p2(); p3()
