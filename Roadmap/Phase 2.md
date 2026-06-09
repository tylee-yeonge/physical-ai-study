# Phase 2: Perception을 위한 기하학 기초


> **기간**: 4주
> **목표**: 카메라 행렬·왜곡·Multi-view 기하를 사용해 **2D 관측과 3D 세계를 연결**한다. 3D Object Detection / Depth Estimation / BEV의 기하학적 기초.
> **주간 시간**: Week 3 6-9시간, Week 4 5-7시간 (하이브리드 적용)
> **언어**: C++ (OpenCV)
> **하드웨어**: Ubuntu PC (RTX 4070) + ELP Stereo Camera


---


## 완료 기준


다음 한 문장에 자신 있게 답할 수 있으면 Phase 2 완료:


> **"카메라 행렬과 왜곡 보정, Multi-view 기하(Rectification, Triangulation, PnP)를 사용해 2D 픽셀과 3D 점을 양방향으로 연결할 수 있고, 이 연산이 Stereo Depth / Monocular 3D Detection / BEV 모델의 기하학적 토대임을 설명할 수 있다."**


면접 관점의 검증 질문:
1. KITTI/nuScenes 의 카메라 캘리브레이션 파일을 보고 fx/fy/cx/cy/distortion 을 해석할 수 있는가?
2. Stereo Depth 모델(HITNet, CRE-Stereo)에 입력하기 전 왜 Rectification 이 필요한가?
3. Monocular 3D Detection (FCOS3D, SMOKE) 이 예측한 3D 박스를 어떻게 2D 이미지에 재투영해 검증하는가?
4. nuScenes 의 6 카메라 rig 에서 한 점의 3D 위치를 복원하려면 어떤 연산이 필요한가?


---


## 4주 구조 한눈에


| Week | 주제 | 학습 상태 | 코드 상태 |
|------|------|----------|----------|
| **1** | 카메라 모델 (핀홀, K, 내부/외부 파라미터) | 학습 완료 | 완료 |
| **2** | 렌즈 왜곡 + 캘리브레이션 | 학습 완료 | 완료 |
| **3** | Multi-view 기하 + Stereo Rectification | 학습 완료 | 작성 + KITTI 검증 + (선택) ELP |
| **4** | 삼각측량 + PnP (Perception 3D 맥락) | 학습 완료 | 이론 + OpenCV API + KITTI/nuScenes |


> 학습 상태는 "학습자(나)의 진행도" 이고, 코드 상태는 "이 디렉토리의 실습 코드 상태"이다.


```mermaid
graph LR
    W1[Week 1<br/>카메라 모델]
    W2[Week 2<br/>왜곡 + 캘리브레이션]
    W3[Week 3<br/>Multi-view 기하<br/>+ Rectification]
    W4[Week 4<br/>삼각측량 + PnP]
    P5[Phase 3<br/>Detection + Depth]


    W1 --> W2 --> W3 --> W4 --> P5
```


---


## Section 2.1: 카메라 모델 (Week 1, 2)


### Week 1: 핀홀 카메라 모델 (학습 완료)


> **C++ 실습**: [Studies/Phase 2/week1/](../Studies/Phase%202/week1/)


#### 기본 개념 (학습 완료)
- [x] 핀홀 카메라 원리
- [x] 3D → 2D 투영 과정
- [x] 초점 거리 (Focal Length) / 주점 (Principal Point)


#### 내부 / 외부 파라미터 (학습 완료)
- [x] 카메라 내부 행렬 K 구조
- [x] [R|t] 외부 파라미터
- [x] 월드 → 카메라 → 이미지 좌표 변환 사슬


#### Perception에서 어디에 쓰이나
- **Depth Estimation**: 모델은 픽셀 disparity 또는 normalized depth 를 출력 — 실제 미터 단위 복원에는 K 가 필수 (`Z = fB/d`)
- **Monocular 3D Detection (FCOS3D, SMOKE)**: 2D 이미지에서 3D 박스를 예측할 때, K 의 fx/fy/cx/cy 가 모델 입력 또는 후처리에 사용됨
- **3D → 2D Back-projection**: 3D 박스 코너를 2D 이미지에 그려 시각적 검증할 때 핵심 연산
- **데이터셋 호환**: KITTI/nuScenes calibration 파일이 정확히 이 형식


