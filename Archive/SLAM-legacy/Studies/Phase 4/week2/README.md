# Week 2: Pre-integration + Factor Graph

> [goal] **목표**: Pre-integration의 필요성과 직관, Factor Graph에서의 역할 이해
> [time] **예상 시간**: 7-10시간
> [tip] **핵심 질문**: "왜 IMU를 단순 적분하지 않고 Pre-integration을 쓰는가?"

---

## [ref] 핵심 개념

### 1. Pre-integration 필요성

#### 문제 상황

```
IMU: ████████████████████████████ (200Hz)
Vision:     #         #         # (20Hz)
           KF_i      KF_j      KF_k
```

키프레임 i→j 사이에 IMU 데이터 수십 개가 있다.

#### 단순 적분의 문제

1. KF_i의 포즈에서 시작하여 IMU를 순차 적분 → KF_j 예측
2. **최적화로 KF_i의 포즈가 바뀌면?**
3. → 처음부터 다시 적분해야 함 (매 최적화 iteration마다!)

#### Pre-integration 아이디어

**포즈에 독립적인 상대 측정값을 미리 계산:**

```
기존:  "서울역에서 출발해서 100m 직진, 좌회전, 50m..."
       (서울역 위치 바뀌면 전부 다시 계산)

Pre-integration: "출발점 기준 최종 변위: (150m, 30도 회전)"
                 (출발점 바뀌어도 상대 변위는 그대로)
```

- **Delta_p**: i 프레임 기준 j까지의 상대 위치
- **Delta_v**: i 프레임 기준 j에서의 상대 속도
- **Delta_q**: i 프레임 기준 j까지의 상대 회전

**핵심**: 이 값들은 i, j의 **절대 포즈와 무관**

### 2. 바이어스 보정 (1차 근사)

바이어스가 조금 변하면 재적분 없이 자코비안으로 보정:

```
Delta_p_corrected ≈ Delta_p + J_p * delta_b
Delta_v_corrected ≈ Delta_v + J_v * delta_b
Delta_q_corrected ≈ Delta_q * Exp(J_q * delta_bg)
```

→ 바이어스가 최적화로 업데이트되어도 빠르게 반영 가능

### 3. Factor Graph

#### 구조

```
변수 노드: 추정할 것 (포즈, 속도, 바이어스)
팩터 노드: 제약 조건 (측정값)

[KF_0]--IMU--[KF_1]--IMU--[KF_2]--IMU--[KF_3]
  |           |           |           |
  +--Visual---+--Visual---+--Visual---+
              \          /
               [3D Point]
```

#### IMU Factor

```
[KF_i] ----[IMU Factor]---- [KF_j]
  |                            |
(p,v,R,b)_i              (p,v,R,b)_j
```

- Pre-integrated measurement (Delta_p, Delta_v, Delta_q)를 Factor로
- 연속 키프레임을 연결
- 공분산(불확실성) 포함

#### Visual Factor

```
[KF_i] ----[Visual Factor]---- [3D Point]
           (재투영 오차)
```

- Phase 3에서 배운 BA의 재투영 오차

### 4. VINS 코드 연결

- `integration_base.h`: Pre-integration 클래스
  - `propagate()`: IMU 데이터 하나씩 적분
  - `midPointIntegration()`: 중점법 적분
- `imu_factor.h`: IMU Factor 정의
  - `Evaluate()`: Factor 오차 계산
  - residual = 예측값(pre-integration) - 최적화 변수에서 계산한 값

---

## [search] 자체 점검

1. Pre-integration이 없으면 왜 비효율적인가?
2. Delta_p, Delta_v, Delta_q가 "포즈 독립적"이라는 게 무슨 의미인가?
3. 바이어스가 변할 때 왜 재적분 없이 보정 가능한가?
4. Factor Graph에서 IMU Factor와 Visual Factor의 역할 차이는?

---

다음: [Week 3 - VIO 초기화 + Camera-IMU 캘리브레이션](../week3/README.md)
