"""Phase 6 Week 1 - 중급 정답"""


def p1():
    sim = 4.0
    vla = 5.3
    bridge = 0.5
    print(f"\n정답: VRAM")
    print(f"  Sim only          : {sim} GB")
    print(f"  + OpenVLA + Bridge : {sim + vla + bridge} GB (~9.8 GB)")
    print(f"  RTX 4070 12GB :")
    print(f"    Sim only: 8 GB 여유")
    print(f"    + OpenVLA: 2.2 GB 여유 (마진 적음)")
    print()
    print("  Phase 7 의 산출물 #4 에서 동시 실행 시:")
    print("    - Sim render 해상도 줄이기")
    print("    - 또는 Sim / OpenVLA 시간 분할")


def p2():
    print("\n정답: B) Conda + pip")
    print()
    print("  Omniverse Launcher 의 한계:")
    print("    - GUI 필수 (X server)")
    print("    - 출장지 ssh 어려움")
    print("    - 큰 disk 사용")
    print()
    print("  Conda + pip:")
    print("    - 순수 Python 패키지")
    print("    - ssh 호환")
    print("    - 가벼움")
    print("    - Isaac Sim 4.x 부터 권장")


def p3():
    print("\n정답: URDF -> USD 변환")
    print()
    print("  URDF (XML 형식):")
    print("    - ROS 의 표준")
    print("    - link / joint / collision / visual 정의")
    print()
    print("  USD:")
    print("    - NVIDIA / Pixar 의 scene format")
    print("    - 더 풍부한 표현 (light / material / physics)")
    print()
    print("  Isaac Sim 의 URDF Importer:")
    print("    URDF 파일 -> 자동 USD 변환")
    print("    Joint property / inertia / collision 모두 보존")
    print("    week 4 에서 자작 팔 URDF 임포트 학습")


if __name__ == "__main__":
    p1(); p2(); p3()
