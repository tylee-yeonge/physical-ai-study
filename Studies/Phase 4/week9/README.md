# Week 9: Pre-integration 심화 - 공분산 전파와 VINS 코드

이전: [Week 8 - Factor Graph에서의 역할](../week8/README.md)

> 🎯 **이번 주 목표**: Pre-integration 공분산 전파 이해 + VINS 코드 분석
> ⏰ **예상 시간**: 10시간
> 💡 **핵심 질문**: "Pre-integration의 불확실성은 어떻게 계산할까?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | 공분산 전파 개념과 A/B 행렬 기초 문제 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | Pre-integration 공분산 전파 구현 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | 공분산 전파와 VINS 코드 분석 실습 |

---

## 🌟 시작하기 전에

### Week 7-8 복습

```
Week 7: Pre-integration 수식
  → ΔR, Δv, Δp 계산 방법

Week 8: Factor Graph에서의 역할
  → IMU Factor: r = 측정 - 추정, 비용 = r^T·Σ^{-1}·r

이번 주 질문:
  "Σ^{-1}의 Σ는 어떻게 구하지?"
  → Pre-integration 공분산 전파!
```

### 비유: 여행 불확실성

```
서울 → 부산 버스 여행에서:

1시간 후: "아마 대전 근처" (불확실성 작음)
3시간 후: "대구 근처... 아마..." (불확실성 중간)
5시간 후: "부산... 인 것 같은데..." (불확실성 큼)

→ 시간이 지날수록 불확실성 누적!
→ Pre-integration도 동일: IMU 데이터가 쌓일수록 Σ 증가
→ 이 Σ가 Factor의 가중치를 결정
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. 왜 공분산 전파가 필요한가?

```
Pre-integration 결과: ΔR, Δv, Δp

하지만 이 값들은 얼마나 정확할까?
  → IMU 노이즈 때문에 불확실성 존재
  → 적분 시간이 길수록 불확실성 증가
  → Factor Graph에서 가중치를 정하려면 Σ 필요!

공분산 Σ가 없으면:
  → 0.1초 적분이나 5초 적분이나 같은 가중치
  → 비합리적! 0.1초 적분이 훨씬 정확해야 함

공분산 Σ가 있으면:
  → 0.1초: Σ 작음 → 큰 가중치 (신뢰도 높음)
  → 5초: Σ 큼 → 작은 가중치 (신뢰도 낮음)
```

---

### 2. Error State 기반 공분산 전파

```
Pre-integration의 Error State:
  δx = [δφ, δv, δp]  (9차원)
    δφ: 회전 오차 (3D)
    δv: 속도 오차 (3D)
    δp: 위치 오차 (3D)

Error 전파 방정식:
  δx_{k+1} = A_k · δx_k + B_k · n_k

  A_k: Error State 전이 행렬 (9×9)
  B_k: 노이즈 입력 행렬 (9×6)
  n_k: IMU 노이즈 [n_a(3), n_g(3)]
```

#### A_k 행렬 (Error State 전이)

**직관: 각 블록이 의미하는 것**

```
A_k는 "현재 시점의 오차가 다음 시점에 어떻게 전파되는가"를 기술:

  -[ω]×     → 회전 오차가 다음 회전 오차에 영향 (자이로 동역학)
  -ΔR·[a]×  → 회전 오차가 속도 오차에 영향!
               (방향이 틀리면 가속도를 잘못된 방향으로 적분)
  I·dt      → 속도 오차가 위치 오차에 직접 전파 (v·dt = p)

핵심: 회전 오차 → 속도 오차 → 위치 오차로 연쇄 전파!
      이것이 IMU 적분에서 위치 오차가 t²에 비례하여 커지는 이유
```

```
A_k = | -[ω]×    0        0     |
      | -ΔR·[a]×  I       0     | · dt + I
      |  0       I·dt     I     |

여기서:
  [ω]× = skew(ω_m - b_g)  (3×3)
  [a]× = skew(a_m - b_a)  (3×3)
  ΔR = 현재까지의 상대 회전
```

#### B_k 행렬 (노이즈 입력)

```
B_k = | -I·dt    0     |
      |  0     -ΔR·dt  |  (9×6)
      |  0      0      |

n_k = [n_g(3), n_a(3)]  (6D)
```

---

### 3. 공분산 재귀 업데이트

```
매 IMU 데이터마다:

  Σ_{k+1} = A_k · Σ_k · A_k^T + B_k · Q · B_k^T

  Σ_k: 현재까지의 공분산 (9×9)
  Q: IMU 노이즈 공분산 (6×6)
    Q = diag(σ²_gyro·I₃, σ²_acc·I₃)