### Week 2: 렌즈 왜곡 + 캘리브레이션 (학습 완료)


> **C++ 실습**: [Studies/Phase 2/week2/](../Studies/Phase%202/week2/)


#### 렌즈 왜곡 / 캘리브레이션 (학습 완료)
- [x] 방사 왜곡 (배럴/핀쿠션) + 접선 왜곡
- [x] 왜곡 계수 (k1, k2, p1, p2, k3)
- [x] 체커보드 캘리브레이션 절차
- [x] 재투영 오차 (Reprojection Error)
- [x] OpenCV C++ 캘리브레이션 (mono + stereo)
- [x] RMS < 0.5 픽셀 달성


#### Perception에서 어디에 쓰이나
- **학습 데이터 전처리**: 왜곡 보정 없이 학습하면 모델 성능 저하 (특히 광각 카메라)
- **데이터셋 표준**: KITTI/nuScenes 모두 "rectified + undistorted" 이미지를 제공 — 이 전처리를 직접 해야 한다는 뜻
- **Fisheye 카메라 대응**: AMR 의 360° 인지에 자주 사용되는 fisheye 모델 (cv::fisheye)
- **Stereo Baseline**: stereo depth 의 절대 스케일은 baseline 에서 나옴 → 캘리브레이션 정확도가 직접 영향
- **Multi-camera rig (BEV)**: nuScenes 의 6 카메라가 모두 정확히 캘리브레이션되어야 BEV 에서 정합 가능


---


## Section 2.2: Multi-view 기하 (Week 3, 4)


### Week 3: Multi-view 기하 + Stereo Rectification


> **C++ 실습**: [Studies/Phase 2/week3/](../Studies/Phase%202/week3/)
> **실습 시간**: 6-8시간


#### 학습 목표
- 에피폴라 제약을 이해하고 Stereo Rectification 까지 한 흐름으로 연결
- Stereo Depth 모델의 **입력 전처리**가 왜 필요한지 직관 확보
- KITTI / nuScenes 의 카메라 rig 구조 해석 능력


#### 핵심 개념
- [x] 에피폴라 제약 (Epipolar Constraint) — 두 뷰 사이 점이 만족해야 하는 선형 관계
- [x] Essential Matrix `E` / Fundamental Matrix `F` — 두 카메라의 상대 자세
- [x] Stereo Rectification — 두 이미지를 같은 평면에 정렬, epipolar line 이 수평이 되도록
- [x] 디스패리티 <-> 깊이 관계 `Z = fB/d`
- [x] (보조) RANSAC — outlier 가 많은 매칭에서 robust 하게 모델 추정


#### Perception에서 어디에 쓰이나
- **Stereo Depth Network (HITNet, CRE-Stereo, RAFT-Stereo)**: 입력은 **rectified stereo pair**. 이 전처리를 못하면 모델은 못 돌린다
- **KITTI Stereo 벤치마크**: 직접 평가를 해보려면 rectification + disparity 파이프라인 이해 필수
- **Multi-camera BEV (BEVFormer, BEVDet)**: 각 카메라의 intrinsic + extrinsic 정렬이 BEV 변환의 근간
- **Visual relocalization (Phase 4 NeRF)**: 두 뷰의 상대 자세 추정은 NeRF/Gaussian Splatting 의 카메라 포즈 입력


#### 실습 코드
- `basic.cpp` — 데모 파이프라인: 샘플 stereo → E/F 계산 → Rectification → Disparity → Depth map → 결과 저장
- `my_basic.cpp` — 사용자 구현 뼈대 (Step 1-6)
- `quiz_easy.cpp` / `quiz_medium.cpp` — 개념 + 구현 퀴즈


