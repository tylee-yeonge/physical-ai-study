# Phase 4.5: VLA v1.5 — OpenVLA LoRA adaptation (before/after 정량 분석)


> **기간**: **Section 0 은 2026.08 (전진 배치)** / Sections 1-3 은 2026.09-11 (약 7-10주). 전진 사유: sim 환경과 zero-shot baseline 이 Sections 1-3 의 선행 조건이고, LoRA 학습을 RunPod 에서 돌리기 위한 컨테이너화를 여기서 끝내 둔다 (LoRA 는 24GB+ 를 요구해 로컬 4070 으로는 불가). v1 에서 sim 을 제외하면서 **sim 환경 구축 + zero-shot baseline 측정이 본 Phase 로 이관**돼 원안(6-8주)보다 약 1-2주 늘었다.
> **목표**: sim task 의 zero-shot 성공률 베이스라인을 **본 Phase 에서 측정**하고, 그 위에서 OpenVLA 를 LoRA 로 **adaptation** 한 뒤 동일 sim task 의 성공률을 **before(zero-shot)/after(fine-tuned)** 로 비교·정량 분석한다.
> **범위 (v1.5)**: 경량 LoRA adaptation, **sim 생성 데이터** (자작 팔 데이터 대기 안 함), 단일 task, 동일 embodiment. real 데이터 확장은 v2.5 (실기 전환 plan §7).
> **언어**: **Python** (HuggingFace transformers + peft) + **ROS2 (rclpy)** (eval 루프 재사용)
> **하드웨어**: 학습은 **RunPod RTX 4090** (Community Cloud, 24GB), 추론·eval 은 **로컬 RTX 4070 + 4-bit 양자화**
> **주간 시간**: 약 6-8시간 (출장 주 보정)
> **완료 (2026.09 초)**: Section 0-3 완료. 실측: `Measurements/openvla-maniskill-zeroshot/` (2026-08-03) · `Measurements/openvla-lora-runpod/` (2026-08-13). **후속 산출물: v2.5 (real, SmolVLA)** — 실기 전환 plan (`../docs/superpowers/plans/2026-08-30-realworld-transition-execution.md`) §7


---


## -> **실습 가이드**: `Studies/Phase 4.5/weekN/` (각 week 별 README + PRACTICE + quiz. **진입 시 다시 체크**)


**핵심 산출물 (v1.5)**:
- LoRA 파이프라인 (sim 데이터 포맷팅 -> RunPod 학습 -> 체크포인트 -> 로컬 머지/양자화)
- eval harness: zero-shot vs fine-tuned 를 **동일 조건 N회** 로 비교 (N 과 분산까지 보고)
- vla-lab 공개 문서 1편: adaptation **설계-실행-분석** 서사 (성공률 상승 여부와 무관하게 성립. 발행 채널: velog → vla-lab, 2026-08-30)


---


## 왜 별도 Phase 인가 (3-Layer 좌표)


- v1(Phase 4)은 **셋째 층** — 현 세대 FM 을 그대로 배포해 추론 루프를 닫는다(zero-shot, 카메라/bag dry-run). sim task 성공률 측정은 본 Phase 로 이관됐다.
- v1.5(본 Phase)는 **둘째 층** — FM 을 특정 태스크/환경에 맞추는 adaptation 의 증거. zero-shot 과 **대비**되어야 둘째 층 역량이 드러나므로 v1 과 섞지 않고 독립 산출물로 둔다.
- LoRA 를 Phase 6(v2)에서 본 Phase 로 **전진 배치** 했다. Phase 6 은 sim-to-real gap(셋째 층)에 집중하고, 둘째 층 증거의 타이밍을 실지원(2027) 전으로 앞당긴다.


---


## 성공 기준 (가장 중요 — 결과 비의존)


> **"성공률이 올랐다"가 성공 기준이 아니다.** 소규모 데이터 LoRA 는 zero-shot 대비 성공률을 못 올리거나(overfitting / catastrophic forgetting), 차이가 통계적 노이즈일 수 있다.


성공 기준은 **"adaptation 파이프라인을 설계-실행하고 결과를 정량 분석했다"** 이다.

