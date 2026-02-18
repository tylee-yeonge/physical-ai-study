# Week 7: Bundle Adjustment 개념 - 모든 것을 한번에 최적화

> 🎯 **이번 주 목표**: Bundle Adjustment가 무엇을 최적화하는지 이해하기
> ⏰ **예상 시간**: 5시간 (개념 중심)
> 💡 **핵심 질문**: "왜 하나씩 업데이트하지 않고 한번에 최적화할까?"

---

## 📋 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 이론 학습 | `README.md` | 아래 핵심 개념 읽기 |
| 2 | C++ 퀴즈 (초급) | `quiz_easy.cpp` | BA 목적함수, g2o Vertex/Edge 개념 확인 |
| 3 | C++ 퀴즈 (중급) | `quiz_medium.cpp` | Schur Complement, setMarginalized 동작 분석 |
| 4 | 실습 | [PRACTICE.md](./PRACTICE.md) | g2o로 간단한 BA 구현 및 Robust Kernel 적용 |

---

## 🌟 시작하기 전에

### Week 1-6의 문제점

**지금까지의 VO:**
```
Frame 1: 포즈 추정 (PnP)
Frame 2: 포즈 추정 (PnP)
Frame 3: 포즈 추정 (PnP)
...
Frame 100: 포즈 추정 (PnP)

결과: 오차가 누적됨! 💥
```

**비유:**
```
상황: 100개 퍼즐 조각 맞추기

나쁜 방법: 하나씩 순서대로
  조각1 + 조각2 (약간 틀림)
  + 조각3 (더 틀림)
  + 조각4 (완전 틀림)
  → 나중에는 거의 안 맞음!

좋은 방법: 전체를 보고 동시에
  모든 조각의 위치를 보면서
  전체가 잘 맞도록 조정
  → Bundle Adjustment!
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. Bundle Adjustment가 뭐죠?

**정의:**
```
Bundle Adjustment (BA) = 
  여러 카메라 포즈 + 3D 점들을
  재투영 오차를 최소화하도록
  "동시에" 최적화

"Bundle" = 광선 다발(ray bundle)
```

**시각화:**
```
카메라 1 ─┐
카메라 2 ─┼─→ 3D 점 A
카메라 3 ─┘

재투영 오차 = Σ ||observed - projected||²

BA는 모든 카메라 포즈와 3D 점을
동시에 조정하여 이 오차를 최소화!
```

**왜 필요한가요?**
```
✅ 오차 누적 방지
   - PnP는 이전 오차에 영향받음
   - BA는 전역 일관성 유지
   
✅ 정확도 향상
   - 모든 관측을 동시에 고려
   - 최적의 전역 해
   
✅ SLAM의 핵심
   - Loop Closure 후 필수
   - 전체 맵 일관성
```

---

### 2. 수학적 정의

#### 목적 함수

```
min Σᵢⱼ ρ(||uᵢⱼ - π(Tᵢ, Xⱼ)||²)

여기서:
- uᵢⱼ: 카메라 i에서 점 j의 관측 (픽셀)
- π: 투영 함수
- Tᵢ: 카메라 i의 포즈 (최적화 변수)
- Xⱼ: 3D 점 j의 위치 (최적화 변수)
- ρ: Robust kernel (Huber 등)
```

#### 문제 구조

```
최적화 변수:
  - N개 카메라 포즈: T₁, T₂, ..., Tₙ
  - M개 3D 점: X₁, X₂, ..., Xₘ
  
  → 총 6N + 3M 차원!

관측:
  - K개 재투영: (카메라 i, 점 j) 쌍

목표:
  minimize Σₖ error_k²
```

---

### 3. g2o 프레임워크

**g2o = General Graph Optimization**

#### 그래프 구조

```
Vertex (정점):
  - 카메라 포즈 (SE3)
  - 3D 점 (XYZ)

Edge (변):
  - 재투영 제약
  - 카메라 ←→ 점

Example:
  Vertex: Pose₁ ─┬─ Edge ─→ Vertex: Point_A
  Vertex: Pose₂ ─┤
  Vertex: Pose₃ ─┘
```

#### g2o 주요 구성요소

**1. Vertex (최적화 변수)**
```cpp
g2o::VertexSE3Expmap  // 카메라 포즈 (SE3)
g2o::VertexPointXYZ    // 3D 점 (XYZ)
```

**2. Edge (제약조건)**
```cpp
g2o::EdgeProjectXYZ2UV  // 재투영 오차
```

**3. Optimizer**
```cpp
g2o::SparseOptimizer    // 비선형 최적화기
```

**4. Solver**
```cpp
g2o::BlockSolver_6_3      // Block matrix solver
g2o::LinearSolverEigen    // Linear solver
```

---

### 4. Sparse vs Dense

**왜 Sparse?**

```
일반 행렬 (Dense):
  [x x x x x]
  [x x x x x]  → O(n³) 복잡도
  [x x x x x]
  [x x x x x]
  
