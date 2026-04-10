# Week 8: 비선형 최적화 (Nonlinear Optimization)

## 📌 개요

> 🎯 **목표**: 반복적 최적화로 비선형 문제 풀기
> ⏱️ **예상 시간**: 이론 3시간 + 실습 2시간

실제 SLAM 문제는 대부분 **비선형**입니다. 카메라 투영, 회전 연산, 지수 함수 등이 모두 비선형이기 때문입니다. 이번 주에는 **Gauss-Newton**, **Levenberg-Marquardt** 알고리즘을 배우고, SLAM 실전 라이브러리인 **Ceres Solver**를 소개합니다.

### 🤔 왜 이걸 배워야 할까요?

**일상 비유**: 산에서 가장 낮은 골짜기 찾기

```
선형 문제:                비선형 문제:
   ╲       ╱                    ╱╲   ╱╲
    ╲     ╱                    ╱  ╲ ╱  ╲
     ╲   ╱                    ╱    ●    ╲
      ╲ ╱                    ╱   최소점   ╲
       ●                    ●              ●
    한 번에 찾음           여러 번 탐색 필요!
```

**SLAM에서의 필요성**:
- **재투영 오차**: 3D→2D 투영은 비선형
- **회전 행렬**: SO(3) 연산은 비선형
- **Bundle Adjustment**: 수천 개 변수 동시 최적화

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | Python 기초 실습 | `nonlinear_basics.py` | Gauss-Newton, 곡선 피팅 실습 |
| 3 | Python 심화 퀴즈 | `nonlinear_quiz.py` | LM 알고리즘, scipy 활용 퀴즈 |

---

## 📖 핵심 개념

### 1. 비선형 vs 선형

#### 선형 문제 (Week 7)

```
Ax = b

→ 정규방정식 AᵀAx = Aᵀb
→ 한 번에 해를 구함
```

#### 비선형 문제 (이번 주)

```
f(x) = z

f가 비선형 함수일 때:
→ 정규방정식 사용 불가!
→ 반복적 접근 필요
```

**비선형 예시**:
- y = a·e^(bx) (지수 함수)
- p = K·[R|t]·P (카메라 투영)
- R = exp(θ) (로드리게스 회전)

---

### 2. 선형화 (Linearization)

#### 핵심 아이디어

```
비선형 함수를 현재 위치 근처에서 "선형 근사"

f(x + Δx) ≈ f(x) + J·Δx
                ↑
          Jacobian (기울기 행렬)
```

**비유**: 곡선 도로를 작은 직선 조각으로 근사

```
실제 곡선:          선형 근사:
    ╭─╮                ╱
   ╱   ╲           접선╱
  ╱     ╲            ╱←── 현재 위치
                    ╱
                   ╱
→ 작은 범위에서는 근사가 잘 맞음!
```

#### Jacobian 행렬

```
J = ∂f/∂x

    ⎡ ∂f₁/∂x₁  ∂f₁/∂x₂  ...  ∂f₁/∂xₙ ⎤
J = ⎢ ∂f₂/∂x₁  ∂f₂/∂x₂  ...  ∂f₂/∂xₙ ⎥
    ⎣   ...       ...    ...    ...   ⎦

크기: m×n (m개 함수, n개 변수)
```

**예제**: f = [a·e^(bx)] 의 Jacobian

```
f(a, b) = a·e^(bx)

∂f/∂a = e^(bx)
∂f/∂b = a·x·e^(bx)

J = [e^(bx),  a·x·e^(bx)]
```

---

### 3. Gauss-Newton 알고리즘

#### 핵심 아이디어

**비선형 문제를 반복적으로 선형화하여 풀기**

```
┌─────────────────────────────────────────┐
│        Gauss-Newton 한 단계             │
│                                         │
│  1. 현재 x에서 Jacobian J 계산          │
│  2. 잔차 r = z - f(x) 계산              │
│  3. 선형화된 정규방정식:                 │
│     (JᵀJ)·Δx = Jᵀr                      │
│  4. 업데이트: x ← x + Δx                │
│  5. 수렴할 때까지 반복                   │
│                                         │
└─────────────────────────────────────────┘
```

#### 왜 JᵀJ인가?

Week 7에서는 AᵀA였는데:

```
선형:   Ax = b        → AᵀAx = Aᵀb
비선형: f(x) ≈ f(x₀) + J·Δx = z
        → JᵀJ·Δx = Jᵀ(z - f(x₀))
        → JᵀJ·Δx = Jᵀr
```

**J가 A 역할을 함!**

#### 시각화

