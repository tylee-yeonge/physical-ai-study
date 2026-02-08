# Phase 1: 수학 핵심

> ⏰ **기간**: 2개월  
> 🎯 **목표**: SLAM에 필요한 수학만, 깊이보다 직관 우선  
> ⏱️ **주간 시간**: 약 7시간

---

## 📋 Section 1.1: 선형대수 기초 (3주)

### Week 1: 3Blue1Brown으로 직관 얻기

#### Essence of Linear Algebra 시청
- [x] Chapter 1: Vectors (벡터란 무엇인가)
- [x] Chapter 2: Linear combinations, span, basis
- [x] Chapter 3: Linear transformations and matrices
- [x] Chapter 4: Matrix multiplication as composition
- [x] Chapter 5: Three-dimensional linear transformations
- [x] Chapter 6: The determinant
- [x] Chapter 7: Inverse matrices, column space and null space
- [x] Chapter 8: Nonsquare matrices
- [x] Chapter 9: Dot products and duality
- [x] Chapter 10: Cross products
- [x] Chapter 11: Cross products in the light of linear transformations
- [x] Chapter 12: Cramer's rule
- [x] Chapter 13: Change of basis
- [x] Chapter 14: Eigenvectors and eigenvalues
- [x] Chapter 15: Abstract vector spaces

#### 핵심 개념 정리
- [x] "행렬 = 선형 변환" 이해했는지 자체 점검
- [x] 고유값/고유벡터가 왜 중요한지 정리
- [x] 내적의 기하학적 의미 정리

### Week 2: 실습으로 확인

#### NumPy/Eigen 기본 연산
- [x] 행렬 곱셈 직접 계산 vs 라이브러리 비교
- [x] 역행렬 계산
- [x] 고유값 분해 실습
- [x] 행렬식 계산

#### SLAM에서 어디에 쓰이나?
- [x] 회전 행렬이 직교 행렬인 이유 이해
- [x] 왜 회전 행렬의 행렬식 = 1인지 확인
- [x] 공분산 행렬과 불확실성 표현 (칼만 필터 예고)

### Week 3: SVD 집중

#### SVD 이해
- [x] SVD의 기하학적 의미 (회전-스케일-회전)
- [x] 특이값의 의미
- [x] SVD를 이용한 최소자승 해 구하기

#### SVD 실습
- [x] NumPy/Eigen으로 SVD 분해
- [x] 이미지 압축에 SVD 적용 (선택)

#### SLAM에서 어디에 쓰이나?
- [x] Essential Matrix에서 R, t 추출할 때 SVD 사용
- [x] Homography 분해에서 SVD 사용
- [x] PnP 문제의 해법에서 SVD 사용

### 🔍 Section 1.1 자체 점검
> 아래 질문에 답할 수 있으면 다음으로 진행

1. 행렬 A가 벡터 v를 어떻게 "변환"하는지 기하학적으로 설명할 수 있는가?
2. 고유벡터는 변환 후에도 방향이 왜 유지되는가?
3. SVD에서 U, Σ, V 각각이 의미하는 것은?

---

## 📋 Section 1.2: 3D 기하학 (3주)

### Week 4: 회전 표현

#### 회전 행렬 (Rotation Matrix)
- [x] 2D 회전 행렬 유도
- [x] 3D 회전 행렬 (Rx, Ry, Rz)
- [x] 회전 행렬의 성질 (직교, 행렬식=1)
- [x] 회전 순서에 따른 결과 차이 (비가환성)

#### 오일러 각 (Euler Angles)
- [x] Roll-Pitch-Yaw 정의
- [x] 짐벌락 문제 이해
- [x] 왜 오일러 각만으로는 부족한지

#### 쿼터니언 (Quaternion)
- [x] 쿼터니언 기본 정의 (w, x, y, z)
- [x] 쿼터니언 곱셈
- [x] 쿼터니언 → 회전 행렬 변환
- [x] 회전 행렬 → 쿼터니언 변환
- [x] SLERP (구면 선형 보간) 개념

#### SLAM에서 어디에 쓰이나?
- [x] VINS-Fusion은 쿼터니언으로 회전 표현
- [x] IMU 적분 시 쿼터니언 사용 (짐벌락 방지)
- [x] 최적화 시 쿼터니언의 단위 제약 처리

### Week 5: 강체 변환

#### SE(3) 이해
- [x] 회전 + 평행이동 결합
- [x] 4x4 동차 변환 행렬
- [x] 변환 행렬의 연산 (곱셈 = 변환 합성)
- [x] 역변환 계산

#### 동차 좌표 (Homogeneous Coordinates)
- [x] 왜 동차 좌표를 쓰는지 (평행이동을 곱셈으로)
- [x] 3D 점의 동차 표현 [x, y, z, 1]
- [x] 투영과 동차 좌표

#### ROS TF와 연결
- [x] TF2에서 사용하는 회전 표현 (쿼터니언)
- [x] geometry_msgs/Transform 메시지 구조 복습
- [x] 실제 AMR 좌표계 변환 예시 떠올려보기 (base_link → camera)

#### SLAM에서 어디에 쓰이나?
- [x] 카메라 포즈 = SE(3) 변환
- [x] 3D 점을 이미지에 투영할 때 변환 행렬 사용
- [x] 키프레임 간 상대 포즈 = SE(3)

### Week 6: Lie 군/대수 기초

> ⚠️ 처음에는 가볍게 훑고, VINS-Fusion 코드 (Phase 5)에서 필요할 때 돌아와서 심화

