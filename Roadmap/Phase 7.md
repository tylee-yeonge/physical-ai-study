# Phase 7: Real-to-Sim-to-Real (차별화 정점 산출물 v3)


> **기간**: 약 3개월 (2027.08~, 실지원과 병행하는 강화 카드)
> **목표**: OpenVLA fork + ROS2 노드 래핑 + 자작 6DOF 팔 + 디지털 트윈 (Isaac Sim) + 안전 인터록 + latency 측정 → **산출물 v3 (차별화 정점)**
> **언어**: **Python** + **C++** (안전 인터록) + **ROS2**
> **하드웨어**: Ubuntu PC (RTX 4070) + 자작 팔 (Hardware-Arm Stage 2, 6DOF) + Isaac Sim 디지털 트윈 (Phase 6 산출)
> **주간 시간**: 약 6-8시간 (실지원 병행 저강도)


---


## -> **실습 가이드**: `Studies/Phase 7/weekN/` (각 week 별 README + PRACTICE + quiz 미리 작성됨. **진입 시 (2027.08) 다시 체크**)


**핵심 산출물 (v3, 결정타)**:
> *Real-to-Sim-to-Real*: 자작 6DOF 팔 + Isaac Sim 디지털 트윈 + OpenVLA fork + ROS2 노드 래핑 + 안전 인터록 + latency 측정 + Sim/Real gap 영상


**산출물 v3** (2027.08~ `physical-ai-study` 레포 공개, 완성 시 포트폴리오/이력서 갱신):
- 통합 데모 영상 1-3분
- ROS2 패키지 (OpenVLA inference 노드 + 안전 인터록 노드 + 자작 팔 드라이버 통합)
- latency 측정 보고서 (Sim, Real, 안전 인터록 오버헤드)
- Sim/Real gap 비교 영상


> **"Sim only 산출물은 박사도 만든다. Sim + 자작 실 팔이면 본인만 만든다."**
>
> 본 Phase 가 *"VLA latency 200ms / 안전 메커니즘 / 양산 비용"* 의 직접 증거. 본인 강점 (펌웨어 2.5년 하드웨어 이해 + AMR ROS 실무 5년, 2021.06~) 이 한 점에 수렴.


---


## 학습 환경


| 단계 | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| OpenVLA fork + 환경 셋업 | Ubuntu PC (원격) | O |
| ROS2 노드 통합 | Ubuntu PC (원격) | O |
| 자작 팔 실기 배포 | Ubuntu PC + 자작 팔 | △ (실기 시 로컬) |
| 안전 인터록 (C++) | Ubuntu PC + 자작 팔 | △ |
| 통합 영상 제작 | Ubuntu PC + 자작 팔 + Sim | △ |


- 사전 자산: Phase 4 의 OpenVLA → ROS2 minimal demo, Phase 6 의 Isaac Sim 디지털 트윈, Hardware-Arm Stage 2 의 6DOF 팔 + 안전 인터록 기초
- 상세: [ENVIRONMENT.md](../ENVIRONMENT.md)


---


## Section 9.0: 시작 전 (Phase 6 종료 후)


- [ ] **6개월 분기 재평가 #2 결과 반영** (2027.05) — VLA 모델 결정 (OpenVLA 유지 or π0/Helix 등으로 갱신)
- [ ] 자작 팔 안전성 점검 (Hardware-Arm Stage 2 의 안전 인터록 동작 확인)
- [x] **Studies/Phase 7/ week 1-12 자료** 미리 작성됨. 진입 시 (2027.05) 본문 다시 체크.


---


## Section 9.1: OpenVLA fork + ROS2 통합 (3주)


> **둘째 층 파이프라인 재사용**: 본 절의 fine-tuning 은 처음부터 만드는 게 아니라, **v1.5(Phase 4.5)에서 sim 데이터로 확립한 LoRA adaptation 파이프라인 + eval harness 를 자작 팔 teleop(real) 데이터로 확장**하는 것이다. 즉 v1.5 의 sim adaptation 증거를 real 도메인으로 끌어올려 sim 증거의 설득력 한계(Phase 4.5 §0.3)를 보완한다. v3 는 셋째 층 정점을 유지하되, 둘째 층 파이프라인을 재사용한다.


