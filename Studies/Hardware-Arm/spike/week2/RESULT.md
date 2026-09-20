# 스파이크 결과 기록 — SO-101 LeRobot 네이티브 실기 검증

> 기간: 2026.09 첫 2주 (타임박스) / 판정: 2026-09-20 1회 (예정일 09-21 의 전날, must 4개의 증거가 모두 확보된 시점)
> 절차·통과 기준의 원본: [실기 전환 plan](../../../../docs/superpowers/plans/2026-08-30-realworld-transition-execution.md) §5
> 실행 절차: Week 1 = [조립 가이드](../week1/so-arm101-assembly-guide.md) / Week 2 (D8-D14) = [week2_guide.md](week2_guide.md)
> 이 문서는 증거·소요·판단의 보존 기록이다 — `Measurements/` 원칙 (정답은 지우고 증거는 남긴다) 을 따르며 원복하지 않는다.

---

## 1. must 4 증거

| # | 기준 | 증거 (링크/ID/수치) | 확보일 |
|---|---|---|---|
| 1 | 리더-팔로워 teleop (6축 추종) | 로컬 데이터셋 `~/.cache/huggingface/lerobot/tylee-yeonge/so101-spike-teleop_20260919_233008` (보존용 사본: `outputs/evidence/` 아래 같은 이름) — teleop 을 `lerobot-record` 로 30초 1 에피소드 녹화 (898 프레임, 29.91 Hz, 예산 초과 틱 0/897). 영상: `videos/observation.images.front/chunk-000/file-000.mp4` (ELP — 리더를 조작하는 손과 따라 움직이는 팔로워가 한 화면), `videos/observation.images.wrist/...` (손목). 수치 (`action` = 리더 목표 vs `observation.state` = 팔로워 실제 위치): 6관절 모두 추종, 지연 100-170 ms (측정 해상도 33 ms), 지연 보정 후 RMSE 0.5-1.7도 (그리퍼 2.2 / 100), 편향 최대 +1.27도 (`elbow_flex` — P 제어의 중력 부하 오차), 1틱 최대 변화 8.7도 (값 튐 없음). 2026-09-19 22:56 재캘리브 기준 (호밍 자세를 가운데로 바로잡은 캘리브 — 조립 가이드 §6.1). 미확인: 최대 신장 자세 — `shoulder_lift` +39도 / `elbow_flex` -57도까지만 움직였다 (한계 +105 / -103도) | 2026-09-19 |
| 2 | `lerobot-record` 단일 task 10 에피소드 + HF Hub (private) | Hub 데이터셋 `tylee-yeonge/so101-spike-pick-cube` (private = True 를 `HfApi().dataset_info` 로 확인, 파일 9개 — 영상 front 153.8 MB · wrist 109.9 MB 가 로컬과 같은 크기). 10 에피소드 · 8980 프레임 · 299.3 s · 30 fps, 지시문 `Pick up the red cube and place it on the tray.`, 루프 29.91-29.92 Hz · 예산 초과 틱 0/897 (10개 모두). 내용 검증: 10개 모두 리더 조작과 파지 구간이 있다 (팔로워 그리퍼가 큐브에 막혀 덜 닫힌 구간 3.8-15.9 s), 1틱 최대 변화 9.4도 (값 튐 없음), 첫 장면은 10개 모두 휴식 자세 + 시작 표시 위의 큐브. 끝 장면은 9개가 큐브를 목표 표시 위에 놓고 휴식 자세로 복귀, **에피소드 5 는 30초 상한에 걸려 큐브를 목표 위에서 쥔 채로 끝났다** (놓기 · 복귀 없음). 로그 `outputs/d9_record.log`. 이 데이터셋의 알려진 결함 — v2.5 규약의 기준으로 쓰지 않는다: ① 에피소드 5 미완 ② 10개 모두 `→` 없이 30초를 채워 시범 뒤 5-11초의 정지 구간이 들어 있다 ③ 큐브는 분홍인데 지시문은 `red` ④ 목표가 트레이가 아니라 큐브 크기의 테이프 표시 (성공 규칙: 큐브가 표시를 일부라도 덮으면 성공) ⑤ 손목 카메라 케이블이 책상 위에 늘어진 채 화면에 찍힘 | 2026-09-20 |
| 3 | `lerobot/smolvla_base` zero-shot 1회 실행 (반응 확인 — 성공 여부 무관) | 리더 없이 `lerobot-rollout` 을 두 번 실행, 둘 다 `Rollout finished` 까지 Traceback 없이 완료. 스마트폰 영상은 찍지 않았다 — 30초 영상 증거는 2차 실행이 남긴 ELP 영상이고, 1차 실행은 로그만 남아 있다. 1차 (`--strategy.type=base`, 30초): 로그 `outputs/d10_zeroshot.log`. 2차 (`--strategy.type=episodic`, 30초 1 에피소드 — 영상 · 관절값을 남기는 실행): 로컬 데이터셋 `~/.cache/huggingface/lerobot/tylee-yeonge/rollout_so101-spike-zeroshot_20260920_231730` (보존용 사본: `outputs/evidence/` 아래 같은 이름) + 로그 `outputs/d10_zeroshot_episodic.log`. 영상: `videos/observation.images.camera2/chunk-000/file-000.mp4` (ELP — 팔이 스스로 일어나는 30초), `videos/observation.images.camera1/...` (손목). 855 프레임. 관찰한 동작: week2_guide §3.5 판정 표의 "예상되는 동작" 행 — 휴식 자세에서 스스로 일어나 약 3초 만에 멈춘 뒤 그 자세를 유지. 수치 (`observation.state` 첫 값 → 마지막 값): `shoulder_lift` -70.4 → +1.9도 (72.3도 이동), `elbow_flex` +99.7 → +42.3도 (57.4도), `wrist_flex` +34.7 → +0.3도 (35.1도), `shoulder_pan` -8.0 → -0.6도, 1틱 최대 변화 1.67도. 정책 출력 (`action`) 은 6관절 모두 -2.3 .. +4.9 범위 — §4 #8 의 예상 (정규화 통계 미적용, 목표가 0 근처) 을 실측으로 확인. `elbow_flex` 만 목표 (약 +2도) 에 못 가고 +42.3도에서 정지 (§4 #11). 큐브 쪽으로 가지 않음. 제어 루프: 1차 28.55 Hz · 예산 초과 17/856 틱 (2.0 %), 2차 28.48 Hz · 17/854 틱 (2.0 %). 추론이 도는 틱은 worst 102-103 ms, 첫 추론은 332-344 ms | 2026-09-20 |
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
- [x] must 2 — 단일 task 10 에피소드 + Hub 업로드 (week2_guide §2) — 2026-09-20. repo id · 내용 검증 · 알려진 결함은 §1 행 2, 부수 실측은 §3, 막힌 지점은 §4 #9 · #10
- [x] must 3 — SmolVLA zero-shot 1회 실행 (week2_guide §3) — 2026-09-20. `base` 1회 + `episodic` 1회, 팔이 리더 없이 스스로 일어남. 증거 · 수치는 §1 행 3, 관찰된 한계는 §4 #11
  - 사전 검증 (2026-09-19, 팔을 움직이지 않음 — 모터 버스 읽기 전용 연결, 쓰기 · 토크 변경 · `send_action` 차단): 가이드 §3.4 의 `lerobot-rollout` 명령 파싱, 캘리브 일치 (`is_calibrated=True`), 카메라 2대 동시 연결 (실측 30.4 / 60.4 fps), 카메라 이름 검사, 실제 관측으로 정책 출력까지 통과. chunk 생성 mean 90.9 / p95 91.6 ms (n=20, 카메라 2대)
  - 실기 실행 (2026-09-20) 으로 확인된 것: `send_action` 경로, 토크가 켜지는 순간 자세 유지, 종료 시 시작 자세 복귀 (약 3초) 후 토크 해제, 제어 루프 28.5 Hz (추론 틱이 33 ms 예산을 넘겨 30 Hz 보다 낮다 — 예산 초과 2.0 %)