초기: Σ_0 = 0 (시작점은 확실)
```

```cpp
// 공분산 전파 구현
void propagateCovariance(
    const Eigen::Matrix3d& delta_R,
    const Eigen::Vector3d& acc,    // a_m - b_a
    const Eigen::Vector3d& gyro,   // ω_m - b_g
    double dt,
    double sigma_acc,   // 가속도 노이즈
    double sigma_gyro,  // 자이로 노이즈
    Eigen::Matrix<double, 9, 9>& Sigma)
{
    // A 행렬
    Eigen::Matrix<double, 9, 9> A = Eigen::Matrix<double, 9, 9>::Identity();
    A.block<3,3>(0,0) += -skew(gyro) * dt;           // 회전-회전
    A.block<3,3>(3,0) = -delta_R * skew(acc) * dt;    // 속도-회전
    A.block<3,3>(6,3) = Eigen::Matrix3d::Identity() * dt; // 위치-속도

    // B 행렬
    Eigen::Matrix<double, 9, 6> B = Eigen::Matrix<double, 9, 6>::Zero();
    B.block<3,3>(0,0) = -Eigen::Matrix3d::Identity() * dt;  // 회전-자이로노이즈
    B.block<3,3>(3,3) = -delta_R * dt;                        // 속도-가속도노이즈

    // 노이즈 공분산
    Eigen::Matrix<double, 6, 6> Q = Eigen::Matrix<double, 6, 6>::Zero();
    Q.block<3,3>(0,0) = Eigen::Matrix3d::Identity() * sigma_gyro * sigma_gyro;
    Q.block<3,3>(3,3) = Eigen::Matrix3d::Identity() * sigma_acc * sigma_acc;

    // 전파
    Sigma = A * Sigma * A.transpose() + B * Q * B.transpose();
}
```

---

### 4. 공분산의 물리적 의미

```
Σ (9×9 공분산):

  Σ = | Σ_φφ  Σ_φv  Σ_φp |
      | Σ_vφ  Σ_vv  Σ_vp |
      | Σ_pφ  Σ_pv  Σ_pp |

Σ_φφ (3×3): 회전 불확실성
  → 자이로 노이즈 축적
  → 시간에 비례 증가 (랜덤 워크)

Σ_vv (3×3): 속도 불확실성
  → 가속도 노이즈 축적
  → 회전 오차도 영향 (가속도 방향 불확실)

Σ_pp (3×3): 위치 불확실성
  → 속도 불확실성 적분
  → 시간의 제곱에 비례 증가!
  → 왜 t²? 노이즈→속도 오차(∝t) →위치 오차는 속도를 다시 적분(∝t²)

교차 항 (Σ_φv, Σ_φp 등): 상관관계
  → 회전 오차가 속도/위치에 영향
```

---

### 5. On-Manifold Pre-integration (개념)

```
기존 Pre-integration:
  ΔR을 행렬(SO(3))로 유지, 나머지는 R^n

On-Manifold Pre-integration (Forster 2017):
  SO(3) × R³ × R³ 위에서 직접 적분
  → 수학적으로 더 엄밀
  → 실용적 차이는 크지 않음

핵심 차이:
  기존: 공분산을 유클리드 공간에서 전파
  On-Manifold: 공분산을 매니폴드의 접선 공간에서 전파
  → SO(3)의 비선형 구조를 더 정확히 반영

지금 단계에서는:
  → 기존 방식을 정확히 이해하는 것이 우선!
  → On-Manifold는 "더 엄밀한 버전이 있다" 정도로 알기
```

---

### 6. VINS 코드 분석

```
VINS-Mono 핵심 파일:

1. integration_base.h (Pre-integration 클래스)
   ─────────────────────────────
   class IntegrationBase {
     // 핵심 멤버
     Quaterniond delta_q;    // ΔR (쿼터니언으로 저장)
     Vector3d delta_v;       // Δv
     Vector3d delta_p;       // Δp
     Matrix<double,15,15> jacobian;  // 바이어스 자코비안
     Matrix<double,15,15> covariance; // 공분산

     // 핵심 함수
     void push_back(dt, acc, gyro)  // IMU 데이터 추가
     void propagate(dt, acc, gyro)  // 적분 수행
     void midPointIntegration(...)  // 중간점 적분
     void repropagate(new_ba, new_bg) // 바이어스 변경 시 재적분
   };

   Note: VINS는 15D (바이어스 포함)
     [δφ(3), δp(3), δv(3), δba(3), δbg(3)]

2. imu_factor.h (IMU Factor)
   ─────────────────────────────
   class IMUFactor : public CostFunction {
     Evaluate():
       // 잔차 계산 (Week 8에서 배운 것)
       r_rotation = ...
       r_velocity = ...
       r_position = ...
       r_ba = ba_j - ba_i  // 바이어스 변화
       r_bg = bg_j - bg_i
   };

