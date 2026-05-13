# Week 2: 렌즈 왜곡과 캘리브레이션


## 개요


> **목표**: 실제 카메라의 왜곡을 이해하고, 캘리브레이션으로 파라미터 측정하기
> **예상 시간**: 이론 2시간 + 실습 3시간


**핀홀 모델** 은 이상적인 카메라를 가정하지만, 실제 카메라 렌즈는 빛을 굴절시키며 **왜곡(distortion)** 을 발생시킵니다. 이번 주에는 왜곡의 종류를 이해하고, **캘리브레이션** 을 통해 카메라 파라미터를 정확히 측정하는 방법을 배웁니다.


### [?] 왜 이걸 배워야 할까요?


**일상 비유**: 숟가락에 비친 자신의 얼굴을 본 적 있나요?


- 얼굴이 휘어 보이고, 가장자리로 갈수록 더 심하게 왜곡됨
- 이것이 바로 **렌즈 왜곡**!
- 카메라 렌즈도 마찬가지로 직선을 곡선으로 만듦


**Perception 에서의 중요성**:
- 왜곡된 이미지로 학습하면 모델 성능 저하 (특히 광각 카메라)
- KITTI / nuScenes 같은 데이터셋의 calibration 파일에 정확히 이 `distortion_parameters` 가 들어 있음
- Stereo Depth network 의 입력은 rectified (= 왜곡 보정 + 정렬) 이미지가 전제
- 캘리브레이션이 잘못되면 3D Detection / BEV 결과 전반의 신뢰도가 떨어짐


---


## 학습 순서


| 순서 | 단계 | 파일 | 연결 퀴즈 |
|:----:|------|------|-----------|
| 1 | 데모 실행 — `./basic` 출력 전체 읽기 | `basic.cpp` | - |
| 2 | 왜곡 이론 학습 (방사/접선) | `README.md` | **easy 문제 2**: 왜곡 유형 판별 |
| 3 | 캘리브레이션 과정 이해 | `README.md` | **easy 문제 4**: 3D-2D 대응점 생성 |
| 4 | my_basic Step 1~3 (생성자, 품질평가, 객체점) | `my_basic.cpp` | **easy 문제 1**: K 행렬 분석, **문제 3**: RMS 평가 |
| 5 | my_basic Step 4~5 (왜곡보정, 체커보드) | `my_basic.cpp` | **easy 문제 5**: 왜곡 크기 분포 분석 |
| 6 | my_basic Step 6~7 (저장, 캘리브레이션) | `my_basic.cpp` | - |
| 7 | 초급 퀴즈 풀기 | `quiz_easy.cpp` | **easy 문제 1~5** |
| 8 | 중급 퀴즈 풀기 | `quiz_medium.cpp` | **medium 문제 1~5** |
| 9 | Fisheye / Kalibr 이론 읽기 | `README.md` | - |
| 10 | 카메라 실습 | [PRACTICE.md](./PRACTICE.md) | 실제 카메라 캘리브레이션 |


---


## Step 1: 먼저 돌려보기


> **이론을 읽기 전에 먼저 코드를 돌려보세요!**


```bash
cd week2 && mkdir build && cd build
cmake .. && make
./basic
```


실행 결과를 관찰한 후, 아래 이론을 읽으며 "아, 이게 이 뜻이었구나" 하고 채워갑니다.


---


## 핵심 개념


### 1. 렌즈 왜곡의 종류


#### 방사 왜곡 (Radial Distortion)


중심에서 바깥으로 방사형으로 발생하는 왜곡입니다.


```
        배럴 왜곡 핀쿠션 왜곡
      (Barrel) (Pincushion)
                                

    +----------+ +----------+
    | +------+ | | +------+ |
    | | | | | | | |
    | | | | | | | |
    | +------+ | | +------+ |
    +----------+ +----------+
    

    실제 → 바깥으로 밀림 실제 → 안쪽으로 당겨짐
    (광각 렌즈에서 흔함) (망원 렌즈에서 흔함)
```


**수학적 모델**:


```
r² = x² + y² (중심에서의 거리)


x_distorted = x(1 + k₁r² + k₂r⁴ + k₃r⁶)
y_distorted = y(1 + k₁r² + k₂r⁴ + k₃r⁶)
```


- k₁, k₂, k₃: 방사 왜곡 계수
- k₁ < 0: 배럴 왜곡 (광각)
- k₁ > 0: 핀쿠션 왜곡 (망원)


**특징**:
- 이미지 **중심에서 멀어질수록** 왜곡 심해짐
- 가장 흔한 왜곡 유형
- 보통 k₁, k₂면 충분, k₃는 극단적인 경우만


#### 접선 왜곡 (Tangential Distortion)


렌즈가 이미지 센서와 **완벽하게 평행하지 않을 때** 발생합니다.


