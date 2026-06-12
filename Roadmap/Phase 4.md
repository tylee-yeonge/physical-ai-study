# Phase 4: VLA v1 — OpenVLA zero-shot 추론 + ROS2 → sim 단일 task 루프


> **기간**: 약 4개월 (2026.06-09)
> **목표**: VLA 아키텍처 다이어그램을 막힘없이 읽는 수준 + pretrained OpenVLA zero-shot inference 를 ROS2 토픽으로 받아 **sim 단일 task 루프를 닫는다**(N회 성공률 기록)
> **범위 (v1)**: pretrained zero-shot (**LoRA adaptation 은 v1.5/Phase 4.5**), **sim embodiment** (자작 팔은 v2), 단일 task, 단일 embodiment
> **언어**: **Python** + **ROS2 (rclpy)**
> **하드웨어**: Ubuntu PC (RTX 4070) — HuggingFace inference / ROS2 노드 / 시각화
> **주간 시간**: 2026.06-07 은 아래 "진행 순서 변형" 절의 예산(계획선 78h)을 따른다. 8월 이후는 8월 초 체크포인트에서 확정 (잠정 주 6-8시간)


---


## -> **실습 가이드**: `Studies/Phase 4/weekN/` (각 week 별 README + PRACTICE + quiz 미리 작성됨. **진입 시 (2026.06) 다시 체크**)


**핵심 산출물 (v1)**:
- RT-2 + OpenVLA 블로그 (아키텍처 / 학습 / 데이터 / inference 흐름 + action representation 비교 축)
- OpenVLA zero-shot inference → ROS2 토픽 → **sim 단일 task 루프 (N회 시도 성공률 기록)** + 1분 영상


**산출물 v1** (2026 하반기까지 `physical-ai-study` 레포 + velog/LinkedIn 공개):
- 블로그: RT-2 / OpenVLA 의 아키텍처·학습·데이터·inference 흐름 + action representation(관절각/EE-delta/token) 비교
- ROS2 패키지: OpenVLA zero-shot inference → `vla_action` 토픽 → sim 단일 task 루프 + 시각화
- **성공 기준**: "추론 루프가 닫힌다"(action 텐서 출력)는 불충분. "정의된 task 1종(예: pick-and-place)에서 N회 시도 성공률 기록"이 어필 기준.
- 한계 명시: v1 의 Body 는 sim, 실암 결합은 v2 예고


> **선정 논문 (2편)**: RT-2, OpenVLA — 2026.11 분기 재평가에서 π0 / Helix / GR00T 등으로 갱신 가능. "현 세대 FM 배포"가 어필 포인트이므로 v1 착수 시 세대 점검 1회.


> **후속 산출물 예고**: v1.5(Phase 4.5) = adaptation(LoRA) / v2(Phase 6) = deployment(sim-to-real gap) / v3(Phase 7) = Real-to-Sim-to-Real 정점. 본 Phase 의 ROS2 wrapper + sim 루프 + 성공률 baseline 은 v1.5 의 eval harness 로 그대로 재사용되고, 이후 자작 팔과 결합된다.


---


## 진행 순서 변형 (2026.06-07 선행 투입)


6월 무출장 + 7월 주 1일 출장(해당일 1h)으로 6-7월 가용 시간이 크다 (최대 약 96h, 계획선 약 78h). 깊은 연속 블록은 환경 구축·디버깅에 가장 값어치가 크므로, 원안 순서(week1-7 정독 → week8-12 실습) 대신 실습을 선행한다. **주차 번호는 재부여하지 않는다** — 아래 표는 실행 순서일 뿐이며, week 디렉토리와 자료 구조는 원안 그대로다 (아래 "원안 week 구조 (자료 맵)" 절 참고).


실행 순서를 week 기준으로 펼치면 다음과 같다. **이 표의 행 순서가 곧 진행 순서다.**

