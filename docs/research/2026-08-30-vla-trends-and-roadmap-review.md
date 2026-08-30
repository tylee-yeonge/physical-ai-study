# VLA 트렌드 취합 + 로드맵 방향 검토 (2026-08)

> 작성일: 2026-08-30
> 성격: **검토 보고서** (트렌드 취합 + 방향 판단). 원본 문서를 직접 수정하지 않으며, §7 의 갱신 지시 목록이 실제 수정의 입력이다
> 사유: v1 설계 시점 (2026.06) 과 현재 사이에 VLA 필드가 한 세대 넘어갔다 (π0.7 / Gemini Robotics 2 / GR00T N1.7). [README 부록 D](../../README.md) 의 재평가 입력 *"OpenVLA 후속 모델 등장 여부"* 는 조건문의 결과가 이미 나온 상태라, 재평가 #1 (2026.11) 전에 입력을 문서로 정리해 둔다
> 현재 상태: **v1.5 (Phase 4.5) 는 마지막 week (Section 3: eval + before/after 분석 + 블로그) 진행 중.** zero-shot baseline (2026-08-03) 과 RunPod LoRA 1사이클 (2026-08-13, 94분 / $1.18) 은 `Measurements/` 에 기록 완료 — 로드맵 원안 (Sections 1-3 = 2026.09-11) 대비 약 2개월 앞서 있다
> 출처 주의: §1 의 트렌드 항목은 2026-08-30 웹 조사 (랩 블로그·논문·커뮤니티) 기준이다. 개별 수치를 외부 산출물 (블로그 등) 에 인용할 때는 원 출처를 재확인한다

---

## 0. 결론 먼저

1. **v1.5 는 갈아엎지 않는다 — OpenVLA 로 그대로 마감한다.** v1.5 의 성공 기준은 "성공률 상승"이 아니라 "adaptation 파이프라인의 설계-실행-정량 분석"이고 (Phase 4.5 성공 기준 절), 그 기준은 OpenVLA 로 이미 성립했다. 모델이 구세대라는 사실은 둘째 층 증거의 유효성을 깎지 않는다.
2. **모델 교체는 "v1.5 재작업"이 아니라 "v1.6: 동일 하네스 2번째 모델"로 신설한다.** 주 대상 SmolVLA (450M, 로컬 4070 학습 가능), 옵션 GR00T N1.7. ManiSkill 환경·하네스 검증·eval N회/분산 코드·부분 도달률 지표는 전부 모델 무관 자산이라 재사용된다. 규모는 2-3주.
3. **OpenVLA 실측 표 (300ms / 3.33Hz / int4) 는 "비교 baseline"으로 재정의한다.** 단독 표일 때는 한 모델의 수치지만, 같은 4070 에서 action-chunk 계열 (SmolVLA) 수치가 나란히 붙는 순간 "AR VLA vs chunk VLA 를 동일 하드웨어에서 비교한 셋째 층 증거"로 격상된다.
4. **문서 정합성 수정은 v1.5 마감 커밋에 같이 넣는다.** 부록 D 의 낡은 조건문 교체, Roadmap 4개 파일의 Dynamixel 잔재 (확정 하드웨어는 Feetech STS3215), README 실측 표의 "재측정 2026-08 예정" 문구.
5. **Phase 5 커리큘럼 교체·Phase 7 확장 (tool-call 구조, 클라우드 latency) 은 2026.11 재평가 #1 안건으로 올린다.** 지금 로드맵 문서를 대수술하지 않는다 — 재평가 메타룰 (부록 D) 대로 결정 시점에 결정한다.

---

## 1. VLA 트렌드 취합 (2026.08 기준)

### 1.1 프론티어 랩 릴리즈 (2026 상반기~현재)

#### Physical Intelligence π0.7 (2026.04)

