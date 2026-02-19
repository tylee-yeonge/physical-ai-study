# Week 9: BA 최적화 기법 - Schur Complement와 희소 구조

> 🎯 **이번 주 목표**: BA가 왜 빠른지 원리 이해하기
> ⏰ **예상 시간**: 10시간
> 💡 **핵심 질문**: "왜 BA는 수천 개 변수도 빠르게 최적화할 수 있을까?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 데모 실행 | `basic.cpp` | 빌드 후 `./basic` 실행, Schur 과정 관찰 |
| 2 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 3 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | 희소 구조, Schur Complement, Local vs Global BA |
| 4 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | Schur 계산량 분석, VINS sliding window |
| 5 | 직접 구현 (Step 1-2) | `my_basic.cpp` | build_hessian, extract_blocks 구현 |
| 6 | 직접 구현 (Step 3-5) | `my_basic.cpp` | schur_complement, solve_reduced, back_substitute |
| 7 | 실습 | [PRACTICE.md](./PRACTICE.md) | Schur Complement 직접 계산 및 옵션 비교 실험 |

### 퀴즈 ↔ basic.cpp 매핑

| 퀴즈 | 문제 | basic.cpp 교육 블록 |
|:----:|------|-------------------|
| easy Q1 | 자코비안이 희소한 이유 | `demoSparseStructure()` — 관측 구조 |
| easy Q2 | Schur에서 먼저 소거하는 변수 | `demoSchurComplement()` — 점 소거 |
| easy Q3 | Local BA vs Global BA | `demoLocalVsGlobal()` — 범위 비교 |
| easy Q4 | Hpp 블록 대각 특징 | `demoSparseStructure()` — 비대각 norm |
| easy Q5 | g2o setMarginalized | `demoGaugeAndMarginalize()` — Schur 연결 |
| easy Q6 | Gauge freedom | `demoGaugeAndMarginalize()` — 7 DoF |
| medium Q1 | Schur 계산량 감소 원리 | `demoSchurComplement()` — 크기 비교 |
| medium Q2 | Local BA 장단점 | `demoLocalVsGlobal()` — 실시간성 vs 드리프트 |
| medium Q3 | VINS sliding window | `demoLocalVsGlobal()` — Local BA 변형 |

---

## 🌟 시작하기 전에

### Week 7/8에서 배운 것

**g2o로 BA:**
```cpp
v->setMarginalized(true);  // 이 한 줄이면 빨라진다!
```

**Ceres로 BA:**
```cpp
options.linear_solver_type = ceres::DENSE_SCHUR;  // Schur 사용!
```

**하지만 궁금하지 않았나요?**
```
❓ setMarginalized(true)가 내부에서 뭘 하길래 빨라지지?
❓ DENSE_SCHUR가 뭔데?
❓ 왜 점을 "marginalize" 한다고 표현하지?
```

**이번 주에 답합니다!**

---

## 📚 핵심 개념

### 1. 비선형 최적화 복습

#### 1.1 BA 문제 정의

```
목표: 재투영 오차 최소화

min Σᵢⱼ ||uᵢⱼ - π(Tᵢ, Xⱼ)||²

여기서:
- uᵢⱼ: 카메라 i에서 점 j의 관측 (픽셀)
- Tᵢ: 카메라 i의 포즈 (6 DoF)
- Xⱼ: 3D 점 j의 위치 (3 DoF)
- π: 투영 함수
```

#### 1.2 Gauss-Newton 방법

**비선형 → 선형 근사:**
```
오차 함수 e(x) ≈ e(x₀) + J·Δx

J = ∂e/∂x  (자코비안)
```

**정규 방정식 (Normal Equation):**
```
H·Δx = b

여기서:
  H = J^T·J     (Hessian 근사, 정방행렬)
  b = -J^T·e    (gradient)
  Δx = ?        (우리가 구할 업데이트량)
```

**반복:**
```
x_new = x_old + Δx
→ 수렴할 때까지 반복
```

#### 1.3 Levenberg-Marquardt

**Gauss-Newton의 문제:**
```
H가 특이(singular)하거나 조건이 나쁘면 발산!
```

