# Phase 4: VLA v1 — OpenVLA zero-shot 추론 + ROS2 추론 루프 (카메라/bag dry-run)


> **기간**: 약 4개월 (2026.06-09)
> **목표**: VLA 아키텍처 다이어그램을 막힘없이 읽는 수준 + pretrained OpenVLA zero-shot inference 를 ROS2 토픽으로 받아 **추론 루프를 닫고**(카메라/bag 입력 → action 토픽) **1분 dry-run 으로 latency/throughput/안정성을 측정**한다. sim 단일 task 성공률은 Phase 4.5(v1.5)로 이관
> **범위 (v1)**: pretrained zero-shot (**LoRA adaptation 은 v1.5/Phase 4.5**), 카메라/bag 입력 dry-run (**sim embodiment 결합 + task 성공률은 v1.5/Phase 4.5**, 자작 팔은 v2), 단일 embodiment
> **언어**: **Python** + **ROS2 (rclpy)**
> **하드웨어**: Ubuntu PC (RTX 4070) — HuggingFace inference / ROS2 노드 / 시각화
> **주간 시간**: 2026.06-07 은 아래 "진행 순서 변형" 절의 예산(계획선 78h)을 따른다. 8월 이후는 8월 초 체크포인트에서 확정 (잠정 주 6-8시간)


---


## -> **실습 가이드**: `Studies/Phase 4/weekN/` (각 week 별 README + PRACTICE + quiz 미리 작성됨. **진입 시 (2026.06) 다시 체크**)


**핵심 산출물 (v1)**:
- OpenVLA zero-shot inference → ROS2 토픽 → **추론 루프 (카메라/bag 입력 → action 토픽, 1분 dry-run 측정)** + 레포 결과 기록 (README + latency/throughput 표). sim 단일 task 성공률은 Phase 4.5 로 이관
- RT-2/OpenVLA 정독 — 아키텍처를 막힘없이 설명하는 수준 (vla-lab 문서 작성은 v2 로 이관)
- v1 엔지니어링 범위 (2026-07 확장): `RobotPolicy` adapter 추상화 + action schema validation + 벤치마크 재현성 — 상세는 아래 산출물 v1


**산출물 v1** (2026 하반기까지 `physical-ai-study` 레포에 **결과 기록만**, 외부 공개는 v2 로 이관):
- ROS2 패키지: OpenVLA zero-shot inference → `vla_action` 토픽, 카메라/bag 입력으로 1분 dry-run + (선택) Rerun 시각화 스크린샷/짧은 gif
- 레포 기록: README + latency/throughput 표. vla-lab 공개·LinkedIn 공유와 1분 영상·문서 작성은 v2(Phase 6)로 이관
- **성공 기준 (v1)**: 추론 루프가 닫히고(카메라/bag 이미지 입력 → action 토픽 연속 발행) 1분 dry-run 에서 mean/p95 latency·throughput·`0 fail`·GPU<10GB 를 측정·기록. sim 단일 task 성공률(N회)은 sim embodiment 결합이 필요하므로 Phase 4.5(v1.5)로 이관 — v1 에서는 task 성공 여부를 판정하지 않는다.
- **adapter 추상화**: 추론 노드가 모델 구현에 직접 결합되지 않도록 `RobotPolicy` 인터페이스로 분리 (모델 교체 대비 — 2026.11 재평가의 모델 갱신 시나리오 대응)
- **action schema validation**: 출력 7-DoF 의 범위·NaN·급변 검증 레이어 (안전 실행 스택 전체가 아니라 스키마 검증까지 — watchdog/fallback/collision 은 검증 대상 실팔이 생기는 v2 로 이관)
- **벤치마크 재현성**: latency 측정을 스크립트 + 고정 조건 문서 (`Measurements/` 연동) 로 재현 가능하게
- 한계 명시: v1 은 sim 없이 카메라/bag dry-run 까지. sim task 성공률은 v1.5, 실팔 결합은 v2 예고

