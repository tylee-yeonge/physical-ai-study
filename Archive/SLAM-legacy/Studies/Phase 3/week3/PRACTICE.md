# Week 3 실습: g2o / Ceres 예제 코드 읽기


> **목표**: g2o와 Ceres의 BA 예제를 읽고 구조를 파악
> **방식**: 코드 분석 중심 (빌드/실행은 선택)
> **예상 시간**: 3-4시간


---


## 실습 1: g2o BA 예제 분석


### 코드 위치


```bash
# g2o 소스코드에서
g2o/examples/ba/ # 또는 ba_demo/
```


### 분석 포인트


1. **Vertex 타입 확인**: 어떤 변수가 최적화 대상인지
2. **Edge 타입 확인**: 어떤 오차 항이 정의되어 있는지
3. **Information matrix**: 어떤 값으로 설정되어 있는지
4. **수렴 과정**: iteration 수와 cost 감소 패턴


---


## 실습 2: Ceres BAL 예제 분석


### 코드 위치


```bash
# Ceres 소스코드에서
ceres-solver/examples/bal_problem.cc
ceres-solver/examples/bundle_adjuster.cc
```


### 분석 포인트


1. **CostFunction 구조**: `operator()`에서 residual 계산 방식
2. **AutoDiffCostFunction**: 템플릿 파라미터 의미
   - `<CostFunctor, residual_dim, param_block1_dim, param_block2_dim>`
3. **Solver 옵션**: `SPARSE_SCHUR`가 왜 BA에 적합한지
4. **Summary 출력**: initial cost → final cost 변화


---


## 실습 3: VINS의 Ceres 사용 분석


### 확인할 파일


```
vins_estimator/src/estimator.cpp # optimization() 함수
vins_estimator/src/factor/
+-- projection_factor.h/.cpp # Visual factor
+-- projection_td_factor.h/.cpp # Time delay 포함 버전
+-- marginalization_factor.h/.cpp # Marginalization
```


### 분석 포인트


1. `optimization()`에서 `ceres::Problem` 생성 과정
2. Visual factor의 residual 계산: `Evaluate()` 함수
3. 어떤 변수가 `AddParameterBlock`으로 추가되는지
4. `ceres::Solve()` 호출 시 옵션 설정


### VINS optimization() 흐름 요약


```cpp
void Estimator::optimization() {
    // 1. Ceres Problem 생성
    ceres::Problem problem;
    

    // 2. Parameter blocks 추가
    // - 각 키프레임의 포즈 (7: qw,qx,qy,qz, tx,ty,tz)
    // - 각 키프레임의 속도+바이어스 (9: vx,vy,vz, bax,bay,baz, bgx,bgy,bgz)
    

    // 3. IMU factor 추가 (연속 키프레임 연결)
    // problem.AddResidualBlock(imu_factor, NULL, ...)
    

    // 4. Visual factor 추가 (3D점 <-> 카메라 연결)
    // problem.AddResidualBlock(projection_factor, loss_function, ...)
    

    // 5. Marginalization factor 추가 (이전 정보 보존)
    // problem.AddResidualBlock(marginalization_factor, NULL, ...)
    

    // 6. 최적화 실행
    ceres::Solve(options, &problem, &summary);
}
```


---


## 실습 4: g2o vs Ceres 비교표 채우기


| 항목 | g2o | Ceres |
|------|-----|-------|
| 그래프 구조 | | |
| 자코비안 | | |
| BA에서 Schur | | |
| VINS에서? | | |
| ORB-SLAM에서? | | |


<details>
<summary>클릭하여 정답 확인</summary>


| 항목 | g2o | Ceres |
|------|-----|-------|
| 그래프 구조 | Vertex/Edge 명시적 | CostFunction/Problem |
| 자코비안 | 수동 작성 | 자동 미분 지원 |
| BA에서 Schur | 내장 | SPARSE_SCHUR 옵션 |
| VINS에서? | 사용 안 함 | **사용** |
| ORB-SLAM에서? | **사용** | 사용 안 함 |


</details>


---


## 체크리스트


- [ ] g2o의 Vertex/Edge 구조 이해
- [ ] Ceres의 AutoDiffCostFunction 패턴 이해
- [ ] VINS optimization() 전체 흐름 파악
- [ ] g2o vs Ceres 차이 설명 가능
