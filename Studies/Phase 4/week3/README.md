# Week 3: VIO 초기화 + Camera-IMU 캘리브레이션

> 🎯 **목표**: VIO 시작 시 왜 초기화가 필요한지, Camera-IMU 캘리브레이션의 중요성 이해
> ⏰ **예상 시간**: 7-10시간
> 💡 **핵심 질문**: "VIO 시작할 때 모르는 것들을 어떻게 알아내는가?"

---

## 📚 핵심 개념

### 1. VIO 초기화

#### 시작 시 모르는 것들

| 미지수 | 왜 모르는가? |
|--------|------------|
| **스케일** (Monocular) | 카메라는 비율만 알 수 있음 |
| **중력 방향** | 센서를 어느 방향으로 잡았는지 모름 |
| **IMU 바이어스** | 센서마다, 온도마다 다름 |
| **초기 속도** | 시작 순간 움직이고 있을 수 있음 |

#### 초기화 3단계

**Stage 1: Vision-only SfM**
- 순수 VO로 up-to-scale 궤적 추정
- Feature tracking → 초기 맵 생성
- 이 시점에서는 스케일이 임의값

**Stage 2: Visual-Inertial Alignment**
- Vision 궤적과 IMU pre-integration을 정렬
- 선형 시스템으로 풀기:
  - 스케일 s
  - 중력 방향 g
  - 속도 v
  - 자이로 바이어스 b_g

**Stage 3: 검증 및 정제**
- Reprojection error 확인
- IMU-Vision 일관성 검증
- 필요시 비선형 최적화로 정제

#### VINS 코드 연결

- `initial_sfm.cpp`: Vision-only 초기화
- `initial_alignment.cpp`: Visual-Inertial 정렬
- `initial_ex_rotation.cpp`: Extrinsic 초기 추정

---

### 2. Camera-IMU 캘리브레이션

#### Extrinsic Calibration

```
Camera Frame ←──[R_ic, t_ic]──→ IMU Frame
```

- Camera와 IMU 사이의 상대 포즈 (R, t)
- **잘못된 extrinsic → VIO 발산**
- mm 단위 오차도 성능 저하, 특히 회전이 민감

#### 시간 동기화 (Time Offset)

- Camera와 IMU 타임스탬프 차이 (td)
- 보통 수십 ms
- VINS는 td도 온라인으로 추정 가능

#### Kalibr

- Camera-IMU 캘리브레이션 표준 도구
- AprilGrid 타겟 + 다양한 축 회전 필요
- 결과: Camera intrinsic + Camera-IMU extrinsic + time offset
- 결과를 VINS config에 반영

```yaml
# VINS config 예시
body_T_cam0:  # IMU → Camera 변환
  - [0.0148655429818, -0.999880929698, ...]
  - [...]
```

---

### 3. Phase 4 전체 정리

```
Week 1: IMU 기초
  - 가속도계/자이로 측정 모델
  - 노이즈, 바이어스
  - Vision과 상호 보완

Week 2: Pre-integration
  - 포즈 독립적 상대 측정값
  - Factor Graph에서의 역할
  - VINS integration_base.h

Week 3: 초기화 + 캘리브레이션
  - VIO 시작 시 미지수 추정
  - Camera-IMU extrinsic의 중요성
```

**Phase 4 완료 기준:**
> "IMU와 Vision이 왜 상호보완적인지, Pre-integration 개념을 직관적으로 설명 가능"

---

## 🔍 자체 점검

1. VIO 초기화에서 스케일을 어떻게 알아내는가?
2. 중력 방향을 왜 추정해야 하는가?
3. Camera-IMU extrinsic이 틀리면 어떤 현상이 나타나는가?
4. Phase 3-4 전체를 통해, VINS의 프론트엔드/백엔드/초기화를 설명할 수 있는가?

---

Phase 4 완료! 다음: Phase 5 (Detection + Depth Estimation)
