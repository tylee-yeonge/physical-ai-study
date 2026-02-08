# Phase 4: VIO 핵심 (Visual-Inertial Odometry)

> ⏰ **기간**: 약 3개월 (12-14주 분량)
> 🎯 **목표**: IMU와 Vision 융합의 핵심 이해
> ⭐ **중요도**: 이 Phase가 AMR VIO의 핵심입니다
> ⏱️ **주간 시간**: 약 10시간
> 💻 **언어**: **C++** (Eigen, Sophus, Ceres)
>

---

## 👉 **실습 가이드**: [`Studies/Phase 4/PRACTICE.md`](../Studies/Phase%204/PRACTICE.md)

**주요 실습**:
- IMU Pre-integration 클래스 구현 (C++)
- Sophus SO(3) Lie 대수 활용
- Ceres로 VIO Factor Graph 최적화
- EuRoC 데이터셋으로 검증

> ⚠️ **필수 C++**: VINS `imu_factor.cpp`, `integration_base.h` 분석 대비

## ⚠️ 시작 전 주의사항

이 Phase는 **가장 어렵고 중요한** 단계입니다.

- Pre-integration은 예상보다 오래 걸릴 수 있음
- 막히면 **혼자 끙끙대지 말고** 커뮤니티 질문 활용
- 완벽한 수식 이해보다 **직관적 이해** 우선
- 막힌 부분은 표시해두고 다음으로, 코드 분석하며 나머지 채우기

---

## 📋 Section 4.1: IMU 기초 (1주)

### Week 1: IMU 센서 이해

#### 센서 종류
- [ ] **가속도계 (Accelerometer)**
  - 선형 가속도 측정
  - ⚠️ **중력도 포함됨** (정지 시 약 9.8 m/s² 측정)
  - 측정값: a_meas = a_true + g + noise + bias

- [ ] **자이로스코프 (Gyroscope)**
  - 각속도 측정 (rad/s)
  - Drift 발생 (바이어스)
  - 측정값: ω_meas = ω_true + noise + bias

#### IMU 데이터 특성
- [ ] 높은 주파수: 100-1000Hz (Vision은 20-30Hz)
- [ ] 단기 정확도 높음 (ms 단위)
- [ ] 장기 Drift 발생 (적분 오차 누적)
- [ ] Vision과 상호 보완적

#### SLAM에서 어디에 쓰이나?
- [ ] Vision이 느릴 때 (프레임 사이) 모션 예측
- [ ] 빠른 움직임에서 Vision 보완
- [ ] 스케일 복구의 핵심
- [ ] VINS `imu_callback()`에서 데이터 수신

#### 실습: IMU 데이터 시각화
- [ ] ROS2에서 IMU 토픽 구독 (`sensor_msgs/Imu`)
- [ ] 가속도계, 자이로 데이터 플롯
- [ ] 정지 상태에서 중력 확인 (~9.8 m/s²)
- [ ] 움직임에 따른 변화 관찰

### Week 2: IMU 노이즈 모델

#### 노이즈 유형
- [ ] **White Noise (측정 노이즈)**
  - 랜덤한 고주파 노이즈
  - 적분하면 Random Walk로 변환
  - 파라미터: σ_a, σ_g

- [ ] **Bias Random Walk**
  - 천천히 변하는 바이어스
  - 온도, 시간에 따라 drift
  - 파라미터: σ_ba, σ_bg

#### 바이어스 (Bias)
- [ ] 가속도계 바이어스 (b_a): 약 0.01-0.1 m/s²
- [ ] 자이로 바이어스 (b_g): 약 0.001-0.01 rad/s
- [ ] **바이어스 추정이 VIO 성능의 핵심**
- [ ] VINS에서 바이어스를 상태 벡터에 포함

#### Allan Variance (개념만)
- [ ] 센서 노이즈 파라미터 측정 방법
- [ ] IMU 데이터시트에서 noise density 찾기
- [ ] VINS config 파일의 `acc_n`, `gyr_n`, `acc_w`, `gyr_w`

