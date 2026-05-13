# SLAM-legacy Archive


> **아카이브 디렉토리** — 과거 SLAM 트랙(Phase 2/3/4) 학습 내용을 보존하는 공간
> **아카이브 시점**: 2026-04-10
> **상위 plan**: [`../../plan.md`](../../plan.md)


---


## 1. 아카이브 사유


이 디렉토리는 **이직 타겟이 Perception Engineer 로 확정되면서** SLAM 트랙(VO/BA/VIO)을 메인 로드맵에서 제거하고 보존 이동한 결과물이다.


원래 로드맵은 **SLAM 기초 + AI Perception** 하이브리드였으나, 실제 Perception Engineer 직무는 SLAM-specific 지식(VO/BA/VIO)을 일상 업무에서 쓰지 않는다. 따라서 다음 결정을 내렸다:


- **Phase 3 (VO & BA, 4주)** + **Phase 4 (VIO, 3주)** = 약 7주를 절약해 **Phase 3–4 (Detection + Depth + 3D Perception)** 와 포트폴리오 Sprint 에 집중
- **Phase 2 (8주)** 도 SLAM 프레이밍("VINS-Fusion 의 feature_tracker 이해")이 강해 Perception 중심 4주 구조로 재작성. 기존 8주는 통째로 이 디렉토리에 보존
- **차별화 포인트**를 "SLAM 기초 이해" → **"AMR 실무 경험 + 로봇에 Perception 배포 경험"** 으로 이동


---


## 2. 원본 위치 매핑


| 기존 경로 | 현재 경로 |
|-----------|-----------|
| `Studies/Phase 2/` | [`Studies/Phase 2/`](Studies/Phase%202/) |
| `Studies/Phase 3/` | [`Studies/Phase 3/`](Studies/Phase%203/) |
| `Studies/Phase 4/` | [`Studies/Phase 4/`](Studies/Phase%204/) |
| `Roadmap/Phase 2.md` | [`Roadmap/Phase 2.md`](Roadmap/Phase%202.md) |
| `Roadmap/Phase 3.md` | [`Roadmap/Phase 3.md`](Roadmap/Phase%203.md) |
| `Roadmap/Phase 4.md` | [`Roadmap/Phase 4.md`](Roadmap/Phase%204.md) |


> 모든 이동은 `git mv` 로 수행되어 git 히스토리(blame, log)가 보존된다.


---


## 3. 보존 내용 요약


### Studies/Phase 2 (8주, C++)
1. week1 — 카메라 모델 (핀홀, K, 내부/외부 파라미터)
2. week2 — 렌즈 왜곡 + 캘리브레이션
3. week3 — 특징점 디텍터 (Harris, FAST, ORB) deep dive
4. week4 — 특징 매칭 + 디스크립터
5. week5 — 에피폴라 기하 + Essential/Fundamental Matrix
6. week6 — Stereo Rectification + 디스패리티
7. week7 — 삼각측량 + PnP
8. week8 — Optical Flow / KLT (SLAM 트래킹용)
9. project/ — Phase 2 통합 미니 프로젝트


### Studies/Phase 3 (4주, C++) — VO & BA
1. week1 — Visual Odometry 개념 + Direct/Feature 방식
2. week2 — Bundle Adjustment 이론 (Ceres/g2o)
3. week3 — Loop Closure + Pose Graph
4. week4 — VO 시스템 통합 + 코드 분석 (ORB-SLAM 등)


### Studies/Phase 4 (3주, C++) — VIO
1. week1 — IMU 모델 + Pre-integration
2. week2 — Loose / Tight Coupling
3. week3 — VIO 시스템 분석 (VINS-Fusion 코드)


### Roadmap (3개 문서)
- 각 Phase 의 학습 가이드, 주차별 목표, 참고 자료 링크


---


## 4. 언제 다시 봐야 하나


이 디렉토리의 내용은 **참고 자료** 로 보존된다. 다음 상황에서 재방문할 수 있다:


- **SLAM 면접 질문 대비**: Perception 면접에서도 SLAM 관련 기본 질문이 나올 수 있음 (VO 원리, BA, IMU 통합 등)
- **Multi-view 기하학 복습**: Essential/Fundamental Matrix, 삼각측량 등은 새 Phase 2 와 일부 겹치지만 더 깊은 설명 필요 시
- **과거 학습 회고**: "내가 무엇을 공부했는지" 기록 (블로그, 이력서 작성 시)
- **실전 SLAM 시스템 코드 읽기**: ORB-SLAM, VINS-Fusion 코드 분석 시 컨셉 복습용


---


## 5. 주의


- 이 디렉토리의 **코드/문서를 새 `Studies/Phase 2/` 에서 직접 import 하거나 인용하지 말 것** — SLAM 맥락 잔재가 새 Perception 트랙으로 누수되는 것을 막기 위함
- 새 Phase 2 의 week3, week4 는 일부 주제가 겹치지만(에피폴라, 삼각측량, PnP) **새로 작성된 Perception 맥락 코드** 를 사용한다
- 이 디렉토리의 README/이론 설명은 SLAM 프레이밍("VINS feature_tracker", "VO 프론트엔드" 등)을 그대로 가지고 있으므로, Perception 학습에는 새 Phase 2 를 우선 참고할 것
