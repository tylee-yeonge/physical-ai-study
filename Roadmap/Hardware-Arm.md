# Hardware-Arm: 자작 팔 트랙 (스파이크 + 2단계)


> **기간**: 2026.10-2027.06 (스파이크 2-3주 → Stage 1: 2개월 → Stage 2: 3개월). 조달·조립은 v1(sim)과 병렬로 지금 착수해 v2 가 하드웨어 리드타임에 게이트되지 않게 한다.
> **목표**: VLA 통합 엔지니어로서 *Brain ↔ Body 통합* 의 가장 완전한 증거 — 자작 6DOF 팔 + URDF + ROS2 드라이버 + 안전 인터록
> **언어**: **ROS2 (Python/C++)** + **URDF/XACRO**
> **하드웨어**: Dynamixel 6DOF + U2D2 + 3D 프린트 부품 + 카메라 (보유 ELP Stereo 활용)
> **예산**: 약 150-225만 (Dynamixel 풀세트, 보유 3D 프린터 활용)


---


## -> **단계별 가이드**: `Studies/Hardware-Arm/` (Stage 별 단계 문서 — week 단위 아님)
> Stage 1 가이드 + Stage 2 가이드 미리 작성됨. **진입 시 (스파이크: 2026.08, Stage 1: 2027.01, Stage 2: 2027.04) 다시 체크**.


---


## 왜 자작 팔인가


*"Brain ↔ Body 통합 SW 엔지니어"* 의 가장 완전한 증거. 본인 약점 (VLA 신입급) 을 본인 강점 (AMR ROS 실무 5년, 2021.06~ + 펌웨어 2.5년 하드웨어 이해) 으로 직접 깨는 카드.


### 박사·연구생이 못 만드는 결과물 3가지
- **latency**: 추론 → 모터 명령까지 ms 단위 측정
- **안전 메커니즘**: e-stop, 토크 한계, 충돌 감지 직접 구현
- **양산 비용 이해**: DIY 팔 BOM 표 — "이 가격대에 이 성능까지"


### 보너스
- **Dynamixel 제조사이자 휴머노이드 양산 상장사 (2순위 C)** 지원 시 직접 매칭 (모터 제조사 = 회사 자체).
- 펌웨어 직무로 지원하지 않지만 *이해의 폭* 으로 차별화.


---


## BOM 합계 (Dynamixel 풀세트, 보유 3D 프린터 활용)


| 항목 | 권장 | 비용 |
|---|---|---|
| 모터 6 DOF | XL330 (소형, Stage 1) + XM430 (출력, Stage 2 추가) | 100-200만 |
| 컨트롤러 | U2D2 + 전원 (SMPS) | ~10만 |
| 그리퍼 | 단순 2-finger 또는 3D 프린트 | 0-15만 |
| 카메라 | 보유 ELP Stereo 활용 (ee-mount 또는 외부) | 0 |
| 3D 프린터 | 보유 시 그대로 | 0 |
| **합계** | | **약 150-225만** |


**대안 (AR4 저예산, ~50-100만)**: 스테퍼 기반이라 토크 피드백 없음 → "안전 메커니즘" 증거 약화 → **추천 안 함**.


---


## 스파이크 (2026.10, 2-3주, 산출물 아님 = 리스크 검증)


첫 하드웨어는 계획의 2-3배 걸린다. 본 빌드 (Stage 1) 전에 짧게 굴려 파이프라인을 먼저 검증한다.


- **범위**: 2-DOF, 안 예뻐도 됨. Dynamixel 1-2개 + U2D2 + ROS2 토픽으로 각도 명령 한 번 보내기.
- **목표**: "Dynamixel + ROS2 + URDF 파이프라인이 내 환경에서 도는가" 만 확인. pick-and-place 아님.
- **출력**: 터지면 Stage 1/2 일정·BOM 을 **2026 년 안에** 재산정. 분기 재평가 #1 (2026.11) 입력.


---


## Stage 1 (2027.01-02, 2개월, 약 30-50만원 — 스파이크로 디리스크된 본 빌드)


### 목표
- pick-and-place 단순 동작
- URDF + ROS2 드라이버 (`dynamixel_hardware` 패키지)
- Isaac Sim 디지털 트윈 첫 사이클
- **v2 선행 하드웨어**: 동작 영상 + URDF + Sim 임포트 영상 (1분, 2027.02까지)


### 이유
스파이크에서 파이프라인을 이미 검증했으므로 본 빌드는 동작 완성도에 집중. Phase 4 (VLA) 종료 후 단독 진행 — 한 구간 1트랙 원칙.


### 단계
| 월 | 내용 | 산출물 |
|---|---|---|
| 2027.01 전반 | Dynamixel XL330 2-3개 + U2D2 구매 (스파이크 부품 재활용). URDF 작성 + ROS2 드라이버 셋업. | URDF + ROS2 launch |
| 2027.01 후반 | 위치 제어 + pick-and-place 단순 동작. | 동작 dry-run |
| 2027.02 | Isaac Sim URDF 임포트 첫 사이클. 1분 영상 마감. | **v2 선행 하드웨어** |