- [x] must 4 — latency 측정 (n=100) (week2_guide §4 + `scripts/measure_latency_smolvla.py`) — 2026-09-19. chunk mean 106.3 / p95 108.9 ms (수치 · 조건은 §1 행 4). 팔 · 카메라 없이 GPU 만 쓰는 측정이라 must 2 · 3 보다 먼저 수행 (week2_guide §0.3). 막힌 지점은 §4 #6

## 3. 소요 시간 (계획 대비)

| 구간 | 계획 | 실제 | 비고 |
|---|---|---|---|
| 조립 (Week 1 Day 1-3) | 3일 | 1일로 추정 (09-12 토요일) — 확정 아님 | 본인 기억은 "조립 가이드를 만든 날 또는 그 다음날". 기록: 첫 캘리브 파일 09-13 00:22 (팔로워) · 00:35 (리더) → 조립 가이드를 쓴 세션 09-13 01:27 시작 → 커밋 01:42. 캘리브는 조립이 끝나야 할 수 있으므로 조립은 09-13 00:22 이전에 끝났고, "다음날" 은 배제된다. 가이드의 소요 참고치 (팔로워 약 2시간 + 리더 약 1.5시간) 와도 하루에 들어맞는다. 다만 08-31 16:27 부터 09-13 01:42 까지 커밋이 없고 사진에 촬영 시각도 없어, 09-12 이전 며칠에 나눠 조립했을 가능성은 기록으로 배제되지 않는다. 수령일도 기록에 없다 (발주 09-01) |
| 셋업·캘리브레이션·teleop (Day 4-5) | 2일 | 조립과 같은 날 밤 (09-12 밤 - 09-13 새벽, 약 1.5시간) + 재작업 1일 (09-19 밤) | 캘리브 폴더 생성 09-13 00:22 (팔로워) · 00:35 (리더), Week 1 완료 커밋 01:42. 09-19 22:56 호밍 자세를 바로잡아 재캘리브 → 23:30 must 1 증거 녹화 |
| 녹화·zero-shot·latency (Week 2) | 4일 | 실작업 3일 (09-16 · 09-19 · 09-20) + 준비 2일 (09-13 가이드 · 스크립트 작성, 09-15 23:06 컨테이너 venv 설치) | 09-16 D8 (23:33 테스트 녹화). 09-19 D11 (18:09 · 23:05) + D10 사전 검증. 09-20 D9 (22:40 첫 시도 중단 → 22:53-23:04 재녹화 · 업로드) + D10 (1차 23:11 · 2차 23:17). D9 와 D10 은 판정 예정일 전날 밤 약 1시간에 몰렸다. 팔을 쓰는 작업은 전부 22시 이후 1-2시간에 이뤄졌다 |

