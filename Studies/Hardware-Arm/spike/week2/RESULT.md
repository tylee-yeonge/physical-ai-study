# 스파이크 결과 기록 — SO-101 LeRobot 네이티브 실기 검증

> 기간: 2026.09 첫 2주 (타임박스) / 판정: 2026-09-21 1회
> 절차·통과 기준의 원본: [실기 전환 plan](../../../../docs/superpowers/plans/2026-08-30-realworld-transition-execution.md) §5
> 실행 절차: Week 1 = [조립 가이드](../week1/2026-09-13-so-arm101-assembly-guide.md) / Week 2 (D8-D14) = [week2_guide.md](week2_guide.md)
> 이 문서는 증거·소요·판단의 보존 기록이다 — `Measurements/` 원칙 (정답은 지우고 증거는 남긴다) 을 따르며 원복하지 않는다.

---

## 1. must 4 증거

| # | 기준 | 증거 (링크/ID/수치) | 확보일 |
|---|---|---|---|
| 1 | 리더-팔로워 teleop (6축 추종) | 로컬 데이터셋 `~/.cache/huggingface/lerobot/tylee-yeonge/so101-spike-teleop_20260919_233008` (보존용 사본: `outputs/evidence/` 아래 같은 이름) — teleop 을 `lerobot-record` 로 30초 1 에피소드 녹화 (898 프레임, 29.91 Hz, 예산 초과 틱 0/897). 영상: `videos/observation.images.front/chunk-000/file-000.mp4` (ELP — 리더를 조작하는 손과 따라 움직이는 팔로워가 한 화면), `videos/observation.images.wrist/...` (손목). 수치 (`action` = 리더 목표 vs `observation.state` = 팔로워 실제 위치): 6관절 모두 추종, 지연 100-170 ms (측정 해상도 33 ms), 지연 보정 후 RMSE 0.5-1.7도 (그리퍼 2.2 / 100), 편향 최대 +1.27도 (`elbow_flex` — P 제어의 중력 부하 오차), 1틱 최대 변화 8.7도 (값 튐 없음). 2026-09-19 22:56 재캘리브 기준 (호밍 자세를 가운데로 바로잡은 캘리브 — 조립 가이드 §6.1). 미확인: 최대 신장 자세 — `shoulder_lift` +39도 / `elbow_flex` -57도까지만 움직였다 (한계 +105 / -103도) | 2026-09-19 |
| 2 | `lerobot-record` 단일 task 10 에피소드 + HF Hub (private) | (데이터셋 repo id) | — |
| 3 | `lerobot/smolvla_base` zero-shot 1회 실행 (반응 확인 — 성공 여부 무관) | (30초 영상 + 로그 경로) | — |
| 4 | 추론 루프 latency (4070, n=100) | SmolVLA base (4070, torch.bfloat16): chunk mean 106.3 / p95 108.9 ms (n=100, 50 actions/chunk, action 당 2.13 ms, 전처리 밖) vs OpenVLA int4 300.3 / 304.8 ms (action 1개). 두 수치는 뜻이 다르다 — chunk 값은 모델이 한 번 판단하는 데 걸리는 시간, action 당 값은 그것을 50 으로 나눈 평균이다. 조건 차이: 정밀도 bfloat16 (양자화 없음) vs int4, 이미지 입력 3장 (config 의 camera1-3 모두 랜덤 텐서) vs 1장. 분포: median 106.7 / std 1.6 / min 103.9 / max 109.7 ms (max 는 본 측정 3번째 반복), p99 109.6 ms, 9.41 chunk/s. VRAM: 로드 직후 0.93 GB, peak 0.97 GB. 원본: `outputs/evidence/smolvla_latency_4070_20260919_2305.npy` · `outputs/evidence/smolvla_latency_4070_summary_20260919_2305.csv` (스크립트는 매번 `outputs/smolvla_latency_4070.*` 에 덮어쓰므로 시각을 붙여 고정한 사본). 재현성: 같은 날 앞서 돌린 실행의 mean 은 106.2 ms 로 0.02 ms 차이였다 (그 실행의 raw 파일은 덮어써져 남아 있지 않다) | 2026-09-19 |

nice: 부분 도달률 (reached / grasped) 기록 — (있으면 기입)

## 2. 진행 체크리스트 (plan §5.2-5.3 복사)

### Week 1 — 조립 · 캘리브레이션 · teleop
- [x] 팔로워 조립 (모터 ID 확인 → 링크 조립 → 배선)
- [x] 리더 조립 (기어 제거 여부는 키트 사양서 확인)
- [x] LeRobot 설치 + 포트 탐색 + 캘리브레이션
- [x] must 1 — teleop (6관절 + 그리퍼 추종 확인. 증거 영상 · 추종 수치는 §1 행 1 — 2026-09-19 재캘리브 후 녹화분)

