# Week 3 — Colab LoRA 파인튜닝


> **목표**: Colab(A100/L4)에서 OpenVLA 7B 를 week2 데이터로 LoRA 파인튜닝하고 체크포인트를 저장한다.
> **선행**: week2 OpenVLA 포맷 데이터, [`SETUP.md`](../SETUP.md) §2.2 + Step 0 실측 통과.


## 학습 순서

1. `peft` LoRA 설정 (target modules, rank) — OpenVLA upstream 권장값 확인
2. 학습 스크립트 작성 + `save_steps` 로 체크포인트 주기 저장 (Drive)
3. 1 사이클 학습 실행 + loss 추이 기록
4. 최종 LoRA 어댑터를 Drive 에 저장


## 핵심 개념

- **LoRA 가 왜 4070 으로 불가능한가**: 풀 파인튜닝은 24GB+ 필요. LoRA 도 OpenVLA 7B 규모에서는 옵티마이저 상태 + 활성값이 12GB 를 넘어 Colab 의존.
- **체크포인트 필수**: Colab 무료 티어는 세션 끊김 → step 단위 저장으로 복구 (SETUP §5.3).
- **rank 의 의미**: 어댑터 용량 vs 표현력 트레이드오프. 작게 시작.


## 이번 주 산출 (must / nice)

- must: LoRA 어댑터 1벌(Drive) + loss 추이 + Step 0 실측치(시간/비용) 기록
- nice: rank 변경 1회 비교


> PRACTICE/quiz 는 진입 시점에 작성한다 (스캐폴드).
