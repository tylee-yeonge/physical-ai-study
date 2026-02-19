# Week 10: g2o 심화 - Solver 구조와 BA 내부 동작 이해

> 🎯 **이번 주 목표**: g2o 내부에서 BA가 풀리는 과정을 단계별로 이해하기
> ⏰ **예상 시간**: 10시간
> 💡 **핵심 질문**: "g2o가 optimize()를 호출하면 내부에서 정확히 무슨 일이 일어날까?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | 데모 실행 | `basic.cpp` | `./basic` — Solver 비교, 희소성, Marginalization 시연 |
| 3 | 직접 구현 | `my_basic.cpp` | TODO 채우기 → `./my_basic` 으로 검증 |
| 4 | 심화 실습 | [PRACTICE.md](./PRACTICE.md) | Solver 조합 비교, setMarginalized 효과 실험 |
| 5 | 헤더 참고 | `g2o_advanced.h` | 함수 시그니처 + Doxygen 주석 확인 |

> ⚠️ **빌드 환경**: Ubuntu + g2o 필요. macOS에서는 이론 학습만 가능.
> 설치: `sudo apt install libg2o-dev` 또는 소스 빌드 참고

---

## 🌟 시작하기 전에

### Week 7에서 배운 것

**g2o로 BA 구현:**
```cpp
// Vertex, Edge를 만들고...
optimizer.initializeOptimization();
optimizer.optimize(10);  // 이 한 줄이면 최적화 끝!
```

### Week 9에서 배운 것

**BA 최적화 원리:**
```
H·Δx = b
→ Schur Complement로 점 소거
→ S·Δc = b' (작은 시스템)
→ Back-substitution으로 점 업데이트
```

### 이번 주의 질문

```
❓ optimize(10)이 호출되면 g2o 내부에서 뭐가 일어나지?
❓ BlockSolver_6_3에서 6과 3은 뭘 의미하지?
❓ LinearSolverEigen vs LinearSolverCholmod, 뭐가 다르지?
❓ ORB-SLAM은 g2o를 어떻게 쓰고 있을까?
```

**이번 주에 답합니다!**

---

## 📚 핵심 개념

### 1. g2o 아키텍처 전체 그림

#### 1.1 계층 구조

```
g2o의 핵심 3계층:

┌─────────────────────────────────────────┐
│           SparseOptimizer               │  ← 최상위: 그래프 관리 + 반복
│  (Vertex/Edge 관리, optimize() 호출)    │
├─────────────────────────────────────────┤
│           BlockSolver                   │  ← 중간: Hessian 블록 구성
│  (H, b 구성, Schur Complement 적용)    │
├─────────────────────────────────────────┤
│           LinearSolver                  │  ← 최하위: 선형 시스템 풀기
│  (S·Δc = b' 를 실제로 푸는 방법)       │
└─────────────────────────────────────────┘
```

**비유:**
```
SparseOptimizer = 프로젝트 매니저
  "전체 최적화 프로세스를 관리"

BlockSolver = 설계 엔지니어
  "문제를 블록으로 나누고 Schur Complement 적용"

LinearSolver = 계산 담당
  "실제 선형 시스템을 풀어서 답 구함"
```

#### 1.2 코드에서의 설정

```cpp
// Week 7에서 쓴 이 코드, 이제 의미를 알 수 있다!

// 3계층: 아래에서 위로 구성
// Layer 1: LinearSolver (선형 시스템 풀기)
auto linearSolver = g2o::make_unique<
    g2o::LinearSolverEigen<g2o::BlockSolver_6_3::PoseMatrixType>>();
//  ^^^^^^^^^^^^^^^^^                       ^^^^^^^^^^^^^^^^^^^
//  어떤 방법으로 풀기?            Schur 후 행렬 타입

// Layer 2: BlockSolver (블록 구조 + Schur)
auto blockSolver = g2o::make_unique<g2o::BlockSolver_6_3>(
//                                  ^^^^^^^^^^^^^^^^
//                                  6: 카메라 차원 (SE3)
//                                  3: 점 차원 (XYZ)
    std::move(linearSolver));

// Layer 3: Algorithm (반복 전략)
auto algorithm = new g2o::OptimizationAlgorithmLevenberg(
//                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                   Levenberg-Marquardt 방법
    std::move(blockSolver));

// 최상위: SparseOptimizer
g2o::SparseOptimizer optimizer;
optimizer.setAlgorithm(algorithm);
```

