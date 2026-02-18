# Week 6: Lie 군/대수 기초

## 📌 개요

> ⚠️ **참고**: 이번 주는 가볍게 훑고, VINS-Fusion 코드 (Phase 5)에서 필요할 때 돌아와서 심화 학습

**Lie 군/대수**는 회전과 변환을 **최적화 가능한 형태**로 표현하는 수학적 프레임워크입니다. 

### 🤔 왜 이게 필요할까요?

일상적인 비유로 시작해봅시다:

**문제 상황**: 자동차(로봇)의 위치를 지속적으로 업데이트한다고 상상해보세요.
- 위치(x, y, z): 간단하게 더하기/빼기로 업데이트 가능 → `x_new = x_old + Δx` ✅
- 회전(방향): 단순히 더하면 회전이 아닌 이상한 값이 됨 → `R_new = R_old + ΔR` ❌

회전 행렬은 9개 숫자로 이루어져 있지만:
- **실제 자유도는 3개** (roll, pitch, yaw)
- 나머지 6개는 "회전 행렬이라는 특별한 조건"을 만족하기 위한 제약

쿼터니언도 4개 숫자지만:
- **실제 자유도는 3개**
- 1개는 "길이가 1이어야 한다"는 제약

**핵심 질문**: 최적화(조금씩 값을 조정해서 더 나은 값 찾기)할 때 이런 **제약 조건**을 어떻게 유지할까요?

**Lie 군/대수의 답**: 회전을 3개의 자유로운 숫자로 표현해서 업데이트하고, 다시 회전으로 변환! 이렇게 하면 제약 조건 걱정 없이 최적화 가능합니다.

## 🎯 학습 목표

1. 왜 회전/변환의 "over-parameterized" 문제가 있는지 이해
2. SO(3), SE(3)의 기본 개념 이해
3. Lie 대수(접선 공간)의 직관적 이해
4. exp/log 매핑의 의미 파악
5. Sophus 라이브러리 기본 사용법

## 📚 사전 지식

- Week 4: 회전 표현
- Week 5: SE(3) 변환

## ⏱️ 예상 학습 시간

| 항목 | 시간 |
|------|------|
| 이론 (가볍게) | 2시간 |
| 실습 예제 | 1-2시간 |
| **총 소요시간** | **3-4시간** |

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | Python 기초 실습 | `lie_basics.py` | Lie 군/대수 기본 연산 (skew, exp, log 구현) |
| 3 | Python 심화 퀴즈 | `lie_quiz.py` | 개념 확인 퀴즈 및 ICP 구현 실습 |

---

## 📖 핵심 개념

### 1. 왜 Lie 군/대수가 필요한가?

#### 📐 Over-parameterized 문제란?

**Over-parameterized**는 "필요 이상으로 많은 변수를 사용한다"는 뜻입니다.

##### 비유: 시계 바늘

시계 바늘의 위치를 나타내는 방법:
1. **각도 하나**: 12시 방향에서 몇 도 회전했는지 (예: 90°) → 자유도 1개
2. **바늘 끝 좌표**: (x, y) 좌표로 표현 → 2개 변수 사용

그런데 바늘은 **원 위를 움직여야** 합니다:
- `x² + y² = 1` (단위 원 위)라는 제약 조건 필요
- 실제 자유도는 1개인데, 2개 변수를 쓰면서 1개 제약을 관리해야 함

이게 바로 **over-parameterized**!

##### 비교표

**케이스 1: 회전만 다룰 때 (SO(3))**

| 표현 | 파라미터 수 | 실제 자유도 | 제약 조건 |
|------|------------|------------|---------|
| 회전 행렬 R | 9개 (3×3) | 3개 | RᵀR=I, det(R)=1 → **6개 제약** |
| 쿼터니언 q | 4개 | 3개 | ‖q‖=1 → **1개 제약** |
| **Lie 대수 ω (so(3))** | **3개** | **3개** | **제약 없음!** ✅ |

**케이스 2: 회전 + 이동을 함께 다룰 때 (SE(3))**

| 표현 | 파라미터 수 | 실제 자유도 | 제약 조건 |
|------|------------|------------|---------|
| 변환 행렬 T | 16개 (4×4) | 6개 | 회전 부분 제약 → **10개 제약** |
| **Lie 대수 ξ (se(3))** | **6개** | **6개** | **제약 없음!** ✅ |

> 💡 **왜 Lie 대수는 "3개 또는 6개"인가?**
> 
> Lie 대수의 차원은 **어떤 Lie 군을 표현하느냐**에 따라 달라집니다:
> 
> - **so(3)**: SO(3)의 Lie 대수 → 회전만 표현 → **3차원 벡터** `ω = [ω₁, ω₂, ω₃]ᵀ`
> - **se(3)**: SE(3)의 Lie 대수 → 회전+이동 표현 → **6차원 벡터** `ξ = [ρ₁, ρ₂, ρ₃, φ₁, φ₂, φ₃]ᵀ`
> 
> 즉, 목적에 따라 적절한 차원의 Lie 대수를 선택합니다!

**왜 자유도가 3개일까?**
- 3D 공간에서 회전 = 어떤 축을 중심으로 얼마나 돌릴까?
  - 축의 방향: 2개 자유도 (구면 위의 점)
  - 회전 각도: 1개 자유도
  - 합계: 3개 자유도

또는 더 직관적으로:
- Roll (x축 회전): 1개
- Pitch (y축 회전): 1개  
- Yaw (z축 회전): 1개
- 합계: 3개 자유도

#### ⚠️ 최적화에서의 문제

최적화는 보통 이렇게 동작합니다:

```python
# 일반적인 최적화: "gradient descent"
# 현재 값 ← 현재 값 + (조금씩 조정값)

x = x + learning_rate * gradient  # ✅ 일반 변수는 OK
```

하지만 회전에 이걸 그대로 적용하면:

```python
# ❌ 문제 1: 회전 행렬에 단순 덧셈
R = [[1, 0, 0],      # 단위 행렬 (회전 없음)
     [0, 1, 0],
     [0, 0, 1]]

ΔR = [[0.1, 0, 0],   # 작은 변화
      [0, 0.1, 0],
      [0, 0, 0.1]]

R_new = R + ΔR
# = [[1.1, 0, 0],    # ❌ 이건 회전 행렬이 아닙니다!
#    [0, 1.1, 0],    #    RᵀR ≠ I
#    [0, 0, 1.1]]
```

**왜 문제일까요?**
- 회전 행렬의 조건: `RᵀR = I` (열벡터들이 서로 수직이고 길이가 1)
- 위 결과는 이 조건을 만족하지 않음
- 포인트를 변환하면 **크기가 변하거나 찌그러짐**

```python
# ❌ 문제 2: 쿼터니언에 단순 덧셈
q = [1, 0, 0, 0]     # w=1, x=y=z=0 (회전 없음)
Δq = [0.1, 0, 0, 0]

q_new = q + Δq
# = [1.1, 0, 0, 0]   # ❌ 길이가 1.1 (단위 쿼터니언이 아님)

# 강제로 정규화?
q_new = q_new / norm(q_new)  # 가능하지만 비효율적
```

#### ✅ 해결책: Lie 대수 방식

Lie 대수는 **제약이 없는 공간**에서 업데이트하고, **자동으로 유효한 회전**으로 변환합니다:

```python
# Lie 대수 방식 (단계별 설명)

# 1단계: Lie 대수에서 업데이트 계산 (3개 파라미터만!)
Δξ = [0.1, 0.05, 0.0]  # [Δroll, Δpitch, Δyaw] 같은 개념
                        # 이건 그냥 일반 벡터 - 제약 없음!

# 2단계: exp 매핑 - Lie 대수 → Lie 군 (회전 행렬)
ΔR = exp(Δξ)           # "마법의 변환" - 항상 유효한 회전 행렬 생성!

# 3단계: 회전 행렬 업데이트 (곱셈 사용)
R_new = ΔR @ R         # ✅ 항상 유효한 회전 행렬!
                       # RᵀR = I 자동으로 만족
```

