# Week 7: Pre-integration 수식 - IMU 측정 모델과 사전 적분

이전: [Week 6 - Pre-integration 필요성](../week6/README.md)

> 🎯 **이번 주 목표**: Pre-integration의 수학적 수식을 단계별로 이해하기
> ⏰ **예상 시간**: 10시간
> 💡 **핵심 질문**: "Δp, Δv, ΔR은 어떤 수식으로 계산되는 걸까?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | IMU 측정 모델 수식과 Pre-integration 기초 문제 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | Pre-integration 업데이트 공식 및 자코비안 구현 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | Pre-integration 수식 단계별 구현 실습 |

---

## 🌟 시작하기 전에

### 수식이 왜 필요한가요?

Week 6에서 Pre-integration의 **직관**을 배웠습니다. 이번 주는 **수식**을 배웁니다.

**비유:**
- Week 6: "Pre-integration은 상대 이동을 미리 계산하는 거야" (직관)
- Week 7: "그래서 구체적으로 어떤 수식으로 계산하는데?" (수식)

**겁먹지 마세요!** 수식은 4단계로 나누어 배웁니다:
1. IMU 측정 모델 (센서가 뭘 측정하는가)
2. 연속 시간 적분 (물리적 의미)
3. Pre-integrated 측정값 (핵심 수식)
4. 바이어스 보정 (자코비안)

---

## 📚 핵심 개념 자세히 알아보기

### 1. Step 1: IMU 측정 모델

IMU는 **가속도계**와 **자이로스코프** 두 센서로 구성됩니다.

#### 가속도계 측정 모델

```
a_m = R_bw^T (a_w - g_w) + b_a + n_a
```

| 기호 | 의미 | 설명 |
|------|------|------|
| a_m | 측정된 가속도 | IMU가 출력하는 값 |
| R_bw | 월드→바디 회전 | 바디 프레임으로 변환 |
| a_w | 월드 가속도 | 실제 가속도 (구하고 싶은 것) |
| g_w | 중력 벡터 | [0, 0, -9.81]^T |
| b_a | 가속도 바이어스 | 천천히 변하는 오프셋 |
| n_a | 가속도 노이즈 | 백색 가우시안 노이즈 |

**직관:**
- IMU는 **중력을 포함**해서 측정합니다
- 정지 상태에서도 약 9.81 m/s^2 출력!
- 바이어스와 노이즈가 항상 끼어있습니다

```cpp
// C++로 이해하기
Eigen::Vector3d getMeasuredAcceleration(
    const Eigen::Matrix3d& R_bw,   // 월드→바디 회전
    const Eigen::Vector3d& a_w,    // 실제 월드 가속도
    const Eigen::Vector3d& g_w,    // 중력 [0,0,-9.81]
    const Eigen::Vector3d& b_a,    // 바이어스
    const Eigen::Vector3d& n_a)    // 노이즈
{
    return R_bw.transpose() * (a_w - g_w) + b_a + n_a;
}

// 역으로, 측정값에서 실제 가속도 추출:
// a_w = R_wb * (a_m - b_a - n_a) + g_w
Eigen::Vector3d getTrueAcceleration(
    const Eigen::Matrix3d& R_wb,   // 바디→월드 회전
    const Eigen::Vector3d& a_m,    // 측정된 가속도
    const Eigen::Vector3d& g_w,    // 중력
    const Eigen::Vector3d& b_a)    // 바이어스
{
    return R_wb * (a_m - b_a) + g_w;  // 노이즈는 무시
}
```

#### 자이로스코프 측정 모델

```
omega_m = omega + b_g + n_g
```

| 기호 | 의미 | 설명 |
|------|------|------|
| omega_m | 측정된 각속도 | IMU가 출력하는 값 |
| omega | 실제 각속도 | 구하고 싶은 것 |
| b_g | 자이로 바이어스 | 천천히 변하는 오프셋 |
| n_g | 자이로 노이즈 | 백색 가우시안 노이즈 |

**직관:**
- 가속도계보다 단순! (중력 영향 없음)
- 하지만 바이어스가 시간에 따라 drift
- 적분하면 오차가 빠르게 누적

