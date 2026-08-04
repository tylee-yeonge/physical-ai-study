# Isaac Sim + SO-101 sim-to-real — 코스 요건·RunPod 실행 가능성·GPU 증설 판정

> NVIDIA 공식 SO-101 sim-to-real 코스의 실제 요건을 조사해, 보유 장비로 어디까지 가능한지와 GPU 증설이 필요한지를 판정한다. Phase 6 진입 전 준비 항목(Isaac Lab 튜토리얼)과 Hardware-Arm Stage 1 의 Isaac Sim 항목이 이 조사에 의존한다.
> 작성일: 2026-08-03

## TL;DR

- 코스는 **4개 옵션 전부 실물 SO-101 을 요구한다.** 팔 없이 완주할 경로는 없다 → 8월에는 완주가 아니라 **선행 학습 + 학습 측 1회 실행**이 목표다
- **학습·sim 평가는 RunPod headless 로 가능**하고 (Isaac Lab NGC 컨테이너가 headless 전용 설계), **teleop·씬 편집 등 GUI 단계는 RunPod 에서 구조적으로 불가**하다 (WebRTC 미디어가 UDP 47998 을 요구, RunPod 은 UDP 미지원)
- 8월 Phase A 비용은 **약 1만-2.5만원** (RunPod GPU 시간 + 볼륨). GPU 증설안 139-208만원의 1/100
- **GPU 증설은 보류한다.** 5070 Ti 16GB 는 Isaac Sim 사양 미달만 해소하고 VLA 파인튜닝 요건(24GB+)은 못 풀어 RunPod 의존이 그대로다. 재판정 조건은 §5.3
- 로컬 4070 PC 는 **Isaac Sim 최소 사양 미달**이다 (VRAM 12 < 16GB, RAM 31 < 32GB, 증설 불가)

## 1. 코스 정체와 모듈 구성

**Train an SO-101 Robot From Sim-to-Real With NVIDIA Isaac** — Intermediate, 6-10시간. 태스크는 vial 을 집어 노란 랙에 넣기. Isaac Lab + Isaac GR00T (N1.6) + LeRobot + Cosmos 를 한 흐름에 묶고, sim-to-real gap 을 줄이는 4가지 전략을 적용한다.

코드는 공개돼 있다 — `isaac-sim/Sim-to-Real-SO-101-Workshop` (teleop/시뮬레이션 컨테이너 + 실물 로봇 컨테이너).

모듈을 **실물 팔 필요 여부**로 가르면 절반 가까이가 팔 없이 접근된다.

| 구간 | 모듈 | 팔 필요 |
|---|---|---|
| 개요 | Overview / How to Take This Course / What Is Sim-to-Real? / LeRobot 배경 | 불요 (이론) |
| 로봇 랩 구축 | Building the Workspace / Get the Code and Models / 캘리브레이션 / 조작 | 필요 |
| 수집·학습·평가 | 도메인 랜덤화 / Isaac GR00T VLA / Sim 평가 / Real 평가 | 필요 (Real 평가 때문) |
| 데이터셋 증강 | co-training with real data / Cosmos 증강 | 불요 (코드만) |
| 액추에이션 갭 | SAGE + GapONet | 필요 |
| 참조 | Quick Reference / Datasets and Models / 비디오 플레이리스트 / 워크숍 코드 | 불요 |

### 1.1 4개 학습 옵션

| 옵션 | 내용 | 실물 팔 | 제공물 |
|---|---|---|---|
| 1. As-Is | 전체 흐름을 가장 빠르게 경험 | 필요 | 사전 수집 데이터셋 + 사전학습 체크포인트 |
| 2. Use Your Own Data | 같은 태스크, 본인 시연 수집 | 필요 | 체크포인트 일부 |
| 3. Bring Your Own Task | 새 태스크 정의 | 필요 | 없음 |
| 4. Going Further | 확장 | 필요 | 없음 |

**NVIDIA 호스팅 컴퓨트(DLI 인스턴스, brev.dev launchable)는 제공되지 않는다.** 학습자가 GPU 를 준비해야 한다.

## 2. 요건 대 보유 장비

| 항목 | Isaac Sim 5.x 최소 | 코스 테스트 사양 | 보유 4070 PC | 판정 |
|---|---|---|---|---|
| VRAM | **16GB** (RTX 4080) | RTX 5090 / RTX PRO 6000 Blackwell / RTX 6000 Ada | **12GB** | 미달 |
| RAM | **32GB** | **64GB** (워크스테이션 구성 125GB) | **31GB** | 미달 (턱걸이) |
| OS | Ubuntu 22.04/24.04 | Ubuntu 24.04 | Ubuntu 24.04.1 | 충족 |
| CPU | 4 코어 | - | i7-14700F, 28 스레드 | 충족 |
| 디스플레이 | - | docker 실행이 `DISPLAY`·X11 전제 | 헤드리스 컨테이너 | GUI 경로 필요 |

