# Week 4: 특징점 매칭 (Feature Matching)

## 📌 개요

> 🎯 **목표**: 서로 다른 이미지에서 같은 점 찾기
> ⏱️ **예상 시간**: 이론 2시간 + 실습 3시간

**특징점 매칭**은 두 이미지에서 같은 3D 점에 해당하는 특징점 쌍을 찾는 과정입니다. 이를 통해 카메라 움직임을 추정하거나 3D 재구성을 할 수 있습니다.

### 🤔 왜 이걸 배워야 할까요?

**일상 비유**: 두 장의 사진에서 같은 건물 찾기

```
사진 1                    사진 2
┌───────────────┐        ┌───────────────┐
│    🏛️         │        │         🏛️    │
│  에펠탑         │   →    │      에펠탑    │
│               │        │               │
└───────────────┘        └───────────────┘

같은 건물이지만 위치/크기가 다름!
→ "어떤 점이 같은 점인가?" 매칭 필요
```

**SLAM에서의 활용**:
- **루프 클로저**: 이전에 방문한 곳 인식
- **재위치화**: 추적 실패 시 복구
- **맵 초기화**: 첫 두 프레임에서 초기 맵 생성

---

## 🚀 먼저 돌려보기 (원칙 1: 역순 학습)

> **이론을 읽기 전에 먼저 코드를 돌려보세요!**

```bash
cd week4 && mkdir build && cd build
cmake .. && make
./basic
```

실행 결과를 관찰한 후, 아래 이론을 읽으며 "아, 이게 이 뜻이었구나" 하고 채워갑니다.

- 📂 [PRACTICE.md](./PRACTICE.md) - 실습 가이드
- 📝 `quiz_easy.cpp` → `quiz_medium.cpp` 순서로 풀기

---

## 📖 핵심 개념

### 1. 매칭 기본 원리

#### 디스크립터 비교

```
이미지 1 특징점 A → 디스크립터 [0.2, 0.8, 0.1, ...]
                      ↓  거리 비교
이미지 2 특징점 1 → [0.1, 0.9, 0.2, ...]  거리 = 0.15 ✅ 가장 가까움
이미지 2 특징점 2 → [0.8, 0.2, 0.7, ...]  거리 = 0.92
이미지 2 특징점 3 → [0.3, 0.7, 0.0, ...]  거리 = 0.22

→ A와 1이 매칭!
```

#### 거리 함수

| 디스크립터 타입 | 거리 함수 | 예시 알고리즘 |
|---------------|----------|--------------|
| 이진 (Binary) | **해밍 거리** (XOR) | ORB, BRIEF |
| 실수 (Float) | **유클리드 거리** | SIFT, SURF |

**해밍 거리** (이진 디스크립터):
```
A = 10110100
B = 10010101
    --------
XOR = 00100001  → 다른 비트 수 = 2
```

---

### 2. Brute-Force 매칭

#### 원리

모든 쌍을 비교하여 가장 가까운 것 선택

```
이미지 1: N개 특징점
이미지 2: M개 특징점

비교 횟수 = N × M  (모든 쌍)
```

```python
# OpenCV Brute-Force Matcher
import cv2

# ORB 디스크립터용 (이진 → 해밍)
bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)
matches = bf.match(desc1, desc2)

# SIFT 디스크립터용 (실수 → L2)
bf = cv2.BFMatcher(cv2.NORM_L2, crossCheck=False)
matches = bf.knnMatch(desc1, desc2, k=2)
```

#### Cross-check

양방향 확인으로 잘못된 매칭 제거:

```
A → B (A에서 B가 가장 가까움)
B → A (B에서 A가 가장 가까움)

둘 다 만족해야 매칭 인정!
```

```python
bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)
```

---

### 3. FLANN 매칭

#### 왜 FLANN인가?

**Fast Library for Approximate Nearest Neighbors**

| 방식 | 속도 | 정확도 |
|------|------|--------|
| Brute-Force | O(N×M) 느림 | 100% 정확 |
| **FLANN** | **O(N×log M)** 빠름 | 약간 손실 (99%+) |