- 파인튜닝된 스페셜리스트와 동등한 성능으로 다양한 dexterous task 를 수행하면서, 학습 데이터에 없던 언어 명령·태스크를 수행하고, 여러 태스크의 스킬을 재조합해 새 문제를 푸는 **compositional generalization 의 첫 징후**를 보고.
- 핵심 변화는 **멀티모달 프롬프팅 프레임워크**: (1) 언어 코칭 (실시간 단계별 지시), (2) 경량 월드모델이 생성한 visual subgoal 이미지, (3) 속도/품질/제어 모달리티를 지정하는 strategy metadata 태그. 실패한 자율 롤아웃 같은 suboptimal 데이터에도 메타데이터를 달아 모델 오염 없이 대량 흡수.
- 단일 π0.7 이 에스프레소 제조·박스 조립에서 RL 튜닝된 π*0.6 스페셜리스트와 동등 이상 → **"generalist 가 specialist 를 따라잡았다"**는 신호로 해석됨.
- 배포 노선: 온디바이스가 아니라 **클라우드 호스팅 파운데이션 모델 + 실시간 제어 파이프라인**에 베팅.

#### Google DeepMind Gemini Robotics 2 (2026.07)

- 3단 스택: VLA (Gemini Robotics 2) + 상위 추론 (ER 2) + 온디바이스 경량 VLA (On-Device 2).
- **한 학습 정책으로 휴머노이드의 다리·몸통·팔·손가락을 모두 제어하는 첫 모델.** Apollo 2 (SharpaWave/Inspire 손), Franka Duo (Robotiq 그리퍼) 를 같은 체크포인트로 제어. multi-finger dexterous manipulation 은 여전히 어렵다고 스스로 인정.
- **ER 2 의 agentic 패턴**: VLA 모델·내비게이션 API 같은 저수준 제어 인터페이스를 **tool 로 선언**하고 비디오·오디오·텍스트를 스트리밍 — 수 분간 수백 번의 의사결정이 필요한 긴 태스크 시퀀스 실행. → 본 리포의 Brain↔Body 통합 포지셔닝과 같은 그림 (§5.2).
- 배포: ER 2 만 Google AI Studio 공개, VLA/On-Device 는 early-access 한정.
- 데모 플랫폼에 Dexmate·Trossen·**SO-101** 연구용 로봇 포함.

#### NVIDIA GR00T N1.7 (2026.04, Apache 2.0)

- Cosmos-Reason2-2B (Qwen3-VL 기반) 백본 + 32층 DiT 의 **3B 오픈 VLA**.
- 핵심은 데이터: **EgoScale — 20,854시간의 인간 1인칭 영상 사전학습.** NVIDIA 는 이를 첫 "로봇 dexterity 스케일링 법칙"이라 부르며, 인간 egocentric 데이터를 1,000 → 20,000시간으로 늘리면 평균 태스크 완수율이 2배 이상이 된다고 주장.

#### 중국 생태계

- AGIBOT: 2026.03 1만 번째 로봇 출하, 2026.04 오픈소스 데이터셋 AGIBOT WORLD 2026 + GO-2 (ViLLA 임바디드 파운데이션 모델) 공개.
- LingBot-VLA: 4개 플랫폼 × 100 태스크의 실세계 벤치마크 GM-100 에서 π0.5 / GR00T N1.6 / WALL-OSS 와 비교. ABot-M0 은 6개 오픈 데이터셋 700만+ 궤적으로 사전학습.
- 차별화 축: **실세계 데이터 물량 + 다중 플랫폼 실기 벤치마크.**

### 1.2 논문 트렌드 — 5갈래

#### (a) RL 후처리 + 학습된 월드모델을 시뮬레이터로 (논문 수 최다)