NVIDIA 문서의 주석이 판정에 결정적이다 — "VRAM 제약 때문에 최소 사양 미달 카드에서는 일부 튜토리얼·벤치마크가 실행되지 않을 수 있고, **센서를 많이 쓰는 워크플로우가 특히 영향받는다.**" 팔 + 작업대 + 카메라 2대의 디지털 트윈이 정확히 그 경우다.

**하드웨어 증설은 선택지가 아니다** (RAM·GPU 변경 불가). 따라서 로컬 완주는 기대하지 않는다.

## 3. RunPod 실행 가능성

단계별로 갈린다.

| 단계 | RunPod | 근거 |
|---|---|---|
| Isaac Lab 학습 | **가능** | NGC 컨테이너가 headless 전용으로 설계돼 X11 을 요구하지 않는다. 클라우드·HPC 대상이라고 문서에 명시 |
| sim 평가 + 결과 영상 | **가능** | `--headless --video --enable_cameras` 로 mp4 파일 생성. 스트리밍 불필요 |
| GR00T post-training | **조건부 가능** | 기본 설정 ~25GB VRAM. 4090 24GB 는 부족하나 `--no-tune_diffusion_model` / `--lora-rank` 로 내린다. 커뮤니티에 RTX 4080 16GB + batch 16 성공 보고 |
| 대화형 GUI / teleop viewport | **불가** | WebRTC 미디어가 **UDP 47998** 필요, NVIDIA 가 "TCP 만 열면 불충분"이라 명시. RunPod 공식 문서: "Pods do not support UDP connections" |
| 실물 팔 배포 | 불가 | USB 연결 필요 |

**headless 가 "렌더를 안 한다"는 뜻이 아니다.** VLA 는 카메라 이미지를 입력으로 받으므로 렌더는 해야 하고 `--enable_cameras` 가 그것을 켠다. 화면에 띄우지 않을 뿐이므로 GPU 렌더 성능은 여전히 필요하다.

**RunPod 이 로컬 사양 미달까지 우회한다.** 코스 테스트 RAM 이 64-125GB 인데 로컬은 31GB 로 증설 불가다. RunPod 은 RAM 넉넉한 인스턴스를 고를 수 있고 4090 24GB 는 Isaac Sim 최소 VRAM 을 넘는다.

**남는 리스크**: 워크숍 docker 는 `DISPLAY`·X11 을 전제하므로 그 이미지를 그대로 RunPod 에 올리는 것이 아니다. Isaac Lab 의 headless 경로로 옮기는 작업이 필요하고, 이것이 Phase A 에서 처음 부딪힐 지점이다.

## 4. 두 단계 분할

### 4.1 Phase A — 2026.08, 팔 없음, 발화 가능 수준 확보

| 순서 | 내용 | 시간 | 비용 |
|---|---|---|---|
| 1 | 이론 4개 모듈 정독 + 비디오 플레이리스트 | 2-3h | 0원 |
| 2 | 워크숍 레포 코드 읽기 — 컨테이너 구성, teleop/학습/평가 스크립트 분기 | 2-3h | 0원 |
| 3 | RunPod 에서 GR00T post-training 1회 (제공 데이터셋, 축소 설정) | 3-6h | GPU 시간 |
| 4 | RunPod 에서 sim 평가 → `--headless --video` 로 결과 영상 확보 | 1-2h | GPU 시간 |
| 5 | Cosmos·co-training 모듈 (코드만) | 2h | GPU 시간 일부 |

비용 추정:

| 항목 | 추정 |
|---|---|
| RunPod Community Cloud RTX 4090 | 시간당 약 $0.4-0.7 |
| 필요 GPU 시간 (이미지 pull + 학습 + 평가) | 8-12시간 |
| GPU 비용 | $4-9 (약 6천-1.3만원) |
| 볼륨 스토리지 (Isaac Lab + GR00T 이미지 100-150GB, 며칠 유지) | $2-5 |
| **합계** | **약 1만-2.5만원** |

이미지 용량이 커서 **작업을 며칠 안에 몰아서 하고 볼륨을 지우는 것**이 비용 관리의 핵심이다. RunPod 단가는 착수일 실가로 확인한다.

### 4.2 Phase B — 2027.01-02, Stage 1 과 겹쳐 진행

