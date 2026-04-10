# Week 4 실습 가이드: 삼각측량 + PnP

## 빌드 & 실행

```bash
cd week4 && mkdir build && cd build
cmake .. && make
./basic          # 전체 파이프라인 데모
./my_basic       # 사용자 구현 뼈대
./quiz_easy      # 개념 퀴즈
./quiz_medium    # 구현 퀴즈
```

## 실습 순서

### 1단계: 데모 실행 (30분)

1. `./basic` 실행
2. `output/` 이미지 확인:
   - `01_3d_box_projections.png` — 두 카메라에서 본 3D 박스
   - `02_reprojection_error.png` — 관측(녹) vs 재투영(적) 비교
3. 콘솔의 삼각측량 오차, PnP 포즈 추정, 재투영 오차 읽기

### 2단계: 개념 퀴즈 (30분)

`./quiz_easy` 실행 후 질문에 답하기:
- 문제 1: 삼각측량 원리 (baseline, 최소 카메라 수)
- 문제 2: PnP 최소 점 수 (P3P, EPnP)
- 문제 3: 재투영 오차 수동 계산

### 3단계: 직접 구현 (2-3시간)

`my_basic.cpp` 의 Step 1~5 구현:
1. 카메라 + 3D 박스 설정
2. 두 카메라에서 투영 (K·[R|t]·P)
3. 삼각측량 (`cv::triangulatePoints`)
4. PnP (`cv::solvePnP`)
5. 재투영 오차 (`cv::projectPoints`)

### 4단계: 구현 퀴즈 (1시간)

`./quiz_medium` 실행 후 TODO 채우기:
- 문제 1: DLT 삼각측량 SVD 직접 구현
- 문제 2: RANSAC 반복 수 공식 계산

## 핵심 API 정리

| 함수 | 입력 | 출력 | 용도 |
|------|------|------|------|
| `cv::triangulatePoints` | P1, P2, pts1, pts2 | 4D 동차 좌표 | 삼각측량 |
| `cv::solvePnP` | 3D pts, 2D pts, K, dist | rvec, tvec | 카메라 포즈 추정 |
| `cv::solvePnPRansac` | 동일 + RANSAC 파라미터 | rvec, tvec, inliers | RANSAC 기반 PnP |
| `cv::projectPoints` | 3D pts, rvec, tvec, K, dist | 2D pts | 3D→2D 재투영 |

## Perception 맥락

- **삼각측량**: nuScenes 6 카메라에서 동일 객체의 3D 위치 복원
- **PnP**: 3D Detection 결과의 재투영 검증, 카메라 외재 캘리브레이션
- **재투영 오차**: Monocular 3D Detection (FCOS3D) 의 평가/학습 지표
