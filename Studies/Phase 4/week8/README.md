# Week 8: Factor Graph에서의 역할 - IMU Factor와 Visual Factor

이전: [Week 7 - Pre-integration 수식](../week7/README.md)

> 🎯 **이번 주 목표**: Pre-integrated measurement를 Factor Graph에 통합하는 방법 이해
> ⏰ **예상 시간**: 10시간
> 💡 **핵심 질문**: "Pre-integration 결과를 어떻게 최적화에 넣을까?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | IMU/Visual Factor 잔차 개념 기초 문제 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | IMU Factor 잔차 계산 및 Factor Graph 구성 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | VIO Factor Graph 구조와 잔차 계산 실습 |

---

## 🌟 시작하기 전에

### Week 7 복습: Pre-integration 결과물

```
Week 7에서 배운 것:
  ΔR_ij, Δv_ij, Δp_ij = Pre-integrated measurements
  → IMU 데이터만으로 계산, 출발 포즈와 무관

이번 주 질문:
  "이 값들을 최적화(Factor Graph)에 어떻게 넣지?"
  → IMU Factor로 변환!
```

### 비유: 지도 만들기

```
Factor Graph = 단서들을 모아 최적의 답을 찾는 퍼즐

단서 1 (IMU Factor):
  "5초 동안 IMU로 측정하니 약 10m 전진, 0.5rad 회전했어"
  → Pre-integrated measurement가 이 단서

단서 2 (Visual Factor):
  "이 3D 점이 카메라에 (320, 240)에 찍혀야 해"
  → 재투영 오차가 이 단서

단서 3 (Prior Factor):
  "시작점은 원점이야"
  → 초기 조건

모든 단서를 동시에 만족하는 최적 해 → 최적화!
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. Factor Graph 복습

```
Factor Graph 구성요소:

  ○ 변수 노드 (Variable Node): 추정할 값
    → 포즈, 속도, 바이어스
    → x_i = {R_i, p_i, v_i, b_a_i, b_g_i}

  ■ 팩터 노드 (Factor Node): 제약 조건
    → 측정값에서 만든 오차 함수
    → 오차가 작을수록 좋음

  구조:
    ○ x_0 ─■─ ○ x_1 ─■─ ○ x_2 ─■─ ○ x_3
              │          │          │
              ■          ■          ■
              │          │          │
              ○ l_1      ○ l_2     ○ l_3

    수평 ■: IMU Factor (연속 포즈 연결)
    수직 ■: Visual Factor (포즈와 3D 점 연결)
```

---

### 2. IMU Factor (핵심!)

#### 잔차 (Residual) 정의

```
IMU Factor는 Pre-integrated measurement와
실제 상태 사이의 '차이'를 잔차로 정의합니다.

상태 복원 공식:
  R_j = R_i · ΔR_ij
  v_j = v_i + g·Δt + R_i · Δv_ij
  p_j = p_i + v_i·Δt + 0.5·g·Δt² + R_i · Δp_ij

잔차 (차이):
  r_ΔR = Log(ΔR_ij^T · R_i^T · R_j)
  r_Δv = R_i^T · (v_j - v_i - g·Δt) - Δv_ij
  r_Δp = R_i^T · (p_j - p_i - v_i·Δt - 0.5·g·Δt²) - Δp_ij

  만약 상태가 완벽하면 → 잔차 = 0!
  상태에 오차가 있으면 → 잔차 ≠ 0 → 최적화로 줄임
```

#### C++ 구현

```cpp
// IMU Factor 잔차 계산
Eigen::VectorXd computeIMUResidual(
    // 상태 i
    const Eigen::Matrix3d& R_i,
    const Eigen::Vector3d& p_i,
    const Eigen::Vector3d& v_i,
    const Eigen::Vector3d& ba_i,
    const Eigen::Vector3d& bg_i,
    // 상태 j
    const Eigen::Matrix3d& R_j,
    const Eigen::Vector3d& p_j,
    const Eigen::Vector3d& v_j,
    // Pre-integrated measurements
    const Eigen::Matrix3d& delta_R,
    const Eigen::Vector3d& delta_v,
    const Eigen::Vector3d& delta_p,
    double delta_t,
    // 중력
    const Eigen::Vector3d& gravity)
{
    Eigen::VectorXd residual(9);  // 3+3+3

    // 회전 잔차 (3D)
    Eigen::Matrix3d r_dR = delta_R.transpose() * R_i.transpose() * R_j;
    residual.segment<3>(0) = logSO3(r_dR);

    // 속도 잔차 (3D)
    residual.segment<3>(3) = R_i.transpose() *
        (v_j - v_i - gravity * delta_t) - delta_v;

    // 위치 잔차 (3D)
    residual.segment<3>(6) = R_i.transpose() *
        (p_j - p_i - v_i * delta_t - 0.5 * gravity * delta_t * delta_t)
        - delta_p;

    return residual;
}
```

#### IMU Factor의 비용 함수

```
비용 함수:
  E_imu = r^T · Σ^{-1} · r

  r: 잔차 벡터 (9차원: 회전3 + 속도3 + 위치3)
  Σ: Pre-integration 공분산 (9×9)
  Σ^{-1}: 정보 행렬 (Information matrix)

  → 공분산이 작은(확실한) 측정의 잔차에 더 큰 가중치!
