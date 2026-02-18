# Week 11: Ceres 실습 - Ceres BA 예제 분석

> 🎯 **이번 주 목표**: Ceres 예제로 BA 실습하기
> ⏰ **예상 시간**: 5시간
> 💡 **핵심 질문**: "Ceres의 BAL 예제는 어떻게 구성되고, 실제 데이터에서 수렴은 어떻게 일어날까?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | Ceres vs g2o 차이, AutoDiff 템플릿 파라미터 확인 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | 잔차 차원 분석, 자코비안 크기 계산 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | BAL 데이터셋으로 Ceres BA 실행 및 수렴 관찰 |

---

## 🌟 시작하기 전에

### Week 8에서 배운 것

**Ceres의 3가지 핵심:**
```cpp
// 1. Cost Function: 오차만 정의
struct ReprojectionError {
    template <typename T>
    bool operator()(const T* camera, const T* point, T* residuals) const {
        // 재투영 오차 계산
    }
};

// 2. Problem: 변수와 제약 추가
ceres::Problem problem;
problem.AddResidualBlock(cost_function, loss_function, camera, point);

// 3. Solver: 최적화 실행
ceres::Solve(options, &problem, &summary);
```

### Week 9-10에서 배운 것

**BA 최적화 원리:**
```
H·Δx = b
→ Schur Complement로 점 소거
→ S·Δc = b' (작은 시스템)
→ DENSE_SCHUR 또는 SPARSE_SCHUR
```

### 이번 주의 목표

```
❓ Ceres의 BAL 예제(bal_problem.cc)는 어떻게 구성되어 있을까?
❓ BAL 데이터셋이란 무엇이고 어떤 형식일까?
❓ 실제 대규모 데이터에서 수렴 과정은 어떻게 진행될까?
❓ Iteration마다 cost가 어떻게 줄어들까?
```

**직접 돌려보고 관찰합니다!**

---

## 📚 핵심 개념

### 1. Ceres BA 개념 복습

#### 1.1 BA란?

```
Bundle Adjustment = 다발 조정

목표: 재투영 오차 최소화
  min Σᵢⱼ ||uᵢⱼ - π(Tᵢ, Xⱼ)||²

여기서:
- uᵢⱼ: 카메라 i에서 점 j의 관측 (픽셀)
- Tᵢ: 카메라 i의 파라미터
- Xⱼ: 3D 점 j의 위치
- π: 투영 함수
```

#### 1.2 Ceres에서 BA 구현 흐름

```
┌──────────────────┐
│ 1. 데이터 로드     │  BAL 파일 읽기
├──────────────────┤
│ 2. Cost Function  │  SnavelyReprojectionError 정의
├──────────────────┤
│ 3. Problem 구성   │  AddResidualBlock 반복
├──────────────────┤
│ 4. Solver 옵션    │  DENSE_SCHUR / SPARSE_SCHUR
├──────────────────┤
│ 5. 최적화 실행     │  ceres::Solve()
├──────────────────┤
│ 6. 결과 분석      │  Summary 출력, 수렴 확인
└──────────────────┘
```

---

### 2. Cost Function 상세

#### 2.1 Snavely 카메라 모델

**BAL에서 사용하는 카메라 모델:**
```
카메라 파라미터 = 9차원
  [0-2]: angle-axis 회전 (3)
  [3-5]: 평행이동 (3)
  [6]:   focal length (1)
  [7-8]: radial distortion k1, k2 (2)
```

**투영 과정:**
```
1. 3D 점 → 카메라 좌표계:
   P' = R * P + t

2. 정규화 좌표:
   p = -P'/P'z    (z축 반전, BAL 관례)

3. 방사 왜곡:
   r² = px² + py²
   distortion = 1 + k1*r² + k2*r⁴
   p_distorted = distortion * p

4. 픽셀 좌표:
   u = f * p_distorted_x
   v = f * p_distorted_y
```

#### 2.2 Cost Function 코드

