# Week 4: 회전 표현 (Rotation Representations)


## 개요


3D 공간에서 물체의 **방향(orientation)** 을 표현하는 방법은 여러 가지가 있습니다. SLAM에서 카메라나 로봇의 자세를 추정할 때 회전은 핵심 요소이며, 어떤 표현 방법을 사용하느냐에 따라 계산 효율성과 안정성이 달라집니다.


이번 주에는 **회전 행렬, 오일러 각, 쿼터니언** 세 가지 회전 표현 방법을 학습하고, 각각의 장단점과 SLAM에서의 활용을 이해합니다.


## 학습 목표


1. 2D/3D 회전 행렬 유도 및 성질 이해
2. 오일러 각의 정의와 짐벌락 문제 이해
3. 쿼터니언의 기본 연산 숙달
4. 회전 표현 간 상호 변환
5. 실전 Perception/로봇 시스템에서 쿼터니언 사용 이유 이해


## 사전 지식


- Week 1-3에서 학습한 선형대수 (행렬 곱셈, 직교 행렬, SVD)
- 삼각함수 기본 (sin, cos)
- 복소수 기본 개념 (선택)


## 예상 학습 시간


| 항목 | 시간 |
|------|------|
| 이론 학습 | 2-3시간 |
| 실습 예제 | 2-3시간 |
| SLAM 연결 이해 | 1-2시간 |
| **총 소요시간** | **5-8시간** |


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | Python 기초 실습 | `rotation_basics.py` | 회전 표현 구현 및 상호 변환 실습 |
| 3 | Python 심화 퀴즈 | `rotation_quiz.py` | 주관식 퀴즈 (문제/답안 분리) |


---


## 핵심 개념


### 1. 회전 행렬 (Rotation Matrix)


#### 2D 회전 행렬


각도 θ만큼 반시계방향으로 회전:


```
R(θ) = [cos(θ) -sin(θ)]
       [sin(θ) cos(θ)]
```


```python
import numpy as np


def rotation_2d(theta):
    """2D 회전 행렬"""
    c, s = np.cos(theta), np.sin(theta)
    return np.array([
        [c, -s],
        [s, c]
    ])
```


#### 3D 회전 행렬


각 축 기준 회전:


| 축 | 회전 행렬 |
|----|----------|
| **X축** | Rx(θ) = [[1, 0, 0], [0, cos(θ), -sin(θ)], [0, sin(θ), cos(θ)]] |
| **Y축** | Ry(θ) = [[cos(θ), 0, sin(θ)], [0, 1, 0], [-sin(θ), 0, cos(θ)]] |
| **Z축** | Rz(θ) = [[cos(θ), -sin(θ), 0], [sin(θ), cos(θ), 0], [0, 0, 1]] |


#### 회전 행렬의 핵심 성질


| 성질 | 수식 | 의미 |
|------|------|------|
| **직교 행렬** | RᵀR = I | 길이/각도 보존 |
| **det(R) = 1** | det(R) = +1 | 순수 회전 (반사 없음) |
| **역행렬 = 전치** | R⁻¹ = Rᵀ | 역회전이 쉬움 |
| **그룹 구조** | R₁R₂ ∈ SO(3) | 회전 합성도 회전 |


#### SO(3): 3차원 회전 행렬의 집합


> [!IMPORTANT]
> **SO(3)는 하나의 회전 행렬이 아니라, 모든 3차원 회전 행렬들의 집합(군, group)입니다.**


**SO(3)의 정의**:


```
SO(3) = Special Orthogonal Group in 3 dimensions
      = {R ∈ ℝ³ˣ³ | RᵀR = I, det(R) = 1}
```


이는 다음을 만족하는 모든 3×3 행렬 R의 집합입니다:
- **Orthogonal (직교)**: RᵀR = I
- **Special (특수)**: det(R) = 1


**수학적 표기법 이해**:


| 표기 | 의미 | 예시 |
|------|------|------|
| `R ∈ SO(3)` | R은 SO(3)의 **원소** | 하나의 회전 행렬 |
| `SO(3)` | 모든 3D 회전 행렬의 **집합** | 무한히 많은 회전들 |
| `R₁, R₂ ∈ SO(3)` | R₁과 R₂ 모두 회전 행렬 | - |
| `R₁R₂ ∈ SO(3)` | 회전의 곱도 회전 | 닫혀있음 (closure) |


**Python 예시**:


```python
import numpy as np


# 이것은 SO(3)의 한 원소 (element of SO(3))
R_x90 = np.array([
    [1, 0, 0],
    [0, 0, -1],
    [0, 1, 0]
]) # x축 기준 90도 회전 → R_x90 ∈ SO(3)


# 이것도 SO(3)의 한 원소
R_z90 = np.array([
    [0, -1, 0],
    [1, 0, 0],
    [0, 0, 1]
]) # z축 기준 90도 회전 → R_z90 ∈ SO(3)


# 두 회전의 곱도 SO(3)에 속함
R_combined = R_z90 @ R_x90 # R_combined ∈ SO(3)


# 검증
print("직교성:", np.allclose(R_combined.T @ R_combined, np.eye(3))) # True
print("det(R):", np.linalg.det(R_combined)) # 1.0
```


