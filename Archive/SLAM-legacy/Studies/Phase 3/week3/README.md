# Week 3: g2o / Ceres 코드 분석

> [goal] **목표**: g2o와 Ceres의 구조를 이해하고, VINS가 Ceres를 사용하는 방식 파악
> [time] **예상 시간**: 7-10시간
> [tip] **핵심 질문**: "최적화 라이브러리는 BA를 어떤 구조로 풀어주는가?"

---

## [ref] 핵심 개념

### 1. g2o (General Graph Optimization)

SLAM 특화 그래프 최적화 라이브러리.

#### 핵심 구조

```
Graph
+-- Vertex (변수 노드)
|   +-- VertexSE3Expmap    # 카메라 포즈 (SE(3))
|   +-- VertexPointXYZ     # 3D 점
+-- Edge (오차 항)
    +-- EdgeProjectXYZ2UV  # 재투영 오차
        +-- connects: Vertex(포즈) <-> Vertex(3D점)
        +-- measurement: 관측된 2D 좌표
        +-- information: 정보 행렬 (공분산 역)
```

#### 사용 패턴

```cpp
// 1. Solver 설정
auto solver = new g2o::OptimizationAlgorithmLevenberg(...);
g2o::SparseOptimizer optimizer;
optimizer.setAlgorithm(solver);

// 2. Vertex 추가 (최적화 변수)
auto pose = new g2o::VertexSE3Expmap();
pose->setEstimate(initial_pose);
optimizer.addVertex(pose);

// 3. Edge 추가 (오차 항)
auto edge = new g2o::EdgeProjectXYZ2UV();
edge->setVertex(0, point);
edge->setVertex(1, pose);
edge->setMeasurement(observed_2d);
optimizer.addEdge(edge);

// 4. 최적화 실행
optimizer.optimize(20);  // 20 iterations
```

---

### 2. Ceres Solver

Google의 범용 비선형 최적화 라이브러리. VINS가 사용.

#### 핵심 구조

```
Problem
+-- ParameterBlock (변수)
|   +-- 카메라 포즈 (쿼터니언 + 평행이동, 7 params)
|   +-- 3D 점 (x, y, z, 3 params)
+-- CostFunction (비용 함수)
|   +-- 재투영 오차 계산
+-- LossFunction (로버스트 커널)
    +-- HuberLoss (outlier 대응)
```

#### 사용 패턴

```cpp
// 1. Problem 생성
ceres::Problem problem;

// 2. Cost function 정의 (자동 미분)
struct ReprojectionError {
    // operator()에서 residual 계산
    template <typename T>
    bool operator()(const T* camera, const T* point, T* residual) const {
        // 3D → 2D 투영 후 관측값과 비교
        residual[0] = predicted_x - observed_x;
        residual[1] = predicted_y - observed_y;
        return true;
    }
};

// 3. Residual block 추가
problem.AddResidualBlock(
    new ceres::AutoDiffCostFunction<ReprojectionError, 2, 7, 3>(
        new ReprojectionError(observed_x, observed_y)),
    new ceres::HuberLoss(1.0),
    camera_params, point_params);

// 4. 최적화 실행
ceres::Solver::Options options;
options.linear_solver_type = ceres::SPARSE_SCHUR;  // Schur complement!
ceres::Solve(options, &problem, &summary);
```

---

### 3. g2o vs Ceres

| 항목 | g2o | Ceres |
|------|-----|-------|
| 특화 | SLAM/그래프 | 범용 최적화 |
| 미분 | 수동 자코비안 | **자동 미분** 지원 |
| 인터페이스 | Vertex/Edge 그래프 | CostFunction/Problem |
| Schur | 내장 | `SPARSE_SCHUR` 옵션 |
| 사용 예 | ORB-SLAM | **VINS-Fusion** |

---

## [search] 자체 점검

1. g2o에서 Vertex와 Edge는 각각 무엇을 의미하는가?
2. Ceres의 automatic differentiation 장점은?
3. VINS `optimization.cpp`에서 Ceres가 어떻게 사용되는가?
4. `SPARSE_SCHUR` solver를 쓰는 이유는?

---

다음: [Week 4 - 스케일 문제 + VINS 코드 마무리](../week4/README.md)
