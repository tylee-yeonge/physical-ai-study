# Week 13: Camera-IMU 외부 캘리브레이션 (Section 4.5)

> 🎯 **이번 주 목표**: Camera-IMU Extrinsic 캘리브레이션과 시간 동기화의 중요성 이해하기
> ⏰ **예상 시간**: 10시간
> 💡 **핵심 질문**: "카메라와 IMU 사이의 상대 포즈를 왜, 어떻게 알아내는가?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | Extrinsic 파라미터와 시간 오프셋 기초 문제 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | 핸드-아이 캘리브레이션 AX=XB 구현 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | Camera-IMU Extrinsic 캘리브레이션 실습 |

---

## 🌟 시작하기 전에

### 비유: 눈과 귀의 위치 관계

```
사람이 소리의 방향을 판단할 때:
  → 두 귀의 정확한 위치(머리 양쪽)를 알아야
  → 소리 도착 시간 차이로 방향을 계산

VIO에서도 마찬가지:
  → 카메라와 IMU의 정확한 상대 위치/회전을 알아야
  → 두 센서의 측정을 올바르게 융합 가능

이 "상대 포즈"가 Extrinsic Calibration입니다.
```

### 전주(Week 12)와의 연결

| Week 12 | → | Week 13 |
|---------|---|---------|
| V-I Alignment에서 R, t 사용 | → | 이 R, t를 어떻게 구하는가 |
| 초기화의 정확도 | → | Extrinsic 오차가 초기화에 미치는 영향 |
| VINS 전체 파이프라인 | → | 캘리브레이션 = VIO 성능의 기반 |

---

## 📚 핵심 개념

### 1. Extrinsic Calibration이란?

```
Camera Frame ←──[R_ci, t_ci]──→ IMU Frame

  R_ci : IMU 프레임에서 Camera 프레임으로의 회전
  t_ci : IMU 프레임에서 Camera 프레임으로의 병진

  p_camera = R_ci · p_imu + t_ci
```

#### 왜 알아야 하는가?

```
IMU 측정:
  가속도, 각속도 → IMU 프레임 기준

Camera 측정:
  특징점 좌표 → Camera 프레임 기준

VIO 융합:
  → 두 측정을 같은 프레임으로 변환해야
  → Extrinsic이 틀리면 변환이 틀림 → VIO 발산
```

### 2. Extrinsic 오차의 영향

#### 회전 오차의 영향 (더 치명적)

```
R_ci에 1도 오차가 있으면:
  → IMU 가속도를 Camera 프레임으로 변환할 때
  → sin(1°) × 9.81 ≈ 0.17 m/s² 오차
  → 이것이 중력 누출과 동일한 효과!

결론: 회전 오차 1도 → 수십 cm/s 단위 드리프트
```

#### 병진 오차의 영향

```
t_ci에 1cm 오차가 있으면:
  → 회전 시 레버암 효과로 가속도 오차 발생
  → a_error = ω × (ω × Δt) + α × Δt
  → 고속 회전 시 영향 커짐

결론: 일반적으로 회전 오차가 더 치명적
```

### 3. 시간 동기화 (Time Offset, td)

```
실제 시스템에서:
  Camera 타임스탬프: t_cam
  IMU 타임스탬프:    t_imu = t_cam + td

  td: 보통 수~수십 ms

문제:
  td를 무시하면 → IMU 데이터를 잘못된 시점에 적분
  → 고속 운동 시 큰 오차
```

#### td가 중요한 이유

```
예시: td = 10ms, 각속도 = 1 rad/s
  → 10ms 동안의 회전 = 0.01 rad ≈ 0.57도
  → 이 오차가 매 프레임마다 발생
  → 누적되면 치명적

VINS-Mono의 접근:
  → td를 상태 변수로 추가
  → 온라인으로 추정 (최적화에 포함)
  → 보통 수렴 후 안정적
```

### 4. 캘리브레이션 방법

#### 오프라인 방법: Kalibr

```
준비:
  1. AprilGrid 타겟 (체커보드도 가능)
  2. Camera + IMU 동시 녹화
  3. 다양한 운동 (모든 축 회전 + 병진)

과정:
  1. kalibr_calibrate_cameras → 카메라 내부 파라미터
  2. kalibr_calibrate_imu_camera → Extrinsic + td
  3. 결과 YAML 파일 분석

검증:
  → Reprojection error < 0.5 pixel
  → 물리적으로 합리적인 R, t 확인
```

