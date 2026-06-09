# Phase 4: VLA v1 — OpenVLA zero-shot 추론 + ROS2 → sim 단일 task 루프


> **기간**: 약 4개월 (2026.06-09)
> **목표**: VLA 아키텍처 다이어그램을 막힘없이 읽는 수준 + pretrained OpenVLA zero-shot inference 를 ROS2 토픽으로 받아 **sim 단일 task 루프를 닫는다**(N회 성공률 기록)
> **범위 (v1)**: pretrained zero-shot (**LoRA 파인튜닝은 v2**), **sim embodiment** (자작 팔은 v2), 단일 task, 단일 embodiment
> **언어**: **Python** + **ROS2 (rclpy)**
> **하드웨어**: Ubuntu PC (RTX 4070) — HuggingFace inference / ROS2 노드 / 시각화
> **주간 시간**: 약 6-8시간 (출장 주 보정)


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


> **v2/v3 의 예고편**: 본 Phase 의 ROS2 wrapper + sim 루프가 v2(자작 팔 + sim-to-real gap)·v3(Real-to-Sim-to-Real)의 토대. 여기서 익힌 inference 파이프라인 + ROS2 통합이 자작 팔과 결합됨.


---


## 학습 환경


| 단계 | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| RT-2 / OpenVLA 논문 정독 | 디바이스 무관 | O |
| HuggingFace inference 셋업 (4-bit 양자화) | Ubuntu PC (원격) | O |
| ROS2 패키지 작성 + sim 단일 task 루프 | Ubuntu PC (원격) | O |
| 블로그 작성 | 디바이스 무관 | O |
| (v2) LoRA 파인튜닝 | **Colab A100/L4 (클라우드) — v2 범위** | O |


- **VRAM 비대칭 (컴퓨트 리스크, 미검증)**: OpenVLA 7B 의 BF16 가중치는 약 14-15GB → RTX 4070 12GB 로 **풀 정밀도 추론 불가**. 4-bit 양자화 시 약 6GB 로 축소되어 12GB 안에 안착. LoRA 파인튜닝은 24GB+ 필요 → 4070 으로 사실상 불가(v2 에서 Colab).
- **v1 착수 시 1회 측정 필요**: 4-bit OpenVLA 7B 가 RTX 4070 12GB 에 OOM 없이 올라가는지 + 추론 latency 가 제어 주기에 맞는지. 안 맞으면 범위 축소(관측 → action 예측 → 시각화, 제어 루프 주장 보류)로 분기.
- **분업 원칙**: 무거운 학습(v2 LoRA)은 Colab, 가벼운 추론(v1)은 로컬 4070 + ROS2.
- 상세:
  - [Studies/Phase 4/SETUP.md](../Studies/Phase%204/SETUP.md) — Phase 4 진입 전 환경 구축 단일 진실 공급원
  - [ENVIRONMENT.md](../ENVIRONMENT.md) — 프로젝트 공용 환경


---


## Section 6.0: 시작 전 (Phase 3 week8 마무리 후, 2026.06)


- [ ] OpenVLA HuggingFace 모델 카드 + 라이선스 확인
- [ ] **v1 범위 확정 (필수 선행)**: OpenVLA 가 지원하는 정확한 embodiment/action space 확인 → sim 환경을 거기에 정합 (zero-shot 유효성 확보)
- [ ] **성공 task 1종 + 성공률 기준 N 정의**
- [ ] **컴퓨트 사전 점검**: 4-bit OpenVLA 7B 가 RTX 4070 12GB 에 올라가는지 + latency 1회 측정 (수치로 기록)
- [ ] 블로그 플랫폼 선정 (Velog / Medium / 본 레포의 `Studies/Phase 4/blog/`)
- [x] **Studies/Phase 4/ week 1-16 자료** 미리 작성됨. 진입 시 (2026.06) 본문 다시 체크.


---


## Section 6.1: RT-2 정독 + 블로그 1편 (3주)


> Vision-Language → Action 의 원조 (Google DeepMind, 2023)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 1 | RT-2 논문 1회독 + Architecture Diagram 정독 | PaLI-X / PaLM-E 기반 |
| 2 | Co-fine-tuning + Action tokenization 이해 | "Action 도 토큰" |
| 3 | 블로그 1편 작성 (RT-2) | 핵심 아키텍처 + 데이터 + 한계 |