> **v1 범위 축소 결정 (2026-06)**: (1) 공개 측면 — v1 은 공개 산출물로 약해, 다듬은 영상·vla-lab 문서·패키징·LinkedIn 공개에 50-70h 를 투입하는 것은 오배치다. 공개 푸시는 실제 팔이 결합되는 v2 가 훨씬 강하므로 **vla-lab 문서 작성·1분 영상·패키징·외부 공개를 모두 v2 로 이관**한다. (2) sim 측면 — sim(ManiSkill) 은 v1 에서 한 번도 구축되지 않았고, sim zero-shot 성공률은 도메인 갭으로 0% 근처 + N=20 신뢰구간이 ±22%p 라 통계적 의미가 약하다. sim 구축·정합·성공률 측정은 sim 을 본격 쓰는 Phase 4.5(eval harness)로 이관하고, **v1 은 sim 없이 추론 루프 dry-run(latency/throughput) + 정독(이해)까지를 레포에 기록**하는 것으로 정의한다. ManiSkill 선정·PickCube 정의·제어 주기 분석(notes.md 순서 3)은 4.5 에서 재사용한다. RT-2/OpenVLA 정독은 v2 문서 (vla-lab) 의 배경으로 재사용된다.


> **선정 논문 (2편)**: RT-2, OpenVLA — 2026.11 분기 재평가에서 π0 / Helix / GR00T 등으로 갱신 가능. "현 세대 FM 배포"가 어필 포인트이므로 v1 착수 시 세대 점검 1회.


> **후속 산출물 예고**: v1.5(Phase 4.5) = adaptation(LoRA) + sim 결합·성공률 측정 / v2(Phase 6) = deployment(sim-to-real gap) / v3(Phase 7) = Real-to-Sim-to-Real 정점. 본 Phase 의 ROS2 wrapper(추론 노드 + `vla_action` 토픽)는 v1.5 의 eval harness 로 그대로 재사용되고, sim 환경 구축과 zero-shot 성공률 baseline 측정은 v1.5 에서 처음 수행된다. 이후 자작 팔과 결합된다.


---


## 진행 순서 변형 (2026.06-07 선행 투입)


6월은 하루 5-8h 확보로 잔여 캐파 50-80h(평일 10일, 주말 제외). 7월은 전면 불확실(하루 1-2h ~ 8h). 따라서 순서 4-5(56h)를 6월 내 완주하고 7월은 계획에서 비의존 처리한다(나면 순서 6 선행, 안 나면 8월). 깊은 연속 블록은 환경 구축·디버깅·통합에 가장 값어치가 크므로 6월 종일 가용일에 우선 배치한다. 원안 순서(week1-7 정독 → week8-12 실습) 대신 실습을 선행한다. **주차 번호는 재부여하지 않는다** — 아래 표는 실행 순서일 뿐이며, week 디렉토리와 자료 구조는 원안 그대로다 (아래 "원안 week 구조 (자료 맵)" 절 참고).


실행 순서를 week 기준으로 펼치면 다음과 같다. **이 표의 행 순서가 곧 진행 순서다.**