| 순서 | 진행할 week | 작업 | 시기 | 계획 투입 |
|---|---|---|---|---|
| 1 | `SETUP.md` (week 아님) | Step 0: 환경 구축 + 레포 청소 + 4-bit 로드 + latency/VRAM 실측 (실측 코드는 `week6` 실습 1-2 선행 수행) | 6월 2주차 | 6h |
| 2 | `week4`-`week5` 일부만 | 표적 skim: action 표현 / unnorm_key / 입력 형식 / embodiment 가정 (+ OpenVLA 모델 카드, 공식 repo README). 정독 아님 — 정독은 순서 6 | 6월 2-3주차 | 4h |
| 3 | `week11` 일부 선행 | sim 환경 선정 + embodiment 정합 + 성공 task 1종 정의 (→ latency 판정 수치 확정) | 6월 2-3주차 | 12h |
| 4 | `week8` → `week9` → `week10` → `week11` → `week12` | 실습 압축: VLAInference → ROS2 노드 → sim 단일 task 루프 | 6월 3주 - 7월 중순 | 34h |
| 5 | `week11`-`week12` 마무리 | 성공률 N회 측정 + 결과 정리 | 7월 중순 - 말 | 22h |
| 6 | `week1` → `week7` 순서대로 | RT-2/OpenVLA 정독 + 블로그 2편 | 8-9월 | 50-70h 잠정 — 8월 초 체크포인트에서 갱신 (순서 7 포함) |
| 7 | `week13` → `week16` 순서대로 | 블로그 마무리 + 패키징 + 영상 + v1 공개 | 8-9월 | (순서 6 에 합산) |

- 순서 1-5 (6-7월) 합계 **78h** = 계획선 정합. 최대 가용(96h)과의 차이 약 18h 는 버퍼로, 막히는 구간(실습/측정)에 흡수한다.


- **계획선 미달 시 절삭 순서**: (1) 버퍼 소진 → (2) 성공률 측정 N 축소 → (3) 측정 항목 축소 (latency 분포는 유지, 부가 지표 절삭). v1 산출물 정의(루트 README 부록 B)를 건드리는 절삭은 별도 의사결정 — 발동 시 부록 B 갱신 + 2026.11 분기 재평가 #1 안건 등재가 전제다.
- **작업 성격과 시간 블록 매칭**: 평일 오전 보장 블록(09-12, 연속 3h) = 환경 구축/디버깅/실습 전용. 출장일 저녁 1h·오후 잔여 시간 = 논문 섹션 읽기/노트/quiz 등 파편화 가능 작업 전용. 오후 학습은 계획에 넣지 않고 버퍼·초과분 흡수처로만 쓴다.
- **오전 블록 사수율 검증**: 보장 블록은 오전 회의를 거절할 수 있을 때만 성립한다. 첫 2주간 사수율을 실측하고, 깨지는 비율이 높으면 보장 블록을 오후로 반전한다.
- **8월 초 체크포인트**: v1 기술 코어(순서 5) 확보 직후 8-12월 예산·배치를 확정한다. 입력: 오전 블록 사수율 실측(6-7월), 출장 빈도(월 5회 이하 예상), v1 진행 결과. 1순위 검토 안건: Phase 4.5(v1.5) 의 9-10월 조기 진입 — 2026.11 분기 재평가를 가설이 아닌 실물(v1.5 결과) 위에서 수행하기 위함. 순서 6-7 의 "50-70h" 는 이 체크포인트 전까지의 잠정치다.
- **RT-2 정독 후행**: RT-2 는 비공개 모델로 순수 개념·블로그용이므로 정독 전량을 8-9월로 이동한다. 6월의 표적 skim 은 구현에 필수인 부분집합(action 표현 / unnorm_key / 입력 형식 / embodiment 가정)만 다룬다.

### 실행 체크리스트 (파일·섹션 단위)

위 표의 각 순서를 실제로 열어볼 파일과 섹션 단위로 펼친 진행 보드. 위에서 아래로 순서대로 진행하고, 완료한 항목에 체크한다. 모든 경로는 `Studies/Phase 4/` 기준.

