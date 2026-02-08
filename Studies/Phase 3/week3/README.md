# Week 3: 3D-2D 모션 추정 - 드디어 스케일 복원!

> 🎯 **이번 주 목표**: 3D 정보를 활용해서 절대 스케일로 카메라 위치 추정하기  
> ⏰ **예상 시간**: 10시간  
> 💡 **핵심 질문**: "이미 알고 있는 3D 점들을 어떻게 활용할까?"

---

## 🌟 시작하기 전에

### Week 2와 Week 3의 가장 큰 차이

**Week 2 (2D-2D):**
```
입력: 2D 점 + 2D 점
출력: R, t (단, ||t|| = 1로 정규화)
문제: 실제로 몇 m 이동했는지 모름! 😢
```

**Week 3 (3D-2D):**
```
입력: 3D 점 + 2D 점
출력: R, t (실제 크기!)
해결: 절대 스케일 복원! 🎉
```

**비유로 이해:**
```
Week 2: "어느 방향으로 갔는지는 아는데, 1m인지 10m인지..."
Week 3: "3D 점을 알고 있으니까 정확히 2.5m 갔네!"
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. 3D-2D 모션 추정이 뭐죠?

**상황:**
```
내가 가진 것:
  - 3D 맵 (Week 2에서 만듦): "책상은 (1.0, 0.5, 5.0)에 있어"
  - 현재 이미지: "책상이 픽셀 (320, 240)에 보여"

질문: "내 카메라가 지금 어디 있지?"
```

**PnP (Perspective-n-Point)의 역할:**
```
알고 있는 3D 위치 + 현재 2D 관측
           ↓
      카메라 포즈 계산!
```

**언제 사용하나요?**
- ✅ **VO 추적** - 매 프레임마다! (VO의 핵심!)
- ✅ **Relocalization** - "길 잃었다가 다시 찾기"
- ✅ **Loop Closure** - "전에 왔던 곳 인식"

---

### 2. PnP 문제 정의 - 천천히 이해하기

#### 수학적 정의

**주어진 것:**
```
n개의 대응:
  {(X₁, x₁), (X₂, x₂), ..., (Xₙ, xₙ)}
  
여기서:
  Xᵢ = 3D 점 (월드 좌표계)
       예: (1.5, 0.8, 5.2) 미터
       
  xᵢ = 2D 관측 (이미지 좌표)
       예: (320, 240) 픽셀
```

**구하려는 것:**
```
[R|t]: 카메라 포즈
  R = 3×3 회전 행렬
  t = 3×1 이동 벡터
```

**관계식 (투영 방정식):**
```
λ · xᵢ = K · [R|t] · Xᵢ

풀어쓰면:
  λ · [u]   [fx  0  cx]   [R  t]   [X]
      [v] = [ 0 fy  cy] · [0  1] · [Y]
      [1]   [ 0  0   1]            [Z]
                                   [1]
```

#### 비유로 이해하기

**GPS 위성 예시:**
```
위성 위치 (3D): 알고 있음
위성 방향 (2D): 관측됨

→ 내 위치 계산 가능! (이게 PnP와 같은 원리)
```

**AR 앱 예시:**
```
QR코드 모서리 (3D): (-5cm, -5cm, 0), (5cm, -5cm, 0), ...
카메라에서 본 모서리 (2D): (100, 150), (500, 150), ...

→ 휴대폰 위치와 방향 계산!
```

---

### 3. PnP 알고리즘들 - 어떤 걸 쓸까?

#### P3P (최소 해법)

**최소 3개 점**으로 풀 수 있음!

**장점:**
```
✅ 최소 점 개수 (RANSAC에 최적)
✅ 빠름
```

**단점:**
```
❌ 4개의 해 존재 → 모호성
❌ 점 배치에 민감 (일직선 배치 시 불안정)
❌ 노이즈에 약함
```

**언제 쓰나요?**
- RANSAC의 minimal solver로

#### EPnP (Efficient PnP) ⭐

**n개 점을 O(n) 시간**에 처리!

**핵심 아이디어:**
```
모든 점을 4개의 "가상 제어점"으로 표현

예: 100개 점이 있어도
    → 4개 제어점만 풀면 끝!