**SLAM에서의 연관 개념**:


| 표기법 | 차원 | 의미 |
|--------|------|------|
| **SO(3)** | 3×3 | 3D 회전만 (9개 파라미터, 3 자유도) |
| **SE(3)** | 4×4 | 3D 회전 + 평행이동 (Week 5에서 학습) |
| **so(3)** | 3×3 | SO(3)의 Lie 대수 (회전의 미분, Week 6) |


> [!TIP]
> - "하나의 3D 회전 행렬" → `R ∈ SO(3)`로 표현
> - "모든 3D 회전 행렬" → `SO(3)` 자체를 의미
> - Visual SLAM에서 카메라 자세 R은 항상 SO(3)의 원소입니다


---


### 2. 오일러 각 (Euler Angles)


#### Roll-Pitch-Yaw 정의


| 각도 | 축 | 설명 |
|------|-----|------|
| **Roll (φ)** | X축 | 좌우 기울임 |
| **Pitch (θ)** | Y축 | 앞뒤 기울임 |
| **Yaw (ψ)** | Z축 | 좌우 회전 (방향) |


```python
def euler_to_rotation(roll, pitch, yaw):
    """오일러 각 → 회전 행렬 (ZYX 순서)"""
    Rx = rotation_x(roll)
    Ry = rotation_y(pitch)
    Rz = rotation_z(yaw)
    return Rz @ Ry @ Rx # 순서 중요!
```


#### 짐벌락 (Gimbal Lock) 문제


> [!WARNING]
> **짐벌락**: pitch가 ±90°일 때 roll과 yaw가 동일한 축을 표현하게 되어 **자유도가 3 → 2로 감소**하는 현상


```
Pitch = 90°일 때:
- Roll과 Yaw가 같은 효과
- 독립적인 3축 제어 불가
- 값이 급격히 변동 (불안정)
```


**왜 문제인가?**
- 드론/비행기: 급상승/급하강 시 발생 가능
- SLAM: 카메라가 위를 보거나 아래를 볼 때
- 보간(interpolation) 시 비정상적 경로


---


### 3. 쿼터니언 (Quaternion)


#### 기본 정의


쿼터니언은 4개의 숫자로 3D 회전을 표현:


```
q = w + xi + yj + zk = (w, x, y, z)


단위 쿼터니언 조건: w² + x² + y² + z² = 1
```


**축-각(Axis-Angle) 표현과의 관계**:
축 **n**을 기준으로 각도 θ만큼 회전:


```
q = (cos(θ/2), sin(θ/2) * nx, sin(θ/2) * ny, sin(θ/2) * nz)
```


#### 쿼터니언 연산


```python
def quaternion_multiply(q1, q2):
    """쿼터니언 곱셈 (Hamilton 곱)"""
    w1, x1, y1, z1 = q1
    w2, x2, y2, z2 = q2
    

    return np.array([
        w1*w2 - x1*x2 - y1*y2 - z1*z2,
        w1*x2 + x1*w2 + y1*z2 - z1*y2,
        w1*y2 - x1*z2 + y1*w2 + z1*x2,
        w1*z2 + x1*y2 - y1*x2 + z1*w2
    ])


def quaternion_conjugate(q):
    """쿼터니언 켤레 (역회전)"""
    return np.array([q[0], -q[1], -q[2], -q[3]])


def rotate_vector_by_quaternion(v, q):
    """벡터 v를 쿼터니언 q로 회전"""
    v_quat = np.array([0, v[0], v[1], v[2]])
    q_conj = quaternion_conjugate(q)
    rotated = quaternion_multiply(quaternion_multiply(q, v_quat), q_conj)
    return rotated[1:] # w 제외
```


#### 상호 변환


**쿼터니언 → 회전 행렬**:


```python
def quaternion_to_rotation_matrix(q):
    """쿼터니언 → 3x3 회전 행렬"""
    w, x, y, z = q
    

    return np.array([
        [1-2*y*y-2*z*z, 2*x*y-2*w*z, 2*x*z+2*w*y],
        [2*x*y+2*w*z, 1-2*x*x-2*z*z, 2*y*z-2*w*x],
        [2*x*z-2*w*y, 2*y*z+2*w*x, 1-2*x*x-2*y*y]
    ])
```


**회전 행렬 → 쿼터니언**:


```python
def rotation_matrix_to_quaternion(R):
    """3x3 회전 행렬 → 쿼터니언"""
    trace = np.trace(R)
    

    if trace > 0:
        s = 0.5 / np.sqrt(trace + 1.0)
        w = 0.25 / s
        x = (R[2,1] - R[1,2]) * s
        y = (R[0,2] - R[2,0]) * s
        z = (R[1,0] - R[0,1]) * s
    # ... (추가 경우 처리 필요)
    

    return np.array([w, x, y, z])
```