---

### 2. Step 2: 연속 시간 적분

IMU에서 실제 위치/속도/회전을 구하려면 **적분**이 필요합니다.

#### 회전 적분

```
R_wb(t) = R_wb(t_0) * Exp(∫[t_0 to t] omega(s) ds)
```

이산(discrete) 형태:
```
R(t+dt) = R(t) * Exp(omega * dt)
```

```cpp
// Exp: 각속도 벡터 → 회전행렬 (SO(3) 지수 사상)
Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
    double angle = omega.norm();
    if (angle < 1e-10) {
        return Eigen::Matrix3d::Identity();
    }
    Eigen::Vector3d axis = omega / angle;
    Eigen::Matrix3d K;
    K << 0, -axis.z(), axis.y(),
         axis.z(), 0, -axis.x(),
        -axis.y(), axis.x(), 0;
    // Rodrigues 공식
    return Eigen::Matrix3d::Identity()
         + sin(angle) * K
         + (1 - cos(angle)) * K * K;
}
```

#### 속도 적분

```
v(t) = v(t_0) + ∫[t_0 to t] (R(s) * (a_m(s) - b_a) + g) ds
```

이산 형태:
```
v(t+dt) = v(t) + (R(t) * (a_m - b_a) + g) * dt
```

#### 위치 적분

```
p(t) = p(t_0) + ∫[t_0 to t] v(s) ds
     = p(t_0) + v(t_0) * (t-t_0) + ∫∫ (R*a + g) ds ds
```

이산 형태:
```
p(t+dt) = p(t) + v(t) * dt + 0.5 * (R(t) * (a_m - b_a) + g) * dt^2
```

---

### 3. Step 3: Pre-integrated 측정값 (핵심!)

이제 핵심입니다. 위의 적분을 **출발 포즈에서 분리**합니다.

#### 위치에 대한 Pre-integration

원래 적분 결과:
```
p_j = p_i + v_i * Δt + 0.5 * g * Δt^2 + R_i * Δp_ij
```

여기서 **Δp_ij**가 Pre-integrated measurement:
```
Δp_ij = ΣΣ [ΔR_ik * (a_mk - b_a) * dt^2]
      = Σ [Δv_ik * dt + 0.5 * ΔR_ik * (a_mk - b_a) * dt^2]
```

**핵심**: Δp_ij에는 p_i, v_i, R_i가 없습니다!

#### 속도에 대한 Pre-integration

```
v_j = v_i + g * Δt + R_i * Δv_ij
```

여기서:
```
Δv_ij = Σ [ΔR_ik * (a_mk - b_a) * dt]
```

#### 회전에 대한 Pre-integration

```
R_j = R_i * ΔR_ij
```

여기서:
```
ΔR_ij = Π Exp((omega_mk - b_g) * dt)
       k=i to j-1
```

(Π는 행렬의 연속 곱)

#### 이산 시간 업데이트 공식 (구현용)

**직관: 각 수식이 하는 일**

```
매 IMU 데이터(dt 간격)마다 "상대 이동"을 조금씩 누적합니다:

ΔR: "프레임 i에서 현재까지 얼마나 회전했나?"
    → 새 각속도를 Exp()로 회전행렬로 변환 후 곱함
    → Exp(ω·dt) = "ω 방향으로 |ω|·dt 라디안 회전"하는 행렬

Δv: "프레임 i 기준으로 속도가 얼마나 변했나?"
    → 가속도(a_m - b_a)를 ΔR로 i 기준 좌표로 변환 후 적분

Δp: "프레임 i 기준으로 위치가 얼마나 변했나?"
    → 속도를 적분 + 가속도의 이중 적분 (0.5·a·dt²)
```

```
ΔR_ij  ← ΔR_ij * Exp((omega_m - b_g) * dt)
Δv_ij  ← Δv_ij + ΔR_ij * (a_m - b_a) * dt
Δp_ij  ← Δp_ij + Δv_ij * dt + 0.5 * ΔR_ij * (a_m - b_a) * dt^2
```

