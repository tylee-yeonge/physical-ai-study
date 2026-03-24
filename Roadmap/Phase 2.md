# Phase 2: 컴퓨터 비전 기초

> ⏰ **기간**: 2개월  
> 🎯 **목표**: Visual SLAM의 프론트엔드 이해를 위한 비전 기초  
> ⏱️ **주간 시간**: 약 7시간

---

## 📋 Section 2.1: 카메라 모델 (2주)

### Week 1: 핀홀 카메라 모델 ✅ (2/2 ~ 2/12)

#### 기본 개념
- [x] 핀홀 카메라 원리 (빛이 작은 구멍 통과)
- [x] 3D → 2D 투영 과정 이해
- [x] 초점 거리 (Focal Length) 개념
- [x] 주점 (Principal Point) 개념

#### 내부 파라미터 (Intrinsic)
- [x] 카메라 내부 행렬 K 구조 이해
  ```
  K = [fx  0  cx]
      [0  fy  cy]
      [0   0   1]
  ```
- [x] fx, fy가 왜 다를 수 있는지 (픽셀 비정방형)
- [x] cx, cy가 이미지 중심이 아닐 수 있는 이유

#### 외부 파라미터 (Extrinsic)
- [x] 월드 좌표 → 카메라 좌표 변환
- [x] 회전 행렬 R과 평행이동 벡터 t
- [x] [R|t] 행렬의 의미

#### SLAM에서 어디에 쓰이나?
- [x] VINS config 파일의 카메라 파라미터가 이것
- [x] 3D 점을 이미지에 투영할 때 K 사용
- [x] 재투영 오차 계산의 기초

### Week 2: 왜곡과 캘리브레이션 ✅ (2/12 ~ 3/2)

> 💻 **C++ 실습**: [PRACTICE.md](../Studies/Phase%202/week2/PRACTICE.md)  
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo Camera  
> ⏰ **실습 시간**: 6-8시간

#### 렌즈 왜곡 (이론)
- [x] 방사 왜곡 (Radial Distortion) - 배럴/핀쿠션
- [x] 접선 왜곡 (Tangential Distortion)
- [x] 왜곡 계수 (k1, k2, p1, p2, k3)
- [x] Fisheye 왜곡 모델 (선택 - VINS는 지원)

#### 카메라 캘리브레이션 개념
- [x] 체커보드 패턴 사용 이유
- [x] 캘리브레이션 과정 이해
- [x] 재투영 오차 (Reprojection Error) 의미

#### 실습: OpenCV C++ 캘리브레이션 ⭐ (→ 나중에 수행)
- [ ] **단일 카메라**: Left/Right 각각 캘리브레이션
  - [ ] `CameraCalibration` 클래스 구현
  - [ ] 체커보드 이미지 20장 캡처
  - [ ] RMS < 0.5 픽셀 달성
  - [ ] `camera_{left|right}_calib.yaml` 저장
- [ ] **스테레오 캘리브레이션**: R, T (baseline) 측정
  - [ ] 동시 촬영 이미지로 스테레오 캘리브레이션
  - [ ] `stereo_calib.yaml` 저장
- [ ] **결과 분석**: fx, fy, cx, cy 값 검증

#### Kalibr 캘리브레이션 (선택 - Phase 4 준비, → 나중에 수행)
- [ ] Kalibr 설치 (Docker 권장)
- [ ] AprilGrid 타겟 준비
- [ ] Camera-only 캘리브레이션 실행
- [ ] 결과 YAML 파일 분석

> 💡 Kalibr는 Phase 4에서 Camera-IMU 캘리브레이션에도 사용됨. 미리 익혀두면 좋음.

#### Stereo 카메라 특화 (→ 나중에 수행)
- [ ] ✅ Stereo의 장점 (스케일 복원 가능 - Monocular 대비)
- [ ] Baseline 개념 및 측정
- [ ] VINS-Fusion Stereo 모드 config 파일 확인

### 🔍 Section 2.1 자체 점검
> 아래 질문에 답할 수 있으면 다음으로 진행

