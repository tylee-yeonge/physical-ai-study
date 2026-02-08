# Week 14 실습: Kalibr 캘리브레이션

## 실습 목표
- Kalibr 설치 및 실행 환경 구축
- Camera-IMU 캘리브레이션 과정 체험
- 캘리브레이션 결과 분석 및 검증

---

## 빌드 방법

```bash
cd Studies/Phase\ 4/week14
mkdir -p build && cd build
cmake .. && make
./quiz_easy      # 개념 퀴즈
./quiz_medium    # 계산 퀴즈
```

---

## 실습 1: Kalibr 설치

### Docker 방식 (추천)

```bash
# Kalibr Docker 이미지 가져오기
docker pull stereolabs/kalibr

# 또는 Dockerfile로 빌드
git clone https://github.com/ethz-asl/kalibr.git
cd kalibr
docker build -t kalibr .

# 실행
docker run -it --rm \
  -v $(pwd)/data:/data \
  kalibr bash
```

### Native 빌드 (Ubuntu 20.04/22.04)

```bash
# 의존성 설치
sudo apt-get install -y \
  python3-catkin-tools \
  python3-osrf-pycommon \
  libopencv-dev

# 워크스페이스 설정
mkdir -p ~/kalibr_ws/src && cd ~/kalibr_ws
catkin init
cd src
git clone https://github.com/ethz-asl/kalibr.git
cd ..
catkin build -j4
source devel/setup.bash
```

---

## 실습 2: AprilGrid 타겟 준비

### YAML 설정 파일

```yaml
# april_6x6.yaml
target_type: 'aprilgrid'
tagCols: 6        # 열 수
tagRows: 6        # 행 수
tagSize: 0.088    # 태그 한 변 길이 (미터) - 실측값!
tagSpacing: 0.3   # 태그 간 간격 / 태그 크기 비율
```

### 타겟 생성 및 인쇄

```bash
# Kalibr에서 타겟 PDF 생성
kalibr_create_target_pdf --type apriltag \
  --nx 6 --ny 6 \
  --tsize 0.088 --tspace 0.3

# A3 크기로 인쇄
# ⚠️ 중요: 확대/축소 없이 '실제 크기'로 인쇄!
# 인쇄 후 실제 tagSize를 자로 재서 YAML에 기입
```

---

## 실습 3: IMU 노이즈 파라미터

### imu.yaml 파일

```yaml
# imu.yaml
rostopic: /imu0
update_rate: 200.0    # IMU 주파수 (Hz)

accelerometer_noise_density: 0.01    # m/s^2/√Hz (acc_n)
accelerometer_random_walk: 0.0002   # m/s^3/√Hz (acc_w)
gyroscope_noise_density: 0.005      # rad/s/√Hz (gyr_n)
gyroscope_random_walk: 3.0e-5       # rad/s^2/√Hz (gyr_w)
```

### 파라미터 출처

```
방법 1: 데이터시트 참고
  → 제조사 스펙 (대략적)

방법 2: Allan Variance 분석
  → 정지 상태에서 장시간 (1시간+) 측정
  → kalibr_allan 또는 imu_utils 패키지 사용
  → 가장 정확

방법 3: 경험적 값 사용
  → MEMS (저가): acc_n ≈ 0.01~0.05, gyr_n ≈ 0.005~0.01
  → Tactical grade: acc_n ≈ 0.001~0.005, gyr_n ≈ 0.0005~0.002
```

---

## 실습 4: 데이터 수집 및 캘리브레이션

### 데이터 수집 (ROS bag)

```bash
# rosbag 녹화
rosbag record /cam0/image_raw /imu0 -O camera_imu.bag

# 촬영 가이드:
#   1. 타겟을 평평한 벽에 부착
#   2. 센서를 손으로 들고 다양하게 움직이기
#   3. 60~120초 촬영
#   4. 모든 축 회전 + 다양한 거리
```

### 실행

```bash
# Step 1: Camera intrinsic
kalibr_calibrate_cameras \
    --bag camera_imu.bag \
    --topics /cam0/image_raw \
    --models pinhole-equi \
    --target april_6x6.yaml \
    --dont-show-report

# Step 2: Camera-IMU extrinsic
kalibr_calibrate_imu_camera \
    --bag camera_imu.bag \
    --cam camchain-camera_imu.yaml \
    --imu imu.yaml \
    --target april_6x6.yaml \
    --dont-show-report
```

---

## 실습 5: 결과 분석

### 핵심 코드: 결과 파싱 및 검증