```
반복 1:        반복 2:        반복 3:
    ╲              ╲              ╲
     ╲              ●              ●
      ●                            │
    시작           근접          수렴!
```

#### 코드

```python
def gauss_newton(f, jacobian, x0, z, max_iter=10):
    x = x0.copy()
    
    for i in range(max_iter):
        J = jacobian(x)          # 1. Jacobian
        r = z - f(x)             # 2. 잔차
        
        # 3. 정규방정식
        dx = np.linalg.solve(J.T @ J, J.T @ r)
        
        x = x + dx               # 4. 업데이트
        
        if np.linalg.norm(dx) < 1e-8:
            break                # 5. 수렴 확인
    
    return x
```

---

### 4. Levenberg-Marquardt (LM)

#### Gauss-Newton의 문제

```
초기값이 멀면 발산!

        최소점 ●
                ╲
                 ╲
          시작점 ●──────▶ 멀리 떠남 💥
```

#### LM의 해결책

**Damping (감쇠) 추가**:

```
(JᵀJ + λI)·Δx = Jᵀr
       ↑
   damping term
```

**λ의 역할**:

```
λ 클 때:                    λ 작을 때:
(JᵀJ + λI) ≈ λI            (JᵀJ + λI) ≈ JᵀJ
→ Δx = (1/λ)·Jᵀr           → Δx = (JᵀJ)⁻¹Jᵀr
→ Gradient Descent!         → Gauss-Newton!
→ 느리지만 안정적            → 빠르지만 불안정 가능
```

#### 적응적 λ 조정

```
비용 감소? → λ ↓ (더 공격적, GN에 가깝게)
비용 증가? → λ ↑ (더 보수적, GD에 가깝게)
```

```
┌────────────────────────────────────────┐
│            LM 알고리즘                  │
├────────────────────────────────────────┤
│                                        │
│    ┌─▶ Δx 계산 (JᵀJ + λI)             │
│    │                                   │
│    │   비용 감소?                       │
│    │      │                            │
│    │   Yes ↓     No                    │
│    │   x 업데이트  └──▶ λ *= 2        │
│    │   λ /= 2          └──┐            │
│    │      │                │           │
│    └──────┴────────────────┘           │
│                                        │
│   수렴할 때까지 반복                    │
│                                        │
└────────────────────────────────────────┘
```

---

### 5. GN vs LM 비교

| 특성 | Gauss-Newton | Levenberg-Marquardt |
|------|-------------|---------------------|
| 수렴 속도 | **빠름** | 중간 |
| 안정성 | 초기값 민감 | **안정적** |
| 파라미터 | 없음 | λ (자동 조정) |
| 사용처 | 좋은 초기값 있을 때 | **일반적 상황** |

**결론**: 실전에서는 대부분 LM 사용

---

### 6. Ceres Solver

#### 소개

Google이 개발한 C++ 최적화 라이브러리
- SLAM 표준 라이브러리
- 자동 미분 지원
- 대규모 희소 문제 처리

#### 기본 구조

```cpp
// 1. 비용 함수 정의
struct MyCost {
    MyCost(double x, double y) : x_(x), y_(y) {}
    
    template <typename T>
    bool operator()(const T* params, T* residual) const {
        // residual = 측정값 - 예측값
        residual[0] = y_ - params[0] * exp(params[1] * x_);
        return true;
    }
    
    double x_, y_;
};

// 2. Problem에 추가
ceres::Problem problem;
for (auto& data : dataset) {
    problem.AddResidualBlock(
        new ceres::AutoDiffCostFunction<MyCost, 1, 2>(
            new MyCost(data.x, data.y)),
        nullptr,   // loss function
        params);   // 최적화할 변수
}

// 3. 최적화 실행
ceres::Solver::Options options;
options.linear_solver_type = ceres::DENSE_QR;
ceres::Solver::Summary summary;
ceres::Solve(options, &problem, &summary);
```

#### Python 대안: scipy

```python
from scipy.optimize import least_squares

def residual(params, x, y):
    a, b = params
    return y - a * np.exp(b * x)

result = least_squares(
    residual, 
    x0=[1, 0.5], 
    args=(x_data, y_data),
    method='lm'  # Levenberg-Marquardt
)
```

---

### 7. Ceres Solver 설치 및 실습

#### 7.1 설치 방법

##### macOS

```bash
# Homebrew 사용
brew install ceres-solver

# 또는 소스에서 빌드
git clone https://ceres-solver.googlesource.com/ceres-solver
cd ceres-solver
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

##### Ubuntu/Debian

```bash
# 의존성 설치
sudo apt-get install cmake libgoogle-glog-dev libgflags-dev
sudo apt-get install libatlas-base-dev libeigen3-dev libsuitesparse-dev

