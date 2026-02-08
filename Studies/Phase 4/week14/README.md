# Week 14: Kalibr 실습 (Section 4.5)

> 🎯 **이번 주 목표**: Kalibr를 사용한 Camera-IMU 캘리브레이션 실습 및 결과 검증
> ⏰ **예상 시간**: 10시간
> 💡 **핵심 질문**: "Kalibr 결과가 정확한지 어떻게 검증하는가?"

---

## 🌟 시작하기 전에

### 비유: 안과 검진

```
안과에서 시력 검사할 때:
  1. 시력 측정 (카메라 intrinsic = 렌즈 특성)
  2. 양안 거리 측정 (stereo extrinsic)
  3. 안경 처방 (보정값 적용)
  4. 재검사로 확인 (검증)

Kalibr 캘리브레이션도 동일:
  1. 카메라 내부 파라미터 측정
  2. Camera-IMU 외부 파라미터 측정
  3. Config 파일에 적용
  4. VIO 실행으로 검증
```

### 전주(Week 13)와의 연결

| Week 13 | → | Week 14 |
|---------|---|---------|
| Extrinsic 이론과 중요성 | → | 실제로 측정하는 방법 |
| 핸드-아이 캘리브레이션 수학 | → | Kalibr가 내부에서 수행 |
| 시간 오프셋 개념 | → | Kalibr의 td 추정 |

---

## 📚 핵심 개념

### 1. Kalibr 개요

```
Kalibr = ETH Zürich에서 개발한 캘리브레이션 도구

지원하는 캘리브레이션:
  1. Camera Intrinsic (단일/다중 카메라)
  2. Camera-Camera Extrinsic (Stereo)
  3. Camera-IMU Extrinsic + Time Offset
  4. IMU 노이즈 파라미터 검증

입력:
  - Camera 영상 (ROS bag)
  - IMU 데이터 (ROS bag)
  - 타겟 정보 (AprilGrid / Checkerboard)

출력:
  - Camera intrinsic (fx, fy, cx, cy, distortion)
  - Camera-IMU extrinsic (R, t)
  - Time offset (td)
  - 결과 리포트 (reprojection error 등)
```

### 2. 데이터 수집 가이드

#### 준비물

```
필수:
  - AprilGrid 타겟 (A3 크기 이상 추천)
  - Camera + IMU가 단단히 고정된 센서 모듈
  - ROS + rosbag record

AprilGrid 설정 예시 (april_6x6.yaml):
  target_type: 'aprilgrid'
  tagCols: 6
  tagRows: 6
  tagSize: 0.088     # 태그 한 변 길이 (m)
  tagSpacing: 0.3    # 태그 간 간격 비율
```

#### 촬영 방법

```
핵심 원칙: "다양한 운동"

1. 모든 축 회전 (Roll, Pitch, Yaw)
   → 각 축 30-45도 이상
   → 관측성 확보의 핵심

2. 다양한 거리
   → 가깝게 (화면 가득 채우기)
   → 멀리 (전체가 보이게)

3. 다양한 위치
   → 화면 중앙, 모서리, 가장자리

4. 60-120초 촬영
   → 너무 짧으면 데이터 부족
   → 너무 길면 처리 시간 증가

5. 일정한 속도 유지
   → 급격한 움직임 피하기 (blur 방지)
   → IMU 한계 내에서 움직이기

나쁜 예:
  ✗ 한 방향으로만 흔들기
  ✗ 너무 빠르게 움직이기
  ✗ 타겟이 화면에서 자주 사라짐
```

### 3. 캘리브레이션 실행

#### Step 1: Camera 캘리브레이션

```bash
# 카메라 intrinsic 먼저 캘리브레이션
kalibr_calibrate_cameras \
    --bag camera_imu.bag \
    --topics /cam0/image_raw \
    --models pinhole-equi \
    --target april_6x6.yaml

# 출력: camchain-camera_imu.yaml
```

#### Step 2: Camera-IMU 캘리브레이션

```bash
# IMU 노이즈 파라미터 파일 (imu.yaml)
# acc_n: 가속도계 noise density
# acc_w: 가속도계 random walk
# gyr_n: 자이로 noise density
# gyr_w: 자이로 random walk

kalibr_calibrate_imu_camera \
    --bag camera_imu.bag \
    --cam camchain-camera_imu.yaml \
    --imu imu.yaml \
    --target april_6x6.yaml

# 출력: camchain-imucam-camera_imu.yaml + 리포트
```

### 4. 결과 해석

#### 출력 YAML 파일 예시

```yaml
cam0:
  T_cam_imu:            # Camera-IMU Extrinsic (4x4)
  - [0.9999, -0.0123, 0.0045, 0.0312]
  - [0.0124,  0.9998, -0.0134, -0.0052]
  - [-0.0043, 0.0135, 0.9999, 0.0021]
  - [0.0, 0.0, 0.0, 1.0]
  timeshift_cam_imu: -0.00856    # td (초)
  camera_model: pinhole
  intrinsics: [461.6, 460.3, 362.7, 248.1]
  distortion_model: equidistant
  distortion_coeffs: [-0.0027, 0.0312, -0.0479, 0.0270]
```