| 순서 | 진행할 week | 작업 | 시기 | 계획 투입 |
|---|---|---|---|---|
| 1 | `SETUP.md` (week 아님) | Step 0: 환경 구축 + 레포 청소 + 4-bit 로드 + latency/VRAM 실측 (실측 코드는 `week6` 실습 1-2 선행 수행) | 6월 2주차 | 6h |
| 2 | `week4`-`week5` 일부만 | 표적 skim: action 표현 / unnorm_key / 입력 형식 / embodiment 가정 (+ OpenVLA 모델 카드, 공식 repo README). 정독 아님 — 정독은 순서 6 | 6월 2-3주차 | 4h |
| 3 | `week11` 일부 선행 | sim 환경 선정 + embodiment 정합 + 성공 task 1종 정의 (→ latency 판정 수치 확정) | 6월 2-3주차 | 12h |
| 4 | `week8` → `week9` → `week10` → `week11` → `week12` | 실습 압축: VLAInference → ROS2 노드 → 카메라/bag 1분 dry-run | 6/17 - 6/26(목표) | 34h |
| 5 | `week11`-`week12` 마무리 | dry-run 결과 정리 (latency/throughput 표) + 8월 체크포인트 | 6/30(목표) | 4h |
| 6 | `week1`,`week2`,`week4`,`week5`,`week6` (정독만) | RT-2/OpenVLA 정독 — 아키텍처 이해 (vla-lab 문서 작성 `week3`/`week7` 은 v2 로 이관) | 파편 시간 (Phase 4.5 실행 대기) | 20-30h 잠정 — `week2` 실측으로 확정 |
| (이관) | `week3`,`week7`,`week12`(실습 3-5),`week13`-`week16` | vla-lab 문서 작성 + 패키징 + 1분 영상 + 외부 공개 → **v2(Phase 6)로 이관** | (v2) | (v1 제외) |

- 순서 1-5 (6-7월) 합계 약 **60h** = 계획선 (순서 5 가 성공률 측정 제외로 22h→4h 로 축소). 최대 가용(96h)과의 차이는 버퍼로, 막히는 구간(실습·통합·dry-run 디버깅)에 흡수한다.
- 2026-06-17 갱신: 순서 1-3 완료(6+4+12 = 22h 소진). 잔여 순서 4-5 = 38h 를 6월 잔여(50-80h, 평일 10일)에 배치. 6월 내 완주가 기준선이며, 5h 추세로 미달 시 절삭 규칙(notes.md) 발동.


- **계획선 미달 시 절삭 순서**: (1) 버퍼 소진 → (2) dry-run 부가 지표 축소 (latency 분포는 유지, 그 외 절삭). (성공률 측정 N 축소는 sim 을 v1 에서 빼면서 더 이상 v1 절삭 대상이 아니다 — Phase 4.5 사안.) v1 산출물 정의(루트 README 부록 B)를 건드리는 절삭은 별도 의사결정 — 발동 시 부록 B 갱신 + 2026.11 분기 재평가 #1 안건 등재가 전제다.
- **작업-시간 매칭**: 연속 블록(통합·디버깅·dry-run)은 6월 종일 가용일에 우선 배치한다. 순서 5 결과 정리·quiz 는 짧은 블록에서도 가능하므로 7월 단시간 가용일의 흡수처로 둔다.
- **8월 체크포인트** (2026-08-12 종료): 8-12월 예산·배치를 확정했다. 결론 4건 (Phase 4.5 8월 완주 / 스파이크 2026.10 실행·9월 구매 — **2026-08-30 실기 전환으로 대체: 즉시 구매·스파이크 2026.09** / 정독 잠정치는 `week2` 실측으로 확정 / git filter-repo 미실행) 은 [notes.md](../Studies/Phase%204/notes.md) 의 "8월 체크포인트 결론" 에 있다.
- **RT-2 정독 후행**: RT-2 는 비공개 모델로 순수 개념용이므로 정독 전량을 8-9월로 이동한다. 6월의 표적 skim 은 구현에 필수인 부분집합(action 표현 / unnorm_key / 입력 형식 / embodiment 가정)만 다룬다. (RT-2 문서 작성은 v2 로 이관 — 정독만 v1 범위.)

### 실행 체크리스트 (파일·섹션 단위)

진행 보드는 [`Studies/Phase 4/notes.md`](../Studies/Phase%204/notes.md) 에 있다 — 순서 1-6 의 파일·섹션 단위 체크 항목과 노트 산출물을 한 파일에서 관리하며, 체크 표기도 그 파일에서만 갱신한다 (순서 7 은 v2 로 이관). 본 문서에는 계획(위 순서 표 + 진행 원칙 bullet)만 남긴다.