Sparse (대부분 0):
  [x 0 0 x 0]
  [0 x 0 0 0]  → O(m) 복잡도 (m = non-zeros)
  [0 0 x 0 x]
  [x 0 0 x 0]
```

**BA의 Hessian은 Sparse!**
```
카메라 i는 일부 점만 관측
점 j는 일부 카메라에만 보임
→ 대부분의 카메라-점 쌍은 연결 없음
→ Sparse matrix!
```

---

### 5. Schur Complement

**문제:**
```
BA는 변수가 엄청 많음:
  - 100 카메라: 600 변수
  - 10,000 점: 30,000 변수
  → 총 30,600 차원!
```

**해결: Schur Complement**

```
원래 문제:
  [Hcc  Hcp] [Δc]   [bc]
  [Hpc  Hpp] [Δp] = [bp]

Schur complement:
  (Hcc - Hcp * Hpp⁻¹ * Hpc) Δc = bc - Hcp * Hpp⁻¹ * bp
  
  먼저 카메라만 최적화 (600 차원)
  그 다음 점 업데이트 (빠름!)
```

**장점:**
```
✅ 카메라 변수만 최적화 (훨씬 작음)
✅ 점들은 closed-form 업데이트
✅ 10-100배 빠름!
```

---

### 6. Robust Kernel

**문제: Outlier**
```
일반적인 최소제곱:
  error² → ∞ (outlier 영향 큼!)
