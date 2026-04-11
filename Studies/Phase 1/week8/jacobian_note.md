# Jacobian Matrix 심층 분석

## 1. Jacobian이란 무엇인가?

Jacobian(자코비안)은 **"다변수 함수를 1차 선형 근사(Linear Approximation)하기 위한 기울기 행렬"** 입니다.
단변수 함수 $y=f(x)$에서 기울기가 $\frac{dy}{dx}$라면, 다변수 벡터 함수 $\mathbf{y} = F(\mathbf{x})$에서는 Jacobian 행렬 $\mathbf{J}$가 그 역할을 합니다.

$$ \mathbf{y}_{new} \approx \mathbf{y}_{old} + \mathbf{J} \cdot \Delta \mathbf{x} $$

---

## 2. 왜 10x2 행렬인가?

이 크기는 **"관측한 데이터의 개수"** 와 **"최적화할 파라미터의 개수"** 에 의해 결정됩니다.

### (1) 행(Row)의 의미: 데이터 포인트 (10개)
우리는 $x$값 0부터 2까지 10개의 데이터를 가지고 있습니다.
우리의 모델이 이 10개의 점 각각에 대해 예측값 $f(x_i)$를 내놓으므로, 오차(Residual)도 10개가 생깁니다.
각 행은 **"특정 데이터 포인트 $i$에서의 변화"** 를 나타냅니다.

### (2) 열(Column)의 의미: 파라미터 (2개)
우리가 조절할 수 있는 나사는 $a$와 $b$ 두 개뿐입니다.
각 열은 **"특정 파라미터가 변할 때 결과가 얼마나 변하는지"**를 나타냅니다.

*   **1열 ($\frac{\partial f}{\partial a}$)**: $a$를 아주 조금 바꿀 때, 10개의 예측값이 각각 어떻게 변하는가?
*   **2열 ($\frac{\partial f}{\partial b}$)**: $b$를 아주 조금 바꿀 때, 10개의 예측값이 각각 어떻게 변하는가?

결국 **(데이터 10개) $\times$ (파라미터 2개) = $10 \times 2$ 행렬** 이 됩니다.

---

## 3. 수식으로 직접 뜯어보기

우리의 모델 함수:
$$ f(x; a, b) = a \cdot e^{b \cdot x} $$

이 함수를 파라미터 $a$와 $b$로 각각 편미분(Partial Derivative)해봅시다.

### 첫 번째 열: $a$에 대한 미분
$a$만 변수라고 생각하고 미분합니다. ($b$와 $x$는 상수로 취급)
$$ \frac{\partial f}{\partial a} = \frac{\partial}{\partial a} (a \cdot e^{bx}) = 1 \cdot e^{bx} = e^{bx} $$

### 두 번째 열: $b$에 대한 미분
$b$만 변수라고 생각하고 미분합니다. ($a$와 $x$는 상수로 취급)
연쇄 법칙(Chain Rule)을 사용합니다.
$$ \frac{\partial f}{\partial b} = \frac{\partial}{\partial b} (a \cdot e^{bx}) = a \cdot \frac{\partial}{\partial b}(e^{bx}) = a \cdot (x \cdot e^{bx}) = ax e^{bx} $$

### 최종 Jacobian 행렬 형태
데이터가 $x_1, x_2, \dots, x_{10}$ 이렇게 10개가 있다면, Jacobian 행렬 $J$는 다음과 같이 구성됩니다.

$$
\mathbf{J} = 
\begin{bmatrix}
\frac{\partial f}{\partial a}(x_1) & \frac{\partial f}{\partial b}(x_1) \\
\frac{\partial f}{\partial a}(x_2) & \frac{\partial f}{\partial b}(x_2) \\
\vdots & \vdots \\
\frac{\partial f}{\partial a}(x_{10}) & \frac{\partial f}{\partial b}(x_{10})
\end{bmatrix}
=
\begin{bmatrix}
e^{b x_1} & a x_1 e^{b x_1} \\
e^{b x_2} & a x_2 e^{b x_2} \\
\vdots & \vdots \\
e^{b x_{10}} & a x_{10} e^{b x_{10}}
\end{bmatrix}
$$

이 행렬이 바로 코드의 `jacobian` 함수가 계산하고 있는 값입니다.

---

## 4. 직관적 예시 (코드의 값 대입)

코드에서 `params_init = [1.0, 0.5]`였습니다. 즉 $a=1, b=0.5$입니다.
데이터 첫 번째 점이 $x_0 = 0$이라고 해봅시다.

*   **1열 1행값 ($\partial f / \partial a$)**:
    $e^{0.5 \times 0} = e^0 = 1$
    -> 의미: $a$를 0.01 늘리면, 첫 번째 데이터 예측값도 0.01만큼(1배) 늘어난다.

*   **2열 1행값 ($\partial f / \partial b$)**:
    $1 \cdot 0 \cdot e^{0.5 \times 0} = 0$
    -> 의미: $b$를 아무리 늘려도, 첫 번째 데이터($x=0$) 예측값은 변하지 않는다. (왜냐하면 $x=0$일 때 $e^{bx}$는 항상 1이므로 지수승 $b$가 힘을 못 씀)

이런 정보들이 모여서 **"오차를 줄이려면 $a$를 고쳐야 할지 $b$를 고쳐야 할지"** 방향을 알려줍니다.
