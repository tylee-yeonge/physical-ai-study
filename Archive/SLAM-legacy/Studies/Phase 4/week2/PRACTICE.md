# Week 2 실습: Pre-integration 개념 확인 + VINS 코드 읽기


> **목표**: VINS의 Pre-integration 코드 구조를 파악
> **방식**: 코드 분석 중심
> **예상 시간**: 4-5시간


---


## 실습 1: VINS Pre-integration 코드 분석


### 확인할 파일


```
vins_estimator/src/
+-- factor/imu_factor.h # IMU Factor 정의
+-- utility/integration_base.h # Pre-integration 클래스
```


### integration_base.h 분석 포인트


1. **멤버 변수 확인**
   - `delta_p`, `delta_v`, `delta_q`: Pre-integrated 측정값
   - `jacobian`: 바이어스 보정용 자코비안
   - `covariance`: 공분산 (불확실성)


2. **`propagate()` 함수 읽기**
   - IMU 데이터를 하나씩 받아 적분
   - 자코비안과 공분산도 함께 업데이트


3. **`evaluate()` 함수 읽기**
   - 바이어스 변화에 따른 1차 보정 확인
   - `corrected_delta_q`, `corrected_delta_v`, `corrected_delta_p`


### imu_factor.h 분석 포인트


1. **`Evaluate()` 함수**
   - residual 계산: 예측값 vs 최적화 변수에서 계산한 값
   - residual 크기: 15차원 (Delta_p 3 + Delta_v 3 + Delta_q 3 + bias_a 3 + bias_g 3)


---


## 실습 2: Factor Graph 직접 그려보기


종이에 VINS의 Factor Graph를 그려보세요:


1. 변수 노드: KF_0 ~ KF_10 (각각 p, v, R, b_a, b_g)
2. IMU Factor: 연속 키프레임 연결
3. Visual Factor: 키프레임과 3D 점 연결
4. Marginalization Factor: 오래된 정보 요약


### 확인할 것
- [ ] 각 Factor가 어떤 변수를 연결하는지
- [ ] Sliding window 밖 정보는 어떻게 처리되는지


---


## 실습 3: Pre-integration vs Naive 적분 비교


개념적으로 비교 정리:


| 항목 | Naive 적분 | Pre-integration |
|------|-----------|-----------------|
| 포즈 변경 시 | 재적분 필요 | 재적분 불필요 |
| 바이어스 변경 시 | 재적분 필요 | 1차 보정으로 해결 |
| 계산 비용 | O(N * iteration) | O(N) + O(1 * iteration) |
| VINS에서 사용 | X | O |


---


## 실습 4: integration_base.h 핵심 코드 따라가기


### midPointIntegration() 흐름


```cpp
// integration_base.h 의 midPointIntegration (개략)
void midPointIntegration(
    double dt,
    const Vector3d &acc_0, const Vector3d &gyro_0, // 이전 IMU
    const Vector3d &acc_1, const Vector3d &gyro_1, // 현재 IMU
    ...)
{
    // 1. 중점법으로 각속도 평균
    Vector3d un_gyr = 0.5 * (gyro_0 + gyro_1) - linearized_bg;
    

    // 2. 회전 업데이트 (쿼터니언)
    result_delta_q = delta_q * Quaterniond(1, un_gyr(0)*dt/2, ...);
    

    // 3. 중점법으로 가속도 평균 (회전 보정 포함)
    un_acc_0 = delta_q * (acc_0 - linearized_ba);
    un_acc_1 = result_delta_q * (acc_1 - linearized_ba);
    un_acc = 0.5 * (un_acc_0 + un_acc_1);
    

    // 4. 위치, 속도 업데이트
    result_delta_p = delta_p + delta_v * dt + 0.5 * un_acc * dt * dt;
    result_delta_v = delta_v + un_acc * dt;
    

    // 5. 자코비안, 공분산 업데이트 (1차 보정용)
    // ... (F, G 행렬 계산)
}
```


**확인할 것:**
- [ ] `linearized_ba`, `linearized_bg`: 바이어스 초기값
- [ ] 바이어스를 빼고 적분하는 이유 (측정 모델 복습)
- [ ] 자코비안이 나중에 바이어스 보정에 어떻게 사용되는지


---


## 실습 5: Pre-integration 면접 대비 (30분)


아래 질문에 **자기 말로** 답을 써보세요:


1. **"Pre-integration이 왜 필요한가요?"**
2. **"바이어스가 바뀌면 어떻게 하나요?"**
3. **"Factor Graph에서 IMU factor의 역할은?"**


---


## 체크리스트


- [ ] integration_base.h의 멤버 변수 (delta_p, delta_v, delta_q) 의미 이해
- [ ] propagate() → midPointIntegration() 흐름 파악
- [ ] imu_factor.h의 Evaluate()에서 residual 계산 확인
- [ ] Factor Graph를 종이에 직접 그려봄
- [ ] Pre-integration vs Naive 적분 차이 설명 가능