#### VINS 코드 연결
- [ ] `parameters.cpp`: IMU 노이즈 파라미터 로드
- [ ] config 파일의 IMU 관련 파라미터 확인

### 🔍 Section 4.1 자체 점검
1. 가속도계가 정지 상태에서도 값을 출력하는 이유는?
2. 자이로 바이어스가 있으면 적분 결과에 어떤 영향?
3. IMU 데이터 주파수가 Vision보다 높은 이유는?

---

## 📋 Section 4.2: 센서 융합 기초 (3주)

### Week 3: 칼만 필터 복습

#### 기본 칼만 필터 직관
- [ ] **예측 (Prediction)**: 모델로 다음 상태 예측
- [ ] **업데이트 (Update)**: 측정값으로 보정
- [ ] **칼만 게인**: 예측과 측정 중 뭘 더 믿을지
- [ ] **공분산**: 불확실성 표현

#### 칼만 필터 = 가중 평균
```
최적 추정 = (예측 × 측정_신뢰도 + 측정 × 예측_신뢰도) / (예측_신뢰도 + 측정_신뢰도)
```
- [ ] 불확실성 작은 쪽에 더 가중치
- [ ] 두 정보원의 최적 융합

#### 실습: 1D 칼만 필터
- [ ] 위치 추정 예제 구현 (Python)
- [ ] 노이즈가 있는 측정값 필터링
- [ ] 공분산 변화 시각화

### Week 4: EKF

#### EKF 필요성
- [ ] 실제 시스템은 비선형 (회전, IMU 적분 등)
- [ ] 선형 칼만 필터 적용 불가
- [ ] 해결: 현재 추정값 주변에서 선형화

#### EKF 단계
- [ ] **예측**: 비선형 상태 전이 f(x)
- [ ] **자코비안 F**: ∂f/∂x로 공분산 전파
- [ ] **업데이트**: 비선형 측정 모델 h(x)
- [ ] **자코비안 H**: ∂h/∂x로 칼만 게인 계산

#### EKF의 한계
- [ ] 강한 비선형에서 선형화 오차 큼
- [ ] 큰 불확실성에서 부정확
- [ ] 회전 표현 문제 (쿼터니언 정규화)

### Week 5: ESKF

#### 왜 Error-State인가?
- [ ] 회전의 over-parameterization 문제
- [ ] 작은 오차는 더 선형적
- [ ] 많은 VIO 시스템이 ESKF 기반

#### ESKF 구조
```
Full State = Nominal State ⊕ Error State
```
- [ ] **Nominal state**: IMU로 적분 (비선형)
- [ ] **Error state**: 작은 오차 추정 (선형)
- [ ] **Reset**: Error를 Nominal에 반영 후 Error 초기화

#### SLAM에서 어디에 쓰이나?
- [ ] MSCKF: ESKF 기반 VIO
- [ ] VINS: 최적화 기반이지만 개념 유사
- [ ] 많은 산업용 VIO가 ESKF 사용

### 🔍 Section 4.2 자체 점검
1. EKF에서 자코비안을 쓰는 이유는?
2. Error-State가 Full-State보다 나은 점은?
3. 칼만 필터에서 공분산이 의미하는 것은?

---

## 📋 Section 4.3: IMU Pre-integration (5주) ⭐

> ⚠️ **VIO의 핵심 개념입니다. 충분한 시간을 투자하세요.**  
> 💡 막히면 SLAM KR 커뮤니티나 Claude에게 질문하세요!

### Week 6: Pre-integration 필요성 이해

#### 문제 상황
```
IMU: ████████████████████████████ (200Hz)
Vision:     ■         ■         ■ (20Hz)
           KF_i      KF_j      KF_k
```
- [ ] IMU는 고주파, Vision은 저주파
- [ ] 키프레임 i→j 사이에 IMU 데이터 수십 개
- [ ] 이걸 어떻게 활용할 것인가?

