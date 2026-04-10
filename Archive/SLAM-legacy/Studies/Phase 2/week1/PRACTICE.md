# Week 1: 핀홀 카메라 모델 실습 (C++)

> 🎯 **목표**: 핀홀 카메라 투영/역투영을 C++로 구현하고 시각화
> 💻 **언어**: C++ (OpenCV 4.x, Eigen3)
> 🛠️ **하드웨어**: Jetson Orin Nano (또는 일반 PC)
> ⏰ **예상 시간**: 6-8시간

---

## 📋 준비사항

### 소프트웨어
```bash
# OpenCV 4.x 확인
pkg-config --modversion opencv4

# Eigen3 설치
sudo apt install libeigen3-dev

# 빌드 도구
sudo apt install cmake build-essential
```

---

## 📂 프로젝트 구조

```
week1/
├── CMakeLists.txt
├── basic.h                  # PinholeProjection 클래스 헤더
├── basic.cpp                # 구현 + 데모 main()
├── quiz_easy.cpp            # 기초 퀴즈
├── quiz_medium.cpp          # 중급 퀴즈
├── quiz_solutions/
│   ├── easy_sol.cpp         # 기초 퀴즈 정답
│   └── medium_sol.cpp       # 중급 퀴즈 정답
└── test/
    └── test_basic.cpp       # GTest 단위 테스트
```

---

## 🔧 실습 1: 핀홀 투영 구현

### Step 1: PinholeProjection 클래스

**basic.h** 에서 핵심 기능:
- `project()`: 3D 월드 점 → 2D 픽셀 투영
- `backProject()`: 2D 픽셀 → 3D 광선 역투영
- `computeFOV()`: 시야각 계산
- `reprojectionError()`: 재투영 오차 계산

```cpp
// 사용 예시
PinholeProjection camera(K, R, t);

// 3D → 2D 투영
cv::Point3d P_world(2.0, 1.0, 5.0);
cv::Point2d pixel = camera.project(P_world);

// 2D → 3D 역투영 (광선)
cv::Vec3d ray = camera.backProject(pixel);

// FOV 계산
cv::Size2d fov = camera.computeFOV(cv::Size(800, 600));
```

### Step 2: 빌드 및 실행

```bash
mkdir build && cd build
cmake ..
make -j4

# 기본 데모 실행
./basic

# 퀴즈 풀기
./quiz_easy
./quiz_medium
```

---

## 🔧 실습 2: 시각화

### 다양한 카메라 위치에서 투영 비교

```
카메라 위치 1 (정면):      카메라 위치 2 (오른쪽):
┌─────────────┐           ┌─────────────┐
│      ●      │           │    ●        │
│      큐브    │           │   큐브       │
│             │           │             │
└─────────────┘           └─────────────┘

→ R, t만 바뀌고 K는 동일!
```

### FOV와 투영 범위 확인

```
광각 (fx=300):              망원 (fx=1200):
┌─────────────────┐        ┌─────────────────┐
│ ●   ●   ●   ● │        │                 │
│   ●   ●   ●   │        │     ●   ●       │
│ ●   ●   ●   ● │        │       ●         │
└─────────────────┘        └─────────────────┘
넓은 시야, 작은 물체          좁은 시야, 큰 물체
```

---

## ✅ 체크리스트

### 기초 구현
- [ ] `PinholeProjection` 클래스 헤더/구현 이해
- [ ] 3D→2D 투영 (`project()`) 동작 확인
- [ ] 역투영 (`backProject()`) 동작 확인
- [ ] 재투영 오차 계산 이해

### 퀴즈
- [ ] quiz_easy: K 행렬 분석, 투영 계산
- [ ] quiz_medium: 다중 점 투영, FOV, 좌표 변환

### 심화
- [ ] 다양한 K 값으로 투영 결과 비교
- [ ] 카메라 위치(R, t) 변경에 따른 투영 변화 관찰
- [ ] 이미지 밖 투영 처리

---

## 💡 핵심 공식 요약

```
투영 (3D → 2D):
  Pc = R · Pw + t          (월드 → 카메라)
  x' = Xc / Zc             (원근 투영)
  u = fx · x' + cx         (정규화 → 픽셀)

역투영 (2D → 3D 광선):
  x' = (u - cx) / fx
  y' = (v - cy) / fy
  ray = [x', y', 1]        (정규화 후 단위벡터)
```

---

**다음 단계**: Week 2 - 카메라 캘리브레이션 실습
