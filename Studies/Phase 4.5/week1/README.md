# Week 1 — sim task 정의 + adaptation 데이터 수집


> **목표**: v1 의 sim 환경을 재사용해, LoRA adaptation 에 쓸 (관측↔action) 데이터를 수집한다.
> **선행**: [`SETUP.md`](../SETUP.md) §2.1 통과, v1(Phase 4) sim 루프 동작.


## 학습 순서

1. v1 의 sim 환경/embodiment 를 데이터 수집용으로 재사용 가능한지 확인
2. adaptation 대상 task 1종 정의 (v1 의 성공률 baseline 이 있는 task 와 동일하게)
3. 관측↔action 페어 수집 (LoRA 가 돌 최소 규모)
4. 수집 데이터가 **OpenVLA 사전학습 분포(OpenX)와 겹치지 않는지** 점검


## 핵심 개념

- **왜 미학습 분포여야 하나**: OpenVLA 는 OpenX 970K 로 사전학습됨. 이미 본 분포를 재학습하면 "adaptation" 이 아니라 "복습"이라 둘째 층 증거가 안 됨.
- **규모의 트레이드오프**: 너무 적으면 학습 신호 부족, 너무 많으면 Colab 비용/시간 폭증. "LoRA 가 수렴 신호를 보일 최소선" 이 기준.


## 이번 주 산출 (must / nice)

- must: task 1종 정의 + 수집 데이터셋 1벌 + 미학습 분포 점검 메모
- nice: 데이터 분포 시각화(관측/action 히스토그램)


> PRACTICE/quiz 는 진입 시점 실측에 맞춰 작성한다 (스캐폴드).