- before/after 를 동일 조건 **N회** 로 측정하고, **N 과 분산(표준편차 또는 신뢰구간)** 을 함께 보고한다.
- 차이가 노이즈 범위 안이면, **그 사실과 원인 분석(데이터 규모/분포/학습 설정)** 자체를 산출물로 삼는다.
- 즉 negative 결과여도 "왜 안 올랐는가"의 분석이 둘째 층 역량의 증거가 되도록 마감 문서 (vla-lab) 논지를 구성한다.
- **before/after 가 신호를 만들려면 측정 지표가 바닥에 붙지 않아야 한다**: OpenVLA 의 sim zero-shot 성공률은 환경에 따라 편차가 크다 — real2sim 정합을 맞춘 Google Robot 계열에서는 중간대가 보고되지만 (visual matching 기준 pick coke can 16.3% / move near 46.2%), WidowX+Bridge 계열에서는 0% 근처다 (SimplerEnv, CoRL 2024). 즉 신호 유무를 결정하는 손잡이는 **task 난이도가 아니라 (1) 환경의 embodiment·카메라 규약 정합, (2) 지표의 해상도**다. 난이도를 낮춰 "중간 구간"을 만들려는 조정은 성립하지 않는다. 대응 두 가지: 최종 성공률과 함께 **단계별 부분 도달률**(reached / grasped / lifted / placed)을 측정해 before 쪽이 0 으로 잘리지 않게 하고, 0% 를 받았을 때 도메인 갭과 통합 버그를 구분할 수 있도록 하네스 검증을 먼저 통과시킨다 (Section 0). before/after 가 무의미해지는 조건은 before 가 0% 인 것이 아니라 **after 도 0%** 인 것이며, 그 경우는 위 negative 분석으로 받친다.


---


## 데이터: 왜 sim 생성 데이터인가


| 옵션 | 채택 여부 | 사유 |
|---|---|---|
| OpenX-Embodiment | 사용 안 함 | OpenVLA 가 OpenX 970K 로 **사전학습됨** -> 이미 본 데이터 재학습은 adaptation 증거로 부적합 |
| 자작 팔 teleop 데이터 | 대기 안 함 | 2027 이후라 둘째 층 증거가 실지원 타이밍보다 늦음 (real 확장은 Phase 7) |
| **sim 생성 데이터** | **채택** | OpenVLA **미학습 분포**로 구성 가능 + 타이밍 확보. v1 순서 3 의 sim 선정(ManiSkill)/embodiment 분석 재사용 (sim 환경 구축은 본 Phase 에서) |


> 진입 시 점검: 생성한 sim 데이터가 OpenVLA 사전학습 분포와 **겹치지 않는지** 확인해야 adaptation 의 의미가 산다.


---


## 컴퓨트 (리스크 — Step 0 실측 필수)


| 작업 | 환경 | 비고 |
|---|---|---|
| LoRA 파인튜닝 | **RunPod RTX 4090 (24GB)** | OpenVLA 7B LoRA 는 24GB+ 필요 -> RTX 4070(12GB) 불가 |
| 머지 + 4-bit 양자화 | 로컬 4070 | 약 6GB 로 축소 -> 12GB 안착 |
| eval 추론 루프 | 로컬 4070 + ROS2 | Phase 4 의 `vla_node`(추론 노드) 재사용. sim 단일 task 루프는 본 Phase 에서 신규 구축 |


- **가용성/비용이 변수**: Community Cloud 인스턴스는 회수될 수 있고 유휴 과금이 있다 -> network volume 체크포인트 필수, 작업 종료 시 pod 중지 (SETUP.md §5).
- **Step 0 실측 실패 시 분기**: LoRA 대상을 action head 인근 어댑터로 축소, sim 소량 데이터, before/after 를 단일 변형으로 한정, 주장 톤을 "경량 adaptation 실험"으로 (롤백 옵션 B).
- 컴퓨트 전략의 단일 진실 공급원은 [`Studies/Phase 4/SETUP.md`](../Studies/Phase%204/SETUP.md) 의 분업 원칙과 공유 — v1.5 는 그 "LoRA 파인튜닝 트랙"을 본 산출물로 승격한 것이다.


---


## Section 0: 시작 전 (Step 0 실측 + sim 구축·이관 — 2026.08)


> 순서 제약: **하네스 검증이 zero-shot baseline 측정보다 앞이다.** 검증 없이 측정하면 성공률 0% 를 받았을 때 도메인 갭과 통합 버그를 구분할 수 없어 수치가 해석 불가가 된다. 나머지 항목은 순서 자유.


