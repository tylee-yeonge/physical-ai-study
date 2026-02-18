# Week 8: Bundle Adjustment (Ceres) - 자동 미분으로 더 쉽게

> 🎯 **이번 주 목표**: Ceres Solver의 자동 미분 개념 이해하기
> ⏰ **예상 시간**: 3-5시간
> 💡 **핵심 질문**: "g2o vs Ceres, 어떤 것을 쓸까?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | Ceres 자동 미분, AutoDiffCostFunction 개념 확인 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | Jet 타입, DENSE_SCHUR, g2o vs Ceres 비교 분석 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | Ceres로 BA 구현 및 HuberLoss 적용 |

---

## 🌟 시작하기 전에

### Week 7에서 배운 것

**g2o로 BA 구현:**
```cpp
// Vertex와 Edge를 직접 정의
g2o::VertexSE3Expmap* v_cam = new g2o::VertexSE3Expmap();
g2o::EdgeProjectXYZ2UV* edge = new g2o::EdgeProjectXYZ2UV();

// 수동으로 Jacobian 계산 필요 (복잡!)
```

**문제점:**
- Jacobian을 직접 계산해야 함 (미분!)
- 새로운 오차 함수마다 코드 작성
- 실수하기 쉬움

**Ceres의 해결책:**
```cpp
// 오차 함수만 정의하면 끝!
struct ReprojectionError {
    double operator()(const double* camera, const double* point) {
        // 재투영 오차만 계산
        return observed - projected;
    }
};

// Ceres가 자동으로 미분! (Auto-differentiation)
```

---

## 📚 핵심 개념

### 1. Ceres Solver란?

**정의:**
```
Ceres Solver = 
  Google이 만든 비선형 최적화 라이브러리
  
특징:
- 자동 미분 (Auto-differentiation)
- 간단한 API
- 빠른 성능
- Apache 2.0 라이선스
```

**왜 인기 있나요?**
```
✅ 사용하기 쉬움
   - Jacobian을 직접 계산하지 않아도 됨
   - 오차 함수만 정의하면 OK
   
✅ 유연함
   - 다양한 최적화 문제에 사용 가능
   - Robust loss function 쉽게 추가
   
✅ 빠름
   - 효율적인 solver
   - Sparse matrix 자동 처리
```

---

### 2. Ceres의 3가지 핵심 개념

#### A. Cost Function (비용 함수)

```cpp
struct ReprojectionError {
    // 생성자: 관측값 저장
    ReprojectionError(double observed_x, double observed_y, double fx, double fy, double cx, double cy)
        : observed_x_(observed_x), observed_y_(observed_y),
          fx_(fx), fy_(fy), cx_(cx), cy_(cy) {}
    
    // operator(): 오차 계산
    template <typename T>
    bool operator()(const T* const camera,  // 카메라 포즈 [6]
                    const T* const point,   // 3D 점 [3]
                    T* residuals) const {   // 출력: 오차 [2]
        
        // 1. 3D 점을 카메라 좌표계로 변환
        T p[3];
        // camera[0,1,2] = 회전 (angle-axis)
        // camera[3,4,5] = 평행이동
        ceres::AngleAxisRotatePoint(camera, point, p);
        p[0] += camera[3];
        p[1] += camera[4];
        p[2] += camera[5];
        
        // 2. 투영
        T predicted_x = fx_ * p[0] / p[2] + cx_;
        T predicted_y = fy_ * p[1] / p[2] + cy_;
        
        // 3. 오차
        residuals[0] = predicted_x - T(observed_x_);
        residuals[1] = predicted_y - T(observed_y_);
        
        return true;
    }
    
private:
    double observed_x_, observed_y_;
    double fx_, fy_, cx_, cy_;
};
```

**핵심:**
- `operator()`만 정의하면 됨
- Template으로 자동 미분 지원
- `T` 타입은 Ceres가 자동으로 처리

#### B. Problem (최적화 문제)

```cpp
ceres::Problem problem;

// 카메라 포즈 파라미터 (6차원)
double camera[6] = {0, 0, 0, 0, 0, 0};  // [rx, ry, rz, tx, ty, tz]

// 3D 점 (3차원)
double point[3] = {1.0, 2.0, 5.0};

// Cost function 추가
ceres::CostFunction* cost_function =
    new ceres::AutoDiffCostFunction<ReprojectionError, 2, 6, 3>(
        new ReprojectionError(observed_x, observed_y, fx, fy, cx, cy)
    );
    // AutoDiffCostFunction<오차함수, 출력차원, 입력1차원, 입력2차원>

problem.AddResidualBlock(cost_function, nullptr, camera, point);
//                                       ^^^^^^^ Loss function (옵션)
```

**파라미터 설명:**
- `2`: 출력 차원 (재투영 오차 x, y)
- `6`: 첫 번째 입력 차원 (카메라 포즈)
- `3`: 두 번째 입력 차원 (3D 점)

#### C. Solver (최적화 실행)

```cpp
ceres::Solver::Options options;
options.linear_solver_type = ceres::DENSE_SCHUR;
options.minimizer_progress_to_stdout = true;

ceres::Solver::Summary summary;
ceres::Solve(options, &problem, &summary);

std::cout << summary.BriefReport() << std::endl;
```

---

### 3. 자동 미분 (Auto-differentiation)

**수동 미분 (g2o):**
```cpp
// Jacobian을 직접 계산
∂error/∂camera = ... (복잡한 수식)
∂error/∂point = ... (복잡한 수식)

// 실수하기 쉬움! 😰
```

**자동 미분 (Ceres):**
```cpp
template <typename T>
bool operator()(const T* camera, const T* point, T* residuals) {
    // 그냥 오차만 계산
    residuals[0] = predicted - observed;
    
    // Ceres가 자동으로 미분! 😎
}
```