**비유**:
- Lie 대수 = 평평한 땅 (평면 좌표계)
- Lie 군 = 지구 표면 (구면)
- exp 매핑 = 평평한 지도를 구 위에 투영하는 것

평평한 땅에서는 자유롭게 움직이고(제약 없음), 지구 표면으로 변환하면 항상 구 위의 점이 됩니다!

---

### 2. 군(Group)의 기본 개념

#### 🎓 군이란 무엇인가?

**군(Group)** 은 수학의 추상적 개념이지만, 직관적으로 이해할 수 있습니다.

**일상 비유**: 시계 숫자(1~12)
- 9시 + 5시간 = 2시 (12를 넘어가면 다시 1부터)
- 역방향도 가능: 2시 - 5시간 = 9시
- 0시간 더하기 = 그대로
- "시간 더하기" 연산이 항상 1~12 안에서 유효함

이게 바로 **군의 성질**입니다!

#### 📋 군의 4가지 성질

모든 군은 다음 4가지 조건을 만족해야 합니다:

| 성질 | 수학적 표현 | 직관적 의미 | 회전 예시 |
|------|-----------|------------|----------|
| **1. 닫힘** | a·b ∈ G | 두 원소를 연산하면<br>결과도 같은 집합 | 두 회전을 합성하면<br>여전히 회전 |
| **2. 결합법칙** | (a·b)·c = a·(b·c) | 계산 순서 바꿔도 OK<br>(단, a,b,c 순서는 유지) | (R₁R₂)R₃ = R₁(R₂R₃) |
| **3. 항등원** | ∃e: e·a = a·e = a | 연산해도 안 바뀌는 원소 | 단위 행렬 I<br>(회전 없음) |
| **4. 역원** | ∀a, ∃a⁻¹: a·a⁻¹ = e | 원래로 돌리는 원소 | 전치 행렬 Rᵀ<br>(반대 회전) |

**예제**: 정수의 덧셈 (Z, +)
- 닫힘: 3 + 5 = 8 (정수)
- 결합: (1+2)+3 = 1+(2+3) = 6
- 항등원: 0 (아무 수 + 0 = 그대로)
- 역원: 3의 역원은 -3 (3 + (-3) = 0)

---


#### 🔄 SO(3): Special Orthogonal Group

```
SO(3) = {R ∈ ℝ³ˣ³ | RᵀR = I, det(R) = 1}
```

**용어 풀이**:
- **Special**: det(R) = 1 (반사 없이 순수한 회전만)
- **Orthogonal**: 열벡터들이 서로 수직 (RᵀR = I)
- **Group**: 위에서 본 4가지 군 성질 만족

**직관적 이해**:
- SO(3) = **모든 가능한 3D 회전의 집합**
- 각 회전 행렬 R = 집합의 한 원소
- 예: Z축 45° 회전, X축 30° 회전, 임의 축 회전 등

**구체적 예시**:

```python
import numpy as np

# 예시 1: Z축 90도 회전
R_z90 = np.array([
    [0, -1, 0],   # x축이 -y축으로
    [1,  0, 0],   # y축이 x축으로  
    [0,  0, 1]    # z축은 그대로
])

# 검증: SO(3) 조건 확인
print("RᵀR =")
print(R_z90.T @ R_z90)  # 단위 행렬 I가 나와야 함
# [[1, 0, 0],
#  [0, 1, 0],
#  [0, 0, 1]]

print("det(R) =", np.linalg.det(R_z90))  # 1.0이 나와야 함
```

**군의 성질 확인**:

1. **닫힘**: 두 회전의 합성

```python
# Z축 90도 + Z축 90도 = Z축 180도
R_z180 = R_z90 @ R_z90
# [[-1, 0, 0],
#  [ 0,-1, 0],
#  [ 0, 0, 1]]
# 이것도 SO(3)의 원소 (180도 회전)
```

2. **항등원**: 단위 행렬 I

```python
I = np.eye(3)  # 회전 없음 (0도 회전)
R_z90 @ I == R_z90  # True
```

3. **역원**: 전치 행렬

```python
# Z축 90도의 역원 = Z축 -90도 (또는 270도)
R_inv = R_z90.T
R_z90 @ R_inv == I  # 원래대로 돌아옴
```

**연산**: 행렬 곱셈
- `R₁ @ R₂` = "먼저 R₂ 회전, 그 다음 R₁ 회전"
- **주의**: 곱셈 순서 중요! `R₁R₂ ≠ R₂R₁` (비가환)

---

#### 🚀 SE(3): Special Euclidean Group

```
SE(3) = {T ∈ ℝ⁴ˣ⁴ | T = [R|t], R ∈ SO(3), t ∈ ℝ³}
                        [0|1]
```

**의미**: **모든 강체 변환의 집합** (회전 + 이동)

**구조**:

```
T = [R  t]   R: 3×3 회전 행렬
    [0  1]   t: 3×1 이동 벡터
             0: [0 0 0] (1×3)
             1: 스칼라
```

**자유도**: 6개
- 회전 (roll, pitch, yaw): 3개
- 이동 (x, y, z): 3개

**구체적 예시**:

```python
# 예: Z축 90도 회전 + [1, 2, 3] 이동
R = np.array([
    [0, -1, 0],
    [1,  0, 0],
    [0,  0, 1]
])
t = np.array([[1], [2], [3]])

T = np.block([
    [R, t],
    [np.zeros((1, 3)), 1]
])
# [[0, -1, 0, 1],
#  [1,  0, 0, 2],
#  [0,  0, 1, 3],
#  [0,  0, 0, 1]]
```

**사용 예시**: 포인트 변환

```python
p = np.array([[1], [0], [0], [1]])  # 동차좌표 (x=1, y=0, z=0)

p_new = T @ p
# [[0],   # x' = 0*1 + (-1)*0 + 0*0 + 1 = 1
#  [1],   # y' = 1*1 + 0*0 + 0*0 + 2 = 3
#  [3],   # z' = 0*1 + 0*0 + 1*0 + 3 = 3
#  [1]]

# 해석: (1,0,0) → 회전 → (0,1,0) → 이동 → (1,3,3)
```

**군의 성질**:

```python
# 두 변환의 합성
T1 @ T2  # 먼저 T2, 그 다음 T1

# 항등원
I = np.eye(4)  # 회전도 이동도 없음

# 역원: 원래 위치로
T_inv = np.block([
    [R.T, -R.T @ t],
    [np.zeros((1, 3)), 1]
])
```

**SLAM에서의 의미**:
- `T_world_camera`: 월드 좌표계에서 본 카메라의 위치/방향
- `T_camera_world`: 카메라 좌표계에서 본 월드 (역변환)

---

### 3. Lie 대수: 접선 공간

#### 🌐 Lie 대수란?

**핵심 아이디어**: Lie 군(곡면)을 **접선 공간(평면)** 에서 표현

**비유**:
```
지구 표면 (곡면, 구)  ←→  SO(3) / SE(3)  (Lie 군)
    ↕                          ↕
평평한 지도 (평면)    ←→  so(3) / se(3)  (Lie 대수)
```

**왜 접선 공간일까?**

1. 곡면 위의 점 (예: 서울)
2. 그 점에서의 접선 평면 = 작은 범위에서는 평평함
3. 접선 평면에서는 일반적인 벡터 연산 가능 (덧셈, 스칼라 곱)
4. 작은 벡터(이동) → exp 매핑 → 곡면 위의 새로운 점

**수학적 정의**:

```
Lie 대수 = Lie 군의 항등원(I)에서의 접선 공간
```

**시각화**:

```
        SO(3) (회전 manifold - 곡면)
           ↗
      exp(ω)  ← ω ∈ so(3) (접선 벡터)
     /
I (항등원) ─────→ so(3) (접선 공간 - 평면)
```

#### 📐 so(3): SO(3)의 Lie 대수