1. 카메라 행렬 K의 각 요소 (fx, fy, cx, cy)는 무엇을 의미하는가?
2. 방사 왜곡과 접선 왜곡의 차이는?
3. 캘리브레이션 결과가 정확한지 어떻게 확인하는가?

---

## 📋 Section 2.2: 특징점 검출과 매칭 (2주)

### Week 3-4: 특징점 검출과 매칭 ✅

> 💻 **C++ 실습**: [PRACTICE.md](../Studies/Phase%202/week3/PRACTICE.md)  
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo Camera  
> ⏰ **실습 시간**: 12-16시간 (2주)

#### Week 3: 코너 검출
- [x] Harris Corner Detector 원리 이해
- [x] 코너 응답 함수
- [x] Non-maximum suppression

#### FAST 코너
- [x] FAST 알고리즘 원리 (16개 픽셀 원에서 연속 밝기 비교)
- [x] 임계값 설정
- [x] 왜 "Fast"인지 — 속도 이점

#### 디스크립터
- [x] 왜 디스크립터가 필요한지 (매칭을 위해)
- [x] BRIEF 개념 (이진 디스크립터)
- [x] ORB = oriented FAST + rotated BRIEF
- [x] 디스크립터 벡터 크기와 의미

#### SLAM에서 어디에 쓰이나?
- [x] VINS `feature_tracker`는 FAST 코너 + KLT 추적 사용
- [x] ORB-SLAM은 ORB 디스크립터 사용
- [x] 검출 속도가 실시간 성능에 직접 영향

#### 실습
- [x] OpenCV로 FAST, ORB 검출 및 시각화
- [x] 파라미터 (임계값, 최대 개수) 변경에 따른 결과 차이 관찰
- [x] 검출 시간 측정

#### Week 4: 특징점 매칭

#### 매칭 알고리즘
- [x] Brute-Force 매칭
- [x] FLANN 매칭 (빠른 근사)
- [x] 해밍 거리 (이진 디스크립터용) vs 유클리드 거리

#### 매칭 필터링
- [x] Lowe's Ratio Test 원리 (best/second-best 비율)
- [x] Cross-check 매칭
- [x] 거리 임계값 설정

#### RANSAC
- [x] RANSAC 알고리즘 원리 (랜덤 샘플링 + 합의)
- [x] Inlier vs Outlier 분류
- [x] 반복 횟수와 inlier 비율 관계

#### SLAM에서 어디에 쓰이나?
- [x] Essential Matrix 추정 시 RANSAC 사용
- [x] Loop Closure 매칭에서 outlier 제거
- [x] Robust한 포즈 추정의 핵심

#### 실습
- [x] 두 이미지 간 ORB 특징점 매칭
- [x] 매칭 결과 시각화 (`cv2.drawMatches`)
- [x] RANSAC으로 outlier 제거 전후 비교

### 🔍 Section 2.2 자체 점검
> 아래 질문에 답할 수 있으면 다음으로 진행

1. FAST가 Harris보다 빠른 이유는?
2. Lowe's Ratio Test는 왜 효과적인가?
3. RANSAC에서 반복 횟수는 어떻게 결정하는가?

---

## 📋 Section 2.3: 에피폴라 기하학 (3주)

### Week 5-7: 에피폴라 기하학 + 스테레오 비전 🔥

> 💻 **C++ 실습**: [PRACTICE.md](../Studies/Phase%202/week5/PRACTICE.md)  
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo (캘리브레이션 완료 필요)  
> ⏰ **실습 시간**: 18-24시간 (3주)

#### Week 5: 기본 개념 (이론)

##### 에피폴라 제약
- [ ] 에피폴 (Epipole) 정의 — 다른 카메라 중심의 투영점
- [ ] 에피폴라 선 (Epipolar Line) 정의
- [ ] 에피폴라 평면 이해
- [ ] 에피폴라 제약의 의미 (2D 검색 → 1D 선으로 축소)