#### 순서 1 — Step 0: 환경 구축 + 레포 청소 + 실측 (6월 2주차, 6h)

- [x] `SETUP.md` §2 사전 점검 체크리스트 통과 (계정 / 로컬 드라이버·디스크·ROS2 / 공통 도구)
- [x] `SETUP.md` §6 로컬 환경 세팅 — 공용 venv `.venv-vla` 생성은 `week8/PRACTICE.md` "환경 설정" 절의 명령 사용
- [x] `SETUP.md` §1.3 실측치 표 숙지 — 이번 실측이 검증할 추정치 (int4 약 7GB / 약 2-3 Hz)
- [x] git 커밋 신원 정리 — 작업 컨테이너에 `git config user.name` / `user.email` 을 GitHub 계정에 연결된 개인 이메일(또는 noreply)로 설정. author `root` + 사내 이메일 상태로는 contribution graph 에 연결되지 않고 개인 포트폴리오에 사내 이메일이 남는다. 기존 커밋 히스토리의 author 재작성(`git filter-repo`) 여부는 v1 공개 전 결정 — 공개·fork 이후에는 불가
- [x] `predict_action` 호출 패턴 일괄 청소 (**순서 4 진입 전 필수**) — `grep -rn "predict_action(\*\*inputs" .` 으로 잔존 위치 확인 후 `input_ids` / `pixel_values` 만 전달하도록 통일. 근거: `predict_action` 은 빈 토큰(29871)을 input_ids 에만 덧붙이므로 processor 출력의 attention_mask 를 그대로 넘기면 eager attention 에서 off-by-one 으로 깨진다 (week6 PRACTICE 는 수정 완료, `week6/README.md`·`week8`-`week12` 자료·quiz 잔존 확인 대상)
- [x] `week6/PRACTICE.md` 실습 1 (첫 OpenVLA 4-bit inference) — OOM 없이 로드되는지
- [x] `week6/PRACTICE.md` 실습 2 (latency 측정 100회 + 통계 + 결과 저장) — mean/p95 와 `nvidia-smi` VRAM 기록
- [x] 실측치를 `SETUP.md` §1.3 추정치와 비교 — 추정 범위(±50%)를 벗어나면 §1.3 을 실측 기준으로 갱신 (실측 mean 300.3 ms / 3.33 Hz, ±50% 범위 내 — §1.3 에 실측 행 추가)

#### 순서 2 — 표적 skim (6월 2-3주차, 4h)

skim 목적은 구현에 필요한 사실 확인이지 정독이 아니다. 항목별로 답을 노트 1페이지에 적으면 끝.

- [ ] 라이선스 확인: OpenVLA HF 모델 카드 — 코드 MIT / weights 는 Llama 2 license. v1 공개물(블로그·영상·레포)에서의 사용 조건 확인
- [ ] action 표현: `week5/README.md` §3 (Action space 표준화) + §3.5 (관절각/EE-delta/token 비교 축) — OpenVLA 출력 7-DoF 가 무엇을 의미하는지
- [ ] unnorm_key / 입력 형식: OpenVLA HF 모델 카드의 사용 예시 코드 + `week6/README.md` "핵심 개념"의 모델 로드/추론 코드 절 — prompt 형식과 `predict_action(input_ids=..., pixel_values=..., unnorm_key=..., do_sample=False)` 호출. **주의**: processor 출력 전체를 `**inputs` 로 넘기는 패턴은 attention_mask 동봉으로 크래시한다 (순서 1 청소 항목의 근거 참조). 참고 자료에 옛 패턴이 남아 있으면 그 자리에서 수정
- [ ] embodiment 가정: `week5/README.md` §1 (OpenX-Embodiment 구조) + §2 (대표 embodiment 특징) + 공식 repo README — 어떤 로봇/카메라 시점을 전제로 학습됐는지, sim 이 거기에 맞을 수 있는지
- [ ] 아키텍처 최소 골격: `week4/README.md` §5 (Architecture Diagram) + "한 페이지 OpenVLA 요약"의 입출력 인터페이스 절