**표현 방법**:

1. **3D 벡터 형태**: `ω = [ω₁, ω₂, ω₃]ᵀ`
   - 물리적 의미: 회전축 방향 × 회전 각도
   - 예: `ω = [0, 0, π/2]` = Z축 주위로 π/2 회전

2. **반대칭 행렬 형태**: `ω^` (hat 연산자)

```python
def skew(w):
    """벡터 → 반대칭 행렬 (hat 연산자 ^)"""
    return np.array([
        [0,     -w[2],  w[1]],
        [w[2],   0,    -w[0]],
        [-w[1],  w[0],  0   ]
    ])

# 예시
w = np.array([1, 2, 3])
w_skew = skew(w)
# [[ 0, -3,  2],
#  [ 3,  0, -1],
#  [-2,  1,  0]]
```

**반대칭 행렬의 성질**:
- `ω^ᵀ = -ω^` (전치하면 부호만 바뀜)
- `ω^ @ p = ω × p` (외적과 동등!)

**벡터 ↔ 행렬 변환**:

```python
def vee(w_skew):
    """반대칭 행렬 → 벡터 (vee 연산자 ∨)"""
    return np.array([
        w_skew[2, 1],  # -w[3]의 반대편
        w_skew[0, 2],  # -w[2]의 반대편
        w_skew[1, 0]   # -w[1]의 반대편
    ])

# 확인
w_original = vee(w_skew)  # [1, 2, 3]
```

**so(3)의 정의**:

```
so(3) = {ω^ ∈ ℝ³ˣ³ | ω^ᵀ = -ω^}
      = 모든 3×3 반대칭 행렬의 집합
```

- **벡터 공간**: 3차원 (3개 자유 파라미터)
- **연산**: 행렬 덧셈, 스칼라 곱 (제약 없음!)

---

#### 🎯 se(3): SE(3)의 Lie 대수

**벡터 표현**: 6차원 벡터

```
ξ = [ρ]  ∈ ℝ⁶
    [φ]

ρ: 평행이동 관련 (3차원)
φ: 회전 관련 (3차원, so(3)와 같음)
```

**주의**: ρ는 정확히 평행이동 벡터 t가 **아닙니다**!
- ρ와 t의 관계는 복잡 (Jacobian 행렬 필요)
- But, 작은 변화에서는 거의 비슷

**행렬 표현**: 4×4 행렬

```python
def se3_hat(xi):
    """6D 벡터 → 4×4 행렬"""
    rho = xi[0:3]  # 평행이동 부분
    phi = xi[3:6]  # 회전 부분
    
    return np.block([
        [skew(phi), rho.reshape(3, 1)],
        [np.zeros((1, 4))]
    ])

# 예시
xi = np.array([1, 2, 3, 0.1, 0.2, 0.3])
xi_mat = se3_hat(xi)
# [[ 0.0 , -0.3,  0.2, 1],
#  [ 0.3,  0.0, -0.1, 2],
#  [-0.2,  0.1,  0.0, 3],
#  [ 0.0,  0.0,  0.0, 0]]
```

**se(3)의 정의**:

```
se(3) = {ξ^ ∈ ℝ⁴ˣ⁴ | ξ^ = [ω^ ρ], ω^ ∈ so(3), ρ ∈ ℝ³}
                              [0  0]
```

---

### 4. Exponential / Log 매핑

#### 🔄 exp와 log: 두 세계를 연결하는 다리

**핵심 관계**:

```
Lie 군 (곡면)  ←──exp──  Lie 대수 (평면)
              ──log──→
```

- **exp**: Lie 대수 → Lie 군 (평면 → 곡면)
- **log**: Lie 군 → Lie 대수 (곡면 → 평면)

**직관적 이해**:

```
exp: "작은 벡터를 회전으로 바꾸기"
     ω = [0, 0, 0.5] → R (Z축 0.5 radian 회전)

log: "회전을 작은 벡터로 표현하기"
     R (Z축 0.5 radian 회전) → ω = [0, 0, 0.5]
```

---

#### 🔬 exp: Lie 대수 → Lie 군 (Rodrigues 공식)

**입력**: `ω ∈ ℝ³` (또는 `ω^ ∈ so(3)`)
**출력**: `R ∈ SO(3)` (회전 행렬)

**Rodrigues 공식**:

```
R = exp(ω^) = I + sin(θ)K + (1 - cos(θ))K²

여기서:
- θ = ‖ω‖ (회전 각도)
- k = ω/θ (단위 회전축)
- K = skew(k) (단위 축의 반대칭 행렬)
```

**단계별 유도**:

1. **회전 벡터** `ω = θk`
   - 방향(k): 회전축
   - 크기(θ): 회전 각도

2. **Taylor 급수 전개**
   ```
   exp(ω^) = I + ω^ + (ω^)²/2! + (ω^)³/3! + ...
   ```

3. **반대칭 행렬의 성질 이용**
   - `K² = kk^T - I` (중요한 성질!)
   - `K³ = -K`, `K⁴ = -K²`, ... (주기성)

4. **sin/cos 급수로 정리**
   ```
   exp(θK) = I + (θ - θ³/3! + θ⁵/5! - ...)K 
              + (θ²/2! - θ⁴/4! + ...)K²
           = I + sin(θ)K + (1 - cos(θ))K²
   ```

**Python 구현**:

```python
def exp_so3(omega):
    """
    SO(3)의 exp 매핑: so(3) → SO(3)
    
    Args:
        omega: 3D 벡터 (회전축 방향 × 각도)
    
    Returns:
        R: 3×3 회전 행렬
    """
    theta = np.linalg.norm(omega)  # 회전 각도
    
    # 특수 경우: 회전 없음
    if theta < 1e-10:
        return np.eye(3)
    
    # 단위 회전축
    axis = omega / theta
    K = skew(axis)
    
    # Rodrigues 공식
    R = np.eye(3) + np.sin(theta) * K + (1 - np.cos(theta)) * (K @ K)
    return R

# 예시 1: Z축 90도 회전
omega = np.array([0, 0, np.pi/2])
R = exp_so3(omega)
print(R)
# [[ 0, -1, 0],
#  [ 1,  0, 0],
#  [ 0,  0, 1]]

# 예시 2: 임의 축 회전
axis = np.array([1, 1, 1]) / np.sqrt(3)  # (1,1,1) 방향 단위벡터
angle = np.pi / 3
omega = axis * angle
R = exp_so3(omega)
```

**물리적 의미**:
- **ω의 방향**: 회전축 (오른손 법칙)
- **ω의 크기**: 회전 각도 (radian)

**시각화**:

```
ω = [1, 0, 0]  →  X축 주위로 1 radian 회전
ω = [0, 2, 0]  →  Y축 주위로 2 radian 회전
ω = [√2, √2, 0] → XY 평면 대각선 주위로 2 radian 회전
```

---

#### 🔍 log: Lie 군 → Lie 대수

**입력**: `R ∈ SO(3)` (회전 행렬)
**출력**: `ω ∈ ℝ³` (회전 벡터)

**공식**:

```
θ = arccos((tr(R) - 1) / 2)
ω^ = θ/(2sin(θ)) × (R - R^T)

여기서:
- tr(R) = R₁₁ + R₂₂ + R₃₃ (대각합)
```

**유도**:

1. **회전 각도 구하기**
   - Rodrigues 공식에서 `tr(R) = 1 + 2cos(θ)`
   - 따라서 `θ = arccos((tr(R) - 1) / 2)`

2. **회전축 구하기**
   - `R - R^T = 2sin(θ)ω^` (반대칭 부분만 남음)
   - 따라서 `ω^ = (R - R^T) / (2sin(θ))`

**Python 구현**:

