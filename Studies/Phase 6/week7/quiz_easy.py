"""Phase 6 Week 7 - 기초"""


def p1():
    print("\n문제 1: 본 phase 권장 카메라 위치")
    print("  A) ee-mount only\n  B) External (전체 뷰 카메라)\n  C) None\n  D) Multi-camera")


def p2():
    print("\n문제 2: 전체 뷰 카메라를 OpenCV 로 열 때 맞는 설정")
    print("  A) /dev/video0, YUYV, 1280x720, 60 fps")
    print("  B) /dev/so101_cam_overview, MJPG, 1280x720, 30 fps")
    print("  C) /dev/so101_cam_overview, MJPG, 1280x480, 60 fps")
    print("  D) /dev/video2, MJPG, 640x480, 25 fps")


def p3():
    print("\n문제 3: 시각 gap 4 차원")
    print("  A) Lighting / Color / Geometry / Noise")
    print("  B) Speed / Memory / Disk")
    print("  C) X / Y / Z / W")
    print("  D) Lens 만")


def p4():
    print("\n문제 4: Side-by-side 의 가치")
    print("  A) 화면 크기 늘리기")
    print("  B) Phase 7 산출물 #4 의 핵심 컨텐츠 (Real vs Sim)")
    print("  C) GPU 빠름")
    print("  D) Memory 적음")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