**해결: Damping 추가:**
```
(H + λI)·Δx = b

λ가 크면: gradient descent처럼 (안전하지만 느림)
λ가 작으면: Gauss-Newton처럼 (빠르지만 불안정)

→ λ를 자동 조절!
```

---

### 2. 자코비안 행렬 구조 (코드 중심)

#### 2.1 자코비안이란?

```
자코비안 = 오차가 변수에 대해 얼마나 민감한지

예: 카메라를 오른쪽으로 1cm 움직이면
    → 재투영 오차가 몇 픽셀 변하나?
```

#### 2.2 블록 구조

**하나의 관측에 대한 자코비안:**
```
e = observed_uv - projected_uv  (2차원 오차)

∂e/∂T: 2×6 행렬 (카메라 포즈에 대한 미분)
∂e/∂X: 2×3 행렬 (3D 점에 대한 미분)
```

**시각화:**
```
       카메라i    점j
       ↓          ↓
J = [ 2×6 블록 | 2×3 블록 ]  ← 하나의 관측
```

#### 2.3 코드에서 어떻게 구현?

**Ceres: 자동미분 (권장)**
```cpp
// 오차 함수만 정의하면 끝!
template <typename T>
bool operator()(const T* camera, const T* point, T* residuals) {
    // Ceres가 자동으로 ∂e/∂camera, ∂e/∂point 계산
}
```

**g2o: 수동 구현**
```cpp
void EdgeProjectXYZ2UV::linearizeOplus() {
    // ∂e/∂camera 직접 계산
    _jacobianOplusXi = ...;  // 2×3 (점)
    _jacobianOplusXj = ...;  // 2×6 (카메라)
}
```

**결론:**
- 수식 암기 필요 없음!
- Ceres 쓰면 자동, g2o는 이미 구현되어 있음
- 중요한 건 **블록 구조** 이해

---

### 3. Hessian 행렬의 희소 구조

#### 3.1 전체 자코비안 구조

**예: 3개 카메라, 5개 점**
```
관측:
  - 카메라0 → 점0, 점1, 점2
  - 카메라1 → 점1, 점2, 점3
  - 카메라2 → 점2, 점3, 점4

전체 자코비안 J:
       C0   C1   C2   P0   P1   P2   P3   P4
     [  ×    ·    ·    ×    ·    ·    ·    · ]  관측(C0,P0)
     [  ×    ·    ·    ·    ×    ·    ·    · ]  관측(C0,P1)
     [  ×    ·    ·    ·    ·    ×    ·    · ]  관측(C0,P2)
J =  [  ·    ×    ·    ·    ×    ·    ·    · ]  관측(C1,P1)
     [  ·    ×    ·    ·    ·    ×    ·    · ]  관측(C1,P2)
     [  ·    ×    ·    ·    ·    ·    ×    · ]  관측(C1,P3)
     [  ·    ·    ×    ·    ·    ×    ·    · ]  관측(C2,P2)
     [  ·    ·    ×    ·    ·    ·    ×    · ]  관측(C2,P3)
     [  ·    ·    ×    ·    ·    ·    ·    × ]  관측(C2,P4)

×: 값이 있음 (블록)
·: 0 (희소!)
```

#### 3.2 Hessian = J^T·J

**블록 구조:**
```
H = J^T · J = [ Hcc   Hcp ]
              [ Hpc   Hpp ]

Hcc: 카메라-카메라 블록 (6N × 6N)
Hpp: 점-점 블록 (3M × 3M) - 대각 블록!
Hcp: 카메라-점 블록 (6N × 3M)
Hpc: Hcp^T
```

**시각화:**
```
          카메라들        점들
         ┌─────────┬───────────────┐
카메라들 │   Hcc   │      Hcp      │
         │(밀집)   │   (희소)      │
         ├─────────┼───────────────┤
점들     │   Hpc   │      Hpp      │
         │(희소)   │  (블록 대각)  │
         └─────────┴───────────────┘

핵심: Hpp가 블록 대각 → 역행렬 계산 쉬움!
```

#### 3.3 왜 희소한가?

```
각 관측은 "하나의 카메라"와 "하나의 점"만 연결

→ 대부분의 카메라-점 쌍은 연결 없음
→ 자코비안에 0이 많음
→ Hessian도 희소!
```

---