```

**해결: Huber Loss**
```
ρ(e) = { e²        if |e| ≤ δ
       { δ(2|e|-δ)  if |e| > δ

작은 오차: 제곱
큰 오차(outlier): 선형
→ Outlier 영향 감소!
```

**g2o에서:**
```cpp
g2o::RobustKernelHuber* rk = new g2o::RobustKernelHuber;
rk->setDelta(1.0);  // δ = 1 픽셀
edge->setRobustKernel(rk);
```

---

## 💡 구현 세부사항

### g2o 설치

```bash
# Dependencies
sudo apt install libsuitesparse-dev libeigen3-dev

# g2o
git clone https://github.com/RainerKuemmerle/g2o.git
cd g2o
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

### 기본 BA 구조

```cpp
#include <g2o/core/sparse_optimizer.h>
#include <g2o/core/block_solver.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include <g2o/solvers/eigen/linear_solver_eigen.h>
#include <g2o/types/sba/types_six_dof_expmap.h>

int main() {
    // 1. Optimizer 생성
    g2o::SparseOptimizer optimizer;
    
    // 2. Solver 설정
    auto linearSolver = g2o::make_unique<
        g2o::LinearSolverEigen<g2o::BlockSolver_6_3::PoseMatrixType>>();
    
    auto solver = new g2o::OptimizationAlgorithmLevenberg(
        g2o::make_unique<g2o::BlockSolver_6_3>(std::move(linearSolver))
    );
    
    optimizer.setAlgorithm(solver);
    
    // 3. 카메라 파라미터
    g2o::CameraParameters* cam_params = new g2o::CameraParameters(
        focal_length, principal_point, baseline);
    cam_params->setId(0);
    optimizer.addParameter(cam_params);
    
    // 4. Vertex: 카메라 포즈
    for (int i = 0; i < num_cameras; i++) {
        g2o::VertexSE3Expmap* v_cam = new g2o::VertexSE3Expmap();
        v_cam->setId(i);
        v_cam->setEstimate(initial_pose[i]);
        
        if (i == 0) {
            v_cam->setFixed(true);  // 첫 카메라 고정 (gauge freedom)
        }
        
        optimizer.addVertex(v_cam);
    }
    
    // 5. Vertex: 3D 점
    for (int j = 0; j < num_points; j++) {
        g2o::VertexPointXYZ* v_point = new g2o::VertexPointXYZ();
        v_point->setId(num_cameras + j);
        v_point->setEstimate(initial_point[j]);
        v_point->setMarginalized(true);  // Schur complement
        
        optimizer.addVertex(v_point);
    }
    
    // 6. Edge: 재투영 제약
    for (auto& obs : observations) {
        g2o::EdgeProjectXYZ2UV* edge = new g2o::EdgeProjectXYZ2UV();
        
        // Vertex 연결
        edge->setVertex(0, optimizer.vertex(num_cameras + obs.point_id));
        edge->setVertex(1, optimizer.vertex(obs.camera_id));
        
        // 관측값
        edge->setMeasurement(obs.uv);
        
        // 정보 행렬 (uncertainty의 역)
        edge->setInformation(Eigen::Matrix2d::Identity());
        
        // 카메라 파라미터
        edge->setParameterId(0, 0);
        
        // Robust kernel
        g2o::RobustKernelHuber* rk = new g2o::RobustKernelHuber;
        rk->setDelta(1.0);
        edge->setRobustKernel(rk);
        
        optimizer.addEdge(edge);
    }
    
    // 7. 최적화!
    optimizer.initializeOptimization();
    optimizer.optimize(10);  // 10 iterations
    
    // 8. 결과 추출
    for (int i = 0; i < num_cameras; i++) {
        g2o::VertexSE3Expmap* v = static_cast<g2o::VertexSE3Expmap*>(
            optimizer.vertex(i));
        
        auto pose = v->estimate();
        std::cout << "Camera " << i << ":\n" << pose.matrix() << std::endl;
    }
    
    return 0;
}
```

---

## 🔍 자체 점검

### 질문 1: BA의 목적
**Q:** Bundle Adjustment가 최소화하는 것은?

**A:**
```
재투영 오차의 제곱합:

Σᵢⱼ ||observed_uv - projected_uv||²

- observed: 실제 관측된 픽셀
- projected: 현재 추정값으로 투영한 픽셀
```

### 질문 2: g2o 구성요소
**Q:** g2o의 3가지 핵심 구성요소는?

**A:**
```
1. Vertex: 최적화 변수
   - VertexSE3Expmap (카메라)
   - VertexPointXYZ (점)

2. Edge: 제약 조건
   - EdgeProjectXYZ2UV (재투영)

3. Optimizer: 최적화기
   - SparseOptimizer
```

### 질문 3: Schur Complement
**Q:** Schur Complement의 장점은?

**A:**
```
1. 차원 감소
   - 전체: 30,600 차원
   - 카메라만: 600 차원
   
2. 빠른 수렴
   - 점들은 closed-form 업데이트
   
3. 효율성
   - 10-100배 빠름
```

### 질문 4: Robust Kernel
**Q:** Huber Loss를 사용하는 이유는?

**A:**
```
Outlier의 영향 감소:

- 작은 오차: e² (정확한 최적화)
- 큰 오차: 선형 (outlier 영향 제한)

→ 잘못된 매칭에 robust!
```

---

## 📝 이번 주 실습

### 구현 항목

1. **간단한 BA (3 카메라 + 5 점)**
   - Vertex 추가
   - Edge 추가
   - 최적화 실행

2. **Robust BA**
   - Huber kernel 추가
   - Outlier 테스트

3. **Schur Complement**
   - setMarginalized(true) 효과
   - 성능 비교

4. **실제 데이터**
   - Week 5 VO에 통합
   - Local BA 구현

### 예상 결과

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Bundle Adjustment with g2o
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Initial error: 45.23 px
Iteration 0: chi2 = 204.5
Iteration 1: chi2 = 87.3
Iteration 2: chi2 = 34.2
Iteration 3: chi2 = 12.5
Iteration 4: chi2 = 4.2
Converged!

Final error: 0.68 px

Camera poses:
  Camera 0: [fixed]
  Camera 1: Δt = 0.03m
  Camera 2: Δt = 0.05m

3D points:
  Point 0: Δ = 0.02m
  Point 1: Δ = 0.04m
  ...

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ BA 성공! 오차 95% 감소!
```

### 체크리스트

- [ ] g2o 설치 및 빌드
- [ ] 간단한 BA 구현
- [ ] Huber kernel 추가
- [ ] Schur complement 확인
- [ ] Week 5 VO에 통합 (선택)

---

## 🎯 핵심 요약

1. **Bundle Adjustment**
   - 카메라 + 점을 "동시에" 최적화
   - 재투영 오차 최소화
   - VO/SLAM의 핵심!

2. **g2o 사용**
   - Vertex: 카메라, 점
   - Edge: 재투영 제약
   - Optimizer: Levenberg-Marquardt

3. **최적화 기법**
   - Sparse matrix (효율성)
   - Schur complement (속도)
   - Robust kernel (outlier)

4. **실무 적용**
   - Local BA: 최근 Keyframe만
   - Global BA: Loop closure 후
   - Pose graph: 카메라만

5. **다음 단계**
   - Week 8: Ceres BA
   - Phase 4: VIO (IMU 융합)
   - Loop Closure Detection

---

**다음 주**: Week 8에서 Ceres Solver 맛보기!

다음: [Week 8 - Ceres Solver 맛보기](../week8/README.md)
