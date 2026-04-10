# Phase 3: Visual Odometry & Bundle Adjustment

> ⏰ **기간**: 4주
> 🎯 **목표**: VO 파이프라인 개념 이해, BA 원리 파악 (코드 분석 중심)
> ⏱️ **주간 시간**: 약 7-10시간
> 💻 **언어**: **C++** (g2o, Ceres 코드 읽기)

---

## 👉 **실습 가이드**: 각 week별 PRACTICE.md (예: [`week8/PRACTICE.md`](../Studies/Phase%203/week8/PRACTICE.md))

---

## 📋 Week 1: VO 파이프라인 + 모션 추정 방법

### VO 개요
- [ ] Visual Odometry 정의 (연속 이미지로 카메라 움직임 추정)
- [ ] SLAM과 VO의 차이 (Loop Closure, 전역 맵 유무)
- [ ] VO 파이프라인 전체 흐름:
  ```
  이미지 입력 → 특징점 검출 → 매칭/추적 → 모션 추정 → (Local 최적화) → 포즈 출력
  ```
- [ ] VO 유형: Monocular / Stereo / RGB-D

### 모션 추정 방법 비교

| 방법 | 입력 | 사용 시점 | 핵심 |
|------|------|----------|------|
| **2D-2D** | 특징점 대응 | VO 초기화 | Essential Matrix → R, t |
| **3D-2D (PnP)** | 3D 점 + 2D 관측 | 매 프레임 추적 | solvePnPRansac |
| **3D-3D (ICP)** | 양쪽 3D 점 | RGB-D, LiDAR | 점 거리 최소화 |

- [ ] 2D-2D: Essential Matrix 기반 초기화, 스케일 모호성
- [ ] 3D-2D (PnP): 일반적인 VO 추적의 핵심, RANSAC 결합
- [ ] 3D-3D (ICP): LiDAR SLAM에서 주로 사용 (개념만)

### SLAM에서 어디에 쓰이나?
- [ ] VINS 프론트엔드 = VO (feature tracking + 포즈 추정)
- [ ] VINS에서 Visual factor = PnP 기반 재투영 오차
- [ ] AMR 휠 오도메트리와 유사한 역할

### 🔍 자체 점검
1. VO와 SLAM의 가장 큰 차이는?
2. 2D-2D와 3D-2D 방법은 각각 언제 사용하는가?
3. Monocular VO의 근본적인 한계는?

---

## 📋 Week 2: 키프레임 + Bundle Adjustment 개념

### 키프레임 관리
- [ ] 왜 키프레임이 필요한가 (메모리, 중복 정보, 계산량)
- [ ] 키프레임 선택 기준: 이동 거리, 회전 각도, 매칭 비율
- [ ] VINS의 Sliding window (`WINDOW_SIZE` 파라미터)

### Bundle Adjustment 개념
- [ ] 정의: 카메라 포즈 + 3D 점을 **동시에** 최적화
- [ ] 비용 함수:
  ```
  min Sigma ||x_ij - pi(K, T_i, X_j)||^2
  ```
  - `x_ij`: 카메라 i에서 관측된 점 j의 2D 좌표
  - `pi()`: 3D → 2D 투영 함수
  - `T_i`: 카메라 i의 포즈, `X_j`: 3D 점 j의 좌표
- [ ] BA의 중요성: VO 드리프트 누적 → BA로 일관성 확보

### BA 최적화 구조
- [ ] 자코비안의 희소 구조 (각 관측 = 하나의 카메라 + 하나의 점)
- [ ] **Schur Complement**: 3D 점 변수 먼저 소거 → 계산 복잡도 감소
- [ ] Local BA (실시간) vs Global BA (Loop closure 후)
- [ ] VINS는 sliding window 내 Local BA

### 🔍 자체 점검
1. BA에서 최소화하는 것은 정확히 무엇인가?
2. Schur complement가 BA를 빠르게 만드는 원리는?
3. VINS의 sliding window 크기는 어떤 trade-off가 있는가?

---

## 📋 Week 3: g2o / Ceres 코드 분석

> 기존 코드를 **읽고 이해**하는 데 집중