---

### 2. optimize() 내부 파이프라인

#### 2.1 전체 흐름

```
optimize(10) 호출!

┌─────────────────────────────────────────┐
│ for iteration = 1 to 10:                │
│                                         │
│   ① computeActiveErrors()              │
│      → 모든 Edge의 잔차(residual) 계산  │
│                                         │
│   ② linearizeOplus()                   │
│      → 모든 Edge의 자코비안 계산        │
│                                         │
│   ③ buildSystem()                      │
│      → J^T·J = H, -J^T·e = b 구성     │
│                                         │
│   ④ solver->solve()                    │
│      → Schur Complement 적용            │
│      → S·Δc = b' 풀기                  │
│      → Back-substitution으로 Δp         │
│                                         │
│   ⑤ update()                           │
│      → x_new = x_old ⊕ Δx             │
│                                         │
│   ⑥ 수렴 판정                          │
│      → cost 감소량이 임계값 이하? → 종료│
│                                         │
└─────────────────────────────────────────┘
```

#### 2.2 각 단계와 Week 9 이론 매핑

```
g2o 내부 단계          │  Week 9 이론
───────────────────────┼──────────────────
① computeActiveErrors │  e(x) 계산
② linearizeOplus      │  J = ∂e/∂x (자코비안)
③ buildSystem         │  H = J^T·J, b = -J^T·e
④ solver->solve       │  Schur: S·Δc = b'
⑤ update              │  x += Δx
⑥ 수렴 판정           │  비용 함수 감소 확인
```

#### 2.3 각 단계 상세

**① computeActiveErrors()**
```cpp
// g2o 내부 (간략화)
for (Edge* edge : activeEdges) {
    edge->computeError();
    // 예: EdgeProjectXYZ2UV의 경우
    // error = observed_uv - project(camera_pose, point_3d)
}
```

**② linearizeOplus()**
```cpp
// g2o 내부 (간략화)
for (Edge* edge : activeEdges) {
    edge->linearizeOplus();
    // _jacobianOplusXi = ∂e/∂point  (2×3)
    // _jacobianOplusXj = ∂e/∂camera (2×6)
}
```

**③ buildSystem()**
```
각 Edge의 자코비안을 모아서 전체 H, b 구성:

H += Jₖ^T · Ωₖ · Jₖ    (Ω = 정보 행렬)
b += -Jₖ^T · Ωₖ · eₖ
```

**④ solver->solve() ← BlockSolver가 담당!**
```
BlockSolver가 하는 일:

1. H를 블록으로 분할:
   [Hcc  Hcp]
   [Hpc  Hpp]

2. marginalized된 변수 (점) 소거:
   S = Hcc - Hcp * Hpp^(-1) * Hpc  ← Schur!

3. LinearSolver에게 S·Δc = b' 전달

4. Back-substitution:
   Δp = Hpp^(-1) * (bp - Hpc * Δc)
```

---

### 3. BlockSolver 심층 분석

#### 3.1 BlockSolverTraits

```cpp
g2o::BlockSolver<g2o::BlockSolverTraits<6, 3>>
//                                      ^  ^
//                                      |  |
//                           카메라 차원  점 차원

// 의미:
// 6 = SE3 포즈의 자유도 (3 회전 + 3 평행이동)
// 3 = 3D 점의 자유도 (x, y, z)
```

**다른 설정도 가능:**
```cpp
// 2D SLAM (포즈 그래프)
BlockSolverTraits<3, 3>  // 3: SE2 (x, y, θ), 3: 2D 점

// 3D SLAM (포즈 그래프, 점 없음)
BlockSolverTraits<6, 6>  // 6: SE3, 6: SE3 (포즈-포즈)

// 가변 크기 (범용)
BlockSolverTraits<-1, -1>  // 동적 크기
```

