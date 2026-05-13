"""
Phase 4 Week 11 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) image age check")
    print("=" * 50 + "\n")
    print("해설:")
    print("  image_callback 의 표준 순서:")
    print("    1. (instruction 비어있으면 skip)")
    print("    2. image age check (header.stamp 기반)")
    print("    3. preprocess (BGR -> RGB -> PIL)")
    print("    4. inference")
    print("    5. publish")
    print()
    print("  Image age check 가 inference 보다 먼저:")
    print("    - inference 는 ~150ms 비싼 연산")
    print("    - 오래된 image (예: 500ms 전) 로 inference 하면 의미 없음")
    print("    - fail fast 원칙")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: A) Single-threaded default")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Single-threaded executor 가 본 phase 에 적합한 이유:")
    print("    1. inference 가 GPU 점유 -> 동시 실행 무의미")
    print("    2. thread-safety 신경 안 써도 됨")
    print("    3. BEST_EFFORT QoS + depth=1 가 큐 누적 방지")
    print()
    print("  Multi-threaded executor 의 함정:")
    print("    - callback 동시 실행 시 model.predict 가 thread-unsafe 일 수 있음")
    print("    - GPU 가 한 번에 한 inference 만 처리 가능 (의미 없음)")
    print()
    print("  본 phase 의 single-thread 한계:")
    print("    - inference 중에 image 새로 와도 처리 불가 (단, BEST_EFFORT 로 drop)")
    print("    - 큐에 의한 latency 증가 없음 (depth=1 이라 항상 latest)")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: C) 0 fail")
    print("=" * 50 + "\n")
    print("해설:")
    print("  양산 SW 의 기준:")
    print("    1. 노드가 죽지 않는다 (crash 0)")
    print("    2. 모든 inference 가 성공 (0 fail)")
    print("    3. latency 가 spec 안 (mean < 200ms 등)")
    print("    4. throughput 안정 (5 Hz +- 10%)")
    print()
    print("  1~4 중 가장 중요한 것: '0 fail'.")
    print("    - 1 fail = 한 frame 의 정보 손실 = robot 의 잠재적 위험")
    print("    - 양산 라인은 시간 당 18000 frame 처리, fail rate 0.01% 가 최저")
    print()
    print("  latency / throughput 은 'tunable',")
    print("  fail 은 '용서 안 됨'.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) 재현 가능한 테스트")
    print("=" * 50 + "\n")
    print("해설:")
    print("  ros2 bag 의 가치:")
    print("    - 동일한 image input 으로 여러 번 테스트")
    print("    - 노드 수정 후 regression 확인")
    print("    - 시연 영상 제작의 input")
    print("    - 시간 / 장소 영향 없는 테스트")
    print()
    print("  본 phase 의 활용:")
    print("    1. ELP Stereo 로 1분 녹화 (한 번)")
    print("    2. bag 으로 저장")
    print("    3. dry-run 시 bag play 로 재생")
    print("    4. latency 측정 / fail rate / 성공률 모두 동일 input 기반")
    print()
    print("  Phase 7 (산출물 #4) 에서도 활용:")
    print("    - 자작 6DOF 팔 환경 bag")
    print("    - LoRA fine-tune 전후 비교")
    print("    - latency before/after")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    print("=" * 50)
