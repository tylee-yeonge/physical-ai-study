# 커리어 검토 결과 레포 동기화 Implementation Plan

> 작성일: 2026-07-20
> 대상: `README.md`, `Roadmap/Hardware-Arm.md`, `Roadmap/Phase 4.md`, `Studies/Phase 4/notes.md`, `.private/notes.md`(gitignored) 수정 + 본인 실행 항목 2건
> 사유: 2026-07-20 커리어·레포 검토 결론을 레포에 동기화 (spec §0.1)
> Spec: `2026-07-20-career-review-sync-design.md` (§2 검토 결론, §3 반영 설계, §6 검증 기준)

> 체크박스(`- [ ]`)로 진행을 추적한다. 각 Task 는 검증 Step 과 커밋 Step 으로 끝난다.

**Goal:** 검토에서 확정된 결정 8건(spec §0.2)을 레포 문서에 반영한다. 완료되면 (1) 2026.11 재평가 안건에 동역학 갭·실지원 시점이 정식 등재되고, (2) 스파이크 실행 시기·부품 구성 결정이 8월 초 체크포인트 안건으로 귀속되고, (3) probe JD 목록이 사거리 재평가를 반영하고, (4) notes.md 보드가 실측 상태와 일치한다.

**Architecture:** 문서 동기화만 수행한다 — 안건의 결정은 각 결정 기구(8월 초 체크포인트, 2026.11 재평가)로 위임 (spec §0.3). 회사는 익명 표기만 사용 (spec §8.2).

**Tech Stack:** 마크다운 편집 + grep 검증.

**Note on verification:** 자동 테스트로 닫히지 않는다. 완료 조건은 spec §6 의 V1-V6 — grep 기반 확인(V1, V3, V4, V5), 파일 부재 확인(V2), 본인 확인 항목(V6)은 체크박스로 추적.

## 0. 확정된 결정

| # | 항목 | 결정 | 근거 |
|---|---|---|---|
| 1 | 스파이크 결정 귀속 | 8월 초 정식 체크포인트 (2026.11 아님) | spec §8.1 |
| 2 | 동역학 갭 | 2026.11 안건 등재, 정정 문안 + 예산 트레이드오프 조건 | spec §8.4 |
| 3 | 실지원 시점 | 2026.11 안건 등재, 즉흥 변경 금지 유지 | spec §2.3 |
| 4 | 사거리 반영 | 제조사 본사 시스템 SW 를 1순위 승격, AMR 자회사 제외 | spec §2.1 |
| 5 | 표기 | 회사 실명 금지, 기존 익명 관행 | spec §8.2 |

## 0.1 이 계획이 보장하지 않는 것

- 스파이크 실행 시기·부품 구성·동역학 편입·실지원 시점의 결정 — 각 결정 기구에서만 판단한다 (spec §0.3).
- 시장 조사 사실의 재검증 — 사후지급금 폐지는 2026.11 판단 전 법령 원문 재확인이 안건 문안에 포함된다.
- dry-run 수치 자체의 확보 — Task 5 는 기록 위치 확인이 1차이고, 재측정은 미기록일 때만 8월 내 수행.

## Global Constraints

- 회사 실명(제조사·스타트업·대기업 실명) 금지 — "Dynamixel 제조사" 등 기존 익명 표기만 사용.
- 외과적 변경 — 아래 명시된 문자열만 수정하고 인접 내용을 다듬지 않는다.
- 라인 번호가 아니라 인용된 문자열 기준으로 편집 위치를 찾는다 (실행 시점에 라인이 밀렸을 수 있음).
- 커밋은 task 단위, Conventional Commits 영어.

---

### Task 1: README 부록 D — 2026.11 안건 2건 추가 (spec §3.1)

**Files:**
- Modify: `README.md` (부록 D 표의 2026.11 행)

- [ ] **Step 1: 안건 2건을 행 말미에 추가**

부록 D 2026.11 행에서 다음 문자열을 찾는다:

```
plan: `docs/superpowers/plans/2026-07-07-repo-review-remediation.md`)
```

그 직후(셀 닫힘 `|` 앞)에 다음을 이어 붙인다:

```
 / **동역학 라잇 트랙 편입 여부** (타겟 제조사 AI 트랙 JD 의 강체 동역학·수치 최적화 요구 대응. 갭은 매니퓰레이터 기구학(FK/IK)·강체 동역학 — 수치 최적화 기초는 Phase 1 week7-8 자산 재활용. 범위 상한: 순기구학·역기구학·자코비안·기초 동역학, must 만으로 면접 방어 수준. 편입 시 주 4-5시간 필요 — 총 예산(주 6-8시간) 안에서 대체 대상(무엇을 빼거나 늦출지)을 함께 결정. spec: `docs/superpowers/specs/2026-07-20-career-review-sync-design.md`) / **본격 실지원 개시 시점 재확인** (사후지급금 폐지(2025.01 — 법령 원문 재확인 필요)로 휴직 중 지원의 경제 페널티 소멸, 채용 프로세스 1-3개월 감안 시 2027.01 지원 = 복직(2027.03) 접속 가능. 판단 입력: 타겟 공고 개폐 / v1·v1.5 완성도 / 스파이크 결과 / 고용주 관계·평판 리스크. 즉흥 변경 금지 — 본 재평가에서만 판단)
```

- [ ] **Step 2: 검증 — spec §6 V3**

```
grep -c "동역학 라잇 트랙 편입 여부\|본격 실지원 개시 시점 재확인" README.md   # 기대: 부록 D 행에서 각 1회
grep "동역학 라잇 트랙" README.md | grep -c "Phase 1\|대체 대상"                # 기대: 1 (두 조건 모두 문안에 포함)
```

- [ ] **Step 3: Commit**

```
git add README.md
git commit -m "docs: add dynamics track and application timing agenda to 2026.11 reassessment"
```

---

### Task 2: README probe 절·마일스톤 — 사거리 재평가 반영 (spec §3.2)

**Files:**
- Modify: `README.md` (probe 절 bullet 2곳 + 마일스톤 JD 정독 항목)

- [ ] **Step 1: 마일스톤 JD 정독 항목 교체**

다음 항목을:

```
- [ ] (2026.07-08) 타겟사 **실제 JD 5-10개 정독** — 1순위 (VLA 모델 직접 개발 코스닥 상장사 / 대기업 SW 자회사 VLA / 신생 휴머노이드 스타트업) + 2순위 (자율주행 SW 자회사 CV/ML / ADAS 양산 중견 / Dynamixel 제조사 / 매니퓰레이션 FM) → 요구 역량 vs 현재 격차 1페이지 매핑
```

다음으로 교체한다:

```
- [ ] (2026.07-08) 타겟사 **실제 JD 5-10개 정독** — 1순위 (Dynamixel 제조사 본사 휴머노이드 시스템 SW 라인 — 2026-07 사거리 재평가로 승격 / VLA 모델 직접 개발 코스닥 상장사 / 대기업 SW 자회사 VLA / 신생 휴머노이드 스타트업 / 매니퓰레이션 FM — 정조준 정합으로 2순위에서 이동 / 양팔 데이터 인프라 스타트업 — 검토 후보, v2.5 정합) + 2순위 (자율주행 SW 자회사 CV/ML / ADAS 양산 중견) → 요구 역량 vs 현재 격차 1페이지 매핑 (제조사 본사 2개 공고(시스템 SW·모방학습) 자격요건 vs v1/v1.5/v2.5 일대일 매핑 우선). 동일 그룹 AMR 물적분할 자회사는 수평이동이라 제외
```

- [ ] **Step 2: probe 1단 bullet 에 우선 배치·제외 원칙·인재풀 부기**

probe 절의 "- **2026.07-08 (재직 중, 저가시성)**:" bullet 말미("이직 신호로 읽히지 않는다)" 뒤)에 다음 문장을 추가한다:

```
 JD 목록에는 Dynamixel 제조사 본사 2개 공고(시스템 SW·모방학습)를 우선 배치하고, 동일 그룹의 AMR 물적분할 자회사는 수평이동이라 제외한다. 인재풀 등록·공고 알림 설정도 저가시성이라 이 단계에서 수행.
```

- [ ] **Step 3: 커피챗 bullet 에 우선순위 부기**

```
- **2026.09 이후 (저강도)**: 현직자 **1-2명 커피챗/메시지** (정보성, 합격 목적 아님)
```

을 다음으로 교체한다:

```
- **2026.09 이후 (저강도)**: 현직자 **1-2명 커피챗/메시지** (정보성, 합격 목적 아님. Dynamixel 제조사 재직자 우선 — 질문 예: 면접 평가 축, 시스템 SW 트랙과 AI 트랙의 조직 관계)
```