#### 순서 3 — sim 정합 + 성공 task 정의 (6월 2-3주차, 12h)

이 구간은 대응하는 week 자료가 없는 신규 작업이다 (원안에서 week11 에 묻혀 있던 선행 의사결정을 분리한 것).

- [ ] 순서 2 의 embodiment 가정에 맞는 sim 후보 비교·선정 — 선정 사유를 노트로 기록
- [ ] 성공 task 1종 + 성공률 기준 N 정의
- [ ] task 의 제어 주기 요구 확정 → `week8/PRACTICE.md` 실습 체크리스트의 latency placeholder ("2 Hz 이상") 를 확정 수치로 교체
- [ ] `week11/README.md` §4 (1분 dry-run 의 success criteria) 미리 읽기 — 순서 4 의 종착점 파악

#### 순서 4 — week8-12 실습 압축 (6월 3주 - 7월 중순, 34h)

각 week 공통 패턴: `README.md` 정독 → `PRACTICE.md` 실습 → `quiz_easy.py` / `quiz_medium.py` (출장일 저녁 등 파편 시간에 배치 가능). 진입 전제: 순서 1 의 `predict_action` 패턴 청소 완료.

- [ ] `week8/README.md` + `week8/PRACTICE.md` 실습 1-4 (VLAInference class / image preprocess / exceptions·config / 100회 stress test) + quiz
- [ ] `week9/README.md` + `week9/PRACTICE.md` 실습 1-3 (I/O spec 1페이지 / msg <-> Python 변환 / BGR->RGB 검증) + quiz
- [ ] `week10/README.md` + `week10/PRACTICE.md` 실습 1-4 (vla_node 패키지 생성 / 골격 노드 / setup.py / 빌드+실행, 실습 5 dummy image 는 선택) + quiz
- [ ] `week11/README.md` + `week11/PRACTICE.md` 실습 1-3 (실 inference 통합 / 빌드+실행 / 1분 dry-run + 통계) — 입력은 순서 3 에서 선정한 sim 으로 연결 (자료의 ros2 bag 재생은 대체 수단) + quiz
- [ ] `week12/README.md` + `week12/PRACTICE.md` 실습 1-2 (Rerun 기본 / rerun_logger 노드) — 영상 제작(실습 3-5)은 순서 7 로 후행 + quiz

#### 순서 5 — 성공률 측정 + 결과 정리 (7월 중순 - 말, 22h)

- [ ] 정의된 task 를 N회 시도, 성공률 기록 — 측정/통계 코드는 `week11/PRACTICE.md` 실습 3 패턴 재사용
- [ ] mean/p95 latency 실측 + task 제어 주기 충족 여부 판정 (`week8/PRACTICE.md` 실습 체크리스트의 확정 기준)
- [ ] 결과 표 정리 — `week12/PRACTICE.md` 실습 5 의 "측정 결과" 표 형식 참고. 여기까지가 v1 기술 코어 (7월 말 목표)
- [ ] **8월 초 체크포인트 수행** — 본 절 상단 bullet 의 입력으로 8-12월 예산·배치 확정 (순서 6-7 잠정치 갱신, v1.5 조기 진입 여부 결정)

#### 순서 6 — week1-7 정독 + 블로그 2편 (8-9월)

- [ ] `week1/` RT-2 1회독 + reading note (PRACTICE 실습 1-3) + quiz
- [ ] `week2/` Co-fine-tuning + action tokenization (PRACTICE 실습 1-3) + quiz
- [ ] 블로그 플랫폼 선정 (Velog / Medium / 본 레포의 `Studies/Phase 4/blog/`) — week3 발행 전 결정
- [ ] `week3/` RT-2 블로그 1편 작성 + 발행 (PRACTICE 실습 1-4)
- [ ] `week4/` OpenVLA 정독 — 순서 2 에서 안 본 부분 중심 (hybrid vision encoder, contribution, 한계) + 실습 1-3 + quiz
- [ ] `week5/` OpenX-Embodiment + LoRA 흐름 정독 + 실습 1-4 + quiz
- [ ] `week6/` README 개념 보충만 — 실습 1-2 는 순서 1 에서 이미 수행, 실습 3 (에러 기록) 은 미기록분 보완
- [ ] `week7/` OpenVLA 블로그 1편 작성 + 발행 — 실측 결과 섹션에 순서 1/5 의 수치 사용 (PRACTICE 실습 1-4)