```python
def log_so3(R):
    """
    SO(3)의 log 매핑: SO(3) → so(3)
    
    Args:
        R: 3×3 회전 행렬
    
    Returns:
        omega: 3D 벡터 (회전축 × 각도)
    """
    # 회전 각도 계산
    trace = np.trace(R)
    theta = np.arccos(np.clip((trace - 1) / 2, -1, 1))
    
    # 특수 경우: 회전 없음
    if theta < 1e-10:
        return np.zeros(3)
    
    # 회전축 계산
    omega_hat = (R - R.T) * theta / (2 * np.sin(theta))
    
    # 반대칭 행렬 → 벡터
    omega = vee(omega_hat)
    return omega

# 예시: Z축 90도 회전 행렬
R = np.array([
    [0, -1, 0],
    [1,  0, 0],
    [0,  0, 1]
])

omega = log_so3(R)
print(omega)  # [0, 0, 1.5708] ≈ [0, 0, π/2]
```

**특수 경우 처리**:

1. **θ = 0** (회전 없음)
   ```python
   if theta < epsilon:
       return np.zeros(3)
   ```

2. **θ = π** (180도 회전)
   - `sin(θ) = 0`이므로 위 공식 사용 불가
   - 대각 원소에서 회전축 추출
   ```python
   if abs(theta - np.pi) < epsilon:
       # R의 가장 큰 대각 원소 찾기
       ...
   ```

---

#### 📊 exp/log 관계의 핵심 성질

**1. 역함수 관계**:

```python
omega = np.array([0.1, 0.2, 0.3])
R = exp_so3(omega)
omega_recovered = log_so3(R)

print(np.allclose(omega, omega_recovered))  # True
```

**2. 단위원**:

```
exp: 평면의 원점 주변 → 구면의 항등원 주변
log: 구면의 항등원 주변 → 평면의 원점 주변
```

**3. 작은 각도 근사**:

```python
# θ ≈ 0일 때
exp(ω^) ≈ I + ω^
log(R) ≈ (R - R^T) / 2
```

**시각적 비유**:

```
지도 (평면)                    지구 (구)
    ↓ exp (투영)                  ↑
서울에서 100km 북쪽   →   실제 지구상 위치
    ↑ log (역투영)                ↓
```

---

#### 🌟 SE(3)의 exp/log

SE(3)도 비슷하지만 더 복잡합니다.

**exp 매핑**: `ξ = [ρ, φ]^T → T`

```python
def exp_se3(xi):
    """
    SE(3)의 exp 매핑
    
    Args:
        xi: 6D 벡터 [ρ(3), φ(3)]
    
    Returns:
        T: 4×4 변환 행렬
    """
    rho = xi[0:3]  # 이동 관련
    phi = xi[3:6]  # 회전
    
    R = exp_so3(phi)  # 회전 부분
    
    # Jacobian 계산 (복잡!)
    theta = np.linalg.norm(phi)
    if theta < 1e-10:
        J = np.eye(3)
    else:
        axis = phi / theta
        K = skew(axis)
        J = np.eye(3) + ((1 - np.cos(theta)) / theta) * K \
            + ((theta - np.sin(theta)) / theta) * (K @ K)
    
    t = J @ rho  # 실제 이동 벡터
    
    T = np.block([
        [R, t.reshape(3, 1)],
        [np.zeros((1, 3)), 1]
    ])
    return T
```

**주요 차이점**:
- `ρ ≠ t`: Jacobian 행렬 J를 통해 변환
- J는 회전 각도 θ에 의존
- 작은 각도에서는 `J ≈ I`, 즉 `ρ ≈ t`

**log 매핑**: 역과정 (생략)

---

### 5. 최적화에서의 활용

#### 🎯 최적화 문제란?

**목표**: 어떤 목적 함수(cost function)를 최소화하는 파라미터 찾기

**예시**: SLAM에서 카메라 포즈 찾기
- 관측: 카메라에서 본 3D 포인트들의 2D 위치
- 목표: 관측과 예측이 가장 잘 맞는 카메라 포즈 T 찾기

**일반적 최적화 방법**: Gradient Descent

```python
# 반복:
x = x - learning_rate * gradient
```

하지만 회전에는 이게 안 통합니다!

---

#### ❌ 기존 방식의 문제점

##### 방법 1: 회전 행렬 직접 최적화

```python
# ❌ 문제: 제약 조건 위반
R = R - learning_rate * gradient_R

# 결과가 회전 행렬이 아닐 수 있음!
# RᵀR ≠ I, det(R) ≠ 1
```

**해결 시도**: 매 스텝마다 투영 (projection)

```python
R = R - learning_rate * gradient_R
# 가장 가까운 회전 행렬로 투영 (SVD 사용)
U, S, Vt = np.linalg.svd(R)
R = U @ Vt  # 가까운 회전 행렬

# 문제:
#  - 비용이 큼 (SVD는 O(n³))
#  - gradient의 의미가 왜곡됨
#  - 수렴이 느리거나 불안정
```

##### 방법 2: 쿼터니언 최적화

```python
# 쿼터니언 q = [w, x, y, z]
q = q - learning_rate * gradient_q

# 정규화 필요
q = q / np.linalg.norm(q)

# 문제:
#  - gradient 계산이 복잡 (제약 조건 고려)
#  - 정규화로 인한 미분 연속성 문제
#  - 4개 파라미터 vs 3 자유도 (여전히 over-parameterized)
```

##### 방법 3: Euler 각 최적화

```python
# [roll, pitch, yaw] 직접 최적화
euler = euler - learning_rate * gradient_euler

# 문제:
#  - Gimbal lock (특정 각도에서 자유도 손실)
#  - 각도 합성이 비선형적
#  - Jacobian 계산이 복잡
```

---

#### ✅ Lie 대수 방식: 제약 없는 최적화

**핵심 아이디어**:
1. Lie 대수에서 업데이트 계산 (3개 자유 파라미터)
2. exp 매핑으로 Lie 군으로 변환
3. 항상 유효한 회전/변환!

##### 단계별 과정

```python
# 현재 상태
R = current_rotation  # SO(3)

# Step 1: 현재 상태에서 log 매핑 (필요시)
#         주로 항등원에서 시작하므로 생략 가능

# Step 2: Lie 대수에서 업데이트 계산
#         (3개 파라미터만 최적화!)
Δω = -learning_rate * gradient_lie  # ∈ ℝ³, 제약 없음!

# Step 3: exp 매핑으로 증분 회전 생성
ΔR = exp_so3(Δω)  # SO(3)의 원소, 항상 유효한 회전

# Step 4: 회전 업데이트 (왼쪽 곱)
R_new = ΔR @ R  # ✅ R_new도 항상 유효한 회전 행렬!
```

**장점 요약**:

| 항목 | 기존 방식 | Lie 대수 방식 |
|------|----------|-------------|
| 파라미터 수 | 9 (R) or 4 (q) | 3 (ω) |
| 제약 조건 | 있음 (복잡) | 없음 ✅ |
| 투영 필요 | 매 스텝 | 불필요 ✅ |
| gradient 계산 | 복잡 | 간단 ✅ |
| 수렴 속도 | 느림 | 빠름 ✅ |

---

#### 🔬 구체적 예시: ICP (Iterative Closest Point)

**문제**: 두 3D 포인트 클라우드를 정렬하는 회전 R과 이동 t 찾기

**목적 함수**:

```
E(R, t) = Σᵢ ‖R·pᵢ + t - qᵢ‖²

pᵢ: source 포인트
qᵢ: target 포인트
```

##### 기존 방식 (SVD)

```python
# 한 번에 최적해 계산 (analytical solution)
# 작은 문제에서만 가능
```

##### Lie 대수 방식 (반복 최적화)

