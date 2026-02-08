# Week 12: VIO 초기화 과정 (Section 4.4)

> 🎯 **이번 주 목표**: Vision-only SfM → Visual-Inertial Alignment → 검증의 3단계 초기화 파이프라인 이해하기
> ⏰ **예상 시간**: 10시간
> 💡 **핵심 질문**: "스케일과 중력을 어떻게 한 번에 추정하는가?"

---

## 🌟 시작하기 전에

### 비유: 줄자 없이 방 크기 재기

```
단안 카메라로 방을 촬영하면:
  → 3D 구조는 복원되지만 "진짜 크기"를 모릅니다
  → 방이 2m인지 20m인지 구분 불가

이때 IMU를 추가하면:
  → IMU의 가속도 적분 = 미터 단위의 실제 이동
  → Vision의 up-to-scale 이동과 비교
  → "비율" = 스케일!

이것이 Visual-Inertial Alignment의 핵심 아이디어입니다.
```

### 전주(Week 11)와의 연결

| Week 11 | → | Week 12 |
|---------|---|---------|
| 추정해야 할 것: s, g, b_g, v | → | 실제로 어떻게 추정하는가 |
| 초기화 실패 → VIO 발산 | → | 안정적인 초기화 파이프라인 |
| 개별 미지수 분석 | → | 연립방정식으로 동시 추정 |

---

## 📚 핵심 개념

### 1. 3단계 초기화 파이프라인

```
┌─────────────────────────────────────────────────────────┐
│                   VIO 초기화 파이프라인                      │
│                                                         │
│  Stage 1          Stage 2              Stage 3          │
│  ┌──────────┐    ┌───────────────┐    ┌──────────────┐  │
│  │ Vision   │    │ Visual-       │    │ 검증 &       │  │
│  │ only SfM │ →  │ Inertial      │ →  │ 비선형 정제  │  │
│  │          │    │ Alignment     │    │              │  │
│  └──────────┘    └───────────────┘    └──────────────┘  │
│  ↓               ↓                    ↓                 │
│  up-to-scale     s, g, v, b_g        정제된 초기값      │
│  궤적             추정                                   │
└─────────────────────────────────────────────────────────┘
```

### 2. Stage 1: Vision-only SfM

**목표**: 카메라만으로 초기 맵과 궤적 생성

```
과정:
  1. Feature Tracking (KLT 또는 Descriptor Matching)
     → 연속 프레임에서 특징점 추적

  2. Essential Matrix 계산
     → 5-Point Algorithm + RANSAC
     → 상대 회전 R, 방향 t (크기는 모름)

  3. Triangulation
     → 매칭된 특징점 → 3D 점 복원
     → up-to-scale (s를 모르므로)

  4. Sliding Window PnP
     → 새 프레임이 올 때마다 PnP로 포즈 추정
     → BA로 정제
```

**핵심 제약**:
- 모든 3D 점과 translation은 **임의 스케일**
- `||t|| = 1`로 정규화하는 것이 일반적

### 3. Stage 2: Visual-Inertial Alignment

**목표**: Vision 궤적과 IMU 데이터를 정렬하여 미지수 추정

#### 정렬 공식

키프레임 i에서 j까지:

```
공식 (VINS-Mono 기반):

  s·p_j^c = s·p_i^c + v_i·Δt - 0.5·g·Δt² + R_i·Δp_ij

여기서:
  s      : 스케일 (추정 대상)
  p_i^c  : Vision에서 추정한 i번째 포즈 위치 (up-to-scale)
  v_i    : i번째 키프레임의 속도 (추정 대상)
  g      : 중력 벡터 (추정 대상)
  R_i    : i번째 키프레임의 회전 (Vision에서 알고 있음)
  Δp_ij  : Pre-integrated 위치 변화 (IMU에서 계산)
  Δt     : 시간 간격
```

#### 연립방정식 구성

```
키프레임 쌍 (i,j)마다 하나의 방정식:

  s·(p_j^c - p_i^c) - v_i·Δt + 0.5·g·Δt² = R_i·Δp_ij

미지수: [s, v_0, v_1, ..., v_n, g_x, g_y, g_z]

행렬 형태: A·x = b
  → 선형 시스템으로 풀 수 있다!
```

#### 자이로 바이어스 먼저 추정하는 이유

