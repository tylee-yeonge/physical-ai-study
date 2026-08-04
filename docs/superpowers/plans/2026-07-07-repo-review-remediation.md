# physical-ai-study 검토 보고서 v1.4 반영 Implementation Plan

> 작성일: 2026-07-07
> 대상: 루트 `README.md` / `Roadmap/` (Phase 3, 4, 4.5, 6, 7, Hardware-Arm) / `Studies/Phase 4/` (SETUP.md, README.md, notes.md) / `Studies/Phase 4.5/` (SETUP.md, README.md) / 신규 `Measurements/`, `Portfolio/`
> 사유: 검토 보고서 v1.4 (2026-07-07) 의 권고 1-9 와 산출물 보존 정책 (보고서 §5) 을 리포에 반영 — 문서-의사결정 동기화 + 증거 보존 채널 신설
> Spec: 별도 spec 없음 — 설계 판단은 검토 보고서 v1.4 에 확정돼 있어 본 plan §0 표에 직접 인용한다. 보고서 원문은 Task 1-1 에서 `.private/reviews/physical-ai-study-repo-review-v1.4.md` 로 보존

> 체크박스(`- [ ]`)로 진행을 추적한다. 각 Task 는 검증 Step 과 커밋 Step 으로 끝난다.

---

## 실행 보드 — 지금 할 일 (순차, 2026.07-09)

