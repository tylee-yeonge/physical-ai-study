# Week 3 실습 가이드: Multi-view 기하 + Stereo Rectification


## 빌드 & 실행


```bash
cd week3 && mkdir build && cd build
cmake .. && make
./basic # 전체 파이프라인 데모
./my_basic # 사용자 구현 뼈대
./quiz_easy # 개념 퀴즈
./quiz_medium # 구현 퀴즈
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


`my_basic.cpp` 의 Step 1-6 을 순서대로 구현:
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


### 5단계: KITTI 검증 실습 (출장지 원격 PC 가능) [1-2시간]


**선행조건**: 3단계(직접 구현) Step 3-6 완료.


**목적**: scratch 구현한 rectify 를 KITTI 실데이터에 적용해 공식 결과와 diff.


1. KITTI raw 샘플 다운로드 (Ubuntu PC)
   - `2011_09_26_drive_0001_sync` 중 image_02, image_03 각 1-2장
   - 캘리브 파일: `calib_cam_to_cam.txt`
2. `P_rect_02`, `P_rect_03`, `R_rect_02`, `K_02`, `K_03`, baseline T 추출
3. 본인의 `buildRectifyMap` 에 파라미터 주입 → `cv::remap` 수행
4. OpenCV `cv::initUndistortRectifyMap` 결과와 픽셀 단위 diff
5. 매칭된 특징점의 y-disparity 측정
6. Rerun.io 로 좌/우 이미지 + 수평 에피폴라선 시각화


**검증 기준**: y-disparity < 1.0 px, OpenCV 결과와 diff 는 sub-pixel 수준.


### 6단계 (선택): ELP 실카메라 캘리브 실습 [2-3시간]


**목적**: 이론/데이터셋 너머 실제 하드웨어에서 rectify 품질을 확인.


**환경**: Ubuntu PC + ELP Stereo Camera (USB 연결). 원격 접속 시 PC 가 ELP 와 항시 연결되어 있으면 출장지에서도 가능.


1. ELP 좌/우 스트림 동시 캡처 (OpenCV `cv::VideoCapture` 로 두 채널)
2. ChArUco 또는 체커보드로 stereo calibrate (`cv::stereoCalibrate`)
3. 본인의 `buildRectifyMap` 에 결과 주입 → remap
4. 특징점 매칭으로 y-disparity 측정 (실데이터 기준 < 1-2 px 목표)
5. Rerun.io 로 실시간 / 스냅샷 시각화


**검증 기준**: reprojection error < 0.5 px (캘리브 품질), y-disparity < 2 px (rectify 품질).


**참고**: 필수 아님. 시간 여유 있을 때 수행. 상세 하드웨어 팁은 [ENVIRONMENT.md](../../../ENVIRONMENT.md) 참조.


### 7단계 (선택): Classical vs Learning-based Depth 비교 [1-2시간]


**목적**: 기하학 stereo 가 실패하는 영역(textureless / repetitive / occlusion)을 딥러닝 monocular depth 가 어떻게 메우는지 직접 시각화.


**전제**: 3단계 또는 5단계에서 얻은 rectified stereo pair 가 있을 것.


#### 7-1. Python 환경 준비 (venv)


```bash
# week3 디렉토리에서 실행
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install torch transformers opencv-python matplotlib pillow numpy
```


GPU 가 있으면 CUDA 지원 torch 를 설치하면 빠름. CPU 만으로도 Small 모델은 수 초 내 추론 가능.


#### 7-2. 스크립트 실행


```bash
python practice_dl_compare.py \
    --left build/output/01_left.png \
    --right build/output/01_right.png \
    --output-dir build/output/dl_compare
```


기본 모델은 `depth-anything/Depth-Anything-V2-Small-hf`. 처음 실행 시 HuggingFace 에서 자동 다운로드.


#### 7-3. 결과 관찰


`build/output/dl_compare/` 에 생성되는 파일:
- `01_sgbm_disparity.png` — SGBM dense disparity (invalid 는 검정)
- `02_sgbm_invalid_mask.png` — 기하가 포기한 영역 (흰색)
- `03_da_depth.png` — Depth Anything relative depth
- `04_comparison.png` — 4-패널 비교 (좌측원본 / SGBM / DA / Overlay)


**관찰 포인트**:
- Textureless 영역(벽, 하늘): SGBM 구멍 vs DA 의 smooth gradient
- Repetitive pattern: SGBM 의 잘못된 mode 선택 vs DA 의 일관된 표면
- Occlusion 경계: SGBM 의 무효 띠 vs DA 의 dense 출력
- 물체 경계 sharpness: 누가 더 선명한가


#### 7-4. (선택) Scale Alignment


Depth Anything 은 relative depth 만 출력. SGBM 의 유효 픽셀을 ground truth 로 써서 least-squares 로 scale/shift 를 맞추면 metric 스케일에 가깝게 변환 가능.


```bash
python practice_dl_compare.py \
    --left build/output/01_left.png \
    --right build/output/01_right.png \
    --output-dir build/output/dl_compare \
    --align-scale
```


이 단계까지 하면 "학습 기반 prior + 기하학 scale anchor" 라는 현대 perception 의 **hybrid 구조**를 직접 구현한 셈이 됩니다.


**검증 기준**: SGBM invalid 영역에서 DA 출력이 주변과 이질적이지 않은 gradient 를 만드는지 정성 평가.


## 핵심 API 정리


| 함수 | 입력 | 출력 | 용도 |
|------|------|------|------|
| `cv::findFundamentalMat` | 대응점 쌍 | F (3×3) | 두 뷰의 기하 관계 |
| `cv::stereoRectify` | K1, K2, dist, R, T | R1, R2, P1, P2, Q | Rectification 변환 계산 |
| `cv::initUndistortRectifyMap` | K, dist, R, P, size | mapx, mapy | Remap 테이블 생성 |
| `cv::remap` | image, mapx, mapy | rectified image | 이미지 변환 적용 |
| `cv::StereoBM::compute` | rect_left, rect_right | disparity (16배 스케일) | 블록 매칭 기반 disparity |


## Perception 맥락 정리


- 이 파이프라인의 **Step 4 결과 (rectified pair)** 가 HITNet / CRE-Stereo / RAFT-Stereo 같은 학습 기반 Stereo Depth 모델의 입력
- KITTI Stereo 벤치마크는 rectified 이미지를 기본 제공 — 이 전처리가 이미 적용된 상태
- `Z = fB/d` 공식은 모든 Stereo Depth 파이프라인의 최종 변환 단계
- **Monocular depth** (Depth Anything v2, MiDaS, ZoeDepth) 는 rectify/baseline 불필요하지만 metric scale 부재 → stereo/LiDAR 의 sparse metric 과 **fusion** 하는 것이 현대 perception 의 표준
- Classical stereo 의 textureless / repetitive / occlusion 실패 영역을 학습 기반 방법이 dense 하게 메우는 것이 핵심 장점 — 7단계에서 직접 체감


## 원격 실행 팁 (출장지 환경)


- VS Code Tunnel: `code tunnel` → vscode.dev 로 접속
- Docker 컨테이너: CUDA + OpenCV + PyTorch 이미지 권장
- Rerun 서버: `rr.serve()` 포트(9090) Tailscale 메시로 브라우저 접속
- 상세: [ENVIRONMENT.md](../../../ENVIRONMENT.md)