```python
def optimize_pose_lie(source_points, target_points, max_iter=50):
    """
    Lie 대수를 이용한 포즈 최적화
    """
    # 초기화 (항등 변환)
    T = np.eye(4)
    
    for iteration in range(max_iter):
        # 현재 변환 적용
        transformed = transform_points(T, source_points)
        
        # 잔차 (residual) 계산
        residuals = transformed - target_points  # (N, 3)
        
        # Lie 대수에서 gradient 계산
        # (여기서는 단순화)
        grad_xi = compute_gradient(residuals, T)  # (6,) 벡터
        
        # Lie 대수에서 업데이트
        delta_xi = -learning_rate * grad_xi  # ∈ ℝ⁶, 제약 없음!
        
        # exp 매핑으로 증분 변환
        delta_T = exp_se3(delta_xi)  # SE(3), 항상 유효!
        
        # 변환 업데이트
        T = delta_T @ T  # ✅ 항상 유효한 변환
        
        # 수렴 확인
        if np.linalg.norm(delta_xi) < 1e-6:
            break
    
    return T
```

**핵심 관찰**:
- `delta_xi`는 제약 없는 6차원 벡터
- 일반적인 gradient descent 적용 가능
- `exp_se3`가 자동으로 유효한 변환 보장

---

#### 📐 Jacobian 계산의 간편함

**문제**: 어떤 함수 f(R)의 미분 계산

**기존 방식**:
```python
# R의 9개 원소로 미분?
# 제약 조건 때문에 복잡...
```

**Lie 대수 방식**:

```python
# ω의 3개 원소로 미분
# 제약 없으므로 일반적인 chain rule 적용!

def jacobian_lie(R, f):
    """
    f: SO(3) → ℝ 함수
    df/dω 계산 (3×1)
    """
    epsilon = 1e-8
    grad = np.zeros(3)
    
    for i in range(3):
        # 작은 perturbation
        delta_omega = np.zeros(3)
        delta_omega[i] = epsilon
        
        # 증분 회전
        delta_R = exp_so3(delta_omega)
        R_plus = delta_R @ R
        
        # 수치 미분
        grad[i] = (f(R_plus) - f(R)) / epsilon
    
    return grad
```

**장점**:
- 3개 변수만 미분 (vs 9개)
- 제약 조건 고려 불필요
- 수치 안정성 우수

---

#### 🎓 수학적 배경: 접선 공간의 의미

**왜 Lie 대수에서 미분하는가?**

```
manifold (곡면)에서의 최적화
    ↓
접선 공간 (tangent space)에서 방향 탐색
    ↓
exp 매핑으로 manifold 위로 이동
```

**비유**:

```
산 정상 찾기 (지구 표면 - 곡면)
    1. 현재 위치에서 경사 측정 (접선 공간)
    2. 경사 반대 방향 결정
    3. 그 방향으로 실제 이동 (곡면 위)
    4. 반복
```

Lie 대수가 바로 "접선 공간"입니다!

---

#### 🌟 최적화 패러다임 비교

**일반 벡터 공간 (ℝⁿ)**:

```python
x_new = x + Δx  # 단순 덧셈
```

**Lie 군 (manifold)**:

```python
R_new = exp(Δω) @ R  # 곱셈 + exp 매핑
```

**핵심 차이**:
- 벡터 공간: additive update (가산)
- Lie 군: multiplicative update (승산)

**왜 곱셈?**
- 회전의 합성 = 행렬 곱
- `exp(Δω)` = "작은 회전"
- `exp(Δω) @ R` = "기존 회전에 작은 회전 추가"

---

## 🤖 SLAM에서의 활용

### 🎯 실제 사용 예시

#### 1. Visual Odometry

**문제**: 연속된 이미지에서 카메라 이동 추정

```python
def estimate_camera_motion(image1, image2):
    """
    두 이미지 사이의 카메라 이동 (R, t) 추정
    """
    # 특징점 매칭
    matches = find_matches(image1, image2)
    
    # 초기 추정 (Essential Matrix)
    E = estimate_essential_matrix(matches)
    R_init, t_init = decompose_essential(E)
    
    # Lie 대수로 refinement
    T = np.block([[R_init, t_init], [np.zeros((1, 3)), 1]])
    
    for _ in range(10):  # 반복 최적화
        # 재투영 오차 계산
        errors = compute_reprojection_errors(T, matches)
        
        # se(3)에서 gradient
        grad_xi = compute_jacobian(T, matches, errors)
        
        # 업데이트
        delta_xi = -0.01 * grad_xi
        delta_T = exp_se3(delta_xi)
        T = delta_T @ T
    
    return T
```

**Lie 대수의 역할**:
- 6-DOF 포즈를 6개 파라미터로 최적화
- 제약 조건 자동 만족
- Jacobian 계산 간편

---

#### 2. IMU 사전적분 (VINS-Fusion) 사용

```cpp
#include <sophus/so3.hpp>
#include <sophus/se3.hpp>

// IMU 사전적분에서 회전 업데이트
Eigen::Quaterniond q = q * Utility::deltaQ(gyr * dt);

// deltaQ 내부: Lie 대수 사용
static Quaterniond deltaQ(const Vector3d &theta) {
    // exp 매핑과 동등
    ...
}
```

### Bundle Adjustment에서

- 카메라 포즈 최적화 시 6 DOF 대신 se(3)의 6차원 벡터 사용
- Jacobian 계산이 간단해짐

---

## 🔧 Sophus 라이브러리

### 📚 Sophus란?

**Sophus**는 Lie 군/대수를 C++에서 사용하기 쉽게 만든 라이브러리입니다.

**주요 기능**:
- SO(3), SE(3) 등의 Lie 군 구현
- exp/log 매핑 자동 처리
- Eigen 라이브러리와 완벽 통합
- 수치 안정성과 효율성 최적화

**설치**:
```bash
# Ubuntu
sudo apt-get install libsophus-dev

# 또는 소스에서 빌드
git clone https://github.com/strasdat/Sophus.git
cd Sophus && mkdir build && cd build
cmake .. && make && sudo make install
```

---

### 💻 C++에서 사용법

#### 기본 사용

```cpp
#include <iostream>
#include <sophus/so3.hpp>
#include <sophus/se3.hpp>
#include <Eigen/Core>

int main() {
    // 1. SO(3) 생성 방법들
    
    // 방법 1: Lie 대수에서 exp 매핑
    Eigen::Vector3d omega(0, 0, M_PI/2);  // Z축 90도
    Sophus::SO3d R1 = Sophus::SO3d::exp(omega);
    
    // 방법 2: 축 회전 (axis rotation)
    Sophus::SO3d R2 = Sophus::SO3d::rotZ(M_PI/2);  // Z축 90도
    Sophus::SO3d R3 = Sophus::SO3d::rotX(M_PI/4);  // X축 45도
    Sophus::SO3d R4 = Sophus::SO3d::rotY(M_PI/3);  // Y축 60도
    
    // 방법 3: 회전 행렬에서
    Eigen::Matrix3d R_mat;
    R_mat << 0, -1, 0,
             1,  0, 0,
             0,  0, 1;
    Sophus::SO3d R5(R_mat);
    
    // 방법 4: 쿼터니언에서
    Eigen::Quaterniond q(1, 0, 0, 0);  // w, x, y, z
    Sophus::SO3d R6(q);
    
    // 2. SO(3) 사용
    
    // 회전 행렬 가져오기
    Eigen::Matrix3d mat = R1.matrix();
    
    // 쿼터니언으로 변환
    Eigen::Quaterniond quat = R1.unit_quaternion();
    
    // log 매핑 (SO(3) → so(3))
    Eigen::Vector3d log_omega = R1.log();
    
    // 포인트 회전
    Eigen::Vector3d p(1, 0, 0);
    Eigen::Vector3d p_rotated = R1 * p;
    
    // 회전 합성
    Sophus::SO3d R_combined = R1 * R2;
    
    // 역회전
    Sophus::SO3d R_inv = R1.inverse();
    
    std::cout << "Rotation matrix:\n" << R1.matrix() << std::endl;
    
    return 0;
}
```

#### SE(3) 사용

