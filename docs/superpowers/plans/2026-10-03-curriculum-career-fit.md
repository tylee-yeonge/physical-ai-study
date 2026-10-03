# 커리큘럼-커리어 정합 보정 Implementation Plan

> 작성일: 2026-10-03
> 대상: master roadmap, `README.md`, `Roadmap/Hardware-Arm.md`, `Studies/Hardware-Arm/stage1/README.md`, 신규 `Studies/Hardware-Arm/stage1/task_supervisor.md`, 회사명 규칙 문서 4개. Task 5 (재평가 후): `Roadmap/Phase 5·6·7.md`, `Studies/Hardware-Arm/stage2/README.md`
> 사유: spec 의 결정 중 레포 문서에 반영할 것을 실행한다
> Spec: `2026-10-03-curriculum-career-fit-design.md` (§0.2, §2, §3, §4, §5, §6)

> 체크박스 (`- [ ]`) 로 진행을 추적한다. 각 Task 는 검증 Step 과 커밋 Step 으로 끝난다. 이 plan 은 **문서 반영만** 다룬다 — W9 구현, LLM 없는 블록, 지인 질문, 판단 게이트 실행 같은 커리어 실행 항목은 master roadmap 에서만 체크한다 (spec §0.2 #1).

**Goal:** 2026-10-03 검토의 즉시 결정 (회사명 규칙, 판단 게이트, 시장 신호 수단, LLM 없는 블록, v1.5 소화 ② 부활, W9 task supervisor) 을 실행 보드와 범위 문서에 반영하고, 재평가 #1 로 넘기는 권고 9건을 안건으로 등재한다. 완료되면 master roadmap 만 보고 10월부터 2027.02 게이트까지의 실행 항목을 따라갈 수 있다.

**Architecture:** master roadmap 을 먼저 고치고 (전파 규약), 결정 원본은 spec 에 둔다. 범위 문서 (stage1 README, Hardware-Arm.md, README) 에는 같은 이름 (W9, task supervisor) 으로 반영한다. 재평가 #1 결정이 필요한 문서는 Task 5 로 미룬다.

**Tech Stack:** Markdown (Mermaid), git

**Note on verification:** 문서 작업이라 자동 테스트가 없다. 각 Task 의 검증은 grep 으로 반영 위치를 확인하고, `git diff --stat` 으로 범위 밖 파일이 바뀌지 않았는지 본다.

## 0. 확정된 결정

| # | 항목 | 결정 | 근거 |
|---|---|---|---|
| 1 | 반영 범위 | spec §0.2 의 "즉시" 항목 (#1-#8) 은 지금 반영, "재평가 #1" 항목 (#9-#17) 은 안건 등재만 하고 내용 반영은 Task 5 | spec §0.2 |
| 2 | 작업 브랜치 | `claude/curriculum-career-fit` 에서 Task 단위 커밋 후 PR 로 main 에 합친다 | 기본 브랜치 직접 커밋 금지 |
| 3 | 회사명 | 이 plan 이 고치는 문서에서는 실명을 쓴다 | spec §0.2 #2 |

## 0.1 이 계획이 보장하지 않는 것

- spec §0.3 의 한계가 그대로 적용된다.
- 이 plan 은 W9 를 구현하지 않는다. 구현 파일 (`task_supervisor.py`, `task_waypoints.yaml`, `CMakeLists.txt` install 줄) 은 W9 실행 때 만든다.
- Task 5 의 내용은 재평가 #1 결정에 따라 바뀐다. 지금은 체크박스 수준의 Draft 다.

## Global Constraints

- `Studies/Hardware-Arm/v25/` 와 `Studies/Hardware-Arm/stage1/ros2_pkg/` 는 건드리지 않는다 (spec §6 S1·S2).
- `Roadmap/Phase 5·6·7.md`, `Studies/Hardware-Arm/stage2/README.md`, README 의 타임라인·gantt·부록 B·부록 E·최종 포지셔닝은 재평가 #1 전에는 건드리지 않는다 (Task 5).
- 기존 기록 (완료된 체크박스, 진행 상황 서술) 의 문장은 고치지 않는다. 바뀐 결정은 supersede 주석으로 붙인다 (master roadmap 전파 규약).
- 문체는 기존 문서 관행을 따른다 — 한국어 본문, 이모지 금지, 다이어그램은 Mermaid.
- 커밋: Conventional Commits, 영어.

---

### Task 1: spec·plan 배치, 결정 기록, 회사명 규칙 교체 (spec §0.2 #1-#8, §8.3)

**Files:**
- Create: `docs/superpowers/specs/2026-10-03-curriculum-career-fit-design.md`, `docs/superpowers/plans/2026-10-03-curriculum-career-fit.md`
- Modify: `docs/superpowers/plans/2026-08-31-master-roadmap.md` (§6), `docs/superpowers/specs/2026-07-20-career-review-sync-design.md`, `docs/superpowers/plans/2026-07-20-career-review-sync.md`, `docs/superpowers/specs/2026-08-05-career-positioning-vla-edge-design.md`, `docs/research/2026-08-31-kr-physical-ai-jd-survey.md`

**Interfaces:** 결정 기록의 이름 "2026-10-03 결정 (curriculum-career-fit)" — Task 2-4 의 supersede 주석이 이 이름을 인용한다.

- [x] **Step 1: spec·plan 작성**

spec 은 결정·근거·설계 (게이트 규칙, supervisor 설계, LLM 블록 규칙), plan 은 이 문서.

- [x] **Step 2: master roadmap §6 에 결정 기록**

§6 의 2026-09-01 결정 아래에 같은 형식으로 추가한다: 결정 항목 #1-#8 요약, 재평가로 넘긴 #9-#17 의 안건 번호, 영향 문서 목록.

- [x] **Step 3: 회사명 규칙 supersede 주석**

| 파일 | 위치 | 주석 |
|---|---|---|
| 2026-07-20 spec | 헤더 (Plan 줄 아래), §0.2 #5 행 끝, §8.2 끝 | 실명 금지는 2026-10-03 에 대체 — 비공개 레포는 실명 허용, 공개 채널 이관 시 익명화 |
| 2026-07-20 plan | Global Constraints 의 실명 금지 줄 끝 | 같은 내용 |
| 2026-08-05 spec | 헤더 "표기" 줄 아래 | 같은 내용 |
| 2026-08-31 research | 헤더 "표기 경고" 줄 아래 | 같은 내용 + "공개 채널 이관 금지·익명화 문장은 그대로 유효" |

- [x] **Step 4: 검증 — 금지 규칙 문서마다 supersede 주석**

```bash
grep -rlE "실명 금지|실명 vs 익명|회사 실명\(" docs | grep -v 2026-10-03 | sort
grep -rl "Supersede (2026-10-03, 회사명 표기)" docs | grep -v 2026-10-03 | sort
```

기대: 두 목록이 같은 파일 4개 (금지 규칙을 서술한 파일마다 supersede 주석이 있다). 이 spec·plan 은 규칙 변경 자체를 서술하므로 두 목록에서 뺀다.

진행 상황 (2026-10-03): 두 목록 일치 확인 (research 1, 2026-07-20 plan·spec 2, 2026-08-05 spec 1). 새 링크 6개의 대상 파일 존재 확인.

- [x] **Step 5: Commit**

```bash
git add docs/superpowers/specs/2026-10-03-curriculum-career-fit-design.md \
        docs/superpowers/plans/2026-10-03-curriculum-career-fit.md \
        docs/superpowers/plans/2026-08-31-master-roadmap.md \
        docs/superpowers/specs/2026-07-20-career-review-sync-design.md \
        docs/superpowers/plans/2026-07-20-career-review-sync.md \
        docs/superpowers/specs/2026-08-05-career-positioning-vla-edge-design.md \
        docs/research/2026-08-31-kr-physical-ai-jd-survey.md
git commit -m "docs: add curriculum career fit spec and plan, allow company names in private docs"
```

---

### Task 2: 운영 변경 반영 (spec §0.2 #4-#7, #12, §2, §4)

**Files:**
- Modify: `docs/superpowers/plans/2026-08-31-master-roadmap.md` (§2, §3, §4, 상시), `README.md` (부록 D)

**Interfaces:** master roadmap §3 의 "2027.02 초 — 판단 게이트" 절. README 부록 D 가 이 절과 spec §2 를 가리킨다.

- [x] **Step 1: §3 10-11월 병행 줄 재구성**

한 줄에 두 체크박스가 붙어 있던 "병행 (≤2)" 을 하위 목록으로 나누고:
- JD 격차 매핑 항목에 추가: 회사별 코딩테스트 언어 열 (RLWRLD 는 Python 고정 2h), RLWRLD 2공고의 원문 매핑은 spec §1.1 이 1차분, 로보티즈 모방학습·네비게이션 원문 대조
- probe 택1 을 "시장 신호 — 아는 사람 2-3명에게 구체 질문 또는 AI 사피엔스 기여" 로 교체 (질문 예 3개, 기록 위치 `.private/`, 게이트 입력 I2)

- [x] **Step 2: v1.5 소화 ② 부활**

§3 ~09.07 의 취소선 줄 끝에 "→ 10월 부활 (2026-10-03)" 주석, §3 10-11월 절에 새 체크박스 (v2.5 착수 전, 30분), §4 스킵 대장 표 아래에 예외 기록.

- [x] **Step 3: 코테 supersede**

§2 일정표 12-2027.02 행과 §3 12월 코테 항목에 주석: 언어는 JD 격차 매핑의 언어 열로 재평가 #1 에서 다시 정한다, C++ 면접 방어는 별도 유지.

- [x] **Step 4: 판단 게이트 등재**

- §2 일정표 12-2027.02 행의 게이트 열에 "판단 게이트 2027.02 초 (패키징 착수 전)"
- §3 에 "2027.02 초 — 판단 게이트" 절: 입력 I1-I5 체크박스 + 결정 규칙은 spec §2.2 링크 + 결과 기록 체크박스
- README 부록 D 표에 "2027.02 초" 행, "시그널 → 행동 매핑" 에 게이트 줄

- [x] **Step 5: 상시 절**

LLM 없는 블록 규칙 요약 (spec §4 링크) 과 월 실적 집계 (9월 표 형식 + "LLM 없는 블록 h" 열, 10월분부터) 추가.

- [x] **Step 6: 검증 — 반영 위치**

```bash
grep -n "판단 게이트" docs/superpowers/plans/2026-08-31-master-roadmap.md README.md
grep -n "LLM 없는 블록" docs/superpowers/plans/2026-08-31-master-roadmap.md
grep -n "v1.5 소화 ②" docs/superpowers/plans/2026-08-31-master-roadmap.md
grep -n "코딩테스트 언어\|코테 언어" docs/superpowers/plans/2026-08-31-master-roadmap.md
grep -n "아는 사람 2-3명" docs/superpowers/plans/2026-08-31-master-roadmap.md
git diff --stat
```

기대: 각 grep 이 1줄 이상, diff 는 master roadmap 과 README 두 파일만.

진행 상황 (2026-10-03): 기대대로 확인 — master roadmap 에서 게이트 6 · LLM 블록 4 · v1.5 소화 ② 3 · 코테 언어 4 · 지인 질문 3 줄, README 게이트 2 줄. diff 는 두 파일 (+26 / -5). 범위 밖 수정 1건: §2 일정표 10-11월 행의 "probe 택1" 문구도 같은 결정으로 교체.

- [x] **Step 7: Commit**

```bash
git add docs/superpowers/plans/2026-08-31-master-roadmap.md README.md docs/superpowers/plans/2026-10-03-curriculum-career-fit.md
git commit -m "docs: add feb 2027 decision gate and llm-free study block to master roadmap"
```

---

### Task 3: task supervisor 를 Stage 1 범위에 추가 (spec §0.2 #3, §3)

**Files:**
- Create: `Studies/Hardware-Arm/stage1/task_supervisor.md`
- Modify: `Studies/Hardware-Arm/stage1/README.md`, `docs/superpowers/plans/2026-08-31-master-roadmap.md` (§1, §2, §3 Stage 1), `Roadmap/Hardware-Arm.md` (Stage 1 절), `README.md` (Stage 1 목표)

**Interfaces:** 이름 "W9 — task supervisor (태스크 실행·실패 복구)". 구현 파일 경로 `scripts/task_supervisor.py`, `config/task_waypoints.yaml` (W9 실행 시 생성).

- [x] **Step 1: `task_supervisor.md` 작성**

W9 구현 가이드: 여는 곳, 만들 파일 3개, 구현 순서, mock·실기 시험 명령, 확정값 기록 표, 막히면, LLM 없는 블록 연계. 설계 원본 (상태·인터페이스·초기값) 은 spec §3 을 가리킨다.

- [x] **Step 2: stage1 README**

일정 블록, must 표 행, 학습 파일 표, 진행 순서 제목 (W1-W9), Mermaid 에 W9 (W5 뒤), "왜 이 순서인가" 에 W9 줄, W9 절 (W8 절 뒤), 완료 체크리스트.

- [x] **Step 3: master roadmap**

§2 10-11월 행에 W9, §3 Stage 1 절에 W9 체크박스 (W8 줄 뒤). 기존 W3 줄 끝에 W4 체크박스가 붙어 렌더링되지 않던 줄바꿈 누락도 함께 고친다.

- [x] **Step 4: Hardware-Arm.md · README Stage 1**

`Roadmap/Hardware-Arm.md` Stage 1 의 목표 bullet, 단계 표 행, 완료 체크리스트 must 항목, "W1-W8" → "W1-W9". `README.md` Stage 1 목표 문구에 W9.

- [x] **Step 5: 검증 — 네 곳의 이름 일치와 범위 보존**

```bash
grep -ln "task supervisor" Studies/Hardware-Arm/stage1/README.md Roadmap/Hardware-Arm.md README.md docs/superpowers/plans/2026-08-31-master-roadmap.md
git diff --stat -- Studies/Hardware-Arm/v25 Studies/Hardware-Arm/stage1/ros2_pkg
```

기대: 첫 명령이 네 파일을 모두 출력, 둘째 명령은 출력 없음.

진행 상황 (2026-10-03): 기대대로 확인 — 네 파일 모두 출력, `v25/` · `stage1/ros2_pkg/` 변경 없음. 가이드를 쓰면서 spec §3 을 보강했다: 인터페이스에 `/robot_description` 구독 (한계값을 URDF 에서 한 번 읽는다 — 하드웨어 읽기 아님), mock 시험에 한계 근접 시나리오, W9 구현 파일 목록에 `package.xml` 의 `python3-yaml` 한 줄. stage1 README 에는 "v2.5 와 겹치는 구간" 에 W9 순서 문장을 함께 넣었다.

- [x] **Step 6: Commit**

```bash
git add Studies/Hardware-Arm/stage1/task_supervisor.md Studies/Hardware-Arm/stage1/README.md \
        docs/superpowers/plans/2026-08-31-master-roadmap.md Roadmap/Hardware-Arm.md README.md \
        docs/superpowers/plans/2026-10-03-curriculum-career-fit.md
git commit -m "docs: add task supervisor fsm to stage1 scope as w9"
```

---

### Task 4: 재평가 #1 안건 등재 (spec §0.2 #9-#17)

**Files:**
- Modify: `docs/superpowers/plans/2026-08-31-master-roadmap.md` (§3 재평가 절, §5), `README.md` (부록 D 2026.11 행)

- [ ] **Step 1: master roadmap §5**

기존 안건 1 (Phase 5) · 3 (부록 B) · 6 (C++·DDS) · 7 (진열 순서) 끝에 권고 부기. 신규 안건 9 (Stage 2 C++ 인터록) · 10 (부록 E) · 11 (해제 시간 용도) · 12 (지원 회사 필터) · 13 (문서 상한).

- [ ] **Step 2: master roadmap §3 재평가 체크 줄**

"§5 안건 1-8" → "§5 안건 1-13".

- [ ] **Step 3: README 부록 D 2026.11 행**

끝에 "curriculum-career-fit 안건" 요약과 spec 경로.

- [ ] **Step 4: 검증**

```bash
grep -n "^9\. \|^10\. \|^11\. \|^12\. \|^13\. " docs/superpowers/plans/2026-08-31-master-roadmap.md
grep -n "안건 1-13" docs/superpowers/plans/2026-08-31-master-roadmap.md
grep -c "curriculum-career-fit" README.md
```

기대: 신규 안건 5줄, 체크 줄 1줄, README 의 curriculum-career-fit 언급 1회 이상.

- [ ] **Step 5: Commit**

```bash
git add docs/superpowers/plans/2026-08-31-master-roadmap.md README.md docs/superpowers/plans/2026-10-03-curriculum-career-fit.md
git commit -m "docs: register curriculum fit agenda items for review 1"
```

---

### Task 5: 재평가 #1 결정 전파 (Draft — 2026.11 하순 재평가 후 세부화)

**Files:**
- Modify (채택 시): `Roadmap/Phase 5.md`, `Roadmap/Phase 6.md`, `Roadmap/Phase 7.md`, `Studies/Hardware-Arm/stage2/README.md`, `Roadmap/Hardware-Arm.md` (Stage 2), `README.md` (타임라인 표·gantt·부록 B·부록 E·최종 포지셔닝·커리어 경로·마일스톤)

- [ ] 재평가 #1 결정 결과를 master roadmap 에 먼저 기록 (전파 규약)
- [ ] Phase 5 재작성 (안건 1 채택 시). `Studies/Phase 5/` 사전 작성분은 안건 2 결정에 따른다
- [ ] Phase 6·7 지위 표기 (안건 3 채택 시)
- [ ] Stage 2 역할 재정의 (안건 9 채택 시)
- [ ] README 정렬 (안건 3·7·10 채택 분)
- [ ] 검증 — spec §6 D5
- [ ] Commit: `docs: propagate review 1 decisions to roadmap docs`

---

## Verification (after all tasks)

- [ ] spec §6 D1 — 게이트가 master roadmap §2·§3 과 README 부록 D 에 있고 spec §2 를 가리킨다
- [ ] spec §6 D2 — 회사명 규칙 문서 4개의 supersede 주석
- [ ] spec §6 D3 — §5 신규 안건 9-13, 기존 1·3·6·7 부기, 체크 줄 1-13
- [ ] spec §6 D4 — W9 가 네 곳에 같은 이름
- [ ] spec §6 D5 — Task 5 후
- [ ] spec §6 T1 — 이 plan 에 커리어 실행 체크박스 없음
- [ ] spec §6 S1·S2 — `v25/`, `stage1/ros2_pkg/` 변경 없음
- [ ] PR 생성 — `claude/curriculum-career-fit` → main

## Self-Review

**Spec coverage:** §0.2 #1·#2 → Task 1 / #3 → Task 3 / #4·#5·#6·#7·#12 (언어 확인 항목) → Task 2 / #8 → Task 1 Step 2 (결정 기록) / #9-#17 → Task 4 (등재), Task 5 (전파) / §2 → Task 2 Step 4 / §3 → Task 3 / §4 → Task 2 Step 5 / §6 → Verification.
**Placeholder scan:** Task 5 는 재평가 결정 전이라 의도적으로 Draft 다. 그 밖에 TBD 없음.
**Type consistency:** "W9", "task supervisor", "판단 게이트", "LLM 없는 블록", "2026-10-03 결정 (curriculum-career-fit)" 를 모든 문서에서 같은 표기로 쓴다.