- 문제의식: 실세계 70-80% 성공률을 99% 로 끌어올리는 파인튜닝은 open problem, RL 파인튜닝에 기대가 크지만 표준 방법 부재.
- VLA-RFT (실 상호작용 데이터로 학습한 월드모델을 제어 가능한 시뮬레이터로, 400 스텝 미만 파인튜닝으로 SFT 초과) 이후 WMPO (ICLR'26), RehearseVLA (CVPR'26), World-Gymnast, RISE, VLAW, GigaBrain-0.5M, WoVR, World-VLA-Loop, PlayWorld, VLA-MBPO, ViVa 등 연쇄 출간.
- World-VLA-Loop: RL 중 정책이 바뀌면 고정 월드모델이 어긋나므로, 개선된 정책의 롤아웃을 월드모델 재학습에 피드백하는 closed loop.
- 문제의 이동: **"실기 RL 은 비싸다" → "비디오 월드모델 안에서 RL" → "월드모델 자체의 일반화가 병목".**

#### (b) World Action Model (WAM): 비디오 생성 백본이 VLA 를 대체하는가

- NVIDIA GEAR **DreamZero** (2026.02) 가 용어를 정의. 진단: SOTA VLA 는 의미적 일반화는 잘하지만 새 환경의 unseen 물리 동작 일반화에 약함. 비디오+액션 공동 모델링으로 실기 SOTA VLA 대비 2배 이상 일반화 개선, 14B 자기회귀 비디오 디퓨전을 7Hz 실시간 closed-loop 로 구동, 30분 play 데이터로 새 embodiment few-shot 적응.
- **DreamZero-DROID**: 대규모 로봇 데이터 사전학습 없이 DROID 만으로 학습했는데 MolmoSpaces·RoboArena 1위 — 커뮤니티 최대 화제.
- 같은 계열: mimic-video, Cosmos Policy, LingBot-VA (Causal World Modeling), DreamDojo (44K시간 egocentric).
- 반론도 이미 등장: "WAM 이 VLA 보다 정말 일반화가 나은가" robustness study, Fast-WAM (test-time future imagination 필요성 의문). VLA 와 WAM 은 상호배타적이지 않음.

#### (c) 인간 1인칭 (egocentric) 영상 스케일링

