# Week 4: 3D-3D 모션 추정 - Point Cloud 정합

> 🎯 **이번 주 목표**: ICP 알고리즘으로 Point Cloud들을 정합하는 방법 이해하기
> ⏰ **예상 시간**: 10시간
> 💡 **핵심 질문**: "두 개의 3D Point Cloud를 어떻게 맞출까?"

---

## 🌟 시작하기 전에

### VIO에서 ICP의 위치

**중요한 알림:** 
```
VO/VIO (Visual Odometry/Inertial)에서는 주로 사용 안 함!
  Week 2: 2D-2D (Essential) ← 초기화
  Week 3: 3D-2D (PnP)       ← 추적 ⭐
  Week 4: 3D-3D (ICP)       ← 참고용
```

**그럼 왜 배우나요?**
```
✅ RGB-D SLAM (Kinect, RealSense)
✅ LiDAR SLAM (자율주행, AMR)
✅ 3D 스캔 정합
✅ Point Cloud 처리 기초

→ LiDAR 기반 시스템에서 필수!
```

**비유:**
```
Week 2-3: 사진(2D)으로 위치 찾기 (카메라)
Week 4:   점 구름(3D)으로 위치 찾기 (LiDAR)
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. 3D-3D 모션 추정이 뭐죠?

**상황:**
```
시간 t=0: LiDAR 스캔 → Point Cloud P
          (책상, 의자, 벽 등 10,000개 점)

시간 t=1: 조금 이동 후 다시 스캔 → Point Cloud Q
          (같은 물체들, 다른 위치)

질문: "얼만큼 이동했지?"
```

**ICP (Iterative Closest Point)의 역할:**
```
두 Point Cloud:
  P = {p₁, p₂, ..., pₙ} (소스)
  Q = {q₁, q₂, ..., qₘ} (타겟)

찾기:
  [R|t]: P를 Q로 변환하는 포즈
```

**언제 사용하나요?**
- RGB-D 카메라 (실내 로봇)
- LiDAR SLAM (자율주행, AMR)
- 3D 모델 정합

---

### 2. ICP 알고리즘 - 단계별 이해

#### 핵심 아이디어

**비유:**
```
상황: 퍼즐 두 조각을 맞추기

방법:
  1. 일단 대충 겹쳐 놓기
  2. 가까운 점들끼리 짝 짓기
  3. 짝들이 최대한 가까워지도록 조정
  4. 2-3 반복!
```

#### ICP 4단계 (반복)

```
┌────────────────────────────────────┐
│ ICP 반복                            │
├────────────────────────────────────┤
│ 1. 대응 찾기 (Correspondence)       │
│    각 p_i → 가장 가까운 q_j 찾기    │
│                                     │
│ 2. 변환 계산 (Alignment)            │
│    대응점들로 R, t 계산 (SVD)       │
│                                     │
│ 3. 점 변환 (Transform)              │
│    p'_i = R·p_i + t                │
│                                     │
│ 4. 수렴 체크 (Convergence)          │
│    오차 감소 < threshold? → 끝     │
└────────────────────────────────────┘
```

#### Step 1: 대응 찾기

**목표:** 각 소스 점에 대해 타겟에서 가장 가까운 점 찾기

**Naive 방법:**
```
for 소스의 각 점 p_i:
    for 타겟의 각 점 q_j:
        거리 계산
    가장 가까운 q_j 선택

복잡도: O(n×m) - 매우 느림! 😢
```

**K-D Tree 사용:**
```
1. 타겟 점들로 K-D Tree 구축: O(m log m)
2. 각 소스 점 검색: O(n log m)

총 복잡도: O((n+m) log m) - 훨씬 빠름! 😊
```

**예시:**
```
소스:
  p₁ = (1.0, 0.5, 2.0)
  p₂ = (1.5, 0.8, 2.3)
  
타겟 (K-D Tree):
  q₁ = (1.1, 0.4, 2.1)  ← p₁과 가까움
  q₂ = (1.6, 0.7, 2.4)  ← p₂와 가까움
  ...

대응:
  (p₁, q₁), (p₂, q₂), ...
```

#### Step 2: 변환 계산 (SVD 방법)

**문제:**
```
대응점들:
  (p₁, q₁), (p₂, q₂), ..., (pₙ, qₙ)

찾기:
  R, t를 최소화:
    Σ ||q_i - (R·p_i + t)||²
```

**SVD로 Closed-form 해:**

```
1. 중심점 계산
   p̄ = (1/n) Σ p_i
   q̄ = (1/n) Σ q_i

2. 중심화
   p'_i = p_i - p̄
   q'_i = q_i - q̄