#### Essential Matrix
- [ ] Essential Matrix E 정의: x'ᵀ E x = 0
- [ ] E = [t]× R 관계 (t의 skew-symmetric matrix)
- [ ] E의 성질 (rank 2, 두 특이값 같음)
- [ ] 5-point 알고리즘 개념

#### Fundamental Matrix
- [ ] Fundamental Matrix F 정의
- [ ] E와 F의 관계: F = K'⁻ᵀ E K⁻¹
- [ ] 8-point 알고리즘 개념
- [ ] 언제 E를, 언제 F를 쓰는가 (캘리브레이션 여부)

#### SLAM에서 어디에 쓰이나?
- [ ] VO 초기화에서 Essential Matrix로 첫 포즈 추정
- [ ] 에피폴라 제약으로 잘못된 매칭 걸러내기
- [ ] VINS 초기화 과정에서 사용

#### Week 6: 포즈 추정 + Stereo Rectification (실습) ⭐

#### Essential Matrix에서 포즈 복원
- [ ] E → R, t 분해 (SVD 사용)
- [ ] 4가지 해가 나오는 이유
- [ ] Cheirality check로 올바른 해 선택 (점이 두 카메라 앞에 있어야)

#### Fundamental Matrix 추정
- [ ] Normalized 8-point algorithm (좌표 정규화의 중요성)
- [ ] RANSAC과 결합하여 robust 추정

#### 실습
- [ ] 두 이미지에서 Essential Matrix 추정 (`cv2.findEssentialMat`)
- [ ] 에피폴라 선 그려서 제약 확인
- [ ] 상대 포즈 (R, t) 복원 (`cv2.recoverPose`)

#### Week 7: Disparity/Depth Map 생성 (실습) ⭐

#### 삼각측량 (Triangulation)
- [ ] 두 카메라 뷰에서 3D 점 복원 원리
- [ ] Linear triangulation (DLT 방법)
- [ ] 재투영 오차 (Reprojection Error) 계산
- [ ] 삼각측량 정확도와 baseline 관계

#### PnP (Perspective-n-Point)
- [ ] 문제 정의: 3D-2D 대응에서 카메라 포즈 추정
- [ ] P3P 알고리즘 개념 (최소 3점)
- [ ] EPnP 개념 (효율적인 방법)
- [ ] OpenCV `solvePnP`, `solvePnPRansac` 사용법

#### SLAM에서 어디에 쓰이나?
- [ ] VO에서 새 프레임 포즈 추정 = PnP
- [ ] 맵 포인트(3D) + 현재 이미지(2D) → 현재 포즈
- [ ] VINS에서 Visual factor의 기초

#### 실습
- [ ] 매칭된 점들로 3D 점 복원 (`cv2.triangulatePoints`)
- [ ] 복원된 점들 3D 시각화 (matplotlib 또는 Open3D)
- [ ] PnP로 새 프레임 포즈 추정

### 🔍 Section 2.3 자체 점검
> 아래 질문에 답할 수 있으면 다음으로 진행

1. Essential Matrix와 Fundamental Matrix의 차이는?
2. 삼각측량에서 baseline이 작으면 왜 정확도가 떨어지는가?
3. PnP에서 최소 몇 개의 점이 필요한가?

---

## 📋 Section 2.4: 광류 (Optical Flow) (1주)

### Week 8: 광류 (Optical Flow) + KLT Tracker 🔥

> 💻 **C++ 실습**: VINS feature_tracker 구조 이해 및 구현  
> 🛠️ **하드웨어**: Jetson Orin Nano + ELP Stereo  
> ⏰ **실습 시간**: 6-8시간

#### 광류 이해

#### 기본 개념
- [ ] 광류의 정의 (프레임 간 픽셀 움직임 벡터)
- [ ] 밝기 항상성 가정 (Brightness Constancy): I(x,y,t) = I(x+u, y+v, t+1)
- [ ] Aperture 문제 (edge에서 수직 방향만 알 수 있음)

#### Lucas-Kanade 방법
- [ ] LK 알고리즘 원리 (지역 윈도우 내 일정 움직임 가정)
- [ ] 피라미드 LK (큰 움직임 처리)
- [ ] KLT Tracker = FAST 검출 + 피라미드 LK 추적