```cpp
#include <sophus/se3.hpp>

int main() {
    // 1. SE(3) 생성
    
    // 방법 1: R과 t로부터
    Sophus::SO3d R = Sophus::SO3d::rotZ(M_PI/2);
    Eigen::Vector3d t(1, 2, 3);
    Sophus::SE3d T1(R, t);
    
    // 방법 2: Lie 대수에서 exp 매핑
    Eigen::Matrix<double, 6, 1> xi;
    xi << 1, 2, 3,      // ρ (이동 관련)
          0.1, 0.2, 0.3; // φ (회전)
    Sophus::SE3d T2 = Sophus::SE3d::exp(xi);
    
    // 방법 3: 4×4 행렬에서
    Eigen::Matrix4d T_mat = Eigen::Matrix4d::Identity();
    T_mat.block<3, 3>(0, 0) = R.matrix();
    T_mat.block<3, 1>(0, 3) = t;
    Sophus::SE3d T3(T_mat);
    
    // 2. SE(3) 사용
    
    // 회전/이동 부분 가져오기
    Sophus::SO3d rotation = T1.so3();
    Eigen::Vector3d translation = T1.translation();
    
    // 4×4 행렬로 변환
    Eigen::Matrix4d transform = T1.matrix();
    
    // log 매핑
    Eigen::Matrix<double, 6, 1> log_xi = T1.log();
    
    // 포인트 변환
    Eigen::Vector3d p(1, 0, 0);
    Eigen::Vector3d p_transformed = T1 * p;
    
    // 변환 합성
    Sophus::SE3d T_combined = T1 * T2;
    
    // 역변환
    Sophus::SE3d T_inv = T1.inverse();
    
    std::cout << "Transform matrix:\n" << T1.matrix() << std::endl;
    
    return 0;
}
```

#### 최적화 예시 (Ceres Solver와 함께)

```cpp
#include <ceres/ceres.h>
#include <sophus/se3.hpp>

// SE(3) 최적화를 위한 Cost Function
class SE3CostFunction {
public:
    SE3CostFunction(const Eigen::Vector3d& observed,
                    const Eigen::Vector3d& landmark)
        : observed_(observed), landmark_(landmark) {}
    
    template <typename T>
    bool operator()(const T* const pose_params, T* residuals) const {
        // pose_params는 se(3)의 6차원 벡터
        Eigen::Map<const Eigen::Matrix<T, 6, 1>> xi(pose_params);
        
        // exp 매핑으로 SE(3) 생성
        Sophus::SE3<T> pose = Sophus::SE3<T>::exp(xi);
        
        // 랜드마크 변환
        Eigen::Matrix<T, 3, 1> predicted = pose * landmark_.cast<T>();
        
        // 잔차 계산
        residuals[0] = predicted[0] - T(observed_[0]);
        residuals[1] = predicted[1] - T(observed_[1]);
        residuals[2] = predicted[2] - T(observed_[2]);
        
        return true;
    }
    
private:
    Eigen::Vector3d observed_;
    Eigen::Vector3d landmark_;
};

// 사용 예시
void optimizePose() {
    // 초기 포즈 (se(3) 파라미터)
    Eigen::Matrix<double, 6, 1> pose_params = 
        Eigen::Matrix<double, 6, 1>::Zero();
    
    // Ceres 문제 설정
    ceres::Problem problem;
    
    // 여러 관측 추가
    for (const auto& observation : observations) {
        ceres::CostFunction* cost_function =
            new ceres::AutoDiffCostFunction<SE3CostFunction, 3, 6>(
                new SE3CostFunction(observation.point, observation.landmark));
        
        problem.AddResidualBlock(cost_function, nullptr, pose_params.data());
    }
    
    // 최적화 실행
    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    
    // 최종 포즈
    Sophus::SE3d final_pose = Sophus::SE3d::exp(pose_params);
    std::cout << "Optimized pose:\n" << final_pose.matrix() << std::endl;
}
```

---

### 🐍 Python에서 사용법

Python에는 Sophus의 직접적인 포팅이 없지만, `scipy`와 직접 구현으로 대체할 수 있습니다.

#### scipy.spatial.transform.Rotation 사용

```python
import numpy as np
from scipy.spatial.transform import Rotation

# 1. Rotation 생성 방법들

# 방법 1: 회전 벡터 (axis-angle, Lie 대수와 같음!)
rotvec = np.array([0, 0, np.pi/2])  # Z축 90도
R1 = Rotation.from_rotvec(rotvec)

# 방법 2: Euler 각
R2 = Rotation.from_euler('xyz', [0, 0, np.pi/2])  # roll, pitch, yaw

# 방법 3: 쿼터니언
quat = np.array([0, 0, np.sin(np.pi/4), np.cos(np.pi/4)])  # x,y,z,w
R3 = Rotation.from_quat(quat)

# 방법 4: 회전 행렬
mat = np.array([
    [0, -1, 0],
    [1,  0, 0],
    [0,  0, 1]
])
R4 = Rotation.from_matrix(mat)

# 2. Rotation 사용

# 회전 행렬로 변환
rotation_matrix = R1.as_matrix()
print("Rotation matrix:\n", rotation_matrix)

# 회전 벡터로 변환 (log 매핑과 같음!)
rotvec_back = R1.as_rotvec()
print("Rotation vector:", rotvec_back)

# 쿼터니언으로 변환
quat = R1.as_quat()  # [x, y, z, w]
print("Quaternion:", quat)

# Euler 각으로 변환
euler = R1.as_euler('xyz')
print("Euler angles:", euler)

# 포인트 회전 (apply 메서드)
p = np.array([1, 0, 0])
p_rotated = R1.apply(p)
print("Rotated point:", p_rotated)

# 여러 포인트 한 번에 회전
points = np.array([[1, 0, 0], [0, 1, 0], [0, 0, 1]])
points_rotated = R1.apply(points)

# 회전 합성
R_combined = R1 * R2  # 먼저 R2, 그 다음 R1

# 역회전
R_inv = R1.inv()
```

#### SE(3) 직접 구현

```python
import numpy as np
from scipy.spatial.transform import Rotation

class SE3:
    """SE(3) 변환 클래스"""
    
    def __init__(self, R=None, t=None):
        """
        Args:
            R: Rotation 객체 또는 3×3 numpy array
            t: 3×1 numpy array
        """
        if R is None:
            self.R = Rotation.identity()
        elif isinstance(R, Rotation):
            self.R = R
        else:
            self.R = Rotation.from_matrix(R)
        
        if t is None:
            self.t = np.zeros(3)
        else:
            self.t = np.array(t).flatten()
    
    @classmethod
    def from_matrix(cls, T):
        """4×4 변환 행렬에서 생성"""
        R = T[:3, :3]
        t = T[:3, 3]
        return cls(R, t)
    
    @classmethod
    def exp(cls, xi):
        """se(3)에서 exp 매핑"""
        rho = xi[:3]  # 이동 관련
        phi = xi[3:]  # 회전
        
        R = Rotation.from_rotvec(phi)
        
        # Jacobian 계산
        theta = np.linalg.norm(phi)
        if theta < 1e-10:
            J = np.eye(3)
        else:
            axis = phi / theta
            K = skew(axis)
            J = np.eye(3) + ((1 - np.cos(theta)) / theta) * K \
                + ((theta - np.sin(theta)) / theta) * (K @ K)
        
        t = J @ rho
        return cls(R, t)
    
    def matrix(self):
        """4×4 변환 행렬 반환"""
        T = np.eye(4)
        T[:3, :3] = self.R.as_matrix()
        T[:3, 3] = self.t
        return T
    
    def log(self):
        """log 매핑: SE(3) → se(3)"""
        phi = self.R.as_rotvec()
        
        # Jacobian 역계산
        theta = np.linalg.norm(phi)
        if theta < 1e-10:
            J_inv = np.eye(3)
        else:
            axis = phi / theta
            K = skew(axis)
            J_inv = np.eye(3) - 0.5 * K \
                + (1 - theta / (2 * np.tan(theta/2))) * (K @ K) / (theta**2)
        
        rho = J_inv @ self.t
        return np.hstack([rho, phi])
    
    def transform_point(self, p):
        """포인트 변환"""
        return self.R.apply(p) + self.t
    
    def __mul__(self, other):
        """변환 합성"""
        if isinstance(other, SE3):
            R_new = self.R * other.R
            t_new = self.R.apply(other.t) + self.t
            return SE3(R_new, t_new)
        else:
            # numpy array (포인트)
            return self.transform_point(other)
    
    def inverse(self):
        """역변환"""
        R_inv = self.R.inv()
        t_inv = -R_inv.apply(self.t)
        return SE3(R_inv, t_inv)

def skew(v):
    """반대칭 행렬"""
    return np.array([
        [0, -v[2], v[1]],
        [v[2], 0, -v[0]],
        [-v[1], v[0], 0]
    ])

# 사용 예시
if __name__ == "__main__":
    # SE(3) 생성
    R = Rotation.from_euler('z', np.pi/2)
    t = np.array([1, 2, 3])
    T = SE3(R, t)
    
    print("Transform matrix:\n", T.matrix())
    
    # 포인트 변환
    p = np.array([1, 0, 0])
    p_transformed = T * p
    print("Transformed point:", p_transformed)
    
    # log 매핑
    xi = T.log()
    print("se(3) vector:", xi)
    
    # exp 매핑으로 복원
    T_recovered = SE3.exp(xi)
    print("Recovered matrix:\n", T_recovered.matrix())
```

