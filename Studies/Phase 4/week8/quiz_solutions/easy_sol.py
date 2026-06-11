"""
Phase 4 Week 8 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: A) BGR -> RGB")
    print("=" * 50 + "\n")
    print("해설:")
    print("  OpenCV (cv2) 의 기본 채널 순서: BGR (Blue, Green, Red)")
    print("  PIL / HuggingFace / 대부분 ML 라이브러리: RGB")
    print()
    print("  BGR 그대로 inference 하면:")
    print("    - 빨간 캔이 파란 캔으로 보임 (R/B swap)")
    print("    - 학습된 적 없는 색 분포 -> 부정확한 action")
    print("    - 에러는 안 나지만 결과 품질 저하 (조용한 실패, 가장 위험)")
    print()
    print("  표준 패턴:")
    print("    cv2.imread() -> cv2.cvtColor(img, cv2.COLOR_BGR2RGB) -> PIL.Image.fromarray()")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) empty_cache + retry 1회 + safety fallback")
    print("=" * 50 + "\n")
    print("해설:")
    print("  OOM 처리의 표준 패턴 (양산):")
    print("    1) torch.cuda.empty_cache()")
    print("    2) retry 1회 (대부분 일회성 fragmentation 해결)")
    print("    3) 또 OOM -> safety fallback action publish (예: zero action)")
    print("    4) log + 상태 모니터링")
    print()
    print("  하지 말아야 할 것:")
    print("    - 노드 종료 (A): robot 이 멈춤. 안전이 더 위험할 수 있음.")
    print("    - GPU 재부팅 (C): 실제로 거의 불필요.")
    print("    - 무한 retry (D): hang -> dead robot.")
    print()
    print("  본 로드맵 Phase 7 산출물 #4 에서 'fast safety policy' 가 이 역할:")
    print("    OOM 발생 시 OpenVLA action publish 안 함 -> safety policy 가 last")
    print("    action 유지 또는 zero velocity 명령.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: B) 100 회 중 0 fail")
    print("=" * 50 + "\n")
    print("해설:")
    print("  양산 SW 기준:")
    print("    - robot 이 18000 frame (1시간 5Hz) 운영 시 fail rate < 0.01%")
    print("    - 즉 100 회는 0 fail, 10000 회 중 1 fail 이 최저")
    print()
    print("  본 주는 100 회 stress test 부터.")
    print("  100 회 중 1 fail 이라도 발생하면:")
    print("    -> 원인 분석 (어떤 image / instruction 에서)")
    print("    -> 예외 처리 코드 추가")
    print("    -> 다시 100 회 시도")
    print()
    print("  이 반복이 양산 SW 의 본질. '안 죽는 코드' 가 1순위.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) ROS2 launch 시 빠른 전환 가능")
    print("=" * 50 + "\n")
    print("해설:")
    print("  config 환경 변수의 실용 가치:")
    print()
    print("  ROS2 launch.py 예시:")
    print("    Node(")
    print("      package='vla_node',")
    print("      env={'VLA_MODEL_ID': 'openvla/openvla-7b',")
    print("           'VLA_QUANT': '4bit'})")
    print()
    print("  다른 환경 (테스트 / 양산):")
    print("    env={'VLA_MODEL_ID': '/local/path/to/finetuned',")
    print("         'VLA_QUANT': '8bit'}")
    print()
    print("  주의: int8 은 본 실험에서 배제 — 성공률 58.1% + A5000 1.2 Hz")
    print("  (OpenVLA Table 2/§5.4). 위 '8bit' 은 전환 가능성 예시일 뿐.")
    print()
    print("  코드 수정 없이:")
    print("    - 모델 교체 (zero-shot vs fine-tuned)")
    print("    - quantization 전환 (속도 vs 정확도)")
    print("    - GPU device 변경 (cuda:0 -> cuda:1)")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
