# Phase 4.5 진입 전 환경 구축 (adaptation 추가분)


> Phase 4.5 의 모든 week 를 시작하기 *전에* 1회 수행. **컴퓨트 분업의 본체는 [`Studies/Phase 4/SETUP.md`](../Phase%204/SETUP.md)** 이고, 본 문서는 그 위에 LoRA adaptation 전용 추가분만 정리한다.


---


## 0. 한 줄 요약


학습은 Colab(A100/L4), 추론·eval 은 로컬 4070 + 4-bit 양자화 — 이 분업과 버전 매칭은 Phase 4 SETUP.md 와 **동일**하다. v1.5 가 추가로 요구하는 것은 (1) sim 데이터 생성 경로, (2) LoRA 학습 스크립트, (3) zero-shot vs fine-tuned eval harness 세 가지다.


---


## 1. Phase 4 SETUP 과 공유하는 부분 (재확인만)


아래는 [`Studies/Phase 4/SETUP.md`](../Phase%204/SETUP.md) 를 그대로 따른다. 본 Phase 진입 시 변경 없는지만 재확인:

- §1 학습/추론 분업 근거 (4070 으로 LoRA 불가, 추론은 양자화)
- §5 Colab 측 환경 (GPU 선택, 표준 셀, 체크포인트 §5.3)
- §6 로컬 측 환경 (머지/양자화, venv, ROS2)
- §7 **버전 매칭** (peft / bitsandbytes / transformers 등 — adapter 포맷 호환성의 핵심)
- §8 가중치 전송 (Colab -> Drive -> 로컬)


> v1.5 에서 LoRA 가 본 트랙으로 승격되므로, Phase 4 SETUP §2.1 의 "Colab Pro 는 v2 LoRA 트랙에만 필요" 문구는 **본 Phase 진입 시점부터 적용**된다. A100/L4 가용성·비용을 Step 0 에서 실측한다.


---


## 2. v1.5 전용 추가분


### 2.1 sim 데이터 생성

- [ ] v1 의 sim 환경/embodiment 를 데이터 수집용으로 재사용 가능한지 확인
- [ ] task 정의 + 수집 규모 (LoRA 가 돌 최소선) 결정
- [ ] 생성 데이터가 **OpenVLA 사전학습 분포와 겹치지 않는지** 점검 (겹치면 adaptation 의미 소실)
- [ ] OpenVLA 학습 포맷(RLDS / 모델 카드 스키마)으로 변환 경로 확보


### 2.2 LoRA 학습

- [ ] `peft` LoRA 설정 (target modules, rank) — OpenVLA upstream 권장값 확인
- [ ] 체크포인트 주기 저장 (Drive, 세션 끊김 대비)
- [ ] Step 0 실측: A100/L4 1 사이클 시간·비용 기록


### 2.3 eval harness

- [ ] zero-shot / fine-tuned 를 **모델만 교체**해 동일 sim task 루프에 투입하는 구조
- [ ] N (반복 횟수) + 통계 처리(표준편차/신뢰구간) 결정 + 기록 포맷
- [ ] 결과 표/그래프 출력 경로 (`eval/outputs/`, gitignore)


---


## 3. Step 0 게이트 (실측 실패 시 분기)


아래 둘 중 하나라도 막히면 Roadmap §롤백 옵션 B(경량 adaptation 축소)로 전환:

- [ ] OpenVLA 7B LoRA 1 사이클이 Colab A100/L4 에서 완주
- [ ] fine-tuned 모델 4-bit 추론이 RTX 4070 12GB 에 OOM 없이 적재


---


## 참고 문서


- [`Studies/Phase 4/SETUP.md`](../Phase%204/SETUP.md) — 컴퓨트 분업/버전 매칭 단일 진실 공급원
- [`Roadmap/Phase 4.5.md`](../../Roadmap/Phase%204.5.md) — v1.5 전체 계획
- [ENVIRONMENT.md](../../ENVIRONMENT.md) — 프로젝트 공용 환경
