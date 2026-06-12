# Phase 4 진행 보드 + 작업 노트

> Phase 4 는 이 파일만 보고 진행한다 — 실행 체크리스트(진행 보드)와 순서별 노트 산출물을 한 파일에서 관리하며, 체크 표기는 여기서만 갱신한다.
> 계획의 배경·시간 예산·산출물 정의는 [`Roadmap/Phase 4.md`](../../Roadmap/Phase%204.md), 환경 구축·컴퓨트 수치는 [`SETUP.md`](SETUP.md) (수치 본체는 §1.3).
> 노트(skim 답 등)는 항목당 몇 줄이면 충분하다 — 정독 노트가 아니다. 모든 경로는 `Studies/Phase 4/` 기준.

## 진행 순서 한눈에

이 표의 행 순서가 곧 진행 순서다. 주차 번호는 재부여하지 않는다 — week 디렉토리와 자료 구조는 원안 그대로다 (자료 맵은 `Roadmap/Phase 4.md`의 "원안 week 구조" 절).

| 순서 | 진행할 week | 작업 | 시기 | 계획 투입 | 상태 |
|---|---|---|---|---|---|
| 1 | `SETUP.md` (week 아님) | Step 0: 환경 구축 + 레포 청소 + 4-bit 로드 + latency/VRAM 실측 | 6월 2주차 | 6h | 완료 |
| 2 | `week4`-`week5` 일부만 | 표적 skim: action 표현 / unnorm_key / 입력 형식 / embodiment 가정 (정독 아님 — 정독은 순서 6) | 6월 2-3주차 | 4h | 진행 중 |
| 3 | `week11` 일부 선행 | sim 환경 선정 + embodiment 정합 + 성공 task 1종 정의 (→ latency 판정 수치 확정) | 6월 2-3주차 | 12h | 대기 |
| 4 | `week8` → `week12` 순서대로 | 실습 압축: VLAInference → ROS2 노드 → sim 단일 task 루프 | 6월 3주 - 7월 중순 | 34h | 대기 |
| 5 | `week11`-`week12` 마무리 | 성공률 N회 측정 + 결과 정리 | 7월 중순 - 말 | 22h | 대기 |
| 6 | `week1` → `week7` 순서대로 | RT-2/OpenVLA 정독 + 블로그 2편 | 8-9월 | 50-70h 잠정 (순서 7 포함, 8월 초 체크포인트에서 갱신) | 대기 |
| 7 | `week13` → `week16` 순서대로 | 블로그 마무리 + 패키징 + 영상 + v1 공개 | 8-9월 | (순서 6 에 합산) | 대기 |

## 진행 원칙

- 순서 1-5 (6-7월) 합계 78h = 계획선. 최대 가용(96h)과의 차이 약 18h 는 버퍼 — 막히는 구간(실습/측정)에 흡수한다.
- 계획선 미달 시 절삭 순서: (1) 버퍼 소진 → (2) 성공률 측정 N 축소 → (3) 측정 항목 축소 (latency 분포는 유지, 부가 지표 절삭). v1 산출물 정의(루트 README 부록 B)를 건드리는 절삭은 별도 의사결정 — 발동 시 부록 B 갱신 + 2026.11 분기 재평가 #1 안건 등재가 전제.
- 작업 성격과 시간 블록 매칭: 평일 오전 보장 블록(09-12, 연속 3h) = 환경 구축/디버깅/실습 전용. 출장일 저녁 1h·오후 잔여 시간 = 논문 섹션 읽기/노트/quiz 등 파편화 가능 작업 전용. 오후 학습은 계획에 넣지 않고 버퍼·초과분 흡수처로만 쓴다.
- 오전 블록 사수율 검증: 첫 2주간 사수율을 실측하고, 깨지는 비율이 높으면 보장 블록을 오후로 반전한다.
- 8월 초 체크포인트: v1 기술 코어(순서 5) 확보 직후 8-12월 예산·배치를 확정한다 (순서 5 의 마지막 체크 항목). 입력: 오전 블록 사수율 실측(6-7월), 출장 빈도(월 5회 이하 예상), v1 진행 결과. 1순위 검토 안건: Phase 4.5(v1.5) 의 9-10월 조기 진입.
- RT-2 정독 후행: RT-2 는 비공개 모델로 순수 개념·블로그용이므로 정독 전량을 8-9월로 이동. 6월의 표적 skim 은 구현에 필수인 부분집합(action 표현 / unnorm_key / 입력 형식 / embodiment 가정)만 다룬다.

