# Studies/Hardware-Arm — 자작 팔 트랙


> **기간**: 2026.10-2027.06 (스파이크 2-3주 + Stage 1: 2개월 + Stage 2: 3개월)
> **마스터 가이드**: [`Roadmap/Hardware-Arm.md`](../../Roadmap/Hardware-Arm.md)
> **단계별 가이드 미리 작성됨** — 진입 시 (스파이크: 2026.10, Stage 1: 2027.01, Stage 2: 2027.04) 다시 체크 후 작업 시작


---


## 디렉토리 구조 (예정)


```
Studies/Hardware-Arm/
  README.md # 본 파일
  BOM.md # 부품 구매 리스트 (Stage 1 + Stage 2)
  stage1/
    README.md # Stage 1 목표 + 학습 순서
    URDF_guide.md # 2~3DOF URDF 작성 가이드
    ros2_driver_setup.md # dynamixel_hardware 패키지 셋업
    isaac_sim_import.md # URDF → USD 임포트 첫 사이클
    urdf/ # URDF 파일 + 3D 프린트 STL
    ros2_pkg/ # ROS2 패키지 (드라이버 + 데모)
  stage2/
    README.md # Stage 2 목표 + 학습 순서
    stage2_6dof_extension.md # 6DOF 확장 BOM + URDF 갱신
    teleop_setup.md # leader-follower 또는 PS4 패드
    safety_interlock.md # 위치/속도/토크 한계 + e-stop
    sim_real_param_match.md # Sim 물리 파라미터 매칭
    urdf/ # 6DOF URDF + 3D 프린트 STL
    ros2_pkg/ # 안전 인터록 + teleop 패키지
    teleop_data/ # 수집한 teleop 데이터 (HDF5 또는 rosbag)
```


## 형식 차이 (Phase X week 형식과 구별)


자작 팔은 *학습 콘텐츠* 가 아니라 *조립 + 통합 매뉴얼* 성격. 따라서:
- **week 단위 X** → **Stage 단위 + 단계 문서**
- `URDF_guide.md`, `BOM.md`, `safety_interlock.md` 등 *주제별 문서*


## 진행 순서


| 시점 | 활동 |
|---|---|
| 2026.10 | **스파이크 (2-3주)** — 2-DOF 파이프라인 리스크 검증 (산출물 아님) |
| 2026.11 | 6개월 분기 재평가 #1 (스파이크 결과 + 일정·BOM 재산정) |
| 2026.12 말 | Stage 1 가이드 재확인 (`stage1/` 안의 단계 문서들) |
| 2027.01-02 | **Stage 1 본 빌드** — v2 선행 하드웨어 (자작 팔 첫 사이클 영상) |
| 2027.04-06 | **Stage 2 진행** — Phase 6 와 병행, 실지원 병행 |
| 2027.08~ | Phase 7 의 산출물 v3 강화 카드 — Stage 2 산출물이 하드웨어 기반 |
