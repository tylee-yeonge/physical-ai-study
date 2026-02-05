# Phase 3: Visual Odometry & Bundle Adjustment

> ⏰ **기간**: 3개월  
> 🎯 **목표**: VO 파이프라인 이해 및 간단한 구현, BA 개념 파악  
> ⌛️ **주간 시간**: 약 10시간
> 💻 **언어**: **C++** (g2o, Ceres)

---

## 👉 **실습 가이드**: [`Studies/Phase 3/PRACTICE.md`](file:///Users/yeonge/SynologyDrive/1.%20YeongE/7.%20Visual%20SLAM%20Study/visual-slam-and-perception-learning/Studies/Phase%203/PRACTICE.md)

**주요 실습**:
- Monocular VO 구현 (C++, FeatureTracker, MotionEstimator)
- g2o Bundle Adjustment 
- Ceres Bundle Adjustment
- KITTI/EuRoC 데이터셋으로 궁적 추정

> ⚠️ **필수 C++**: VINS `optimization.cpp` 분석 대비

---

## 📋 Section 3.1: Visual Odometry 개요 (1주)

### Week 1: VO 파이프라인 이해

#### VO란?
- [ ] Visual Odometry 정의 (연속 이미지로 카메라 움직임 추정)
- [ ] SLAM과 VO의 차이 (Loop Closure, 전역 맵 유무)
- [ ] Odometry vs Localization

#### VO 파이프라인 전체 흐름
```
이미지 입력 → 특징점 검출 → 매칭/추적 → 모션 추정 → (Local 최적화) → 포즈 출력
```
- [ ] 각 단계가 하는 일 파악
- [ ] 어디서 병목이 생기는지
- [ ] 실시간 처리를 위한 고려사항

#### VO 유형
- [ ] Monocular VO: 단안 카메라, 스케일 모호성
- [ ] Stereo VO: 스테레오 카메라, 스케일 복원 가능
- [ ] RGB-D VO: 깊이 카메라, 직접 깊이 사용

#### SLAM에서 어디에 쓰이나?
- [ ] VINS의 프론트엔드 = VO (feature tracking + 포즈 추정)
- [ ] VO 결과가 백엔드 최적화의 입력
- [ ] AMR에서 휠 오도메트리와 유사한 역할

### 🔍 Section 3.1 자체 점검
1. VO와 SLAM의 가장 큰 차이는?
2. Monocular VO의 근본적인 한계는?
3. VO 파이프라인에서 가장 계산량이 많은 단계는?

---

## 📋 Section 3.2: 모션 추정 방법 (4주)

### Week 2: 2D-2D 모션 추정

#### 개념
- [ ] 두 이미지 간 2D 특징점 대응만 있을 때
- [ ] Essential Matrix 기반
- [ ] 주로 **VO 초기화**에 사용

#### 과정
- [ ] 특징점 매칭 (ORB 또는 KLT 추적)
- [ ] Essential Matrix 추정 (5-point + RANSAC)
- [ ] E 분해 → R, t 복원
- [ ] Cheirality check로 올바른 해 선택
- [ ] 삼각측량으로 초기 3D 맵 생성

#### 한계
- [ ] 스케일 모호성: t의 방향은 알지만 크기는 모름
- [ ] Pure rotation 문제: 평행이동 없으면 E 추정 불안정
- [ ] Degenerate case: 평면 장면

#### 실습
- [ ] OpenCV `findEssentialMat` + `recoverPose` 사용
- [ ] 두 이미지에서 상대 포즈 추정
- [ ] 삼각측량으로 3D 점 복원

### Week 3: 3D-2D 모션 추정 (PnP)

#### 개념
- [ ] 이미 알고 있는 3D 점 + 현재 2D 관측
- [ ] **일반적인 VO 추적의 핵심** 방법
- [ ] 초기화 이후 매 프레임 사용

#### 과정
- [ ] 이전 프레임/맵에서 3D 점 확보
- [ ] 현재 프레임에서 대응 2D 점 찾기 (매칭 또는 추적)
- [ ] PnP + RANSAC으로 현재 포즈 추정

#### 알고리즘
- [ ] P3P: 최소 3점, 최대 4개의 해
- [ ] EPnP: 효율적인 O(n) 방법
- [ ] OpenCV `solvePnPRansac` 사용법