```cpp
struct SnavelyReprojectionError {
    SnavelyReprojectionError(double observed_x, double observed_y)
        : observed_x_(observed_x), observed_y_(observed_y) {}

    template <typename T>
    bool operator()(const T* const camera,
                    const T* const point,
                    T* residuals) const {
        // 1. angle-axis 회전
        T p[3];
        ceres::AngleAxisRotatePoint(camera, point, p);

        // 2. 평행이동
        p[0] += camera[3];
        p[1] += camera[4];
        p[2] += camera[5];

        // 3. 정규화 (BAL: -z 방향)
        T xp = -p[0] / p[2];
        T yp = -p[1] / p[2];

        // 4. 방사 왜곡
        const T& l1 = camera[7];
        const T& l2 = camera[8];
        T r2 = xp * xp + yp * yp;
        T distortion = T(1.0) + r2 * (l1 + l2 * r2);

        // 5. focal length 적용
        const T& focal = camera[6];
        T predicted_x = focal * distortion * xp;
        T predicted_y = focal * distortion * yp;

        // 6. 잔차
        residuals[0] = predicted_x - T(observed_x_);
        residuals[1] = predicted_y - T(observed_y_);

        return true;
    }

    static ceres::CostFunction* Create(double observed_x, double observed_y) {
        return new ceres::AutoDiffCostFunction<SnavelyReprojectionError, 2, 9, 3>(
            new SnavelyReprojectionError(observed_x, observed_y)
        );
        // AutoDiffCostFunction<Functor, 잔차차원, 카메라차원, 점차원>
        //                               2        9           3
    }

private:
    double observed_x_;
    double observed_y_;
};
```

**핵심 포인트:**
```
- 잔차 차원: 2 (x, y 재투영 오차)
- 카메라 파라미터: 9 (Week 8의 6보다 큼!)
  → focal length + radial distortion 포함
- 3D 점: 3 (x, y, z)
- Template <typename T>로 자동 미분 지원
```

---

### 3. Auto-diff 원리

#### 3.1 수동 미분 vs 자동 미분

**수동 미분 (g2o 방식):**
```
∂error/∂camera = ?  (9×2 자코비안)
∂error/∂point = ?   (3×2 자코비안)

→ 투영 함수를 직접 미분해야 함
→ 방사 왜곡까지 포함하면 매우 복잡!
→ 실수하기 쉬움
```

**자동 미분 (Ceres 방식):**
```cpp
template <typename T>
bool operator()(const T* camera, const T* point, T* residuals) {
    // 그냥 순방향 계산만 하면 됨!
    // T = ceres::Jet<double, N> 타입이 자동으로 미분 계산
}
```

#### 3.2 Jet 타입의 비밀

```
Jet<double, N> = (값, 미분 벡터)

예: x = Jet(3.0, [1, 0])  (값=3, ∂x/∂x=1, ∂x/∂y=0)
    y = Jet(4.0, [0, 1])  (값=4, ∂y/∂x=0, ∂y/∂y=1)

연산자 오버로딩:
  x + y = Jet(7.0, [1, 1])
  x * y = Jet(12.0, [4, 3])  ← 곱의 미분 법칙!
  sin(x) = Jet(sin(3), [cos(3), 0])

→ Chain rule이 자동 적용!
→ 정확한 미분값 (수치 미분이 아님!)
```

#### 3.3 성능 비교

```
방법             정확도    구현 난이도   실행 속도
────────────────────────────────────────────────
수동 미분        정확      어려움       빠름
자동 미분(Ceres) 정확      쉬움        약간 느림
수치 미분        근사      쉬움        느림

결론:
→ 프로토타입: 자동 미분 (Ceres)
→ 프로덕션 (속도 중요): 수동 미분 (g2o)
→ 수치 미분: 디버깅용으로만 사용
```

---

### 4. BAL 데이터셋 소개

#### 4.1 BAL (Bundle Adjustment in the Large)

```
BAL = Washington 대학의 대규모 BA 벤치마크 데이터셋

특징:
- Structure from Motion에서 추출한 실제 데이터
- 다양한 규모 (작은~매우 큰 문제)
- Ceres 공식 예제에서 사용
- 무료 다운로드 가능
```

**데이터셋 규모 예시:**

| 데이터셋 | 카메라 | 점 | 관측 |
|----------|--------|-----|------|
| problem-16-22106 | 16 | 22,106 | 83,718 |
| problem-49-7776 | 49 | 7,776 | 31,843 |
| problem-89-110973 | 89 | 110,973 | 394,707 |
| problem-257-65132 | 257 | 65,132 | 225,911 |
| problem-1778-993923 | 1,778 | 993,923 | 9,462,645 |