**별칭들:**
```cpp
typedef BlockSolver<BlockSolverTraits<6, 3>> BlockSolver_6_3;
typedef BlockSolver<BlockSolverTraits<3, 2>> BlockSolver_3_2;
typedef BlockSolver<BlockSolverTraits<-1, -1>> BlockSolverX;
```

#### 3.2 setMarginalized()의 내부 동작

**setMarginalized(true)를 호출하면:**
```
1. BlockSolver가 Vertex를 두 그룹으로 분류:
   - Group 0: Non-marginalized (카메라) → Schur 후에도 남음
   - Group 1: Marginalized (점) → Schur로 소거됨

2. buildSystem() 시:
   Hessian을 [Hcc, Hcp; Hpc, Hpp] 블록으로 자동 분할

3. solve() 시:
   - Hpp 역행렬 계산 (블록 대각)
   - Schur complement S 계산
   - LinearSolver로 S·Δc = b' 풀기
   - Back-substitution으로 Δp 계산
```

**setMarginalized(false)라면:**
```
전체 H·Δx = b를 그대로 풀어야 함
→ 행렬 크기: (6N + 3M) × (6N + 3M)
→ 훨씬 느림!
```

---

### 4. LinearSolver 타입 비교

#### 4.1 사용 가능한 LinearSolver

```
g2o에서 제공하는 LinearSolver들:

┌────────────────────────┬──────────────────────────────────┐
│ LinearSolver           │ 특징                             │
├────────────────────────┼──────────────────────────────────┤
│ LinearSolverEigen      │ Eigen 내장, 추가 설치 불필요     │
│                        │ 소규모~중규모 문제에 적합        │
├────────────────────────┼──────────────────────────────────┤
│ LinearSolverDense      │ Dense 행렬 (QR/LU 분해)         │
│                        │ 아주 작은 문제에만 적합          │
├────────────────────────┼──────────────────────────────────┤
│ LinearSolverCholmod    │ SuiteSparse CHOLMOD 사용         │
│                        │ 대규모 희소 문제에 최적          │
├────────────────────────┼──────────────────────────────────┤
│ LinearSolverCSparse    │ CSparse 사용                     │
│                        │ 중규모 희소 문제에 적합          │
├────────────────────────┼──────────────────────────────────┤
│ LinearSolverPCG        │ Preconditioned Conjugate Gradient│
│                        │ 매우 큰 문제 (반복법)            │
└────────────────────────┴──────────────────────────────────┘
```

#### 4.2 언제 어떤 것을 선택?

```
문제 규모에 따른 선택 가이드:

카메라 <10개, 점 <100개  → LinearSolverDense 또는 Eigen
  (작은 문제, 뭘 써도 빠름)

카메라 10~100개, 점 100~10,000개 → LinearSolverEigen
  (중규모, Eigen으로 충분)

카메라 100+개, 점 10,000+개 → LinearSolverCholmod
  (대규모, 희소 구조 활용 필수)

카메라 1,000+개 → LinearSolverPCG
  (초대규모, 직접법 불가, 반복법 필요)
```

#### 4.3 코드 예시

```cpp
// 1. LinearSolverEigen (기본, 추가 설치 불필요)
auto linearSolver = g2o::make_unique<
    g2o::LinearSolverEigen<BlockSolverType::PoseMatrixType>>();

// 2. LinearSolverDense (작은 문제)
auto linearSolver = g2o::make_unique<
    g2o::LinearSolverDense<BlockSolverType::PoseMatrixType>>();

// 3. LinearSolverCholmod (대규모, libcholmod 필요)
auto linearSolver = g2o::make_unique<
    g2o::LinearSolverCholmod<BlockSolverType::PoseMatrixType>>();

// 4. LinearSolverCSparse (중규모, libcsparse 필요)
auto linearSolver = g2o::make_unique<
    g2o::LinearSolverCSparse<BlockSolverType::PoseMatrixType>>();

// 5. LinearSolverPCG (초대규모)
auto linearSolver = g2o::make_unique<
    g2o::LinearSolverPCG<BlockSolverType::PoseMatrixType>>();
```

---

### 5. g2o 최적화 알고리즘

#### 5.1 Levenberg-Marquardt vs Gauss-Newton