---


## Section 6.2: OpenVLA 정독 + 블로그 1편 (4주)


> open-source VLA 의 표준 (Stanford, 2024)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 4 | OpenVLA 논문 1회독 + Architecture | Llama 7B + DINOv2 + SigLIP |
| 5 | OpenX-Embodiment 데이터 + Fine-tuning 흐름 + **스키마/action representation 정독** | 970K episodes / observation↔action 페어링 / 관절각·EE-delta·token 비교 |
| 6 | OpenVLA HuggingFace 모델 카드 + 환경 셋업 (4-bit 양자화) | `transformers` + GPU |
| 7 | 블로그 1편 작성 (OpenVLA) | RT-2 와 비교 + open-source 의 의미 + **action representation 비교 축** (week5 §3.5) |


---


## Section 6.3: OpenVLA zero-shot inference → ROS2 → sim 단일 task 루프 (5주, 핵심)


> Brain ↔ Body 의 첫 통합. v2/v3 의 예고편. **Phase 3 week8 통합 노드를 wrapper 스캐폴드로 재사용**.


| 주차 | 내용 | 핵심 |
|------|------|------|
| 8 | HuggingFace inference 셋업 + 컴퓨트 측정 | 4-bit quantization, VRAM/latency 수치 |
| 9 | inference 입력/출력 인터페이스 정리 + **제어 인터페이스 매핑** | image + instruction → action / 이동 플랫폼이면 매핑이 어떻게 달라지나 |
| 10 | ROS2 패키지 골격 작성 (Phase 3 week8 스캐폴드 재사용) | `vla_node` 노드 + `vla_action` 토픽 |
| 11 | sim embodiment 연결 → zero-shot 추론 → 단일 task 루프 | N회 시도 성공률 기록 |
| 12 | Rerun 시각화 + 1분 영상 제작 | Demo 영상 마감 |


**ROS2 인터페이스 예시**:
```
Input  : /camera/image_raw (sensor_msgs/Image), /vla/instruction (std_msgs/String)
Output : /vla/action (팔: EE-delta [dx,dy,dz,rx,ry,rz] -> geometry_msgs/Twist + gripper Float64, 또는 custom msg)
```

> 메시지 선택 상세는 `Studies/Phase 4/week9` §1. 같은 `Twist` 가 **이동 플랫폼에서는 base 선속도/각속도**로 의미가 바뀐다 — 팔->이동 이식 시 재매핑 지점이자 cross-embodiment(부록 D 2026.11 재평가) 의 기술적 단서.


---


## Section 6.4: 블로그 마무리 + 패키징 + v1 공개 (4주)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 13 | 블로그 2편 퇴고 + 다이어그램 작성 | 면접관 진입점 |
| 14 | ROS2 demo README + 환경 세팅 가이드 + 성공률 표 | 재현성 |
| 15 | 1분 데모 영상 마감 + 자막 | 패키징 |
| 16 | **산출물 v1 공개** (`physical-ai-study/Portfolio/01_VLA_v1/` + velog + LinkedIn) | 2026 하반기, 정찰 지원 카드 |


> **공개 글의 논지 3축**: (1) 현 세대 FM 을 배포해 단일 task 루프를 닫음, (2) 양자화 배포 + ROS2 통합을 프로덕션 관점으로 처리(AMR 미들웨어 ROS 5년 경력 연결), (3) perception → VLA → (예정) 자작 팔 궤적. v1 의 Body 가 sim 임을 솔직히 명시, 실암 결합은 v2 예고.


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
- **하드웨어 스파이크** (2026.10, 2-3주) — 2-DOF Dynamixel + ROS2 파이프라인 리스크 검증 (조달은 v1 과 병렬로 이미 착수)
- **정찰 지원** (2026.11-12, 2-3개사, 합격 기대 X, 반응 측정)
- **6개월 분기 재평가 #1 (2026.11)** — 스파이크 결과 / v1 반응 / VLA 모델 갱신 검토 / 시장 신호 probe 반응 / cross-embodiment 좌표 점검
- **Hardware-Arm Stage 1 본 빌드** (2027.01-02) → v2 선행 하드웨어
- **Phase 6 (Isaac Sim 디지털 트윈) + 자작 팔 결합 → 산출물 v2** (헤드라인, sim-to-real gap)