```

**장점:**
```
✅ 빠름 (O(n) - 선형 시간)
✅ 안정적
✅ 점이 많을수록 더 정확
✅ OpenCV 기본 알고리즘
```

**언제 쓰나요?**
- 일반적인 VO 추적
- 보통 50~200개 점 사용

**OpenCV 코드:**
```cpp
cv::solvePnP(
    points3d,          // 3D 점들
    points2d,          // 2D 관측
    K,                 // 카메라 행렬
    cv::Mat(),         // 왜곡 계수 (없음)
    rvec, tvec,        // 출력: 회전, 이동
    false,             // useExtrinsicGuess
    cv::SOLVEPNP_EPNP  // EPnP 사용
);
```

#### Iterative PnP (반복 최적화)

**EPnP 결과를 더 정밀하게!**

**원리:**
```
1. EPnP로 초기 해 구함
2. 재투영 오차 최소화 (Levenberg-Marquardt)
3. 수렴할 때까지 반복
```

**장점:**
```
✅ 가장 정확
✅ Sub-pixel 정밀도
```

**단점:**
```
❌ 느림
❌ 초기값 필요
```

**OpenCV 코드:**
```cpp
cv::solvePnPRefineLM(
    points3d, points2d, K, cv::Mat(),
    rvec, tvec  // EPnP 결과로 초기화
);
```

---

### 4. RANSAC + PnP - Outlier 제거하기

#### 왜 RANSAC이 필요한가요?

**문제 상황:**
```
200개 매칭:
  ✅ 정확: 180개
  ❌ 틀림: 20개 (잘못된 매칭 - Outlier)

Outlier가 하나만 있어도:
  → 포즈 추정이 크게 틀어짐! 😱
```

#### RANSAC 작동 원리

```
┌─────────────────────────────────────┐
│ RANSAC-PnP (100회 반복)               │
├─────────────────────────────────────┤
│ 반복 i:                              │
│   1. 무작위로 4개 점 선택               │
│   2. P3P로 포즈 계산                   │
│   3. 전체 200개 점 검증:               │
│      재투영 오차 < 8픽셀? → Inlier      │
│   4. Inlier 개수 기록                 │
└─────────────────────────────────────┘
              ↓
     가장 많은 Inlier를 가진 포즈 선택!
              ↓
     그 Inlier들만으로 EPnP 재계산
```

**OpenCV 코드:**
```cpp
std::vector<int> inliers;
cv::solvePnPRansac(
    points3d,      // 3D 점들
    points2d,      // 2D 관측
    K,             // 카메라 행렬
    cv::Mat(),     // 왜곡 없음
    rvec, tvec,    // 출력: 포즈
    false,         // useExtrinsicGuess
    100,           // 반복 횟수
    8.0,           // 재투영 오차 임계값 (픽셀)
    0.99,          // 신뢰도 99%
    inliers        // 출력: inlier 인덱스
);

// 결과 확인
double inlier_ratio = inliers.size() / (double)points3d.size();
std::cout << "Inlier ratio: " << inlier_ratio * 100 << "%" << std::endl;
```

---

### 5. VO에서 PnP의 역할 - 가장 중요!

#### VO 전체 파이프라인

```
┌────────────┐
│ Frame 0, 1 │ Week 2: 초기화
│  (2D-2D)   │ Essential Matrix
└─────┬──────┘ → 초기 3D 맵 생성
      │
      ↓
┌────────────┐
│ Frame 2    │ Week 3: 추적 시작!
│  (3D-2D)   │ PnP로 포즈 추정
└─────┬──────┘
      │
      ↓
┌────────────┐
│ Frame 3    │ 계속 PnP 추적
│  (3D-2D)   │
└─────┬──────┘
      │
      ⋮
```

#### 매 프레임 추적 과정

```
Frame t-1 (이전)          Frame t (현재)
─────────────            ──────────────
3D 맵 포인트              새 이미지
{X₁, X₂, ...}            특징점 검출
      │                       │
      │                  특징점 추적
      │                  (Optical Flow)
      │                       │
      └───────→ 대응 ─────────┘
               {(X₁,x₁), (X₂,x₂), ...}
                       ↓
                  PnP + RANSAC
                       ↓
                   [R|t]_t
                       ↓
            Frame t의 포즈 추정 완료!
```

#### VINS-Mono 실제 코드 흐름

```cpp
void Estimator::processImage() {
    // 1. 특징점 추적
    trackFeatures();  // Optical Flow
    
    // 2. Initial PnP (프론트엔드)
    if (frame_count == WINDOW_SIZE) {
        // 3D 점 + 2D 관측으로 초기 포즈 추정
        bool result = solvePnP();
    }
    
    // 3. 백엔드 최적화
    optimization();  // Bundle Adjustment
}
```

---

### 6. 스케일 복원 - Week 3의 하이라이트!

#### 왜 스케일이 복원되나요?

**Week 2 복습:**
```
2D-2D (Essential Matrix):
  E = t^ × R
  E = (α·t)^ × R  ← 똑같음!
  
  → t의 크기를 알 수 없음
  → ||t|| = 1로 정규화