- [ ] **Step 4: 검증 — spec §6 V5**

```
grep "2순위 (자율주행" README.md | grep -c "Dynamixel"   # 기대: 0 (2순위에서 제거됨)
grep -c "AMR 물적분할 자회사" README.md                  # 기대: 2 (마일스톤 + probe 절)
```

- [ ] **Step 5: Commit**

```
git add README.md
git commit -m "docs: apply target range reassessment to probe lists"
```

---

### Task 3: Hardware-Arm.md + Phase 4.md — 스파이크 결정 이관과 문구 현행화 (spec §3.3, §3.4)

**Files:**
- Modify: `Roadmap/Hardware-Arm.md` (가이드 안내 행, 보너스 절, 재설계 대기 노트)
- Modify: `Roadmap/Phase 4.md` (진행 원칙의 8월 초 체크포인트 bullet)

- [ ] **Step 1: 가이드 안내 행의 잔재 표기 정정**

```
> Stage 1 가이드 + Stage 2 가이드 미리 작성됨. **진입 시 (스파이크: 2026.08, Stage 1: 2027.01, Stage 2: 2027.04) 다시 체크**.
```

을 다음으로 교체한다:

```
> Stage 1 가이드 + Stage 2 가이드 미리 작성됨. **진입 시 (스파이크: 실행 시기 8월 초 체크포인트에서 확정 — 원안 2026.10, Stage 1: 2027.01, Stage 2: 2027.04) 다시 체크**.
```

- [ ] **Step 2: 보너스 절의 순위 표기 교체**

```
- **Dynamixel 제조사이자 휴머노이드 양산 상장사 (2순위 C)** 지원 시 직접 매칭 (모터 제조사 = 회사 자체).
```

을 다음으로 교체한다:

```
- **Dynamixel 제조사이자 휴머노이드 양산 상장사 (2026-07 사거리 재평가로 1순위 승격 — 시스템 SW 라인 정조준)** 지원 시 직접 매칭 (모터 제조사 = 회사 자체).
```

- [ ] **Step 3: 재설계 대기 노트 분리 (스파이크는 8월 체크포인트, Stage 1/2 는 2026.11 유지)**

```
> **재설계 대기 (2026-07)**: Koch v1.1 확정에 따라 아래 스파이크 (2-DOF 커스텀) 와 Stage 1/2 구성 (XL330 본 빌드, XM430 6DOF 확장) 의 재설계는 2026.11 분기 재평가 안건이다 — 이하 해당 절은 재설계 전 원안이다.
```

을 다음으로 교체한다:

```
> **재설계 대기 (2026-07)**: Koch v1.1 확정에 따라 이하 해당 절은 재설계 전 원안이다. 스파이크의 실행 시기(9월 절충안 vs 원안 10월)와 부품 구성(2-DOF 커스텀 vs Koch v1.1 BOM 정합)은 **8월 초 체크포인트**(`Studies/Phase 4/notes.md` 진행 보드)에서 확정하며, 확정 전 부품 발주는 보류한다. Stage 1/2 구성 (XL330 본 빌드, XM430 6DOF 확장) 의 재설계는 2026.11 분기 재평가 안건으로 유지한다.
```

- [ ] **Step 4: Phase 4.md 체크포인트 bullet 현행화 (기결 사안 반영 + 안건 위임)**

```
- **8월 초 체크포인트**: v1 기술 코어(순서 4) 확보 직후 8-12월 예산·배치를 확정한다. 입력: 7월 실가용 실적(사후 집계), v1 진행 결과. 1순위 검토 안건: Phase 4.5(v1.5) 의 9-10월 조기 진입 — 2026.11 분기 재평가를 가설이 아닌 실물(v1.5 결과) 위에서 수행하기 위함. 순서 6 정독의 "20-30h" 는 이 체크포인트 전까지의 잠정치다.
```

을 다음으로 교체한다:

```
- **8월 초 체크포인트**: v1 기술 코어(순서 4) 확보 직후 8-12월 예산·배치를 확정한다. 입력: 7월 실가용 실적(사후 집계), v1 진행 결과. 조기 수행분(notes.md 2026-06-29 노트 + 2026-07-07 갱신)에서 Phase 4.5 전면 조기 진입 보류와 Section 0 의 2026.08 전진이 기결됐고, 정식 체크포인트의 안건 목록은 notes.md 진행 보드를 따른다. 순서 6 정독의 "20-30h" 는 이 체크포인트 전까지의 잠정치다.
```