```
g2o에서 제공하는 알고리즘:

┌────────────────────────────────────┬──────────────────────┐
│ OptimizationAlgorithmLevenberg     │ (H + λI)·Δx = b     │
│                                    │ λ 자동 조절          │
│                                    │ 안정적, 느림         │
├────────────────────────────────────┼──────────────────────┤
│ OptimizationAlgorithmGaussNewton   │ H·Δx = b             │
│                                    │ damping 없음         │
│                                    │ 빠르지만 불안정      │
├────────────────────────────────────┼──────────────────────┤
│ OptimizationAlgorithmDogleg        │ Trust region 방법    │
│                                    │ GN과 GD 사이 전환    │
│                                    │ 균형적               │
└────────────────────────────────────┴──────────────────────┘
```

**일반적 선택:**
```
BA → OptimizationAlgorithmLevenberg (가장 안정적)
Pose Graph → OptimizationAlgorithmGaussNewton (빠름)
```

#### 5.2 코드에서의 선택

```cpp
// Levenberg-Marquardt (권장)
auto solver = new g2o::OptimizationAlgorithmLevenberg(
    std::move(blockSolver));

// Gauss-Newton (빠르지만 주의)
auto solver = new g2o::OptimizationAlgorithmGaussNewton(
    std::move(blockSolver));

// Dogleg
auto solver = new g2o::OptimizationAlgorithmDogleg(
    std::move(blockSolver));
```

---

### 6. Robust Kernel 심화

#### 6.1 왜 필요한가?

```
문제: 특징점 매칭에서 outlier 발생

정상 관측: error = 0.5 pixel → error² = 0.25
Outlier:   error = 50 pixel  → error² = 2500 ← 지배적!

→ Outlier 하나가 전체 최적화를 망침
→ Robust kernel로 큰 오차의 영향 제한
```

#### 6.2 g2o에서 제공하는 Kernel들

```
┌──────────────────────┬───────────────────────────────────┐
│ RobustKernelHuber    │ 가장 많이 사용                    │
│                      │ 작은 오차: 제곱, 큰 오차: 선형    │
├──────────────────────┼───────────────────────────────────┤
│ RobustKernelCauchy   │ 더 강한 suppression               │
│                      │ 극단적 outlier에 효과적           │
├──────────────────────┼───────────────────────────────────┤
│ RobustKernelTukey    │ 임계값 이상 완전 무시             │
│                      │ outlier를 아예 배제               │
├──────────────────────┼───────────────────────────────────┤
│ RobustKernelSaturation│ 오차에 상한 설정                 │
│                      │ 단순하고 직관적                   │
└──────────────────────┴───────────────────────────────────┘
```

#### 6.3 delta 파라미터 선택

```cpp
g2o::RobustKernelHuber* rk = new g2o::RobustKernelHuber;
rk->setDelta(1.0);  // delta = ?
edge->setRobustKernel(rk);

// delta 선택 기준:
// - 재투영 오차 기준: 보통 1~2 pixel
// - chi-square 기준: 5.991 (95% 신뢰도, 2 DoF)
//
// 실무 팁:
// - ORB-SLAM: sqrt(5.991) ≈ 2.45 pixel
// - 보수적: 1.0 pixel
// - 공격적: 3.0 pixel
```

---

### 7. 실제 SLAM에서의 g2o

#### 7.1 ORB-SLAM에서의 g2o 활용

```
ORB-SLAM2/3은 g2o를 핵심 백엔드로 사용:

1. Tracking (매 프레임)
   - Motion-only BA: 카메라 포즈만 최적화
   - 점은 고정 (setFixed(true))

2. Local Mapping
   - Local BA: 최근 키프레임 + 관련 점 최적화
   - setMarginalized(true)로 Schur 적용

3. Loop Closing
   - Pose Graph Optimization: SE3 포즈만
   - 점 없이 포즈 간 상대 제약
   - Global BA: 전체 맵 최적화
```

#### 7.2 ORB-SLAM의 Solver 설정