### 4. Schur Complement (핵심!)

#### 4.1 문제: 변수가 너무 많다

```
일반적인 BA:
  - 100개 카메라: 600 변수 (6 × 100)
  - 10,000개 점: 30,000 변수 (3 × 10,000)
  - 총: 30,600 변수!

H·Δx = b를 풀려면?
  → 30,600 × 30,600 행렬의 역행렬... 💀
  → O(n³) = 너무 느림!
```

#### 4.2 핵심 아이디어: 점을 먼저 소거!

**원래 시스템:**
```
[ Hcc   Hcp ] [ Δc ]   [ bc ]
[ Hpc   Hpp ] [ Δp ] = [ bp ]

Δc: 카메라 업데이트 (작음, 600)
Δp: 점 업데이트 (큼, 30,000)
```

**2단계로 분리:**

**Step 1: 카메라만 푼다**
```
(Hcc - Hcp·Hpp⁻¹·Hpc)·Δc = bc - Hcp·Hpp⁻¹·bp
         ↑
    Schur Complement (S)

S·Δc = b'

S는 600×600 행렬! (30,600이 아니라!)
```

**Step 2: 점은 Back-substitution**
```
Δp = Hpp⁻¹·(bp - Hpc·Δc)

Hpp가 블록 대각이라 Hpp⁻¹ 계산이 쉬움!
```

#### 4.3 왜 빠른가?

```
일반 해법:
  H는 30,600 × 30,600
  → O(30,600³) ≈ 엄청 오래 걸림

Schur Complement:
  S는 600 × 600
  → O(600³) + O(M) ≈ 훨씬 빠름!

점이 많을수록 효과 큼!
```

#### 4.4 시각적 이해

```
Before Schur:
┌─────────────────────────────────────┐
│                                     │
│         30,600 × 30,600             │  ← 거대한 행렬
│              H                      │
│                                     │
└─────────────────────────────────────┘

After Schur:
┌───────┐
│ 600×  │  ← 작은 행렬만 풀면 됨!
│  600  │
│   S   │
└───────┘
```

#### 4.5 코드에서는?

**g2o:**
```cpp
v_point->setMarginalized(true);  // 점을 marginalize
// → 내부적으로 Schur complement 적용
```

**Ceres:**
```cpp
options.linear_solver_type = ceres::DENSE_SCHUR;
// 또는 ceres::SPARSE_SCHUR (대규모일 때)
```

**이게 전부!** 한 줄로 Schur Complement 적용

---

### 5. Marginalization (간략)

#### 5.1 Schur Complement = Marginalization

```
수학적으로 같은 연산!

Schur Complement:
  - 변수를 "소거"한다

Marginalization:
  - 변수를 "주변화"한다
  - 확률적 해석: 해당 변수를 적분해서 없앰
```

#### 5.2 VINS에서의 활용

```
Sliding Window:
  [KF1] [KF2] [KF3] [KF4] [KF5]
                            ↑ 현재

새 키프레임 추가 시:
  → 가장 오래된 KF1을 제거해야 함
  → 하지만 KF1의 정보는 보존하고 싶음!
  → Marginalization으로 정보 요약
```

**Phase 4에서 더 자세히 다룸!**

---

### 6. Local BA vs Global BA

#### 6.1 비교

| 구분 | Local BA | Global BA |
|------|----------|-----------|
| **범위** | 최근 N개 키프레임 | 전체 맵 |
| **변수 수** | 적음 (~100) | 많음 (~10,000+) |
| **속도** | 빠름 (실시간 가능) | 느림 |
| **정확도** | 지역적 일관성 | 전역적 일관성 |
| **사용 시점** | 매 키프레임 | Loop closure 후 |

#### 6.2 시각화

```
전체 맵:
[KF1]─[KF2]─[KF3]─...─[KF98]─[KF99]─[KF100]
                              └─────────────┘
                                  ↑
                            Local BA 범위
                            (최근 10개만)

Loop Closure 발생!
[KF1]─[KF2]─...─[KF100]
  └───────────────────┘
          ↑
    Global BA 필요
    (전체 최적화)
```

#### 6.3 VINS의 접근