#### 단순한 방식의 문제
1. 키프레임 i의 포즈에서 시작
2. IMU를 순차적으로 적분하여 j 예측
3. **문제**: 최적화로 i의 포즈가 바뀌면?
4. → 다시 처음부터 적분해야 함 (비효율!)

#### Pre-integration 아이디어
- [ ] 포즈에 **독립적인** 상대 측정값 미리 계산
- [ ] i→j 사이의 "상대 변화량" (Δp, Δv, Δq)
- [ ] i 포즈가 바뀌어도 상대 변화량은 유지
- [ ] 재적분 불필요!

#### 직관적 비유
```
기존: "서울역에서 출발해서 100m 직진, 좌회전, 50m..."
     (서울역 위치 바뀌면 전부 다시 계산)

Pre-integration: "출발점 기준 최종 변위: (150m, 30도 회전)"
                (출발점 바뀌어도 상대 변위는 그대로)
```

### Week 7: Pre-integration 수식 이해 (단계별)

#### Step 1: IMU 측정 모델
- [ ] 가속도계: `a_m = R^T(a - g) + b_a + n_a`
- [ ] 자이로: `ω_m = ω + b_g + n_g`
- [ ] R: 월드→바디 회전, g: 중력

#### Step 2: 연속 시간 적분 (개념)
```
위치: p(t) = 이중적분(가속도)
속도: v(t) = 적분(가속도)
회전: R(t) = 적분(각속도)
```

#### Step 3: Pre-integrated 측정값
```
Δp_ij = i 프레임 기준 j까지의 상대 위치
Δv_ij = i 프레임 기준 j에서의 상대 속도
Δq_ij = i 프레임 기준 j까지의 상대 회전
```
- [ ] **핵심**: 이 값들은 i, j의 **절대 포즈와 무관**

#### Step 4: 바이어스 보정 (1차 근사)
- [ ] 바이어스가 조금 변하면?
- [ ] 자코비안으로 1차 보정: `Δp_corrected ≈ Δp + J_p * δb`
- [ ] 재적분 없이 빠르게 업데이트

### Week 8: Factor Graph에서의 역할

#### Factor Graph 복습
- [ ] **변수 노드**: 추정할 것 (포즈, 속도, 바이어스)
- [ ] **팩터 노드**: 제약 조건 (측정값)
- [ ] 그래프 최적화 = 모든 팩터 오차 최소화

#### IMU Factor
```
[KF_i] ----[IMU Factor]---- [KF_j]
  |                            |
(p,v,R,b)_i              (p,v,R,b)_j
```
- [ ] Pre-integrated measurement (Δp, Δv, Δq)를 Factor로
- [ ] 연속 키프레임을 연결
- [ ] 공분산 (불확실성) 포함

#### Visual Factor
```
[KF_i] ----[Visual Factor]---- [3D Point]
           (재투영 오차)
```
- [ ] 재투영 오차를 Factor로
- [ ] 카메라 포즈와 3D 점 연결

#### 전체 그래프
```
[KF_0]--IMU--[KF_1]--IMU--[KF_2]--IMU--[KF_3]
  |           |           |           |
  +--Visual---+--Visual---+--Visual---+
              \          /
               [3D Point]
```

### Week 9: Pre-integration 심화

#### 공분산 전파
- [ ] Pre-integration 중 불확실성도 누적
- [ ] 공분산 행렬 Σ_ij 계산
- [ ] Factor의 정보 행렬 = Σ^(-1)

#### On-Manifold Pre-integration (선택적 심화)
- [ ] 회전의 SO(3) 구조 활용
- [ ] Forster et al. 논문 참고
- [ ] 지금은 개념만 이해해도 OK

#### VINS 코드 연결
- [ ] `integration_base.h`: Pre-integration 클래스
- [ ] `imu_factor.h`: IMU Factor 정의
- [ ] `Evaluate()`: Factor 오차 계산

### Week 10: 실습 - 간단한 IMU 적분

#### 목표
- [ ] Pre-integration 전에 기본 IMU 적분 이해