#### 순서 7 — 패키징 + v1 공개 (8-9월)

- [ ] `week12/PRACTICE.md` 실습 3-5 (1분 영상 시나리오 / 녹화+편집 / Portfolio 패키징) — 순서 4 에서 미룬 분량
- [ ] `week13/` 블로그 2편 퇴고 + 다이어그램 통일 (PRACTICE 실습 1-4)
- [ ] `week14/` ROS2 demo README + 환경 세팅 가이드 + 검증 (PRACTICE 실습 1)
- [ ] `week15/` 영상 자막/thumbnail/최종 export (PRACTICE 실습 1-5)
- [ ] `week16/` 산출물 v1 공개 + 회고 + Phase 4.5 진입 준비 (PRACTICE 실습 1-5)


---


## 학습 환경


| 단계 | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| RT-2 / OpenVLA 논문 정독 | 디바이스 무관 | O |
| HuggingFace inference 셋업 (4-bit 양자화) | Ubuntu PC (원격) | O |
| ROS2 패키지 작성 + sim 단일 task 루프 | Ubuntu PC (원격) | O |
| 블로그 작성 | 디바이스 무관 | O |
| (v1.5) LoRA 파인튜닝 | **Colab A100/L4 (클라우드) — Phase 4.5 범위** | O |


- **VRAM 비대칭 (컴퓨트 리스크, 실측 검증 완료)**: OpenVLA 7B 의 BF16 가중치는 약 14-15GB → RTX 4070 12GB 로 **풀 정밀도 추론 불가**. int4 양자화 시 약 7GB (논문 실측) 로 12GB 안에 안착 — 순서 1 실측에서 OOM 없이 로드 확인. LoRA 파인튜닝은 24GB+ 필요 → 4070 으로 사실상 불가(v1.5/Phase 4.5 에서 Colab).
- **v1 착수 시 1회 실측 (완료, 2026-06 순서 1)**: int4 OpenVLA 7B 가 RTX 4070 12GB 에 OOM 없이 로드되고, 추론 latency 실측 mean 300.3 ms (**3.33 Hz**, n=100) — 외삽 추정(약 2-3 Hz)의 ±50% 범위 내. 제어 주기 충족 여부 판정은 순서 3 의 task 선정 후 확정하며, 미충족 시 범위 축소(관측 → action 예측 → 시각화, 제어 루프 주장 보류)로 분기. 수치 본체: [`Studies/Phase 4/SETUP.md`](../Studies/Phase%204/SETUP.md) §1.3.
- **분업 원칙**: 무거운 학습(v1.5 LoRA)은 Colab, 가벼운 추론(v1)은 로컬 4070 + ROS2. 이 컴퓨트 표는 v1 과 v1.5 가 **공유**한다 (v1.5 의 단일 진실 공급원은 [`Studies/Phase 4/SETUP.md`](../Studies/Phase%204/SETUP.md)).
- **원격 워크플로우**: 사무실 오전 블록의 GPU 의존 작업은 자택 4070 에 Tailscale/VS Code tunnel 로 원격 수행. 경로는 순서 1 실측에서 겸사 검증.
- 상세:
  - [Studies/Phase 4/SETUP.md](../Studies/Phase%204/SETUP.md) — Phase 4 진입 전 환경 구축 단일 진실 공급원
  - [ENVIRONMENT.md](../ENVIRONMENT.md) — 프로젝트 공용 환경