#### KITTI 데이터 검증 (출장지 원격 PC 가능)
- scratch 구현한 rectify 결과를 **KITTI 공식 rectified 이미지와 픽셀 단위 diff**
- 매칭 특징점의 **y-disparity 측정** → 학습한 품질 지표를 실데이터에 적용
- 실행: Ubuntu PC (원격), 시각화: Rerun.io
- 이 단계가 Phase 3 (데이터셋 기반) 로의 자연스러운 전환점
- 상세: [Studies/Phase 2/week3/PRACTICE.md](../Studies/Phase%202/week3/PRACTICE.md) 5단계


#### ELP 실카메라 캘리브 실습 (선택)
- 이론으로 배운 캘리브/rectify 를 실제 하드웨어로 확인
- 체커보드 또는 ChArUco 보드로 stereo 캘리브 → 본인 구현으로 rectify → y-disparity 측정
- 필수 아님 — 시간 여유 있을 때 수행
- 상세: [Studies/Phase 2/week3/PRACTICE.md](../Studies/Phase%202/week3/PRACTICE.md) 6단계


### Week 4: 삼각측량 + PnP (Perception 3D 맥락)
> **C++ 실습**: [Studies/Phase 2/week4/](../Studies/Phase%202/week4/)
> **실습 시간**: 6-8시간


#### 학습 목표
- 삼각측량 = Multi-view Depth / Multi-view 3D Detection 의 기초 연산
- PnP = 3D 박스 <-> 2D 이미지 관계 검증의 도구
- 재투영 오차 (Reprojection Error) = 3D Detection 평가 지표의 기초


#### 핵심 개념
- [x] DLT (Direct Linear Transform) 삼각측량 — 두 시점에서 보인 점의 3D 위치
- [x] PnP 알고리즘 — 3D-2D 대응으로부터 카메라 포즈 추정 (P3P, EPnP, Iterative)
- [x] RANSAC + PnP — outlier 에 강건한 포즈 추정
- [x] 재투영 오차 (Reprojection Error) — 3D 점을 다시 2D 에 투영했을 때의 픽셀 오차


#### Perception에서 어디에 쓰이나
- **Monocular 3D Detection (FCOS3D, SMOKE, MonoFlex)**: 예측한 3D 박스 8 코너를 2D 에 투영 → 재투영 오차로 검증, 학습 시 loss term 으로도 사용
- **nuScenes Multi-view 객체 매칭**: 여러 카메라에서 보인 같은 객체 → 삼각측량으로 3D 위치 복원
- **BEV Detection 의 역연산**: Camera → BEV 변환의 수학적 기반
- **NeRF / Gaussian Splatting (Phase 4 preview)**: Multi-view reconstruction 의 가장 기본 연산
- **카메라 외재 캘리브레이션**: 알려진 3D 패턴(체커보드)으로 PnP 풀어 카메라 포즈 추정


#### 실습 방식 (하이브리드)


scratch 구현은 **생략**한다. 이유:
- Week 3 에서 rectify scratch 로 행렬/좌표 변환을 한 번 직접 다뤘다 → "내부 수학 한 번 경험" 충족
- PnP / 삼각측량의 본질은 SVD + 수치 안정화 — 교육용 scratch 는 이해 비용만 높고 실무 가치 낮음
- Perception 실무 감각은 **API 활용 + 데이터셋 처리**가 핵심


실습 흐름:
1. README 이론 정독 (DLT, PnP, 재투영 오차)
2. OpenCV API 조립 (`cv::triangulatePoints`, `cv::solvePnP`, `cv::projectPoints`) — my_basic.cpp 의 TODO 를 순서대로 채움 (이미 API 조립 중심으로 작성되어 있음)
3. KITTI Object Detection calib + label 로 3D 박스 2D 재투영
4. nuScenes mini-split 6-cam 샘플로 `cv::solvePnP` 외재 추정
5. Rerun.io 시각화


> `quiz_medium.cpp` 의 **문제 1 (DLT 삼각측량 SVD scratch 구현)** 은 개념 이해만 하고 풀이는 선택. **문제 2 (RANSAC 반복 수 공식 계산)** 은 권장.


