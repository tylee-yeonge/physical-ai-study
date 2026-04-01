# Week 2 실습: BA 개념 확인 + VINS 최적화 코드 읽기

> 🎯 **목표**: BA 비용 함수와 VINS의 최적화 구조를 코드에서 확인
> 💻 **방식**: 코드 분석 중심
> ⏰ **예상 시간**: 3-4시간

---

## 실습 1: VINS 최적화 구조 확인

### 확인할 파일

```
vins_estimator/src/
├── estimator.cpp          # optimization() 호출부
├── factor/
│   ├── projection_factor.cpp  # Visual factor (재투영 오차)
│   └── imu_factor.cpp         # IMU factor (Phase 4)
```

### 확인 포인트

1. `estimator.cpp`에서 `optimization()` 함수 찾기
   - Ceres Problem 생성 과정
   - AddResidualBlock 호출 확인
   - Visual factor와 IMU factor가 어떻게 추가되는지

2. `projection_factor.cpp`에서 재투영 오차 확인
   - `Evaluate()` 함수에서 residual 계산 방식
   - 3D 점 → 2D 투영 → 관측값과 비교

3. `WINDOW_SIZE` 파라미터 확인
   - sliding window 크기가 최적화에 미치는 영향

---

## 실습 2: Schur Complement 직관 이해

아래 질문에 답해보세요:

1. BA에서 카메라 포즈 수와 3D 점 수 중 어느 쪽이 훨씬 많은가?
2. Schur complement로 먼저 소거하는 것은 어느 쪽이고, 왜인가?
3. VINS가 Global BA 대신 sliding window를 쓰는 이유는?

---

## 실습 3: 키프레임 선택 로직 확인

VINS에서 키프레임 선택 관련 코드를 찾아보세요:

- `estimator.cpp`에서 `addFeatureCheckParallax()` 또는 유사한 함수
- Parallax(시차) 기반 키프레임 판단 로직
- `WINDOW_SIZE`에 도달했을 때 어떤 키프레임을 제거하는지

### 키프레임 판단 핵심 로직

```cpp
// feature_manager.cpp에서 (개략적 흐름)
// 새 프레임의 특징점과 이전 키프레임의 공유 특징점 시차(parallax) 계산
// parallax > threshold → 키프레임으로 채택
// parallax < threshold → 일반 프레임 (sliding window에서 제거 대상)
```

**확인할 것:**
- [ ] parallax 계산 방식 (2D 좌표 차이의 평균)
- [ ] threshold 값 (config에서 `MIN_PARALLAX`)
- [ ] MARGIN_OLD vs MARGIN_SECOND_NEW 차이

---

## 실습 4: BA 개념 정리표 채우기

| 항목 | 내용 |
|------|------|
| BA의 목표 | |
| 비용 함수 | |
| 최적화 변수 | |
| Schur complement 역할 | |
| VINS에서 어디? | |
| Local vs Global 차이 | |

<details>
<summary>클릭하여 정답 확인</summary>

| 항목 | 내용 |
|------|------|
| BA의 목표 | 재투영 오차 최소화 |
| 비용 함수 | Sigma \|\|x_ij - pi(K, T_i, X_j)\|\|^2 |
| 최적화 변수 | 카메라 포즈 T_i + 3D 점 X_j |
| Schur complement 역할 | 3D 점 먼저 소거 → 포즈만의 작은 시스템 |
| VINS에서 어디? | estimator.cpp → optimization() |
| Local vs Global 차이 | Local: 최근 키프레임만 (실시간) / Global: 전체 맵 (Loop 후) |

</details>

---

## 체크리스트

- [ ] VINS optimization() 함수의 Ceres Problem 구성 흐름 파악
- [ ] Visual factor의 residual 계산 방식 이해
- [ ] Schur complement가 왜 BA에 필요한지 설명 가능
- [ ] 키프레임 선택의 parallax 기반 로직 확인
