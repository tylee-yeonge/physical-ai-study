# Phase 6: Isaac Sim 디지털 트윈 + 자작 팔 → 산출물 v2 (헤드라인)


> **기간**: 약 3개월 (2027.05-07, 실지원 병행 저강도)
> **목표**: 자작 팔 (Hardware-Arm Stage 1/2) 의 URDF 를 Isaac Sim 에 임포트 → 디지털 트윈 + **자작 팔 결합 sim-to-real gap 수치 측정 = 헤드라인 산출물 v2**
> **언어**: **Python** (Isaac Sim API) + **ROS2**
> **하드웨어**: Ubuntu PC (RTX 4070) + 자작 팔 (Hardware-Arm Stage 2, 6DOF)
> **주간 시간**: 약 6-8시간 (실지원 병행 저강도)


---


## -> **실습 가이드**: `Studies/Phase 6/weekN/` (각 week 별 README + PRACTICE + quiz 미리 작성됨. **진입 시 (2027.05) 다시 체크**)


**병행 트랙**: **Hardware-Arm Stage 2 (2027.04-06)** — 6DOF 확장 + teleop + 안전 인터록. URDF / 캘리브 / Sim 물리 파라미터가 본 Phase 와 공유됨.


**핵심 메시지**:
- 본 Phase 는 *Sim only 산출물* 을 만들지 않는다. 자작 팔과 결합해 **sim-to-real gap 을 수치로 측정·보고 = 헤드라인 산출물 v2**. v1(sim) 성공률이 그 gap 의 분모가 된다.
- **adaptation(LoRA)은 본 Phase 가 아니라 v1.5(Phase 4.5)에서 다룬다** — 이전 README 표에 있던 "LoRA 파인튜닝"은 Phase 4.5 로 전진 배치됐고, 본 Phase 는 **둘째 층(adaptation)이 아니라 셋째 층(sim-to-real gap) 에 집중**한다. (README ↔ 본문 정합)
- Sim 만 돌려본 결과물은 차별점이 되지 않는다 — 자작 팔과 결합되어야 본인 강점 (실배포·통합) 이 실린다. (Sim-only 배제 원칙 유지)
- Phase 7(v3, Real-to-Sim-to-Real)은 본 Phase 의 v2 를 6DOF + 안전 인터록 + OpenVLA fork 로 확장한 정점.


---


## 학습 환경


| 단계 | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| Isaac Sim 환경 셋업 | Ubuntu PC (원격) | O |
| URDF 임포트 + 디지털 트윈 | Ubuntu PC + 자작 팔 (캘리브 시 필요) | △ (캘리브는 로컬) |
| Sim/Real gap 측정 인프라 | Ubuntu PC + 자작 팔 | △ |


- Isaac Sim 사양: NVIDIA RTX (Ampere 이상) 필요. RTX 4070 OK.
- 데이터: 자작 팔 teleop 데이터 (Hardware-Arm Stage 2 산출물)
- 상세: [ENVIRONMENT.md](../ENVIRONMENT.md)


---


## Section 8.0: 시작 전 (Phase 5 종료 후)


- [ ] Isaac Sim 4.x 설치 + Workstation 라이선스 확인
- [ ] 자작 팔 URDF 검증 (Hardware-Arm Stage 1 자료 활용)
- [ ] **6개월 분기 재평가 #2 결과 반영** (2027.05 가 본 Phase 종료 시점이라 *진입 시점에는 미반영*. 종료 시 재평가)
- [x] **Studies/Phase 6/ week 1-12 자료** 미리 작성됨. 진입 시 (2027.05) 본문 다시 체크.


---


## Section 8.1: Isaac Sim 환경 셋업 (3주)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 1 | Isaac Sim 4.x 설치 + Workstation 셋업 | Conda 환경 |
| 2 | 기본 시뮬레이션 + USD 이해 | Stage / Prim |
| 3 | ROS2 Bridge 셋업 + Sim → ROS2 토픽 흐름 | omni.isaac.ros2_bridge |


---


## Section 8.2: URDF 임포트 + 디지털 트윈 (4주)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 4 | 자작 팔 URDF → USD 임포트 | URDF Importer |
| 5 | Joint / Link 매핑 + 시각 검증 | Sim 의 Joint State |
| 6 | 자작 팔 Real Joint State ↔ Sim Joint State 매칭 | ROS2 Bridge |
| 7 | 카메라 부착 (자작 팔 ee-mount 또는 외부) | Sim 시각 ↔ Real 시각 |


---


## Section 8.3: Sim/Real gap 측정 인프라 (5주)


| 주차 | 내용 | 핵심 |
|------|------|------|
| 8 | latency 측정 인프라 (Sim, Real, gap) | timestamping 표준화 |
| 9 | 반복성 측정 (같은 명령 → 결과 분산) | Sim 의 결정성 vs Real |
| 10 | force / torque feedback (Real → Sim 비교) | Dynamixel 토크 피드백 활용 |
| 11 | 카메라 이미지 gap (조명, 노이즈) | Domain Randomization 의 시작점 |
| 12 | Sim/Real gap 보고서 + Phase 7 진입 준비 | 인프라 검증 영상 |


---


## Phase 6 완료 체크리스트


### Isaac Sim 환경
- [ ] Isaac Sim 4.x 동작 (Sim Stage 빌드 + 재현)
- [ ] ROS2 Bridge 동작 (Sim ↔ ROS2 토픽)


### 디지털 트윈
- [ ] 자작 팔 URDF → Isaac Sim 임포트
- [ ] Sim Joint State ↔ Real Joint State 매칭 (오차 정량화)
- [ ] 카메라 부착 + Sim/Real 시각 비교


### Sim/Real gap 측정 인프라 → 산출물 v2
- [ ] latency / 반복성 / force / 시각 gap 측정 코드
- [ ] 보고서 + 시각화 (Rerun)
- [ ] **산출물 v2 공개** (자작 팔 + 디지털 트윈 + sim-to-real gap 수치, 헤드라인)


---


## Phase 6 완료 기준


> "자작 팔의 URDF 를 Isaac Sim 에 임포트해 디지털 트윈을 동작시키고, Sim 과 Real 의 4가지 gap (latency / 반복성 / force / 시각) 을 정량 측정해 **산출물 v2 (sim-to-real gap 보고, 헤드라인)** 로 공개할 수 있다."


---


## 참고 자료


- Isaac Sim 공식 문서: https://docs.isaacsim.omniverse.nvidia.com/
- omni.isaac.ros2_bridge: https://docs.omniverse.nvidia.com/isaacsim/latest/ros2_tutorials.html
- URDF Importer: https://docs.omniverse.nvidia.com/isaacsim/latest/advanced_tutorials/tutorial_advanced_import_urdf.html
- Hardware-Arm.md (자작 팔 트랙)


---


## [?] 다음 단계


Phase 6 완료 후 (2027.05):
- **6개월 분기 재평가 #2 (2027.05)** — Phase 5 결과 / Hardware-Arm Stage 2 완성도 / VLA 모델 갱신 검토 / Phase 6 진입 준비도
- **Phase 7: Real-to-Sim-to-Real (2027.08~)** — 정점 산출물 v3 (OpenVLA fork + ROS2 + 자작 6DOF 팔 + 안전 인터록 + latency 측정 + Sim/Real gap 영상)