---


### 4. SLERP (구면 선형 보간)


두 회전 사이를 **부드럽게 보간**하는 방법:


```python
def slerp(q1, q2, t):
    """구면 선형 보간 (Spherical Linear Interpolation)"""
    dot = np.dot(q1, q2)
    

    # 가장 짧은 경로 선택
    if dot < 0:
        q2 = -q2
        dot = -dot
    

    if dot > 0.9995:
        # 거의 같으면 선형 보간
        result = q1 + t * (q2 - q1)
        return result / np.linalg.norm(result)
    

    theta = np.arccos(dot)
    sin_theta = np.sin(theta)
    

    s1 = np.sin((1-t) * theta) / sin_theta
    s2 = np.sin(t * theta) / sin_theta
    

    return s1 * q1 + s2 * q2
```


> [!TIP]
> SLERP는 일정한 각속도로 회전하므로 애니메이션, 궤적 보간에 적합합니다.


---


## SLAM에서의 활용


### 왜 실전 로봇/Perception 시스템은 쿼터니언을 사용하는가?


| 이유 | 설명 |
|------|------|
| **짐벌락 없음** | 어떤 방향에서도 안정적 |
| **정규화 용이** | 단위 제약 하나만 유지 |
| **연속 보간** | SLERP로 부드러운 궤적 |
| **IMU 적분** | 연속적인 회전 표현에 적합 |


### IMU 적분 시 쿼터니언 사용


```python
def integrate_angular_velocity(q, omega, dt):
    """각속도를 쿼터니언에 적분"""
    theta = np.linalg.norm(omega) * dt
    

    if theta < 1e-10:
        return q
    

    axis = omega / np.linalg.norm(omega)
    dq = np.array([
        np.cos(theta/2),
        np.sin(theta/2) * axis[0],
        np.sin(theta/2) * axis[1],
        np.sin(theta/2) * axis[2]
    ])
    

    q_new = quaternion_multiply(q, dq)
    return q_new / np.linalg.norm(q_new) # 정규화
```


### 최적화 시 쿼터니언의 단위 제약 처리


```python
# 방법 1: 정규화 (간단하지만 비최적)
q = q / np.linalg.norm(q)


# 방법 2: Lie 대수 사용 (Week 6에서 학습)
# - 접선 공간에서 3개 파라미터로 표현
# - 제약 없이 최적화 가능
```


---


## 회전 표현 비교


| 특성 | 회전 행렬 | 오일러 각 | 쿼터니언 |
|------|----------|----------|----------|
| **파라미터 수** | 9 | 3 | 4 |
| **자유도** | 3 | 3 | 3 |
| **제약조건** | 6 (직교+det=1) | 0 | 1 (단위) |
| **짐벌락** | 없음 | **있음** | 없음 |
| **곱셈 비용** | 27 곱셈 | 변환 필요 | 16 곱셈 |
| **보간** | 복잡 | 비선형 | SLERP |
| **직관성** | 중간 | 높음 | 낮음 |


---


## 실습 파일


이 폴더에 포함된 실습 파일:


| 파일 | 내용 |
|------|------|
| `rotation_basics.py` | 회전 표현 구현 및 상호 변환 실습 |
| `rotation_quiz.py` | 주관식 퀴즈 (문제/답안 분리) |


### 실행 방법


```bash
cd "Studies/Phase 1/week4"
python3 rotation_basics.py
python3 rotation_quiz.py
```


---


## 추천 영상


| 영상 | 설명 |
|------|------|
| [3Blue1Brown - Quaternions](https://youtu.be/zjMuIxRvygQ) | 쿼터니언의 기하학적 이해 |
| [Visualizing Quaternions](https://eater.net/quaternions) | 인터랙티브 시각화 |
| [Gimbal Lock Explained](https://www.youtube.com/watch?v=zc8b2Jo7mno) | 짐벌락 시각적 설명 |


---


## 학습 완료 체크리스트


- [ ] 2D/3D 회전 행렬을 직접 구성할 수 있다
- [ ] 회전 행렬이 직교 행렬인 이유를 설명할 수 있다
- [ ] 오일러 각의 순서에 따라 결과가 달라짐을 이해한다
- [ ] 짐벌락이 언제, 왜 발생하는지 설명할 수 있다
- [ ] 쿼터니언으로 벡터를 회전시키는 공식을 안다
- [ ] 쿼터니언 <-> 회전 행렬 변환 코드를 작성할 수 있다
- [ ] SLERP의 용도와 필요성을 이해한다
- [ ] 실전 로봇/Perception 시스템이 쿼터니언을 사용하는 이유를 설명할 수 있다


---


## 다음 단계


Week 4 완료 후 → **Week 5: 강체 변환**(SE(3))으로 이동
- 회전 + 평행이동 결합
- 동차 좌표와 4x4 변환 행렬
- ROS TF2와의 연결