```
     이상적 실제
       | \
       | \
    ---+--- 센서 ----\-- 센서 (약간 기울어짐)
       | \
       | \
       

     렌즈 렌즈
```


**수학적 모델**:


```
x_distorted = x + [2p₁xy + p₂(r² + 2x²)]
y_distorted = y + [p₁(r² + 2y²) + 2p₂xy]
```


- p₁, p₂: 접선 왜곡 계수
- 보통 방사 왜곡보다 훨씬 작음
- 고정밀 작업에서만 중요


#### 전체 왜곡 모델 (OpenCV 표준)


```
왜곡 계수: (k₁, k₂, p₁, p₂, k₃)


1. 정규화 좌표 계산: (x, y) = ((u-cx)/fx, (v-cy)/fy)
2. 거리 계산: r² = x² + y²
3. 방사 왜곡 적용:
   x' = x(1 + k₁r² + k₂r⁴ + k₃r⁶)
   y' = y(1 + k₁r² + k₂r⁴ + k₃r⁶)
4. 접선 왜곡 적용:
   x'' = x' + [2p₁xy + p₂(r² + 2x²)]
   y'' = y' + [p₁(r² + 2y²) + 2p₂xy]
5. 픽셀 좌표 복원:
   u' = fx·x'' + cx
   v' = fy·y'' + cy
```


---


### 2. 왜곡 보정 (Undistortion)


#### 왜 왜곡 보정이 필요한가?


```
왜곡된 이미지 보정된 이미지
+--------------+ +--------------+
| +--------+ | | +----------+ |
| | curved | | ------> | | straight | |
| +--------+ | | +----------+ |
+--------------+ +--------------+


직선이 곡선으로 보임 직선이 직선으로!
```


**Perception 시스템에서의 문제**:
- 에피폴라 기하학 가정: 직선이 직선으로 보존
- 왜곡 있으면 → Stereo Rectification 결과가 어긋나 disparity 추정 정확도 하락
- Multi-camera BEV 의 정합도 영향 — 각 카메라 왜곡이 보정되어야 BEV 평면이 정렬됨


#### OpenCV 왜곡 보정


```python
import cv2


# 왜곡 보정
undistorted = cv2.undistort(distorted_image, K, dist_coeffs)


# 또는 맵 사용 (더 빠름, 반복 사용 시)
mapx, mapy = cv2.initUndistortRectifyMap(K, dist_coeffs, None, K,
                                          (width, height), cv2.CV_32FC1)
undistorted = cv2.remap(distorted_image, mapx, mapy, cv2.INTER_LINEAR)
```


---


### 3. 카메라 캘리브레이션


#### 캘리브레이션이란?


카메라의 **내부 파라미터(K)와 왜곡 계수(dist_coeffs)**를 측정하는 과정입니다.


```
입력: 체스보드 이미지 10-20장
           |
           v
    +--------------+
    | 캘리브레이션 |
    | 알고리즘 |
    +--------------+
           |
           v
출력: K, dist_coeffs, 재투영 오차
```


#### 왜 체스보드를 사용하나?


| 특성 | 이유 |
|------|------|
| 격자 패턴 | 코너 검출 쉬움 |
| 알려진 크기 | 실제 3D 좌표 알 수 있음 |
| 평면 | 간단한 기하학 |
| 고대비 | 정확한 검출 |


**대안**: AprilTag, ChArUco, 원형 패턴


#### 캘리브레이션 과정


```
Step 1: 체스보드 코너 검출
        cv2.findChessboardCorners()
        

Step 2: 서브픽셀 정밀도로 코너 개선
        cv2.cornerSubPix()
        

Step 3: 3D-2D 대응점 수집
        - 3D: 체스보드의 실제 좌표 (알려진 값)
        - 2D: 검출된 코너 픽셀 (측정값)
        

Step 4: 캘리브레이션 실행
        cv2.calibrateCamera()
        

Step 5: 결과 검증
        재투영 오차 확인 (< 0.5 픽셀이면 좋음)
```


#### Python 구현