---


## 학습 환경


| 단계 | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| RT-2 / OpenVLA 논문 정독 | 디바이스 무관 | O |
| HuggingFace inference 셋업 (4-bit 양자화) | Ubuntu PC (원격) | O |
| ROS2 패키지 작성 + 카메라/bag dry-run | Ubuntu PC (원격) | O |
| vla-lab 문서 작성 | 디바이스 무관 | O |
| (v1.5) LoRA 파인튜닝 | **RunPod RTX 4090 (클라우드) — Phase 4.5 범위** | O |


- **VRAM 비대칭 (컴퓨트 리스크, 실측 검증 완료)**: OpenVLA 7B 의 BF16 가중치는 약 14-15GB → RTX 4070 12GB 로 **풀 정밀도 추론 불가**. int4 양자화 시 약 7GB (논문 실측) 로 12GB 안에 안착 — 순서 1 실측에서 OOM 없이 로드 확인. LoRA 파인튜닝은 24GB+ 필요 → 4070 으로 사실상 불가(v1.5/Phase 4.5 에서 RunPod).
- **v1 착수 시 1회 실측 (완료, 2026-06 순서 1)**: int4 OpenVLA 7B 가 RTX 4070 12GB 에 OOM 없이 로드되고, 추론 latency 실측 mean 300.3 ms (**3.33 Hz**, n=100) — 외삽 추정(약 2-3 Hz)의 ±50% 범위 내. 제어 주기 충족 여부 판정은 순서 3 의 task 선정 후 확정하며, 미충족 시 범위 축소(관측 → action 예측 → 시각화, 제어 루프 주장 보류)로 분기. 수치 본체: [`Studies/Phase 4/SETUP.md`](../Studies/Phase%204/SETUP.md) §1.3.
- **분업 원칙**: 무거운 학습(v1.5 LoRA)은 RunPod, 가벼운 추론(v1)은 로컬 4070 + ROS2. 이 컴퓨트 표는 v1 과 v1.5 가 **공유**한다 (v1.5 의 단일 진실 공급원은 [`Studies/Phase 4/SETUP.md`](../Studies/Phase%204/SETUP.md)).
- **원격 워크플로우**: 사무실에서의 GPU 의존 작업은 자택 4070 에 Tailscale/VS Code tunnel 로 원격 수행. 경로는 순서 1 실측에서 겸사 검증.
- 상세:
  - [Studies/Phase 4/SETUP.md](../Studies/Phase%204/SETUP.md) — Phase 4 진입 전 환경 구축 단일 진실 공급원
  - [ENVIRONMENT.md](../ENVIRONMENT.md) — 프로젝트 공용 환경


---


## 원안 week 구조 (자료 맵)


각 week 디렉토리(`Studies/Phase 4/weekN/`)가 담는 내용의 참조 표. **실행 순서의 단일 진실 공급원은 위 "진행 순서 변형" 절, 진행 체크의 단일 진실 공급원은 [`Studies/Phase 4/notes.md`](../Studies/Phase%204/notes.md) 다** — 이 표는 자료가 어디에 있는지 찾을 때만 쓴다.