팔 조립 후 캘리브레이션, teleop 수집, Real 평가, 액추에이션 갭 측정을 수행한다. 이때 Isaac Sim GUI 가 필요하면 §2 의 사양 미달을 다시 판정한다.

**실물 단계에는 팔 외에 작업대 자재(vial, 랙 등 코스 BoM)가 필요하다** — 2026.09-10 SO-101 구매 시 함께 잡는다.

### 4.3 Phase A 로 확보되는 발화 수준

- SO-101 sim-to-real 파이프라인을 GR00T post-training 부터 sim 평가까지 돌렸고, teleop 수집과 실물 평가는 팔 확보 시점으로 분리했다
- 로컬 장비가 Isaac Sim 최소 사양 미달인 것을 계산으로 확인하고 학습·평가를 RunPod headless 로 옮겼다. GUI 는 RunPod 의 UDP 미지원으로 이관 불가라는 것도 확인했다
- GR00T 기본 설정이 25GB VRAM 을 요구해 24GB 에서 축소 설정으로 돌렸고, 그 선택이 결과에 무엇을 의미하는지 안다

세 번째가 핵심이다. "코스를 들었다"가 아니라 **"제약을 계산하고 우회 경로를 설계했다"** 로 읽히며, 툴 숙련도 질문이 아니라 엔지니어링 판단 질문을 부른다.

## 5. GPU 증설 판정

### 5.1 검토한 안 (기준 견적: i7-14700F / RTX 4070 12GB / DDR5 16GB x2 / 750W / B760M 스틸레전드 WiFi D5 / 미들타워)

| 안 | 구성 | 추정 금액 |
|---|---|---|
| A. 최소 | RTX 5070 Ti 16GB + DDR5 16GB x2 추가 (총 64GB, 4슬롯) + 기존 750W 유지 | 139만-190만원 |
| B. 권장 | A + 850W ATX 3.1 (네이티브 12V-2x6) | 152만-208만원 |
| C. 깔끔 | RTX 5070 Ti + DDR5 32GB x2 교체 (총 64GB, 2슬롯) + 850W | 178만-258만원 |

단가 추정: 5070 Ti 115만-140만원 (미국 실거래 $899-960, MSRP $749) / DDR5 16GB 12만-25만원 / DDR5 32GB 25만-50만원 / 850W 13만-18만원.

**메모리가 가장 불확실하다** — 2026 Q1 DRAM 계약가가 55-60% 상승하고 연간 110% 상승 보고가 있어, 2024 견적 단가(16GB 56,000원)를 쓸 수 없다. 착수 시 실가 재확인 필요.

### 5.2 무엇을 풀고 무엇을 못 푸는가

| 항목 | 증설 후 | 판정 |
|---|---|---|
| Isaac Sim 최소 VRAM 16GB | 16GB | 해결 (정확히 최소선) |
| Isaac Sim 권장 RAM 64GB | 64GB | 해결 |
| 디지털 트윈 (팔+작업대+카메라 2대) | 가능 전망 | 해결 |
| GR00T post-training 기본 ~25GB | 부족 | 축소 플래그로 우회 (정석 아님) |
| **OpenVLA LoRA 24GB+** | **부족** | **미해결 — RunPod 의존 유지** |

즉 이 지출은 **Isaac Sim/디지털 트윈 진입에만 정확히 대응**하고 학습 쪽 벽은 건드리지 못한다.

실무 함정 3개: (a) B760 계열은 DDR5 4슬롯을 채우면 정격을 못 내 5600 → 4400-5200 으로 내려앉고 로트 혼용 불안정 위험이 있다 (C안이 기술적으로 깔끔), (b) 5070 Ti 는 16핀 전원이고 2024년형 FOCUS GM-750 은 네이티브 12V-2x6 케이블이 없어 GPU 동봉 어댑터가 필요하다, (c) 5070 Ti 300W + i7-14700F PL2 219W → 피크 570-600W 로 750W 정격은 충분하나 NVIDIA 는 고성능 CPU 조합에 850W 를 권한다.

### 5.3 판정: 보류

**지금 증설하지 않는다.** 근거:

1. Phase A 가 1만-2.5만원으로 발화 가능 수준을 확보하므로, 139-208만원은 "트윈을 직접 만든다"는 추가분에만 대응한다
2. 트윈은 Phase 6 (2027.05-07) 항목이고 본격 실지원(2027.03) 이후에 나온다 — 지원 시점 어필 자료로 쓰이지 않는다
3. 휴직 구간(2026.09-2027.02)은 소득이 줄고 SO-101 구매(56-58만원)가 겹치는 시기다
4. 증설로 얻는 24GB 미만 VRAM 은 이미 RunPod 으로 해결된 학습 문제를 중복 해결하지 못한다