- [ ] **Step 5: 검증**

```
grep -c "스파이크: 2026.08" Roadmap/Hardware-Arm.md      # 기대: 0 (잔재 제거)
grep -c "2순위 C" Roadmap/Hardware-Arm.md                 # 기대: 0
grep -c "9-10월 조기 진입" "Roadmap/Phase 4.md"           # 기대: 0 (기결 문구 제거)
```

- [ ] **Step 6: Commit**

```
git add Roadmap/Hardware-Arm.md "Roadmap/Phase 4.md"
git commit -m "docs: move spike timing decision to august checkpoint"
```

---

### Task 4: notes.md — 보드 상태 정정 + 정식 체크포인트 안건 등재 (spec §3.5)

**Files:**
- Modify: `Studies/Phase 4/notes.md` (진행 표 2행 + 조기 체크포인트 노트)

- [ ] **Step 1: 진행 표 상태 열 갱신**

순서 5 행의 `| 4h | 대기 |` 를 `| 4h | 완료 |` 로, 순서 6 행의 `| 20-30h 잠정 (8월 초 체크포인트에서 갱신) | 대기 |` 를 `| 20-30h 잠정 (8월 초 체크포인트에서 갱신) | 진행 중 |` 으로 교체한다 (순서 6 은 week1 정독 완료로 착수 상태).

- [ ] **Step 2: 정식 체크포인트 안건 bullet 추가**

"8월 체크포인트 조기 수행" 노트의 마지막 bullet("- **2026-07-07 갱신**: ... Sections 1-3 은 2026.09-11.") 바로 아래에 추가한다:

```
- **정식 8월 초 체크포인트 안건 (2026-07-20 등재, 입력: 7월 실가용 실적)**: (1) 조기 결론 재확인 (Section 0 2026.08 / Sections 1-3 2026.09-11), (2) 하드웨어 스파이크 실행 시기 결정 — 9월 절충안 vs 원안 10월 (8월은 GPU 종속 작업 과밀로 제외) + 부품 구성(2-DOF 커스텀 vs Koch v1.1 BOM 정합) 선결정, 확정 전 부품 발주 보류, (3) 순서 6 정독 잠정치 갱신, (4) git filter-repo 실행 여부 확정 (진행 원칙의 기존 항목).
```

- [ ] **Step 3: 진행 원칙의 기결 문구 현행화 (Phase 4.md 와 동일 사유)**

진행 원칙의 8월 초 체크포인트 bullet 에서:

```
1순위 검토 안건: Phase 4.5(v1.5) 의 9-10월 조기 진입. 순서 6 정독 잠정치(20-30h)도 여기서 갱신한다.
```

을 다음으로 교체한다:

```
조기 수행분(아래 노트)에서 Phase 4.5 전면 조기 진입 보류·Section 0 의 2026.08 전진이 기결 — 정식 안건 목록은 아래 "정식 8월 초 체크포인트 안건" bullet 을 따른다. 순서 6 정독 잠정치(20-30h)도 여기서 갱신한다.
```

- [ ] **Step 4: 검증 — spec §6 V4**

```
grep -c "| 대기 |" "Studies/Phase 4/notes.md"             # 기대: 0
grep -c "정식 8월 초 체크포인트 안건" "Studies/Phase 4/notes.md"   # 기대: 2 (진행 원칙 참조 + 안건 bullet)
grep -c "1순위 검토 안건" "Studies/Phase 4/notes.md"       # 기대: 0 (기결 문구 제거)
```

- [ ] **Step 4: Commit**

```
git add "Studies/Phase 4/notes.md"
git commit -m "docs: update phase 4 board states and formal checkpoint agenda"
```

---

### Task 5: dry-run 전체 루프 수치 기록 위치 확인 (본인 실행, spec §3.6)

**Files:**
- 확인 대상: `README.md` Evidence 절, `Measurements/openvla-rtx4070-int4/`, week11/week12 로컬 산출물

- [ ] **Step 1: 기록 위치 확인**