---

### 🔑 핵심 정리: Sophus 사용 팁

1. **C++에서는 Sophus를 적극 활용**
   - Eigen과 완벽한 통합
   - VINS-Fusion, ORB-SLAM 등 대부분의 SLAM 라이브러리가 사용

2. **Python에서는 scipy.Rotation + 직접 구현**
   - scipy.Rotation이 SO(3) 역할
   - SE(3)는 간단한 래퍼 클래스로 구현

3. **주의사항**
   - Sophus의 로그는 **행렬**이 아닌 **벡터** 반환
   - exp/log 매핑은 수치적으로 안정적이지만 특이점 주의 (θ = π)

4. **최적화 시**
   - Sophus 객체를 직접 최적화 변수로 사용하지 말고
   - se(3) 또는 so(3)의 **벡터 표현**(6D 또는 3D)을 사용


---

## 💻 실습 파일

| 파일 | 내용 | 난이도 |
|------|------|--------|
| `lie_basics.py` | Lie 군/대수 기본 연산 (skew, exp, log 구현) | ⭐⭐ |
| `lie_quiz.py` | 개념 확인 퀴즈 및 실습 문제 | ⭐⭐⭐ |

### 실습 가이드

**lie_basics.py 실습 내용**:
1. `skew()`, `vee()` 함수 구현
2. `exp_so3()` - Rodrigues 공식 구현
3. `log_so3()` - 역매핑 구현
4. `exp_se3()`, `log_se3()` - SE(3) 매핑
5. 다양한 회전 검증 및 시각화

**lie_quiz.py 실습 내용**:
1. SO(3) 원소 확인 (RᵀR = I, det(R) = 1)
2. exp/log 매핑의 역함수 관계 검증
3. 회전 합성과 Lie 대수의 관계
4. 간단한 포즈 최적화 문제
5. ICP 알고리즘 구현

**권장 학습 순서**:
1. `README.md` 이론 학습 (2시간)
2. `lie_basics.py` 코드 실행 및 이해 (1시간)
3. `lie_quiz.py` 문제 풀이 (1-2시간)
4. 필요시 이론 복습

---

## 📊 종합 정리

### 🔄 전체 흐름도

```
문제: 회전/변환 최적화 어려움
    ↓
원인: Over parameterization + 제약 조건
    ↓
해결: Lie 군/대수 프레임워크
    
┌─────────────────────────────────────────┐
│           Lie 군 (SO(3), SE(3))          │
│        (회전/변환 - 곡면/manifold)          │
│                                         │
│  - 실제로 사용하는 공간                      │
│  - 제약 조건 있음 (RᵀR=I 등)                │
│  - 직접 최적화 어려움                        │
└─────────────────────────────────────────┘
         ↑                    ↓
    exp 매핑              log 매핑
         ↑                    ↓
┌─────────────────────────────────────────┐
│          Lie 대수 (so(3), se(3))         │
│         (접선 공간 - 평면/벡터)             │
│                                         │
│  - 최적화하는 공간                          │
│  - 제약 조건 없음                          │
│  - 일반적인 gradient descent 사용          │
└─────────────────────────────────────────┘
```

### 🎯 핵심 개념 맵

```
Lie 군/대수 = 회전 최적화의 해답

┌─────────────┐
│  문제 정의   │
└──────┬──────┘
       │
       ├─► Over-parametrization
       │   - 회전: 9→3  (6개 제약)
       │   - 쿼터니언: 4→3  (1개 제약)
       │
       ├─► 최적화 곤란
       │   - R_new = R + ΔR  ❌
       │   - 제약 조건 위반
       │
       └─► 해결책: Lie 대수
           │
           ├─► 군(Group) 이론
           │   - SO(3): 회전의 집합
           │   - SE(3): 변환의 집합
           │
           ├─► Lie 대수 (접선 공간)
           │   - so(3): 3D 벡터
           │   - se(3): 6D 벡터
           │   - 제약 없음!
           │
           ├─► exp/log 매핑
           │   - exp: 벡터 → 회전
           │   - log: 회전 → 벡터
           │   - Rodrigues 공식
           │
           └─► 최적화
               - Δω = -lr * grad  (제약 없음)
               - R_new = exp(Δω) @ R  (항상 유효)
               - 빠르고 안정적
```

### 📝 핵심 공식 정리

| 항목 | 공식 | 의미 |
|------|------|------|
| **SO(3) 정의** | `RᵀR = I, det(R) = 1` | 3D 회전 행렬 조건 |
| **SE(3) 정의** | `T = [R│t; 0│1]` | 강체 변환 (회전+이동) |
| **so(3) 벡터** | `ω ∈ ℝ³` | 회전축 × 각도 |
| **so(3) 행렬** | `ω^ = skew(ω)` | 3×3 반대칭 행렬 |
| **se(3) 벡터** | `ξ = [ρ, φ]ᵀ ∈ ℝ⁶` | 6-DOF 표현 |
| **Rodrigues** | `R = I + sin(θ)K + (1-cos(θ))K²` | exp 매핑 공식 |
| **log 매핑** | `θ = arccos((tr(R)-1)/2)` | 회전 각도 복원 |
| **최적화** | `R_new = exp(Δω) @ R` | 업데이트 공식 |

### 💡 직관적 이해를 위한 비유

| 개념 | 일상 비유 | 수학적 대응 |
|------|---------|-----------|
| **Lie 군** | 지구 표면 (구면) | SO(3), SE(3) |
| **Lie 대수** | 평평한 지도 (평면) | so(3), se(3) |
| **exp 매핑** | 지도 → 지구 투영 | 벡터 → 회전 |
| **log 매핑** | 지구 → 지도 변환 | 회전 → 벡터 |
| **접선 공간** | 한 점에서의 평면 | 항등원에서의 Lie 대수 |
| **최적화** | 산 정상 찾기 | 회전 파라미터 조정 |

### ⚡ 핵심 장점 요약

**Lie 대수를 사용하면**:

✅ **파라미터 효율성**
- 회전: 9개 → 3개 (66% 감소)
- SE(3): 16개 → 6개 (62% 감소)

✅ **제약 조건 자동 만족**
- exp 매핑이 자동으로 유효한 회전 생성
- 별도의 투영/정규화 불필요

✅ **계산 효율성**
- Gradient 계산 단순화
- Jacobian 크기 감소
- 수치 안정성 향상

✅ **이론적 우아함**
- 미분 기하학적 근거
- manifold 최적화의 표준 방법
- 현대 SLAM의 필수 도구

---

## ✅ 학습 완료 체크리스트

### 기초 이해 (필수)