---


## 원안 week 구조 (자료 맵)


각 week 디렉토리(`Studies/Phase 4/weekN/`)가 담는 내용의 참조 표. **실행 순서와 체크 항목의 단일 진실 공급원은 위 "진행 순서 변형" 절이다** — 이 표는 자료가 어디에 있는지 찾을 때만 쓴다.

| week | 구간 | 내용 | 핵심 |
|---|---|---|---|
| 1 | RT-2 정독 | RT-2 논문 1회독 + Architecture Diagram 정독 | PaLI-X / PaLM-E 기반 |
| 2 | RT-2 정독 | Co-fine-tuning + Action tokenization 이해 | "Action 도 토큰" |
| 3 | RT-2 정독 | 블로그 1편 작성 (RT-2) | 핵심 아키텍처 + 데이터 + 한계 |
| 4 | OpenVLA 정독 | OpenVLA 논문 1회독 + Architecture | Llama 7B + DINOv2 + SigLIP |
| 5 | OpenVLA 정독 | OpenX-Embodiment 데이터 + Fine-tuning 흐름 + 스키마/action representation 정독 | 970K episodes / observation↔action 페어링 / 관절각·EE-delta·token 비교 |
| 6 | OpenVLA 정독 | OpenVLA HuggingFace 모델 카드 + 환경 셋업 (4-bit 양자화) | `transformers` + GPU |
| 7 | OpenVLA 정독 | 블로그 1편 작성 (OpenVLA) | RT-2 와 비교 + open-source 의 의미 + action representation 비교 축 (week5 §3.5) |
| 8 | sim task 루프 | HuggingFace inference 셋업 + 컴퓨트 측정 | 4-bit quantization, VRAM/latency 수치 |
| 9 | sim task 루프 | inference 입력/출력 인터페이스 정리 + 제어 인터페이스 매핑 | image + instruction → action / 이동 플랫폼이면 매핑이 어떻게 달라지나 |
| 10 | sim task 루프 | ROS2 패키지 골격 작성 (Phase 3 week8 스캐폴드 재사용) | `vla_node` 노드 + `vla_action` 토픽 |
| 11 | sim task 루프 | sim embodiment 연결 → zero-shot 추론 → 단일 task 루프 | N회 시도 성공률 기록 |
| 12 | sim task 루프 | Rerun 시각화 + 1분 영상 제작 | Demo 영상 마감 |
| 13 | 패키징·공개 | 블로그 2편 퇴고 + 다이어그램 작성 | 면접관 진입점 |
| 14 | 패키징·공개 | ROS2 demo README + 환경 세팅 가이드 + 성공률 표 | 재현성 |
| 15 | 패키징·공개 | 1분 데모 영상 마감 + 자막 | 패키징 |
| 16 | 패키징·공개 | 산출물 v1 공개 (`physical-ai-study/Portfolio/01_VLA_v1/` + velog + LinkedIn) | 2026 하반기, 정찰 지원 카드 |

week8-12 (sim task 루프) 는 **Brain ↔ Body 의 첫 통합**이자 v2/v3 의 예고편 — Phase 3 week8 통합 노드를 wrapper 스캐폴드로 재사용한다.

**ROS2 인터페이스 예시** (week8-12):
```
Input  : /camera/image_raw (sensor_msgs/Image), /vla/instruction (std_msgs/String)
Output : /vla/action (팔: EE-delta [dx,dy,dz,rx,ry,rz] -> geometry_msgs/Twist + gripper Float64, 또는 custom msg)
```

> 메시지 선택 상세는 `Studies/Phase 4/week9` §1. 같은 `Twist` 가 **이동 플랫폼에서는 base 선속도/각속도**로 의미가 바뀐다 — 팔->이동 이식 시 재매핑 지점이자 cross-embodiment(부록 D 2026.11 재평가) 의 기술적 단서.