#### Sparse vs Dense
- [ ] Sparse Optical Flow: 특징점만 추적 (빠름)
- [ ] Dense Optical Flow: 모든 픽셀 (느림, Farneback 등)
- [ ] VIO에서는 Sparse 사용 (실시간 필수)

#### SLAM에서 어디에 쓰이나?
- [ ] **VINS feature_tracker의 핵심**: FAST 검출 + KLT 추적
- [ ] 프레임마다 특징점 재검출 안 하고 추적 (효율)
- [ ] 추적 실패 시 새 특징점 검출

#### 실습
- [ ] OpenCV `cv2.calcOpticalFlowPyrLK` 사용
- [ ] 웹캠으로 실시간 특징점 추적 구현
- [ ] 추적 실패 감지 및 새 특징점 추가 로직

#### VINS feature_tracker 코드 미리보기 (선택)
- [ ] `feature_tracker_node.cpp` 구조 훑어보기
- [ ] `FeatureTracker` 클래스에서 KLT 사용 부분 찾기
- [ ] 특징점 관리 방식 파악 (ID 부여, 추적 횟수)

### 🔍 Section 2.4 자체 점검
> 아래 질문에 답할 수 있으면 다음으로 진행

1. Lucas-Kanade가 가정하는 것은?
2. 피라미드 LK는 왜 필요한가?
3. VINS에서 특징점을 프레임마다 새로 검출하지 않는 이유는?

---

## ✅ Phase 2 완료 체크리스트

### 카메라 모델
- [x] 핀홀 모델 설명 가능
- [ ] OpenCV로 카메라 캘리브레이션 직접 수행 가능
- [ ] Kalibr 기본 사용법 파악
- [x] 왜곡 보정 적용 가능

### 특징점
- [x] FAST, ORB 특징점 검출 구현 가능
- [x] 특징점 매칭 및 시각화 가능
- [x] RANSAC 원리 이해

### 에피폴라 기하학
- [ ] Essential Matrix와 Fundamental Matrix 차이 설명 가능
- [ ] 삼각측량으로 3D 점 복원 가능
- [ ] PnP로 포즈 추정 가능

### 광류
- [ ] LK Tracker로 특징점 추적 가능
- [ ] VINS feature_tracker의 동작 원리 이해

---

## 🎯 Phase 2 완료 기준

> "VINS-Fusion의 `feature_tracker` 노드가 뭘 하는지 이해하고, 각 단계가 왜 필요한지 설명 가능"

---

## 📚 참고 자료

### 책 (사전처럼 사용)

| 책 | 용도 |
|------|------|
| Multiple View Geometry (Hartley & Zisserman) | 에피폴라 기하학 |
| Computer Vision: Algorithms and Applications (Szeliski) | 전반적 비전 |

### 온라인

| 자료 | 용도 |
|------|------|
| OpenCV 공식 튜토리얼 | 실습 코드 |
| First Principles of CV (YouTube) | 비전 기초 개념 |
| Cyrill Stachniss - Photogrammetry | 기하학 심화 |

### 도구

| 도구 | 용도 |
|------|------|
| OpenCV | 비전 실습 |
| Kalibr | Camera-IMU 캘리브레이션 |

---

## 💡 팁

1. **실습 우선**: 이론 30%, 실습 70%
2. **OpenCV 문서 활용**: 함수 파라미터와 반환값 정확히 확인
3. **시각화**: 항상 결과를 그림으로 확인 (매칭, 에피폴라 선, 3D 점)
4. **VINS 코드와 연결**: "이 개념이 VINS 어디에 쓰이나?" 계속 생각
5. **Kalibr 미리 설치**: Docker 이미지로 준비해두면 Phase 4에서 편함

---

## ❓ 다음 단계

Phase 2 완료 후:
- Phase 3 (Visual Odometry & Bundle Adjustment)로 진행
- 배운 개념들을 연결해서 VO 파이프라인 직접 구현