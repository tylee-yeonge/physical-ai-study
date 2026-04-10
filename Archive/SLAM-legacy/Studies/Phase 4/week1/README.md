# Week 1: IMU 기초 + 센서 융합 개념

> 🎯 **목표**: IMU 센서 동작 원리, 노이즈 모델, 칼만 필터 직관 이해
> ⏰ **예상 시간**: 7-10시간
> 💡 **핵심 질문**: "왜 카메라만으로는 부족하고, IMU가 필요한가?"

---

## 📚 핵심 개념

### 1. IMU 센서

#### 가속도계 (Accelerometer)

- 선형 가속도 측정
- **중력도 포함됨** (정지 시 약 9.81 m/s^2 출력)

```
a_meas = R_wb^T * (a_world - g_world) + b_a + n_a

- R_wb: World → Body 회전 행렬
- g_world: 중력 벡터 [0, 0, -9.81]^T
- b_a: 가속도계 바이어스 (느리게 변함)
- n_a: 백색 잡음
```

| 상태 | 센서 출력 | 실제 가속도 |
|------|----------|-----------|
| 정지 (테이블) | ~9.81 m/s^2 | 0 |
| 자유 낙하 | ~0 | 9.81 m/s^2 |

#### 자이로스코프 (Gyroscope)

- 각속도 측정 (rad/s)

```
omega_meas = omega_true + b_g + n_g

- b_g: 자이로 바이어스 (시간에 따라 drift)
- n_g: 백색 잡음
```

**바이어스의 무서움:**
```
bias = 0.01 rad/s
10초 후: 0.1 rad = 5.7도
100초 후: 1.0 rad = 57.3도  ← 방향 완전 틀어짐!
→ 바이어스 추정이 VIO 성능의 핵심
```

### 2. IMU 데이터 특성

```
IMU:    ████████████████████████████ 200-1000 Hz
Vision: ■         ■         ■        20-30 Hz
```

| 특성 | IMU | Vision |
|------|-----|--------|
| 주파수 | 100-1000 Hz | 20-30 Hz |
| 단기 정확도 | 높음 | 보통 |
| 장기 정확도 | 낮음 (drift) | 높음 |
| 스케일 | 절대 스케일 | 상대 스케일만 |

### 3. IMU 노이즈 모델

| 유형 | 설명 | 파라미터 |
|------|------|---------|
| **White Noise** | 랜덤 고주파 노이즈 | sigma_a, sigma_g |
| **Bias Random Walk** | 천천히 변하는 바이어스 | sigma_ba, sigma_bg |

VINS config에서:
- `acc_n`: 가속도계 white noise
- `gyr_n`: 자이로 white noise
- `acc_w`: 가속도계 bias random walk
- `gyr_w`: 자이로 bias random walk

### 4. 센서 융합 직관

#### 칼만 필터 = 가중 평균

```
최적 추정 = 예측 x 측정_신뢰도 + 측정 x 예측_신뢰도
            ─────────────────────────────────────
                 예측_신뢰도 + 측정_신뢰도

→ 불확실성 작은 쪽에 더 가중치
```

#### EKF (Extended Kalman Filter)

- 비선형 시스템 → 현재 추정값 주변에서 **선형화**
- 자코비안 F(상태전이), H(관측)로 공분산 전파

#### ESKF (Error-State Kalman Filter)

- Nominal state: IMU로 적분 (비선형)
- Error state: 작은 오차 추정 (더 선형적)
- 많은 VIO 시스템이 ESKF 기반 (MSCKF 등)

---

## 🔍 자체 점검

1. 가속도계가 정지 상태에서도 값을 출력하는 이유는?
2. IMU 데이터 주파수가 Vision보다 높은 이유는?
3. 칼만 필터에서 공분산이 의미하는 것은?
4. EKF에서 자코비안을 쓰는 이유는?

---

다음: [Week 2 - Pre-integration + Factor Graph](../week2/README.md)