## 순서 1 — Step 0: 환경 구축 + 레포 청소 + 실측 (6월 2주차, 6h) — 완료

- [x] `SETUP.md` §2 사전 점검 체크리스트 통과 (계정 / 로컬 드라이버·디스크·ROS2 / 공통 도구)
- [x] `SETUP.md` §6 로컬 환경 세팅 — 공용 venv `.venv-vla` 생성은 `week8/PRACTICE.md` "환경 설정" 절의 명령 사용
- [x] `SETUP.md` §1.3 실측치 표 숙지 — 이번 실측이 검증할 추정치 (int4 약 7GB / 약 2-3 Hz)
- [x] git 커밋 신원 정리 — 작업 컨테이너에 `git config user.name` / `user.email` 을 GitHub 계정에 연결된 개인 이메일(또는 noreply)로 설정. 기존 커밋 히스토리의 author 재작성(`git filter-repo`) 여부는 v1 공개 전 결정 — 공개·fork 이후에는 불가
- [x] `predict_action` 호출 패턴 일괄 청소 (**순서 4 진입 전 필수**) — `grep -rn "predict_action(\*\*inputs" .` 으로 잔존 위치 확인 후 `input_ids` / `pixel_values` 만 전달하도록 통일. 근거: `predict_action` 은 빈 토큰(29871)을 input_ids 에만 덧붙이므로 processor 출력의 attention_mask 를 그대로 넘기면 eager attention 에서 off-by-one 으로 깨진다
- [x] `week6/PRACTICE.md` 실습 1 (첫 OpenVLA 4-bit inference) — OOM 없이 로드되는지
- [x] `week6/PRACTICE.md` 실습 2 (latency 측정 100회 + 통계 + 결과 저장) — mean/p95 와 `nvidia-smi` VRAM 기록
- [x] 실측치를 `SETUP.md` §1.3 추정치와 비교 — 추정 범위(±50%)를 벗어나면 §1.3 을 실측 기준으로 갱신 (실측 mean 300.3 ms / 3.33 Hz, ±50% 범위 내 — §1.3 에 실측 행 추가)

### 노트: 순서 1 핵심 결과

- int4 OpenVLA 7B 가 RTX 4070 12GB 에 OOM 없이 로드.
- latency 실측 (n=100): mean 300.3 ms / median 301.3 ms / std 3.8 ms / p95 304.8 ms → **3.33 Hz**. 외삽 추정(약 2-3 Hz)의 ±50% 범위 내. 단, `predict_action` 호출만 측정 — 이미지 전처리/ROS2 오버헤드 제외 (제어 루프 전체 주기는 순서 4 의 week11 dry-run 에서 확인).
- 결과 파일: `week6/openvla_latency_4070_int4.npy` (Phase 7 산출물 v3 의 비교 baseline).
- 수치 본체: `SETUP.md` §1.3.

## 순서 2 — 표적 skim (6월 2-3주차, 4h)

skim 목적은 구현에 필요한 사실 확인이지 정독이 아니다. 항목별로 답을 아래 노트 섹션에 적으면 끝.