**비밀:**
```
T = ceres::Jet<double, N>

Jet은 값 + 미분을 동시에 계산:
  - x.a = 값
  - x.v = 미분

연산자 오버로딩으로 chain rule 자동 적용!
```

---

### 4. Robust Loss Functions

**문제: Outlier**
```
재투영 오차가 100픽셀!
→ 일반 최소제곱: error² = 10,000
→ 전체 최적화를 망침! 💥
```

**해결: Robust Loss**
```cpp
// Huber Loss
ceres::LossFunction* loss_function = new ceres::HuberLoss(1.0);
problem.AddResidualBlock(cost_function, loss_function, camera, point);
//                                       ^^^^^^^^^^^^^

ρ(e) = { e²/2        if |e| ≤ δ
       { δ(|e|-δ/2)  if |e| > δ

작은 오차: 제곱
큰 오차: 선형 → outlier 영향 감소
```

**다른 옵션들:**
```cpp
new ceres::CauchyLoss(1.0);    // ρ(e) = log(1 + e²)
new ceres::TukeyLoss(4.685);   // 극단적인 outlier 무시
nullptr;                        // Loss function 없음 (일반 최소제곱)
```

---

### 5. Local Parameterization

**문제:**
```
카메라 포즈 = SO(3) × R³ (리 군)
→ 6차원이지만 제약 조건 있음
→ 일반 벡터처럼 더하면 안 됨!
```

**해결:**
```cpp
// Quaternion으로 회전 표현 시
ceres::LocalParameterization* quaternion_local_param =
    new ceres::EigenQuaternionParameterization();

problem.SetParameterization(camera_quaternion, quaternion_local_param);
```

**Angle-Axis는?**
```
Ceres가 자동으로 처리!
AngleAxisRotatePoint() 사용 시 걱정 없음
```

---

## 💡 g2o vs Ceres 비교

| 특징 | g2o | Ceres |
|------|-----|-------|
| **사용 난이도** | 어려움 (Jacobian 필요) | 쉬움 (자동 미분) |
| **유연성** | SLAM 특화 | 범용 최적화 |
| **성능** | 매우 빠름 | 빠름 |
| **코드량** | 많음 | 적음 |
| **커뮤니티** | SLAM 중심 | 다양한 분야 |
| **라이선스** | BSD | Apache 2.0 |

**언제 뭘 쓸까?**

**Ceres 추천:**
```
✅ 빠르게 프로토타입 제작
✅ 새로운 오차 함수 실험
✅ SLAM 외 최적화 문제
✅ 코드 간결성 중요
```

**g2o 추천:**
```
✅ 최고 성능이 중요
✅ 표준 SLAM 문제 (Pose graph 등)
✅ 대규모 최적화 (10,000+ 변수)
✅ 기존 g2o 코드베이스
```

**실무:**
```
SLAM 프로젝트 = g2o + Ceres 혼용
- Pose graph optimization: g2o
- 새로운 센서 fusion: Ceres
- Bundle Adjustment: 둘 다 OK
```

---

## 🔧 Ceres 설치

```bash
# Dependencies
sudo apt install libgoogle-glog-dev libgflags-dev
sudo apt install libatlas-base-dev
sudo apt install libeigen3-dev

# Ceres
git clone https://ceres-solver.googlesource.com/ceres-solver
cd ceres-solver
mkdir build && cd build
cmake ..
make -j4
sudo make install

# 확인
pkg-config --modversion ceres
```

---

## 📝 이번 주 실습

### 구현 항목

1. **간단한 BA (Ceres)**
   - ReprojectionError 정의
   - AutoDiffCostFunction 사용
   - 최적화 실행

2. **Robust BA**
   - HuberLoss 추가
   - Outlier 테스트

3. **g2o vs Ceres 성능 비교**
   - 같은 문제를 양쪽으로 구현
   - 시간/정확도 측정

4. **실제 데이터**
   - Week 5 VO에 통합 (선택)

### 예상 결과

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Bundle Adjustment with Ceres
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Initial cost: 245.67
Iteration 0: cost = 245.67
Iteration 1: cost = 87.34
Iteration 2: cost = 23.45
Iteration 3: cost = 5.67
Iteration 4: cost = 1.23
Converged!

Final cost: 1.23
Cameras optimized: 3
Points optimized: 5

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ BA 성공! 오차 99% 감소!
```

### 체크리스트

- [ ] Ceres 설치 및 빌드
- [ ] ReprojectionError 구현
- [ ] AutoDiffCostFunction 사용
- [ ] HuberLoss 적용
- [ ] g2o vs Ceres 비교
- [ ] Week 5 VO에 통합 (선택)

---

## 🎯 핵심 요약

1. **Ceres Solver**
   - 자동 미분으로 쉬운 구현
   - 오차 함수만 정의하면 OK
   - Google 제공, 범용 최적화

2. **3가지 구성요소**
   - Cost Function: 오차 계산
   - Problem: 변수와 제약 조건
   - Solver: 최적화 실행

3. **자동 미분**
   - Template + Jet 타입
   - Jacobian 자동 계산
   - 실수 없음, 빠른 개발

4. **Robust Loss**
   - HuberLoss, CauchyLoss
   - Outlier 영향 감소
   - 실제 데이터에 필수

5. **g2o vs Ceres**
   - Ceres: 개발 속도 ↑
   - g2o: 실행 속도 ↑
   - 목적에 따라 선택

6. **다음 단계**
   - Week 9: BA 최적화 기법 (Schur Complement)
   - Phase 4: VIO 개념

---

**다음**: Week 9 - BA 최적화 기법

다음: [Week 9 - BA 최적화 기법](../week9/README.md)