- **EgoScale**: 20,854시간 액션 라벨 egocentric 인간 영상으로 VLA 학습 → 데이터 규모와 validation loss 의 log-linear 스케일링 법칙.
- **HumanScale** (2026.06): 같은 양이면 egocentric 데이터가 실로봇 데이터보다 validation loss 24% 감소, ID/OOD 실기 성공률 52.5%/90% 향상 — **"인간 영상이 텔레옵 로봇 데이터를 능가할 수 있다".**
- 데이터 병목 해법의 이동: "로봇 텔레옵 물량" → "인간 영상 + 소량 로봇 데이터로 액션 공간 정렬". ACE-Ego-0, VITRA (ICRA'26), EgoLive 등.

#### (d) 아키텍처: discrete diffusion, 액션 토크나이저, ECoT

- ICLR 제출작 기준 VLA 논문 수: 2024 년 1편 → 2025 년 9편 → **2026 년 164편** (Moritz Reuss 분석).
- discrete diffusion VLA (dVLA, DIVA, Unified Diffusion VLA): 자기회귀 대비 병렬 생성 — 긴 액션 시퀀스를 몇 번의 forward pass 로, ECoT 의 서브골·추론을 액션과 병렬 생성.
- 새 액션 토크나이저: FASTer (RVQ + DCT 주파수 손실), OmniSAT (B-Spline).
- catastrophic forgetting 회피: 로봇 데이터를 서브태스크/텍스트 액션/중간 계획으로 재라벨링 → VQA 성능 손실 없이 LoRA 만으로 강한 액션 예측.
- **VLM4VLA: 다운스트림 VLA 성능은 VLM 백본의 표준 벤치마크 점수와 상관없음** — Phase 5 커리큘럼 재검토의 직접 근거 (§5.1).

#### (e) Cross-embodiment / 전신 제어

- X-VLA (데이터셋별 soft-prompt 토큰, 스케일링 분석), WholeBodyVLA (ICLR'26), RDT2 (UMI 데이터 스케일업, zero-shot cross-embodiment).
- 산업 수렴: Gemini Robotics 2 의 전신 단일 정책, GR00T N1.6 의 relative action 기본 액션 공간 채택.

### 1.3 오픈소스·커뮤니티 생태계

| 항목 | 내용 |
|---|---|
| **LeRobot = 사실상 표준 허브** | 2026.07 NVIDIA 가 GR00T 1.7 + Isaac Teleop 을 LeRobot 에 통합, Isaac Lab-Arena 를 LeRobot EnvHub 에 등록 — GR00T/Pi/SmolVLA 정책을 한 구조에서 학습·평가. Pi0/Pi0.5/Pi0-FAST/X-VLA/SmolVLA 포팅 완료 |
| **SO-101 = 커뮤니티 기준 하드웨어** | SmolVLA (450M) 는 lerobot 태그 487개 커뮤니티 데이터셋 1,000만 프레임으로 학습. Gemini Robotics 2 데모에도 포함. NVIDIA 공식 SO-101 sim-to-real 코스 (→ [`isaac-sim-so101-course.md`](isaac-sim-so101-course.md)) |
| **평가의 독립화** | RoboArena / MolmoSpaces 같은 독립 운영 zero-shot 벤치마크로 이동. 오랫동안 non-Pi 모델이 크게 뒤처졌으나 2026 년 DreamZero 가 1위를 가져가며 판이 바뀜 |

### 1.4 커뮤니티의 비판적 시각

1. **벤치마크 포화**: LIBERO 는 사실상 풀렸고 99% vs 98% 는 무의미. 잘 튜닝된 Diffusion Policy 로도 VLA 없이 경쟁 가능. CALVIN ABC 4.0+ 가 표준, 4.5+ 가 SOTA 권.
2. **프론티어-학계 격차**: sim 에서는 오픈소스 VLA 가 π0.5 급을 넘지만, 실제 격차는 논문이 거의 평가하지 않는 **사전학습 후 zero-shot open-world 행동**에서 발생. 원인 — 데이터 품질 격차, 좁은 평가 범위, 운영 인력·로봇 플릿 규모, 리뷰어 인센티브.
3. **데이터 품질 연구 부족**: OXE 대부분이 저품질이라는 건 공공연한 비밀인데 모방학습 데이터 품질의 정량화 방법이 없음.
4. **VLA 정의 논쟁**: 인터넷 규모 vision-language 사전학습이 핵심 차별점 (없으면 multimodal policy) vs TRI 의 LBM. WAM 등장으로 경계 더 흐려짐.
5. **sim-only 성과의 평가절하**: 2026 학계 논문 절반 이상이 LIBERO/SIMPLER 위에서 돌고, 실기 결과 없는 sim-only 성과는 평가절하되는 추세. → 소규모라도 SO-101 실기 성공률 (n≥20, unseen object 포함) 확보가 v2.5/v3 의 가치를 결정.

---

## 2. 이 리포에 갖는 의미 — 유지 / 변경

### 2.1 유지되는 것 (트렌드가 오히려 강화)

| 항목 | 판단 |
|---|---|
| **3층 좌표 (셋째 층 핵심 + 둘째 층 방어선)** | 유지. 프론티어가 클라우드 추론 (PI) vs 온디바이스 (Gemini On-Device) 로 갈릴수록 "안 돌아가는 FM 을 돌게 만드는" 셋째 층의 하중은 커진다 |
| **SO-101 선택 (2026-07 확정)** | 유지 — 적중. 커뮤니티 데이터셋, SmolVLA 사전학습, NVIDIA 공식 코스, Gemini Robotics 2 데모까지 전부 SO-101 기준 |
| **자작 팔 우선 / sim-only 지양** | 유지 — §1.4-5 의 커뮤니티 추세와 정확히 일치 |
| **v1.5 성공 기준 (결과 비의존, N·분산·부분 도달률)** | 유지 — LIBERO 포화 논쟁 이후 커뮤니티가 요구하는 보고 형식이 바로 이것 |
| **cross-embodiment 를 표제로 안 쓰는 원칙** | 유지. 단 v1.6 이 생기면 "복수 모델 × 단일 embodiment" 증거는 부산물로 싸게 확보된다 |
| **재평가 메타룰 (부록 D)** | 유지 — 이 문서 자체가 그 룰의 산물 |

### 2.2 바뀌는 것

| 항목 | 기존 | 변경 |
|---|---|---|
| OpenVLA 의 지위 | v1/v1.5/v3 의 척추 | **v1/v1.5 의 완결된 증거 + 비교 baseline.** 2026 논문 베이스라인은 π0.5 / GR00T N1.6-1.7 / X-VLA / SmolVLA 로 이동했고 OpenVLA (2024.06) 는 양자화·토크나이저 연구의 실험 대상으로 남음 |
| 모델 갱신 시점 | 부록 D: "OpenVLA 가 한 세대 뒤 → **2027.05** 재평가 시 갱신 (π0/Helix/GR00T 중 1)" | 조건문의 결과가 이미 나옴. **2026.11 재평가에서 v1.6 (§4) 확정** — 2027.05 까지 기다리면 v1.5 를 한 번 더 만드는 재작업이 된다 |
| 300ms/3.33Hz 실측의 의미 | 단독 수치 | AR 토큰 생성 방식의 구조적 한계 증거. flow-matching action chunk 계열 (SmolVLA/π0) 과의 **동일 하드웨어 비교표의 한 축**으로 격상 |
| Phase 5 커리큘럼 | ViT/CLIP/DINOv2/SigLIP 12주 | 2024 년형. VLM4VLA (백본 벤치마크 점수 ↮ VLA 성능) 이후 면접 질문은 백본이 아니라 **액션 표현** — §5.1 교체안 (재평가 안건) |
| Phase 7 통합 구조 | VLA 단독 ROS2 노드 | ER 2 스타일 "상위 추론이 VLA 를 tool 로 호출"하는 2단 구조가 표준화 중 — §5.2 확장안 (재평가 안건) |

---

## 3. v1.5 마감 방침 (지금 — 마지막 week)

### 3.1 갈아엎지 않는 근거

- v1.5 의 성공 기준은 **"adaptation 파이프라인을 설계-실행하고 결과를 정량 분석했다"** (Phase 4.5 성공 기준 절) — OpenVLA 로 이미 성립. 모델 세대는 증거 유효성과 무관하다.
- 지금 갈아엎으면 잃는 것은 6-8주 산출물, 얻는 것은 같은 형태의 산출물을 더 새 모델로 한 번 더 만드는 것뿐. 주 6-8시간 예산에서 최악의 거래다.
- 실측이 이미 앞서 있다: zero-shot baseline (08-03, 상한 20/20·하한 1/20 하네스 검증 포함) + LoRA 1사이클 (08-13, 94분/$1.18, VRAM 18.5/24GB). 남은 것은 Section 3 (eval N회 + 분석 + 블로그) 뿐이다.

### 3.2 블로그 논지에 추가할 한 단락

> "OpenVLA 를 고른 시점 (2026.06) 과 마감 시점 사이에 π0.7 / GR00T N1.7 / Gemini Robotics 2 가 나왔고, AR 토큰 생성 방식의 300ms 한계는 flow-matching action chunk 계열에서 구조적으로 해소됐다. 다음 단계는 모델을 다시 고르는 것이 아니라 **같은 하네스에 그 모델을 넣는 것**이다."

결점 고백이 아니라 **필드 변화를 읽고 있다는 신호**로 쓴다. adaptation 파이프라인이 모델 비의존으로 설계됐다는 주장의 실증 예고이기도 하다.

### 3.3 v1.5 마감 커밋에 함께 넣을 문서 수정

§7 갱신 지시 목록의 **[지금]** 표시 항목. 로드맵 대수술은 아니고 정합성 수정 + 조건문 결과 반영이다.

---

## 4. v1.6 정의 (신설 제안 — 2026.11 재평가 확정)

### 4.1 정의

> **v1.6: 동일 하네스 2번째 모델.** v1.5 에서 확립한 ManiSkill 환경 + 하네스 검증 + eval N회/분산 + 부분 도달률 지표에 action-chunk 세대 모델을 투입해, OpenVLA 실측과 **동일 조건 비교표** (latency·제어주기·성공률·부분 도달률) 를 만든다. 별도 Phase 문서는 만들지 않는다 — 부록 B 에 한 행 추가하고 실행 정의는 본 문서 §4 를 참조한다.

### 4.2 대상 모델

| | **SmolVLA (주 대상)** | GR00T N1.7 (옵션) |
|---|---|---|
| 크기 / 라이선스 | 450M / Apache 2.0 | 3B / Apache 2.0 |
| 파인튜닝 컴퓨트 | **로컬 4070 12GB 가능 → RunPod 의존 제거** | 24GB 급 → RunPod 유지 |
| 액션 헤드 | flow-matching action chunk | DiT action chunk |
| SO-101 관계 | **사전학습 데이터가 LeRobot 커뮤니티 SO-100/101 데이터셋** → v2.5/v3 에서 zero-shot 이 0% 에 붙지 않을 가능성 | 인간 egocentric 사전학습, cross-embodiment |
| 시장 신호 | HF 생태계 표준, NVIDIA-HF 통합의 3대 정책 중 하나 | NVIDIA 스택 (Isaac Lab-Arena, Jetson) |

SmolVLA 를 주 대상으로 하는 이유: (a) v1.6→v2.5→v3 가 전부 LeRobot 데이터 포맷으로 통일되어 "sim 파이프라인을 real 데이터로 확장"하는 Phase 7 서사가 코드 수준에서 성립, (b) 로컬 학습이 되면 RunPod 리스크 (회수·유휴 과금·**비용의 23% 가 체크포인트 저장**이라는 LoRA 실측 결과) 가 사라짐, (c) 450M 이라 실험 회전이 빠름.

### 4.3 must / nice 절단

"한 구간 메인 1트랙" 원칙과의 충돌을 피하기 위해 v1.6 은 **메인 트랙이 아니다.**

- **must**: LeRobot 데이터 포맷 변환 (RLDS → LeRobot) + SmolVLA 학습 1회 + 동일 하네스 eval 1회
- **nice**: OpenVLA vs SmolVLA 동일 조건 비교표 (README 실측 절 격상) + GR00T N1.7 투입 + README "재측정 2026-08 예정" (p50/p99/VRAM peak) 을 이 비교 측정과 통합해 해소
- **배치**: 2026.10-11, HW 스파이크 (10월, 2-3주) 와 병렬. 스파이크는 하드웨어 대기·디버깅으로 손이 비는 시간이 많아 병렬이 현실적. v1.5 실측이 로드맵보다 약 2개월 앞서 있어 시간 여유는 원 일정 안에 있다
- **재사용 자산 (모델 무관)**: ManiSkill 환경, scripted-solution 하네스 검증, eval N회·분산 코드, 부분 도달률 지표, Docker 구성. **바뀌는 것은 데이터 포맷과 학습 스크립트뿐**

---

## 5. 2026.11 재평가 #1 안건 추가분

### 5.1 Phase 5 커리큘럼 교체안

근거: VLM4VLA (§1.2-d) — 다운스트림 VLA 성능은 VLM 백본 벤치마크 점수와 무상관. 면접 질문의 무게는 백본이 아니라 액션 표현으로 이동. 같은 12주 예산 안에서:

| 구간 | 기존 | 교체안 |
|---|---|---|
| 6주 | ViT (3) + CLIP (3) | **유지**: ViT (3) + SigLIP (3) — 대부분의 VLA 백본이 SigLIP 계열 |
| 6주 | DINOv2 (3) + SigLIP·demo 보강 (3) | **교체**: 액션 토크나이저 (binning → FAST → RVQ) 3주 + flow-matching action expert / action chunking·temporal ensembling 3주. 기존 체크리스트의 "ACT-Diffusion-VLA 계보 노트"를 여기에 흡수 |

"동작 원리 수준, fine-tune 안 함" 원칙은 유지 — 범위 성격은 그대로고 주제만 바뀐다.

### 5.2 Phase 7 확장 (각 1주 이내, 통합 구조라 셋째 층 좌표 유지)

1. **상위 추론 → VLA tool-call 구조**: ROS2 VLA 노드 위에 VLM (API 호출) 플래너를 얹어 "서브태스크 분해 → VLA 실행 → 진행 판정" 1사이클. Gemini ER 2 의 표준 패턴 재현 — §9.1 week 2 (inference 노드 통합) 의 확장으로 1주.
2. **클라우드 추론 vs 로컬 추론 latency 비교**: 같은 모델을 RunPod 서빙 + Tailscale 왕복 latency 실측 → "어느 제어 주기까지 클라우드 추론이 성립하는가". PI 노선 (클라우드) vs 온디바이스 노선의 비교 데이터 — §9.3 week 7 에 한 줄. 반나절 규모.

### 5.3 Phase 6 진입 시 (2027.05) 체크 항목

- Isaac Lab-Arena 가 LeRobot EnvHub 에 등록됨 (2026.07) + NVIDIA 공식 SO-101 sim-to-real 코스 → "Isaac Sim URDF 임포트 자체 구축" 대신 이 경로를 쓰면 디지털 트윈 셋업 주차 (1-7주) 단축 가능성. 지금 바꾸지 않고 진입 시 판단. 상세: [`isaac-sim-so101-course.md`](isaac-sim-so101-course.md)

---

## 6. 추가하지 않을 것 (명시적 non-goal)

| 항목 | 이유 |
|---|---|
| RL 후처리 / 월드모델 시뮬레이터 (§1.2-a) | 논문 수 최다지만 컴퓨트·시간 예산 밖 + 첫째 층 성격. v1.5 블로그의 negative 분석에서 "SFT/LoRA 한계 → 업계가 RL·월드모델로 가는 이유" 한 단락 언급으로 충분 |
| World Action Model (§1.2-b) | 14B 비디오 디퓨전 — 4070 불가 |
| cross-embodiment 표제 승격 | 기존 원칙 유지 (복수 embodiment 실경험 전 표제 금지). v1.6 의 "복수 모델 × 단일 embodiment"는 부산물로만 |
| egocentric 데이터 수집 (§1.2-c) | 트렌드 인지는 하되 실행 편입 안 함 — 데이터 트랙은 v2.5 (SO-101 teleop) 로 충분 |

---

## 7. 원본 문서 갱신 지시 목록

> **[지금]** = v1.5 마감 커밋에 포함. **[재평가]** = 2026.11 재평가 #1 에서 확정 후 반영. 이 목록이 소화되면 본 문서의 해당 절은 역사 기록이 된다.

| # | 시점 | 파일 | 변경 |
|---|---|---|---|
| 1 | [지금] | `README.md` 부록 D (2026.11 행) | "OpenVLA 후속 모델 등장 여부" → **"후속 세대 등장 확인됨 (2026-08 조사, 본 문서) — v1.6 실행 여부·범위 확정"** 으로 교체 |
| 2 | [지금] | `README.md` 부록 D (시그널 매핑) | "OpenVLA 가 한 세대 뒤 → 2027.05 재평가 시점에 모델 갱신 (π0 / Helix / GR00T 중 1)" 삭제 → **"모델 세대 교체 확인됨 → v1.6 (동일 하네스 2번째 모델: SmolVLA 주 대상, GR00T N1.7 옵션) 을 2026.11 재평가에서 확정"** |
| 3 | [지금] | `README.md` 부록 B | v1.6 행 추가 (§4.1 정의, must/nice 는 §4.3). 우선순위 2 (v1.5 와 동급 — 비교표가 핵심 가치) |
| 4 | [지금] | `README.md` 실측 결과 절 | 표 제목/리드에 "비교 baseline" 성격 명시 + "재측정 2026-08 예정" → **"재측정은 v1.6 비교 측정 (2026.10-11) 과 통합"** 으로 교체 |
| 5 | [지금] | `README.md` 541행 재평가 #1 입력 | "OpenVLA 후속 모델 등장 여부" → #1 과 동일하게 교체 |
| 6 | [지금] | `Roadmap/Phase 4.5.md` "다음 단계" 절 | v1.6 항목 추가 (본 문서 §4 참조 링크) |
| 7 | [지금] | `Roadmap/Phase 7.md` 90행 (§9.2 week 4) | "Dynamixel 피드백 활용" → **"Feetech STS3215 피드백 (present current / present load) 활용"** — Hardware-Arm 2026-07 확정 (Koch 비채택) 이전 문서 잔재 |
| 8 | [지금] | `Roadmap/Phase 7.md` 170행 (참고 자료) | "Dynamixel SDK / `dynamixel_hardware`" → **"`feetech_ros2_driver` (ros2_control)"** |
| 9 | [지금] | `Roadmap/Phase 6.md` 97행 | "Dynamixel 토크 피드백 활용" → "STS3215 present load 활용" |
| 10 | [지금] | `Roadmap/Phase 3.md` 153행 · `Phase 4.md` 208행 | "2-DOF Dynamixel + ROS2 파이프라인" → "Feetech 버스 + `feetech_ros2_driver` 파이프라인" (스파이크 정의는 Hardware-Arm.md 가 원본) |
| 11 | [재평가] | `Roadmap/Phase 5.md` | §5.1 커리큘럼 교체 (Section 7.3 DINOv2 → 액션 토크나이저, Section 7.4 → flow-matching/chunking) |
| 12 | [재평가] | `Roadmap/Phase 7.md` §9.1·§9.3 | §5.2 의 tool-call 1주 + 클라우드 latency 항목 |
| 13 | [재평가] | `Roadmap/Phase 6.md` Section 0 | §5.3 의 Isaac Lab-Arena / LeRobot EnvHub 경로 체크 항목 |
| 14 | [재평가] | `README.md` 간트/부록 A | v1.6 확정 시 타임라인 반영 (2026.10-11, 스파이크 병렬) |

---

## 8. 출처

> 2026-08-30 웹 조사 기준. 인용 전 원 출처 재확인.

- Physical Intelligence — π0.7 발표 (physicalintelligence.company 블로그, 2026.04)
- Google DeepMind — Gemini Robotics 2 / ER 2 발표 (2026.07)
- NVIDIA — GR00T N1.7 릴리즈 노트·EgoScale 기술 보고 (2026.04) / GEAR 팀 DreamZero (2026.02) / LeRobot 통합·Isaac Lab-Arena EnvHub (2026.07)
- Moritz Reuss — ICLR 2026 VLA 제출작 분석 (mbreuss 블로그): 논문 수 추이, VLM4VLA, 벤치마크 포화, 프론티어-학계 격차 논평
- HumanScale (2026.06), EgoScale, VLA-RFT, WMPO (ICLR'26), RehearseVLA (CVPR'26), World-VLA-Loop, X-VLA, RDT2, FASTer, OmniSAT — arXiv (제목 검색)
- Hugging Face — SmolVLA 모델 카드 / LeRobot 문서
- AGIBOT — AGIBOT WORLD 2026 · GO-2 발표 (2026.04)
- 리포 내부: [`Measurements/openvla-maniskill-zeroshot/`](../../Measurements/openvla-maniskill-zeroshot/findings.md) · [`Measurements/openvla-lora-runpod/`](../../Measurements/openvla-lora-runpod/findings.md) · [`Roadmap/Phase 4.5.md`](../../Roadmap/Phase%204.5.md) · [README 부록 B/D](../../README.md)