- [ ] OpenVLA 7B LoRA 1회 사이클이 RunPod RTX 4090 에서 가능한지 실측 (시간/비용/체크포인트) — 학습 데이터 (Section 1 산출) 가 입력이라 학습 자료 기준 week3 에서 닫힌다. 마감은 week3 (LoRA 본 학습) 진입 전
- [ ] fine-tuned 모델 4-bit 추론이 RTX 4070 12GB 에 OOM 없이 올라가는지 확인 — LoRA 1사이클의 머지 체크포인트가 필요해 학습 자료 기준 week4 에서 닫힌다
- [ ] sim 데이터 생성 방식 확정 (어느 sim, 어떤 task, 수집 규모) + OpenVLA 미학습 분포인지 점검. **정합 요건과 미학습 요건은 상충하므로 함께 판단한다** — embodiment·카메라 규약은 정합시켜야 zero-shot 이 바닥을 벗어나고, task·물체·씬은 신규여야 adaptation 의 의미가 산다
- [ ] eval N 및 통계 처리(분산/신뢰구간) 기준 1차 결정
- [ ] **sim 환경 구축** (v1 순서 3 의 ManiSkill 선정/PickCube 정의 재사용, sim 자체는 v1 에 없으므로 본 Phase 에서 신규 구축)
- [ ] **하네스 검증** (zero-shot baseline 측정의 선행 조건) — 성공률과 독립적으로 루프·action 변환이 정상임을 입증한다. 수단 최소 1건: (a) ManiSkill 내장 motion planning / scripted solution 을 동일 루프에 투입해 성공률 상한 확인, (b) SimplerEnv 계열 bridge 평가 환경에서 공개된 OpenVLA zero-shot 수치와 자체 결과 대조
- [ ] **zero-shot 성공률 baseline 측정** (하네스 검증 통과 후)
- [ ] **before/after 신호 성립 점검**: 최종 성공률이 0% 로 나와도 **부분 도달률**(reached / grasped / lifted / placed)에 0 이 아닌 단계가 남는지 확인. 전 단계가 0 이면 조정 대상은 task 난이도가 아니라 **환경의 embodiment·카메라 규약 정합**이다 (성공 기준 절 참조)
- [ ] **학습 측 환경 Docker 컨테이너화** (2026.08 — LoRA 학습을 RunPod 에서 돌리기 위한 준비. eval·sim 은 로컬 유지 — remediation plan 결정 #5)
- [ ] **RunPod 에서 컨테이너 기동 + zero-shot 1회 추론 재현 확인** (2026.08 — 로컬 GPU 비의존 검증)


---


## Section 1: sim 데이터 생성 + 포맷팅 (1-2주, 2026.09)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 1 | sim task 정의 (**부분 도달률이 0 이 아닌 환경·지표 조합 확정**) + 데이터 수집 (관측↔action 페어) + 미학습 분포 점검 | sim 환경은 Section 0 에서 구축(v1 순서3 ManiSkill 분석 재사용). 규모는 LoRA 가 돌 최소선. 신호는 난이도가 아니라 환경 정합·지표 해상도에서 나온다 |
| 2 | OpenVLA 학습 포맷으로 변환 (RLDS / 모델 카드 스키마 확인) | 데이터 포맷팅이 adaptation 의 절반 |


> 보충: 후보 task 를 1-2개 zero-shot 으로 빠르게 돌려 최종 성공률과 부분 도달률을 함께 본다. 최종 성공률 0% 는 정상 범위이므로 그것만으로 task 를 바꾸지 않는다. 판정은 부분 도달률로 한다 — 전 단계가 0 이면 task 를 쉽게 만드는 대신 환경 정합(embodiment·카메라 규약)을 먼저 손본 뒤 본 데이터를 수집한다.


---


## Section 2: LoRA 학습 + 로컬 이식 (2-3주, 2026.09-10)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 3 | RunPod LoRA 파인튜닝 + 체크포인트 주기 저장 | 중단 대비 (SETUP.md §5.3) |
| 4 | LoRA 가중치 -> 로컬 rsync -> 머지 + 4-bit 양자화 + 호환성 검증 | 버전 매칭(SETUP.md §7) |


---


## Section 3: eval harness + before/after 분석 (2-3주, 2026.10-11)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 5 | eval harness: zero-shot vs fine-tuned 를 **동일 조건 N회** | N / 분산 / 신뢰구간 명문화 |
| 6 | before/after 정량 분석 + vla-lab 공개 문서 1편 + v1.5 공개 | negative 결과도 성립하는 논지 |


> eval 은 본 Phase 에서 구축한 sim 단일 task 루프를 쓰되 모델만 zero-shot / fine-tuned 로 바꿔 N회 반복한다. 변인은 모델 하나로 고정.


---


## Phase 4.5 완료 체크리스트


### adaptation 파이프라인
- [x] sim 생성 데이터가 OpenVLA 미학습 분포임을 확인 — week1 `outputs/distribution_check.md` (시각 도메인만 미학습, embodiment·task 는 겹침으로 정밀화)
- [x] LoRA 파인튜닝 1 사이클 완료 (RunPod) + 체크포인트 — week3 `outputs/train_log.md` (2,000스텝 완주, 어댑터 회수)
- [x] 로컬 머지 + 4-bit 양자화 + 호환성 검증 통과 — week4 `outputs/remerge_check.md`, `compat_check.md` (4층 검증)


### eval / 분석
- [x] before/after 신호가 성립하는 지표를 썼는지 확인 (최종 성공률 + 부분 도달률 병기, 부분 도달률이 전 단계 0 이 아님) — fine-tuned reached 92/98, grasped 75/98 (`Measurements/openvla-lora-eval/findings.md`)
- [x] zero-shot vs fine-tuned 를 동일 조건 N회 측정 (N 명시) — N=100, seed 0-99, 메타 차이 2항목 확인 (week5)
- [x] 성공률 차이를 분산(표준편차/신뢰구간)과 함께 보고 — placed 0/98 양쪽, 95% Wilson [0%, 3.8%] + 짝지은 판정 (week6 `outputs/results.md`)
- [x] 차이가 노이즈면 원인 분석을 산출물로 작성 (negative 대응) — week6 `outputs/causal_analysis.md` (배제 10건 / 잔여 7건)


### 산출물 v1.5 공개
- [ ] LoRA 파이프라인 + eval harness 코드 정리 + README — 코드·기록은 `Measurements/openvla-lora-eval/` 로 마감했으나 vla-lab (공개 repo) 이관·README 미발행
- [ ] vla-lab 공개 문서 1편 (adaptation 설계-실행-분석 서사 + 선정 시점 vs 마감 시점의 필드 변화 단락) + LinkedIn 링크 공유 — 초고 완성 (week6 `outputs/blog_draft.md`), 필드 변화 단락 미작성 + 발행·공유 대기 (LinkedIn 은 probe 2단 일정)


---


## Phase 4.5 완료 기준


> "OpenVLA 를 sim 생성 데이터로 LoRA adaptation 하고, zero-shot 대비 성공률을 동일 조건 N회로 측정해 분산까지 보고했다. 성공률 상승 여부와 무관하게, adaptation 을 **설계-실행-정량 분석**한 서사를 산출물로 공개할 수 있다. 단순 '학습이 돌았다'는 통과가 아니다."


---


## 참고 자료


- OpenVLA (Stanford, 2024): https://openvla.github.io/
- OpenVLA fine-tuning / LoRA: HuggingFace `peft` (https://huggingface.co/docs/peft)
- SimplerEnv (CoRL 2024) — real2sim 평가 벤치마크. 하네스 검증용 대조 수치와 환경 정합 근거의 출처: https://github.com/simpler-env/SimplerEnv
- 컴퓨트 분업 / 버전 매칭: [`Studies/Phase 4/SETUP.md`](../Studies/Phase%204/SETUP.md)
- [ENVIRONMENT.md](../ENVIRONMENT.md) — 프로젝트 공용 환경


---


## [?] 다음 단계


Phase 4.5 완료 후:
- **6개월 분기 재평가 #1 (2026.11)** — sim adaptation 이 AI 트랙 JD 에서 둘째 층 증거로 읽히는지 점검 (sim 증거의 설득력 한계)
- **Phase 6 (v2)**: sim-to-real gap(셋째 층)에 집중 — LoRA 는 본 Phase 로 이관됨
- **v2.5 (2026.11-12)**: 본 Phase 의 eval 논리 (N회·분산·부분 도달률·하네스 검증) 를 **SO-101 실기 + SmolVLA** 로 이식 (둘째 층 증거를 real 도메인으로 — 실기 전환 plan §7). **Phase 7 (v3)** 은 그 위에 안전 인터록·디지털 트윈 결합
