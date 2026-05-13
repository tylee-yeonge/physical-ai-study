"""
Phase 4 Week 11 - 실 inference 통합 기초 퀴즈
"""


def problem1_callback_first_step():
    print("\n" + "=" * 50)
    print("문제 1: image_callback 의 첫 단계")
    print("=" * 50 + "\n")
    print("질문: image_callback 에서 inference 직전 가장 먼저 해야 할 것은?\n")
    print("보기:")
    print("  A) image preprocess (BGR -> RGB)")
    print("  B) image age check (header.stamp 기반)")
    print("  C) GPU memory 확인")
    print("  D) instruction 길이 확인")


def problem2_executor():
    print("\n" + "=" * 50)
    print("문제 2: rclpy default executor")
    print("=" * 50 + "\n")
    print("질문: 본 phase 의 vla_node 에 적합한 executor 는?\n")
    print("보기:")
    print("  A) Single-threaded (default)")
    print("  B) Multi-threaded")
    print("  C) AsyncIO")
    print("  D) Custom thread pool")


def problem3_dry_run_criteria():
    print("\n" + "=" * 50)
    print("문제 3: 1분 dry-run 의 success criteria")
    print("=" * 50 + "\n")
    print("질문: 1분 dry-run 의 가장 엄격한 기준은?\n")
    print("보기:")
    print("  A) Mean latency < 200ms")
    print("  B) p95 latency < 300ms")
    print("  C) 0 fail (모든 inference 성공)")
    print("  D) Throughput > 5 Hz")


def problem4_bag_use():
    print("\n" + "=" * 50)
    print("문제 4: ros2 bag 의 사용 목적")
    print("=" * 50 + "\n")
    print("질문: 본 phase 에서 ros2 bag 의 가장 큰 가치는?\n")
    print("보기:")
    print("  A) GPU memory 절약")
    print("  B) 재현 가능한 테스트 (동일 image input)")
    print("  C) inference 속도 향상")
    print("  D) 모델 weight 저장")


if __name__ == "__main__":
    print("=" * 50)
    problem1_callback_first_step()
    problem2_executor()
    problem3_dry_run_criteria()
    problem4_bag_use()
    print("=" * 50)