D9 부수 실측 (v2.5 N 역산의 입력 — `../../v25/README.md` §1. 출처: `outputs/d9_record.log` 의 시각 + 데이터셋의 `action`):

| 항목 | 값 | 어떻게 쟀나 |
|---|---|---|
| 에피소드당 소요 | 61.4 s | `Recording episode 0` (22:53:26) - `Stop recording` (23:03:40) = 614 s 를 10 으로 나눔. 리셋 · 저장 포함, 업로드 제외 |
| 에피소드당 순수 녹화 | 29.9 s | 8980 프레임 / 30 fps / 10. 10개 모두 30초 상한을 채웠다 |
| 오버헤드 | 31.5 s | 위 둘의 차이 — 리셋 15 s + 저장 (인코딩) 약 17 s |
| 실제 시범 길이 | 평균 20.3 s (17.3-24.1 s) | 리더가 처음 움직인 프레임부터 마지막으로 움직인 프레임까지. 집기 · 놓기 · 휴식 자세 복귀 포함, 완결된 9개 기준. 30초 상한은 이 조작자에게 여유가 6-13초뿐이고 1개가 초과했다 |
| 업로드 | 64 s | `Stop recording` - `Exiting`, 253 MB |

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
| 9 | D9 첫 실행에서 시범을 에피소드 0 에만 하고 기다렸다 — 에피소드 1-3 이 팔이 정지한 채 녹화됨 | `lerobot-record` 는 키 입력을 기다리지 않고 타이머 (녹화 30 s → 리셋 15 s → 저장 약 17 s) 로 다음 에피소드를 자동 시작한다. `Svt[info]` 줄과 `spd-say` 경고에 묻혀 `Recording episode N` 을 놓쳤다 | `Ctrl+C` 로 중단 (두 번째 `Ctrl+C` 가 업로드 직전의 연결 해제 단계에 걸려 Hub 에는 올라가지 않음). 로컬 폴더를 `so101-spike-pick-cube_aborted_20260920_2240` 으로 옮기고 같은 명령으로 처음부터 재녹화. 구간 전환은 다른 터미널의 `tail -f d9_record.log \| grep --line-buffered -aE "Recording episode\|Reset the environment\|Stop recording\|Exiting"` 로 신호 줄만 본다. 키로 진행을 잡으려면 `--dataset.reset_time_s=600` + `→` (녹화 시작 직전의 키 대기는 코드에 없다 — `lerobot_record.py:495-554`) | 약 13분 (22:40-22:53) |
| 10 | D9 로그가 `outputs/` 가 아니라 `/d9_record.log` 에 쌓임 (두 번) | 녹화를 실행한 셸에 `SPIKE_OUT` 이 없어 `tee -i $SPIKE_OUT/d9_record.log` 가 `/d9_record.log` 로 풀렸다 (가이드 §0.2 의 export 는 세션마다 필요) | 녹화에는 영향 없음. 끝난 뒤 `outputs/d9_record.log` 로 옮김 | — |
| 11 | zero-shot 에서 `elbow_flex` 가 목표 (약 +2도) 에 못 가고 +42.3도에서 완전히 정지 (5초 이후 표준편차 0.00). 그 관절의 `\|action - state\|` 가 전 구간에서 3도를 넘어 `Relative goal position magnitude had to be clamped` 경고가 거의 매 틱 찍힘 (1차 1003회 · 2차 999회) | 가장 유력한 설명: `--robot.max_relative_target=3` 은 서보에 보내는 목표를 "현재 위치 ± 3도" 로 묶는데, 서보는 P 제어라 위치 오차에 비례한 토크만 낸다. 오차가 3도로 묶이면 낼 수 있는 토크도 묶이고, 전완 + 그리퍼 + 손목 카메라를 수평으로 들어 올리는 중력 토크가 그 상한과 같아지는 각도에서 평형을 이룬다. 근거: D9 teleop (클램프 없음) 에서도 팔을 든 자세의 `elbow_flex` 는 목표보다 평균 +1.4도 · 최대 +2.6도 처져 있었고 (준정적 224 프레임), 0도 자세는 그리퍼를 앞으로 뻗어 부하가 더 크다. 영상에서 케이블은 느슨하고 물리적 걸림은 없다. 실험으로 확정한 것은 아니다 (클램프를 올려 재실행하면 가려진다) | must 3 ("반응하는가") 판정에는 무관. `max_relative_target` 은 속도 제한이면서 사실상 토크 제한으로도 작동한다 — v2.5 에서 안전 상한을 정할 때 "팔이 목표 자세를 중력에 맞서 유지할 수 있는 최소값" 을 먼저 잰다 | — |