### g2o 구조 파악
- [ ] Vertex: 최적화 변수 (포즈, 3D 점)
- [ ] Edge: 오차 항 (재투영 오차)
- [ ] Solver: 최적화 알고리즘
- [ ] `g2o/examples/ba` 예제 코드 읽기

### Ceres 구조 파악
- [ ] Cost function (재투영 오차)
- [ ] Automatic differentiation
- [ ] Parameter blocks: 포즈 (쿼터니언 + 평행이동), 3D 점
- [ ] `examples/bal_problem.cc` 코드 읽기

### g2o vs Ceres
- [ ] g2o: SLAM 특화, 그래프 구조 명확
- [ ] Ceres: 범용적, 자동 미분 편리
- [ ] **VINS는 Ceres 사용**

### 🔍 자체 점검
1. g2o에서 Vertex와 Edge는 각각 무엇을 의미하는가?
2. Ceres의 automatic differentiation 장점은?
3. VINS `optimization.cpp`에서 Ceres가 어떻게 사용되는가?

---

## 📋 Week 4: 스케일 문제 + Phase 3 마무리

### Monocular 스케일 모호성
- [ ] 핀홀 모델: 3D 점 X와 lambdaX가 같은 2D 점에 투영
- [ ] Essential Matrix에서 t는 방향만, 크기는 임의
- [ ] 스케일 드리프트: 매 프레임 오차 누적

### 스케일 복구 방법
- [ ] **Stereo**: baseline 알면 절대 깊이 계산 (`depth = f * b / disparity`)
- [ ] **IMU 융합**: 가속도 적분 → 절대 스케일 (Phase 4 핵심)
- [ ] Vision(방향 정확, 스케일 모호) + IMU(스케일 제공, 드리프트) = **상호 보완**

### VINS 코드 훑어보기
- [ ] `feature_manager.cpp`: 특징점 생명주기 관리
- [ ] `optimization.cpp`: Ceres 기반 최적화 구조
- [ ] Visual factor가 BA의 재투영 오차임을 확인

### 🔍 자체 점검
1. Monocular VO에서 스케일이 틀어지는 근본 원인은?
2. IMU가 스케일 복구에 도움이 되는 원리는?
3. VINS의 최적화 구조를 개략적으로 설명할 수 있는가?

---

## ✅ Phase 3 완료 체크리스트

### Visual Odometry
- [ ] VO 파이프라인 전체 흐름 설명 가능
- [ ] 2D-2D, 3D-2D, 3D-3D 방법 차이 이해
- [ ] 키프레임 선택 기준 이해

### Bundle Adjustment
- [ ] BA 비용 함수 설명 가능
- [ ] 재투영 오차 개념 이해
- [ ] Schur complement 원리 이해
- [ ] g2o, Ceres 코드 구조 파악

### 스케일
- [ ] Monocular 스케일 문제 설명 가능
- [ ] 왜 IMU가 필요한지 명확히 이해

---

## 🎯 Phase 3 완료 기준

> "VO 파이프라인 흐름을 설명하고, BA가 무엇을 최적화하는지 설명 가능. VINS 코드에서 해당 부분을 찾을 수 있음."

---

## 📚 참고 자료

### 강의

| 자료 | 용도 |
|------|------|
| Cyrill Stachniss - SLAM Course | VO, BA 이론 |
| TUM - Computer Vision Group | VO 개념 참고 |

### 라이브러리 (코드 읽기용)

| 라이브러리 | 용도 | 링크 |
|------------|------|------|
| g2o | 그래프 최적화 | github.com/RainerKuemmerle/g2o |
| Ceres Solver | 비선형 최적화 | ceres-solver.org |

---

## 💡 팁

1. **구현보다 이해**: 이 Phase는 코드를 **읽는** 것이 목표
2. **VINS 코드와 연결**: 개념마다 VINS 어디에 해당하는지 확인
3. **면접 대비**: "VO와 SLAM 차이?", "BA란?" 등 설명 연습
4. **80% 이해하면 다음으로**: Phase 5-6에 시간 확보가 우선

---

## ❓ 다음 단계

Phase 3 완료 후:
- Phase 4 (VIO 개념)로 진행
- Pre-integration 직관적 이해 + VINS 코드 분석