3. Covariance 행렬
   H = Σ p'_i · (q'_i)ᵀ
   
4. SVD 분해
   H = U · Σ · Vᵀ

5. R, t 계산
   R = V · Uᵀ
   t = q̄ - R · p̄
```

**OpenCV/Eigen 코드:**
```cpp
// 중심점
Eigen::Vector3d p_mean = ...;
Eigen::Vector3d q_mean = ...;

// 중심화
for (auto& p : src) p -= p_mean;
for (auto& q : tgt) q -= q_mean;

// Covariance
Eigen::Matrix3d H = Eigen::Matrix3d::Zero();
for (int i = 0; i < n; i++) {
    H += src[i] * tgt[i].transpose();
}

// SVD
Eigen::JacobiSVD<Eigen::Matrix3d> svd(H, 
    Eigen::ComputeFullU | Eigen::ComputeFullV);
R = svd.matrixV() * svd.matrixU().transpose();

// Reflection 체크
if (R.determinant() < 0) {
    V.col(2) *= -1;
    R = V * U.transpose();
}

t = q_mean - R * p_mean;
```

#### Step 3: 점 변환

**간단:**
```cpp
for (auto& p : src) {
    p = R * p + t;
}
```

#### Step 4: 수렴 체크

**방법:**
```
오차(iteration i) = Σ ||q_i - (R·p_i + t)||² / n

IF |오차_i - 오차_{i-1}| < 1e-6:
    수렴! 끝!
ELSE:
    다시 Step 1로
```

**보통 10-20회 반복으로 수렴**

---

### 3. Point-to-Point vs Point-to-Plane

#### Point-to-Point (위에서 설명)

**최소화:**
```
min Σ ||q_i - (R·p_i + t)||²
```

**장점:**
```
✅ 간단
✅ SVD로 Closed-form 해
```

**단점:**
```
❌ 수렴 느림
❌ 평면에 수직 방향으로 슬라이딩
```

#### Point-to-Plane (더 빠른 수렴)

**최소화:**
```
min Σ (n_i · (R·p_i + t - q_i))²

n_i: 점 q_i에서의 법선 벡터
```

**장점:**
```
✅ 수렴 훨씬 빠름 (5회 이내)
✅ 평면에서 더 정확
```

**단점:**
```
❌ 법선 계산 필요
❌ Linear system (SVD 불가)
```

**비유:**
```
Point-to-Point: 점과 점 사이 거리 최소화
                (모든 방향 균등)

Point-to-Plane: 평면에 수직 거리만 최소화
                (접선 방향은 무시)
                → 평면 장면에서 더 빠름!
```

---

### 4. ICP의 한계점과 해결 방법

#### 한계 1: Local Minimum

**문제:**
```
초기 위치가 너무 멀면:
  → 잘못된 대응 → 잘못된 R, t
  → 발산하거나 잘못된 해에 수렴
```

**해결:**
```
✅ 좋은 초기값 필요:
   - IMU로 대략적인 포즈 예측
   - Wheel Odometry
   - 이전 프레임 포즈

✅ Multi-scale ICP:
   - Coarse → Fine
```

#### 한계 2: Outlier에 민감

**문제:**
```
동적 물체 (사람, 차):
  → 잘못된 대응
  → 전체 결과 왜곡
```

**해결:**
```
✅ Trimmed ICP:
   - 상위 k% 오차 제거
   
✅ Robust Kernel:
   - Huber, Tukey loss
   - Outlier 영향 감소

✅ RANSAC + ICP:
   - Random sampling
   - Inlier 많은 것 선택
```

#### 한계 3: 계산량

**문제:**
```
실시간 LiDAR (10Hz):
  - 100,000개 점/프레임
  - 너무 느림!
```

**해결:**
```
✅ Voxel Grid Filter:
   - Point Cloud 간소화
   - 10,000개로 줄임

✅ K-D Tree:
   - 빠른 최근접 이웃

✅ GPU 가속:
   - CUDA ICP
```

---

### 5. VIO vs LiDAR SLAM 비교

| 항목 | VIO (Week 2-3) | LiDAR SLAM (Week 4) |
|------|----------------|---------------------|
| **센서** | 카메라 | LiDAR |
| **데이터** | 2D 이미지 | 3D Point Cloud |
| **방법** | Essential + PnP | ICP |
| **스케일** | 모호 (IMU 필요) | 정확 (거리 측정) |
| **실내** | 텍스처 필요 | 항상 OK |
| **실외** | 햇빛 OK | 비/눈 약함 |
| **가격** | 저렴 | 비쌈 |
| **전력** | 낮음 | 높음 |

**결론:**
```
VIO: 드론, 핸드헬드, AR/VR
LiDAR: 자율주행, 대형 로봇, 건설
```

---

## 💡 핵심 개념 정리

### K-D Tree

**목적:** 빠른 최근접 이웃 검색

**복잡도:**
```
구축: O(n log n)
검색: O(log n) (평균)