```cpp
// VINS: Sliding Window = Local BA
const int WINDOW_SIZE = 10;

void optimization() {
    // 최근 10개 키프레임만 최적화
    for (int i = 0; i < WINDOW_SIZE; i++) {
        // 카메라, IMU, 점 추가
    }
    // Ceres로 최적화
}
```

---

### 7. VINS 코드와의 연결

#### 7.1 관련 파일 위치

```
VINS-Mono/
├── vins_estimator/src/
│   ├── estimator.cpp        # 메인 추정기
│   ├── factor/              # 각종 factor 정의
│   │   ├── projection_factor.cpp   # Visual factor (재투영)
│   │   └── marginalization_factor.cpp  # Marginalization
│   └── utility/
│       └── visualization.cpp
```

#### 7.2 핵심 함수

```cpp
// estimator.cpp
void Estimator::optimization() {
    // 1. Ceres Problem 생성
    ceres::Problem problem;

    // 2. 파라미터 블록 추가 (카메라 포즈, 속도, 바이어스)
    for (int i = 0; i < WINDOW_SIZE + 1; i++) {
        problem.AddParameterBlock(para_Pose[i], SIZE_POSE, ...);
    }

    // 3. Visual factor 추가 (BA!)
    for (auto &it_per_id : f_manager.feature) {
        // ProjectionFactor = 재투영 오차
        ProjectionFactor *f = new ProjectionFactor(...);
        problem.AddResidualBlock(f, loss_function, ...);
    }

    // 4. 최적화 실행
    ceres::Solve(options, &problem, &summary);
}
```

#### 7.3 Schur는 어디에?

```cpp
// estimator.cpp
ceres::Solver::Options options;
options.linear_solver_type = ceres::DENSE_SCHUR;
//                           ^^^^^^^^^^^^^^^^^
//                           여기서 Schur 적용!
```

---

## 📝 이번 주 실습

### 실습 구성

| Step | 내용 | 예상 시간 |
|------|------|----------|
| 1 | 간단한 블록 예제로 Schur 이해 | 3시간 |
| 2 | g2o/Ceres에서 옵션 비교 | 2시간 |
| 3 | VINS 코드 분석 (선택) | 2시간 |

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

---

## ✅ 체크리스트

### 개념 이해
- [ ] Gauss-Newton과 정규 방정식 H·Δx = b 이해
- [ ] 자코비안의 블록 구조 (2×6, 2×3) 이해
- [ ] Hessian이 희소한 이유 설명 가능
- [ ] Schur Complement 원리 설명 가능
- [ ] Local BA vs Global BA 차이 설명 가능

### 코드 이해
- [ ] g2o의 setMarginalized(true)가 하는 일 이해
- [ ] Ceres의 DENSE_SCHUR가 하는 일 이해
- [ ] VINS optimization.cpp 구조 파악

### 실습 완료
- [ ] Step 1: Schur Complement 직접 계산
- [ ] Step 2: g2o/Ceres 옵션 비교
- [ ] Step 3: VINS 코드 분석 (선택)

---

## 🎯 핵심 요약

### 1. BA 최적화의 핵심

```
문제: 변수가 너무 많다 (카메라 + 점)
해결: Schur Complement로 점을 소거

H·Δx = b  →  S·Δc = b'
           (30,600 → 600)
```

### 2. 왜 빠른가?

```
Hpp (점-점)가 블록 대각!
→ 역행렬 계산 쉬움
→ 점을 효율적으로 소거
→ 카메라만의 작은 시스템
```

### 3. 코드에서는 한 줄

```cpp
// g2o
v->setMarginalized(true);

// Ceres
options.linear_solver_type = ceres::DENSE_SCHUR;
```

### 4. Local vs Global

```
Local BA: 실시간, 최근 키프레임만
Global BA: Loop closure 후, 전체 맵
```

### 5. VINS 연결

```
VINS = Sliding Window + Local BA + Marginalization
     = 실시간 VIO의 핵심!
```

---

## 🔗 다음 단계

Week 9 완료 후:

- Week 10: g2o 심화
- Week 11: Ceres 실습
- Section 3.5: 스케일 문제 이해
- **Phase 4: VIO 개념**: IMU와 Vision의 상호보완 관계 이해

다음: [Week 10 - g2o 심화](../week10/README.md)
