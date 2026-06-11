# Phase 4.5: VLA v1.5 — OpenVLA LoRA adaptation (before/after 정량 분석)


> **기간**: 약 6-8주 (2026 하반기, v1 공개 직후 — v1 지연 최소화 원칙상 v1 우선)
> **목표**: v1 의 zero-shot 베이스라인 위에서 OpenVLA 를 LoRA 로 **adaptation** 하고, 동일 sim task 의 성공률을 **before(zero-shot)/after(fine-tuned)** 로 비교·정량 분석한다.
> **범위 (v1.5)**: 경량 LoRA adaptation, **sim 생성 데이터** (자작 팔 데이터 대기 안 함), 단일 task, 동일 embodiment. real 데이터 확장은 Phase 7.
> **언어**: **Python** (HuggingFace transformers + peft) + **ROS2 (rclpy)** (eval 루프 재사용)
> **하드웨어**: 학습은 **Colab A100/L4** (24GB+ 필요), 추론·eval 은 **로컬 RTX 4070 + 4-bit 양자화**
> **주간 시간**: 약 6-8시간 (출장 주 보정)


---


## -> **실습 가이드**: `Studies/Phase 4.5/weekN/` (각 week 별 README + PRACTICE + quiz. **진입 시 다시 체크**)


**핵심 산출물 (v1.5)**:
- LoRA 파이프라인 (sim 데이터 포맷팅 -> Colab 학습 -> 체크포인트 -> 로컬 머지/양자화)
- eval harness: zero-shot vs fine-tuned 를 **동일 조건 N회** 로 비교 (N 과 분산까지 보고)
- 블로그 1편: adaptation **설계-실행-분석** 서사 (성공률 상승 여부와 무관하게 성립)


---


## 왜 별도 Phase 인가 (3-Layer 좌표)


- v1(Phase 4)은 **셋째 층** — 현 세대 FM 을 그대로 배포해 단일 task 루프를 닫는다(zero-shot).
- v1.5(본 Phase)는 **둘째 층** — FM 을 특정 태스크/환경에 맞추는 adaptation 의 증거. zero-shot 과 **대비**되어야 둘째 층 역량이 드러나므로 v1 과 섞지 않고 독립 산출물로 둔다.
- LoRA 를 Phase 6(v2)에서 본 Phase 로 **전진 배치** 했다. Phase 6 은 sim-to-real gap(셋째 층)에 집중하고, 둘째 층 증거의 타이밍을 실지원(2027) 전으로 앞당긴다.


---


## 성공 기준 (가장 중요 — 결과 비의존)


> **"성공률이 올랐다"가 성공 기준이 아니다.** 소규모 데이터 LoRA 는 zero-shot 대비 성공률을 못 올리거나(overfitting / catastrophic forgetting), 차이가 통계적 노이즈일 수 있다.


성공 기준은 **"adaptation 파이프라인을 설계-실행하고 결과를 정량 분석했다"** 이다.

- before/after 를 동일 조건 **N회** 로 측정하고, **N 과 분산(표준편차 또는 신뢰구간)** 을 함께 보고한다.
- 차이가 노이즈 범위 안이면, **그 사실과 원인 분석(데이터 규모/분포/학습 설정)** 자체를 산출물로 삼는다.
- 즉 negative 결과여도 "왜 안 올랐는가"의 분석이 둘째 층 역량의 증거가 되도록 블로그 논지를 구성한다.
- **before/after 가 신호를 만들려면 task 난이도가 적절해야 한다**: zero-shot 이 이미 잘 되는 task 는 adaptation 개선 폭이 안 보이고, 거의 0% 인 task 는 비교 자체가 무의미하다. task 는 **zero-shot 이 어중간하게 되는 중간 구간**으로 선정해야 before/after 가 의미 있는 신호를 만든다 (Section 0 / Section 1 에서 점검). 이 점검을 건너뛰면 데이터·학습을 다 돌리고도 "차이가 안 보이는" 결과가 구조적으로 나올 수 있다 — 이는 negative 분석(위)으로 받칠 수는 있으나, task 설계 단계에서 먼저 피하는 것이 우선이다.


