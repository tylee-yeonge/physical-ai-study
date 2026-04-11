# Week 4 실습: 스케일 문제 확인 + VINS 전체 구조 읽기

> [goal] **목표**: 스케일 드리프트를 코드로 확인하고, VINS 전체 구조를 파악
> [code] **방식**: 코드 분석 중심
> [time] **예상 시간**: 3-4시간

---

## 실습 1: VINS 전체 코드 구조 파악

### 파일 트리 확인

```
vins_estimator/src/
+-- estimator.cpp/.h          # 핵심: 상태 추정 메인 클래스
+-- estimator_node.cpp        # ROS 노드 진입점
+-- parameters.cpp/.h         # config 파라미터 로드
+-- feature_manager.cpp/.h    # 특징점 생명주기 관리
+-- factor/
|   +-- imu_factor.h          # IMU factor (Phase 4)
|   +-- projection_factor.cpp # Visual factor (재투영 오차)
|   +-- marginalization_factor.cpp
+-- initial/
|   +-- initial_sfm.cpp       # Vision-only 초기화
|   +-- initial_alignment.cpp # Visual-Inertial 정렬
+-- utility/
```

### 분석 포인트

1. **estimator.cpp**: `processImage()` 흐름 따라가기
   - 특징점 수신 → 키프레임 판단 → 초기화/최적화
2. **feature_manager.cpp**: 특징점이 어떻게 관리되는지
   - 추가, 삭제, 키프레임 연결
3. **parameters.cpp**: config 파일에서 어떤 값을 로드하는지

---

## 실습 2: Phase 3 전체 복습 정리

아래 질문에 자신만의 답을 작성해보세요 (면접 대비):

1. **VO 파이프라인을 설명해주세요**
   - 6단계 흐름 + 각 단계의 역할

2. **BA가 무엇인가요?**
   - 비용 함수, 최적화 대상, Schur complement

3. **Monocular VO의 한계와 해결 방법은?**
   - 스케일 모호성 → IMU/Stereo

4. **VINS-Fusion의 구조를 설명해주세요**
   - 프론트엔드(feature tracker) + 백엔드(Ceres 최적화)

---

## 실습 3: VINS 전체 흐름 추적 (핵심!)

`estimator.cpp`의 `processImage()` 함수를 따라가며 전체 흐름을 파악하세요:

```
processImage() 호출
+-- addFeatureCheckParallax()  # 키프레임 여부 판단
+-- if (!INIT_DONE)
|   +-- initialStructure()     # SfM + VI 정렬 (초기화)
+-- else
    +-- optimization()         # Ceres BA (추적 모드)
        +-- IMU factor
        +-- Visual factor
        +-- Marginalization factor
```

**확인할 것:**
- [ ] 초기화 전/후 분기 구조
- [ ] 키프레임이면 sliding window에 추가, 아니면 교체
- [ ] optimization() 호출 후 결과가 어디로 출력되는지

---

## 실습 4: Phase 4 준비

Phase 4에서 배울 IMU 관련 코드를 미리 확인:

- `factor/imu_factor.h`: IMU factor의 구조 훑기 (멤버 변수만 확인)
- `utility/integration_base.h`: Pre-integration 클래스 존재 확인
- config 파일에서 IMU 관련 파라미터 (`acc_n`, `gyr_n`, `acc_w`, `gyr_w`) 확인

---

## 체크리스트

- [ ] VINS 전체 코드 트리 파악
- [ ] processImage() → optimization() 흐름 이해
- [ ] Phase 3 면접 질문 4개에 자기 말로 답 작성 완료
- [ ] Phase 4 IMU 코드 위치 사전 확인
