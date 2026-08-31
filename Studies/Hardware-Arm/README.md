# Studies/Hardware-Arm — 자작 팔 트랙 (SO-101)

> **기간**: 2026.09-2027.06 (스파이크: 2026.09 첫 2주 → Stage 1: 2026.10-11 → Stage 2: 2027.04-06) — **2026-08-30 실기 전환으로 전진**
> **하드웨어**: SO-101 (SO-ARM101) 리더-팔로워, Feetech STS3215 (2026-07 확정 — Koch·커스텀 비채택 기록은 마스터 가이드)
> **마스터 가이드**: [`Roadmap/Hardware-Arm.md`](../../Roadmap/Hardware-Arm.md) / 절차·판정 원본: [실기 전환 plan](../../docs/superpowers/plans/2026-08-30-realworld-transition-execution.md) §5-§7
> **단계별 가이드 미리 작성됨** — 진입 시 (스파이크: 2026.09, Stage 1: 2026.10, Stage 2: 2027.04) 다시 체크 후 작업 시작

---

## 스택 역할 분리 (전 단계 공통)

> **데이터·학습 = LeRobot / 배포·통합 = ROS 2.** 스파이크는 LeRobot 네이티브만으로 실기 경로를 검증하고, ROS 2 층은 Stage 1 에서 얹는다. 두 스택이 같은 시리얼 포트를 쓰므로 동시에 한 스택만 버스에 붙인다.

---

## 디렉토리 구조

```
Studies/Hardware-Arm/
  README.md                    # 본 파일
  BOM.md                       # SO-101 키트 구매 리스트 + 확인 항목 (구매는 스파이크 선행)
  spike/
    RESULT.md                  # 스파이크 결과 기록 (must 4 증거 + 판정 2026-09-21)
  stage1/
    README.md                  # Stage 1 목표 + 진행 순서 (본 빌드: 완성도 + ROS2 층)
    URDF_guide.md              # SO-101 공개 URDF 재사용 + 검증
    ros2_driver_setup.md       # feetech_ros2_driver + ros2_control 셋업
    isaac_sim_import.md        # URDF → USD 임포트 (nice — Phase 6 이월 허용)
    urdf/                      # URDF + 캘리브레이션 오프셋 반영본
    ros2_pkg/                  # ROS2 패키지 (드라이버 래핑 + 데모)
  stage2/
    README.md                  # Stage 2 목표 + 진행 순서 (확장 수단·teleop 확장 가이드는 재평가 결정 후 작성)
    safety_interlock.md        # 위치/속도/토크 한계 + e-stop (C++)
    sim_real_param_match.md    # Sim 물리 파라미터 매칭
    urdf/                      # 갱신 URDF
    ros2_pkg/                  # 안전 인터록 + teleop 패키지
    teleop_data/               # 수집 teleop 데이터 (LeRobot 포맷, 원본은 HF Hub)
```

## 형식 차이 (Phase X week 형식과 구별)

자작 팔은 *학습 콘텐츠* 가 아니라 *조립 + 통합 매뉴얼* 성격. 따라서:
- **week 단위 X** → **Stage 단위 + 단계 문서**
- `URDF_guide.md`, `BOM.md`, `safety_interlock.md` 등 *주제별 문서*

## 진행 순서

| 시점 | 활동 |
|---|---|
| 2026.08 말-09 초 | **SO-101 키트 즉시 구매** (약 55만원 + 손목 카메라, 리드타임 3일 — `stage1/BOM.md`) |
| 2026.09 첫 2주 | **스파이크 (타임박스)** — 조립 + LeRobot 네이티브 검증. must 4: teleop / 10 에피소드 녹화+Hub / SmolVLA zero-shot / latency. **판정 2026-09-21 1회** → `spike/RESULT.md` |
| 2026.10-11 | **Stage 1 본 빌드** — 완성도 + 안전 기초 + ROS2 래핑 + 이중 latency (v2 선행 하드웨어) |
| 2026.11-12 | **v2.5 병행** — teleop 데이터셋 (LeRobot 포맷, HF Hub) + SmolVLA 실기 before/after + vla-lab 공개 문서 |
| 2026.11 | 6개월 분기 재평가 #1 (스파이크 판정 + v2.5 진행률 + **Stage 2 확장 수단 결정**) |
| 2027.04-06 | **Stage 2 진행** — Phase 6 와 병행, 실지원 병행 |
| 2027.08~ | Phase 7 의 산출물 v3 강화 카드 — Stage 2 산출물이 하드웨어 기반 |

> 롤백 조건 (스파이크 teleop 불가 또는 2주 초과 → 원안 일정 복귀) 은 실기 전환 plan §5.4.
