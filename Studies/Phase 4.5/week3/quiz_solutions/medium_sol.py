"""
Phase 4.5 Week 3 - 메모리 산수 + 학습 운영 퀴즈 정답
"""


def problem1_solution():
    print("\n" + "=" * 50)
    print("문제 1 해설: 줄어드는 부분과 안 줄어드는 부분")
    print("=" * 50 + "\n")
    print("답의 뼈대:")
    print("  (1) 72GB -> 27GB 로 줄어드는 것: 배치에 비례하는 부분, 즉 활성값이다.")
    print("      배치 16 에서 1 로 내리면 활성값이 그만큼 작아진다.")
    print()
    print("  (2) 27GB 에서 더 안 내려가는 것: 배치와 무관한 부분이다.")
    print("      핵심은 frozen base 가중치(bf16, 약 15GB)이고, 여기에 어댑터·")
    print("      옵티마이저 상태·CUDA 컨텍스트·단편화 여유가 얹힌다.")
    print("      배치를 0 으로 만들 수는 없으므로 이 합이 하한이 된다.")
    print()
    print("  (3) 4070 은 12GB 다. base 가중치 하나가 이미 그것을 넘으므로,")
    print("      배치·grad accumulation 을 어떻게 조합해도 표준 경로로는 올라가지 않는다.")
    print("      그래서 24GB 급이 필요하고, 그 GPU 도 27GB 하한에 가까워")
    print("      배치를 낮춰 맞추는 조정이 필요하다 (실습 3).")


def problem2_solution():
    print("\n" + "=" * 50)
    print("문제 2 정답: B) 원인 분리가 안 된다")
    print("=" * 50 + "\n")
    print("해설:")
    print("  Phase 4.5 의 성공 기준은 성공률 상승이 아니라 before/after 정량 분석이다.")
    print("  결과가 안 올랐을 때 '왜' 를 설명할 수 있어야 산출물이 성립한다.")
    print()
    print("  4-bit 학습은 upstream 이 스스로 '성능을 해친다' 고 경고한 경로다.")
    print("  이것을 쓰면 원인 후보가 (데이터 규모 / 분포 / 학습 설정) 에")
    print("  '4-bit 학습' 이 추가되고, 분리가 안 되면 분석이 무력해진다.")
    print()
    print("  비용은 이유가 아니다 -- 24GB 인스턴스 1사이클 비용은 이 리스크를")
    print("  감수해 아낄 규모가 아니다. 같은 사고방식이 week2 에서 자작 로더를")
    print("  배제한 근거이기도 하다.")


def problem3_solution():
    print("\n" + "=" * 50)
    print("문제 3 해설: 사라지는 이유와 남는 이유")
    print("=" * 50 + "\n")
    print("답의 뼈대:")
    print("  (1) RunPod 의 두 역할을 분리한다:")
    print("      사라지는 것 -- 로컬 PC 단일 장애점 보험 (물리 접근이 유지되므로)")
    print("      남는 것     -- VRAM. 12GB 로는 표준 LoRA 경로가 돌지 않는다 (문제 1)")
    print("      즉 '못 만져서' 가 아니라 '안 돌아서' 쓰는 것으로 근거가 바뀐다.")
    print()
    print("  (2) 범위가 줄어드는 것: 컨테이너화의 대상이다.")
    print("      eval 이 영구히 로컬이면 sim + eval 을 컨테이너에 넣을 필요가 없고,")
    print("      학습 측 이미지 하나만 만들면 된다. sim 컨테이너화는 Vulkan 요구로")
    print("      난이도가 별개이므로 이 축소의 체감이 크다.")
    print()
    print("  남는 정당화: 재현 가능한 이미지 자체가 배포 역량의 증거다.")


def problem4_solution():
    print("\n" + "=" * 50)
    print("문제 4 정답: B) 틀린 라벨을 잘 맞춘 것")
    print("=" * 50 + "\n")
    print("해설:")
    print("  week2 §8 의 조용한 실패들(선정규화 / 회전 표현 / gripper 부호 /")
    print("  낡은 통계 캐시 / 라벨 인덱스 밀림)은 모두 loss 를 정상적으로 내려가게 한다.")
    print("  라벨이 일관되게 틀려 있으면 모델은 그 틀린 라벨을 잘 맞추게 되고,")
    print("  loss 는 예쁘게 떨어진다. 곡선은 라벨의 정확성을 말해 주지 않는다.")
    print()
    print("  그래서 이번 주의 통과 기준은 곡선이 아니다:")
    print("    완주 / 실측 기록 / 복구 가능 / 회수 목록 완비")
    print("  성공률이 올랐는지는 Section 3 의 eval 이 답한다.")


def problem5_solution():
    print("\n" + "=" * 50)
    print("문제 5 해설: 용량과 되돌릴 자유의 거래")
    print("=" * 50 + "\n")
    print("답의 뼈대:")
    print("  (1) 최신만 덮어쓰기 -- 용량이 작다. 대가는 과거로 돌아갈 수 없다.")
    print("      시점별 보관     -- 되돌릴 수 있다. 대가는 volume 용량과 비용이다.")
    print("      (7B 머지 가중치는 체크포인트 하나가 크다는 점을 계산에 넣어야 한다)")
    print()
    print("  (2) 시점별 보관이 필요한 상황: 학습이 발산했거나 후반에 품질이")
    print("      나빠졌을 때 이전 지점으로 돌아가야 한다. 덮어쓰기만 썼다면")
    print("      나빠진 상태 하나만 남는다.")
    print()
    print("  (3) 기록해야 하는 이유: 실습 5 의 복구 리허설이 이 선택에 의존한다.")
    print("      무엇이 어디에 몇 개 남는지 모르면 재개 절차를 검증할 수 없고,")
    print("      회수된 뒤에 처음 알아보는 상황이 된다.")


def problem6_solution():
    print("\n" + "=" * 50)
    print("문제 6 정답: B) 전송 + 4-bit 양자화 + 호환성 검증")
    print("=" * 50 + "\n")
    print("해설:")
    print("  학습 스크립트가 체크포인트를 저장할 때 어댑터를 base 에 머지해 둔다.")
    print("  따라서 week4 에서 손으로 머지할 일은 없고, 남는 것은")
    print("    - 머지 가중치를 로컬로 내리기 (용량이 크므로 재개 가능한 전송)")
    print("    - 4-bit 양자화로 12GB 안에 안착시키기")
    print("    - 버전 호환성 검증 (transformers / timm / bitsandbytes 조합)")
    print("  이다.")
    print()
    print("  단 어댑터 원본도 함께 회수한다. rank 를 바꿔 비교하거나 머지를")
    print("  다시 하고 싶을 때 필요하고, 용량도 작다.")
    print()
    print("  그리고 통계 파일이 함께 있어야 추론에서 unnorm_key 를 쓸 수 있다.")


if __name__ == "__main__":
    print("=" * 50)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    problem4_solution()
    problem5_solution()
    problem6_solution()
    print("=" * 50)