- [x] 라이선스 확인: OpenVLA HF 모델 카드 — 코드 MIT / weights 는 Llama 2 license. v1 공개물(블로그·영상·레포)에서의 사용 조건 확인 (노트 작성 완료)
- [ ] action 표현: `week5/README.md` §3 (Action space 표준화) + §3.5 (관절각/EE-delta/token 비교 축) — OpenVLA 출력 7-DoF 가 무엇을 의미하는지
- [ ] unnorm_key / 입력 형식: OpenVLA HF 모델 카드의 사용 예시 코드 + `week6/README.md` "핵심 개념"의 모델 로드/추론 코드 절 — prompt 형식과 `predict_action(input_ids=..., pixel_values=..., unnorm_key=..., do_sample=False)` 호출. **주의**: processor 출력 전체를 `**inputs` 로 넘기는 패턴은 attention_mask 동봉으로 크래시한다 (순서 1 청소 항목의 근거 참조). 참고 자료에 옛 패턴이 남아 있으면 그 자리에서 수정
- [ ] embodiment 가정: `week5/README.md` §1 (OpenX-Embodiment 구조) + §2 (대표 embodiment 특징) + 공식 repo README — 어떤 로봇/카메라 시점을 전제로 학습됐는지, sim 이 거기에 맞을 수 있는지
- [ ] 아키텍처 최소 골격: `week4/README.md` §5 (Architecture Diagram) + "한 페이지 OpenVLA 요약"의 입출력 인터페이스 절

### 노트: 라이선스

- 코드(공식 repo, training codebase 포함): MIT. 복사·개작해 내 레포에 넣으면 MIT 저작권 고지 유지 필요.
- weights(openvla-7b, openvla-v01-7b): Llama-2 파생이므로 Llama 2 Community License 적용. HF 모델 카드의 license 태그는 MIT 로 되어 있으나 GitHub README 가 Llama 2 적용을 명시 — 보수적으로 후자를 따른다.
- v1 공개물 영향: 블로그·영상·레포 공개 모두 의무 없음. weights 를 레포에 포함하지 않고 HF 에서 로드만 하면 재배포에 해당하지 않는다. 출력물(추론 결과, latency 수치, sim 데모) 공개 제약 없음 — Llama 2 의 출력물 제한은 "다른 LLM 개선에 사용 금지"뿐.
- 경계: int4 양자화본을 파일로 저장해 레포나 HF 에 올리면 weights 파생물 재배포 — 라이선스 사본 동봉 + "Llama 2 is licensed under the LLAMA 2 Community License, Copyright (c) Meta Platforms, Inc." 고지 의무 발생. 현재 계획(로컬 로드·양자화, 결과만 공개)에서는 해당 없음.

### 노트: action 표현

- (작성 전) OpenVLA 출력 7-DoF 가 무엇을 의미하는지 — `week5/README.md` §3, §3.5

### 노트: unnorm_key / 입력 형식

- (작성 전) prompt 형식과 `predict_action` 호출 — OpenVLA HF 모델 카드 사용 예시 + `week6/README.md` "핵심 개념"

### 노트: embodiment 가정

- (작성 전) 어떤 로봇/카메라 시점을 전제로 학습됐는지, sim 이 거기에 맞을 수 있는지 — `week5/README.md` §1, §2 + 공식 repo README

### 노트: 아키텍처 최소 골격

- (작성 전) 입출력 인터페이스 — `week4/README.md` §5 + "한 페이지 OpenVLA 요약"

## 순서 3 — sim 정합 + 성공 task 정의 (6월 2-3주차, 12h)

이 구간은 대응하는 week 자료가 없는 신규 작업이다 (원안에서 week11 에 묻혀 있던 선행 의사결정을 분리한 것).

- [ ] 순서 2 의 embodiment 가정에 맞는 sim 후보 비교·선정 — 선정 사유를 아래 노트에 기록
- [ ] 성공 task 1종 + 성공률 기준 N 정의
- [ ] task 의 제어 주기 요구 확정 → `week8/PRACTICE.md` 실습 체크리스트의 latency placeholder ("2 Hz 이상") 를 확정 수치로 교체
- [ ] `week11/README.md` §4 (1분 dry-run 의 success criteria) 미리 읽기 — 순서 4 의 종착점 파악

### 노트: sim 후보 비교·선정 사유

- (작성 전)

### 노트: 성공 task 정의 + 성공률 기준 N

- (작성 전)

## 순서 4 — week8-12 실습 압축 (6월 3주 - 7월 중순, 34h)

각 week 공통 패턴: `README.md` 정독 → `PRACTICE.md` 실습 → `quiz_easy.py` / `quiz_medium.py` (출장일 저녁 등 파편 시간에 배치 가능). 진입 전제: 순서 1 의 `predict_action` 패턴 청소 완료.

