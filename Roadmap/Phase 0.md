# Phase 0: 역순 시작 - VINS-Fusion 먼저 돌려보기

> ⏰ **기간**: 2주  
> 🎯 **목표**: 먼저 돌려보고, 모르는 것을 파악하여 학습 동기 확보

---

## 📋 Week 1: 환경 구축

### 1.1 개발 환경 준비

#### Ubuntu 환경 선택
- [ ] **Option A**: Ubuntu 22.04 네이티브 설치
- [ ] **Option B**: Docker 컨테이너 환경 구성
- [x] **Option C**: 기존 ROS2 개발환경 활용

> 💡 이미 ROS2 개발환경이 있다면 그대로 활용하세요

#### 필수 패키지 설치
- [ ] ROS 설치 확인 (ROS1 Noetic 또는 ROS2 Humble)
- [ ] OpenCV 설치 확인 (4.x)
- [ ] Eigen3 설치 확인
- [ ] Ceres Solver 설치

### 1.2 ROS 버전 결정

> ⚠️ VINS-Fusion 원본은 **ROS1 기반**입니다. 아래 중 하나 선택:

#### Option A: ROS1 사용 (권장 - 가장 안정적)
- [x] ROS1 Noetic 설치
- [x] 원본 VINS-Fusion 사용

#### Option B: ROS2 + Bridge
- [ ] ROS2 Humble 환경에서 ros1_bridge 설정
- [ ] ROS1 노드를 ROS2에서 실행

#### Option C: ROS2 포팅 버전
- [ ] ROS2 포팅된 VINS-Fusion 검색 (커뮤니티 버전)
- [ ] 안정성 확인 필요

> 💡 Phase 0에서는 **Option A (ROS1)**로 빠르게 돌려보는 것을 권장. ROS2 통합은 Phase 6에서 진행.

### 1.3 VINS-Fusion 빌드