---


## 데이터: 왜 sim 생성 데이터인가


| 옵션 | 채택 여부 | 사유 |
|---|---|---|
| OpenX-Embodiment | 사용 안 함 | OpenVLA 가 OpenX 970K 로 **사전학습됨** -> 이미 본 데이터 재학습은 adaptation 증거로 부적합 |
| 자작 팔 teleop 데이터 | 대기 안 함 | 2027 이후라 둘째 층 증거가 실지원 타이밍보다 늦음 (real 확장은 Phase 7) |
| **sim 생성 데이터** | **채택** | OpenVLA **미학습 분포**로 구성 가능 + 타이밍 확보. v1 의 sim 환경/embodiment 재사용 |


> 진입 시 점검: 생성한 sim 데이터가 OpenVLA 사전학습 분포와 **겹치지 않는지** 확인해야 adaptation 의 의미가 산다.


---


## 컴퓨트 (리스크 — Step 0 실측 필수)


| 작업 | 환경 | 비고 |
|---|---|---|
| LoRA 파인튜닝 | **Colab A100/L4** | OpenVLA 7B LoRA 는 24GB+ 필요 -> RTX 4070(12GB) 불가 |
| 머지 + 4-bit 양자화 | 로컬 4070 | 약 6GB 로 축소 -> 12GB 안착 |
| eval 추론 루프 | 로컬 4070 + ROS2 | Phase 4 의 `vla_node` / sim 루프 재사용 |


- **가용성/비용이 변수**: A100 이 항상 잡히지 않음. L4 + gradient accumulation 으로 우회 가능(시간 증가).
- **Step 0 실측 실패 시 분기**: LoRA 대상을 action head 인근 어댑터로 축소, sim 소량 데이터, before/after 를 단일 변형으로 한정, 주장 톤을 "경량 adaptation 실험"으로 (롤백 옵션 B).
- 컴퓨트 전략의 단일 진실 공급원은 [`Studies/Phase 4/SETUP.md`](../Studies/Phase%204/SETUP.md) 의 분업 원칙과 공유 — v1.5 는 그 "LoRA 파인튜닝 트랙"을 본 산출물로 승격한 것이다.


---


## Section 0: 시작 전 (Step 0 실측)


- [ ] OpenVLA 7B LoRA 1회 사이클이 Colab A100/L4 에서 가능한지 실측 (시간/비용/체크포인트)
- [ ] fine-tuned 모델 4-bit 추론이 RTX 4070 12GB 에 OOM 없이 올라가는지 확인
- [ ] sim 데이터 생성 방식 확정 (어느 sim, 어떤 task, 수집 규모) + OpenVLA 미학습 분포인지 점검
- [ ] eval N 및 통계 처리(분산/신뢰구간) 기준 1차 결정
- [ ] v1(Phase 4) 의 sim 환경 / 성공 task / 성공률 표가 baseline 으로 재사용 가능한지 확인
- [ ] **before/after 측정 성립 구간 확인**: v1 task 의 zero-shot 성공률이 포화(개선 폭 안 보임)도 0% 근처(비교 무의미)도 아닌 **중간 구간**인지 점검. 벗어나면 Section 1 에서 task 난이도를 조정


---


## Section 1: sim 데이터 생성 + 포맷팅 (1-2주)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 1 | sim task 정의 (**before/after 신호가 나올 난이도대 선정**) + 데이터 수집 (관측↔action 페어) + 미학습 분포 점검 | v1 환경 재사용, 규모는 LoRA 가 돌 최소선. task 는 zero-shot 이 어중간하게 되는 구간으로 |
| 2 | OpenVLA 학습 포맷으로 변환 (RLDS / 모델 카드 스키마 확인) | 데이터 포맷팅이 adaptation 의 절반 |