**재판정 조건** (하나라도 성립하면 다시 계산):

- **JD 정독 결과 Isaac Sim/Isaac Lab 이 필수 요건으로 확인된다** — 그러면 "발화 가능"으로는 서류가 통과되지 않는다. 이것이 1차 게이트다
- 지원 시점을 2027.05 이후로 미룰 수 있다 (Phase 6 가 지원 전에 들어온다)
- 실물 팔 트랙이 막힌다 (조달·조립 실패) — sim 이 유일한 실물성 증거가 되므로 우선순위가 역전된다
- 복직(2027.03) 으로 소득이 회복된다 — 그 시점에는 5070 Ti 가 아니라 당시 기준 24GB+ 가 후보다

**중간 선택지**: RAM 만 32→64GB (24만-50만원). Isaac Sim 권장을 충족하고 카메라 2대 동시 녹화·데이터 수집·컨테이너 작업 등 실물 트랙에도 직접 이득이다. 단 VRAM 12GB 가 그대로라 이것만으로 Isaac Sim 이 열리지는 않는다.

**선행 확인**: 회사 자산 개조 규정. `ENVIRONMENT.md` 는 하드웨어 변경 불가를 전제하고 있고, 개조가 허용되더라도 반납 시 원상복구를 위해 탈거 부품을 보관해야 한다.

## 6. 이 조사가 확정하지 않은 것

- **워크숍 docker 를 RunPod headless 로 옮기는 실제 난이도.** Isaac Lab NGC 컨테이너가 headless 전용인 것은 문서로 확인됐으나, 워크숍 이미지를 그대로 쓸 수 없다는 것까지만 알고 구체적 이식 비용은 미측정이다
- **GR00T N1.6 의 축소 설정 성능 손실.** `--no-tune_diffusion_model` 사용 시 결과가 얼마나 나빠지는지는 커뮤니티 보고 수준이다
- **국내 부품 실가.** §5.1 은 미국 실거래가와 2024 견적 기반 추정이며 다나와·컴퓨존 확인이 필요하다
- **RunPod 인스턴스의 RAM·디스크 실제 선택 폭과 단가**

## 출처

- 코스: [인덱스](https://docs.nvidia.com/learning/physical-ai/sim-to-real-so-101/latest/index.html) · [How to Take This Course](https://docs.nvidia.com/learning/physical-ai/sim-to-real-so-101/latest/02-how-to-take-this-course.html) · [Isaac GR00T](https://docs.nvidia.com/learning/physical-ai/sim-to-real-so-101/latest/10-groot.html) · [Sim Evaluation](https://docs.nvidia.com/learning/physical-ai/sim-to-real-so-101/latest/11-sim-evaluation.html) · [Real Evaluation](https://docs.nvidia.com/learning/physical-ai/sim-to-real-so-101/latest/12-real-evaluation.html)
- 코드: [isaac-sim/Sim-to-Real-SO-101-Workshop](https://github.com/isaac-sim/Sim-to-Real-SO-101-Workshop)
- Isaac Sim 사양: [Requirements 5.0](https://docs.isaacsim.omniverse.nvidia.com/5.0.0/installation/requirements.html) · [5.1](https://docs.isaacsim.omniverse.nvidia.com/5.1.0/installation/requirements.html) · [Livestream Clients (WebRTC 포트)](https://docs.isaacsim.omniverse.nvidia.com/latest/installation/manual_livestream_clients.html)
- Isaac Lab: [Container Deployment](https://isaac-sim.github.io/IsaacLab/main/source/deployment/index.html) · [Recording video clips during training](https://isaac-sim.github.io/IsaacLab/main/source/how-to/record_video.html)
- GR00T post-training: [Post-Training Isaac GR00T N1.5 for LeRobot SO-101 Arm](https://huggingface.co/blog/nvidia/gr00t-n1-5-so101-tuning)
- RunPod: [Expose ports (UDP 미지원)](https://docs.runpod.io/pods/configuration/expose-ports)
- 부품 가격: [RTX 5070 Ti 가격 추이](https://bestvaluegpu.com/history/new-and-used-rtx-5070-ti-price-history-and-specs/) · [DDR5 가격 상승 2026](https://tech-insider.org/ddr5-ram-prices-2026/) · [RAM 가격 지수](https://www.tomshardware.com/pc-components/ram/ram-price-index-2026-lowest-price-on-ddr5-and-ddr4-memory-of-all-capacities)