#### 소스코드 준비
- [x] VINS-Fusion 저장소 클론 (~~`HKUST-Aerial-Robotics/VINS-Fusion`~~, [stevenf7/VINS-Fusion](https://github.com/stevenf7/VINS-Fusion))
    - 
- [x] 의존성 패키지 설치

#### 빌드
- [x] catkin 빌드 실행 (ROS1)
- [x] 빌드 에러 해결 (있다면 기록)
    - ceres 1.14.0 설치 (ARM 버전에서 Abseil 의존성 문제 발생, 해당 의존성 제거 버전)
- [x] 빌드 성공 확인

### 1.4 평가 도구 설치

#### evo 설치
- [x] `pip install evo --upgrade`
- [x] 설치 확인: `evo_ape --help`

> 💡 evo는 궤적 평가의 표준 도구. ATE, RPE 등을 계산해줌.

### ~~1.5 카메라 드라이버 (선택사항)~~

#### ~~Realsense 사용 시~~
- [ ] ~~librealsense 설치~~
- [ ] ~~realsense-ros 패키지 설치~~
- [ ] ~~카메라 연결 테스트 (`realsense-viewer`)~~

#### ~~다른 카메라 사용 시~~
- [ ] ~~해당 카메라 ROS 드라이버 설치~~
- [ ] ~~토픽 발행 확인~~

---

## 📋 Week 2: VINS-Fusion 첫 실행

### 2.1 데이터셋 준비

#### EuRoC 데이터셋 다운로드
- [x] EuRoC MAV Dataset 웹사이트 접속
- [x] `MH_01_easy` 시퀀스 다운로드 (가장 쉬운 시퀀스)
- [x] ROS bag 형식 선택 권장
- [x] 데이터 압축 해제 및 경로 확인

#### 데이터셋 구조 파악
- [x] 카메라 이미지 토픽 확인
- [x] IMU 데이터 토픽 확인
- [x] Ground truth 데이터 확인 (`state_groundtruth_estimate0`)
- [x] 캘리브레이션 파일 확인

### 2.2 VINS-Fusion 실행

#### 설정 파일 확인
- [x] EuRoC용 config 파일 찾기 (`config/euroc/`)
- [x] 카메라 파라미터 확인
- [x] IMU 파라미터 확인

#### 실행
- [x] VINS-Fusion 노드 실행
- [x] RViz 시각화 실행
- [x] 데이터셋 재생 (`rosbag play MH_01_easy.bag`)

#### 결과 확인
- [x] 궤적이 RViz에 표시되는지 확인
- [x] 특징점 트래킹 시각화 확인
- [x] VINS 추정 궤적 저장 (TUM 또는 EuRoC 형식)

### 2.3 evo로 첫 평가

#### 궤적 비교
- [x] Ground truth와 추정 궤적 형식 맞추기
- [x] ATE 계산: `evo_ape euroc gt.csv estimate.csv -va --plot`
- [x] 결과 스크린샷 저장
- [x] 대략적인 오차 수준 파악 (숫자 자체보다 "돌아간다"가 중요)

> 💡 이 단계에서 정확한 숫자는 중요하지 않음. "평가하는 방법을 안다"가 목표.

### 2.4 모르는 개념 목록 작성 ⭐

> 이게 가장 중요합니다. 이 목록이 Phase 1-4의 학습 가이드가 됩니다.

#### 관찰하면서 질문 작성
- [x] Feature Tracker 창에서 점들이 뭘 의미하는지?
- [x] IMU data가 어떻게 Vision과 합쳐지는지?
- [x] Loop closure가 뭔지?
- [x] Marginalization이 뭔지?
- [x] Pre-integration이 뭔지?
- [x] Factor graph가 뭔지?
- [x] ATE, RPE가 정확히 뭘 측정하는지?

#### 코드 훑어보면서 질문 작성
- [x] `Eigen::Quaterniond`가 뭔지?
- [x] `SE3`, `SO3`가 뭔지?
- [x] `ceres::Problem`이 뭔지?
- [x] `imu_factor`가 뭘 하는지?

---

## ✅ Phase 0 완료 체크리스트

### 환경
- [x] VINS-Fusion 빌드 성공
- [x] EuRoC 데이터셋 다운로드 완료
- [x] evo 툴 설치 완료

### 실행
- [x] VINS-Fusion + EuRoC 실행 성공
- [x] RViz에서 궤적 확인
- [x] evo로 Ground truth 비교 실행  

### 학습 준비
- [x] **"모르는 개념 목록"** 작성 완료 (최소 10개)
- [x] 우선순위 정하기 (가장 궁금한 것부터)

---

## 🎯 Phase 0 완료 기준

> "VINS-Fusion이 돌아가는 것을 보았고, 무엇을 모르는지 안다. evo로 평가하는 방법도 안다."

---

## 📚 참고 자료

### VINS-Fusion
- GitHub: `HKUST-Aerial-Robotics/VINS-Fusion`
- 원본 논문: "VINS-Mono: A Robust and Versatile Monocular Visual-Inertial State Estimator"

### EuRoC 데이터셋
- https://projects.asl.ethz.ch/datasets/euroc-mav/

### evo 툴
- GitHub: `MichaelGrupp/evo`
- 문서: https://github.com/MichaelGrupp/evo/wiki

### 난이도별 시퀀스

| 시퀀스 | 난이도 | 특징 |
|--------|--------|------|
| MH_01_easy | ⭐ | 느린 움직임, 밝은 조명 |
| MH_02_easy | ⭐ | 느린 움직임 |
| MH_03_medium | ⭐⭐ | 중간 속도 |
| MH_04_difficult | ⭐⭐⭐ | 빠른 움직임 |
| MH_05_difficult | ⭐⭐⭐ | 빠른 움직임, 조명 변화 |

---

## 💡 팁

1. **완벽하게 이해하려 하지 마세요** - 이 단계의 목표는 "돌려보기"입니다
2. **에러가 나면 기록하세요** - 해결한 에러도 나중에 도움이 됩니다
3. **질문 목록을 많이 만드세요** - 이게 다음 Phase의 로드맵이 됩니다
4. **스크린샷 찍어두세요** - 블로그 또는 문서화에 활용
5. **ROS1으로 시작하세요** - ROS2 통합은 Phase 6에서 해도 늦지 않음

---

## ❓ 다음 단계

Phase 0 완료 후:
- "모르는 개념 목록"을 바탕으로 Phase 1-4에서 우선순위 조정
- 가장 궁금한 개념부터 Phase 1 시작