```

---

### 3. Visual Factor

```
Visual Factor = 재투영 오차 (Reprojection Error)

3D 점 P_w를 카메라 i로 투영:
  P_c = R_cw · P_w + t_cw
  [u, v] = π(P_c) = [fx·X/Z + cx, fy·Y/Z + cy]

잔차:
  r_visual = z_measured - π(R_i^T · (P_w - p_i))
           = [u_meas - u_proj, v_meas - v_proj]

  → 2차원 잔차 (픽셀 오차)
```

```cpp
// Visual Factor 잔차 계산
Eigen::Vector2d computeVisualResidual(
    const Eigen::Matrix3d& R_i,
    const Eigen::Vector3d& p_i,
    const Eigen::Vector3d& P_w,    // 3D 랜드마크
    const Eigen::Vector2d& z_meas, // 측정된 픽셀 좌표
    double fx, double fy, double cx, double cy)
{
    // 월드 → 카메라 변환
    Eigen::Vector3d P_c = R_i.transpose() * (P_w - p_i);

    // 카메라 투영
    double u_proj = fx * P_c(0) / P_c(2) + cx;
    double v_proj = fy * P_c(1) / P_c(2) + cy;

    // 잔차
    return z_meas - Eigen::Vector2d(u_proj, v_proj);
}
```

---

### 4. 전체 VIO Factor Graph

```
┌─────────────────────────────────────────────────────────┐
│              VIO Factor Graph 구조                        │
│                                                         │
│  [KF_0]──IMU──[KF_1]──IMU──[KF_2]──IMU──[KF_3]        │
│    │  \          │  \          │  \          │          │
│    │   Vis       │   Vis       │   Vis       │          │
│    │    \        │    \        │    \        │          │
│    │   [P_1]     │   [P_2]     │   [P_3]     │          │
│    │              │                           │          │
│  Prior         Bias          Bias           Bias        │
│                                                         │
│  KF_i = {R_i, p_i, v_i, b_a_i, b_g_i}                 │
│  P_j  = 3D 랜드마크 점                                   │
│                                                         │
│  Factor 종류:                                            │
│    IMU:    Pre-integrated measurement (9D 잔차)          │
│    Vis:    재투영 오차 (2D 잔차)                           │
│    Prior:  초기값 제약                                    │
│    Bias:   바이어스 연속성 (Random Walk)                   │
└─────────────────────────────────────────────────────────┘
```

---

### 5. 최적화 목표

```
전체 비용 함수:
  E_total = E_prior + Σ E_imu + Σ E_visual + Σ E_bias

  = ||r_prior||²_{Σ_prior}
  + Σ_i ||r_imu(x_i, x_{i+1})||²_{Σ_imu}
  + Σ_{i,j} ||r_visual(x_i, P_j)||²_{Σ_visual}
  + Σ_i ||b_{i+1} - b_i||²_{Σ_bias}

||r||²_Σ = r^T · Σ^{-1} · r  (마하라노비스 거리)

최적화: argmin_{x, P} E_total
  → Gauss-Newton 또는 Levenberg-Marquardt
  → Ceres Solver, GTSAM, g2o 등 사용
```

---

### 6. Sliding Window 최적화

```
전체 그래프가 커지면 → 실시간 불가능
→ Sliding Window로 최근 N개 키프레임만 최적화

[KF_k-3]──IMU──[KF_k-2]──IMU──[KF_k-1]──IMU──[KF_k]
    ↑
 Marginalization
 (과거 정보를
  Prior로 변환)

Marginalization:
  1. 오래된 키프레임을 그래프에서 제거
  2. 그 키프레임의 정보를 Prior Factor로 변환
  3. → 과거 정보를 잃지 않으면서 계산량 제한!

VINS-Mono: Window 크기 = 10 키프레임
```

---

### 7. VINS 코드 연결

```
VINS-Mono 코드 구조:

integration_base.h:
  → Pre-integration 클래스
  → integrate(): IMU 데이터 적분
  → repropagate(): 바이어스 변경 시 재적분