# Ceres Solver 설치
sudo apt-get install libceres-dev

# 또는 소스에서 빌드
git clone https://ceres-solver.googlesource.com/ceres-solver
cd ceres-solver
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

##### Windows

```bash
# vcpkg 사용 (권장)
vcpkg install ceres

# 또는 CMake GUI로 소스 빌드
# https://github.com/ceres-solver/ceres-solver/releases
```

#### 7.2 기본 사용법

##### 단계별 설명

```
┌─────────────────────────────────────────┐
│      Ceres Solver 사용 흐름             │
├─────────────────────────────────────────┤
│                                         │
│  1. 비용 함수 정의 (CostFunction)        │
│     ↓                                   │
│  2. Problem 객체 생성                   │
│     ↓                                   │
│  3. ResidualBlock 추가                  │
│     ↓                                   │
│  4. Solver Options 설정                 │
│     ↓                                   │
│  5. Solve() 실행                        │
│     ↓                                   │
│  6. Summary 확인                        │
│                                         │
└─────────────────────────────────────────┘
```

##### 완전한 곡선 피팅 예제

**문제**: y = a·e^(b·x) 형태의 곡선 피팅

**curve_fitting.cpp**:

```cpp
#include <iostream>
#include <ceres/ceres.h>
#include <vector>
#include <cmath>
#include <random>

// 1️⃣ 비용 함수 정의
struct ExponentialResidual {
    ExponentialResidual(double x, double y)
        : x_(x), y_(y) {}
    
    // Ceres가 호출할 함수
    // params[0] = a, params[1] = b
    template <typename T>
    bool operator()(const T* const params, T* residual) const {
        // residual = 측정값 - 예측값
        residual[0] = T(y_) - params[0] * exp(params[1] * T(x_));
        return true;
    }
    
    // Factory method for AutoDiffCostFunction
    static ceres::CostFunction* Create(double x, double y) {
        return new ceres::AutoDiffCostFunction<ExponentialResidual, 1, 2>(
            new ExponentialResidual(x, y));
    }
    
private:
    const double x_;
    const double y_;
};

int main() {
    // 🎲 데이터 생성 (실제 값: a=2.5, b=0.3)
    std::vector<double> x_data, y_data;
    std::default_random_engine generator;
    std::normal_distribution<double> noise(0.0, 0.1);
    
    const double true_a = 2.5;
    const double true_b = 0.3;
    
    for (int i = 0; i < 50; ++i) {
        double x = i * 0.1;
        double y = true_a * exp(true_b * x) + noise(generator);
        x_data.push_back(x);
        y_data.push_back(y);
    }
    
    // 2️⃣ 초기 추정값 (일부러 틀리게)
    double params[2] = {1.0, 0.1};
    
    std::cout << "초기값: a = " << params[0] 
              << ", b = " << params[1] << std::endl;
    
    // 3️⃣ Problem 생성
    ceres::Problem problem;
    
    // 4️⃣ 각 데이터 포인트에 대해 ResidualBlock 추가
    for (size_t i = 0; i < x_data.size(); ++i) {
        ceres::CostFunction* cost_function = 
            ExponentialResidual::Create(x_data[i], y_data[i]);
        
        problem.AddResidualBlock(
            cost_function,      // 비용 함수
            nullptr,            // loss function (nullptr = squared loss)
            params);            // 최적화할 변수
    }
    
    // 5️⃣ Solver 옵션 설정
    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.minimizer_progress_to_stdout = true;
    options.max_num_iterations = 100;
    
    // 6️⃣ 최적화 실행
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    
    // 7️⃣ 결과 출력
    std::cout << "\n" << summary.BriefReport() << "\n\n";
    std::cout << "최적화 결과:\n";
    std::cout << "  a = " << params[0] << " (실제: " << true_a << ")\n";
    std::cout << "  b = " << params[1] << " (실제: " << true_b << ")\n";
    
    return 0;
}
```

**CMakeLists.txt**:

```cmake
cmake_minimum_required(VERSION 3.10)
project(ceres_example)

set(CMAKE_CXX_STANDARD 14)

# Ceres 찾기
find_package(Ceres REQUIRED)

# 실행 파일
add_executable(curve_fitting curve_fitting.cpp)
target_link_libraries(curve_fitting Ceres::ceres)
```

**빌드 및 실행**:

```bash
mkdir build && cd build
cmake ..
make
./curve_fitting
```

**출력 예시**:

```
초기값: a = 1, b = 0.1

iter      cost      cost_change  |gradient|   |step|    tr_ratio  tr_radius  ls_iter  iter_time  total_time
   0  9.842456e+02    0.00e+00    1.23e+03   0.00e+00   0.00e+00  1.00e+04        0    1.23e-04    2.45e-04
   1  1.234567e+01    9.72e+02    5.67e+01   1.23e+00   9.99e-01  3.00e+04        1    3.45e-04    5.92e-04
   2  2.345678e+00    9.99e+00    1.23e+01   2.34e-01   9.99e-01  9.00e+04        1    2.12e-04    8.15e-04
   ...
  10  1.234567e-02    5.67e-03    1.23e-03   1.23e-04   1.00e+00  2.70e+08        1    1.98e-04    2.34e-03

Ceres Solver Report: Iterations: 11, Initial cost: 9.842e+02, Final cost: 1.235e-02

최적화 결과:
  a = 2.498 (실제: 2.5)
  b = 0.301 (실제: 0.3)
```

#### 7.3 핵심 개념 설명

##### AutoDiffCostFunction

```cpp
// template <비용함수타입, 잔차차원, 파라미터1차원, 파라미터2차원, ...>
ceres::AutoDiffCostFunction<ExponentialResidual, 1, 2>
                                                  ↑  ↑
                                                  │  └─ 2개 파라미터 (a, b)
                                                  └─── 1차원 잔차 (scalar)
```

**자동 미분의 장점**:
- 수동으로 Jacobian 계산 불필요
- 실수 방지, 유지보수 쉬움
- Jet 타입으로 forward automatic differentiation

##### Loss Function

```cpp
// Squared Loss (기본)
nullptr

// Huber Loss (outlier 강건)
new ceres::HuberLoss(1.0)

// Cauchy Loss (더 강건)
new ceres::CauchyLoss(1.0)
```

**언제 쓰나?**
- `nullptr`: 데이터가 깨끗할 때
- `HuberLoss`: SLAM에서 일반적 (outlier 있을 때)
- `CauchyLoss`: outlier가 매우 많을 때

##### Solver Options 주요 설정

```cpp
ceres::Solver::Options options;

// 선형 솔버 선택
options.linear_solver_type = ceres::DENSE_QR;        // 작은 문제
options.linear_solver_type = ceres::SPARSE_NORMAL_CHOLESKY;  // 큰 문제
options.linear_solver_type = ceres::DENSE_SCHUR;     // Bundle Adjustment

// Trust region 전략
options.trust_region_strategy_type = ceres::LEVENBERG_MARQUARDT;  // 기본
options.trust_region_strategy_type = ceres::DOGLEG;

// 최대 반복 횟수
options.max_num_iterations = 100;

// 수렴 기준
options.function_tolerance = 1e-6;
options.gradient_tolerance = 1e-10;
options.parameter_tolerance = 1e-8;

// 로그 출력
options.minimizer_progress_to_stdout = true;
```

#### 7.4 실전 팁

##### ✅ Best Practices

```cpp
// 1. 좋은 초기값 제공
double params[2] = {1.0, 0.1};  // ❌ 너무 멀 수 있음
double params[2] = {2.0, 0.2};  // ✅ 대략적 사전 지식 활용

// 2. 파라미터 범위 제한
problem.SetParameterLowerBound(params, 0, 0.0);   // a > 0
problem.SetParameterUpperBound(params, 0, 10.0);  // a < 10

// 3. 특정 파라미터 고정
problem.SetParameterBlockConstant(params);

// 4. Loss function 사용 (outlier 대응)
problem.AddResidualBlock(
    cost_function,
    new ceres::HuberLoss(1.0),  // ✅
    params);
```

##### ⚠️ 주의사항

```cpp
// ❌ 잘못된 예: 메모리 관리
ceres::CostFunction* cost = ExponentialResidual::Create(x, y);
delete cost;  // ❌ Ceres가 자동으로 관리함!

// ✅ 올바른 예
problem.AddResidualBlock(cost, nullptr, params);
// Ceres가 소멸자에서 자동 해제

// ❌ 잘못된 예: 파라미터 범위
double params[2];
// ... solve ...
// params가 스택 변수라면 함수 끝나면 소멸! 

// ✅ 올바른 예: 충분한 생명주기 보장
std::vector<double> params(2);
// 또는 heap 할당
```

---

### 8. SLAM에서의 활용

#### Bundle Adjustment

```
           카메라 1        카메라 2        카메라 3
              ●──────────────●──────────────●
             ╱│╲            ╱│╲            ╱│╲
            ╱ │ ╲          ╱ │ ╲          ╱ │ ╲
           ╱  │  ╲        ╱  │  ╲        ╱  │  ╲
          ★   ★  ★     ★   ★   ★     ★   ★  ★
          
       동시에 최적화:
       - 모든 카메라 포즈 (R, t)
       - 모든 3D 점 위치
       - 재투영 오차 최소화
```