| week | 구간 | 내용 | 핵심 |
|---|---|---|---|
| 1 | RT-2 정독 | RT-2 논문 1회독 + Architecture Diagram 정독 | PaLI-X / PaLM-E 기반 |
| 2 | RT-2 정독 | Co-fine-tuning + Action tokenization 이해 | "Action 도 토큰" |
| 3 | RT-2 정독 | vla-lab 문서 1편 작성 (RT-2) — **v2 로 이관** | 핵심 아키텍처 + 데이터 + 한계 |
| 4 | OpenVLA 정독 | OpenVLA 논문 1회독 + Architecture | Llama 7B + DINOv2 + SigLIP |
| 5 | OpenVLA 정독 | OpenX-Embodiment 데이터 + Fine-tuning 흐름 + 스키마/action representation 정독 | 970K episodes / observation↔action 페어링 / 관절각·EE-delta·token 비교 |
| 6 | OpenVLA 정독 | OpenVLA HuggingFace 모델 카드 + 환경 셋업 (4-bit 양자화) | `transformers` + GPU |
| 7 | OpenVLA 정독 | vla-lab 문서 1편 작성 (OpenVLA) — **v2 로 이관** | RT-2 와 비교 + open-source 의 의미 + action representation 비교 축 (week5 §3.5) |
| 8 | sim task 루프 | HuggingFace inference 셋업 + 컴퓨트 측정 | 4-bit quantization, VRAM/latency 수치 |
| 9 | sim task 루프 | inference 입력/출력 인터페이스 정리 + 제어 인터페이스 매핑 | image + instruction → action / 이동 플랫폼이면 매핑이 어떻게 달라지나 |
| 10 | sim task 루프 | ROS2 패키지 골격 작성 (Phase 3 week8 스캐폴드 재사용) | `vla_node` 노드 + `vla_action` 토픽 |
| 11 | sim task 루프 | sim embodiment 연결 → zero-shot 추론 → 단일 task 루프 | N회 시도 성공률 기록 |
| 12 | sim task 루프 | Rerun 시각화 (레포 기록용 선택) / 1분 영상 제작은 **v2 로 이관** | 시각화 스크린샷·gif |
| 13 | 패키징·공개 (**v2 이관**) | vla-lab 문서 2편 퇴고 + 다이어그램 작성 | 면접관 진입점 |
| 14 | 패키징·공개 (**v2 이관**) | ROS2 demo README + 환경 세팅 가이드 + 성공률 표 | 재현성 |
| 15 | 패키징·공개 (**v2 이관**) | 1분 데모 영상 마감 + 자막 | 패키징 |
| 16 | 패키징·공개 (**v2 이관**) | 산출물 외부 공개 (Portfolio + vla-lab + LinkedIn) | v2 헤드라인 공개 |

week8-12 (sim task 루프) 는 **Brain ↔ Body 의 첫 통합**이자 v2/v3 의 예고편 — Phase 3 week8 통합 노드를 wrapper 스캐폴드로 재사용한다.

**ROS2 인터페이스 예시** (week8-12):
```
Input  : /camera/image_raw (sensor_msgs/Image), /vla/instruction (std_msgs/String)
Output : /vla/action (팔: EE-delta [dx,dy,dz,rx,ry,rz] -> geometry_msgs/Twist + gripper Float64, 또는 custom msg)
```

> 메시지 선택 상세는 `Studies/Phase 4/week9` §1. 같은 `Twist` 가 **이동 플랫폼에서는 base 선속도/각속도**로 의미가 바뀐다 — 팔->이동 이식 시 재매핑 지점이자 cross-embodiment(부록 D 2026.11 재평가) 의 기술적 단서.

> **공개 글의 논지 3축** (v2 공개 시 사용 — week13-16 자료): (1) 현 세대 FM 을 배포해 단일 task 루프를 닫음, (2) 양자화 배포 + ROS2 통합을 프로덕션 관점으로 처리(AMR 미들웨어 ROS 5년 경력 연결), (3) perception → VLA → 자작 팔 궤적. v1 단계에서는 외부 공개를 하지 않고 레포에 결과만 기록하며, 이 논지 3축은 실제 팔이 결합된 v2 공개에서 v1 의 sim 결과를 토대로 전개한다.


---


## Phase 4 완료 체크리스트


### 논문 reading
- [ ] RT-2 아키텍처 다이어그램을 막힘없이 설명 가능
- [ ] OpenVLA 아키텍처 다이어그램을 막힘없이 설명 가능
- [ ] RT-2 와 OpenVLA 의 차이 (open-source / 모델 크기 / 데이터) 정리
- [ ] action representation (관절각 / EE-delta / token) 을 모델 간 비교하고, 그 출력이 제어 인터페이스 및 다른 플랫폼(이동 로봇) 과 어떻게 연결되는지 설명 가능