대규모 특징점에서 효율적!

```python
# FLANN 매처 설정
FLANN_INDEX_LSH = 6  # 이진 디스크립터용

index_params = dict(algorithm=FLANN_INDEX_LSH,
                    table_number=6,
                    key_size=12,
                    multi_probe_level=1)

search_params = dict(checks=50)

flann = cv2.FlannBasedMatcher(index_params, search_params)
matches = flann.knnMatch(desc1, desc2, k=2)
```

---

### 4. Lowe's Ratio Test

#### 문제: Outlier 매칭

```
특징점 A의 매칭 후보:
  1번 점: 거리 = 0.45  (최선)
  2번 점: 거리 = 0.48  (차선)
  
→ 거리가 비슷함 = 확신 없음 = 신뢰 불가!
```

#### 해결: Ratio Test

```
ratio = 최선 거리 / 차선 거리

ratio < 0.75  →  매칭 수락
ratio >= 0.75 →  매칭 거부 (모호함)
```

**직관**: 가장 가까운 점이 두 번째보다 **확연히** 가까워야 신뢰

```python
# KNN으로 2개 후보 찾기
matches = bf.knnMatch(desc1, desc2, k=2)

# Ratio Test 적용
good_matches = []
for m, n in matches:
    if m.distance < 0.75 * n.distance:
        good_matches.append(m)
```

---

### 5. RANSAC (Random Sample Consensus)

#### 문제: Outlier가 여전히 존재

Ratio Test 후에도 잘못된 매칭(outlier)이 남음

```
매칭 결과:
  ✅ 정상 매칭 (inlier) - 80%
  ❌ 잘못된 매칭 (outlier) - 20%
```

#### RANSAC 원리

```
반복:
  1. 랜덤하게 최소 샘플 선택 (예: 4쌍)
  2. 모델 추정 (예: Homography)
  3. 모든 점에 모델 적용
  4. inlier 개수 세기 (모델에 맞는 점)
  
최고의 모델 선택 (가장 많은 inlier)
```

```
┌──────────────────────────────────────┐
│                                      │
│    ●───●    ●───●                    │
│      ╲        ╲      ← inlier        │
│       ●───●    ●───●                 │
│                                      │
│           ✕                          │
│            ╲     ← outlier           │
│             ✕                        │
│                                      │
└──────────────────────────────────────┘
```

#### OpenCV 사용

```python
# Fundamental Matrix 추정 + RANSAC
F, mask = cv2.findFundamentalMat(
    pts1, pts2, 
    cv2.FM_RANSAC, 
    ransacReprojThreshold=3.0
)

# mask[i] = 1: inlier, 0: outlier
inlier_matches = [m for m, ok in zip(matches, mask.ravel()) if ok]
```

#### RANSAC 파라미터

| 파라미터 | 의미 | 권장값 |
|---------|------|--------|
| `ransacReprojThreshold` | inlier 판단 임계값 (픽셀) | 1.0 ~ 3.0 |
| `confidence` | 성공 확률 | 0.99 |
| `maxIters` | 최대 반복 횟수 | 1000~5000 |

---

### 6. 매칭 파이프라인 정리

```
이미지 1                           이미지 2
    │                                 │
    ▼                                 ▼
특징점 검출 (ORB)                특징점 검출 (ORB)
    │                                 │
    ▼                                 ▼
디스크립터 계산                  디스크립터 계산
    │                                 │
    └─────────┬───────────────────────┘
              ▼
      Brute-Force / FLANN 매칭
              │
              ▼
      Lowe's Ratio Test (0.75)
              │
              ▼
      RANSAC (Outlier 제거)
              │
              ▼
        최종 매칭 결과
```

---

### 7. SLAM에서의 활용

#### ORB-SLAM3 매칭

```
새 프레임
    │
    ▼
ORB 검출 + 디스크립터
    │
    ├──▶ 로컬 맵 매칭 (추적)
    │      - 이전 프레임 특징점과 매칭
    │      - 빠른 검색 (projection 기반)
    │
    └──▶ 루프 클로저 검색
           - 전체 맵과 매칭
           - Bag of Words 사용
           - RANSAC 검증
```