imu_factor.h:
  → Ceres Cost Function
  → Evaluate(): IMU 잔차 계산
  → 위에서 배운 r_ΔR, r_Δv, r_Δp 계산

projection_factor.h:
  → Visual 잔차 계산
  → 재투영 오차

estimator.cpp:
  → Sliding Window 관리
  → Ceres Problem 구성
  → 최적화 실행
```

---

## 💡 꼭 이해해야 할 핵심 개념

### 잔차의 의미

```
잔차 = "측정값이 말하는 것" vs "현재 추정이 말하는 것"

IMU 잔차:
  "IMU로 보면 10m 이동했는데, 현재 추정은 10.5m 이동"
  → 잔차 = 0.5m → 최적화가 이 차이를 줄임

Visual 잔차:
  "카메라에 (320,240)에 보여야 하는데, 추정으로는 (322,238)"
  → 잔차 = (2, 2) 픽셀 → 최적화가 이 차이를 줄임
```

### 정보 행렬 (가중치)

```
IMU가 매우 정확 (Σ 작음) → Σ^{-1} 큼 → 큰 가중치
Visual이 부정확 (Σ 큼) → Σ^{-1} 작음 → 작은 가중치

→ 정확한 센서에 더 의존!
→ 칼만 필터의 가중 평균과 같은 원리
```

---

## 🔍 자체 점검 - 이해했는지 확인!

### Q1: IMU Factor 잔차
**Q:** IMU Factor의 잔차가 9차원인 이유는?

**A:** 회전 잔차 3D + 속도 잔차 3D + 위치 잔차 3D = 9D. 바이어스는 별도 Factor로 처리합니다.

### Q2: Visual Factor 잔차
**Q:** Visual Factor의 잔차가 2차원인 이유는?

**A:** 이미지 평면에서의 재투영 오차이므로 (u, v) 2차원입니다. 하나의 3D 점이 여러 카메라에서 관측되면 여러 개의 2D 잔차가 생깁니다.

### Q3: Sliding Window
**Q:** 전체 그래프 대신 Sliding Window를 사용하는 이유는?

**A:** 실시간성 확보입니다. 키프레임이 늘어나면 최적화 비용이 O(n^3)으로 증가합니다. Window를 고정하고 과거 정보를 Marginalization으로 Prior에 흡수시킵니다.

### Q4: 마하라노비스 거리
**Q:** ||r||^2_Σ = r^T · Σ^{-1} · r 에서 Σ^{-1}의 역할은?

**A:** 센서별 신뢰도에 따른 가중치입니다. 공분산이 작은(정확한) 센서의 잔차에 더 큰 가중치를 줘서, 정확한 측정에 더 의존합니다.

---

## 📝 이번 주 실습 & 다음 주 준비

### 이번 주 체크리스트

- [ ] IMU Factor 잔차 수식 이해 (r_ΔR, r_Δv, r_Δp)
- [ ] Visual Factor 잔차 (재투영 오차) 이해
- [ ] 전체 VIO Factor Graph 그림 그릴 수 있음
- [ ] 마하라노비스 거리와 정보 행렬의 역할 이해
- [ ] Sliding Window와 Marginalization 개념 이해
- [ ] `PRACTICE.md`, `quiz_easy.cpp`, `quiz_medium.cpp` 완료

### 다음 주 미리보기: Pre-integration 심화

```
Week 9에서 배울 것:
  1. Pre-integration 공분산 전파 (9×9 공분산)
  2. On-Manifold Pre-integration (SO(3)에서 직접)
  3. VINS 실제 코드 분석 (integration_base.h)
```

---

## 🎯 이번 주 핵심 요약

1. **IMU Factor**: Pre-integrated measurement와 상태의 차이를 잔차로
   - 9D 잔차: 회전(3) + 속도(3) + 위치(3)
   - 공분산으로 가중치 부여

2. **Visual Factor**: 3D 점의 재투영 오차
   - 2D 잔차: 픽셀 오차 (u, v)
   - 여러 프레임에서 동일 점 관측 → 강한 제약

3. **VIO Factor Graph**: IMU + Visual + Prior + Bias
   - 모든 Factor의 가중 합 최소화
   - Gauss-Newton / LM 최적화

4. **Sliding Window**: 실시간을 위한 제한
   - 최근 N개 키프레임만 최적화
   - Marginalization으로 과거 정보 보존

5. **VINS 코드**: integration_base.h, imu_factor.h, projection_factor.h
   - 이론 → 코드 연결의 핵심 파일들

---

다음: [Week 9 - Pre-integration 심화](../week9/README.md)