```cpp
#include <Eigen/Dense>
#include <iostream>
#include <cmath>

using namespace Eigen;

/**
 * Kalibr 결과 검증
 *
 * T_cam_imu (4x4 변환 행렬)에서 R_ci, t_ci 추출 후 검증
 */
void validateCalibResult(const Matrix4d& T_cam_imu) {
    Matrix3d R_ci = T_cam_imu.block<3,3>(0,0);
    Vector3d t_ci = T_cam_imu.block<3,1>(0,3);

    std::cout << "=== 캘리브레이션 결과 검증 ===\n" << std::endl;

    // 1. 회전 행렬 유효성
    double det = R_ci.determinant();
    double orthogonality = (R_ci * R_ci.transpose() - Matrix3d::Identity()).norm();
    std::cout << "회전 행렬 검증:" << std::endl;
    std::cout << "  det(R) = " << det << " (기대: 1.0)" << std::endl;
    std::cout << "  ||R*R^T - I|| = " << orthogonality << " (기대: ≈0)\n" << std::endl;

    // 2. 병진 크기 확인
    std::cout << "병진 벡터:" << std::endl;
    std::cout << "  t = [" << t_ci.transpose() << "] m" << std::endl;
    std::cout << "  ||t|| = " << t_ci.norm() << " m" << std::endl;
    std::cout << "  (물리적 센서 간 거리와 비교)\n" << std::endl;

    // 3. 회전을 angle-axis로 변환
    AngleAxisd aa(R_ci);
    std::cout << "회전 (angle-axis):" << std::endl;
    std::cout << "  각도: " << aa.angle() * 180.0 / M_PI << " 도" << std::endl;
    std::cout << "  축: [" << aa.axis().transpose() << "]" << std::endl;
    std::cout << "  (센서 배치와 일치하는지 확인)\n" << std::endl;
}

/**
 * Reprojection error 통계 분석
 */
void analyzeReprojectionError(
    const std::vector<double>& errors
) {
    int n = errors.size();
    if (n == 0) return;

    // 평균
    double mean = 0;
    for (double e : errors) mean += e;
    mean /= n;

    // 표준편차
    double var = 0;
    for (double e : errors) var += (e - mean) * (e - mean);
    var /= (n - 1);
    double std_dev = std::sqrt(var);

    // 최대값
    double max_err = *std::max_element(errors.begin(), errors.end());

    // 중앙값
    std::vector<double> sorted = errors;
    std::sort(sorted.begin(), sorted.end());
    double median = sorted[n/2];

    std::cout << "Reprojection Error 분석:" << std::endl;
    std::cout << "  N = " << n << " 점" << std::endl;
    std::cout << "  평균: " << mean << " pixel" << std::endl;
    std::cout << "  중앙값: " << median << " pixel" << std::endl;
    std::cout << "  표준편차: " << std_dev << " pixel" << std::endl;
    std::cout << "  최대값: " << max_err << " pixel\n" << std::endl;

    // 판정
    if (mean < 0.3) {
        std::cout << "  → 우수한 결과!" << std::endl;
    } else if (mean < 0.5) {
        std::cout << "  → 양호한 결과" << std::endl;
    } else if (mean < 1.0) {
        std::cout << "  → 보통 (개선 권장)" << std::endl;
    } else {
        std::cout << "  → 불량 (재캘리브레이션 필요)" << std::endl;
    }
}
```

---

## 체크리스트: EuRoC 데이터셋으로 연습

```
Kalibr를 직접 실행하기 어려운 경우,
EuRoC MAV 데이터셋의 캘리브레이션 결과를 분석해보세요.

1. EuRoC 캘리브레이션 데이터 확인:
   → sensor.yaml 파일에 intrinsic, extrinsic 포함
   → T_BS: Body(IMU) → Sensor(Camera) 변환

2. 값들을 추출하여 위 코드로 검증:
   → det(R) ≈ 1, R*R^T ≈ I
   → t의 크기가 센서 물리적 배치와 일치

3. VINS-Mono config와 비교:
   → euroc_config.yaml의 extrinsic 값과 일치 확인
```

---

## 참고 자료

- Kalibr 공식 wiki: https://github.com/ethz-asl/kalibr/wiki
- EuRoC MAV Dataset: https://projects.asl.ethz.ch/datasets/doku.php?id=kmavvisualinertialdatasets
- VINS-Mono: https://github.com/HKUST-Aerial-Robotics/VINS-Mono

---

이전: [Week 13 PRACTICE](../week13/PRACTICE.md)
다음: [Phase 5 Week 1 PRACTICE](../../Phase%205/week1/PRACTICE.md)