#### 온라인 방법: VINS-Mono

```
VINS의 온라인 Extrinsic 추정:
  → initial_ex_rotation.cpp
  → 여러 프레임에서 Camera R과 IMU ΔR 수집
  → 핸드-아이 캘리브레이션 문제로 풀기
  → AX = XB (A=Camera 회전, B=IMU 회전, X=Extrinsic)

장점: 별도 캘리브레이션 불필요
단점: 정확도가 오프라인보다 낮을 수 있음
```

### 5. 핸드-아이 캘리브레이션

```
문제 공식화:
  Camera 상대 회전: R_cam_ij = R_ci · R_imu_ij · R_ci^T
  → R_cam_ij · R_ci = R_ci · R_imu_ij
  → AX = XB 형태!

여기서:
  A = R_cam_ij (카메라에서 관측)
  B = R_imu_ij (IMU에서 관측)
  X = R_ci (구하고 싶은 Extrinsic 회전)

풀이:
  → 여러 (A, B) 쌍을 모아서 최소자승
  → Quaternion 기반 풀이가 일반적
```

### 6. VINS-Mono 코드 매핑

| 개념 | 파일 | 함수/변수 |
|------|------|----------|
| Extrinsic 회전 | `parameters.cpp` | `RIC[0]`, `TIC[0]` |
| 온라인 추정 | `initial_ex_rotation.cpp` | `CalibrationExRotation()` |
| td 추정 | `feature_manager.cpp` | `td` |
| Config 파일 | `*.yaml` | `extrinsicRotation`, `extrinsicTranslation` |

---

## 🤔 자체 점검 퀴즈

1. **Extrinsic 회전 오차 1도가 왜 치명적인가?**
   - 중력 방향이 틀어지면서 중력 누출과 같은 효과
   - 매 프레임마다 오차가 누적

2. **시간 오프셋 td를 왜 추정해야 하는가?**
   - Camera와 IMU의 시계가 정확히 동기화되지 않음
   - 수 ms의 td라도 고속 운동 시 큰 회전/위치 오차

3. **핸드-아이 캘리브레이션 AX=XB에서 각 항의 의미는?**
   - A: Camera 관측 상대 회전, B: IMU 관측 상대 회전
   - X: Camera-IMU 간 Extrinsic 회전

4. **오프라인(Kalibr) vs 온라인 캘리브레이션의 트레이드오프는?**
   - 오프라인: 높은 정확도, 별도 과정 필요
   - 온라인: 편의성, 정확도 다소 낮음, 초기 수렴 시간

---

## ✅ 이번 주 체크리스트

- [ ] Extrinsic (R_ci, t_ci)의 정의와 물리적 의미 이해
- [ ] 회전 오차가 병진 오차보다 치명적인 이유 설명 가능
- [ ] 시간 오프셋 td의 영향 이해
- [ ] 핸드-아이 캘리브레이션 AX=XB 공식 유도
- [ ] Kalibr 캘리브레이션 파이프라인 이해
- [ ] VINS-Mono의 온라인 Extrinsic 추정 흐름 파악

---

## 📝 핵심 요약

```
┌─────────────────────────────────────────────────────────┐
│  Camera-IMU Extrinsic Calibration                       │
│                                                         │
│  Extrinsic: Camera ←[R_ci, t_ci]→ IMU                  │
│    → 두 센서의 상대 위치/회전                            │
│    → 틀리면 VIO 전체가 발산                              │
│                                                         │
│  회전 오차가 더 치명적:                                   │
│    → 1도 오차 → 중력 누출 효과 → 드리프트               │
│                                                         │
│  시간 동기화 (td):                                       │
│    → Camera와 IMU 시계 차이                              │
│    → VINS는 온라인 추정 지원                             │
│                                                         │
│  캘리브레이션 방법:                                       │
│    오프라인: Kalibr (AprilGrid + 다양한 운동)            │
│    온라인: 핸드-아이 AX=XB (VINS initial_ex_rotation)    │
└─────────────────────────────────────────────────────────┘
```

---

이전: [Week 12 - VIO 초기화 과정](../week12/README.md)
다음: [Week 14 - Kalibr 실습](../week14/README.md)