```cpp
// Pre-integration 핵심 업데이트
void preintegrate(
    const Eigen::Vector3d& acc_measured,
    const Eigen::Vector3d& gyro_measured,
    const Eigen::Vector3d& bias_acc,
    const Eigen::Vector3d& bias_gyro,
    double dt,
    Eigen::Matrix3d& delta_R,   // in-out
    Eigen::Vector3d& delta_v,   // in-out
    Eigen::Vector3d& delta_p)   // in-out
{
    // 바이어스 제거
    Eigen::Vector3d acc  = acc_measured - bias_acc;
    Eigen::Vector3d gyro = gyro_measured - bias_gyro;

    // 위치 (먼저! 현재 delta_v 사용)
    delta_p += delta_v * dt + 0.5 * delta_R * acc * dt * dt;

    // 속도
    delta_v += delta_R * acc * dt;

    // 회전
    delta_R = delta_R * expSO3(gyro * dt);
}
```

---

### 4. Step 4: 바이어스 보정 (1차 근사)

최적화 과정에서 바이어스 추정값이 변합니다. Pre-integrated 값을 수정해야 합니다.

#### 1차 테일러 전개

```
Δp(b + δb) ≈ Δp(b) + ∂Δp/∂b_a * δb_a + ∂Δp/∂b_g * δb_g
                       ├─ J_p_ba ─┤      ├─ J_p_bg ─┤

Δv(b + δb) ≈ Δv(b) + J_v_ba * δb_a + J_v_bg * δb_g

ΔR(b + δb) ≈ ΔR(b) * Exp(J_R_bg * δb_g)
```

#### 자코비안 업데이트 (재귀적)

**직관: 자코비안이 답하는 질문**

```
"바이어스 추정이 δb만큼 틀렸을 때, ΔR/Δv/Δp는 얼마나 틀릴까?"

예) J_v_ba = ∂Δv/∂b_a
  → "가속도 바이어스가 0.01 틀리면, Δv가 J_v_ba × 0.01 만큼 틀림"
  → 재적분 없이 간단한 곱셈으로 보정 가능!

왜 재귀적인가?
  → Pre-integration은 매 dt마다 누적되므로
  → 자코비안도 매 dt마다 "이전 영향 + 새 영향"을 누적
```

매 IMU 데이터가 올 때마다 자코비안도 업데이트됩니다:

```
J_R_bg ← Exp(-omega*dt)^T * J_R_bg - Jr(omega*dt) * dt

J_v_ba ← J_v_ba - ΔR * dt
J_v_bg ← J_v_bg - ΔR * [acc]_x * J_R_bg * dt

J_p_ba ← J_p_ba + J_v_ba * dt - 0.5 * ΔR * dt^2
J_p_bg ← J_p_bg + J_v_bg * dt - 0.5 * ΔR * [acc]_x * J_R_bg * dt^2
```

여기서:
- `[acc]_x` : acc의 skew-symmetric 행렬 (hat 연산)
- `Jr` : SO(3)의 right Jacobian

```cpp
// Skew-symmetric 행렬 (hat 연산)
Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
    Eigen::Matrix3d m;
    m <<    0, -v.z(),  v.y(),
         v.z(),     0, -v.x(),
        -v.y(),  v.x(),     0;
    return m;
}

// SO(3) Right Jacobian
Eigen::Matrix3d rightJacobian(const Eigen::Vector3d& phi) {
    double angle = phi.norm();
    if (angle < 1e-10) {
        return Eigen::Matrix3d::Identity();
    }
    Eigen::Vector3d axis = phi / angle;
    Eigen::Matrix3d K = skew(axis);
    return Eigen::Matrix3d::Identity()
         - (1 - cos(angle)) / (angle * angle) * skew(phi)
         + (angle - sin(angle)) / (angle * angle * angle)
           * skew(phi) * skew(phi);
}

// 자코비안 업데이트
void updateJacobians(
    const Eigen::Vector3d& acc,       // bias 제거된 가속도
    const Eigen::Vector3d& gyro,      // bias 제거된 각속도
    const Eigen::Matrix3d& delta_R,   // 현재 상대 회전
    double dt,
    Eigen::Matrix3d& J_R_bg,
    Eigen::Matrix3d& J_v_ba, Eigen::Matrix3d& J_v_bg,
    Eigen::Matrix3d& J_p_ba, Eigen::Matrix3d& J_p_bg)
{
    Eigen::Matrix3d dR_inv = expSO3(-gyro * dt);
    Eigen::Matrix3d Jr = rightJacobian(gyro * dt);

    // 위치 자코비안 (먼저! 이전 J_v 값 사용)
    J_p_ba += J_v_ba * dt - 0.5 * delta_R * dt * dt;
    J_p_bg += J_v_bg * dt
            - 0.5 * delta_R * skew(acc) * J_R_bg * dt * dt;

    // 속도 자코비안
    J_v_ba -= delta_R * dt;
    J_v_bg -= delta_R * skew(acc) * J_R_bg * dt;

    // 회전 자코비안
    J_R_bg = dR_inv * J_R_bg - Jr * dt;
}
```