#### SLAM에서 어디에 쓰이나?
- [ ] VINS에서 Visual factor = PnP 기반 재투영 오차
- [ ] 매 프레임 포즈 추정의 기본
- [ ] Relocalization에서도 PnP 사용

#### 실습
- [ ] 삼각측량된 3D 점으로 다음 프레임 포즈 추정
- [ ] RANSAC inlier 비율 확인
- [ ] 추정 정확도 시각화

### Week 4: 3D-3D 모션 추정 (ICP)

#### 개념
- [ ] 두 프레임 모두 3D 점이 있을 때
- [ ] 주로 RGB-D, LiDAR에서 사용
- [ ] AMR에서 LiDAR SLAM에 많이 사용

#### ICP (Iterative Closest Point)
- [ ] Point-to-Point ICP: 점과 점 거리 최소화
- [ ] Point-to-Plane ICP: 점과 평면 거리 최소화 (더 빠른 수렴)
- [ ] 수렴 조건 및 초기값 중요성

#### SVD 기반 방법 (대응점 알 때)
- [ ] Closed-form 해 존재
- [ ] 중심점 정렬 → SVD로 회전 추정 → 평행이동 계산

> 💡 VIO에서는 주로 2D-2D (초기화)와 3D-2D (추적)를 사용. ICP는 참고로 알아두기.

### Week 5: 미니 프로젝트 - 간단한 Mono VO

#### 목표
- [ ] 데이터셋에서 연속 프레임으로 카메라 궤적 추정

#### 구현 단계
1. [ ] 데이터셋 준비 (KITTI 또는 EuRoC)
2. [ ] 특징점 검출 (FAST 또는 ORB)
3. [ ] 특징점 추적 (KLT) 또는 매칭
4. [ ] 첫 두 프레임: 2D-2D로 초기화 (Essential Matrix)
5. [ ] 이후 프레임: 3D-2D로 추적 (PnP)
6. [ ] 새 맵 포인트 삼각측량
7. [ ] 궤적 시각화 및 Ground truth 비교

#### 예상 결과
- [ ] 궤적이 대략적인 형태는 맞지만 스케일 드리프트 관찰
- [ ] 이게 왜 IMU가 필요한지에 대한 동기 부여

### 🔍 Section 3.2 자체 점검
1. 2D-2D와 3D-2D 방법은 각각 언제 사용하는가?
2. PnP에서 RANSAC을 쓰는 이유는?
3. Monocular VO 구현 시 스케일이 틀어지는 이유는?

---

## 📋 Section 3.3: 키프레임 관리 (2주)

### Week 6: 키프레임 개념

#### 왜 키프레임이 필요한가
- [ ] 모든 프레임 저장: 메모리 폭발
- [ ] 유사한 프레임 중복: 정보량 낮음
- [ ] 최적화 계산량 감소 필수

#### 키프레임 선택 기준
- [ ] 이동 거리 기반: 일정 거리 이상 이동 시
- [ ] 회전 각도 기반: 일정 각도 이상 회전 시
- [ ] 특징점 매칭 비율: 기존 키프레임과 공유 특징점 감소 시
- [ ] 시간 간격: 최소 시간 보장

#### VINS에서의 키프레임
- [ ] Sliding window 내 키프레임 유지
- [ ] 오래된 키프레임 → Marginalization
- [ ] `WINDOW_SIZE` 파라미터

### Week 7: 로컬 맵 관리

#### 로컬 맵 구조
- [ ] 최근 N개 키프레임 + 관측된 맵 포인트
- [ ] Covisibility graph: 공동 관측 키프레임 연결
- [ ] 효율적인 검색을 위한 구조

#### 맵 포인트 관리
- [ ] 생성: 두 키프레임에서 삼각측량
- [ ] 업데이트: 추가 관측으로 정확도 향상
- [ ] 제거 (Culling): 관측 횟수 적거나 outlier인 점

#### SLAM에서 어디에 쓰이나?
- [ ] VINS `FeatureManager`: 특징점 생명주기 관리
- [ ] Sliding window가 로컬 맵 역할
- [ ] Marginalization = 오래된 정보 요약