```python
import cv2
import numpy as np
import glob


# 체스보드 설정
CHECKERBOARD = (9, 6) # 내부 코너 개수
square_size = 0.025 # 한 칸 크기 (m)


# 3D 점 (체스보드의 실제 좌표)
objp = np.zeros((CHECKERBOARD[0] * CHECKERBOARD[1], 3), np.float32)
objp[:, :2] = np.mgrid[0:CHECKERBOARD[0], 0:CHECKERBOARD[1]].T.reshape(-1, 2)
objp *= square_size


# 수집할 데이터
objpoints = [] # 3D points
imgpoints = [] # 2D points


# 이미지에서 코너 검출
images = glob.glob('calibration_images/*.jpg')


for fname in images:
    img = cv2.imread(fname)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    

    ret, corners = cv2.findChessboardCorners(gray, CHECKERBOARD, None)
    

    if ret:
        # 서브픽셀 정밀도로 개선
        corners2 = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1),
                                     criteria=(cv2.TERM_CRITERIA_EPS +
                                              cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001))
        objpoints.append(objp)
        imgpoints.append(corners2)


# 캘리브레이션
ret, K, dist_coeffs, rvecs, tvecs = cv2.calibrateCamera(
    objpoints, imgpoints, gray.shape[::-1], None, None
)


print(f"카메라 행렬 K:\n{K}")
print(f"왜곡 계수: {dist_coeffs.flatten()}")
print(f"재투영 오차: {ret:.4f} 픽셀")
```


---


### 4. Fisheye (어안) 카메라


#### 일반 렌즈 vs Fisheye


| 특성 | 일반 렌즈 | Fisheye |
|------|---------|---------|
| FOV | 60-90° | 150-180° |
| 왜곡 | 약함 | 매우 강함 |
| 모델 | 다항식 | 등거리/등립체각 |
| 용도 | 일반 카메라 | 로봇, 드론, VR |


**Fisheye 왜곡 모델** (등거리):


```
θ = arctan(r)
θ_distorted = θ(1 + k₁θ² + k₂θ⁴ + k₃θ⁶ + k₄θ⁸)
```


**OpenCV Fisheye 모듈**:
```python
import cv2


# Fisheye 캘리브레이션
ret, K, D, rvecs, tvecs = cv2.fisheye.calibrate(
    objpoints, imgpoints, gray.shape[::-1], K, D
)


# Fisheye 왜곡 보정
undistorted = cv2.fisheye.undistortImage(img, K, D)
```


> OpenCV 와 대부분의 Perception 데이터셋 도구가 Pinhole + Fisheye 두 모델 모두 지원합니다.


---


### 5. Kalibr 캘리브레이션


#### Kalibr란?


ETH Zurich에서 개발한 카메라/IMU 캘리브레이션 도구입니다.


**장점**:
- 카메라-카메라 캘리브레이션 (Stereo)
- 카메라-IMU 캘리브레이션 (sensor fusion 시 유용)
- 시간 오프셋 추정
- YAML 형식 결과 (다양한 도구와 호환)


#### Docker로 설치 (권장)


```bash
# Docker 이미지 다운로드
docker pull stereolabs/kalibr


# 실행
docker run -it --rm \
  -v /path/to/data:/data \
  stereolabs/kalibr
```


#### AprilGrid 타겟


체스보드 대신 **AprilGrid**를 사용합니다:
- 각 태그에 고유 ID
- 부분 가림도 검출 가능
- 더 정확한 캘리브레이션


**타겟 생성**:
```bash
kalibr_create_target_pdf --type apriltag \
  --nx 6 --ny 6 --tsize 0.03 --tspace 0.3
```


#### 캘리브레이션 실행


```bash
kalibr_calibrate_cameras \
  --target april_6x6.yaml \
  --bag /data/calibration.bag \
  --models pinhole-radtan \
  --topics /cam0/image_raw
```


**결과 YAML 예시**:
```yaml
cam0:
  camera_model: pinhole
  distortion_model: radtan
  intrinsics: [458.654, 457.296, 367.215, 248.375]
  distortion_coeffs: [-0.28340811, 0.07395907, 0.00019359, 1.76187114e-05]
  resolution: [752, 480]
```


---


### 6. Stereo 카메라 (선택)


> ** 스테레오 캘리브레이션은 지금 하지 않아도 됩니다.**
> Jetson Orin Nano + ELP 스테레오 카메라가 필요하며, Phase 3 Stereo Depth 실습 전에 수행하면 됩니다.


#### Stereo의 장점


단일 카메라의 문제:
- **스케일 모호성**: 크기를 알 수 없음 (가까운 작은 물체 vs 먼 큰 물체)


Stereo 카메라의 해결:
- **두 카메라 사이 거리 (Baseline)** = 알려진 값
- 삼각측량으로 **절대 스케일** 복원 가능!


```
       물체 P
         *
        /| \
       / | \
      / | \
     / | \
    *----+-----*
  Cam0 B Cam1
   

  B = Baseline (알려진 값)
```


#### KITTI Stereo 캘리브레이션 예시


KITTI 의 `calib_cam_to_cam.txt` 는 4 카메라 rig (왼쪽/오른쪽 흑백 + 왼쪽/오른쪽 컬러) 의 K, 왜곡 계수, 그리고 rectified projection 행렬 `P_rect` 를 함께 제공합니다:


```text
# 핵심 필드만 발췌
S_02: 1.392000e+03 5.120000e+02 # 이미지 크기 (left color)
K_02: 9.597910e+02 0.0 6.960217e+02 # K 행렬
      0.0 9.569251e+02 2.241806e+02
      0.0 0.0 1.0
D_02: -3.7e-01 1.9e-01 1.0e-03 -3.0e-04 0.0 # 왜곡 계수 (k1,k2,p1,p2,k3)
T_02: 5.956621e-02 2.900141e-04 2.577209e-03 # baseline (오른쪽 컬러 대비)
P_rect_02: ... # rectified projection 행렬
```


→ 이번 주에 배운 K + (k1,k2,p1,p2,k3) 형식이 그대로 직렬화돼 있습니다. nuScenes 도 sensor 별 calibrated_sensor 항목에 동일한 정보를 JSON 으로 저장합니다.


---


## 실습 파일


---


## 핵심 정리


### 왜곡 종류


| 왜곡 유형 | 원인 | 계수 | 특징 |
|---------|------|------|------|
| 방사 왜곡 | 렌즈 곡률 | k₁, k₂, k₃ | 중심에서 멀수록 심함 |
| 접선 왜곡 | 센서 정렬 오류 | p₁, p₂ | 보통 작음 |


### 캘리브레이션 체크리스트


| 항목 | 권장값 | 판단 |
|------|-------|------|
| 사용 이미지 수 | 15-30장 | 최소 10장 |
| 재투영 오차 | < 0.5 픽셀 | < 1.0 OK |
| 이미지 다양성 | 다양한 각도 | 한 방향만 |
| 체스보드 가림 | 없음 | 부분 가림 |


### 왜곡 모델 매핑


| 모델 | OpenCV 함수 | 용도 |
|------|------------|------|
| Pinhole + Radtan (k1,k2,p1,p2,k3) | `cv::calibrateCamera` / `cv::undistort` | 일반 카메라, KITTI/nuScenes 표준 |
| Equidistant / Fisheye | `cv::fisheye::calibrate` / `cv::fisheye::undistortImage` | 광각 카메라 (AMR 360° 인지) |


---


## 학습 완료 체크리스트


### 기초 이해 (필수)
- [ ] 방사 왜곡과 접선 왜곡의 차이 설명 가능
- [ ] 배럴/핀쿠션 왜곡 구분 가능
- [ ] 왜곡 계수 (k₁, k₂, p₁, p₂, k₃) 의미 알기


### 실용 활용 (권장)
- [ ] OpenCV로 체스보드 캘리브레이션 수행 가능
- [ ] 왜곡 보정(undistort) 적용 가능
- [ ] 재투영 오차로 캘리브레이션 품질 판단 가능


### 심화 (선택)
- [ ] Fisheye 모델 이해
- [ ] Kalibr 설치 및 사용 (Docker)
- [ ] Stereo 캘리브레이션 개념 이해


---


## 다음 단계


### Week 3: Multi-view 기하 + Stereo Rectification


왜곡이 보정된 이미지에서:
- 에피폴라 제약과 Essential/Fundamental Matrix
- Stereo Rectification → Depth network 입력 전처리
- KITTI Stereo 벤치마크 이해


---


## 참고 자료


- OpenCV Camera Calibration Tutorial
- Kalibr Wiki: [https://github.com/ethz-asl/kalibr/wiki](https://github.com/ethz-asl/kalibr/wiki)
- KITTI 데이터셋 README — calibration 형식 설명
- nuScenes devkit (`nuscenes.utils.geometry_utils`)


---


## [?] FAQ


**Q1: 캘리브레이션은 얼마나 자주 해야 하나요?**
A: 렌즈 교체, 줌 변경, 큰 충격 후에만 다시 하면 됩니다. 같은 설정이면 한 번만!


**Q2: 재투영 오차가 큰 이유는?**
A: 체스보드 검출 오류, 이미지 부족, 다양성 부족, 흔들린 이미지 등.


**Q3: Fisheye 카메라를 일반 모델로 캘리브레이션하면?**
A: 오차가 커집니다. 반드시 Fisheye 모델 사용!


**Q4: Perception 파이프라인에서 왜곡 보정은 어디서 하나요?**
A: 일반적으로 데이터 로딩 / 전처리 단계에서 적용합니다. 학습 데이터셋(KITTI/nuScenes)은 보통 미리 보정된 이미지를 제공하지만, 직접 수집한 영상은 모델 입력 전에 `cv::undistort` 또는 `cv::initUndistortRectifyMap` + `cv::remap` 으로 보정해야 합니다.


---


** Week 2 핵심 메시지:**


> 실제 카메라 = 핀홀 모델 + 왜곡
>
> **캘리브레이션**으로 K와 왜곡 계수를 정확히 측정해야
> Stereo Depth, 3D Detection, BEV 등 Perception 파이프라인이 정확합니다!
