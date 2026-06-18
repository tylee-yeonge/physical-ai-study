"""Phase 6 Week 2 - 중급"""


def p1():
    """Camera 의 focal length 계산"""
    print("\n문제 1: Camera FOV 계산")
    sensor_w = 36.0  # mm
    focal_mm = 15.0
    # FOV (horizontal) = 2 * atan( sensor_w / (2 * focal) )
    print(f"  Sensor width: {sensor_w} mm")
    print(f"  Focal length: {focal_mm} mm")
    # TODO
    your_fov = 0.0
    print(f"  FOV: {your_fov:.1f} deg")
    print("\n  정답은 quiz_solutions/medium_sol.py 참고")


def p2():
    """Sim 의 image 와 Real 의 image 차이"""
    print("\n문제 2: Sim image vs Real ELP Stereo")
    print()
    print("  주요 차이:")
    print("  - Lens distortion (ELP 의 barrel distortion)")
    print("  - 노이즈 (sensor noise)")
    print("  - White balance / exposure")
    print("  - 광원 색온도")
    print()
    print("  Phase 6 week 11 에서 정량 측정")


def p3():
    """Camera resolution 의 Sim 메모리"""
    print("\n문제 3: 640x480 RGB camera 의 메모리")
    w, h, c = 640, 480, 3
    bytes_per_pixel = 1  # uint8
    # TODO
    bytes_total = 0
    print(f"  당신: {bytes_total} bytes")
    print("  정답은 quiz_solutions/medium_sol.py 참고")


if __name__ == "__main__":
    p1(); p2(); p3()
