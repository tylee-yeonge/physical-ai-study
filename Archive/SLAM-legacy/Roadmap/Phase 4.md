# Phase 4: VIO 핵심 (Visual-Inertial Odometry)


> **기간**: 3주
> **목표**: IMU와 Vision 융합의 핵심 직관적 이해
> **주간 시간**: 약 7-10시간
> **언어**: **C++** (VINS 코드 읽기 중심)


---


## -> **실습 가이드**: [`Studies/Phase 4/PRACTICE.md`](../Studies/Phase%204/PRACTICE.md)


---


## Week 1: IMU 기초 + 센서 융합 개념


### IMU 센서 이해


#### 가속도계 (Accelerometer)
- [ ] 선형 가속도 측정
- [ ] **중력도 포함됨** (정지 시 약 9.8 m/s^2 측정)
- [ ] 측정값: `a_meas = a_true + g + noise + bias`


#### 자이로스코프 (Gyroscope)
- [ ] 각속도 측정 (rad/s)
- [ ] Drift 발생 (바이어스)
- [ ] 측정값: `omega_meas = omega_true + noise + bias`


### IMU 데이터 특성
- [ ] 높은 주파수: 100-1000Hz (Vision은 20-30Hz)
- [ ] 단기 정확도 높음 (ms 단위)
- [ ] 장기 Drift 발생 (적분 오차 누적)
- [ ] Vision과 상호 보완적


### IMU 노이즈 모델
- [ ] **White Noise**: 랜덤 고주파 노이즈 (sigma_a, sigma_g)
- [ ] **Bias Random Walk**: 천천히 변하는 바이어스 (sigma_ba, sigma_bg)
- [ ] **바이어스 추정이 VIO 성능의 핵심**
- [ ] VINS config의 `acc_n`, `gyr_n`, `acc_w`, `gyr_w`


### 센서 융합 직관
- [ ] 칼만 필터 = 가중 평균: 불확실성 작은 쪽에 더 가중치
- [ ] EKF: 비선형 시스템을 현재 추정값 주변에서 선형화
- [ ] ESKF: 작은 오차는 더 선형적 → 회전 문제 해결


### 자체 점검
1. 가속도계가 정지 상태에서도 값을 출력하는 이유는?
2. IMU 데이터 주파수가 Vision보다 높은 이유는?
3. 칼만 필터에서 공분산이 의미하는 것은?


---


## Week 2: Pre-integration + Factor Graph


> VIO의 핵심 개념. 직관적 이해에 집중.


### Pre-integration 필요성


#### 문제 상황
```
IMU: ████████████████████████████ (200Hz)
Vision: # # # (20Hz)
           KF_i KF_j KF_k
```
- [ ] 키프레임 i→j 사이에 IMU 데이터 수십 개
- [ ] 단순 적분: i의 포즈가 바뀌면 처음부터 재적분 (비효율!)


### Pre-integration 아이디어
- [ ] 포즈에 **독립적인** 상대 측정값 미리 계산
- [ ] i→j 사이의 "상대 변화량" (Delta_p, Delta_v, Delta_q)
- [ ] i 포즈가 바뀌어도 상대 변화량은 유지 → 재적분 불필요


#### 직관적 비유
```
기존: "서울역에서 출발해서 100m 직진, 좌회전, 50m..."
     (서울역 위치 바뀌면 전부 다시 계산)


Pre-integration: "출발점 기준 최종 변위: (150m, 30도 회전)"
                (출발점 바뀌어도 상대 변위는 그대로)
```


### 바이어스 보정 (1차 근사)
- [ ] 바이어스가 조금 변하면 자코비안으로 1차 보정
- [ ] `Delta_p_corrected ≈ Delta_p + J_p * delta_b`
- [ ] 재적분 없이 빠르게 업데이트


### Factor Graph에서의 역할


```
[KF_0]--IMU--[KF_1]--IMU--[KF_2]--IMU--[KF_3]
  | | | |
  +--Visual---+--Visual---+--Visual---+
              \ /
               [3D Point]
```


- [ ] **IMU Factor**: Pre-integrated measurement로 연속 키프레임 연결
- [ ] **Visual Factor**: 재투영 오차로 카메라 포즈와 3D 점 연결
- [ ] 그래프 최적화 = 모든 팩터 오차 최소화