- [ ] `week8/README.md` + `week8/PRACTICE.md` 실습 1-4 (VLAInference class / image preprocess / exceptions·config / 100회 stress test) + quiz
- [ ] `week9/README.md` + `week9/PRACTICE.md` 실습 1-3 (I/O spec 1페이지 / msg <-> Python 변환 / BGR->RGB 검증) + quiz
- [ ] `week10/README.md` + `week10/PRACTICE.md` 실습 1-4 (vla_node 패키지 생성 / 골격 노드 / setup.py / 빌드+실행, 실습 5 dummy image 는 선택) + quiz
- [ ] `week11/README.md` + `week11/PRACTICE.md` 실습 1-3 (실 inference 통합 / 빌드+실행 / 1분 dry-run + 통계) — 입력은 순서 3 에서 선정한 sim 으로 연결 (자료의 ros2 bag 재생은 대체 수단) + quiz
- [ ] `week12/README.md` + `week12/PRACTICE.md` 실습 1-2 (Rerun 기본 / rerun_logger 노드) — 영상 제작(실습 3-5)은 순서 7 로 후행 + quiz

## 순서 5 — 성공률 측정 + 결과 정리 (7월 중순 - 말, 22h)

- [ ] 정의된 task 를 N회 시도, 성공률 기록 — 측정/통계 코드는 `week11/PRACTICE.md` 실습 3 패턴 재사용
- [ ] mean/p95 latency 실측 + task 제어 주기 충족 여부 판정 (`week8/PRACTICE.md` 실습 체크리스트의 확정 기준)
- [ ] 결과 표 정리 — `week12/PRACTICE.md` 실습 5 의 "측정 결과" 표 형식 참고. 여기까지가 v1 기술 코어 (7월 말 목표)
- [ ] **8월 초 체크포인트 수행** — "진행 원칙"의 입력으로 8-12월 예산·배치 확정 (순서 6-7 잠정치 갱신, v1.5 조기 진입 여부 결정)

## 순서 6 — week1-7 정독 + 블로그 2편 (8-9월)

- [ ] `week1/` RT-2 1회독 + reading note (PRACTICE 실습 1-3) + quiz
- [ ] `week2/` Co-fine-tuning + action tokenization (PRACTICE 실습 1-3) + quiz
- [ ] 블로그 플랫폼 선정 (Velog / Medium / 본 레포의 `Studies/Phase 4/blog/`) — week3 발행 전 결정
- [ ] `week3/` RT-2 블로그 1편 작성 + 발행 (PRACTICE 실습 1-4)
- [ ] `week4/` OpenVLA 정독 — 순서 2 에서 안 본 부분 중심 (hybrid vision encoder, contribution, 한계) + 실습 1-3 + quiz
- [ ] `week5/` OpenX-Embodiment + LoRA 흐름 정독 + 실습 1-4 + quiz
- [ ] `week6/` README 개념 보충만 — 실습 1-2 는 순서 1 에서 이미 수행, 실습 3 (에러 기록) 은 미기록분 보완
- [ ] `week7/` OpenVLA 블로그 1편 작성 + 발행 — 실측 결과 섹션에 순서 1/5 의 수치 사용 (PRACTICE 실습 1-4)

## 순서 7 — 패키징 + v1 공개 (8-9월)

- [ ] `week12/PRACTICE.md` 실습 3-5 (1분 영상 시나리오 / 녹화+편집 / Portfolio 패키징) — 순서 4 에서 미룬 분량
- [ ] `week13/` 블로그 2편 퇴고 + 다이어그램 통일 (PRACTICE 실습 1-4)
- [ ] `week14/` ROS2 demo README + 환경 세팅 가이드 + 검증 (PRACTICE 실습 1)
- [ ] `week15/` 영상 자막/thumbnail/최종 export (PRACTICE 실습 1-5)
- [ ] `week16/` 산출물 v1 공개 + 회고 + Phase 4.5 진입 준비 (PRACTICE 실습 1-5)