---

### 5. 전체 수식 정리 (한눈에 보기)

```
┌──────────────────────────────────────────────────────┐
│           Pre-integration 수식 요약                    │
├──────────────────────────────────────────────────────┤
│                                                      │
│  [IMU 측정 모델]                                      │
│  a_m = R^T(a-g) + b_a + n_a                         │
│  ω_m = ω + b_g + n_g                                │
│                                                      │
│  [Pre-integrated 측정값]                               │
│  ΔR_ij = Π Exp((ω_m - b_g)·dt)                      │
│  Δv_ij = Σ ΔR_ik·(a_m - b_a)·dt                     │
│  Δp_ij = Σ [Δv_ik·dt + 0.5·ΔR_ik·(a_m-b_a)·dt²]   │
│                                                      │
│  [상태 복원 공식]                                      │
│  R_j = R_i · ΔR_ij                                   │
│  v_j = v_i + g·Δt + R_i · Δv_ij                     │
│  p_j = p_i + v_i·Δt + 0.5·g·Δt² + R_i · Δp_ij      │
│                                                      │
│  [바이어스 보정 (1차 근사)]                              │
│  Δp' ≈ Δp + J_p_ba·δb_a + J_p_bg·δb_g               │
│  Δv' ≈ Δv + J_v_ba·δb_a + J_v_bg·δb_g               │
│  ΔR' ≈ ΔR · Exp(J_R_bg·δb_g)                        │
│                                                      │
└──────────────────────────────────────────────────────┘
```

---

## 💡 꼭 이해해야 할 핵심 개념

### 왜 R^T(a-g) 인가? (가속도계 모델의 의미)

많은 학생이 가속도계 모델에서 헷갈립니다.

**핵심**: 가속도계는 **바디 프레임**에서 측정합니다.

```
월드 프레임에서의 실제 가속도: a_w
월드 프레임에서의 중력: g = [0, 0, -9.81]

가속도계가 측정하는 것:
  a_m = R^T_wb * (a_w - g_w)
       ↑ 월드→바디 변환  ↑ 중력 빼기(관성력 원리)

왜 g를 빼나? → 자유낙하 중인 가속도계는 0을 출력!
             → 정지 상태에서는 g의 반대 방향(+9.81)을 출력
```

**예시:**

```
정지 상태 (a_w = 0):
  a_m = R^T * (0 - [0,0,-9.81]) = R^T * [0, 0, 9.81]
  → z축으로 약 9.81 m/s^2 출력! (중력의 반대)

자유낙하 (a_w = g):
  a_m = R^T * (g - g) = 0
  → 0 출력! (무중력 상태)
```

### 순서가 중요하다: 위치 → 속도 → 회전

코드 구현 시 업데이트 순서에 주의해야 합니다:

```
1. delta_p 업데이트 (현재 delta_v, delta_R 사용)
2. delta_v 업데이트 (현재 delta_R 사용)
3. delta_R 업데이트

순서를 바꾸면 결과가 달라집니다!
(또는 mid-point integration 사용 시 순서 무관)
```

### 중간점 적분 (Mid-point Integration)

더 정확한 적분을 위해 VINS에서는 중간점 방법을 사용합니다:

```
acc_mean = 0.5 * (ΔR_k * acc_k + ΔR_{k+1} * acc_{k+1})
gyro_mean = 0.5 * (gyro_k + gyro_{k+1})

Δp += Δv * dt + 0.5 * acc_mean * dt^2
Δv += acc_mean * dt
ΔR *= Exp(gyro_mean * dt)
```

---

## 🔍 자체 점검 - 이해했는지 확인!

### 질문 1: 가속도계 측정 모델
**Q:** IMU가 정지 상태에서 z축으로 9.81 m/s^2를 측정했습니다. 실제 가속도는 얼마인가요?

**A:** 0 m/s^2 (정지 상태)입니다. 가속도계는 중력의 반대 방향을 측정하므로, 정지 시 g를 출력합니다. 실제 가속도를 구하려면 측정값에서 중력을 제거해야 합니다.

### 질문 2: Pre-integration 수식
**Q:** Δv_ij = Σ ΔR_ik · (a_m - b_a) · dt 에서 ΔR_ik의 역할은 무엇인가요?

**A:** ΔR_ik는 시각 i 기준으로 시각 k의 상대 회전입니다. 바디 프레임에서 측정된 가속도를 i 프레임 기준으로 변환하는 역할을 합니다.

### 질문 3: 자코비안
**Q:** 바이어스가 0.01 rad/s만큼 변했을 때, Pre-integration 값을 재적분 없이 보정하려면 어떻게 하나요?

**A:** ΔR' ≈ ΔR · Exp(J_R_bg · δb_g)로 보정합니다. J_R_bg는 Pre-integration 과정에서 이미 계산된 자코비안이므로, 간단한 행렬 연산만으로 보정 가능합니다.

### 질문 4: 구현 순서
**Q:** Pre-integration 업데이트 시 왜 위치를 먼저, 회전을 나중에 업데이트해야 하나요?

**A:** 위치 업데이트에 현재 속도와 회전이 필요하고, 속도 업데이트에 현재 회전이 필요하기 때문입니다. 회전을 먼저 업데이트하면 "다음 시점"의 회전을 사용하게 되어 오차가 발생합니다.

---

## 📝 이번 주 실습 & 다음 주 준비

### 이번 주 실습

**체크리스트:**
- [ ] IMU 측정 모델 수식 직접 손으로 써보기
- [ ] Pre-integration 업데이트 공식 코드로 구현 (quiz_medium.cpp)
- [ ] 자코비안 계산 공식 이해
- [ ] PRACTICE.md의 단계별 구현 따라하기

```bash
cd week7
mkdir build && cd build
cmake ..
make
./quiz_easy
./quiz_medium
```

### 다음 주 준비사항

**Week 8: Factor Graph에서의 역할**에서 배울 것:
- Pre-integrated measurement를 Factor Graph에 넣는 방법
- IMU Factor와 Visual Factor의 관계
- 전체 VIO 그래프 구조

**준비물:**
- ✅ Phase 3의 Factor Graph / Bundle Adjustment 복습
- ✅ 비용 함수(cost function) 개념 복습
- ✅ 잔차(residual) 개념 이해

---

## 🎯 이번 주 핵심 요약

1. **IMU 측정 모델**: a_m = R^T(a-g) + b_a + n_a, omega_m = omega + b_g + n_g
   - 가속도계는 중력을 포함해서 측정한다

2. **Pre-integrated 측정값**: ΔR, Δv, Δp
   - 출발 포즈와 무관하게 IMU 데이터만으로 계산
   - 이산 업데이트: ΔR → Δv → Δp 순서로

3. **상태 복원**: p_j = p_i + v_i·Δt + 0.5·g·Δt^2 + R_i·Δp_ij
   - Pre-integrated 값을 사용해 절대 포즈 복원

4. **바이어스 보정**: 자코비안으로 1차 근사
   - J_p_ba, J_p_bg, J_v_ba, J_v_bg, J_R_bg 5개 자코비안

5. **구현 핵심**: 업데이트 순서 (p → v → R)와 바이어스 제거
   - 더 정확하려면 mid-point integration 사용

---

다음: [Week 8 - Factor Graph에서의 역할](../week8/README.md)