### vla-lab 문서 (작성·공개는 v2 로 이관 — 정독만 v1)
- vla-lab 문서 작성/공개는 v1 범위 아님. 위 "논문 reading" 의 정독(아키텍처 설명 가능)까지가 v1.


### v1 ROS2 추론 루프 (카메라/bag dry-run)
- [ ] 컴퓨트 점검 (4-bit OpenVLA 7B 가 RTX 4070 12GB 에서 OOM 없이 동작 + latency 제어 주기 이내)
- [ ] OpenVLA zero-shot inference 동작
- [ ] `vla_node` ROS2 패키지 빌드 + 실행 (Phase 3 week8 스캐폴드 재사용)
- [ ] **카메라/bag 입력으로 1분 dry-run**: action 토픽 연속 발행 + mean/p95 latency·throughput·`0 fail`·GPU<10GB 측정·기록 (sim task 성공률은 Phase 4.5 로 이관)
- [ ] (선택) Rerun 시각화 스크린샷/짧은 gif 1장 — 레포 결과 기록용 (1분 영상 제작은 v2)
- [ ] `RobotPolicy` adapter 인터페이스 분리 (추론 노드가 모델 구현에 직접 결합되지 않게)
- [ ] action schema validation 레이어 (7-DoF 범위·NaN·급변 검증)
- [ ] 벤치마크 재현 스크립트 + 고정 조건 문서 (`Measurements/openvla-rtx4070-int4/` 연동)


### 산출물 v1 레포 기록 (외부 공개는 v2 로 이관)
- [ ] `physical-ai-study` 레포에 결과 기록: README + latency/throughput 표
- 1분 영상·vla-lab 문서 작성·LinkedIn 공유는 v2(Phase 6)에서 수행


---


## Phase 4 완료 기준


> "RT-2 와 OpenVLA 의 아키텍처를 막힘없이 설명할 수 있고, pretrained OpenVLA zero-shot inference 를 ROS2 토픽으로 받아 추론 루프를 닫고(카메라/bag 입력 → action 토픽) 1분 dry-run 으로 latency/throughput/안정성을 측정·기록한다. 그리고 action representation 이 실제 제어 인터페이스 및 다른 플랫폼(이동 로봇) 과 어떻게 연결되는지 설명할 수 있다. 단순 1회 action 출력은 통과가 아니다 — 루프가 닫히고 dry-run 측정이 기록돼야 한다. (sim 단일 task 성공률은 Phase 4.5 범위.)"


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
- **Phase 4.5 (VLA v1.5)** — OpenVLA LoRA adaptation + before/after 정량 분석 (둘째 층 증거). Section 0 (sim 구축·Docker·RunPod 이관) 은 2026.08 전진 확정 (Sections 1-3 의 선행 조건 + RunPod 학습 준비), Sections 1-3 은 2026.09-11 ([Phase 4.5.md](Phase%204.5.md))
- **하드웨어 스파이크** (2026.09, 2주) — SO-101 (Feetech) LeRobot 네이티브 검증 (정의: [Hardware-Arm.md](Hardware-Arm.md))
- **6개월 분기 재평가 #1 (2026.11)** — 스파이크 결과 / v1 결과(레포) / VLA 모델 갱신 검토 / 시장 신호 probe 반응 / cross-embodiment 좌표 점검. **육아휴직(2026.09-2027.02) 중이라 정찰 지원은 하지 않으며, 본격 실지원은 복직(2027.03) 직후 개시한다.**
- **Hardware-Arm Stage 1 본 빌드** (2027.01-02) → v2 선행 하드웨어
- **Phase 6 (Isaac Sim 디지털 트윈) + 자작 팔 결합 → 산출물 v2** (헤드라인, sim-to-real gap)