```

**Week 3의 마법:**
```
3D-2D (PnP):
  3D 점: X = (1.5, 0.8, 5.2) m  ← 절대 크기!
  
  투영: x = K · [R|t] · X
  
  → X의 절대 크기를 알고 있으므로
  → t의 절대 크기도 계산 가능! ✨
```

#### 구체적인 예시

**Week 2 초기화:**
```
Frame 0 → Frame 1

Essential Matrix 추정:
  R = (5도 회전)
  t = [1.0, 0.0, 0.0]  ← 정규화됨!

초기 3D 맵 생성:
  Point A: (2.0, 1.0, 8.0)  ← 단위는 임의
  Point B: (3.0, 1.5, 7.5)
  Point C: (1.5, 0.5, 9.0)
```

**Week 3 추적:**
```
Frame 2

PnP 입력:
  3D 점: Point A, B, C (Week 2에서 만든 것)
  2D 관측: A→(310,235), B→(405,248), C→(275,220)

PnP 출력:
  R = (3도 회전)
  t = [2.1, 0.1, 0.05]  ← 정규화 안 됨!
  
✅ 스케일 유지됨!
   Frame 0→1과 같은 단위계
```

**왜 유지되나요?**
```
3D 맵의 스케일 = Week 2에서 정한 스케일
→ 그 맵을 사용하는 PnP
→ 같은 스케일로 t 계산됨

단, 절대 크기는 여전히 모름
(Week 2에서 임의로 정했으니까)
```

---

### 7. 재투영 오차 - 포즈의 정확도 측정

#### 재투영 오차가 뭐죠?

**원리:**
```
1. 추정한 포즈 [R|t]로 3D 점을 이미지에 투영
2. 실제 관측된 위치와 비교
3. 차이 = 재투영 오차
```

**수식:**
```
재투영 오차 = ||xᵢ - π(K, R, t, Xᵢ)||²

여기서:
  xᵢ: 실제 관측된 2D 점
  π(K, R, t, Xᵢ): 3D 점을 투영한 2D 위치
```

**시각화:**
```
이미지:
  📍 실제 관측 (320, 240)
  📍 투영 결과 (322, 239)
  
  거리 = √[(322-320)² + (239-240)²]
       = √[4 + 1]
       = 2.24 픽셀  ← 재투영 오차
```

#### 얼마나돼야 좋나요?

```
재투영 오차:
  < 1 픽셀:  훌륭! 😊
  1~3 픽셀:  양호
  3~5 픽셀:  보통
  > 5 픽셀:  문제 있음 😢
```

**OpenCV 코드:**
```cpp
// 재투영
std::vector<cv::Point2f> projected;
cv::projectPoints(points3d, rvec, tvec, K, cv::Mat(), projected);

// 오차 계산
double sum_error = 0.0;
for (int i = 0; i < points2d.size(); i++) {
    double dx = projected[i].x - points2d[i].x;
    double dy = projected[i].y - points2d[i].y;
    sum_error += std::sqrt(dx*dx + dy*dy);
}
double rms_error = sum_error / points2d.size();

std::cout << "RMS 재투영 오차: " << rms_error << " px" << std::endl;
```

---

## 💡 핵심 개념 정리

### Minimal Solver

**비교:**
```
2D-2D (Essential):
  최소 5개 점 필요
  (자유도 5 = 회전3 + 방향2)

3D-2D (PnP):
  최소 3개 점 필요 (P3P)
  (자유도 6 = 회전3 + 이동3)
  
→ PnP가 더 적은 점으로 가능!
```

### Inlier Ratio - VO 품질 지표

**정의:**
```
Inlier Ratio = (Inlier 개수) / (전체 매칭 개수)
```

**해석:**
```
> 80%:  매우 좋음! 안정적인 추적
60~80%: 양호
40~60%: 보통 (주의)
< 30%:  위험! 추적 실패 가능성
          → 재초기화 필요
```

**코드 예시:**
```cpp
if (inlier_ratio < 0.3) {
    std::cout << "⚠️ 추적 실패!" << std::endl;
    // 재초기화 (2D-2D로 다시 시작)
    reinitialize();
}
```

### Covisibility - 공동 관측

**개념:**
```
Covisibility = 같은 3D 점을 보는 프레임들

예:
  Frame 5: Point A, B, C 관측
  Frame 6: Point B, C, D 관측
  Frame 7: Point C, D, E 관측
  