### Week 2 — 데이터 녹화 · zero-shot · latency
- [x] 카메라 세팅 + 테스트 녹화 (week2_guide §1) — 2026-09-16. 로컬 데이터셋 `~/.cache/huggingface/lerobot/local/so101-spike-test_20260916_233310` (보존용 사본: `outputs/evidence/` 아래 같은 이름. 30 fps · 2 ep · 1198 frames · front 1280x480 + wrist 1280x720, 루프 29.92 Hz · 예산 초과 틱 0/1196). 로그 `outputs/d8_record.log`. 리더를 잡지 않은 정지 녹화라 관절값은 일정 — 파이프라인 검증만. 수령 확인 ③ 은 §4 #5 (거치 모듈 장착 불가, 임시 고정)
- [ ] must 2 — 단일 task 10 에피소드 + Hub 업로드 (week2_guide §2)
- [ ] must 3 — SmolVLA zero-shot 1회 실행 (week2_guide §3)
  - 사전 검증 (2026-09-19, 팔을 움직이지 않음 — 모터 버스 읽기 전용 연결, 쓰기 · 토크 변경 · `send_action` 차단): 가이드 §3.4 의 `lerobot-rollout` 명령 파싱, 캘리브 일치 (`is_calibrated=True`), 카메라 2대 동시 연결 (실측 30.4 / 60.4 fps), 카메라 이름 검사, 실제 관측으로 정책 출력까지 통과. chunk 생성 mean 90.9 / p95 91.6 ms (n=20, 카메라 2대). 미검증: `send_action`, 토크가 켜지는 순간의 거동, 종료 시 시작 자세 복귀, 30 Hz 루프 유지
- [x] must 4 — latency 측정 (n=100) (week2_guide §4 + `scripts/measure_latency_smolvla.py`) — 2026-09-19. chunk mean 106.3 / p95 108.9 ms (수치 · 조건은 §1 행 4). 팔 · 카메라 없이 GPU 만 쓰는 측정이라 must 2 · 3 보다 먼저 수행 (week2_guide §0.3). 막힌 지점은 §4 #6

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
| 6 | `measure_latency_smolvla.py` 가 `SmolVLAPolicy.from_pretrained` 에서 `ImportError: 'transformers' is required but not installed` 로 중단 | venv 가 Week 1 의 extras `core_scripts,feetech` 로만 설치됨. SmolVLA 의 시각-언어 모델을 읽는 `transformers` 는 `smolvla` extra 에 들어 있고, lerobot 은 이를 import 시점이 아니라 정책 객체 생성 시점에 검사 | `pip install "lerobot[smolvla]"` — transformers 5.5.4 · accelerate 1.15.0 · num2words 0.5.14 추가. lerobot 본체 (git 커밋 고정) 와 torch 2.11.0+cu130 은 그대로. 가이드 §0.2 | — |
| 7 | 가이드 §3.4 의 `lerobot-record --policy.path` 로는 정책 실행 불가 (D10 실행 전 소스 확인으로 발견) | lerobot 0.6.2 의 `lerobot-record` 는 녹화 전용 — `--teleop.*` 없이는 "use lerobot-rollout instead" 로 중단하고 `eval_` repo_id 도 거부. `lerobot.utils.control_utils` 도 없어 가이드 §4.3 의 grep 이 실패 | `lerobot-rollout --strategy.type=base --duration=30` (추론 `sync`). 가이드 §3 · §4.3 을 이 기준으로 작성 | — |
| 8 | zero-shot 의 관절 목표가 6개 모두 0 근처 (±2 이내) 로 과제와 무관 (사전 검증에서 발견 — 팔에 보내지는 않음) | `smolvla_base` 의 정규화 통계 키가 `so100.` · `so100-blue.` · `so100-red.` 접두어라 0.6.2 의 역정규화가 `action` 키를 못 찾고 값을 그대로 통과시킴. 입력 관절값도 정규화되지 않음. 0.6.2 팔로워의 단위는 각도 (`use_degrees=true`) 라 `max_relative_target` 은 틱당 도 | must 3 ("반응하는가") 판정에는 무관 — 예상 동작으로 가이드 §3.5 에 기록, 첫 실행은 `max_relative_target=3`. 통계 · 단위 규약 맞추기는 v2.5 첫 항목 | — |

## 5. 판정 (2026-09-21, 1회만 — plan §5.4 표 기준)

- **결과**: (must 4개 통과 / 1-3 통과·4 미완 / 1-2 통과·3 실패 / 롤백 — 해당 행 명시)
- **다음 행동**: (Stage 1 2026.10 개시 / latency 는 본 빌드 첫 주 / v2.5 첫 항목 디버깅 / 원안 일정 복귀)
- **분기 재평가 #1 입력 요약**: (2-3줄)
