"""
Phase 4.5 Week 0 - 기초 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 정답: B) Vulkan 로더와 nvidia ICD 파일")
    print("=" * 50 + "\n")
    print("해설:")
    print("  SAPIEN 은 OpenGL 이 아니라 Vulkan 으로 렌더한다.")
    print("    - 로더: libvulkan1 (apt)")
    print("    - ICD: /usr/share/vulkan/icd.d/ 의 nvidia 항목 (드라이버가 설치)")
    print("    - 진단: vulkaninfo --summary 에 GPU 가 보이는가")
    print()
    print("  GPU 대신 llvmpipe 만 보이면 CPU 소프트웨어 렌더로 떨어진 상태다.")
    print("  이 경우 느리거나 검은 화면이 나오며, 진행해도 측정값을 못 쓴다.")
    print()
    print("  DISPLAY 는 헤드리스 배열 렌더에는 불필요하고, CUDA 는 렌더가 아니라")
    print("  추론 경로의 의존성이라 검은 화면의 원인이 아니다.")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) 0% 의 원인을 구분하기 위해")
    print("=" * 50 + "\n")
    print("해설:")
    print("  0% 를 만드는 원인이 두 가지이고 성격이 정반대다:")
    print("    - 도메인 갭      -> 정상 결과. baseline 으로 기록하고 진행")
    print("    - 통합 버그      -> 측정 무효. 고치고 다시 재야 한다")
    print()
    print("  같은 숫자가 나오므로 숫자만 보고는 구분할 수 없다.")
    print("  검증을 먼저 통과시키면 0% 가 해석 가능한 결과가 된다.")
    print()
    print("  Roadmap Phase 4.5 Section 0 의 순서 제약이 이 근거로 걸려 있다.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 정답: C) 설치된 버전의 판정 함수 소스코드를 직접 읽는다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  성공 판정은 버전마다 바뀔 수 있고, 문서가 코드보다 늦다.")
    print("  notes.md 순서 3 도 임계값을 '대략 2.5cm' 로만 적고")
    print("  '설치된 버전에서 1회 확인' 을 남겨 뒀다 -- 그 확인이 이번 주 몫이다.")
    print()
    print("  D 가 틀린 이유: 임계값을 새로 정의하면 내장 success 와 다른 지표가")
    print("  되어, 상한 대조와 fine-tuned 측정의 비교 기준이 어긋난다.")
    print("  판정은 내장 플래그를 그대로 채택하고, 정의만 사실로 확인한다.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) fine-tuned 측정에서 같은 목록을 써야 변인이 고정된다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Phase 4.5 의 성공 기준은 '동일 조건 N회 before/after 비교' 다.")
    print("  seed 가 초기 큐브 위치와 목표를 결정하므로, 목록이 다르면")
    print("  before/after 의 차이에 '문제 난이도 차이' 가 섞여 들어간다.")
    print()
    print("  즉 seed 목록은 부속 로그가 아니라 주장의 근거 자체다.")
    print("  A 가 틀린 이유: 신뢰구간 폭은 N 이 결정한다 (N=20 -> 약 +-22%p).")


def problem5_solution():
    print("\n" + "=" * 50)
    print("문제 5 정답: B) 최종 성공률이 0 에 붙어도 신호가 남게 하기 위해")
    print("=" * 50 + "\n")
    print("해설:")
    print("  공개 벤치마크에서 OpenVLA 의 sim zero-shot 은 환경에 따라 0% 근처가")
    print("  나오는 것이 정상 범위다. 최종 성공률 하나만 재면 before 쪽이 바닥에")
    print("  붙어 adaptation 이 무엇을 개선했는지 볼 수 없다.")
    print()
    print("  reached / grasped / lifted 를 함께 재면 앞 단계의 이동을 볼 수 있어")
    print("  before/after 가 신호를 만든다.")
    print()
    print("  주의: 성공률의 '대체' 가 아니라 '병기' 다. 최종 성공률은 그대로 보고한다.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    print("=" * 50)