week11 dry-run(카메라/bag 1분)의 전체 루프 수치(mean/p95/throughput/GPU)가 어디에 정리됐는지 확인한다. 2026-07-20 검증에서 레포 내 기록을 찾지 못했다 — README Evidence 는 `predict_action` 단독 수치뿐이고 `Measurements/.../methodology.md` 는 전체 루프를 "별도 측정" 미래형으로 기술. 후보: 로컬 미커밋 로그, week11/week12 실습 중 생성 파일.

- [ ] **Step 2: 미기록 시 확보·기록 (8월 내 — 4070 반납 전)**

기존 로그가 없으면 dry-run 1회 재실행으로 수치를 확보하고, README Evidence 표에 전체 루프 행을 추가하거나 `Measurements/openvla-rtx4070-int4/findings.md` 에 기록한다 (2026-08 재측정과 병합 가능).

- [ ] **Step 3: 검증 — spec §6 V6**

README Evidence 또는 Measurements 에서 전체 루프 수치가 조건(입력 소스, 측정 구간)과 함께 확인 가능하다.

- [ ] **Step 4: Commit** (기록 추가 시에만)

```
git add README.md Measurements/
git commit -m "docs: record week11 dry-run full-loop measurements"
```

---

### Task 6: 레포 밖 본인 실행 항목 (커밋 없음)

- [ ] Dynamixel 제조사 인재풀 등록 + 공고 알림 설정 (저가시성 — probe 1단 성격, 재직 중 가능)
- [ ] 스파이크 부품 발주 보류 확인 — 8월 초 체크포인트에서 부품 구성 확정 전에는 XL330·U2D2 등 커스텀 부품을 구매하지 않는다 (Koch v1.1 BOM 과의 정합 미결)

---

### Task 7: .private/notes.md 실명 매핑 현행화 (spec §3.7 — gitignored, 커밋 없음)

**Files:**
- Modify: `.private/notes.md` (1-3순위 표, 제외/폐기 표, 매핑 표, 정찰 지원 후보 절, 출처 헤더)

- [ ] **Step 1: 사거리 재평가 반영** — 로보티즈를 2순위 표에서 1순위 표로 이동(본사 시스템 SW 정조준 / 모방학습·AI 트랙 상향 병기), 컨피그를 3순위(모니터링)에 추가, 로보티즈AI(AMR 물적분할 자회사)를 제외 표에 추가, 리얼월드 행에 상향 재분류 부기, 두산·레인보우 행에 2026-07 재평가(중장기 2028-)와 기존 제약의 긴장 부기
- [ ] **Step 2: 매핑 표 갱신** — "양팔 데이터 인프라 스타트업" = 컨피그 행 추가
- [ ] **Step 3: 정찰 지원 후보 절을 원칙과 정합화** — 휴직 중 지원 금지(정찰 포함, 2026-07 확정)를 명시하고, 목록은 2026.11 안건(실지원 개시 시점 재확인)에서 조기 지원이 결정될 경우의 조건부 후보로만 보존
- [ ] **Step 4: 검증** — `git check-ignore .private/notes.md` 통과(커밋 대상 아님 확인), 문서 내 로보티즈가 2순위 표에 잔존하지 않음

---

## Verification (after all tasks)

- [ ] 회사 실명 grep 0건 — spec §6 V1: `grep -rn "로보티즈\|씨메스\|RLWRLD\|컨피그\|피트인\|두산\|레인보우" --include="*.md" . | grep -v Archive/` 결과 없음
- [ ] 검토 원본 부재 — spec §6 V2: `ls 2026-07-20-career-repo-review.md` 실패 (2026-07-20 삭제 완료)
- [ ] 부록 D 안건·probe 목록·보드 상태의 상호 참조 일관성 — Task 1-4 의 개별 검증 재확인

## Self-Review

**Spec coverage:** §3.1 -> Task 1 / §3.2 -> Task 2 / §3.3 -> Task 3 Step 1-3 / §3.4 -> Task 3 Step 4 / §3.5 -> Task 4 / §3.6 -> Task 5 / §0.2 결정 4·5 -> Task 2·6 + Global Constraints. 전수 대응 확인.
**Placeholder scan:** TBD/TODO 없음. 미완 항목은 전부 체크박스로 추적 (Task 5 는 조건부 Step 명시).
**Type consistency:** "8월 초 체크포인트" 명칭이 Hardware-Arm.md·Phase 4.md·notes.md 세 문서에서 동일 표기이고, 안건 목록의 단일 진실 공급원은 notes.md 로 통일됨.