3. projection_factor.h (Visual Factor)
   ─────────────────────────────
   class ProjectionFactor : public CostFunction {
     Evaluate():
       // 재투영 오차 계산
   };
```

---

## 💡 꼭 이해해야 할 핵심 개념

### IMU 노이즈와 공분산의 관계

```
IMU 사양서의 파라미터:
  acc_n: 가속도 noise density [m/s²/√Hz]
  gyr_n: 자이로 noise density [rad/s/√Hz]
  acc_w: 가속도 bias random walk [m/s³/√Hz]
  gyr_w: 자이로 bias random walk [rad/s²/√Hz]

이산화:
  σ_acc = acc_n / √dt
  σ_gyro = gyr_n / √dt

  또는 (표기법에 따라):
  Q_acc = acc_n² / dt · I₃
  Q_gyro = gyr_n² / dt · I₃
```

### 공분산 크기의 시간 의존성

```
적분 시간 0.1초 (IMU 20개):
  Σ_position 작음 → 높은 가중치

적분 시간 1.0초 (IMU 200개):
  Σ_position 약 100배 큼 → 낮은 가중치

→ 키프레임 간격이 짧을수록 IMU Factor 신뢰도 높음
→ 이것이 VINS에서 키프레임 간격을 적절히 유지하는 이유
```

---

## 🔍 자체 점검 - 이해했는지 확인!

### Q1: 공분산이 필요한 이유
**Q:** Pre-integration에서 공분산 Σ를 계산하지 않으면?

**A:** Factor Graph에서 IMU Factor의 가중치를 정할 수 없습니다. 짧은 적분이든 긴 적분이든 같은 가중치가 되어 비합리적인 최적화가 됩니다.

### Q2: 공분산 증가
**Q:** 왜 적분 시간이 길어질수록 공분산이 커지나요?

**A:** IMU 노이즈가 매 스텝 누적되기 때문입니다. Σ_{k+1} = A·Σ_k·A^T + B·Q·B^T에서 뒤의 항이 매 스텝 추가됩니다.

### Q3: VINS 15D vs 기본 9D
**Q:** VINS에서 공분산이 15×15인 이유는?

**A:** 9D (δφ, δv, δp) + 6D (δb_a, δb_g) = 15D. 바이어스의 불확실성도 같이 추적하기 때문입니다.

### Q4: On-Manifold
**Q:** On-Manifold Pre-integration이 기존 방식보다 나은 점은?

**A:** SO(3)의 비선형 구조를 수학적으로 더 엄밀하게 처리합니다. 실용적으로는 큰 차이가 없지만, 이론적으로 더 정확합니다.

---

## 📝 이번 주 실습 & 다음 주 준비

### 이번 주 체크리스트

- [ ] Error State 기반 공분산 전파 방정식 이해
- [ ] A_k, B_k 행렬 구성 이해
- [ ] Σ_{k+1} = A·Σ·A^T + B·Q·B^T 재귀 업데이트 이해
- [ ] 공분산이 Factor 가중치를 결정하는 원리 이해
- [ ] VINS integration_base.h 구조 파악
- [ ] `PRACTICE.md`, `quiz_easy.cpp`, `quiz_medium.cpp` 완료

### 다음 주 미리보기: IMU 적분 실습

```
Week 10에서는 실제 데이터로 실습!
  → EuRoC 데이터셋의 IMU 데이터 사용
  → 순수 IMU 적분으로 드리프트 관찰
  → Pre-integration과 직접 적분 비교
```

---

## 🎯 이번 주 핵심 요약

1. **공분산 전파가 필요한 이유**
   - Pre-integration의 불확실성 정량화
   - Factor Graph에서 적절한 가중치 부여

2. **Error State 기반 전파**
   - Σ_{k+1} = A·Σ·A^T + B·Q·B^T
   - A: Error State 전이, B: 노이즈 입력

3. **공분산의 물리적 의미**
   - Σ_φφ: 회전 불확실성 (∝ 시간)
   - Σ_pp: 위치 불확실성 (∝ 시간²)
   - 교차항: 상관관계

4. **On-Manifold Pre-integration**
   - SO(3)에서 더 엄밀한 처리
   - 개념만 이해, 기존 방식 우선 학습

5. **VINS 코드 구조**
   - integration_base.h: 적분 + 공분산
   - imu_factor.h: Factor 잔차
   - 15D 공분산 (바이어스 포함)

---

다음: [Week 10 - 실습: IMU 적분](../week10/README.md)