| 주차 | 내용 | 핵심 |
|------|------|------|
| 1 | OpenVLA fork + Phase 4.5 파이프라인을 자작 팔 teleop 데이터로 확장 | sim→real 데이터 교체, 파이프라인 재사용 (Stage 2 산출) |
| 2 | inference 노드 통합 (Phase 4 demo 확장) | image + instruction → joint action |
| 3 | Sim 환경 (Isaac Sim) 에서 dry-run | 디지털 트윈 활용 |


---


## Section 9.2: 안전 인터록 통합 (3주)


> "안전 메커니즘" (e-stop, 토크 한계, 충돌 감지) 의 직접 증거. C++ 기반 노드.


| 주차 | 내용 | 핵심 |
|------|------|------|
| 4 | 위치 / 속도 / 토크 한계 (C++ 노드) | Dynamixel 피드백 활용 |
| 5 | e-stop + 충돌 감지 | 토크 급증 감지 |
| 6 | inference 출력 → 안전 노드 → 모터 명령 | 인터록 오버헤드 측정 |


---


## Section 9.3: latency 측정 + Sim/Real gap 영상 (3주)


> "VLA latency 200ms" (양산 시점의 차별화 메시지) 의 직접 증거.


| 주차 | 내용 | 핵심 |
|------|------|------|
| 7 | 전체 파이프라인 latency 측정 | image → inference → 안전 → 모터 |
| 8 | Sim 과 Real 의 동일 명령 비교 영상 | 디지털 트윈 활용 |
| 9 | 4가지 gap 정량 (Phase 6 인프라 활용) | latency / 반복성 / force / 시각 |


---


## Section 9.4: 통합 영상 + 패키징 (3주)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 10 | 통합 영상 1-3분 마감 (Sim → Real 흐름) | 면접관 진입점 |
| 11 | ROS2 패키지 정리 + README | 재현성 |
| 12 | 산출물 v3 공개 + 포트폴리오/이력서 갱신 (강화 카드) | 2027.08~ |


---


## Phase 7 완료 체크리스트


### OpenVLA 통합
- [ ] OpenVLA fork + 자작 팔용 인터페이스 정리
- [ ] ROS2 inference 노드 (Phase 4 demo 확장)
- [ ] Sim 환경 dry-run 동작


### 안전 인터록 (C++)
- [ ] 위치 / 속도 / 토크 한계 검증
- [ ] e-stop + 충돌 감지 동작
- [ ] 인터록 오버헤드 측정


### latency / Sim-Real gap
- [ ] 전체 파이프라인 latency 측정 (ms 단위)
- [ ] Sim/Real 동일 명령 비교 영상
- [ ] 4가지 gap 정량 보고서


### 산출물 v3 공개
- [ ] 통합 영상 1-3분
- [ ] ROS2 패키지 + README
- [ ] `physical-ai-study/Portfolio/03_VLA_v3/` 정리


---


## Phase 7 완료 기준


> "자작 6DOF 팔에 OpenVLA fork 를 ROS2 노드로 래핑해 배포하고, 안전 인터록 통과 + Sim/Real gap 측정 + latency 측정 까지 완료한 *Real-to-Sim-to-Real* 통합 영상을 공개한다."


---


## 참고 자료


- OpenVLA: https://openvla.github.io/
- Dynamixel SDK / `dynamixel_hardware`: 자작 팔 트랙 ([Hardware-Arm.md](Hardware-Arm.md)) 참고
- Isaac Sim ROS2 Bridge ([Phase 6.md](Phase%206.md))


---


## [?] 다음 단계


Phase 7 완료 후 (2027 후반~, 실지원은 이미 2027 부터 진행 중):
- **산출물 v3 를 정점 카드로 추가** — 포트폴리오/이력서/영상 갱신, 1순위 3개사 (VLA 모델 직접 개발 코스닥 상장사 / 대기업 SW 자회사 VLA / 신생 휴머노이드 스타트업) 어필 강화
- **실지원 지속** — 분기당 면접 2-3건 목표
- **6개월 분기 재평가 #3 (2027.11)** — 면접 결과 누적 / 2028.03 fallback 진입 여부 (AMR/AV Perception SW) / Jetson 옵션 진입 여부
