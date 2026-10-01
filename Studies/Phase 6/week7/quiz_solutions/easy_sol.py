"""Phase 6 Week 7 - 기초 정답"""


def p1():
    print("\n정답: B) External (전체 뷰 카메라)")
    print("  Stage 1 W3 에서 고정한 자리. 옮기면 v2.5 데이터셋과 시점이 달라진다. ee-mount 는 Phase 7 옵션")


def p2():
    print("\n정답: B) /dev/so101_cam_overview, MJPG, 1280x720, 30 fps")
    print("  /dev/video* 번호는 재부팅 · 재연결로 바뀌므로 so101-attach 가 만드는 고정 경로를 쓴다")
    print("  이 카메라가 지원하는 fps 는 30 뿐 — 60 을 적으면 failed to set fps=60 으로 멈춘다")
    print("  1280x480 은 지원 목록에 없는 해상도라 failed to set capture_width 로 멈춘다")


def p3():
    print("\n정답: A) Lighting / Color / Geometry / Noise")
    print("  week 11 의 측정 대상")


def p4():
    print("\n정답: B) Phase 7 산출물 #4 의 핵심 컨텐츠")
    print("  '같은 task 의 Real 과 Sim 동시 진행' 영상이 면접관에게 가장 어필")


if __name__ == "__main__":
    p1(); p2(); p3(); p4()