#### 구현 (Python/C++)
1. [ ] EuRoC 데이터셋에서 IMU 데이터 로드
2. [ ] 단순 적분으로 궤적 추정
   ```python
   velocity += (acceleration - gravity) * dt
   position += velocity * dt
   orientation = integrate_gyro(orientation, gyro, dt)
   ```
3. [ ] Ground truth와 비교
4. [ ] **드리프트 관찰** — 몇 초 만에 발산

#### 배우는 것
- [ ] IMU만으로는 부족한 이유 체감
- [ ] 바이어스 영향 확인
- [ ] Vision이 왜 필요한지 동기 부여

### 🔍 Section 4.3 자체 점검
1. Pre-integration이 없으면 왜 비효율적인가?
2. Δp, Δv, Δq가 "포즈 독립적"이라는 게 무슨 의미인가?
3. 바이어스가 변할 때 왜 재적분 없이 보정 가능한가?

---

## 📋 Section 4.4: VIO 초기화 (2주)

### Week 11: 초기화 문제

#### 왜 초기화가 어려운가
VIO 시작 시 모르는 것들:
- [ ] 스케일 (Monocular)
- [ ] 중력 방향 (어느 쪽이 "아래"인지)
- [ ] IMU 바이어스
- [ ] 초기 속도

#### 초기화에서 추정할 것
```
[추정 대상]
- 중력 방향 (world frame에서)
- 스케일 s (vision 궤적의 실제 크기)
- 초기 속도 v_0
- 자이로 바이어스 b_g
```

#### SLAM에서 왜 중요한가
- [ ] 초기화 실패 → VIO 전체 실패
- [ ] 잘못된 초기화 → 발산
- [ ] VINS는 robust한 초기화 제공

### Week 12: 초기화 과정

#### Stage 1: Vision-only SfM
- [ ] 순수 Visual Odometry로 시작
- [ ] Feature tracking으로 초기 맵 생성
- [ ] Up-to-scale 궤적 추정

#### Stage 2: Visual-Inertial Alignment
- [ ] Vision 궤적과 IMU pre-integration 정렬
- [ ] 풀어야 할 것:
  - 스케일 s
  - 중력 방향 g
  - 속도 v
  - 자이로 바이어스 b_g
- [ ] 선형 시스템으로 풀기 (closed-form)

#### Stage 3: 검증 및 정제
- [ ] Reprojection error 확인
- [ ] IMU-Vision 일관성 확인
- [ ] 필요시 비선형 최적화로 정제

#### VINS 코드 연결
- [ ] `initial_sfm.cpp`: Vision-only 초기화
- [ ] `initial_alignment.cpp`: Visual-Inertial 정렬
- [ ] `initial_ex_rotation.cpp`: Extrinsic 초기 추정

### 🔍 Section 4.4 자체 점검
1. VIO 초기화에서 스케일을 어떻게 알아내는가?
2. 중력 방향을 왜 추정해야 하는가?
3. 자이로 바이어스는 왜 먼저 추정하고, 가속도계 바이어스는 나중인가?

---

## 📋 Section 4.5: Camera-IMU 캘리브레이션 (1-2주)

### Week 13: 외부 캘리브레이션

#### Extrinsic Calibration이란?
```
Camera Frame ←[R, t]→ IMU Frame
```
- [ ] Camera와 IMU 사이의 상대 포즈
- [ ] 회전 R_cam_imu (또는 R_imu_cam)
- [ ] 평행이동 t_cam_imu

#### 왜 중요한가?
- [ ] 잘못된 extrinsic → VIO 발산
- [ ] mm 단위 오차도 성능 저하
- [ ] 특히 회전이 민감

#### 시간 동기화 (Time Offset)
- [ ] Camera와 IMU 타임스탬프 차이
- [ ] td: 보통 수십 ms
- [ ] VINS는 td도 추정 가능 (온라인)

### Week 14: Kalibr 실습

#### 준비
- [ ] Kalibr 설치 (Docker 권장)
- [ ] AprilGrid 타겟 준비 (프린트)
- [ ] 카메라 + IMU 데이터 동시 녹화