### 🔍 Section 3.3 자체 점검
1. 키프레임 선택이 너무 빈번하면 어떤 문제가 생기는가?
2. 키프레임 선택이 너무 드물면 어떤 문제가 생기는가?
3. VINS의 sliding window 크기는 어떤 trade-off가 있는가?

---

## 📋 Section 3.4: Bundle Adjustment (4주)

### Week 8: BA 개념

#### Bundle Adjustment란?
- [ ] 정의: 카메라 포즈들과 3D 점들을 **동시에** 최적화
- [ ] "Bundle": 3D 점에서 나온 광선 다발
- [ ] "Adjustment": 광선들이 일관되도록 조정

#### 비용 함수
```
min Σ ||x_ij - π(K, T_i, X_j)||²
```
- [ ] `x_ij`: 카메라 i에서 관측된 점 j의 2D 좌표
- [ ] `π()`: 3D → 2D 투영 함수
- [ ] `T_i`: 카메라 i의 포즈
- [ ] `X_j`: 3D 점 j의 좌표

#### BA의 중요성
- [ ] VO만으로는 드리프트 누적
- [ ] BA로 전체적인 일관성 확보
- [ ] SLAM 백엔드의 핵심

### Week 9: BA 최적화 기법

#### 자코비안 행렬 구조
- [ ] ∂e/∂T (카메라 포즈에 대한 미분)
- [ ] ∂e/∂X (3D 점에 대한 미분)
- [ ] 희소 구조: 각 관측은 하나의 카메라, 하나의 점에만 연결

#### Schur Complement (핵심!)
- [ ] BA의 희소 구조 활용
- [ ] Hessian을 카메라-점 블록으로 분할
- [ ] 3D 점 변수를 먼저 소거
- [ ] 카메라 포즈만의 작은 시스템 풀기
- [ ] 계산 복잡도 대폭 감소

#### Local BA vs Global BA
- [ ] Local BA: 최근 키프레임 + 관련 점만 (실시간 가능)
- [ ] Global BA: 전체 맵 (Loop closure 후)
- [ ] VINS는 sliding window 내 Local BA

#### SLAM에서 어디에 쓰이나?
- [ ] VINS `optimization()` 함수
- [ ] `slidingWindowOptimization` = Local BA + IMU factor
- [ ] Visual factor가 BA의 재투영 오차

### Week 10: g2o 실습

#### g2o 기본 구조
- [ ] Vertex: 최적화 변수 (포즈, 3D 점)
- [ ] Edge: 오차 항 (재투영 오차)
- [ ] Solver: 최적화 알고리즘

#### 실습
- [ ] g2o 설치 확인
- [ ] `g2o/examples/ba` 예제 분석
- [ ] 간단한 BA 문제 직접 구성
  - Vertex 추가: `VertexSE3Expmap` (포즈), `VertexPointXYZ` (3D 점)
  - Edge 추가: `EdgeProjectXYZ2UV`
- [ ] 최적화 실행 및 결과 확인

### Week 11: Ceres 실습

#### Ceres로 BA 구현
- [ ] Cost function 정의 (재투영 오차)
- [ ] Automatic differentiation 사용
- [ ] Parameter blocks: 포즈 (쿼터니언 + 평행이동), 3D 점

#### 실습
- [ ] Ceres `examples/bal_problem.cc` 분석
- [ ] BAL 데이터셋으로 실행
- [ ] 수렴 과정 관찰 (iteration, cost)

#### g2o vs Ceres
- [ ] g2o: SLAM 특화, 그래프 구조 명확
- [ ] Ceres: 범용적, 자동 미분 편리
- [ ] VINS는 Ceres 사용

### 🔍 Section 3.4 자체 점검
1. BA에서 최소화하는 것은 정확히 무엇인가?
2. Schur complement가 BA를 빠르게 만드는 원리는?
3. Local BA와 Global BA는 각각 언제 수행하는가?

---

## 📋 Section 3.5: 스케일 문제 이해 (2주)

### Week 12: Monocular 스케일 모호성

#### 왜 스케일을 모르는가
- [ ] 핀홀 모델: 3D 점 X와 λX가 같은 2D 점에 투영
- [ ] Essential Matrix에서 t는 방향만, 크기는 임의
- [ ] 초기화 시 ||t|| = 1로 정규화