#### 결과 검증 기준

```
좋은 캘리브레이션:
  ✓ Reprojection error: < 0.5 pixel (이상적으로 < 0.3)
  ✓ IMU error: 합리적 범위
  ✓ td: |td| < 50ms (하드웨어에 따라)
  ✓ R_ci: 물리적으로 합리적 (센서 배치와 일치)
  ✓ t_ci: 실측값과 ±1cm 이내

나쁜 캘리브레이션:
  ✗ Reprojection error > 1.0 pixel
  ✗ t_ci가 물리적으로 불가능한 값
  ✗ 수렴하지 않음 (반복마다 값이 크게 변함)
```

### 5. 결과를 VINS에 적용

```yaml
# VINS config 파일에 반영

# Camera intrinsic
model_type: PINHOLE
camera_name: camera
image_width: 752
image_height: 480
distortion_parameters:
   k1: -0.0027
   k2: 0.0312
   p1: -0.0479
   p2: 0.0270
projection_parameters:
   fx: 461.6
   fy: 460.3
   cx: 362.7
   cy: 248.1

# Camera-IMU Extrinsic
extrinsicRotation: !!opencv-matrix
   rows: 3
   cols: 3
   data: [0.9999, -0.0123, 0.0045,
          0.0124,  0.9998, -0.0134,
         -0.0043,  0.0135,  0.9999]
extrinsicTranslation: !!opencv-matrix
   rows: 3
   cols: 1
   data: [0.0312, -0.0052, 0.0021]

# Time offset
td: -0.00856
estimate_td: 1    # 온라인으로도 추가 추정
```

### 6. 트러블슈팅

| 문제 | 가능한 원인 | 해결 방법 |
|------|------------|----------|
| 높은 reprojection error | 타겟 크기 오류 | tagSize 재확인 |
| | 모션 블러 | 더 천천히 움직이기 |
| | 타겟 검출 부족 | 조명 개선, 해상도 확인 |
| 비수렴 | 데이터 불충분 | 더 다양한 운동으로 재촬영 |
| | IMU 노이즈 파라미터 부정확 | Allan Variance로 재측정 |
| 비합리적 t_ci | 단위 오류 | tagSize 단위 확인 (미터) |
| 큰 td | 하드웨어 동기화 문제 | 동기화 트리거 확인 |

---

## 🤔 자체 점검 퀴즈

1. **AprilGrid vs Checkerboard의 장단점은?**
   - AprilGrid: 부분 검출 가능, ID 기반, 더 robust
   - Checkerboard: 단순, 코너 검출이 더 정확할 수 있음

2. **Reprojection error 0.8 pixel은 좋은 결과인가?**
   - 보통 수준, 이상적으로는 0.5 미만
   - 재촬영하거나 타겟 크기 확인 권장

3. **IMU 노이즈 파라미터가 결과에 미치는 영향은?**
   - 최적화의 가중치에 영향
   - 부정확하면 extrinsic 추정도 부정확

4. **캘리브레이션 결과를 VIO에서 최종 검증하는 방법은?**
   - VINS 실행 후 궤적 정확도 확인
   - 루프가 있는 경로에서 closure 확인

---

## ✅ 이번 주 체크리스트

- [ ] Kalibr 설치 (Docker 또는 native)
- [ ] AprilGrid 타겟 준비 및 YAML 작성
- [ ] 데이터 수집 가이드라인 숙지 (다양한 운동)
- [ ] Camera 캘리브레이션 실행 및 결과 확인
- [ ] Camera-IMU 캘리브레이션 실행
- [ ] 결과 YAML 파일 해석 (R, t, td)
- [ ] Reprojection error 검증 (< 0.5 pixel)
- [ ] VINS config 파일에 결과 반영

---

## 📝 핵심 요약

```
┌─────────────────────────────────────────────────────────┐
│  Kalibr Camera-IMU 캘리브레이션                          │
│                                                         │
│  준비:                                                   │
│    AprilGrid 타겟 + Camera/IMU 고정 + ROS bag            │
│                                                         │
│  데이터 수집:                                             │
│    모든 축 회전 + 다양한 거리/위치 + 60~120초             │
│                                                         │
│  실행:                                                    │
│    1. kalibr_calibrate_cameras → intrinsic               │
│    2. kalibr_calibrate_imu_camera → extrinsic + td       │
│                                                         │
│  검증:                                                    │
│    reprojection error < 0.5 pixel                        │
│    R, t가 물리적 배치와 일치                              │
│    |td| 합리적 범위                                      │
│                                                         │
│  적용:                                                    │
│    결과를 VINS config YAML에 반영 → VIO 실행 검증         │
└─────────────────────────────────────────────────────────┘
```

---

이전: [Week 13 - 외부 캘리브레이션](../week13/README.md)
다음: [Phase 5 - Detection + Depth Estimation](../../../Roadmap/Phase%205.md)
