# Week 3 실습 가이드: Multi-view 기하 + Stereo Rectification

## 빌드 & 실행

```bash
cd week3 && mkdir build && cd build
cmake .. && make
./basic          # 전체 파이프라인 데모
./my_basic       # 사용자 구현 뼈대
./quiz_easy      # 개념 퀴즈
./quiz_medium    # 구현 퀴즈
```

## 실습 순서

### 1단계: 데모 실행 (30분)

1. `./basic` 실행
2. `output/` 디렉토리의 결과 이미지 확인:
   - `01_left.png` / `01_right.png` — 원본 스테레오 쌍
   - `02_epipolar_lines.png` — 에피폴라 선 시각화
   - `03_rectified_pair.png` — Rectified 쌍 (수평선 정렬 확인)
   - `04_disparity.png` — Disparity map
   - `05_depth_map.png` — Depth map
3. 콘솔 출력을 읽으며 각 Step 이해

### 2단계: 개념 퀴즈 (30분)

`./quiz_easy` 실행 후 TODO 를 채우세요:
- 문제 1: 에피폴라 선 계산 (F × p1)
- 문제 2: Rectification 전후 비교 (개념 확인)
- 문제 3: Z = fB/d 공식 적용

### 3단계: 직접 구현 (2-3시간)

`my_basic.cpp` 의 Step 1~6 을 순서대로 구현:
1. 카메라 파라미터 설정
2. 합성 스테레오 쌍 생성
3. 특징점 매칭 + Fundamental Matrix
4. Stereo Rectification (`cv::stereoRectify`, `cv::initUndistortRectifyMap`, `cv::remap`)
5. Disparity 계산 (`cv::StereoBM`)
6. Depth 변환 (`Z = fB/d`)

### 4단계: 구현 퀴즈 (1시간)

`./quiz_medium` 실행 후 TODO 를 채우세요:
- 문제 1: 수동 Disparity → Depth + 3D 점 복원
- 문제 2: Rectification 품질 검증 (y-disparity 오차)

## 핵심 API 정리

| 함수 | 입력 | 출력 | 용도 |
|------|------|------|------|
| `cv::findFundamentalMat` | 대응점 쌍 | F (3×3) | 두 뷰의 기하 관계 |
| `cv::stereoRectify` | K1, K2, dist, R, T | R1, R2, P1, P2, Q | Rectification 변환 계산 |
| `cv::initUndistortRectifyMap` | K, dist, R, P, size | mapx, mapy | Remap 테이블 생성 |
| `cv::remap` | image, mapx, mapy | rectified image | 이미지 변환 적용 |
| `cv::StereoBM::compute` | rect_left, rect_right | disparity (16배 스케일) | 블록 매칭 기반 disparity |

## Perception 맥락 정리

- 이 파이프라인의 **Step 4 결과 (rectified pair)** 가 HITNet / CRE-Stereo 같은 Stereo Depth 모델의 입력
- KITTI Stereo 벤치마크는 rectified 이미지를 기본 제공 — 이 전처리가 이미 적용된 상태
- `Z = fB/d` 공식은 모든 Stereo Depth 파이프라인의 최종 변환 단계