#### 스케일 드리프트
- [ ] 매 프레임 스케일 오차 누적
- [ ] 실제 1m 이동을 0.9m로 추정 → 오차 누적
- [ ] 긴 궤적에서 맵이 실제와 크게 달라짐

#### 실습으로 확인
- [ ] 미니 프로젝트 VO 결과와 Ground truth 비교
- [ ] 스케일 정렬 (Sim(3)) 후 비교
- [ ] 드리프트 패턴 관찰

### Week 13: 스케일 복구 방법

#### Stereo 카메라
- [ ] 베이스라인 b를 알면 절대 깊이 계산 가능
- [ ] `depth = f * b / disparity`
- [ ] 스케일 문제 없음

#### IMU 융합 (VIO의 핵심!)
- [ ] 가속도 적분 → 속도 → 위치 (절대 스케일)
- [ ] Vision: 방향은 정확, 스케일 모호
- [ ] IMU: 스케일 정보 제공, 드리프트 있음
- [ ] **상호 보완적** → Phase 4에서 자세히

#### 기타 방법 (참고)
- [ ] 알려진 물체 크기 사용
- [ ] 높이 고정 가정 (지면 평면)
- [ ] GPS 융합

#### 왜 IMU인가?
- [ ] AMR에는 이미 IMU가 있음
- [ ] 추가 센서 없이 스케일 복구 가능
- [ ] 빠른 움직임에서 Vision 보완

### 🔍 Section 3.5 자체 점검
1. Monocular VO에서 스케일이 틀어지는 근본 원인은?
2. Stereo 카메라가 스케일을 알 수 있는 이유는?
3. IMU가 스케일 복구에 도움이 되는 원리는?

---

## ✅ Phase 3 완료 체크리스트

### Visual Odometry
- [ ] VO 파이프라인 전체 흐름 설명 가능
- [ ] 2D-2D, 3D-2D, 3D-3D 방법 차이 이해
- [ ] 간단한 Mono VO 구현 경험
- [ ] 키프레임 선택 기준 이해

### Bundle Adjustment
- [ ] BA 문제 정의 (비용 함수) 설명 가능
- [ ] 재투영 오차 개념 이해
- [ ] Schur complement 원리 이해
- [ ] g2o 또는 Ceres로 BA 실행 경험

### 스케일
- [ ] Monocular 스케일 문제 설명 가능
- [ ] 스케일 드리프트 직접 관찰
- [ ] 왜 IMU가 필요한지 명확히 이해

---

## 🎯 Phase 3 완료 기준

> "왜 순수 Vision만으로는 부족하고, IMU가 필요한지 구체적 예시와 함께 설명 가능"

---

## 📚 참고 자료

### 강의

| 자료 | 용도 |
|------|------|
| Cyrill Stachniss - SLAM Course | VO, BA 이론 |
| TUM - Computer Vision Group | VO 구현 참고 |

### 라이브러리

| 라이브러리 | 용도 | 링크 |
|------------|------|------|
| g2o | 그래프 최적화 | github.com/RainerKuemmerle/g2o |
| Ceres Solver | 비선형 최적화 | ceres-solver.org |
| GTSAM | Factor graph | gtsam.org |

### 데이터셋

| 데이터셋 | 용도 |
|----------|------|
| KITTI | Stereo VO 테스트 |
| EuRoC | Mono/Stereo + IMU |
| TUM RGB-D | RGB-D VO 테스트 |

---

## 💡 팁

1. **간단한 것부터**: 합성 데이터나 짧은 시퀀스로 먼저 테스트
2. **시각화 필수**: 궤적, 맵 포인트, 최적화 과정을 항상 시각화
3. **수렴 확인**: BA에서 iteration 수, cost 감소 패턴 관찰
4. **VINS와 연결**: `optimization.cpp`가 이 Phase의 내용 구현
5. **스케일 드리프트 체험**: 직접 겪어봐야 IMU 필요성 체감됨

---

## ❓ 다음 단계

Phase 3 완료 후:
- Phase 4 (VIO 핵심)로 진행
- IMU를 어떻게 Vision과 융합하는지 학습
- Pre-integration 개념 이해