#### 실전 로봇/Perception 시스템에서 (Ceres 예시)

```cpp
// vins_estimator/src/factor/projection_factor.h

// 재투영 오차 비용 함수
class ProjectionFactor : public ceres::SizedCostFunction<2, 7, 7, 7, 1> {
    // 잔차: 관측된 2D점 - 투영된 2D점
    // Jacobian: 자동 미분 또는 분석적 계산
};

// optimization.cpp
ceres::Problem problem;

// 카메라 포즈 파라미터
for (auto& frame : frames) {
    problem.AddParameterBlock(frame.pose, 7);
}

// 재투영 오차 추가
for (auto& obs : observations) {
    problem.AddResidualBlock(
        new ProjectionFactor(...),
        loss_function,
        pose_i, pose_j, inv_depth);
}

ceres::Solve(options, &problem, &summary);
```

---

## 💻 실습 파일

| 파일 | 내용 | 난이도 |
|------|------|--------|
| `nonlinear_basics.py` | Gauss-Newton, 곡선 피팅 | ⭐⭐⭐ |
| `nonlinear_quiz.py` | LM, scipy 활용 | ⭐⭐⭐ |

---

## 📊 핵심 정리

### 알고리즘 요약

| 알고리즘 | 정규방정식 | 특징 |
|---------|-----------|------|
| 선형 LS | AᵀAx = Aᵀb | 한 번에 해결 |
| Gauss-Newton | JᵀJ·Δx = Jᵀr | 반복, 빠름 |
| LM | (JᵀJ + λI)Δx = Jᵀr | 반복, 안정적 |

### 핵심 포인트

```
1. 비선형 → 선형화 (Jacobian)
2. 반복적 접근 (수렴할 때까지)
3. GN: 빠르지만 초기값 민감
4. LM: λ로 안정성-속도 균형
5. Ceres: SLAM 실전 라이브러리
```

---

## ✅ 학습 완료 체크리스트

### 기초 이해 (필수)
- [ ] 선형화와 Jacobian 관계 설명 가능
- [ ] Gauss-Newton 한 스텝 설명 가능
- [ ] LM이 GN보다 안정적인 이유 설명 가능

### 실용 활용 (권장)
- [ ] Jacobian 계산 가능
- [ ] scipy.optimize.least_squares 사용 가능
- [ ] 수렴 여부 판단 가능

### 심화 (선택)
- [ ] Ceres CostFunction 구조 이해
- [ ] Hessian ≈ JᵀJ 근사의 의미 이해
- [ ] VINS 최적화 코드 흐름 분석

---

## 🔗 Phase 1 완료! 🎉

8주간의 수학 핵심 학습을 완료했습니다:

| 주차 | 주제 | 핵심 |
|------|------|------|
| 1-2 | 선형대수 | 벡터, 행렬, SVD |
| 3 | SVD 활용 | Null space, rank |
| 4-5 | 확률/베이즈 | 가우시안, 추정 |
| 6 | 3D 변환 | SO(3), SE(3), Lie |
| 7 | 최소자승 | AᵀAx = Aᵀb |
| **8** | **비선형 최적화** | **GN, LM, Ceres 설치/실습** |

**다음: Phase 2 - 컴퓨터 비전 기초! 🚀**

---

## 📚 참고 자료

- Numerical Optimization (Nocedal & Wright)
- Ceres Solver Documentation
- OpenCV calib3d 모듈 (PnP, BA 실전 활용)

---

## ❓ FAQ

**Q1: Jacobian은 어떻게 구하나요?**
A: 분석적 미분(손계산) 또는 자동 미분(Ceres AutoDiff).

**Q2: GN이 발산하면 어떻게 알 수 있나요?**
A: 비용 함수가 증가하거나, 파라미터가 발산.

**Q3: λ 초기값은 어떻게 정하나요?**
A: 보통 0.01~1. 자동 조정되므로 크게 중요하지 않음.

**Q4: SLAM에서 GN vs LM?**
A: 대부분 LM. 하지만 BA에서는 GN + trust region 조합도 많이 사용.

---

**🎯 Week 8 핵심 메시지:**

> 비선형 문제 = 반복적 선형화
>
> **Gauss-Newton**: JᵀJ·Δx = Jᵀr
> **LM**: (JᵀJ + λI)·Δx = Jᵀr
>
> SLAM 최적화의 엔진!