> **공개 글의 논지 3축** (week13-16): (1) 현 세대 FM 을 배포해 단일 task 루프를 닫음, (2) 양자화 배포 + ROS2 통합을 프로덕션 관점으로 처리(AMR 미들웨어 ROS 5년 경력 연결), (3) perception → VLA → (예정) 자작 팔 궤적. v1 의 Body 가 sim 임을 솔직히 명시, 실암 결합은 v2 예고.


---


## Phase 4 완료 체크리스트


### 논문 reading
- [ ] RT-2 아키텍처 다이어그램을 막힘없이 설명 가능
- [ ] OpenVLA 아키텍처 다이어그램을 막힘없이 설명 가능
- [ ] RT-2 와 OpenVLA 의 차이 (open-source / 모델 크기 / 데이터) 정리
- [ ] action representation (관절각 / EE-delta / token) 을 모델 간 비교하고, 그 출력이 제어 인터페이스 및 다른 플랫폼(이동 로봇) 과 어떻게 연결되는지 설명 가능


### 블로그
- [ ] RT-2 블로그 1편 공개
- [ ] OpenVLA 블로그 1편 공개


### v1 sim 단일 task 루프
- [ ] 컴퓨트 점검 (4-bit OpenVLA 7B 가 RTX 4070 12GB 에서 OOM 없이 동작 + latency 제어 주기 이내)
- [ ] OpenVLA zero-shot inference 동작 (sim embodiment 정합)
- [ ] `vla_node` ROS2 패키지 빌드 + 실행 (Phase 3 week8 스캐폴드 재사용)
- [ ] 정의된 단일 task 에서 **N회 시도 성공률 기록** (단순 action 출력으로는 불충분)
- [ ] 1분 데모 영상 제작


### 산출물 v1 공개
- [ ] `physical-ai-study` 레포 산출물 v1 디렉토리 정리 + README + 성공률 표
- [ ] velog + LinkedIn 공개 (첫 공개 산출물)


---


## Phase 4 완료 기준


> "RT-2 와 OpenVLA 의 아키텍처를 막힘없이 설명할 수 있고, pretrained OpenVLA zero-shot inference 를 ROS2 토픽으로 받아 sim 단일 task 루프를 닫아 N회 성공률을 기록한다. 그리고 action representation 이 실제 제어 인터페이스 및 다른 플랫폼(이동 로봇) 과 어떻게 연결되는지 설명할 수 있다. 단순 action 출력은 통과가 아니다."


---


## 참고 자료


### 논문
- RT-2 (Google DeepMind, 2023): https://robotics-transformer2.github.io/
- OpenVLA (Stanford, 2024): https://openvla.github.io/


### 도구
- HuggingFace transformers: https://huggingface.co/docs/transformers
- OpenVLA HuggingFace 모델: https://huggingface.co/openvla
- ROS2 (Humble 또는 Iron): https://docs.ros.org/


---


## [?] 다음 단계


Phase 4 완료 후 (2026 하반기):
- **Phase 4.5 (VLA v1.5)** — OpenVLA LoRA adaptation + before/after 정량 분석 (둘째 층 증거). v1 우선 공개 후 착수가 원안이나, 8월 초 체크포인트에서 9-10월 조기 진입 여부 결정 ([Phase 4.5.md](Phase%204.5.md))
- **하드웨어 스파이크** (2026.10, 2-3주) — 2-DOF Dynamixel + ROS2 파이프라인 리스크 검증 (조달은 v1 과 병렬로 이미 착수)
- **정찰 지원** (2026.11-12, 2-3개사, 합격 기대 X, 반응 측정)
- **6개월 분기 재평가 #1 (2026.11)** — 스파이크 결과 / v1 반응 / VLA 모델 갱신 검토 / 시장 신호 probe 반응 / cross-embodiment 좌표 점검
- **Hardware-Arm Stage 1 본 빌드** (2027.01-02) → v2 선행 하드웨어
- **Phase 6 (Isaac Sim 디지털 트윈) + 자작 팔 결합 → 산출물 v2** (헤드라인, sim-to-real gap)
