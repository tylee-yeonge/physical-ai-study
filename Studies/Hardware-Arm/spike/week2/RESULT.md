# 스파이크 결과 기록 — SO-101 LeRobot 네이티브 실기 검증

> 기간: 2026.09 첫 2주 (타임박스) / 판정: 2026-09-21 1회
> 절차·통과 기준의 원본: [실기 전환 plan](../../../../docs/superpowers/plans/2026-08-30-realworld-transition-execution.md) §5
> 실행 절차: Week 1 = [조립 가이드](../week1/2026-09-13-so-arm101-assembly-guide.md) / Week 2 (D8-D14) = [week2_guide.md](week2_guide.md)
> 이 문서는 증거·소요·판단의 보존 기록이다 — `Measurements/` 원칙 (정답은 지우고 증거는 남긴다) 을 따르며 원복하지 않는다.

---

## 1. must 4 증거

| # | 기준 | 증거 (링크/ID/수치) | 확보일 |
|---|---|---|---|
| 1 | 리더-팔로워 teleop (6축 추종) | (30초 영상 링크) | — |
| 2 | `lerobot-record` 단일 task 10 에피소드 + HF Hub (private) | (데이터셋 repo id) | — |
| 3 | `lerobot/smolvla_base` zero-shot 1회 실행 (반응 확인 — 성공 여부 무관) | (30초 영상 + 로그 경로) | — |
| 4 | 추론 루프 latency (4070, n=100) | (mean/p95 수치 — OpenVLA 300.3ms/304.8ms 와 병기) | — |

nice: 부분 도달률 (reached / grasped) 기록 — (있으면 기입)

## 2. 진행 체크리스트 (plan §5.2-5.3 복사)

### Week 1 — 조립 · 캘리브레이션 · teleop
- [x] 팔로워 조립 (모터 ID 확인 → 링크 조립 → 배선)
- [x] 리더 조립 (기어 제거 여부는 키트 사양서 확인)
- [x] LeRobot 설치 + 포트 탐색 + 캘리브레이션
- [x] must 1 — teleop (6관절 + 그리퍼 추종 확인. §1 증거 영상 링크는 미기입)

### Week 2 — 데이터 녹화 · zero-shot · latency
- [x] 카메라 세팅 + 테스트 녹화 (week2_guide §1) — 2026-09-16. 로컬 데이터셋 `~/.cache/huggingface/lerobot/local/so101-spike-test_20260916_233310` (30 fps · 2 ep · 1198 frames · front 1280x480 + wrist 1280x720, 루프 29.92 Hz · 예산 초과 틱 0/1196). 로그 `outputs/d8_record.log`. 리더를 잡지 않은 정지 녹화라 관절값은 일정 — 파이프라인 검증만. 수령 확인 ③ 은 §4 #5 (거치 모듈 장착 불가, 임시 고정)
- [ ] must 2 — 단일 task 10 에피소드 + Hub 업로드 (week2_guide §2)
- [ ] must 3 — SmolVLA zero-shot 1회 실행 (week2_guide §3)
- [ ] must 4 — latency 측정 (n=100) (week2_guide §4 + `scripts/measure_latency_smolvla.py`)

## 3. 소요 시간 (계획 대비)

| 구간 | 계획 | 실제 | 비고 |
|---|---|---|---|
| 조립 (Week 1 Day 1-3) | 3일 | — | |
| 셋업·캘리브레이션·teleop (Day 4-5) | 2일 | — | |
| 녹화·zero-shot·latency (Week 2) | 4일 | — | |

## 4. 막힌 지점과 해결

| # | 증상 | 원인 | 해결 / 우회 | 소요 |
|---|---|---|---|---|
| 1 | `lerobot-record` 가 카메라 연결에서 `failed to set fps=30 (actual_fps=25.0)` 으로 중단 | ELP 는 1280x480 MJPG 에서 25 / 60 fps 만 지원 (USB 디스크립터 확인). 가이드 예시의 30 이 목록에 없음 | `fps: 60` 으로 지정. 데이터셋 fps 30 과는 별개 (record 루프는 매 틱 최신 프레임만 가져감) | — |
| 2 | 컨테이너에 `/dev/v4l/by-id/` 가 없어 가이드 §1.3 의 고정 경로를 못 씀 | 컨테이너 /dev 는 tmpfs 라 호스트 udev 심링크가 없음 | `so101-attach` 가 USB 시리얼로 `/dev/so101_cam_overview`(ELP) · `/dev/so101_cam_wrist`(손목) 를 생성. 설정의 `index_or_path` 에 이 경로 | — |
| 3 | 데이터셋이 가이드 §1.5 의 경로 (`.../so101-spike-test`) 에 없음 | lerobot 0.6.2 는 기본으로 repo_id 에 `_YYYYMMDD_HHMMSS` 를 붙임 (로컬 폴더 · Hub 이름 모두) | D9 부터 `--dataset.no_stamp=true` 로 끔. 테스트 데이터셋은 `ls ~/.cache/huggingface/lerobot/local/` 로 실제 폴더 확인 | — |
| 4 | 녹화 중 키 조작 불가 (`Keyboard controls unavailable: no usable display ... stdin is not an interactive terminal`) | 헤드리스 컨테이너 + 비대화형 stdin | 타이머만으로 진행돼 테스트 녹화에는 무관. D9 는 TTY 터미널에서 실행해야 화살표 · `r` · `q` 가 듣는다 | — |
| 5 | 수령 확인 ③ — ELP 가 키트 기본 정면 카메라 거치 모듈에 안 붙음 | 거치 모듈의 1/4 나사 체결부와 ELP 가 맞지 않음 | 임시 고정으로 우회 (스파이크는 "안 예뻐도 된다" — plan §5.1). Week 2 내내 카메라를 움직이지 않는다. 고정 방식 확정 + 위치 마킹은 Stage 1 W3 에서 | — |

## 5. 판정 (2026-09-21, 1회만 — plan §5.4 표 기준)

- **결과**: (must 4개 통과 / 1-3 통과·4 미완 / 1-2 통과·3 실패 / 롤백 — 해당 행 명시)
- **다음 행동**: (Stage 1 2026.10 개시 / latency 는 본 빌드 첫 주 / v2.5 첫 항목 디버깅 / 원안 일정 복귀)
- **분기 재평가 #1 입력 요약**: (2-3줄)