> 위에서 아래로 순서대로 진행한다. **일상 체크는 이 절에서만 한다** — 아래 Phase 1·2 의 체크박스는 기록·통과 기준의 원본이며, Phase 2 는 대응 항목 완료 시점에 한 번에 닫는다.
> 전제: 4070 (우분투 PC) 은 반납하지 않기로 확정 (2026-07-20, 결정 #5) 됐고 **2026.09 자택 이전이 확정**됐다 — 휴직 기간에도 물리 접근이 유지되므로 **GPU 작업의 8월 마감은 없다.** 배치 기준은 물리 접근이 아니라 가용 시간과 선후 의존이다. 가용: 재직 구간(-2026.08) 평일 저녁 약 2h (주 8-10h).
> 월별 총부하·트랙 충돌 개관은 [2026 하반기 실행 가이드](2026-07-28-year-end-execution-guide.md) 참조 (체크는 이 절에서만).

### 지금 (7월)

- [x] 1. **Block 1 — int4 메모리 산수** (2-3h, GPU 사용). 손계산 14GB → int4 예측 → 실측 대조 → 차이 원인. 결과는 [findings.md §4.1](../../../Measurements/openvla-rtx4070-int4/findings.md) 에 본인 문장으로 (Task 2-1)
  - 진행 (2026-07-10): 측정 완료 — `memory_breakdown.py` 실행, 수치는 methodology §4 에 기입 (context 0.19 / 로드 후 4.74 / 추론 후 5.05 GB, dtype 분해 포함). 부수 발견: 실측 5.05 GB 는 SETUP.md §1.3 의 "약 7GB (논문 실측)" 과 불일치 — 간극 해석은 Step 4, 문구 갱신은 Task 2-2 수치 갱신에서 처리
  - 완료 (2026-07-16): findings §4.1 Step 1-4 + 사다리 표 작성 완료 (QLoRA §3 두 문단 정독 포함). 정제 예측 4.284 GB vs 실측 4.38 GB (잔차 2단 분해: 버퍼 + 미확인 장부 잔차), nvidia-smi 재구성 잔차 로드 시 0 / 추론 후 0.02 GB (allocator 밖 할당 추정). 잔여 이관: "약 7GB (논문 실측)" 과의 간극 해석은 findings §4.1 Step 5 에서 완료 (2026-07-16, 논문 Table 2 + openvla 코드 기본값 확인 — DQ 미사용 +0.34 / 측정 오버헤드 차 +1.05 / 미확인 0.56 GB) — SETUP.md 문구 병기 갱신만 Task 2-2 에 잔존
- [x] 2. **Block 2 — int8 열위 메커니즘** (3-4h). LLM.int8() vs NF4 논문 해당 절만 → findings.md §4.2 → 커밋 (Task 2-1)
  - 진행 (2026-07-28, 커밋 `b2131ee`): findings §4.2 **Step 1-4 완료** — 직관 가정 체크, 선행 정독 (LLM.int8 §2·§3·§4 / QLoRA §3 재독 / bitsandbytes 0.49.2 소스 대조), Step 3 경로 대조표, Step 4 성공률 58.1% 인과 경로
  - 완료 (2026-07-28): Step 5 + 완료 판정 표 4행 작성 → §4.2 5개 Step 전부 닫힘. **단 Step 5·판정 표는 `(by claude)` 작성이다** (본인 요청, findings 내 표기됨) — 산출물은 완성됐으나 "답변 가능" 이라는 통과 기준의 판정은 **#9 자가 검증 10문항으로 이관**한다
  - 부수 산출 2건: (1) **A5000 의 bf16 Hz 가 논문에 없음**을 확인 → int4/bf16 비율은 미확인 종결, Figure 5 캡션의 정성 진술로 대체. (2) **소재 정정** — 속도 수치는 OpenVLA §5.4 본문이고 Table 2 는 성공률·메모리만 담는다. int4 3 Hz 를 §1 에 병기하고 §1 미해소 항목을 해소 표시로 갱신
- [ ] 3. **JD 5-10개 정독 + 격차 매핑 1페이지** — 짧은 블록에 병행

### 8월 초

- [ ] 4. **Phase 4.5 Section 0 전반: ManiSkill sim 구축 + zero-shot baseline** — 상세: [Roadmap/Phase 4.5.md](../../../Roadmap/Phase%204.5.md) Section 0
  - 미착수. 추정 20-30h — 주 8-10h 기준 2-3주 연속 블록이 필요하다. **실제 마감은 8월 말이 아니라 Section 1 착수 (9월 중순) 전 zero-shot baseline 확보**인데, 8월 2주차 착수 기준 가용이 약 24-30h 로 여유가 0 이고, 9월 초는 휴직 적응기라 가용이 줄 수 있다 (실행 가이드 §2) — **즉시 착수가 안전하다**
- [ ] 5. **v1 확장 구현: `RobotPolicy` adapter + action schema validation + 벤치마크 재현 스크립트** (8-12h 추정) — #6 Docker 화 전에 코드 변경 완료. 상세: [Roadmap/Phase 4.md](../../../Roadmap/Phase%204.md) v1 체크리스트
- [ ] 6. **Section 0 후반: Docker 컨테이너화 → RunPod 기동·재현 확인 + LoRA 1사이클 실측 + fine-tuned 4-bit 의 4070 안착 확인** (8-12h + 4-bit 안착 1-2h 추정. 마지막 항목만 4070 필요)
  - 컨테이너화 범위는 **학습 측만**이다 (eval 은 로컬 유지 — 결정 #5). 8월에 닫을 수 있는 것은 Docker 이미지 + RunPod 기동 재현까지다. LoRA 1사이클 실측은 학습 데이터 (Section 1 산출) 가 입력이라 학습 자료 기준 week3 에서, 4-bit 안착은 그 머지 체크포인트가 필요해 week4 에서 닫힌다 — 이 두 항목의 마감은 달력 (8월) 이 아니라 **week3 (LoRA 본 학습) 진입 전**이다
- [ ] 7. **8월 체크포인트**: git author (filter-repo) 실행 여부 결정 + 정독 잠정치 갱신 + 하드웨어 스파이크 시기 확정. 상세: [notes.md](../../../Studies/Phase%204/notes.md) 진행 원칙 (자택 이전 안건은 2026.09 이전 확정으로 닫힘)
- [ ] 7-1. **NVIDIA SO-101 sim-to-real 코스 Phase A** (10-16h, GPU 로컬 불요 — RunPod headless). 이론 4개 모듈 + 워크숍 코드 읽기 + GR00T post-training·sim 평가 1회. 비용 약 1만-2.5만원. 상세: [`docs/research/isaac-sim-so101-course.md`](../../research/isaac-sim-so101-course.md) §4.1
  - 선행 의존이 없어 언제 해도 되지만 **8월 부하와 경쟁한다** (#4 Section 0 20-30h, #8 4-6h 와 같은 달). 과밀하면 9월 이후로 이월해도 손실이 없다 — 팔·물리 접근이 모두 불요다
  - 목적은 코스 완주가 아니라 **발화 가능 수준 확보**다. 완주는 실물 팔이 필요해 Phase B (Stage 1, 2027.01-02) 로 간다
- [ ] 8. **재측정 + Block 3** (4-6h, **GPU 필수**). 예측 → 측정 → 오차 설명. 구간 분해 (전처리 / vision encoder / prefill / decode 7토큰 / un-norm) + [methodology.md](../../../Measurements/openvla-rtx4070-int4/methodology.md) §3 미기록 4항목 (구간 분해·실카메라 입력·notebook→`.py`·`summary.csv`) + SETUP §1.3·README Evidence 표 갱신 → 커밋 (Task 2-2 Step 1·3·4)
  - 배치 근거: GPU 물리 접근이 필요한 항목 중 가장 큰 덩어리다. **vla-lab 첫 발행의 트리거**이기도 해서 (career-sync Task 6) 8월 2-3주에 두고, 그 뒤 발행이 9월에 걸리도록 한다
  - Block 2 에서 인수한 과제: Step 5 가 "roofline 상한 약 4배 vs 실제 근사 동등" 의 간극 배분을 여기로 넘겼다. **decode 실측 (토큰당 ms) 을 대역폭 하한과 대조**해 (a) 가 어디까지 유효했는지 판정하고, vision encoder·prefill·decode 의 ms 비중으로 간극을 배분한다 (findings §4.2 Step 5 오차 설명 → §4.3 Step 3)
- [ ] 8-1. **Block 4 — 3.33 Hz 조건부 판정** (2-3h, **GPU 불요**). 제어 계층 표 + task 유형별 판정 + action chunking. 입력은 Block 3 의 구간 분해 결과와 공개 문헌뿐 (findings §4.4 골격: "새 측정 없음") → 커밋 (Task 2-2 Step 2)
  - GPU 가 필요 없다. 8월 4주에 붙이는 것이 자연스럽지만, 과밀 시 #9 와 함께 9월로 이월해도 안전하다
- [ ] 9. **자가 검증 10문항** — 문서 참조 없이 전부 답변 (문항: 검토 보고서 §2.6). 막히면 해당 Block 재수행 (Task 2-3)
  - 선후: **#8-1 다음**이다. 10문항 중 "병목 구간"·"Jetson 이식 예측" 은 Block 3, "task 경계" 는 Block 4 산출물이라 그 전에 치면 답이 없다
  - 9월로 밀려도 안전하다. 자택 이전 확정으로 휴직 중에도 재측정이 가능하므로, "측정 기반 문항이 막혔을 때 GPU 로 돌아갈 수 없다" 는 리스크가 없다

### 8월 중

- [ ] 10. **Phase 3 재현 확인** (GPU 필수) — TensorRT 엔진 재빌드 + week8 통합 노드 1회 구동, 결과 기록 (Task 2-4)
- [ ] 11. **Rerun 스크린샷/gif 1장** → README Evidence 절 삽입 + `Measurements/.../plots/` 보존 → 커밋 (Task 2-4)
- [ ] 12. GPU 필요 잔여분 (#6 의 4-bit 확인, #8, #10-11) 0건 확인 + **4070 원격 운용 전환 점검** — BIOS 전원 복구 자동 부팅 (Restore on AC Power Loss), 원격 접속 경로 이중화 (SSH + 오버레이 VPN 등), 커널 패닉 자동 리부트 (`kernel.panic` sysctl), (선택) 원격 전원 리셋 수단. 재부팅으로 안 풀리는 하드웨어 장애는 커버 불가 — 그 계층의 보험은 #6 의 RunPod 재현
  - Tailscale 경유 Isaac Sim 원격 GUI 검증은 **드롭한다** — PC 가 자택에 있으므로 GUI 는 로컬에서 띄운다. 출장지 원격 사용이 실제로 필요해질 때 수행한다 (약 30분, 절차는 [Hardware-Arm.md](../../../Roadmap/Hardware-Arm.md) Stage 1 실행 머신 절)
  - [ ] **Jetson Orin Nano CUDA 스택 확인** (약 30분) — 256GB SSD + Ubuntu 22.04 는 설치 완료 상태이므로 재플래시는 불요 전망. 확인할 것은 **CUDA·cuDNN 동반 여부**뿐이다 (`cat /etc/nv_tegra_release`, `dpkg -l nvidia-jetpack`) — 없으면 PyTorch GPU 가 안 붙고, JetPack 재설치에는 **x86 Ubuntu 호스트 (= 이 PC) 가 필요**하다. 맥북은 SDK Manager 호스트가 될 수 없다. PC 가 자택에 있으므로 시점 제약은 없고, Jetson 이 v3 배포 타깃으로 실제 필요해질 때 수행한다

### 남는 시간에 (이월 가능 — GPU 작업과 충돌하면 무조건 뒤로)

- [ ] 13. **정독 week2·4·5·6** (20-30h, GPU 무관 — 휴직 중으로 밀려도 안전). 상세: [notes.md](../../../Studies/Phase%204/notes.md) 순서 6

### 휴직 개시 (2026.09) 직후

- [ ] 14. LinkedIn 헤드라인 교체 (승인 확정 2026-07-28 — 선행 조건 해소, 9월 1주 집행) + 현직자 커피챗 개시
- [ ] 15. Phase 4.5 Sections 1-3 착수 (2026.09-11)

> **8월 배치 판정**: 자택 이전 확정으로 GPU 물리 접근 마감이 없어졌으므로, 8월에 둘 항목은 **선후 의존과 가용 시간**으로만 정한다. 재직 구간(-2026.08)이 주 8-10h 로 가장 빡빡하므로 시간이 많이 드는 항목(#8, Section 0)을 앞에 두고, 나머지는 9월 이월을 허용한다.
> Docker·RunPod (#6) 이 필요한 이유는 **LoRA 가 24GB+ 를 요구해 로컬 4070 으로 학습이 불가능하기 때문**이다 (장애 대비가 아니다). RunPod 은 Isaac Sim GUI 를 대체하지 못하므로 (UDP 미지원) Sim GUI 계열은 로컬 전용이다.

### 문서 지도 (뭘 어디서 보나)

| 문서 | 용도 |
|---|---|
| **본 문서 §실행 보드** | 지금 할 일 순서 — **유일한 일상 체크 보드** |
| `docs/superpowers/plans/2026-07-28-year-end-execution-guide.md` | 2026.08-12 월별 배치·부하·트랙 충돌 개관 (체크박스 없음 — 배치를 다시 짤 때만 열기) |
| `README.md` | 전체 로드맵·타임라인·실측 결과 (외부에 보여주는 얼굴) |
| `Roadmap/Phase *.md` | Phase 별 상세 계획 (착수할 때만 열기) |
| `Studies/Phase 4/notes.md` | Phase 4 학습 노트 + 정독 체크 |
| `Measurements/` | 실측 증거 — Block 결과는 findings.md 에 작성 |
| `Portfolio/evidence-index.md` | 증거 목록 (실험 마칠 때 한 줄 추가) |
| 본 문서 Phase 1·2 | 검토 보고서 반영 기록 + 통과 기준 원본 (참조용) |

---

**Goal:** 검토 보고서가 확정한 의사결정 (RunPod 전환, 자작 팔 SO-101 (결정 #6), 육아휴직 2026.09-2027.02, Phase 4.5 Section 0 전진, probe 분해 일정, Phase 3 supporting 공개, 포지셔닝 하향, v1 정의 업그레이드, v2.5 신설) 을 리포 문서에 동기화하고, `Measurements/` + `Portfolio/` 구조를 신설해 "정답은 지우고 증거는 남긴다" 정책을 가동한다. 완료되면 리포가 계획의 single source of truth 로 복원되고, 4070 실측 증거가 외부인이 접근 가능한 위치에 놓인다.

**Architecture:** 리포를 `Studies/` (정답 없는 반복 학습) / `Measurements/` (실측 증거 보존) / `Portfolio/` (evidence index) 로 역할 분리한다 (보고서 §5.5). 전면 재편은 하지 않고 신설 + 이관 + 문서 갱신만 수행한다 (보고서 권고 5 의 단계적 적용).

**Tech Stack:** Markdown (CommonMark/GFM), Mermaid, git. 코드 변경 없음 — 문서·구조 작업.

**Note on verification:** 자동 테스트로 닫히지 않는 작업이다. 완료 조건은 (1) grep 기반 잔존 문자열 0 확인 (Colab, 비공개 리허설, 구 휴직 기간 등), (2) 이동한 파일의 참조 링크 무결성, (3) 권고 1-9 ↔ Task 커버리지 전수 매핑 (Self-Review). Phase 2 (본인 실행 항목) 는 통과 기준을 각 Task 에 명시하고 체크박스로만 추적한다.

## 0. 확정된 결정

| # | 항목 | 결정 | 근거 |
|---|---|---|---|
| 1 | 산출물 보존 정책 | **"정답은 지우고 증거는 남긴다"** — quiz 정답·완성 실습 코드는 원복, 실측·판단 기록은 `Measurements/` 에 보존. 워크플로우는 증거 보존이 원복보다 먼저 | 보고서 §5.1-5.6 |
| 2 | 구조 신설 범위 | `Measurements/` + `Portfolio/` 만 지금 신설. `Studies/` 전면 재배치·README 결과 중심 전면 개편은 v1 기록 시점 (2026 하반기) 과 결합 | 보고서 권고 5 |
| 3 | 학습 컴퓨트 | Colab **배제** (SSH 불가·세션 휘발), **RunPod Community Cloud RTX 4090 (24GB)** 확정 | 보고서 §2.3 |
| 4 | 육아휴직 | **2026.09-2027.02** (2026-07-01 신청, **2026-07-28 승인 확정**. 복직 2027.03 동일). 2026.06-08 은 휴직이 아니라 **재직 구간** (주 8-10h 로 가용 시간이 가장 빡빡한 구간) | 보고서 §2.3 + 2026-07-28 승인 |
| 5 | GPU 반납·배치 | RTX 4070 (우분투 PC) 은 **반납하지 않기로 확정 (2026-07-20)** 이고 **2026.09 자택 이전 확정**이다 → 휴직 기간에도 물리 접근이 유지되므로 GPU 종속 작업에 시점 마감이 없고, SO-101 팔 옆 기계도 이 PC 다. 하드웨어 증설 (RAM·GPU) 은 불가하므로 로컬 사양 부족은 RunPod 이관으로 대응하되, **GUI 스트리밍 워크로드는 이관 불가** (RunPod UDP 미지원) | 보고서 §2.3, 권고 2-4 + 2026-07-20·09 이전 확정 |
| 6 | 자작 팔 | **SO-101 / SO-ARM101 리더-팔로워, 약 56-58만원, 2026.09-10 구매** (국내 조달, 리드타임 3일). 근거: LeRobot/HF 생태계 표준 = 경험의 이식성. 커스텀 XL330+XM430 안과 Koch v1.1 은 비채택 (사유: Hardware-Arm.md 비채택 기록) | 보고서 §2.3 + 2026-07-28 결정 |
| 7 | Phase 4.5 | **Section 0 (ManiSkill/SAPIEN sim 구축 → Docker → RunPod 이관) 을 2026.08 로 전진 배치**, Sections 1-3 은 2026.09-11 | 보고서 §2.3 |
| 8 | probe 일정 | 2026.06 일괄 가정 폐기 → 가시성 기준 분해: **리포 공개 증거·JD 정독 2026.07-08 / LinkedIn 헤드라인·커피챗 2026.09** (휴직 개시·승인 확정 후) | 보고서 §2.2 |
| 9 | Phase 3 지위 | 비공개 리허설 → **supporting system work 로 공개**. 조건 2개: supporting 라벨 + README 계층 하위 배치 / 빌드 스크립트 기준 재현 확인 (4070 에서 재빌드 — PC 자택 이전 확정으로 시점 제약 없음) | 보고서 §2.3 |
| 10 | v1 정의 | adapter 추상화 (`RobotPolicy` 인터페이스) + action schema validation + 벤치마크 재현성 을 v1 범위에 추가. 안전 실행 스택 (watchdog, fallback, collision) 은 v2 로 명시 이관 | 보고서 권고 6 |
| 11 | 산출물 확장 | 부록 B 에 **v2.5 (자작 데이터셋, LeRobot 포맷, HF Hub 공개)** 신설. SO-101 조립 후 LeRobot ACT 1회 실행. Diffusion Policy 는 라잇 정리 수준 (면접 방어용). Isaac Lab 튜토리얼 1개 (RunPod, NVIDIA 무료 코스) | 보고서 권고 8, 9 |
| 12 | 포지셔닝 | "cross-embodiment VLA/FM 통합 엔지니어" → **"Robot Learning Deployment / Physical AI Systems Engineer"** (복수 embodiment 실경험 전 방어 불가한 표제 배제). "박사가 못 만드는 3가지" 서사 → **실행 품질 근거** 서사로 교체 | 보고서 §2.5, 권고 7 |
| 13 | 실측 증거 승격 | 4070 실측 표를 README 최상단으로 승격 (현재 보유분 그대로, 재측정 대기 안 함) | 보고서 권고 1 |
| 14 | 재측정·이해 검증 | 단순 재실행이 아니라 **예측 → 측정 → 오차 설명** 4개 블록 (11-16시간) 으로 수행. Block 1-2 는 2026.07, Block 3-4 + 재측정은 2026.08 | 보고서 §2.6, 권고 2 |

## 0.1 이 계획이 보장하지 않는 것

- Phase 2 항목 (재측정, 이해 검증 4블록, Phase 3 재현 확인) 의 **수행 자체**는 본인 작업이다 — 본 plan 은 구조·통과 기준·데드라인만 제공하고, 산출물 품질 (면접 방어 가능 수준의 이해) 을 자동으로 만들지 않는다.
- 이력서·LinkedIn 문구의 동일 지위 통일은 리포 밖 작업 — Task 1-4 에 리마인더로만 남긴다.
- SO-101 확정 (2026-07-28) 에 따른 Stage 2 확장 수단 재설계는 본 plan 에서 판단하지 않는다 — BOM·구매 확정분만 반영하고, 재설계는 2026.11 분기 재평가 안건으로 이관한다. 스파이크는 완제품 키트에 맞춰 "선구매" 가 아니라 "조립 전 선검증" 으로 재정의됐다 (Hardware-Arm.md 스파이크 절).
- Phase 4.5 Sections 1-3 (2026.09-11) 과 스파이크 (2026.10) 의 병행이 "한 구간 1트랙" 원칙과 충돌하는 문제 — 8월 체크포인트 / 2026.11 재평가 안건.
- `Studies/` 하위 week 학습 자료 내부의 Colab 잔존 언급 (Phase 4 week1·week8, Phase 4.5 week1·3·4) 은 해당 week 진입 시 갱신한다 — "진입 시 재검토" 원칙과 일치, 본 plan 범위 밖.
- 리포 전면 재편 (README 결과 중심 전면 개편, `Studies/` 재배치) 은 v1 레포 기록 시점 (2026 하반기) 별도 작업.

## Global Constraints

- **SETUP.md §1.3 실측 수치 본체는 불변** — 재측정 (Task 2-2) 전까지 수치를 고치지 않는다. 이동·인용만 한다.
- `Studies/` 학습 자료의 문제 상태 (정답 미포함) 를 훼손하지 않는다. 본 plan 이 만드는 새 파일은 `Measurements/`, `Portfolio/`, `docs/` 하위로 한정.
- 마크다운 규칙 준수: bold 인접 조사 (`**용어**(영문명)조사`), 범위는 `-` (`~` 금지), 다이어그램은 Mermaid, 이모지 금지, 현재 상태 기준 서술 (변경 이력 서술 금지 — 단 의사결정 로그 성격의 notes.md 갱신 노트는 예외).
- 커밋은 Task 단위, Conventional Commits (영어, 소문자, 마침표 없음), AI 표기 없음. push 는 별도 승인 전 금지.
- 본 plan 파일은 커밋한다 (2026-07-07 결정 — 진행 체크박스를 리포에서 추적). 검토 보고서 원문은 전략·probe 일정 등 외부 노출 부적합 내용이 있어 `.private/` (gitignore 대상) 에만 보존하고 커밋하지 않는다.
- 진행 로그 (완료 일자, 미완 사유) 는 본 plan 의 해당 Step 아래에만 기록한다.

---

## Phase 1: 문서·구조 반영 (2026.07, 세션에서 즉시 실행 가능)

### Task 1-1: 검토 보고서 원문 보존

**Files:**
- Create: `.private/reviews/physical-ai-study-repo-review-v1.4.md`

**Interfaces:** 이후 모든 Task 가 인용하는 근거 문서의 리포 내 고정 경로.

- [x] **Step 1: 보고서 저장**

검토 보고서 v1.4 원문 (대화 첨부본) 을 위 경로에 그대로 저장한다. `.private/` 은 gitignore 대상이므로 커밋되지 않는다 — 전략·probe 일정 등 외부 노출 부적합 내용을 담고 있어 의도적 비커밋.

진행 상황 (2026-07-09): 저장 완료.

- [x] **Step 2: 검증 — 파일 존재 + 참조 성립**

```bash
test -f ".private/reviews/physical-ai-study-repo-review-v1.4.md" && echo OK
git check-ignore ".private/reviews/physical-ai-study-repo-review-v1.4.md" && echo IGNORED
```

기대: OK, IGNORED 둘 다 출력. (커밋 Step 없음 — gitignore 대상.)

---

### Task 1-2: `Measurements/` + `Portfolio/` 신설 + 보존 정책 문서화 (보고서 §5, 권고 5 전반부)

**Files:**
- Create: `Measurements/README.md` (보존 정책 본문)
- Create: `Measurements/openvla-rtx4070-int4/environment.md`, `methodology.md`, `findings.md`
- Move: `Studies/Phase 4/week6/openvla_latency_4070_int4.npy` → `Measurements/openvla-rtx4070-int4/raw/`
- Move (조건부): `Studies/Phase 4/week6/practice.ipynb` → `Measurements/openvla-rtx4070-int4/scripts/` (내용이 latency 측정 코드일 때 — 보존 정책 §5.4-1 "측정 스크립트는 남기되 Measurements 로")
- Create: `Portfolio/evidence-index.md`
- Modify: `Studies/Phase 4/SETUP.md` (§1.3 표의 `.npy` 경로), `Studies/Phase 4/notes.md` (순서 1 노트의 결과 파일 경로)

**Interfaces:** `Measurements/openvla-rtx4070-int4/` — Task 1-3 (README 승격 링크), Task 2-2 (재측정 산출물 착지점) 가 의존.

- [x] **Step 1: `Measurements/README.md` 작성**

보고서 §5 를 리포 언어로 옮긴다. 포함 항목:

1. 원칙 — "정답은 지우고, 증거는 남긴다." `Studies/` 는 언제든 처음부터 다시 풀 수 있는 상태로 유지, 실측·판단 기록은 여기 보존 (원복 금지)
2. 저장하지 않는 것 (원복 대상): quiz 정답 / PRACTICE 완성 코드 / 단계별 solution / 따라 친 튜토리얼 코드. 판정 기준: "다음 복습 때 이것이 남아 있으면 답을 베끼게 되는가?"
3. 반드시 저장하는 것: 측정 raw 데이터·조건 (warm-up, batch, 해상도, 동기화) / 결과 통계 (p50/p95/p99, VRAM peak) / 환경·재현 정보 (버전, hardware, 명령어, commit) / 판단 기록 (오류·해결·배제 근거) / 시각 자료. 판정 기준: "제 3자가 이 수행·판단을 검증할 수 있는가?"
4. 경계 사례 3규칙: 측정 스크립트는 `Measurements/<실험명>/scripts/` 에 남긴다 (위치 분리로 두 목적 동시 달성) / 판단에 필요한 코드 스니펫은 findings.md 에 인용 / 애매하면 남긴다 (비대칭: 지운 증거는 복구 불가)
5. 실험 디렉터리 표준 구조: `environment.md`, `methodology.md`, `raw/`, `summary.csv`, `plots/`, `scripts/`, `findings.md`
6. 워크플로우 (실습 1건 종료 시): 증거 커밋 → evidence-index 1줄 추가 → Studies 원복 → notes.md 회고. **증거 보존이 원복보다 먼저**
7. 소급 적용: Phase 4 실측만 이관 (본 디렉터리), Phase 0-2 단순 실습은 소급 보존 안 함

- [x] **Step 2: 실험 디렉터리 생성 + `.npy`·스크립트 이관**

진행 상황 (2026-07-09): `practice.ipynb` 내용 확인 결과 latency 측정 코드 (warm-up 5회 + n=100 + synchronize + 통계 산출) 로 판정 — `scripts/` 로 `git mv` 완료. 외부 참조 없음 확인.

```bash
mkdir -p "Measurements/openvla-rtx4070-int4/raw" "Measurements/openvla-rtx4070-int4/scripts"
git mv "Studies/Phase 4/week6/openvla_latency_4070_int4.npy" "Measurements/openvla-rtx4070-int4/raw/"
# practice.ipynb 는 내용 확인 후: 측정 코드면 git mv(미추적이면 mv), 학습 정답 코드면 보존 정책 §5.2 에 따라 원복 대상으로 분류만 하고 이동하지 않음
```

- [x] **Step 3: `environment.md` 작성 (기지 사실만)**

진행 상황 (2026-07-09): 측정 기록 commit `1652c81` 확인·기입.

`Studies/Phase 4/SETUP.md` §7.1 실설치 열에서 옮긴다: Ubuntu PC, RTX 4070 12GB, driver 580.159.03, CUDA 13.0, Python 3.12.3, PyTorch 2.12.0, transformers 4.40.1, tokenizers 0.19.1, timm 0.9.16, accelerate 1.0.1, bitsandbytes 0.49.2, eager attention, 측정일 2026-06 (week6). 측정 시점 commit 은 `git log --oneline -- "Studies/Phase 4/week6"` 로 확인해 기입.

- [x] **Step 4: `methodology.md` 골격 작성**

기지 사실 (n=100, `predict_action` 단독 측정 — 이미지 전처리·ROS2 제외, bitsandbytes nf4, do_sample=False) 을 기입하고, 미기록 항목은 체크박스로 남긴다 (본인이 Task 2-2 재측정에서 채움): warm-up 횟수, batch size, 입력 해상도, CPU/GPU 동기화 방식, preprocessing 포함 여부, p50/p99, VRAM peak, 재실행 명령어.

진행 상황 (2026-07-09): 측정 노트북 실행 기록에서 계획이 미기록으로 가정했던 항목 다수가 확인됨 — warm-up 5회, batch 1, 224x224 랜덤 RGB (매 반복 새 이미지), synchronize 앞뒤 수행, 전처리 제외, p50 301.3 / p99 305.6 / min 290.4 / max 308.2 ms, `memory_allocated` 4.38 GB → 기지 사실로 기입. 잔여 미기록: nvidia-smi 기준 VRAM peak, 구간별 분해, 실카메라 입력 조건, 재실행 스크립트 정리, summary.csv.

- [x] **Step 5: `findings.md` 골격 작성**

기존 판단 기록을 출처 표기와 함께 발췌 이관 (SETUP.md §1.3, notes.md 순서 1·3 노트): int8 배제 사유 (성공률 58.1% vs int4 71.9%, 1.2 Hz), 3.33 Hz 의 제어 주기 해석 (quasi-static 적합 추정, 전체 루프 2 Hz 하한 근거), 외삽 대비 ±50% 검증. 본인 재작성 영역 (Block 1-4 결과: int4 메모리 산수, int8 열위 메커니즘, 300 ms 구간 분해, 3.33 Hz 조건부 판정) 은 소제목 + 체크박스로 비워 둔다.

- [x] **Step 6: `Portfolio/evidence-index.md` 작성**

| 증거 | 날짜 | 위치 | 입증 역량 |
|---|---|---|---|
| OpenVLA 7B int4 latency 실측 (RTX 4070) | 2026-06 (재측정 2026-08 예정) | `Measurements/openvla-rtx4070-int4/` | 양자화 배포 실측·컴퓨트 의사결정 (셋째 층) |
| Phase 3 perception 통합 노드 (supporting) | 2026-06 | `Studies/Phase 3/` week8 | TensorRT + ROS2 통합 (VLA wrapper 스캐폴드) |

- [x] **Step 7: 참조 갱신**

`Studies/Phase 4/SETUP.md` §1.3 표의 출처 셀과 `Studies/Phase 4/notes.md` 순서 1 노트의 결과 파일 경로를 새 경로 (`Measurements/openvla-rtx4070-int4/raw/openvla_latency_4070_int4.npy`) 로 교체.

- [x] **Step 8: 검증**

```bash
grep -rn "week6/openvla_latency" --include="*.md" . | grep -v .private   # 기대: 0건
test -f "Measurements/openvla-rtx4070-int4/raw/openvla_latency_4070_int4.npy" && echo OK
git status --short   # 이동이 rename 으로 잡히는지 확인
```

- [x] **Step 9: Commit**

```bash
git add Measurements Portfolio "Studies/Phase 4/SETUP.md" "Studies/Phase 4/notes.md" "Studies/Phase 4/week6"
git commit -m "docs: adopt evidence retention policy with Measurements and Portfolio structure"
```

---

### Task 1-3: README 최상단 실측 증거 승격 (권고 1)

**Files:**
- Modify: `README.md` (제목 blockquote 직후, 첫 `---` 앞에 섹션 삽입)

- [x] **Step 1: "실측 결과" 섹션 삽입**

README.md:8 (전제 blockquote) 뒤에 다음 섹션을 삽입한다:

```markdown
## 실측 결과 (Evidence)

> 이 리포의 가장 강한 증거를 먼저 둔다. 상세 데이터·방법론·판단 근거: [`Measurements/openvla-rtx4070-int4/`](./Measurements/openvla-rtx4070-int4/)

| 항목 | 결과 | 조건 |
|---|---|---|
| OpenVLA 7B int4 로드 | RTX 4070 12GB 에 OOM 없이 안착 (약 7GB) | bitsandbytes nf4 + transformers 4.40.1 |
| 추론 latency | mean 300.3 ms / p95 304.8 ms / std 3.8 ms (n=100) | `predict_action` 단독, 전처리·ROS2 제외 |
| throughput | 3.33 Hz | quasi-static 단일 task 적합 추정 |
| int8 경로 | 배제 — 성공률 58.1% (int4 71.9%), 1.2 Hz | OpenVLA 논문 Table 2·§5.4 근거 + 실측 판단 |

측정: 2026-06, 재측정 (방법론 보강 + p50/p99/VRAM peak) 2026-08 예정. Rerun 시각화 gif 는 확보 시 이 절에 추가 (4070 반납 전).
```

- [x] **Step 2: 검증 — 렌더링 확인**

VS Code 미리보기로 표·링크 렌더링 확인, `Measurements/` 상대 링크 클릭 가능 확인.

- [x] **Step 3: Commit**

```bash
git add README.md
git commit -m "docs: promote 4070 measurement results to readme top"
```

---

### Task 1-4: 포지셔닝 문구 교체 (권고 7, 보고서 §2.5)

**Files:**
- Modify: `README.md`, `Roadmap/Hardware-Arm.md`, `Roadmap/Phase 6.md`, `Roadmap/Phase 7.md`

- [x] **Step 1: 표제 교체 (4곳)**

- README.md:4 — `**cross-embodiment VLA/Foundation Model 통합 엔지니어**` → `**Robot Learning Deployment / Physical AI Systems Engineer**`
- README.md:462 (커리어 경로 종점) — `cross-embodiment VLA/FM 통합 엔지니어` → `Robot Learning Deployment / Physical AI Systems Engineer`
- README.md:471-474 (최종 포지셔닝 blockquote) — 첫 줄을 "이기종 플랫폼(이동+조작)에 Foundation Model 을 실제 로봇 (자작 팔 포함) 에 배포해본 **Robot Learning Deployment / Physical AI Systems Engineer**" 로 교체. 좌표 줄 (둘째 층 + 셋째 층 묶음) 은 유지
- README.md:682 (말미) — 동일 교체

- [x] **Step 2: cross-embodiment 서사 지위 하향**

README.md:481 blockquote 의 라벨 `**cross-embodiment 서사** (헤드라인 좌표)` → `**cross-embodiment 서사** (지향 좌표 — 복수 embodiment 실경험 확보 전에는 표제로 쓰지 않음, 2026.11 재평가 입력)`. 본문 서사 (분석·연결 계획) 는 유지 — 주장이 아닌 계획 서술이므로.

- [x] **Step 3: "박사가 못 만드는 3가지" 서사 교체 (2곳)**

진행 상황 (2026-07-09): README 는 원문이 blockquote 목록이라 heading 대신 blockquote 도입문 형태로 동일 취지 반영, Hardware-Arm.md 는 계획 문안대로 heading + 도입문. 최종 포지셔닝 첫 줄의 기존 표제는 "Brain-Body 통합 SW 엔지니어" 였음 — 계획 문안대로 교체.

README.md:279-282 와 Roadmap/Hardware-Arm.md:27-30 의 제목·도입을 교체한다. 항목 3개 (latency 측정 / 안전 메커니즘 / BOM 비용 이해) 는 유지하되 희소성 주장을 제거:

```markdown
### 실행 품질로 증명하는 3가지

latency 측정, e-stop, BOM 이해는 로보틱스 랩에서도 일상적으로 다룬다 — 항목 자체가 희소한 것이 아니다.
차별점은 양산 ROS 5년 경험자가 이것들을 제품 수준 감각 (측정 방법론, 안전 설계 관행, 원가 구조) 으로 수행한다는 **실행 품질**이다.
```

- [x] **Step 4: "박사도 만든다" 문구 완화 (3곳)**

README.md:269, Roadmap/Phase 7.md:28, Roadmap/Phase 6.md:23 — `"Sim only 산출물은 박사도 만든다. Sim + 자작 실 팔이면 본인만 만든다."` → `"Sim-only 산출물은 차별점이 되지 않는다 — 자작 실 팔과 결합되어야 본인 강점 (실배포·통합) 이 실린다."` (Phase 6.md 는 문장 구조에 맞게 동일 취지로.)

- [x] **Step 5: 검증**

```bash
grep -rn "cross-embodiment VLA" README.md Roadmap/          # 기대: 0건 (표제 용법)
grep -rn "박사" README.md Roadmap/                            # 기대: 0건
```

리마인더 (리포 밖): 이력서·LinkedIn 초안의 표제도 동일 문구로 통일 — 2026.09 헤드라인 교체 시 적용.

- [x] **Step 6: Commit**

```bash
git add README.md Roadmap/
git commit -m "docs: reposition headline to robot learning deployment engineer"
```

---

### Task 1-5: 컴퓨트 동기화 — Colab → RunPod (권고 3, 결정 #3·#5)

**Files:**
- Modify: `Studies/Phase 4/SETUP.md` (본체), `Studies/Phase 4/README.md`, `Roadmap/Phase 4.md`, `Studies/Phase 4.5/SETUP.md`, `Studies/Phase 4.5/README.md`

**Interfaces:** SETUP.md 가 컴퓨트 전략의 단일 진실 공급원 — Roadmap/Phase 4.5.md (Task 1-8) 는 이 결과를 인용만 한다.

- [x] **Step 1: `Studies/Phase 4/SETUP.md` 전환**

섹션별 지시 (수치 본체 §1.3 은 불변):

| 절 | 변경 |
|---|---|
| §0 한 줄 요약 | "무거운 LoRA 파인튜닝·sim 작업은 RunPod (Community Cloud RTX 4090 24GB), 가벼운 추론은 로컬 4070 + 4bit 양자화. **Colab 은 SSH 불가·세션 휘발로 배제** (2026-07 결정). 4070 반납 (2026.08) 후 추론·eval 은 Docker 이미지로 RunPod 이관 (Phase 4.5 Section 0)" 로 재작성 |
| §1.2 표 | "적합 환경" 행: Colab → **RunPod (클라우드 GPU)**. Colab 단점 무력화 서술 문단 → "클라우드 GPU 의 단점 (세션·ROS2·카메라) 이 학습에는 무력화된다" 로 일반화 |
| §2.1 계정 | Colab Pro / Google Drive 항목 → RunPod 계정 + 크레딧 충전, (Drive 항목은 삭제 또는 백업 용도로 강등) |
| §3 도식·상세 | Mermaid: Colab 서브그래프 → RunPod (RTX 4090), Drive 전송 계층 → "RunPod network volume + rsync/scp". §3.2 Colab 측 → RunPod 측 (SSH 접속, Docker 이미지 기동, 체크포인트를 volume 에 저장) |
| §4 표 | "Colab 필요" 열명 → "클라우드 GPU 필요". 병행 행의 "O (A100 권장)" → "O (RTX 4090)" |
| §5 전면 재작성 | 제목 "RunPod 측 환경 세팅". §5.1 GPU 선택: Community Cloud RTX 4090 24GB 기준 (LoRA 24GB+ 충족), 요금은 셋업 시점 확인 (수치 하드코딩 금지 — 변동). §5.2 표준 셋업: SSH key 등록 → pod 생성 (CUDA·PyTorch 는 §7 매트릭스와 일치하는 Docker 이미지) → 코드/데이터 rsync. §5.3 중단 대비: network volume 에 체크포인트 (`save_steps` 명시) — pod 중지 시에도 volume 잔존, 유휴 시 pod 중지로 과금 차단 |
| §7 표 | "Colab (학습, ...)" 열명 → "RunPod (학습, v1.5 진입 시 맞춤)". `requirements_colab.txt` 참조 → `requirements_runpod.txt` (미작성 파일 — 문자열만 교체) |
| §8 전송 | "Colab → Drive → 로컬" → "RunPod volume ↔ 로컬 rsync/scp" 로 재작성. Mermaid sequenceDiagram 참여자 교체. rclone/gdown 절 → rsync 예시 1줄 + scp 대안 |
| §9.2 리스크 | "Colab 세션 불안정" → "RunPod 가용성·비용" (Community Cloud 는 인스턴스 회수 가능성 있음 → 체크포인트 필수, 유휴 과금 주의) |
| §11.2 체크리스트 | Colab 문구 3곳 → RunPod 로 교체 |

- [x] **Step 2: 인용 문서 동기화**

- `Studies/Phase 4/README.md:16` — "학습은 Colab A100/L4" → "학습은 RunPod (RTX 4090)" 문장 교체
- `Roadmap/Phase 4.md:85` — "(v1.5) LoRA 파인튜닝 | Colab A100/L4 (클라우드)" → "RunPod RTX 4090 (클라우드)"
- `Roadmap/Phase 4.md:88, 90` — "v1.5/Phase 4.5 에서 Colab" → "RunPod", "무거운 학습(v1.5 LoRA)은 Colab" → "RunPod"
- `Studies/Phase 4.5/SETUP.md` — §0 한 줄 요약·§1 공유 목록의 Colab 5곳 → RunPod (구조 재작성은 Phase 4.5 진입 시 — 문자열 수준만)
- `Studies/Phase 4.5/README.md:17` — 동일 교체

- [x] **Step 3: 검증**

진행 상황 (2026-07-09): 잔존 1건은 `Studies/Phase 4/SETUP.md` §0 의 배제 사유 기록 ("Colab 은 SSH 불가·세션 휘발로 배제") — 낡은 계획 참조가 아니라 의사결정 근거이므로 의도적 잔존. 그 외 0건. Roadmap/Phase 4.5.md 의 컴퓨트 문자열 교체 (계획상 Task 1-8 몫) 도 검증 정합을 위해 본 Task 에서 함께 수행함. `Studies/Phase 4/notes.md` 의 과거 노트 내 Colab 2건은 의미 보존 선에서 "클라우드 GPU" 로 일반화.

```bash
grep -rn "Colab" README.md Roadmap/ "Studies/Phase 4/SETUP.md" "Studies/Phase 4/README.md" \
  "Studies/Phase 4/notes.md" "Studies/Phase 4.5/SETUP.md" "Studies/Phase 4.5/README.md"
# 기대: 0건 (week 자료 내 잔존은 범위 밖 — §0.1 참조)
```

- [x] **Step 4: Commit**

```bash
git add "Studies/Phase 4" "Studies/Phase 4.5" Roadmap/ README.md
git commit -m "docs: replace colab with runpod as cloud compute across setup and roadmap"
```

---

### Task 1-6: 일정 동기화 — 육아휴직·probe·v1 표기 잔재 (권고 3, 결정 #4·#8)

**Files:**
- Modify: `README.md`, `Roadmap/Phase 4.md`, `Roadmap/Phase 3.md`

- [x] **Step 1: 육아휴직 기간 교체 (2026.06-2027.03 → 2026.09-2027.02)**

- README.md:46 — "**2026.06-2027.03 은 육아휴직 기간**" → "**2026.09-2027.02 는 육아휴직 기간** (2026-07-01 신청, 승인 대기. 2026.06-08 은 재직 + GPU 반납 전 구간 — 평일 저녁 약 2h 가용 전제)"
- README.md:67 (gantt) — `:active, leave, 2026-06, 9M` → `:active, leave, 2026-09, 6M`
- README.md:101 (표 행) — "2026.06-2027.03" → "2026.09-2027.02", 내용에 재직 구간 주석 추가
- README.md:317, 320 — 기간 문자열 교체 + "육아휴직 중" 전제로 쓰인 2026.06-08 활동은 "재직 중 저강도" 로 전제 수정
- README.md:637 (부록 D) — "육아휴직 중이라" 문구는 2026.11 시점 기준으로 여전히 참 — 유지, 기간 표기만 있으면 교체
- Roadmap/Phase 4.md:202 — "육아휴직(2026.06-2027.03)" → "(2026.09-2027.02)"
- Roadmap/Phase 3.md:154 — 동일 교체

- [x] **Step 2: probe 일정 분해 (2026.06 일괄 → 가시성 기준 3분해)**

- README.md:99 (표 행) — "2026.06-08 (병행) | 시장 신호 probe: JD 정독 + 커피챗 + LinkedIn 헤드라인" → 2행 분해: "2026.07-08 | probe 1단 (저가시성): 타겟사 JD 5-10개 정독 + 격차 매핑, 학습 리포 공개 증거 정비" / "2026.09- | probe 2단 (고가시성): LinkedIn 헤드라인 교체 + 커피챗 1-2건 (휴직 개시·승인 확정 후)"
- README.md:320-324 (probe 절 목록) — 항목별 시점 재배치: JD 정독 (2026.07-08), 헤드라인 교체 (2026.09 — "육아휴직 승인 계류 중 교체는 고용주에 이직 신호 → 승인 확정 후" 근거 병기), 커피챗 (2026.09 이후)
- README.md:435 (커리어 경로) — "시장 신호 probe (2026.06~, ...)" → "(1단 2026.07-08 / 2단 2026.09~)"
- README.md:499-504 (마일스톤 "#### 2026.06 (Phase 4 와 병행)" 절) — 헤더를 "#### 2026.07-09 — 시장 신호 probe (가시성 분해)" 로 교체, LinkedIn 항목에 "(2026.09, 휴직 개시 후)" 명기
- Roadmap/Phase 3.md:153 — "2026.06~" → 분해 표기

- [x] **Step 3: v1 범위 표기 잔재 정정**

진행 상황 (2026-07-09): 계획 대상 (README 커리어 경로·마일스톤·부록 B v1 행) 외에 Roadmap/Phase 3.md:151 의 "sim 단일 task 루프" 잔재도 발견해 함께 정정. Roadmap/Phase 4.5.md 의 동일 문구 2곳은 4.5 범위 서술이라 정상 — 유지.

README.md:441, 510 의 "sim 단일 task 루프" → "카메라/bag dry-run (sim task 성공률은 v1.5)" — Phase 4.md·부록 B 의 확정 표기와 정합. README.md:511 의 "성공률 표" → "latency/throughput 표".

- [x] **Step 4: 검증**

```bash
grep -rn "2026.06-2027.03" README.md Roadmap/     # 기대: 0건
grep -rn "sim 단일 task 루프" README.md            # 기대: 0건
```

- [x] **Step 5: Commit**

```bash
git add README.md Roadmap/
git commit -m "docs: sync parental leave window, probe schedule, and v1 scope wording"
```

---

### Task 1-7: 자작 팔 확정 반영 (권고 3, 결정 #6)

> 확정 내용의 정본은 [Hardware-Arm.md](../../../Roadmap/Hardware-Arm.md) 하드웨어 확정 절이다 (비채택 사유는 같은 문서 비채택 기록). 아래 Step 은 그 내용을 문서에 반영하는 절차이며, 확정 표를 이 문서에 복제하지 않는다 — 복제본은 결정이 갱신될 때 반드시 어긋난다.

**Files:**
- Modify: `README.md` (Hardware-Arm 절 + BOM 표), `Roadmap/Hardware-Arm.md`

- [x] **Step 1: BOM·구성 반영**

Hardware-Arm.md 의 하드웨어 확정 표 (구성 / 조달 / 비용 / 모터 / 포함·미포함 / 발주 / 근거) 를 채우고, 헤더 예산 줄과 README 자작 팔 절의 확정 표를 같은 내용으로 맞춘다. 커스텀 XL330+XM430 안은 비채택 기록으로 이관한다.

- [x] **Step 2: 미결 사항 명시**

Hardware-Arm.md 의 스파이크 절과 Stage 2 절 앞에 미확정 범위를 주석으로 둔다 — 스파이크 실행 시기는 8월 초 체크포인트, Stage 2 확장 수단은 2026.11 분기 재평가 안건 (§0.1 세 번째 항목의 문서화).

- [x] **Step 3: 검증**

확정 표와 헤더 예산이 Hardware-Arm.md·README 에서 일치하고, 커스텀 XL330+XM430 안과 Koch v1.1 이 비채택 기록에만 남아 있는지 확인한다 (낡은 계획 용법으로 남으면 안 된다).

- [x] **Step 4: Commit**

`README.md` 와 `Roadmap/Hardware-Arm.md` 를 함께 커밋한다.

---

### Task 1-8: Phase 4.5 재설계 — Section 0 전진 배치 (권고 3, 결정 #7)

**Files:**
- Modify: `Roadmap/Phase 4.5.md`, `README.md` (gantt·타임라인·마일스톤·부록 A·B), `Studies/Phase 4/notes.md` (갱신 노트 1줄), `Studies/Phase 4.5/README.md`, `Roadmap/Phase 4.md` (다음 단계 절)

- [x] **Step 1: `Roadmap/Phase 4.5.md` 갱신**

진행 상황 (2026-07-09): 하드웨어 줄의 Colab→RunPod 교체는 Task 1-5 에서 선행 완료. Section 0 확장 시 "ManiSkill/SAPIEN sim 환경 구축" 은 기존 체크 항목이 이미 커버해 중복 추가하지 않고, Docker 컨테이너화 + RunPod 이관 검증 2개만 신설. Section 1-3 은 표가 아니라 절 제목에 시기 병기 (2026.09 / 2026.09-10 / 2026.10-11).

- 헤더 기간 — "약 7-10주 (2026 하반기, v1 레포 기록 직후)" → "**Section 0: 2026.08 (전진 배치)** / Sections 1-3: 2026.09-11". 전진 사유 병기: "GPU (4070) 반납 (2026.08) 전에 sim 환경을 구축·컨테이너화해 로컬 GPU 없이 재현 가능한 상태로 만들어야 한다"
- 하드웨어 줄 — Colab A100/L4 → RunPod RTX 4090 (6곳: 8, 19, 76, 81, 92, 123, 150행. Task 1-5 와 동일 문구)
- Section 0 체크리스트 확장 — 기존 항목 유지 + 추가: `- [ ] ManiSkill/SAPIEN sim 환경 구축 (v1 순서 3 선정 재사용)` / `- [ ] sim + eval 환경 Docker 컨테이너화` / `- [ ] RunPod 에서 컨테이너 기동 + zero-shot 1회 추론 재현 확인 (로컬 GPU 비의존 검증)` — 각각 2026.08 데드라인 명기
- Sections 1-3 표의 시기 열을 2026.09-11 로 갱신

- [x] **Step 2: README 타임라인 동기화**

- README.md:57 (gantt) — `Phase 4.5 (VLA v1.5: LoRA adaptation) :a35, 2026-11, 2M` → 2행: `Phase 4.5 Section 0 (sim+Docker+RunPod) :a35a, 2026-08, 1M` / `Phase 4.5 Sections 1-3 (LoRA+eval) :a35b, 2026-09, 3M`
- README.md:69 — v1.5 공개 마일스톤 `2026-12` → `2026-11` (Sections 1-3 종료 직후)
- README.md:98 (표 행) — "2026.11-12 | Phase 4.5" → "2026.08 (Section 0) + 2026.09-11 (Sections 1-3)"
- README.md:514-518 (마일스톤 "#### 2026.10-12" 절) — Phase 4.5 항목을 Section 0 (2026.08) / Sections 1-3 (2026.09-11) 로 분해, 헤더 기간 표기 조정
- README.md:582 (부록 A) — Phase 4.5 기간 "2026.11-12" → "2026.08-11"
- README.md:600 (부록 B v1.5 행) — 시점 "2026 하반기" 유지, 내용에 Section 0 전진 주석
- Roadmap/Phase 4.md:200 (다음 단계) — "8월 초 체크포인트에서 9-10월 조기 진입 여부 결정" → "Section 0 은 2026.08 전진 확정 (GPU 반납 대비), Sections 1-3 은 2026.09-11"

- [x] **Step 3: `Studies/Phase 4/notes.md` 배치 반영**

notes.md 의 8-12월 예산·배치 노트에 Phase 4.5 배치 (Section 0 2026.08 / Sections 1-3 2026.09-11) 와 그 근거를 반영한다. 배치의 정본은 그 노트이며 이 문서에 복제하지 않는다.

- [x] **Step 4: 검증**

gantt 렌더링 확인 (VS Code 미리보기) + 기간 표기 일관성:

```bash
grep -rn "2026.11-12" README.md Roadmap/    # 기대: Phase 4.5 용법 0건
```

- [x] **Step 5: Commit**

```bash
git add Roadmap/ README.md "Studies/Phase 4/notes.md" "Studies/Phase 4.5/README.md"
git commit -m "docs: frontload phase 4.5 section 0 to august for gpu return deadline"
```

---

### Task 1-9: Phase 3 supporting 공개 전환 (권고 4, 결정 #9)

**Files:**
- Modify: `Roadmap/Phase 3.md`, `README.md` (Phase 3 절·커리어 경로·부록 B·부록 E)

- [x] **Step 1: `Roadmap/Phase 3.md` 지위 전환**

- 제목 "(비공개 리허설)" → "(supporting system work — 보조 엔지니어링 증거)"
- 상태 줄 재작성: "**supporting 증거로 공개** — 대표작 아님. YOLO+Depth 조합은 AMR ROS 5년차 기준 commodity 이므로 헤드라인에 올리지 않고, TensorRT/양자화 배포 + ROS2 통합의 보조 증거로만 공개한다. v1 공개 전까지 README 계층에서 하위 배치 (헤드라인 오독 방지)"
- "공개하지 않는 이유" 절 → "supporting 으로 공개하는 이유와 조건" 으로 재작성. 조건 2개 명기: (1) supporting 라벨 + 하위 배치, (2) 빌드 스크립트 기준 재현 확인 (TensorRT 엔진은 GPU 아키텍처 종속 → 마지막 기회는 4070 반납 전 2026.08 — Task 2-4)
- fallback 예외 (부록 E 승격 공개) 는 유지

- [x] **Step 2: README 정합**

진행 상황 (2026-07-09): 계획 대상 외 추가 발견분도 함께 전환 — README gantt 행, 부록 A 표 행, Roadmap/Phase 3.md 내부 체크리스트·week8 산출 서술 ("비공개 로그 커밋" 2곳). "RT-2 는 비공개 모델" (Roadmap/Phase 4.md) 은 다른 용법이라 유지.

- README.md:96, 179-181, 195 — "비공개 리허설 / 비공개 로그" → "supporting 공개 (보조 엔지니어링 증거)" 로 교체, "velog/LinkedIn 공개 어필 안 함" 취지는 유지
- README.md:438 (커리어 경로) — 동일 교체
- README.md:603 (부록 B "(내부 로그)" 행) — "(supporting)" 으로 교체, 내용 셀에 "supporting system work 로 리포 내 공개, 어필 헤드라인 아님" 명기
- README.md:664, 669 (부록 E) — "비공개 로그" → "supporting 로그" 로 문구 정합 (fallback 시 velog 승격 서술 유지)

- [x] **Step 3: 검증**

```bash
grep -rn "비공개 리허설\|비공개 로그" README.md Roadmap/    # 기대: 0건
```

리마인더 (리포 밖): 이력서·LinkedIn 에서도 Phase 3 를 동일 지위 (supporting) 로 통일.

- [x] **Step 4: Commit**

```bash
git add README.md "Roadmap/Phase 3.md"
git commit -m "docs: publish phase 3 as supporting system work"
```

---

### Task 1-10: v1 정의 업그레이드 + v2.5·ACT·Diffusion·Isaac Lab 편입 (권고 6·8·9, 결정 #10·#11)

**Files:**
- Modify: `Roadmap/Phase 4.md` (핵심 산출물·성공 기준·체크리스트), `README.md` (부록 B·마일스톤), `Roadmap/Hardware-Arm.md` (Stage 1 체크리스트), `Roadmap/Phase 6.md` (진입 전 준비 절 신설)

- [x] **Step 1: v1 정의에 3항목 추가 (`Roadmap/Phase 4.md`)**

"핵심 산출물 (v1)" 과 "산출물 v1" 절에 추가:

```markdown
- **adapter 추상화**: 추론 노드가 모델 구현에 직접 결합되지 않도록 `RobotPolicy` 인터페이스로 분리 (모델 교체 대비 — 2026.11 재평가의 모델 갱신 시나리오 대응)
- **action schema validation**: 출력 7-DoF 의 범위·NaN·급변 검증 레이어 (안전 실행 스택 전체가 아니라 스키마 검증까지 — watchdog/fallback/collision 은 검증 대상 실팔이 생기는 v2 로 이관)
- **벤치마크 재현성**: latency 측정을 스크립트 + 고정 조건 문서 (`Measurements/` 연동) 로 재현 가능하게
```

완료 체크리스트에 대응 항목 3개 추가. README.md:599 (부록 B v1 행) 내용 셀에 "adapter 추상화 + action schema validation + 벤치마크 재현성 포함" 추가.

- [x] **Step 2: 부록 B 에 v2.5 행 신설 (`README.md`)**

v2 행 앞에 v2.5 (데이터 파이프라인 증거) 행을 넣는다 — 자작 팔 teleop 으로 데이터셋 수집 → LeRobot 포맷 + HF Hub 공개 + ACT 1회 학습, Diffusion Policy 는 라잇 정리 수준. 행 문안의 정본은 README 부록 B 이며 이 문서에 복제하지 않는다.

- [x] **Step 3: 실행 지점 연결**

- Roadmap/Hardware-Arm.md Stage 1 완료 체크리스트에 LeRobot ACT 1회 학습 항목 추가 (teleop 소량 데이터, v2.5 선행 실험)
- README.md:521-524 (2027.01-02 마일스톤 절) 에 추가: `- [ ] ACT-Diffusion-VLA 정책 계보 라잇 정리 노트 (면접 방어용, 학습 아님)`

- [x] **Step 4: Isaac Lab 튜토리얼 편입 (`Roadmap/Phase 6.md`)**

진행 상황 (2026-07-09): Phase 6.md 에는 이미 "Section 8.0: 시작 전" 절이 진입 전 준비 역할을 하고 있어, 별도 절 신설 대신 그 절의 첫 항목으로 편입 (외과적 변경). 부수 정정: v2 의 gap 분모 표기 "v1 성공률" → "v1.5(sim) 성공률" (README 부록 B v2 행, Phase 6.md 핵심 메시지 — 성공률 측정이 v1.5 로 이관된 결정과 정합). README 마일스톤에 v2.5 착수 항목도 추가.

주차 표 앞에 "진입 전 준비" 절 신설:

```markdown
## 진입 전 준비 (선행 가능)

- [ ] **NVIDIA SO-101 sim-to-real 코스 Phase A** — 이론 모듈 + 워크숍 코드 읽기 + RunPod headless 로 GR00T post-training·sim 평가 1회 (약 1만-2.5만원, 팔 불요, 2026.08 가능). GUI 가 필요한 teleop·씬 편집과 실물 평가는 Phase B (Stage 1, 2027.01-02) 로 분리한다 — 코스 4개 옵션 전부 실물 팔을 요구한다. 근거·경로: [`docs/research/isaac-sim-so101-course.md`](../../research/isaac-sim-so101-course.md)
```

- [x] **Step 5: 검증**

```bash
grep -n "RobotPolicy" "Roadmap/Phase 4.md" README.md   # 기대: 각 1건 이상
grep -n "v2.5" README.md                                # 기대: 부록 B 행 존재
grep -n "Isaac Lab" "Roadmap/Phase 6.md"                # 기대: 1건 이상
```

- [x] **Step 6: Commit**

```bash
git add "Roadmap/Phase 4.md" "Roadmap/Phase 6.md" Roadmap/Hardware-Arm.md README.md
git commit -m "docs: expand v1 scope and add v2.5 dataset deliverable with act baseline"
```

---

## Phase 2: 본인 실행 항목 (2026.07-08 — 휴직 중 PC 장애 리스크 대비 8월 내 마감)

> 이 Phase 는 문서 편집이 아니라 본인이 수행하는 측정·학습 작업이다. 본 plan 은 통과 기준과 착지 위치만 고정한다. 산출물이 생기는 즉시 `Measurements/` 에 커밋 (보존 정책: 증거 보존이 원복보다 먼저).
> **일상 실행·체크는 본 문서 상단의 §실행 보드가 단일 보드다 (2026-07-09 지정)** — 이 절은 통과 기준의 원본으로만 유지하고, 아래 체크박스는 각 Task 완료 시점에 한 번에 닫는다.

### Task 2-1: 이해 검증 Block 1-2 (2026.07, 약 5-7시간)

**Files:**
- Modify: `Measurements/openvla-rtx4070-int4/findings.md` (본인 작성 영역)

- [x] **Step 1: Block 1 — 메모리 산수 (2-3h)**

7B x 2byte = 14GB 손계산 → int4 예측치 산출 → `torch.cuda.memory_allocated` / `nvidia-smi` 실측 대조 → 차이 원인 (allocator 예약, activation, CUDA context) 을 본인 문장으로 findings.md 에 기록.
**통과 기준**: "왜 int4 가 약 7GB 인가" 를 숫자로 답변 가능.

- [x] **Step 2: Block 2 — 양자화 원리 (3-4h)**

> 본 Step 은 findings.md §4.2 **Block 2 전체**에 대응한다 (findings 안의 Step 1-5 를 모두 포함). 두 문서의 "Step" 은 층위가 다르므로 혼동하지 않는다 — plan 의 Step 2 = Block 2 하나, findings 의 Step 2 = 그 안의 선행 정독 하나.

LLM.int8() 의 outlier 채널 fp16 분해 오버헤드 vs NF4 의 fused kernel — QLoRA·LLM.int8() 논문 해당 절만 정독, findings.md 의 "int8 열위 메커니즘" 영역 작성.
**통과 기준**: int8 이 int4 보다 느리고 부정확한 메커니즘 답변 + "fp16 대비 int4 속도" 방향 예측.

진행 상황 (2026-07-28, 커밋 `b2131ee`): findings §4.2 **Step 1-4 완료**. 통과 기준 두 개 중 **앞쪽 (메커니즘 답변) 충족** — Step 3 경로 대조표 (가중치 트래픽 2배 + activation absmax·threshold 스캔·outlier 분해·출력 dequant 의 매 forward 반복) 과 Step 4 인과 경로 (퍼플렉시티 무손실 vs closed-loop 성공률은 다른 지표라는 구분 + 1.2 Hz 가 궤적 보정 간격을 늘려 실패로 이어지는 경로) 로 답변 가능.

완료 (2026-07-28): Step 5 + 완료 판정 표 작성으로 §4.2 5개 Step 전부 닫힘. 뒤쪽 기준 ("fp16 대비 int4 속도" 방향 예측) 의 답은 **근사 동등** — (a) 트래픽 1/4 의 이득이 decode 7토큰에만 온전히 적용되고 vision encoder·prefill 은 compute 비중이 커서, (b) 전 구간에 붙는 dequant 비용 (특히 prefill 의 fp16 텐서 왕복) 이 이를 상쇄한다.

**통과 판정 유보**: Step 5 와 완료 판정 표는 본인 요청에 따라 `(by claude)` 로 작성됐고 findings 안에 그렇게 표기돼 있다. 산출물은 완성이지만 통과 기준의 "답변" 여부는 Task 2-3 (자가 검증 10문항) 에서 판정한다 — 해당 문항에서 막히면 이 Step 으로 되돌아온다.

부수 산출 2건:
- **A5000 의 bf16 Hz 가 논문에 없다** → int4/bf16 비율은 미확인 종결 (Block 1 Step 5 와 같은 방식). Figure 5 캡션 ("bf16·int4 모두 높은 throughput") 의 정성 진술로 대체하고, §3.5 의 4090 bf16 약 6 Hz 를 A5000 으로 외삽한 값은 본인 외삽임을 명시
- **소재 정정** — 속도 수치는 §5.4 **본문**이고 Table 2 는 성공률·메모리만 담는다. Table 2 캡션의 "4-bit 가 bfloat16 의 성능을 맞춘다" 에서 성능은 성공률이다. int4 3 Hz 를 §1 에 병기하고 §1 미해소 항목을 해소 표시로 갱신 (Global Constraints 의 §1.3 불변 제약과 무관 — findings 내부 서술이다)

- [x] **Step 3: 검증 + Commit**

findings.md 의 Block 1-2 체크박스 닫힘 확인.

검증 (2026-07-28): Block 1 (Step 1-5 + QLoRA 정독 하위) / Block 2 (Step 1-5 + 가정 2건 + 정독 (a)-(e)) 전부 `[x]`, **§4.1-§4.2 범위 미닫힘 0건**. Task 2-1 완료 — 잔여 통과 판정만 Task 2-3 으로 이관 (Step 2 로그 참조).

```bash
git add Measurements/
git commit -m "docs: add memory and quantization analysis to openvla findings"
```

### Task 2-2: 재측정 + Block 3-4 (2026.08 — 약 6-9시간)

**Files:**
- Modify: `Measurements/openvla-rtx4070-int4/methodology.md`, `findings.md`, `summary.csv`, `raw/`, `scripts/`
- Modify (수치 갱신): `Studies/Phase 4/SETUP.md` §1.3, `README.md` 실측 결과 절

> **시점 배치 (2026-07-28 정리)**: 두 Step 의 GPU 필요 여부가 다르므로 실행 보드에서 분리했다 — Step 1 은 #8 (8월 2-3주, GPU 필수), Step 2 는 #8-1 (GPU 불요, 9월 이월 가능). Step 3-4 는 Step 1 과 같은 세션에서 처리한다 (수치 갱신 대상이 Block 3 산출물이므로). 월별 배치 개관은 [실행 가이드](2026-07-28-year-end-execution-guide.md) §3.2 참조.

- [ ] **Step 1: Block 3 — 300ms 해부 (4-6h, GPU 필수)**

추론 경로 (전처리 → vision encoder → action 토큰 7개 autoregressive 생성 → un-normalization) 구간별 프로파일링. `torch.cuda.synchronize` 필요성, warm-up 근거, n=100 분산 검증을 이 과정에서 함께 확인. **예측 → 측정 → 오차 설명** 순서로 수행 — 재측정 스크립트를 `scripts/` 에 저장, raw 갱신, methodology.md 의 미기록 항목 (warm-up, batch, 해상도, 동기화, p50/p99, VRAM peak, 재실행 명령어) 전부 채움.
**통과 기준**: 구간 분해표 작성 + "해상도 절반이면 어디가 얼마나" 예측-측정-오차 설명.

methodology §3 잔여 4항목이 이 Step 에서 함께 닫힌다 (VRAM peak 는 2026-07-10 에 선행 완료): 구간별 분해 / 실카메라 입력 조건 / notebook → 단일 `.py` / `summary.csv`. 부수 산출: 이 Step 완료가 **vla-lab 첫 발행의 트리거**다 (career-sync Task 6).

- [ ] **Step 2: Block 4 — 3.33Hz 의 의미 (2-3h, GPU 불요)**

제어 계층 (전동기 수백Hz-1kHz vs 정책 수Hz) 과 연결, 정적 task 가능 / 동적 추적 불가의 경계, action chunking 이 간극을 메우는 방식 정리.
**통과 기준**: "3.33Hz 면 충분한가" 에 task 유형별 조건부 답변.

입력은 Step 1 의 구간 분해 결과 + §2 의 전체 루프 하한 (2 Hz) + 공개 문헌 (ACT 논문, OpenVLA §5.4) 뿐이고 **새 측정이 없다** (findings §4.4 골격). 따라서 GPU 가 필요 없으며, 과밀 시 Task 2-3 과 함께 9월로 이월한다.

- [ ] **Step 3: 문서 수치 갱신**

재측정 결과로 SETUP.md §1.3 표와 README 실측 결과 절 갱신 (Global Constraints 의 §1.3 불변 제약은 이 Step 에서 해제).

- [ ] **Step 4: 검증 + Commit**

```bash
git add Measurements/ "Studies/Phase 4/SETUP.md" README.md
git commit -m "docs: complete openvla benchmark methodology with remeasured statistics"
```

### Task 2-3: 자가 검증 10문항 (2026.08, 재측정 직후)

- [ ] **Step 1: 셀프 테스트**

보고서 §2.6 의 10문항: int4 근거 / int8 열위 이유 / 정확도 확인 방법 / 병목 구간 / synchronize / warm-up / p95 / action 토큰 구조 / task 경계 / Jetson 이식 예측. 막히는 문항의 해당 Block 재수행.
**통과 기준**: 10문항 전부 문서 참조 없이 답변. (결과는 이 Step 아래 진행 로그로만 기록 — 커밋 없음.)

**이관 사항 (2026-07-28)**: Block 2 의 통과 판정이 이 Step 으로 넘어왔다 — findings §4.2 Step 5 와 완료 판정 표가 `(by claude)` 작성이라 Task 2-1 Step 2 의 "답변 가능" 이 본인 도출로 검증되지 않았다. 관련 문항은 **"int8 열위 이유"** 와 **"Jetson 이식 예측"** (후자는 Block 2 Step 5 의 memory-bound 논리 + Block 3 의 대역폭 외삽이 재료) 이다. 이 두 문항에서 막히면 §4.2 를 본인 문장으로 다시 쓴다.

### Task 2-4: Phase 3 재현 확인 + Rerun 시각 자료 (2026.08)

**Files:**
- Modify: `README.md` (실측 결과 절 gif 삽입), `Portfolio/evidence-index.md`

- [ ] **Step 1: Phase 3 빌드 스크립트 기준 재현 확인**

TensorRT 엔진 재빌드 + week8 통합 노드 1회 구동 (엔진은 GPU 아키텍처 종속이라 4070 에서만 재빌드된다). 결과 (성공 여부, 소요, 걸림돌) 를 Roadmap/Phase 3.md 상태 줄 또는 `Measurements/` 에 기록 — supporting 공개 조건 2 충족.

- [ ] **Step 2: Rerun 시각 자료 1건 확보**

week11-12 dry-run 의 Rerun 스크린샷 또는 짧은 gif 1장 → README 실측 결과 절에 삽입, `Measurements/openvla-rtx4070-int4/plots/` 에 원본 보존.

- [ ] **Step 3: 검증 + Commit**

```bash
git add README.md Measurements/ Portfolio/ "Roadmap/Phase 3.md"
git commit -m "docs: add phase 3 reproduction check and rerun visualization"
```

---

## Verification (after all tasks)

- [x] 잔존 문자열 0 확인 (week 자료 제외 범위): — 통과 (2026-07-09). 예외 1건: SETUP.md §0 의 Colab 배제 사유 기록 (의도적 잔존, Task 1-5 로그 참조)

```bash
grep -rn "Colab" README.md Roadmap/ "Studies/Phase 4/SETUP.md" "Studies/Phase 4/README.md" "Studies/Phase 4.5/SETUP.md" "Studies/Phase 4.5/README.md"
grep -rn "2026.06-2027.03\|비공개 리허설\|박사" README.md Roadmap/
grep -rn "cross-embodiment VLA" README.md Roadmap/
grep -rn "week6/openvla_latency" --include="*.md" .
```

- [x] 링크 무결성: README 실측 결과 절 → `Measurements/`, SETUP.md §1.3 → raw 경로, Roadmap 상호 링크 — 상대 경로 계산 확인 (2026-07-09). 미리보기 육안 확인은 본인 1회 권장
- [x] gantt 2곳 (타임라인, 육아휴직·Phase 4.5 행) Mermaid 문법 확인 (2026-07-09) — 렌더링 육안 확인은 본인 1회 권장
- [x] `Portfolio/evidence-index.md` 의 각 행이 실존 경로를 가리킴 — 확인 (2026-07-09)
- [ ] Phase 2 데드라인 준수 확인: Task 2-2·2-4 가 2026.08 내 (휴직 개시 전) 완료 — 미완 시 2026.11 재평가 #1 에 사유와 함께 등재
- [x] 검토 보고서의 이행 점검 항목 (보고서 §4): 권고 1-4 이행 여부·10문항 통과 여부·probe 2026.09 개시 여부를 2026.11 분기 재평가 #1 안건으로 등재 — README 부록 D 2026.11 행에 추가 완료 (2026-07-09)

## Self-Review

**권고 coverage (보고서 §3 ↔ Task 매핑):**

| 권고 | 대응 Task |
|---|---|
| 1 (실측 즉시 승격) | 1-3 (+ gif 는 2-4) |
| 2 (재측정 + 이해 검증 통합) | 1-2 (구조), 2-1, 2-2, 2-3 (수행) |
| 3 (문서-의사결정 동기화) | 1-5 (컴퓨트), 1-6 (휴직·probe), 1-7 (자작 팔), 1-8 (Phase 4.5) |
| 4 (Phase 3 supporting 공개) | 1-9 (+ 재현 확인은 2-4) |
| 5 (구조 재편 — Measurements 선행분) | 1-2. 전면 재편은 §0.1 로 명시 이월 |
| 6 (v1 정의 업그레이드) | 1-10 Step 1 |
| 7 (포지셔닝 수정) | 1-4 |
| 8 (ACT + 데이터셋 v2.5) | 1-10 Step 2-3 |
| 9 (Isaac Lab 편입) | 1-10 Step 4 |
| §5 보존 정책 | 1-2 (명문화 + 소급 적용 §5.7) |

**Placeholder scan:** 본문에 TBD 없음. methodology.md·findings.md 의 빈 영역은 placeholder 가 아니라 Phase 2 체크박스로 추적되는 본인 작성 영역 (Task 1-2 Step 4-5 에 명시).

**Type consistency:** Task 1-2 가 만드는 경로 (`Measurements/openvla-rtx4070-int4/`) 를 1-3 (링크)·2-1·2-2 (착지점) 가 동일 문자열로 소비. Task 1-5 의 컴퓨트 문구 (RunPod Community Cloud RTX 4090) 를 1-8 이 동일하게 인용. Task 1-9 의 supporting 라벨을 2-4 (재현 확인) 와 1-2 Step 6 (evidence-index 행) 이 공유.