### Stage 1 완료 체크리스트
- [ ] Dynamixel 2-3DOF 조립 완료 + 동작 확인
- [ ] URDF 작성 + RViz 시각화
- [ ] ROS2 드라이버 빌드 + `ros2 launch` 동작
- [ ] pick-and-place 단순 동작 시연
- [ ] Isaac Sim URDF 임포트 + Sim Joint State 매칭 (첫 사이클)
- [ ] 1분 영상 + v2 선행 하드웨어 디렉토리 정리


---


## Stage 2 (2027.04-06, 3개월, 약 100-150만원 추가 — 6DOF 확장, 실지원 병행)


### 목표
- 6DOF 확장 (Dynamixel XM430 추가)
- teleop 데이터 수집 (leader-follower 또는 PS4 패드)
- 카메라 ↔ 팔 base 캘리브 (Phase 2 자산 활용)
- 안전 인터록 (위치/속도/토크 한계 + e-stop)
- Sim 물리 파라미터 매칭 (Phase 6 토대)


### 출력
- Phase 6 (Isaac Sim) 의 자연스러운 토대
- Phase 7 산출물 v3 의 하드웨어 기반


### 단계
| 월 | 내용 | 산출물 |
|---|---|---|
| 2027.04 | XM430 추가 + 6DOF 확장 + URDF 갱신. RViz 검증. | 6DOF URDF + RViz |
| 2027.05 | teleop 입력 (leader-follower 또는 PS4) + 데이터 수집 파이프라인. 카메라-팔 base 캘리브. | teleop 동작 + 캘리브 결과 |
| 2027.06 | 안전 인터록 (C++ 노드, 위치/속도/토크 한계 + e-stop). Sim 물리 파라미터 매칭. | 안전 인터록 동작 + Sim 매칭 |


### Stage 2 완료 체크리스트
- [ ] 6DOF 조립 + 동작 확인
- [ ] URDF 갱신 + RViz 시각화
- [ ] teleop 입력 동작 (leader-follower 또는 PS4)
- [ ] teleop 데이터 수집 파이프라인 (rosbag 또는 HDF5)
- [ ] 카메라 ↔ 팔 base 캘리브 완료 (재투영 오차 측정)
- [ ] 안전 인터록 (C++) — 위치/속도/토크 한계 + e-stop 동작
- [ ] Sim 물리 파라미터 매칭 (Phase 6 준비도)


---


## 안전 메커니즘 설계 노트


> VLA / 양산 시스템의 "안전 메커니즘 부족" 의 직접 해결 증거. 면접에서 가장 강한 질문 영역.


- **위치 한계**: Joint 별 min/max position 검증. 위반 시 즉시 정지.
- **속도 한계**: rad/s 단위 한계. 위반 시 감속.
- **토크 한계**: Dynamixel 의 *present current* / *present load* 활용. 충돌 감지.
- **e-stop**: 별도 토픽 (`/emergency_stop`) — true 시 모든 모터 즉시 토크 OFF.
- **인터록 노드는 C++** — Python GIL latency 회피.
- **dual-channel**: 정상 명령 채널과 안전 채널 분리. 안전 채널이 우선.


---


## 참고 자료


### 모터 / 컨트롤러
- Dynamixel 공식: https://emanual.robotis.com/
- `dynamixel_hardware` (ROS2): https://github.com/dynamixel-community/dynamixel_hardware


### 오픈소스 자작 팔 디자인 (참고)
- AR4 (스테퍼 기반, 저예산): https://www.anninrobotics.com/
- BCN3D MOVEO: https://github.com/BCN3D/BCN3D-Moveo


### URDF / Isaac Sim 임포트
- URDF Tutorials: https://wiki.ros.org/urdf/Tutorials
- Isaac Sim URDF Importer: ([Phase 6.md](Phase%206.md))


---


## 일정 정렬 (전체 로드맵 내)


| 시점 | 활동 |
|---|---|
| 2026.10 | **스파이크 (2-3주)** — 2-DOF 파이프라인 리스크 검증 (산출물 아님) |
| 2026.11 | 6개월 분기 재평가 #1 (스파이크 결과 + 일정·BOM 재산정) |
| 2026.12 말 | Studies/Hardware-Arm/ Stage 1 가이드 재확인 |
| 2027.01-02 | **Stage 1 본 빌드** — v2 선행 하드웨어 (Phase 4 종료 후 단독) |
| 2027.04-06 | **Stage 2** — Phase 6 와 병행, 실지원 병행 저강도 |
| 2027.05 | 6개월 분기 재평가 #2 (Stage 2 완성도 확인) |
| 2027.08~ | Phase 7 의 산출물 v3 강화 카드 — Stage 2 산출물이 하드웨어 기반 |
