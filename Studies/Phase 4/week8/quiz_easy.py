"""
Phase 4 Week 8 - Inference wrapper + Stability 기초 퀴즈
"""


def problem1_bgr_rgb():
    print("\n" + "=" * 50)
    print("문제 1: OpenCV BGR <-> PIL RGB")
    print("=" * 50 + "\n")
    print("질문: OpenCV 로 카메라에서 받은 이미지를 OpenVLA 에 입력 시")
    print("      반드시 해야 할 변환은?\n")
    print("보기:")
    print("  A) BGR -> RGB (cv2.COLOR_BGR2RGB)")
    print("  B) 그대로 사용해도 무관")
    print("  C) HSV 로 변환")
    print("  D) Grayscale 변환")


def problem2_oom_recovery():
    print("\n" + "=" * 50)
    print("문제 2: OOM 발생 시 표준 복구 패턴")
    print("=" * 50 + "\n")
    print("질문: inference 중 OOM (CUDA out of memory) 발생 시 첫 대처는?\n")
    print("보기:")
    print("  A) 노드 종료")
    print("  B) torch.cuda.empty_cache() 후 retry 1회, 그래도 실패 시 safety fallback")
    print("  C) GPU 재부팅")
    print("  D) sleep(1) 후 재시도 무한 반복")


def problem3_stress_threshold():
    print("\n" + "=" * 50)
    print("문제 3: Stress test 통과 기준")
    print("=" * 50 + "\n")
    print("질문: 본 로드맵의 stress test 통과 기준은?\n")
    print("보기:")
    print("  A) 100 회 중 95 회 success")
    print("  B) 100 회 중 0 fail (모든 시도 성공)")
    print("  C) 1000 회 중 990 회 success")
    print("  D) latency < 100 ms 1회라도 달성")


def problem4_config_env():
    print("\n" + "=" * 50)
    print("문제 4: config 를 환경 변수로 빼는 이유")
    print("=" * 50 + "\n")
    print("질문: 모델 경로 / quantization 설정을 환경 변수로 분리하는 가장 큰 이점?\n")
    print("보기:")
    print("  A) 코드가 더 짧아짐")
    print("  B) ROS2 launch 시 코드 수정 없이 다른 모델로 빠르게 전환 가능")
    print("  C) 메모리를 적게 사용")
    print("  D) GPU 가 더 빨라짐")


if __name__ == "__main__":
    print("=" * 50)
    problem1_bgr_rgb()
    problem2_oom_recovery()
    problem3_stress_threshold()
    problem4_config_env()
    print("=" * 50)
