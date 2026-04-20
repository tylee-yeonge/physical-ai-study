# Week 4 실습 가이드: 삼각측량 + PnP (하이브리드 방식)

## 빌드 & 실행

```bash
cd week4 && mkdir build && cd build
cmake .. && make
./basic          # 전체 파이프라인 데모
./my_basic       # 사용자 구현 뼈대 (API 조립)
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

### 3단계: OpenCV API 실습 (1-2시간)

`my_basic.cpp` 의 **OpenCV API 호출 TODO** 를 순서대로 채운다. 이미 API 조립 중심으로 작성되어 있어 scratch 유도 블록 없음.

- `cv::triangulatePoints(P1, P2, pts1, pts2)`
- `cv::solvePnP(objectPoints, imagePoints, K, dist, rvec, tvec)`
- `cv::projectPoints(objectPoints, rvec, tvec, K, dist, imagePoints)`

합성 데이터로 파이프라인 동작 확인.

### 4단계: KITTI 3D Object Detection 실습 (2시간) [원격 PC]

1. KITTI Object Detection (image_2, calib, label_2 각 ~1GB) 다운로드 (선행 작업)
2. `label_2/000000.txt` 에서 3D 박스 (h, w, l, x, y, z, ry) 파싱
3. `calib/000000.txt` 의 P2 로 8 코너를 2D 재투영
4. 재투영 결과의 정합도 평가
5. Rerun.io 로 이미지 + 3D 박스 투영 시각화

**검증 기준**: 2D 박스 어노테이션과 재투영 8 코너 2D 바운딩이 대체로 일치.

### 5단계: nuScenes 샘플 실습 (1-2시간) [원격 PC]

1. nuScenes mini-split (~3GB) 다운로드 (선행 작업), devkit 설치
2. 한 sample 에서 6 카메라 `calibrated_sensor` 파라미터 추출
3. 두 카메라의 동일 객체 2D 어노테이션 → `cv::triangulatePoints` 로 3D 복원
4. annotation 3D 위치와의 오차 측정
5. Rerun 3D 뷰로 검증

**검증 기준**: 삼각측량 복원 3D 위치가 annotation 과 수십 cm 오차 이내.

### 6단계: 구현 퀴즈 (30분)

`./quiz_medium` 실행:
- **문제 1 (DLT 삼각측량 SVD scratch)**: 개념 이해만 하고 풀이는 선택
- **문제 2 (RANSAC 반복 수 공식 계산)**: 풀 것 — 공식 적용 수준

## 핵심 API 정리

| 함수 | 입력 | 출력 | 용도 |
|------|------|------|------|
| `cv::triangulatePoints` | P1, P2, pts1, pts2 | 4D 동차 좌표 | 삼각측량 |
| `cv::solvePnP` | 3D pts, 2D pts, K, dist | rvec, tvec | 카메라 포즈 추정 |
| `cv::solvePnPRansac` | 동일 + RANSAC 파라미터 | rvec, tvec, inliers | RANSAC 기반 PnP |
| `cv::projectPoints` | 3D pts, rvec, tvec, K, dist | 2D pts | 3D→2D 재투영 |

## 데이터셋 준비 체크리스트

> 다운로드 시간 (네트워크에 따라 30분~1시간) 은 실습 시간에 미포함. **출장 전 미리 받아두길 권장**.

- [ ] KITTI Object Detection (`image_2`, `calib`, `label_2`) 다운로드
- [ ] nuScenes mini-split 다운로드 + devkit 설치 (`pip install nuscenes-devkit`)
- [ ] 권장 저장 경로: `~/datasets/kitti_object/`, `~/datasets/nuscenes_mini/`
- [ ] Rerun.io 설치 (`pip install rerun-sdk`) 및 기본 예제 실행 확인

## Perception 맥락

- **삼각측량**: nuScenes 6 카메라에서 동일 객체의 3D 위치 복원
- **PnP**: 3D Detection 결과의 재투영 검증, 카메라 외재 캘리브레이션
- **재투영 오차**: Monocular 3D Detection (FCOS3D) 의 평가/학습 지표

## 원격 실행 팁

- VS Code Tunnel / vscode.dev → Ubuntu PC 접속
- Docker + Tailscale 메시 권장
- Rerun 서버 포트(9090) 브라우저 접속
- 상세: [ENVIRONMENT.md](../../../ENVIRONMENT.md)
