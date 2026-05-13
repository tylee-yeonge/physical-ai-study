# Week 3 실습: VIO 초기화 코드 읽기 + Phase 4 마무리


> **목표**: VINS 초기화 코드 구조 파악, Phase 4 전체 복습
> **방식**: 코드 분석 중심
> **예상 시간**: 3-4시간


---


## 실습 1: VINS 초기화 코드 분석


### 확인할 파일


```
vins_estimator/src/initial/
+-- initial_sfm.cpp # Vision-only SfM
+-- initial_sfm.h
+-- initial_alignment.cpp # Visual-Inertial Alignment
+-- initial_alignment.h
+-- initial_ex_rotation.cpp # Extrinsic 초기 추정
```


### initial_sfm.cpp 분석 포인트


1. SfM 초기화 과정: 특징점으로 상대 포즈 추정
2. 삼각측량으로 3D 맵 생성
3. PnP로 추가 프레임 포즈 추정


### initial_alignment.cpp 분석 포인트


1. `solveGyroscopeBias()`: 자이로 바이어스 추정
   - Vision 회전과 IMU 회전 비교
2. `LinearAlignment()`: 스케일, 중력, 속도 추정
   - 선형 시스템 구성 및 풀기
3. 중력 방향 정제: 크기 9.81 m/s^2 제약


---


## 실습 2: Config 파일에서 Extrinsic 확인


VINS config 파일(예: `euroc_stereo_config.yaml`)에서:


1. `body_T_cam0`: IMU → Camera 변환 행렬
   - 회전 부분(3x3)과 평행이동 부분(3x1) 분리
2. `estimate_extrinsic`: 0(고정) / 1(초기값 제공, 정제) / 2(완전 추정)
3. `td`: time offset 초기값


---


## 실습 3: Extrinsic과 Time Offset 확인 (30분)


VINS config에서 Camera-IMU 관계를 확인:


```yaml
# body_T_cam0: IMU body frame → Camera 0 변환
body_T_cam0: !!opencv-matrix
  rows: 4
  cols: 4
  data: [0.0148655429818, -0.999880929698, 0.00414029679422, -0.0216401454975,
         0.999557249008, 0.0149672133247, 0.025715529948, -0.064676986768,
         -0.0257744366974, 0.00375618835797, 0.999660727178, 0.00981073058949,
         0.0, 0.0, 0.0, 1.0]


# estimate_extrinsic: 0=고정, 1=초기값+정제, 2=완전 추정
estimate_extrinsic: 1


# td: Camera-IMU 시간 오프셋
td: 0.0
estimate_td: 1 # 온라인 추정 여부
```


**확인할 것:**
- [ ] 4x4 행렬에서 회전(3x3)과 평행이동(3x1) 분리
- [ ] `estimate_extrinsic` 옵션에 따른 동작 차이
- [ ] `td`가 0이 아닌 경우 어떤 보정이 필요한지


---


## 실습 4: Phase 3-4 종합 면접 대비 (1시간)


아래 질문에 **자기 말로** 답을 노트에 작성하세요. 이것이 이직 준비의 핵심 자산입니다.


### VO & BA (Phase 3)


1. **"VO 파이프라인을 설명해주세요"**
   - 6단계, 각 단계의 역할, VINS에서 어디에 해당하는지


2. **"Bundle Adjustment가 무엇인가요?"**
   - 비용 함수, 최적화 대상, Schur complement


3. **"Monocular VO의 한계는?"**
   - 스케일 모호성, 해결 방법 (IMU, Stereo)


### VIO (Phase 4)


4. **"IMU가 Vision에 제공하는 정보는?"**
   - 스케일, 빠른 모션, 프레임 간 보간, 초기화 지원


5. **"Pre-integration이 왜 필요한가요?"**
   - Naive 적분의 비효율성, 포즈 독립적 상대 측정값


6. **"VIO 초기화에서 추정하는 것들은?"**
   - 스케일, 중력, 속도, 자이로 바이어스


### 전체 연결


7. **"VINS-Fusion의 구조를 설명해주세요"**
   ```
   프론트엔드: feature_tracker (FAST + KLT)
   초기화: SfM → VI Alignment (스케일, 중력, 바이어스)
   백엔드: Ceres (Visual factor + IMU factor + Marginalization)
   Loop closure: 4-DOF 포즈 그래프 최적화
   ```


8. **"Camera-IMU 캘리브레이션이 왜 중요한가요?"**
   - extrinsic 오차 → VIO 발산, 시간 동기화 문제


---


## 체크리스트


- [ ] VINS 초기화 코드 (initial_sfm, initial_alignment) 구조 파악
- [ ] solveGyroscopeBias()의 역할 이해
- [ ] Camera-IMU extrinsic 파라미터 확인
- [ ] **Phase 3-4 면접 질문 8개 모두 자기 말로 답 작성 완료**
- [ ] Phase 5 시작 준비 (Python/PyTorch 환경 확인)