상세: [Studies/Phase 2/week4/PRACTICE.md](../Studies/Phase%202/week4/PRACTICE.md)


**완료 기준**: `my_basic.cpp` 실행 시 Step 5 재투영 오차의 **평균 / 최대 / RMS** 픽셀값 콘솔 출력. 합성 데이터 평균 < 0.01 px.


---


## 학습 환경 (Phase 2)


| Week | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| 1, 2 (완료) | 보존 | - |
| 3 | Ubuntu PC (원격) | O (KITTI 검증) / 선택: ELP 실카메라 (PC 로컬 또는 원격) |
| 4 | Ubuntu PC (원격) | O |


- 시각화 기본: Rerun.io
- 상세: [ENVIRONMENT.md](../ENVIRONMENT.md)


---


## Phase 2 최종 자체 점검


다음 질문에 짧게라도 설명할 수 있으면 Phase 2 완료:


1. KITTI 캘리브레이션 파일에서 `P0`, `P1`, `R_rect`, `Tr_velo_to_cam` 이 각각 무엇인가?
2. Stereo Depth 모델이 rectified pair 를 입력으로 받는 이유는?
3. Disparity 가 1 픽셀일 때 Depth 가 얼마인지 계산하는 공식은? (fx, baseline 주어짐)
4. Monocular 3D Detection 모델이 출력한 3D 박스가 정확한지 어떻게 시각적으로 검증하는가?
5. PnP 알고리즘이 최소 몇 개의 3D-2D 대응을 필요로 하는가? P3P 와 EPnP 의 차이는?
6. RANSAC 의 반복 횟수를 결정하는 공식과 그 의미는?
7. KITTI 의 P0/P1/P_rect_02 와 본인이 구현한 rectify 결과가 어느 수준까지 일치하는가? (픽셀 diff, y-disparity 수치)
8. nuScenes 의 calibrated_sensor + ego_pose 체인을 통해 월드 좌표계에서의 카메라 위치를 구할 수 있는가?


---


## -> 다음 단계


Phase 2 완료 후 → **[Phase 3: Detection + Depth → PC TensorRT + ROS2 노드](Phase%203.md)** (약 2개월, 2026.06-08) 로 직진.


> SLAM 트랙(VO/BA, VIO)은 [Archive/SLAM-legacy/](../Archive/SLAM-legacy/) 에 있음. 메인 로드맵에는 포함되지 않는다.
>
> Phase 3 는 8주 구성 (PC TensorRT + ROS2 노드). Jetson 실기 배포는 v3 이후 옵션.


---


## 참고 자료


### Multi-view Geometry
- *Multiple View Geometry in Computer Vision* (Hartley & Zisserman) — 1, 6, 9, 10, 11장
- OpenCV 공식 문서: [Camera Calibration and 3D Reconstruction](https://docs.opencv.org/4.x/d9/d0c/group__calib3d.html)
- Cyrill Stachniss 강의 영상 (Photogrammetry I/II) — 에피폴라 / 삼각측량 부분


### Perception 맥락
- KITTI 데이터셋 README (calibration 형식 설명)
- nuScenes devkit (`nuscenes.utils.geometry_utils` 의 카메라 변환 코드)
- HITNet 논문 — Stereo network 입력 조건
- FCOS3D / SMOKE 논문 — Monocular 3D detection 의 기하학적 후처리


### Phase 2 가 다루지 않는 것 (Archive 참조)
- ORB / SIFT / AKAZE 등 특징점 디스크립터 deep dive → [Archive/SLAM-legacy/Studies/Phase 2/week3/](../Archive/SLAM-legacy/Studies/Phase%202/week3/)
- Optical Flow / KLT 트래킹 → [Archive/SLAM-legacy/Studies/Phase 2/week8/](../Archive/SLAM-legacy/Studies/Phase%202/week8/)
- VO/BA/VIO 시스템 통합 → [Archive/SLAM-legacy/Studies/Phase 3/](../Archive/SLAM-legacy/Studies/Phase%203/), [Phase 4/](../Archive/SLAM-legacy/Studies/Phase%204/)
