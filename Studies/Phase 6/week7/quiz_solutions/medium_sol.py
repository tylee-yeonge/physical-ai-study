"""Phase 6 Week 7 - 중급 정답"""
import math


def p1():
    print("\n정답: Camera viewpoint 매칭")
    print("  위치는 줄자로 잰 (x, y, z) 를 그대로 넣고, 각도는 trial-and-error")
    print("  자작 팔의 base 가 같은 픽셀 위치에 + landmark (지정된 point) 가 같은 픽셀 매칭")


def p2():
    D = 0.60  # 렌즈에서 줄자까지 거리 (m)
    L = 0.92  # 화면 가로에 들어온 폭 (m)
    aperture = 20.955  # Sim 카메라의 센서 가로 폭 (mm)
    hfov = 2 * math.atan(L / (2 * D))  # 가로 화각 (rad)
    sim_focal = aperture / (2 * math.tan(hfov / 2))  # 같은 화각이 되는 초점거리 (mm)
    print(f"\n정답: (1) HFOV = 2 * atan(0.92 / 1.20) = {math.degrees(hfov):.1f} deg")
    print(f"  (2) focal = 20.955 / (2 * tan({math.degrees(hfov) / 2:.1f} deg)) = {sim_focal:.2f} mm")
    print("  (3) 사양서의 화각은 대각선 기준인 경우가 많다. 16:9 프레임에서 대각선 90도는 가로 약 80도다")
    print("      Sim 의 focal 은 가로 aperture 와 짝이므로 가로 화각 (HFOV) 으로 계산해야 한다")


def p3():
    print("\n정답: RayTracedLighting 권장")
    print("  Phase 6 의 디지털 트윈은 실시간 (~ 30 FPS) 필요")
    print("  RayTracedLighting 가 사실성 + 속도 균형")
    print("  PathTracing 은 영상 capture 용 (1 frame ~ 수초)")


if __name__ == "__main__":
    p1(); p2(); p3()