```cpp
// ORB-SLAM2의 Local BA (Optimizer.cc)
typedef g2o::BlockSolver<g2o::BlockSolverTraits<6, 3>> BlockSolver_6_3;
typedef g2o::LinearSolverEigen<BlockSolver_6_3::PoseMatrixType> LinearSolverType;

// ORB-SLAM2의 Pose Graph (Loop Closing)
typedef g2o::BlockSolver<g2o::BlockSolverTraits<7, 3>> BlockSolver_7_3;
//                                               ^
//                                            7 = Sim(3)!
//                                            (스케일 자유도 추가)
```

#### 7.3 g2o vs Ceres 선택 기준

```
g2o를 선택하는 경우:
  ✅ 그래프 기반 SLAM (Vertex/Edge가 자연스러움)
  ✅ 포즈 그래프 최적화
  ✅ SE3, Sim(3) 등 SLAM 전용 타입 내장
  ✅ ORB-SLAM, LSD-SLAM 등 많은 SLAM이 사용

Ceres를 선택하는 경우:
  ✅ 자동미분 (복잡한 오차 함수에 편리)
  ✅ 범용 최적화 (SLAM 외에도 사용)
  ✅ VIO에서 다양한 factor 조합
  ✅ VINS-Mono, Cartographer 등이 사용

핵심 차이:
  g2o:   그래프 구조 명확, 수동 자코비안
  Ceres: 범용적, 자동 자코비안
```

---

## 📝 이번 주 실습

### 실습 구성

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | Solver 조합 비교 실험 | 필수 | 3시간 |
| 2 | setMarginalized() 효과 실험 | 필수 | 3시간 |
| 3 | g2o 예제 심층 분석 | 필수 | 2시간 |
| 4 | 더 큰 BA 문제 실험 | 선택 | 2시간 |

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

---

## ✅ 체크리스트

### 아키텍처 이해
- [ ] SparseOptimizer → BlockSolver → LinearSolver 계층 설명 가능
- [ ] BlockSolverTraits<6,3>에서 6과 3의 의미 설명 가능
- [ ] optimize() 호출 시 6단계 파이프라인 설명 가능

### 내부 동작 이해
- [ ] setMarginalized(true)가 트리거하는 Schur Complement 과정 이해
- [ ] LinearSolver 타입별 특징과 선택 기준 설명 가능
- [ ] Levenberg-Marquardt vs Gauss-Newton 차이 이해

### 실무 연결
- [ ] ORB-SLAM에서 g2o가 어떻게 사용되는지 구조적 이해
- [ ] g2o vs Ceres 선택 기준 설명 가능
- [ ] Robust Kernel의 delta 파라미터 의미 이해

### 실습 완료
- [ ] Step 1: Solver 조합 비교 실험
- [ ] Step 2: setMarginalized() 효과 실험
- [ ] Step 3: g2o 예제 분석
- [ ] Step 4: 큰 BA 문제 실험 (선택)

---

## 🎯 핵심 요약

### 1. g2o 3계층 구조

```
SparseOptimizer → 그래프 관리, 반복 제어
BlockSolver     → Hessian 블록 구성, Schur 적용
LinearSolver    → 선형 시스템 풀기
```

### 2. optimize() 파이프라인

```
① 잔차 계산 → ② 자코비안 → ③ H,b 구성
→ ④ Schur + 풀기 → ⑤ 업데이트 → ⑥ 수렴?
```

### 3. BlockSolverTraits<6,3>

```
6 = 카메라 포즈 (SE3, 6 DoF)
3 = 3D 점 (XYZ, 3 DoF)
→ Hessian을 6×6, 3×3 블록으로 분할
```

### 4. LinearSolver 선택

```
작은 문제 → Dense 또는 Eigen
중규모    → Eigen 또는 CSparse
대규모    → Cholmod
초대규모  → PCG
```

### 5. SLAM에서의 활용

```
ORB-SLAM: g2o (그래프 구조 활용)
VINS:     Ceres (자동미분, VIO factor)
→ 둘 다 내부적으로 Schur Complement 사용!
```

---

## 🔗 다음 단계

Phase 3 계속:
- Week 11: Ceres 실습
- Week 12-13: 스케일 문제 이해

다음: [Week 11 - Ceres 실습](../week11/README.md)