### VINS 코드 연결
- [ ] `integration_base.h`: Pre-integration 클래스
- [ ] `imu_factor.h`: IMU Factor 정의
- [ ] `Evaluate()`: Factor 오차 계산


### 자체 점검
1. Pre-integration이 없으면 왜 비효율적인가?
2. Delta_p, Delta_v, Delta_q가 "포즈 독립적"이라는 게 무슨 의미인가?
3. 바이어스가 변할 때 왜 재적분 없이 보정 가능한가?


---


## Week 3: VIO 초기화 + Camera-IMU 캘리브레이션


### VIO 초기화


#### 시작 시 모르는 것들
- [ ] 스케일 (Monocular)
- [ ] 중력 방향 (어느 쪽이 "아래"인지)
- [ ] IMU 바이어스
- [ ] 초기 속도


#### 초기화 과정
1. **Vision-only SfM**: 순수 VO로 up-to-scale 궤적 추정
2. **Visual-Inertial Alignment**: Vision 궤적과 IMU pre-integration 정렬
   - 스케일 s, 중력 방향 g, 속도 v, 자이로 바이어스 b_g 추정
3. **검증 및 정제**: Reprojection error, IMU-Vision 일관성 확인


#### VINS 코드 연결
- [ ] `initial_sfm.cpp`: Vision-only 초기화
- [ ] `initial_alignment.cpp`: Visual-Inertial 정렬


### Camera-IMU 캘리브레이션


#### Extrinsic Calibration
```
Camera Frame ←[R, t]→ IMU Frame
```
- [ ] Camera와 IMU 사이의 상대 포즈 (R, t)
- [ ] 잘못된 extrinsic → VIO 발산
- [ ] 시간 동기화 (td): Camera-IMU 타임스탬프 차이


#### Kalibr (개념)
- [ ] Camera-IMU 캘리브레이션 표준 도구
- [ ] AprilGrid 타겟, 다양한 축 회전 필요
- [ ] 결과를 VINS config에 반영


### 자체 점검
1. VIO 초기화에서 스케일을 어떻게 알아내는가?
2. 중력 방향을 왜 추정해야 하는가?
3. Camera-IMU extrinsic이 틀리면 어떤 현상이 나타나는가?


---


## Phase 4 완료 체크리스트


### IMU 기초
- [ ] IMU 센서 동작 원리 이해
- [ ] 노이즈, 바이어스 개념 이해


### Pre-integration (핵심!)
- [ ] 왜 Pre-integration이 필요한지 **명확히** 설명 가능
- [ ] Delta_p, Delta_v, Delta_q의 의미 이해
- [ ] Factor Graph에서의 역할 이해


### 초기화 & 캘리브레이션
- [ ] VIO 초기화 과정 개념 이해
- [ ] Camera-IMU extrinsic 중요성 이해


---


## Phase 4 완료 기준


> "IMU와 Vision이 왜 상호보완적인지, Pre-integration 개념을 직관적으로 설명 가능"


---


## 참고 자료


### 논문 (필독)


| 논문 | 용도 | 우선순위 |
|------|------|---------|
| VINS-Mono (Qin et al., 2018) | VIO 전체 | |
| On-Manifold Preintegration (Forster et al., 2017) | Pre-integration 상세 | |


### 책


| 책 | 용도 |
|------|------|
| State Estimation for Robotics (Barfoot) | EKF, Factor Graph, Lie 군 |
| Probabilistic Robotics (Thrun) | 칼만 필터 기초 |


### 강의


| 강의 | 용도 |
|------|------|
| Cyrill Stachniss - Factor Graphs | Factor Graph 개념 |
| Joan Sola - Quaternion Kinematics | 회전, IMU 적분 |


---


## 팁


1. **수식에 겁먹지 말기**: 직관적 이해가 먼저, 수식은 나중에
2. **논문 반복 읽기**: VINS 논문을 3번 이상 읽기
3. **그림으로 이해**: Factor Graph를 직접 그려보기
4. **코드와 연결**: 개념 이해되면 VINS 코드에서 확인


---


## [?] 다음 단계


Phase 4 완료 후:
- Phase 5 (Detection + Depth Estimation)로 진행
- SLAM 기초 위에 AI Perception 역량을 쌓는 단계로 전환