#### 왜 필요한가?
- [x] 회전 행렬은 9개 파라미터지만 자유도는 3 (over-parameterized)
- [x] 쿼터니언도 4개 파라미터, 단위 제약 필요
- [x] 최적화할 때 미분 가능한 표현이 필요

#### 기본 개념만
- [x] SO(3): 회전의 집합 (3자유도)
- [x] SE(3): 강체 변환의 집합 (6자유도)
- [x] Lie 대수: 접선 공간에서의 표현 (so(3), se(3))
- [x] exp/log 매핑 개념

### 🔍 Section 1.2 자체 점검
> 아래 질문에 답할 수 있으면 다음으로 진행

1. 쿼터니언이 오일러 각보다 나은 점은?
2. SE(3) 행렬에서 회전과 평행이동 부분은 어디인가?
3. 왜 Lie 대수를 쓰면 최적화가 더 쉬워지는가? (대략적으로)

---

## 📋 Section 1.3: 최적화 기초 (2주)

### Week 7: 최소자승법

#### 기본 개념
- [x] 선형 최소자승 문제 정의: min ||Ax - b||²
- [x] 정규방정식 (Normal Equation): AᵀAx = Aᵀb
- [x] 과결정 시스템 (Overdetermined System)

#### 비용 함수 이해
- [x] 비용 함수 (Cost Function) = 최소화할 목표
- [x] 잔차 (Residual): r = 측정값 - 예측값
- [x] L2 norm 최소화

#### 실습
- [x] 직선 피팅 문제: y = ax + b에서 a, b 찾기
- [x] NumPy `np.linalg.lstsq` 사용
- [x] Eigen `JacobiSVD`로 최소자승 해 구하기

#### SLAM에서 어디에 쓰이나?
- [x] PnP 문제 = 최소자승 문제
- [x] Bundle Adjustment = 대규모 최소자승 문제
- [x] 재투영 오차 최소화

### Week 8: 비선형 최적화

#### Gauss-Newton
- [x] 비선형 문제: f(x) ≈ f(x₀) + J·Δx 로 선형화
- [x] 자코비안 (Jacobian) 행렬: 편미분들의 행렬
- [x] 반복적 최적화: x ← x + Δx

#### Levenberg-Marquardt
- [x] Gauss-Newton의 한계 (초기값 멀면 발산)
- [x] LM 알고리즘: (JᵀJ + λI)Δx = -Jᵀr
- [x] λ가 크면 → Gradient Descent처럼 (안정적)
- [x] λ가 작으면 → Gauss-Newton처럼 (빠름)

#### SLAM에서 어디에 쓰이나?
- [x] VINS-Fusion은 Ceres를 사용해 최적화
- [x] Factor Graph의 각 factor = 비용 함수 항
- [x] BA에서 카메라 포즈 + 3D 점 동시 최적화

### 🔍 Section 1.3 자체 점검
> 아래 질문에 답할 수 있으면 다음으로 진행

1. 최소자승 문제에서 정규방정식은 어떻게 유도되는가?
2. Gauss-Newton이 수렴하지 않을 때 LM은 어떻게 해결하는가?
3. Ceres에서 `CostFunction`과 `Problem`의 역할은?

---

## ✅ Phase 1 완료 체크리스트

### 선형대수
- [x] "행렬은 선형 변환이다"를 직관적으로 설명 가능
- [x] SVD의 기하학적 의미 설명 가능
- [x] 고유값/고유벡터 개념 이해

### 3D 기하학
- [x] 쿼터니언과 회전 행렬 상호 변환 가능
- [x] SE(3) 변환 행렬 구조 이해
- [x] ROS TF2와 연결 가능

### 최적화
- [x] 최소자승 문제 정의하고 풀 수 있음
- [x] Gauss-Newton, LM 알고리즘 개념 이해

---

## 🎯 Phase 1 완료 기준

> "VINS-Fusion 코드에서 `Eigen::Matrix`, `Quaterniond`, `ceres::Problem`이 뭔지 안다"

---

## 📚 참고 자료

### 영상

| 자료 | 용도 |
|------|------|
| 3Blue1Brown Essence of Linear Algebra | 선형대수 직관 |
| 3Blue1Brown Essence of Calculus | 미분 복습 (필요시) |
| Cyrill Stachniss - Least Squares | 최적화 이론 |

### 책 (사전처럼 사용)

| 책 | 용도 |
|------|------|
| State Estimation for Robotics (Barfoot) | Appendix A, B |
| Introduction to Linear Algebra (Strang) | 선형대수 심화 |

### 라이브러리 문서

| 라이브러리 | 링크 |
|-----------|------|
| Eigen | https://eigen.tuxfamily.org/dox/ |
| Sophus | https://github.com/strasdat/Sophus |
| Ceres Solver | http://ceres-solver.org/tutorial.html |

---

## 💡 팁

1. **증명보다 직관**: 수학적 증명보다 "왜 이게 SLAM에 쓰이는가"에 집중
2. **작은 예제**: 2x2, 3x3 작은 행렬로 손으로 계산해보기
3. **시각화**: 가능하면 그래프나 그림으로 확인
4. **80% 이해하면 다음으로**: 완벽 추구하지 않기
5. **Lie 군은 나중에**: Week 6은 가볍게 보고, Phase 5에서 필요할 때 돌아오기

---

## ❓ 다음 단계

Phase 1 완료 후:
- Phase 2 (컴퓨터 비전 기초)로 진행
- 또는 Phase 0에서 정리한 "모르는 개념"이 수학 관련이면 해당 부분 심화