"""Phase 6 Week 2 - 중급 정답"""
import math


def p1():
    fov = math.degrees(2 * math.atan(36 / (2 * 15)))
    print(f"\n정답: FOV ~ {fov:.1f} deg")
    print(f"  sensor 36mm + focal 15mm -> 100 deg wide-angle")
    print(f"  웹캠 (전체 뷰 카메라) 과 비슷한 wide FOV -- 실측은 Phase 6 week 7")


def p2():
    print("\n정답: Sim vs 실카메라 (전체 뷰 웹캠) 차이 (요약)")
    print("  Lens distortion: 웹캠의 barrel 왜곡 (수치는 캘리브레이션으로 구한다)")
    print("  Noise          : 센서 노이즈 (저조도에서 커진다)")
    print("  Exposure       : Sim 은 고정, 웹캠은 auto")
    print()
    print("  Phase 6 week 11 에서 image gap 정량 측정 + Domain Randomization 시작")


def p3():
    w, h, c = 640, 480, 3
    bytes_per = w * h * c
    print(f"\n정답: {bytes_per} bytes/frame")
    print(f"  = {bytes_per/1e3:.1f} KB/frame")
    print(f"  30 FPS = {bytes_per*30/1e6:.1f} MB/s")
    print(f"  ROS2 image_raw 의 일반 data rate")


if __name__ == "__main__":
    p1(); p2(); p3()