→ Frame 5-6-7은 Point C를 공동 관측
```

**왜 중요한가요?**
```
VO/SLAM에서:
  - PnP에 사용할 3D 점 선택
  - Local Bundle Adjustment 범위
  - Loop Closure 후보 검출
```

---

## 🔍 자체 점검 - 이해했는지 확인!

### 질문 1: 스케일 복원
**Q:** PnP가 스케일을 복원하는 이유는?

**A:**
```
3D 점의 절대 크기를 알고 있기 때문!

예: X = (1.5, 0.8, 5.2) ← 단위는 Week 2에서 정함

투영: λ·x = K·[R|t]·X
→ X를 알면 [R|t]의 절대 크기도 계산 가능

vs Week 2:
  E = (α·t)^ × R  ← α를 결정할 방법이 없음
```

### 질문 2: P3P vs EPnP
**Q:** P3P와 EPnP 중 언제 뭘 쓰나요?

**A:**
```
P3P (최소 3점):
  - RANSAC의 minimal solver로
  - 빠르지만 불안정
  - 4개 해 존재

EPnP (n점, O(n)):
  - 일반적인 추적용
  - 안정적
  - 점이 많을수록 정확
  - OpenCV 기본
  
실전: RANSAC에서 P3P 반복 → 최종적으로 EPnP
```

### 질문 3: VO 역할
**Q:** VO에서 PnP의 역할은?

**A:**
```
★ 매 프레임 포즈 추정 (추적) ★

Week 2 (Essential): 초기화 (처음 2 프레임)
Week 3 (PnP): 추적 (나머지 모든 프레임)

매 프레임:
  1. 특징점 추적 (Optical Flow)
  2. 3D-2D 대응 구성
  3. PnP로 포즈 추정
  4. 반복!
```

### 질문 4: Inlier Ratio
**Q:** Inlier ratio가 25%면 어떻게 해야 하나요?

**A:**
```
25% < 30% → 추적 실패!

조치:
  1. 재초기화 시도
     - 2D-2D (Essential)로 다시 시작
     
  2. Relocalization
     - 이전 키프레임과 매칭
     
  3. IMU 활용 (VIO)
     - IMU로 대략적인 포즈 예측
     - 다음 프레임에서 복구 시도
```

---

## 📝 이번 주 실습 & 다음 주 준비

### 이번 주 실습

`basic.cpp` 실행:
```bash
cd week3
mkdir build && cd build
cmake ..
make
./basic
```

**체크리스트:**
- [ ] PnP 포즈 추정 확인
- [ ] 재투영 오차 확인 (< 1픽셀)
- [ ] Inlier ratio 확인 (> 70%)
- [ ] 연속 프레임 추적 시뮬레이션
- [ ] **스케일 유지 확인!** ⭐

### 예상 결과

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
VO 추적 시뮬레이션
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

초기 맵: 100개 3D 점

Frame 0:
   Inliers: 92/100 (92.0%)
   Reproj error: 0.58 px
   Est t: [0.098, 0.021, 0.012]

Frame 1:
   Inliers: 89/100 (89.0%)
   Reproj error: 0.64 px
   Est t: [0.201, 0.019, 0.023]

Frame 2:
   Inliers: 91/100 (91.0%)
   Reproj error: 0.61 px
   Est t: [0.305, 0.024, 0.028]

✅ 스케일 일관성 유지!
```

### 다음 주 준비

다음: [Week 4 - 3D-3D 모션 추정 (ICP)](../week4/README.md)

---

## 🎯 이번 주 핵심 요약

1. **PnP = 3D→2D 대응으로 포즈 추정**
   - 3D 점 위치 알고 있음
   - 2D 관측으로 카메라 위치 계산

2. **EPnP가 일반적**
   - O(n) 빠른 속도
   - 안정적
   - OpenCV 기본

3. **RANSAC 필수**
   - Outlier 제거
   - Inlier ratio 70% 이상 목표

4. **VO의 핵심 = PnP 추적**
   - Week 2: 초기화 (Essential)
   - Week 3: 추적 (PnP) ← 매 프레임!

5. **스케일 복원! 🎉**
   - 3D 점 크기를 아니까
   - t의 절대 크기 계산 가능
   - Week 2의 스케일 유지

---

**거의 다 왔습니다!** Week 4는 참고용이고, Week 5에서 드디어 Week 2+3를 합쳐서 완전한 VO를 만듭니다! 🚀

다음: [Week 4 - 3D-3D 모션 추정 (ICP)](../week4/README.md)