> task 난이도 선정 보충: 후보 task 를 1-2개 zero-shot 으로 빠르게 돌려 성공률대를 먼저 본다. 포화(예: 90%+)면 더 어려운 변형으로, 바닥(예: 10% 미만)이면 더 쉬운 변형으로 조정해 before/after 가 움직일 여지를 확보한 뒤 본 데이터를 수집한다.


---


## Section 2: LoRA 학습 + 로컬 이식 (2-3주)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 3 | Colab LoRA 파인튜닝 + 체크포인트 주기 저장 | 세션 끊김 대비 (SETUP.md §5.3) |
| 4 | LoRA 가중치 -> Drive -> 로컬 머지 + 4-bit 양자화 + 호환성 검증 | 버전 매칭(SETUP.md §7) |


---


## Section 3: eval harness + before/after 분석 (2-3주)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 5 | eval harness: zero-shot vs fine-tuned 를 **동일 조건 N회** | N / 분산 / 신뢰구간 명문화 |
| 6 | before/after 정량 분석 + 블로그 1편 + v1.5 공개 | negative 결과도 성립하는 논지 |


> eval 은 v1 의 sim 단일 task 루프를 그대로 쓰되 모델만 zero-shot / fine-tuned 로 바꿔 N회 반복한다. 변인은 모델 하나로 고정.


---


## Phase 4.5 완료 체크리스트


### adaptation 파이프라인
- [ ] sim 생성 데이터가 OpenVLA 미학습 분포임을 확인
- [ ] LoRA 파인튜닝 1 사이클 완료 (Colab) + 체크포인트
- [ ] 로컬 머지 + 4-bit 양자화 + 호환성 검증 통과


### eval / 분석
- [ ] task 가 before/after 측정 성립 구간이었는지 확인 (zero-shot 이 포화도 0% 근처도 아님)
- [ ] zero-shot vs fine-tuned 를 동일 조건 N회 측정 (N 명시)
- [ ] 성공률 차이를 분산(표준편차/신뢰구간)과 함께 보고
- [ ] 차이가 노이즈면 원인 분석을 산출물로 작성 (negative 대응)


### 산출물 v1.5 공개
- [ ] LoRA 파이프라인 + eval harness 코드 정리 + README
- [ ] 블로그 1편 (adaptation 설계-실행-분석 서사) + velog/LinkedIn 공개


---


## Phase 4.5 완료 기준


> "OpenVLA 를 sim 생성 데이터로 LoRA adaptation 하고, zero-shot 대비 성공률을 동일 조건 N회로 측정해 분산까지 보고했다. 성공률 상승 여부와 무관하게, adaptation 을 **설계-실행-정량 분석**한 서사를 산출물로 공개할 수 있다. 단순 '학습이 돌았다'는 통과가 아니다."


---


## 참고 자료


- OpenVLA (Stanford, 2024): https://openvla.github.io/
- OpenVLA fine-tuning / LoRA: HuggingFace `peft` (https://huggingface.co/docs/peft)
- 컴퓨트 분업 / 버전 매칭: [`Studies/Phase 4/SETUP.md`](../Studies/Phase%204/SETUP.md)
- [ENVIRONMENT.md](../ENVIRONMENT.md) — 프로젝트 공용 환경


---


## [?] 다음 단계


Phase 4.5 완료 후:
- **6개월 분기 재평가 #1 (2026.11)** — sim adaptation 이 AI 트랙 JD 에서 둘째 층 증거로 읽히는지 점검 (sim 증거의 설득력 한계)
- **Phase 6 (v2)**: sim-to-real gap(셋째 층)에 집중 — LoRA 는 본 Phase 로 이관됨
- **Phase 7 (v3)**: 본 Phase 의 adaptation 파이프라인 + eval 을 **자작 팔 teleop(real) 데이터**로 확장 (둘째 층 증거를 real 도메인으로)