#### 데이터 수집
- [ ] 다양한 움직임 필수 (모든 축 회전)
- [ ] 60-120초 정도
- [ ] 타겟이 항상 보이도록

#### 캘리브레이션 실행
- [ ] `kalibr_calibrate_cameras`: 카메라만 먼저
- [ ] `kalibr_calibrate_imu_camera`: Camera-IMU 캘리브레이션
- [ ] 결과 YAML 파일 분석

#### 결과 검증
- [ ] Reprojection error < 0.5 pixel
- [ ] 결과를 VINS config에 반영
- [ ] 실제 VIO 돌려서 확인

### 🔍 Section 4.5 자체 점검
1. Camera-IMU extrinsic이 틀리면 어떤 현상이 나타나는가?
2. Time offset이 있으면 왜 문제가 되는가?
3. Kalibr에서 좋은 결과를 얻으려면 어떤 움직임이 필요한가?

---

## ✅ Phase 4 완료 체크리스트

### IMU 기초
- [ ] IMU 센서 동작 원리 이해
- [ ] 노이즈, 바이어스 개념 이해
- [ ] ROS에서 IMU 데이터 시각화 가능

### 센서 융합
- [ ] 칼만 필터 기본 원리 이해
- [ ] EKF와 ESKF 개념 이해
- [ ] 간단한 칼만 필터 구현 경험

### Pre-integration (핵심!)
- [ ] 왜 Pre-integration이 필요한지 **명확히** 설명 가능
- [ ] Δp, Δv, Δq의 의미 이해
- [ ] Factor Graph에서의 역할 이해
- [ ] VINS 논문 Section III-B 읽음

### 초기화
- [ ] VIO 초기화 과정 개념 이해
- [ ] 스케일, 중력 추정 이유 이해

### 캘리브레이션
- [ ] Kalibr로 Camera-IMU 캘리브 가능
- [ ] 결과를 VINS config에 적용 가능

---

## 🎯 Phase 4 완료 기준

> "IMU와 Vision이 왜 상호보완적인지, Pre-integration 개념을 직관적으로 설명 가능"

---

## 📚 참고 자료

### 논문 (필독)

| 논문 | 용도 | 우선순위 |
|------|------|---------|
| VINS-Mono (Qin et al., 2018) | VIO 전체 | ⭐⭐⭐ |
| On-Manifold Preintegration (Forster et al., 2017) | Pre-integration 상세 | ⭐⭐ |
| MSCKF (Mourikis & Roumeliotis, 2007) | EKF 기반 VIO | ⭐ |

### 책

| 책 | 용도 |
|------|------|
| State Estimation for Robotics (Barfoot) | EKF, Factor Graph, Lie 군 |
| Probabilistic Robotics (Thrun) | 칼만 필터 기초 |

### 강의

| 강의 | 용도 |
|------|------|
| Cyrill Stachniss - Factor Graphs | Factor Graph 개념 |
| Joan Solà - Quaternion Kinematics | 회전, IMU 적분 |

### 커뮤니티

| 커뮤니티 | 용도 |
|----------|------|
| SLAM KR (카카오톡, 페이스북) | 질문, 정보 공유 |
| GitHub Issues (VINS-Fusion) | 구체적 문제 해결 |

---

## 💡 팁

1. **수식에 겁먹지 말기**: 직관적 이해가 먼저, 수식은 나중에
2. **논문 반복 읽기**: VINS 논문을 3번 이상 읽으세요
3. **그림으로 이해**: Factor Graph를 직접 그려보세요
4. **막히면 질문**: 혼자 끙끙대지 말고 커뮤니티 활용
5. **코드와 연결**: 개념 이해되면 VINS 코드에서 확인
6. **IMU 적분 직접 해보기**: 드리프트를 체험해야 필요성 체감

---

## ❓ 다음 단계

Phase 4 완료 후:
- Phase 5 (Detection + Depth Estimation)로 진행
- SLAM 기초 위에 AI Perception 역량을 쌓는 단계로 전환