vs Naive (전체 탐색):
검색: O(n)

→ 10,000개 점일 때:
   Naive: 10,000번 비교
   K-D Tree: ~13번 비교!
```

### Voxel Grid Filter

**목적:** Point Cloud 간소화

**방법:**
```
1. 3D 공간을 격자로 나눔 (Voxel)
   예: 5cm × 5cm × 5cm

2. 각 Voxel에서 대표점 1개만 선택
   (중심점 or 평균)

100,000개 → 10,000개 (10배 감소)
```

---

## 🔍 자체 점검

### 질문 1: ICP 정의
**Q:** ICP가 푸는 문제는?

**A:**
```
입력: 두 Point Cloud (P, Q)
출력: 변환 [R|t]
목표: Σ ||q_i - (R·p_i + t)||² 최소화

방법: 반복적으로
  1. 대응 찾기
  2. R, t 계산
  3. 수렴까지 반복
```

### 질문 2: Point-to-Point vs Point-to-Plane
**Q:** 어떤 게 더 빠르게 수렴하나요?

**A:**
```
Point-to-Plane이 훨씬 빠름!

이유:
  - 평면 제약 활용
  - 5회 이내 수렴 (vs 20회)

단, 법선 계산 필요
```

### 질문 3: VIO vs LiDAR
**Q:** VIO에서 ICP를 안 쓰는 이유?

**A:**
```
VIO (카메라):
  - 2D 이미지 입력
  - 3D Point Cloud 없음
  - → ICP 불가능!
  
대신:
  - Week 2: 2D-2D (Essential)
  - Week 3: 3D-2D (PnP) ⭐

ICP가 필요한 경우:
  - RGB-D 카메라
  - LiDAR
  - Stereo (dense depth)
```

### 질문 4: 초기값
**Q:** ICP에서 초기값이 왜 중요한가요?

**A:**
```
Local Minimum 문제!

초기 위치가 멀면:
  → 잘못된 대응
  → 발산 or 잘못된 해

해결:
  - IMU로 예측
  - 이전 프레임 포즈
  - Multi-scale ICP
```

---

## 📝 이번 주 실습 & 다음 주 준비

### 이번 주 실습

`basic.cpp` 실행:
```bash
cd week4
mkdir build && cd build
cmake ..
make
./basic
```

**체크리스트:**
- [ ] SVD 기반 정합 확인
- [ ] ICP 반복 과정 관찰
- [ ] 수렴 속도 확인 (10-20회)
- [ ] 오차 감소 추이 확인

### 예상 결과

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
ICP 반복
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Iteration 0:
   Error: 12.35

Iteration 1:
   Error: 3.45 (72% improvement)

Iteration 2:
   Error: 0.87 (75% improvement)

Iteration 3:
   Error: 0.21 (76% improvement)

✅ Converged at iteration 3

최종 변환:
  R = [[0.998, -0.052, 0.000],
       [0.052,  0.998, 0.000],
       [0.000,  0.000, 1.000]]
  t = [0.45, 0.12, 0.03]

Translation 오차: 0.0023
```

### 다음 주 준비

**Week 5: Mini VO 프로젝트** 🎉

드디어!
```
Week 2 (Essential) + Week 3 (PnP)
          ↓
    완전한 VO 시스템!

구현:
  - 초기화 (2D-2D)
  - 추적 (3D-2D)
  - 맵 관리
  - 드리프트 관찰

테스트:
  - KITTI or EuRoC 데이터셋
  - 실제 궤적 비교
```

---

## 🎯 이번 주 핵심 요약

1. **ICP = Point Cloud 정합**
   - 두 3D 점 구름 맞추기
   - 반복적으로 대응 찾고 변환 계산

2. **4단계 반복**
   - 대응 → 정렬 → 변환 → 수렴

3. **SVD로 Closed-form**
   - 대응점 주어지면 한 번에 R, t 계산
   - Point-to-Point 방법

4. **Point-to-Plane이 더 빠름**
   - 평면 제약 활용
   - 5회 이내 수렴

5. **VIO에서는 참고용**
   - RGB-D/LiDAR SLAM에서 핵심
   - AMR의 LiDAR 기반 위치 추정

---

다음: [Week 5 - Mini VO 프로젝트](../week5/README.md)
