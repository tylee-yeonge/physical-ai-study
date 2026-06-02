# Phase 4: VLA 논문 reading + OpenVLA → ROS2 minimal demo


> **기간**: 약 4개월 (2026.09-12)
> **목표**: VLA 의 아키텍처 다이어그램을 막힘없이 읽을 수 있는 수준 + OpenVLA inference 를 ROS2 토픽으로 받는 minimal demo
> **언어**: **Python** + **ROS2 (rclpy)**
> **하드웨어**: Ubuntu PC (RTX 4070) — HuggingFace inference / ROS2 노드 / 시각화
> **주간 시간**: 약 6-8시간 (출장 주 보정)


---


## -> **실습 가이드**: `Studies/Phase 4/weekN/` (각 week 별 README + PRACTICE + quiz 미리 작성됨. **진입 시 (2026.09) 다시 체크**)


**핵심 산출물 (#2)**:
- RT-2 + OpenVLA 블로그 2편 (각 1편)
- OpenVLA HuggingFace inference → ROS2 토픽 minimal demo + 1분 영상


**산출물 #2** (2026.12까지 `physical-ai-study` 레포 공개):
- 블로그 2편 (RT-2, OpenVLA 의 아키텍처 / 학습 / 데이터 / inference 흐름)
- ROS2 패키지: OpenVLA inference → `vla_action` 토픽 publish + 시각화


> **선정 논문 (2편)**: RT-2, OpenVLA — 2026.11 분기 재평가에서 π0 / Helix / GR00T 등으로 갱신 가능. 본 마스터는 *시점 기준 OpenVLA 가 표준* 이라는 가정으로 작성.


> **#4 결정타의 예고편**: 본 Phase 의 ROS2 minimal demo 가 Phase 7 의 *Real-to-Sim-to-Real* 산출물 #4 의 토대. 여기서 익힌 inference 파이프라인 + ROS2 통합이 Phase 7 에서 자작 팔과 결합됨.


---


## 학습 환경


| 단계 | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| RT-2 / OpenVLA 논문 정독 | 디바이스 무관 | O |
| (선택) LoRA 파인튜닝 | **Colab A100/L4 (클라우드)** | O |
| HuggingFace inference 셋업 (양자화) | Ubuntu PC (원격) | O |
| ROS2 패키지 작성 | Ubuntu PC (원격) | O |
| 블로그 작성 | 디바이스 무관 | O |


- VRAM 비대칭: OpenVLA 7B 의 BF16 가중치는 약 14-15GB → RTX 4070 12GB 로 **풀 정밀도 추론도 불가**. 4bit 양자화 시 약 6GB 로 축소되어 12GB 안에 안착. LoRA 파인튜닝은 24GB+ 필요 → 4070 으로 사실상 불가.
- **분업 원칙**: 무거운 학습은 Colab, 가벼운 추론은 로컬 4070 + ROS2. Phase 6-7 의 Jetson 배포에서도 동일 패턴이 재사용된다.
- 상세 (의사결정 근거, 버전 매칭, 가중치 전송, 주차별 환경 요구도, 마일스톤 체크리스트):
  - [Studies/Phase 4/SETUP.md](../Studies/Phase%204/SETUP.md) — Phase 4 진입 전 환경 구축 단일 진실 공급원
  - [ENVIRONMENT.md](../ENVIRONMENT.md) — 프로젝트 공용 환경 (장비/원격 접속/시각화)


---


## Section 6.0: 시작 전 (Phase 3 산출물 #1 공개 후)


- [ ] OpenVLA HuggingFace 모델 카드 + 라이선스 확인
- [ ] PC VRAM 점검 (7B 모델 inference 가능성, 4-bit quantization 옵션 확인)
- [ ] 블로그 플랫폼 선정 (Velog / Medium / 본 레포의 `Studies/Phase 4/blog/`)
- [x] **Studies/Phase 4/ week 1-16 자료** 미리 작성됨. 진입 시 (2026.09) 본문 다시 체크.


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
| 6 | OpenVLA HuggingFace 모델 카드 + 환경 셋업 | `transformers` + GPU |
| 7 | 블로그 1편 작성 (OpenVLA) | RT-2 와 비교 + open-source 의 의미 + **action representation 비교 축** (week5 §3.5) |


---


## Section 6.3: OpenVLA HuggingFace → ROS2 minimal demo (5주)


> Brain ↔ Body 의 첫 통합. Phase 7 의 예고편.


| 주차 | 내용 | 핵심 |
|------|------|------|
| 8 | HuggingFace inference 셋업 | 4-bit quantization (필요 시) |
| 9 | inference 입력/출력 인터페이스 정리 + **제어 인터페이스 매핑** | image + instruction → action / 이동 플랫폼이면 매핑이 어떻게 달라지나 |
| 10 | ROS2 패키지 골격 작성 | `vla_node` 노드 + `vla_action` 토픽 |
| 11 | image subscribe → inference → action publish | 단순 동영상 입력으로 dry-run |
| 12 | Rerun 시각화 + 1분 영상 제작 | Demo 영상 마감 |


**ROS2 인터페이스 예시**:
```
Input  : /camera/image_raw (sensor_msgs/Image), /vla/instruction (std_msgs/String)
Output : /vla/action (팔: EE-delta [dx,dy,dz,rx,ry,rz] -> geometry_msgs/Twist + gripper Float64, 또는 custom msg)
```

> 메시지 선택 상세는 `Studies/Phase 4/week9` §1. 같은 `Twist` 가 **이동 플랫폼에서는 base 선속도/각속도**로 의미가 바뀐다 — 팔->이동 이식 시 재매핑 지점이자 부록 D(2026.11) 재평가의 기술적 단서.


---


## Section 6.4: 블로그 마무리 + 패키징 (4주)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 13 | 블로그 2편 퇴고 + 다이어그램 작성 | 면접관 진입점 |
| 14 | ROS2 demo README + 환경 세팅 가이드 | 재현성 |
| 15 | 1분 데모 영상 마감 + 자막 | 패키징 |
| 16 | 산출물 #2 공개 (`physical-ai-study/Portfolio/02_VLA_demo/`) | 2026.12 |


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


### ROS2 minimal demo
- [ ] OpenVLA HuggingFace inference 동작 (PC RTX 4070)
- [ ] `vla_node` ROS2 패키지 빌드 + 실행
- [ ] image subscribe → inference → action publish 흐름 확인
- [ ] 1분 데모 영상 제작


### 산출물 #2 공개
- [ ] `physical-ai-study` 레포 산출물 #2 디렉토리 정리
- [ ] README + 영상 + 블로그 링크


---


## Phase 4 완료 기준


> "RT-2 와 OpenVLA 의 아키텍처를 막힘없이 설명할 수 있고, OpenVLA HuggingFace inference 를 ROS2 토픽으로 받는 minimal demo 를 동작시킬 수 있다. 그리고 action representation 이 실제 제어 인터페이스 및 다른 플랫폼(이동 로봇) 과 어떻게 연결되는지 설명할 수 있다."


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


Phase 4 완료 후 (2026.12):
- **Hardware-Arm Stage 1 본 빌드** (2027.01-02, 스파이크로 디리스크) → 산출물 #2.5
- **초기 패키징 + 실지원 개시** (2027.02~, 트리거: #1 + #2 + 스파이크 확보)
- **Phase 5: Foundation Model 기초** (2027.02-04, 실지원 병행 저강도)
- **6개월 분기 재평가 #1 (2026.11)** — 스파이크 결과 / Phase 4 진행률 / VLA 모델 갱신 검토 / 시장 신호 probe 반응