## 5. 판정 (2026-09-20, 1회만 — plan §5.4 표 기준)

- **결과**: **must 4개 통과** (plan §5.4 표의 첫 행). must 1 · 4 는 2026-09-19, must 2 · 3 은 2026-09-20 에 확보했고 증거는 §1 의 네 행이다. 타임박스 (2026.09 첫 2주 + 판정일 09-21) 안에 끝났다. 통과 기준은 "되는가" 이고 품질이 아니다 — must 2 데이터셋의 알려진 결함 5개 (§1 행 2) 와 must 3 의 한계 (§4 #8 · #11) 는 판정을 바꾸지 않고 v2.5 의 입력이 된다
- **다음 행동**: Stage 1 본 빌드 2026.10 개시, v2.5 2026.11 개시
- **분기 재평가 #1 입력 요약**:
  - 일정: must 4개는 타임박스 안에 닫혔지만 계획한 "하루 한 항목" 이 아니라 22시 이후 1-2시간짜리 저녁 3회 (09-16 · 09-19 · 09-20) 에 몰렸고, must 2 · 3 은 판정 예정일 전날의 마지막 1시간에 끝났다. Week 1 은 계획 5일 + 버퍼 2일이었는데 기록상 09-12 하루 (조립부터 teleop 까지) 에 끝난 것으로 추정된다 — 구간 안에 커밋이 없어 확정하지 못한다 (§3). Stage 1 · v2.5 의 일정은 "일" 이 아니라 "저녁 1-2시간 세션" 단위로 잡고, 작업한 날마다 커밋을 남긴다. 마감에 붙여 끝내는 패턴이 반복되면 재평가에서 범위를 줄인다.
  - 막힌 지점: 11개 중 7개가 문서가 없거나 틀려서 걸렸고 (#1 · #2 · #3 · #4 · #6 · #7 · #8 — lerobot 0.6.2 와 컨테이너 환경이 가이드를 쓸 때의 가정과 달랐다), 2개는 문서에 있었는데도 걸렸다 (#9 · #10). 반대로 팔에 연결하기 전에 소스와 옵션 검사로 먼저 검증한 명령 (D10 `base` · `episodic`) 은 첫 실행에서 통과했다. v2.5 에서도 새 명령은 같은 방식으로 먼저 검증한다.
  - 기술: zero-shot 은 관측 → 모델 → 명령의 경로가 이어진다는 것만 보였고 과제 수행 능력에 대해서는 아무것도 말해 주지 않는다 — 정규화 통계가 적용되지 않아 정책 출력이 영상 · 지시문과 무관하게 0 근처였다 (§4 #8). v2.5 의 첫 항목은 통계 · 단위 규약 맞추기와 `max_relative_target` 하한 실측 (§4 #11) 이다. 데이터 수집 시간은 병목이 아니다: N=50 은 에피소드당 61.4 s 기준 51분, `→` 로 끊으면 약 38분이라 저녁 세션 1회에 들어간다. 단 must 2 데이터셋은 결함 5개 (§1 행 2) 때문에 v2.5 의 기준으로 쓰지 않는다.
