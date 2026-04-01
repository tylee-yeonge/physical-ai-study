# Phase 4: VIO 실습 개요

> 🎯 **목표**: IMU와 Vision 융합의 핵심 직관적 이해
> 💻 **언어**: C++ (VINS 코드 읽기 중심)
> ⏰ **예상 시간**: 약 25시간 (3주)
> 📌 각 주차별 상세 실습은 `week{N}/PRACTICE.md`를 참고하세요.

---

## 주차별 실습 요약

| Week | 주제 | 실습 내용 |
|------|------|----------|
| 1 | IMU 기초 + 센서 융합 | VINS config 파라미터 확인, IMU 적분 드리프트 체험 |
| 2 | Pre-integration + Factor Graph | VINS integration_base.h / imu_factor.h 코드 분석 |
| 3 | VIO 초기화 + 캘리브레이션 | VINS initial_sfm / initial_alignment 코드 분석 |

---

## 핵심 분석 대상 파일

```
vins_estimator/src/
├── factor/
│   ├── imu_factor.h              # IMU Factor (Week 2)
│   └── projection_factor.cpp     # Visual Factor (Phase 3 복습)
├── utility/
│   └── integration_base.h        # Pre-integration (Week 2)
├── initial/
│   ├── initial_sfm.cpp           # Vision-only 초기화 (Week 3)
│   └── initial_alignment.cpp     # VI 정렬 (Week 3)
└── parameters.cpp                # IMU 노이즈 파라미터 (Week 1)
```