#### 4.2 BAL 파일 형식

```
파일 구조:
<카메라 수> <점 수> <관측 수>
<카메라ID> <점ID> <관측x> <관측y>   ← 관측 반복
...
<camera 0 파라미터 9개>              ← 카메라 파라미터
<camera 1 파라미터 9개>
...
<point 0 좌표 3개>                   ← 3D 점
<point 1 좌표 3개>
...
```

**구체적 예시:**
```
16 22106 83718           ← 16카메라, 22106점, 83718관측
0 0 -3.859330e+02 3.871820e+02    ← 카메라0이 점0을 관측
0 1 -3.844740e+02 3.849960e+02
...
1.5741515942e-02         ← 카메라0 angle-axis[0]
-3.5572723260e-03        ← 카메라0 angle-axis[1]
-2.9438782339e-03        ← 카메라0 angle-axis[2]
-3.2877937571e-01        ← 카메라0 tx
1.2570207636e-01         ← 카메라0 ty
1.2775362691e+00         ← 카메라0 tz
3.9793378201e+02         ← 카메라0 focal
-2.3753635995e-07        ← 카메라0 k1
3.4261246498e-13         ← 카메라0 k2
...
```

#### 4.3 다운로드

```bash
# BAL 데이터셋 다운로드 (작은 것부터 시작)
wget https://grail.cs.washington.edu/projects/bal/data/ladybug/problem-49-7776-pre.txt.bz2
bunzip2 problem-49-7776-pre.txt.bz2

# 또는 Ceres 소스에 포함된 테스트 데이터 사용
# ceres-solver/data/
```

---

### 5. g2o vs Ceres 비교 정리

#### 5.1 전체 비교표

| 항목 | g2o | Ceres |
|------|-----|-------|
| **개발사** | TU Freiburg | Google |
| **미분 방법** | 수동 자코비안 | 자동 미분 (Auto-diff) |
| **API 스타일** | 그래프 (Vertex + Edge) | 함수형 (CostFunction + Problem) |
| **BA 설정** | BlockSolver_6_3 + setMarginalized | DENSE_SCHUR / SPARSE_SCHUR |
| **카메라 표현** | SE3Expmap (내장) | 직접 정의 (유연) |
| **Robust Loss** | setRobustKernel() | LossFunction |
| **코드량** | 많음 | 적음 |
| **학습 곡선** | 가파름 | 완만함 |
| **성능** | 약간 빠름 | 약간 느림 |
| **사용 사례** | ORB-SLAM, LSD-SLAM | VINS-Mono, Cartographer |

#### 5.2 같은 문제, 다른 코드

**g2o로 BA 설정:**
```cpp
// 1. Solver 설정 (3계층)
auto linearSolver = g2o::make_unique<
    g2o::LinearSolverEigen<g2o::BlockSolver_6_3::PoseMatrixType>>();
auto blockSolver = g2o::make_unique<g2o::BlockSolver_6_3>(
    std::move(linearSolver));
auto algorithm = new g2o::OptimizationAlgorithmLevenberg(
    std::move(blockSolver));

g2o::SparseOptimizer optimizer;
optimizer.setAlgorithm(algorithm);

// 2. Vertex 추가
g2o::VertexSE3Expmap* v_cam = new g2o::VertexSE3Expmap();
v_cam->setEstimate(pose);
optimizer.addVertex(v_cam);

g2o::VertexPointXYZ* v_pt = new g2o::VertexPointXYZ();
v_pt->setMarginalized(true);  // Schur!
optimizer.addVertex(v_pt);

// 3. Edge + 자코비안 (내부 또는 직접)
g2o::EdgeProjectXYZ2UV* edge = new g2o::EdgeProjectXYZ2UV();
edge->setRobustKernel(new g2o::RobustKernelHuber);

// 4. 최적화
optimizer.initializeOptimization();
optimizer.optimize(10);
```

