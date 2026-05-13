"""Phase 6 Week 6 - 중급 정답"""


def p1():
    print("\n정답: D) 모두 가능")
    print()
    print("  20mm 오차 디버깅 순서:")
    print("  1. dof_mapping 검증 (week 5)")
    print("  2. FK 직접 계산 (Real / Sim 각자 - PyKDL)")
    print("  3. URDF link length 실측 검증")
    print("  4. Real encoder 캘리브레이션")


def p2():
    print("\n정답: thread 분리 + 동기화")
    print()
    print("  표준 패턴:")
    print("  - ROS2 spin_once 를 sim main loop 안에서 호출")
    print("  - 별도 thread 없음 -> thread safety 신경 안 써도")
    print()
    print("  while sim_app.is_running():")
    print("      rclpy.spin_once(node, timeout_sec=0.001)")
    print("      world.step(render=True)")


def p3():
    print("\n정답: Phase 7 의 완전한 closed loop")
    print()
    print("  Phase 6 의 결과 (week 6):")
    print("    Real -> Sim (one-way digital twin)")
    print()
    print("  Phase 7 산출물 #4:")
    print("    1. Real camera -> OpenVLA -> action -> Real robot 실행")
    print("    2. Real robot joint -> Sim 매칭 (디지털 트윈)")
    print("    3. Sim 의 camera 도 OpenVLA inference 의 backup")
    print()
    print("  '두 환경에서 동시에 동일 task 가 진행' 이 Real-to-Sim-to-Real")
    print("  의 본질. 양산 시점에 Sim 으로 사전 검증 후 Real 배포 가능.")


if __name__ == "__main__":
    p1(); p2(); p3()