#### VINS-Fusion 차이점

VINS는 매칭 대신 **추적(Tracking)**:
- Lucas-Kanade Optical Flow
- 디스크립터 계산 안 함
- 더 빠름, 연속 프레임에 적합
- Week 8에서 자세히!

---

## 💻 실습 파일

| 파일 | 내용 | 난이도 |
|------|------|--------|
| `feature_matching_basics.py` | BF, Ratio Test 구현 | ⭐⭐ |
| `matching_quiz.py` | RANSAC, 성능 비교 | ⭐⭐⭐ |

---

## 📊 핵심 정리

### 매칭 방법 비교

| 방법 | 속도 | 정확도 | 용도 |
|------|------|--------|------|
| Brute-Force | 느림 | 정확 | 소규모 |
| FLANN | 빠름 | 99%+ | 대규모 |
| + Ratio Test | - | ↑ | 모호함 제거 |
| + RANSAC | - | ↑↑ | outlier 제거 |

### 필터링 단계

```
원본 매칭     Ratio Test     RANSAC
 1000개   →    600개    →    450개 (inlier)
                   ↓              ↓
              모호함 제거    기하학적 검증
```

### 권장 설정

```python
# 표준 매칭 파이프라인
orb = cv2.ORB_create(nfeatures=1000)
bf = cv2.BFMatcher(cv2.NORM_HAMMING)

kp1, desc1 = orb.detectAndCompute(img1, None)
kp2, desc2 = orb.detectAndCompute(img2, None)

# KNN + Ratio Test
matches = bf.knnMatch(desc1, desc2, k=2)
good = [m for m, n in matches if m.distance < 0.75 * n.distance]

# RANSAC
pts1 = np.float32([kp1[m.queryIdx].pt for m in good])
pts2 = np.float32([kp2[m.trainIdx].pt for m in good])
F, mask = cv2.findFundamentalMat(pts1, pts2, cv2.FM_RANSAC)
```

---

## ✅ 학습 완료 체크리스트

### 기초 이해 (필수)
- [ ] 해밍 거리 vs 유클리드 거리 설명 가능
- [ ] Ratio Test 원리 설명 가능
- [ ] RANSAC이 왜 필요한지 설명 가능

### 실용 활용 (권장)
- [ ] OpenCV로 ORB 매칭 구현 가능
- [ ] Ratio Test + RANSAC 적용 가능
- [ ] 매칭 결과 시각화 가능

### 심화 (선택)
- [ ] FLANN 파라미터 튜닝 가능
- [ ] Homography vs Fundamental Matrix 차이 이해
- [ ] ORB-SLAM3 매칭 코드 흐름 이해

---

## 🔗 다음 단계

### Week 5: 에피폴라 기하학

매칭된 점들로 할 수 있는 것:
- Essential Matrix 계산
- Fundamental Matrix 계산
- 카메라 포즈(R, t) 추정

---

## 📚 참고 자료

- OpenCV Feature Matching Tutorial
- RANSAC 알고리즘 논문
- ORB-SLAM3 Tracking 코드

---

## ❓ FAQ

**Q1: Ratio Test의 0.75는 어디서 나온 값인가요?**
A: Lowe의 SIFT 논문에서 제안. 0.7~0.8이 일반적으로 좋은 결과.

**Q2: RANSAC 반복 횟수는 어떻게 정하나요?**
A: 이론적으로 `log(1-confidence) / log(1-inlier_ratio^sample_size)`로 계산.

**Q3: 매칭이 너무 적으면?**
A: 특징점 수 늘리기, threshold 낮추기, Ratio 값 높이기(0.8).

**Q4: Cross-check vs Ratio Test?**
A: Cross-check은 양방향 확인, Ratio Test는 모호함 검사. 둘 다 쓸 수 있지만 보통 Ratio Test가 더 효과적.

---

**🎯 Week 4 핵심 메시지:**

> 매칭 = 검출 + 비교 + 필터링
> 
> **Ratio Test** + **RANSAC** = 신뢰할 수 있는 매칭!
