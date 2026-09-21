# Studies/Hardware-Arm — 자작 팔 트랙 (SO-101)

> **기간**: 2026.09-2027.06 (스파이크: 2026.09 → Stage 1: 2026.10-11 → Stage 2: 2027.04-06)
> **현재 위치**: 구매·조립·스파이크 완료 (판정 2026-09-20, must 4개 통과 — [`spike/week2/RESULT.md`](spike/week2/RESULT.md)). 다음은 **Stage 1 W1** (2026.10 첫 주, ROS2 드라이버 검증 — [`stage1/ros2_driver_setup.md`](stage1/ros2_driver_setup.md) §0-§1)
> **하드웨어**: SO-101 (SO-ARM101) 리더-팔로워, Feetech STS3215 (2026-07 확정 — Koch·커스텀 비채택 기록은 마스터 가이드)
> **마스터 가이드**: [`Roadmap/Hardware-Arm.md`](../../Roadmap/Hardware-Arm.md) / 절차·판정 원본: [실기 전환 plan](../../docs/superpowers/plans/2026-08-30-realworld-transition-execution.md) §5-§7 / 주 단위 체크: [master roadmap](../../docs/superpowers/plans/2026-08-31-master-roadmap.md) §3
> **Stage 1 · v2.5 · Stage 2 가이드는 미리 작성됨** — 진입 시 (Stage 1: 2026.10, v2.5: 2026.11, Stage 2: 2027.04) 다시 체크 후 작업 시작. `spike/` 는 실행을 마친 절차·증거 기록이고, Stage 1 · v2.5 가 하드웨어 사실 (포트·모터 ID·캘리브·record 규약) 의 원본으로 참조한다

---

## 스택 역할 분리 (전 단계 공통)

> **데이터·학습 = LeRobot / 배포·통합 = ROS 2.** 스파이크는 LeRobot 네이티브만으로 실기 경로를 검증하고, ROS 2 층은 Stage 1 에서 얹는다. 두 스택이 같은 시리얼 포트를 쓰므로 동시에 한 스택만 버스에 붙인다.

---

## 디렉토리 구조

```
Studies/Hardware-Arm/
  README.md                    # 본 파일
  BOM.md                       # SO-101 구매 구성 + 키트 사양 (구매·조립 완료)
  spike/
    week1/
      so-arm101-assembly-guide.md   # Week 1 (D1-D7) 실행 절차: 검수 · 모터 ID · 조립 · 캘리브 · teleop
      images/so-arm101/        # 조립 가이드 그림
    week2/
      week2_guide.md           # Week 2 (D8-D14) 실행 절차: 카메라 · 녹화 · zero-shot · latency · 판정
      RESULT.md                # 스파이크 결과 기록 (must 4 증거 + 판정 2026-09-20)
      scripts/
        measure_latency_smolvla.py   # D11 latency 측정 (must 4 — OpenVLA 300ms 와 병기)
        live_view.py                 # 카메라 실시간 화면 (헤드리스 컨테이너용 MJPEG 스트림 — 구도 확인)
        analyze_teleop_tracking.py   # teleop 녹화에서 추종 지연 · 오차 계산 (must 1 의 수치 증거)
      outputs/                 # 실행 로그 · latency 원본 · evidence/ (증거 데이터셋 사본) — gitignore, 로컬에만 보존
  stage1/
    README.md                  # Stage 1 목표 + 진행 순서 (본 빌드: 완성도 + ROS2 층)
    URDF_guide.md              # SO-101 공개 URDF 재사용 + 검증
    ros2_driver_setup.md       # feetech_ros2_driver + ros2_control 셋업
    isaac_sim_import.md        # URDF → USD 임포트 (nice — Phase 6 이월 허용)
    scripts/
      print_joint_command.py   # 현재 관절값을 읽어 붙여 넣을 위치 명령을 출력 (읽기 전용 — ros2_driver_setup.md §4.2)
    ros2_pkg/                  # 직접 만드는 ROS2 패키지의 원본. /workspace/so101_ws/src/ 에 심링크를 걸어 빌드한다
      so101_description/       #   URDF (공개 URDF 재사용, urdf/ 한 곳에만) + meshes + controller config + launch 2개. 캘리브 오프셋 반영은 W4
  v25/
    README.md                  # v2.5 학습 가이드 (N 역산·실기 측정 설계·하네스 검증)
    PRACTICE.md                # 수집·eval·파인튜닝 명령 골격
  stage2/
    README.md                  # Stage 2 목표 + 진행 순서 (확장 수단·teleop 확장 가이드는 재평가 결정 후 작성)
    safety_interlock.md        # 위치/속도/토크 한계 + e-stop (C++)
    sim_real_param_match.md    # Sim 물리 파라미터 매칭
    urdf/                      # (예정 — Stage 2 에서 생성) 갱신 URDF
    ros2_pkg/                  # (예정 — Stage 2 에서 생성) 안전 인터록 + teleop 패키지
    teleop_data/               # (예정 — Stage 2 에서 생성) 수집 teleop 데이터 (LeRobot 포맷, 원본은 HF Hub)
```

## 형식 차이 (Phase X week 형식과 구별)

자작 팔은 *학습 콘텐츠* 가 아니라 *조립 + 통합 매뉴얼* 성격. 따라서:
- **week 단위 X** → **Stage 단위 + 단계 문서** (스파이크만 2주 타임박스라 `week1/` · `week2/` 로 나눈다)
- `URDF_guide.md`, `BOM.md`, `safety_interlock.md` 등 *주제별 문서*

## 진행 순서

| 시점 | 활동 |
|---|---|
| 2026.09 초 | **SO-101 키트 + Wrist 카메라 옵션 구매** — 완료 (발주 2026-09-01, 60만원, 리드타임 3일 — [`BOM.md`](BOM.md)) |
| 2026.09 | **스파이크 (타임박스)** — 완료. 조립 (2026-09-12) + LeRobot 네이티브 검증. must 4: teleop / 10 에피소드 녹화+Hub / SmolVLA zero-shot / latency. **판정 2026-09-20: must 4개 통과** → `spike/week2/RESULT.md` |
| 2026.10-11 | **Stage 1 본 빌드** (다음 단계) — 조립 완성 + 안전 기초 (소프트 리밋·토크 상한·소프트웨어 정지) + ROS2 래핑 + URDF 오프셋 + 이중 latency + 1분 영상 (v2 선행 하드웨어). W1 은 ROS2 드라이버 검증. must 최종 마감 2027.02 말 |
| 2026.11-12 | **v2.5 병행** — teleop 데이터셋 (LeRobot 포맷, HF Hub) + SmolVLA 실기 before/after + vla-lab 공개 문서. 착수 전 선결 2개: 정규화 통계 · 단위 규약 맞추기, `max_relative_target` 하한 실측 (`spike/week2/RESULT.md` §4 #8 · #11) |
| 2026.11 | 6개월 분기 재평가 #1 (스파이크 판정 + v2.5 진행률 + **Stage 2 확장 수단 결정**) |
| 2027.04-06 | **Stage 2 진행** — Phase 6 와 병행, 실지원 병행 |
| 2027.05 | 6개월 분기 재평가 #2 (Stage 2 완성도 확인) |
| 2027.08~ | Phase 7 의 산출물 v3 강화 카드 — Stage 2 산출물이 하드웨어 기반 |

> 스파이크가 통과해 롤백 조건 (teleop 불가 또는 2주 초과 → 원안 일정 복귀, 실기 전환 plan §5.4) 은 발동하지 않았다.
