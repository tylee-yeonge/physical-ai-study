# Week 4 — 로컬 머지 + 4-bit 양자화 + 호환성 검증


> **목표**: Drive 의 LoRA 어댑터를 로컬 4070 으로 내려 베이스에 머지하고, 4-bit 로 양자화해 추론 가능 상태로 만든다.
> **선행**: week3 LoRA 어댑터, [`SETUP.md`](../SETUP.md) §1 (Phase 4 SETUP §6-8).


## 학습 순서

1. rclone/gdown 으로 LoRA 어댑터 다운로드
2. 베이스 OpenVLA + LoRA 머지
3. 4-bit 양자화(bitsandbytes) → 약 6GB → 12GB 안착 확인 (VRAM 측정)
4. **호환성 검증** (Phase 4 SETUP §7.3) — 어댑터 로드 OK 확인


## 핵심 개념

- **버전 매칭이 핵심 함정**: Colab 과 로컬의 peft/transformers/bitsandbytes 버전이 어긋나면 어댑터 로드 실패. 학습 직후 1회 검증으로 재학습 비용 회피.
- **양자화 품질 저하**: 4-bit 는 action 정확도를 떨어뜨릴 수 있음 → eval(week5)에서 zero-shot 도 동일 양자화 조건으로 맞춰 변인 통제.


## 이번 주 산출 (must / nice)

- must: 머지+양자화된 fine-tuned 모델 로컬 적재 + VRAM 측정 + 호환성 검증 통과
- nice: 양자화 전후 메모리/속도 비교 표


> PRACTICE/quiz 는 진입 시점에 작성한다 (스캐폴드).