```
1. 자이로 바이어스 b_g 추정:
   → Vision의 R_ij와 Pre-integrated ΔR_ij 비교
   → min ||Log(ΔR_ij^T · R_i^T · R_j)||
   → b_g에 대해 1차 근사 후 선형 시스템

2. b_g를 알면 Pre-integration을 보정
   → ΔR, Δv, Δp를 보정된 값으로 업데이트

3. 그 다음 s, g, v를 한꺼번에 추정
   → 위의 선형 시스템으로 풀기
```

### 4. Stage 3: 검증 및 비선형 정제

```
검증 기준:
  1. 중력 크기: ||g_estimated|| ≈ 9.81 (±0.1)
     → 크게 벗어나면 초기화 실패 판정

  2. 스케일 양수: s > 0
     → 음수면 물리적으로 의미 없음

  3. Reprojection error < 임계값
     → 3D 점을 재투영한 오차가 작아야

비선형 정제:
  → 선형 해를 초기값으로
  → Ceres/g2o로 BA 수행
  → 모든 미지수를 동시 최적화
```

### 5. VINS-Mono 코드 매핑

| 단계 | 파일 | 함수 |
|------|------|------|
| Feature Tracking | `feature_tracker.cpp` | `readImage()` |
| Vision SfM | `initial_sfm.cpp` | `construct()` |
| 자이로 바이어스 추정 | `initial_alignment.cpp` | `solveGyroscopeBias()` |
| V-I 정렬 | `initial_alignment.cpp` | `LinearAlignment()` |
| 중력 정제 | `initial_alignment.cpp` | `RefineGravity()` |
| 전체 초기화 | `estimator.cpp` | `initialStructure()` |

---

## 🤔 자체 점검 퀴즈

1. **Vision-only SfM에서 왜 스케일을 알 수 없는가?**
   - Essential Matrix에서 t의 크기가 정규화되기 때문
   - 단안 카메라의 근본적 한계 (depth-scale ambiguity)

2. **왜 자이로 바이어스를 먼저 추정하는가?**
   - 회전은 스케일과 무관 (순수하게 각도 정보)
   - b_g를 알아야 Pre-integration 보정이 정확
   - 가속도계 바이어스는 중력과 결합되어 분리가 어려움

3. **선형 시스템으로 풀 수 있는 이유는?**
   - s, v, g가 모두 Pre-integration 공식에서 선형으로 등장
   - R은 Vision에서 이미 알고 있으므로 미지수가 아님

4. **||g|| ≈ 9.81 검증이 왜 중요한가?**
   - 중력은 물리 상수 → 강력한 제약 조건
   - 크기가 맞지 않으면 추정 자체가 잘못된 것

---

## ✅ 이번 주 체크리스트

- [ ] 3단계 초기화 파이프라인 전체 흐름 이해
- [ ] Vision-only SfM의 up-to-scale 의미 이해
- [ ] V-I Alignment 연립방정식 유도 과정 이해
- [ ] 자이로 바이어스를 먼저 추정하는 이유 설명 가능
- [ ] 중력 벡터 크기를 이용한 검증 이해
- [ ] VINS-Mono `initial_alignment.cpp` 흐름 파악

---

## 📝 핵심 요약

```
┌─────────────────────────────────────────────────────────┐
│  VIO 초기화 = 3단계 파이프라인                            │
│                                                         │
│  Stage 1: Vision-only SfM                               │
│    → Feature tracking + Essential Matrix + Triangulation │
│    → up-to-scale 궤적 (s를 모름)                         │
│                                                         │
│  Stage 2: Visual-Inertial Alignment                     │
│    ① 자이로 바이어스 b_g: Vision R vs IMU ΔR 비교        │
│    ② 스케일 s + 중력 g + 속도 v: 선형 시스템 Ax=b        │
│    공식: s·Δp_vision - v·Δt + 0.5·g·Δt² = R·Δp_imu    │
│                                                         │
│  Stage 3: 검증 + 비선형 정제                              │
│    → ||g|| ≈ 9.81 확인                                   │
│    → s > 0 확인                                          │
│    → BA로 정제                                           │
└─────────────────────────────────────────────────────────┘
```

---

이전: [Week 11 - VIO 초기화 문제](../week11/README.md)
다음: [Week 13 - 외부 캘리브레이션](../week13/README.md)