- [ ] **Over-parameterization 문제**
  - [ ] 회전 행렬이 왜 9개가 아닌 3 자유도인지 설명 가능
  - [ ] 쿼터니언의 제약 조건(‖q‖=1) 이해
  - [ ] 최적화 시 R_new = R + ΔR이 왜 안 되는지 설명 가능

- [ ] **군(Group) 개념**
  - [ ] 4가지 군 공리(닫힘, 결합, 항등원, 역원) 이해
  - [ ] SO(3)가 무엇인지 설명 가능 (모든 3D 회전의 집합)
  - [ ] SE(3)가 무엇인지 설명 가능 (모든 강체 변환의 집합)

- [ ] **Lie 대수 개념**
  - [ ] Lie 대수가 "접선 공간"이라는 의미 이해
  - [ ] so(3)의 두 가지 표현(벡터, 반대칭 행렬) 알기
  - [ ] se(3)의 6차원 구조 이해

- [ ] **exp/log 매핑**
  - [ ] exp: Lie 대수 → Lie 군 (예: 벡터 → 회전)
  - [ ] log: Lie 군 → Lie 대수 (예: 회전 → 벡터)
  - [ ] 두 매핑이 역함수 관계임을 이해

### 실용 활용 (권장)

- [ ] **Rodrigues 공식**
  - [ ] 공식의 형태 기억: `R = I + sin(θ)K + (1-cos(θ))K²`
  - [ ] 간단한 예시 (Z축 90도) 손으로 계산 가능

- [ ] **최적화 이해**
  - [ ] Lie 대수 방식의 업데이트 과정 설명 가능
  - [ ] 왜 Lie 대수를 쓰면 편한지 3가지 이상 나열

- [ ] **Sophus 활용**
  - [ ] C++에서 SO3d, SE3d 기본 사용법 알기
  - [ ] Python에서 scipy.Rotation 사용법 알기

### 심화 (선택)

- [ ] **수학적 배경**
  - [ ] manifold와 tangent space 개념 이해
  - [ ] Taylor 급수에서 Rodrigues 공식 유도 과정 이해
  - [ ] Jacobian 역할 이해 (SE(3)에서 ρ와 t의 관계)

- [ ] **SLAM 적용**
  - [ ] Visual Odometry에서 Lie 대수 역할 이해
  - [ ] Bundle Adjustment에서의 사용 이해
  - [ ] IMU 사전적분에서의 활용 이해

---

## 🎓 학습 팁

### 초심자를 위한 권장 학습 경로

1. **1단계: 동기 이해** (30분)
   - "왜 필요한가" 섹션 집중 학습
   - Over-parameterization 문제 완전히 이해
   - **목표**: Lie 대수가 있으면 뭐가 좋은지 명확히 알기

2. **2단계: 기본 개념** (1시간)
   - 군의 4가지 성질 이해
   - SO(3), SE(3) 예시 코드 실행해보기
   - **목표**: 추상적 개념을 구체적 예시로 연결

3. **3단계: 핵심 도구** (1시간)
   - exp/log 매핑 이해
   - Rodrigues 공식 적용 예시 실습
   - **목표**: "어떻게 사용하는가" 명확히 알기

4. **4단계: 실습** (1-2시간)
   - `lie_basics.py` 실행 및 수정
   - `lie_quiz.py` 문제 풀이
   - **목표**: 코드로 개념 확인

5. **5단계: 실전 준비** (선택)
   - Sophus 라이브러리 사용
   - 간단한 최적화 문제 구현
   - **목표**: 실제 프로젝트 적용 준비

### 이해가 안 될 때

**막힐 때 시도해볼 것**:

1. **구체적 예시로 돌아가기**
   - 추상적 개념 → Z축 90도 회전 예시로 확인

2. **시각화**
   - 회전 행렬을 실제로 포인트에 적용
   - matplotlib로 회전 결과 그려보기

3. **단계별 확인**
   - 큰 개념을 작은 단계로 나누기
   - 각 단계마다 코드로 검증

4. **비유 활용**
   - 지구/지도 비유로 직관 키우기
   - 산 정상 찾기로 최적화 이해

### 다음 단계 준비

Week 6 완료 후 다음을 확인하세요:

✅ **이 내용들을 설명할 수 있나요?**
- "왜 회전 행렬을 직접 최적화하면 안 되나요?"
- "Lie 대수는 무엇이고, 왜 유용한가요?"
- "exp 매핑은 뭐하는 건가요?"

**위 질문에 답할 수 있다면** → Week 7으로 진행 ✅  
**어렵다면** → 실습 파일 한 번 더 실행 후 재도전

---

## 🔗 다음 단계

### Week 7: 최소자승법 및 비선형 최적화

Week 6에서 배운 Lie 대수를 실제로 활용하는 방법을 배웁니다:

**학습 내용**:
- 비용 함수(Cost Function)와 잔차(Residual)
- 정규방정식(Normal Equation)
- Gauss-Newton, Levenberg-Marquardt
- **SLAM의 기본 최적화 형태**

**연결 고리**:
- Week 6: Lie 대수로 회전 **표현** 방법 학습
- Week 7: Lie 대수로 회전 **최적화** 방법 학습

이제 회전을 어떻게 표현하는지 알았으니, 이를 어떻게 조정해서 최적값을 찾는지 배웁니다!

---

## 📚 참고 자료

### 추천 자료 (난이도 순)

1. **입문**
   - [Lie Groups for 2D and 3D Transformations](http://ethaneade.com/lie.pdf) - Ethan Eade
   - 가장 친절한 입문 자료, 실용적 접근

2. **중급**
   - 고상호의 《SLAM KR》 - Lie 군/대수 장
   - 한글 자료, 예시 풍부

3. **고급**
   - "State Estimation for Robotics" - Timothy Barfoot
   - 로봇공학 관점의 체계적 설명
   - [무료 PDF](http://asrl.utias.utoronto.ca/~tdb/bib/barfoot_ser17.pdf)

4. **수학적 배경**
   - "A Micro Lie Theory for State Estimation in Robotics" - Joan Solà
   - 수학적으로 엄밀한 유도
   - [arXiv](https://arxiv.org/abs/1812.01537)

### 온라인 강의

- **Cyrill Stachniss - Photogrammetry**
  - [YouTube Playlist](https://www.youtube.com/playlist?list=PLgnQpQtFTOGRsi5vzy9PiQpNWHjq-bKN1)
  - Lecture 5-6에서 Lie 군/대수 다룸

---

## ❓ FAQ

**Q1: Lie 군/대수를 꼭 알아야 하나요?**  
A: 현대 SLAM (특히 Visual-Inertial SLAM)을 이해하려면 필수입니다. VINS-Fusion, ORB-SLAM3 등 대부분의 최신 SLAM은 Lie 대수를 사용합니다.

**Q2: 수학이 너무 어려워요**  
A: 처음에는 "어떻게 사용하는가"에 집중하세요. 수학적 유도는 나중에 Phase 5 (VINS-Fusion 코드 분석)에서 필요할 때 다시 보면 됩니다.

**Q3: exp/log 매핑을 외워야 하나요?**  
A: Sophus 라이브러리가 다 해줍니다! Rodrigues 공식의 형태만 기억하고, 실제로는 라이브러리를 사용하세요.

**Q4: Python으로 실습해도 되나요?**  
A: 네! 이론 학습에는 Python이 더 쉽습니다. 나중에 실제 SLAM 구현 시 C++로 전환하면 됩니다.

**Q5: 이해는 못 했는데 넘어가도 될까요?**  
A: Week 6은 "가볍게 훑고", Phase 5에서 다시 볼 예정입니다. 기본 개념만 알고 넘어가도 OK!

---

**🎉 Week 6 학습을 마치셨습니다!**

Lie 군/대수는 어려운 주제지만, SLAM의 핵심 도구입니다.  
완벽히 이해하지 못해도 괜찮습니다. 계속 진행하면서 자연스럽게 익숙해질 것입니다.

**다음**: Week 7 - 최소자승법 및 비선형 최적화 🚀