**Ceres로 BA 설정:**
```cpp
// 1. Problem 생성
ceres::Problem problem;

// 2. Residual Block 추가 (Cost Function + Loss)
for (auto& obs : observations) {
    ceres::CostFunction* cost =
        new ceres::AutoDiffCostFunction<ReprojError, 2, 9, 3>(
            new ReprojError(obs.x, obs.y));

    problem.AddResidualBlock(cost,
        new ceres::HuberLoss(1.0),  // Robust Loss
        cameras[obs.cam_id],
        points[obs.pt_id]);
}

// 3. Solver 옵션
ceres::Solver::Options options;
options.linear_solver_type = ceres::DENSE_SCHUR;
options.minimizer_progress_to_stdout = true;

// 4. 최적화
ceres::Solver::Summary summary;
ceres::Solve(options, &problem, &summary);
```

**비교 요약:**
```
g2o:   Vertex + Edge + Jacobian + 3계층 Solver 설정
Ceres: CostFunction + Problem + Options

코드량: g2o > Ceres
유연성: Ceres > g2o (새로운 오차 함수 추가 쉬움)
성능:   g2o ≈ Ceres (BA 규모에 따라 다름)
```

---

## 🔧 실습 안내

### 실습 구성

| Step | 내용 | 예상 시간 |
|------|------|----------|
| 1 | BAL 데이터셋 다운로드 및 형식 이해 | 30분 |
| 2 | Ceres BAL 예제 코드 분석 | 1.5시간 |
| 3 | 직접 Ceres BA 코드 작성 및 실행 | 2시간 |
| 4 | 수렴 과정 관찰 (Iteration, Cost 변화) | 1시간 |

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

---

## ✅ 체크리스트

### 개념 이해
- [ ] SnavelyReprojectionError의 9개 카메라 파라미터 설명 가능
- [ ] AutoDiffCostFunction<Functor, 2, 9, 3>의 숫자 의미 설명 가능
- [ ] BAL 파일 형식 읽을 수 있음
- [ ] DENSE_SCHUR vs SPARSE_SCHUR 차이 설명 가능
- [ ] HuberLoss가 왜 필요한지 설명 가능

### 코드 이해
- [ ] SnavelyReprojectionError의 투영 과정 이해
- [ ] Ceres Problem에 ResidualBlock 추가하는 과정 이해
- [ ] Solver::Summary에서 수렴 정보 읽기

### 실습 완료
- [ ] BAL 데이터셋 다운로드 완료
- [ ] Ceres BA 코드 빌드 및 실행
- [ ] Iteration별 cost 변화 관찰
- [ ] HuberLoss 유무에 따른 결과 비교

---

## 🎯 핵심 요약

### 1. Ceres BA 구조

```
SnavelyReprojectionError (functor)
  → 카메라 9파라미터 + 3D 점 3파라미터
  → 잔차 2차원 (재투영 오차 x, y)
↓
AutoDiffCostFunction<..., 2, 9, 3> (자동 미분)
↓
Problem + AddResidualBlock (문제 구성)
↓
Solver (DENSE_SCHUR로 최적화)
```

### 2. BAL 데이터셋

```
실제 SfM 데이터:
- 수십~수천 카메라
- 수천~수십만 3D 점
- 수만~수백만 관측
→ Ceres로 실제 대규모 BA 테스트 가능!
```

### 3. 수렴 관찰

```
전형적인 수렴 패턴:
Iter 0:  cost = 1.234e+07  (초기)
Iter 1:  cost = 5.678e+05  (급감)
Iter 2:  cost = 1.234e+04  (감소)
Iter 3:  cost = 8.901e+03  (수렴 중)
...
Iter 10: cost = 8.456e+03  (수렴!)
```

### 4. g2o vs Ceres

```
g2o:   그래프 구조, 수동 자코비안, SLAM 특화
Ceres: 함수형, 자동 미분, 범용 최적화
→ 둘 다 Schur Complement 지원!
→ 프로젝트에 따라 선택
```

### 5. 다음 단계

```
Week 12: Monocular 스케일 모호성
→ Monocular VO에서 왜 스케일을 알 수 없는지
→ IMU / Stereo로 어떻게 해결하는지
```

---

**다음**: Week 12 - Monocular 스케일 모호성

다음: [Week 12 - Monocular 스케일 모호성](../week12/README.md)
