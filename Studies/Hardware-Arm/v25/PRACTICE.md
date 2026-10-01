# v2.5 PRACTICE — 명령 골격

> 명령어·설정 키는 LeRobot 버전에 따라 다르다 — **착수 시 https://huggingface.co/docs/lerobot 의 SmolVLA·SO-101 페이지로 재확인**하고 아래 골격을 맞춘다. 기준선은 스파이크에서 lerobot 0.6.2 로 검증한 명령이다 — 전체 인자 (포트 · `id` · 카메라 설정 · `--dataset.no_stamp=true` · `--display_data=false`) 는 [`../spike/week2/week2_guide.md`](../spike/week2/week2_guide.md) §2.2 (녹화) · §3 (정책 실행) 에 있다.
>
> **착수 전에 먼저 닫을 것 2개** (`../spike/week2/RESULT.md` §4 #8 · #11): ① `smolvla_base` 의 정규화 통계 키가 `so100.` 접두어라 0.6.2 에서 역정규화가 적용되지 않는다 — 통계 · 단위 규약 (관절은 도, 그리퍼는 0-100) 을 맞추기 전에는 zero-shot 출력이 영상 · 지시문과 무관하게 0 근처다 ② `--robot.max_relative_target` 은 속도 제한이면서 사실상 토크 제한이다 — 팔이 목표 자세를 중력에 맞서 유지할 수 있는 최소값을 먼저 잰다.

## 1. 본 수집 (Stage 1 완료 상태에서)

스파이크 must 2 와 같은 규약 (시작 자세 통일, 에피소드 길이 상한, 배치 번호 메모) 이되, must 2 데이터셋의 결함 5개 (RESULT §1 행 2) 는 되풀이하지 않는다 — 시범이 끝나면 `→` 로 에피소드를 끊는다 (10-01 테스트 녹화에서도 안 눌러 정지 꼬리 9-13초가 들어갔다), 지시문의 물체 색 · 목표물은 실제와 맞춘 문장 (아래), 손목 카메라 케이블은 작업 영역을 가로지르지 않게 (팔과 함께 움직이는 구간은 남는다). `lerobot-record` 는 키 입력을 기다리지 않고 타이머로 다음 에피소드를 시작한다 (RESULT §4 #9).

시작 전 점검은 week2_guide §0.2 의 세션 점검 + §2.1 의 구도 체크 (기준 프레임 `../spike/week2/outputs/ref_overview.png` 와 실시간 화면 비교, 뷰어는 끈다). 카메라 설정은 week2_guide §1.4 와 같다 (두 대 모두 1280x720 @ 30 MJPG).

```bash
acl
mkdir -p /workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/outputs
export CAMS="{ front: {type: opencv, index_or_path: /dev/so101_cam_overview, width: 1280, height: 720, fps: 30, fourcc: MJPG}, wrist: {type: opencv, index_or_path: /dev/so101_cam_wrist, width: 1280, height: 720, fps: 30, fourcc: MJPG} }"
ls -la /dev/so101_* && echo $CAMS $HF_USER && hf auth whoami   # 노드 4개 · 두 변수 · 계정이 찍혀야 한다
pgrep -af "live_view|ros2_control_node" || echo "뷰어 · ROS2 꺼짐"   # 켜져 있으면 카메라 · 포트 연결에서 실패한다
ls -d ~/.cache/huggingface/lerobot/$HF_USER/so101-pick-cube-v25 2>/dev/null || echo "로컬에 같은 이름 없음 (정상)"
lerobot-record \
    --robot.type=so101_follower \
    --robot.port=$FOLLOWER_PORT \
    --robot.id=so101_follower_01 \
    --robot.cameras="$CAMS" \
    --teleop.type=so101_leader \
    --teleop.port=$LEADER_PORT \
    --teleop.id=so101_leader_01 \
    --display_data=false \
    --dataset.repo_id=$HF_USER/so101-pick-cube-v25 \
    --dataset.no_stamp=true \
    --dataset.num_episodes=50 \
    --dataset.fps=30 \
    --dataset.episode_time_s=30 \
    --dataset.reset_time_s=15 \
    --dataset.single_task="Pick up the pink cube and place it in the yellow square." \
    --dataset.private=true \
    --dataset.push_to_hub=true \
    2>&1 | tee -i /workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/outputs/record.log
```

- `--dataset.num_episodes=50` 은 README §1 의 출발점이다. 0번 (측정 설계) 에서 N 을 확정하면 그 값으로 바꾼다. 여러 세션에 나눠 찍을 때는 같은 명령에 `--resume=true` 를 붙이고 `num_episodes` 를 **이번 세션에서 추가로 찍을 개수**로 바꾼다 (week2_guide §2.2 "중간에 끊겼을 때").
- `--dataset.no_stamp=true`: 이름 뒤에 날짜가 붙지 않아 로컬 폴더와 Hub 이름이 고정된다. 이어 찍기와 학습 (§3) 이 같은 이름을 쓴다.
- 녹화 중 키 조작과 에피소드 리듬 (녹화 → 리셋 → 저장 약 20초) 은 week2_guide §2.2-§2.3. 큐브를 놓친 시범은 `←` 로 버린다 (expert 데이터만).
- 에피소드 메타에 배치 번호를 남긴다 (태그 또는 별도 CSV — 0번에서 형식 확정).

## 2. 실기 eval 루프 (zero-shot / fine-tuned 공용)

lerobot 0.6.2 에서 정책 실행은 `lerobot-rollout` 이다 (`lerobot-record` 는 녹화 전용). 영상 · 관절값을 데이터셋으로 남기는 `--strategy.type=episodic` 을 쓴다. 카메라 이름은 모델 config 의 키에 맞춘다 — `camera1` = 전체 뷰, `camera2` = 손목 (SmolVLA 사전학습 규약, week2_guide §3.2). 실행 절차 · 안전 준비 · 터미널에 찍히는 것 · 멈추는 법은 week2_guide §3.3-§3.4 그대로다.

한 번 실행 = 배치 마커 1개 = 에피소드 1개. 배치 마커 i 에 큐브를 놓고 → 아래 명령 1회 → 4단계 판정 (reached / grasped / lifted / placed) 을 기록한다. 데이터셋 이름에 lerobot 이 시각을 붙이므로 매 실행이 다른 폴더에 남는다. ABBA 세션 교차 (같은 배치에서 zero-shot · fine-tuned 를 번갈아) 와 판정 규칙은 README §2 에서 고정한 문서 기준.

```bash
acl
mkdir -p /workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/outputs
export CAMS_ZS="{ camera1: {type: opencv, index_or_path: /dev/so101_cam_overview, width: 1280, height: 720, fps: 30, fourcc: MJPG}, camera2: {type: opencv, index_or_path: /dev/so101_cam_wrist, width: 1280, height: 720, fps: 30, fourcc: MJPG} }"
pgrep -af "live_view|ros2_control_node" || echo "뷰어 · ROS2 꺼짐"   # 켜져 있으면 카메라 · 포트 연결에서 실패한다
```

zero-shot (A):

```bash
lerobot-rollout \
    --strategy.type=episodic \
    --policy.path=lerobot/smolvla_base \
    --policy.device=cuda \
    --robot.type=so101_follower \
    --robot.port=$FOLLOWER_PORT \
    --robot.id=so101_follower_01 \
    --robot.cameras="$CAMS_ZS" \
    --robot.max_relative_target=3 \
    --task="Pick up the pink cube and place it in the yellow square." \
    --fps=30 \
    --dataset.repo_id=$HF_USER/rollout_so101-pick-cube-v25-zeroshot \
    --dataset.num_episodes=1 \
    --dataset.episode_time_s=30 \
    --dataset.push_to_hub=false \
    2>&1 | tee -a -i /workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/outputs/eval_zeroshot.log
```

fine-tuned (B) — §3 의 마지막 체크포인트:

```bash
lerobot-rollout \
    --strategy.type=episodic \
    --policy.path=/workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/outputs/smolvla_v25/checkpoints/last/pretrained_model \
    --policy.device=cuda \
    --robot.type=so101_follower \
    --robot.port=$FOLLOWER_PORT \
    --robot.id=so101_follower_01 \
    --robot.cameras="$CAMS_ZS" \
    --robot.max_relative_target=3 \
    --task="Pick up the pink cube and place it in the yellow square." \
    --fps=30 \
    --dataset.repo_id=$HF_USER/rollout_so101-pick-cube-v25-finetuned \
    --dataset.num_episodes=1 \
    --dataset.episode_time_s=30 \
    --dataset.push_to_hub=false \
    2>&1 | tee -a -i /workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/outputs/eval_finetuned.log
```

- `--robot.max_relative_target=3` 은 스파이크에서 쓴 안전값이다. 착수 전 항목 ② 에서 "목표 자세를 유지할 수 있는 최소값" 을 재면 그 값으로 바꾼다 — 3 으로는 `elbow_flex` 가 뻗은 자세에 못 간다 (RESULT §4 #11).
- zero-shot 은 착수 전 항목 ① (정규화 통계 키) 을 닫은 뒤에 돌린다. 안 닫으면 출력이 영상과 무관하게 0 근처다.
- `--dataset.repo_id` 는 `rollout_` 로 시작해야 한다. `tee -a` 라 실행마다 같은 로그 파일 뒤에 이어 붙는다.
- 끝난 뒤 수치 확인 (관절별 이동 폭, 정책 출력 범위) 은 week2_guide §3.5 의 parquet 스니펫.

기록 형식: v1.5 `eval_*.jsonl` 스키마 재사용 (메타 1줄 + 에피소드 N줄) — 분석 스크립트 (`analyze_results.py`) 를 그대로 다시 쓴다.

## 3. SmolVLA 파인튜닝 (로컬 4070)

팔 · 카메라 없이 GPU 만 쓴다 (ROS2 · 뷰어와 겹쳐도 된다). 데이터셋은 §1 에서 Hub 에 올린 것을 받아 쓴다. 공식 SmolVLA 파인튜닝 예시 (batch 64 · 20000 steps) 를 4070 12GB 에 맞춰 batch 를 줄인 것이다.

```bash
acl
lerobot-train \
    --policy.path=lerobot/smolvla_base \
    --policy.device=cuda \
    --policy.push_to_hub=false \
    --dataset.repo_id=$HF_USER/so101-pick-cube-v25 \
    --rename_map='{"observation.images.front": "observation.images.camera1", "observation.images.wrist": "observation.images.camera2"}' \
    --output_dir=/workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/outputs/smolvla_v25 \
    --job_name=smolvla_v25 \
    --batch_size=8 \
    --steps=20000 \
    --save_freq=5000 \
    --wandb.enable=false \
    2>&1 | tee -i /workspace/study/physical-ai-study/Studies/Hardware-Arm/v25/outputs/train.log
```

- `--rename_map`: 데이터셋의 이미지 키 (`front` · `wrist`) 를 사전학습 모델의 키 (`camera1` · `camera2`) 로 바꿔 넣는다. 시작 로그의 input_features 에 `camera1` · `camera2` 가 잡히는지 확인한다 — 안 잡히면 영상 없이 학습된다.
- `--batch_size=8` 은 출발값이다. 첫 100 step 동안 `nvidia-smi` 로 VRAM 을 보고 여유가 있으면 16 · 32 로 올린다. OOM 이면 줄인다 (v1.5 에서 쓴 수법 — 작은 batch + 긴 steps).
- `--policy.push_to_hub=false`: 체크포인트를 Hub 에 올리지 않는다 (기본값이 올림).
- `--output_dir` 는 없어야 한다 — 있으면 `FileExistsError`. 이어서 학습하려면 `--resume=true`.
- 체크포인트는 `outputs/smolvla_v25/checkpoints/<step>/pretrained_model`, 마지막 것은 `checkpoints/last/pretrained_model` (§2 의 B 가 이 경로를 쓴다). `outputs/` 는 gitignore 대상이다.
- 기록할 것: loss 곡선 (`train.log` 의 loss 줄), 체크포인트 주기, VRAM 피크, 벽시계 시간 — Measurements environment 용.

## 4. 비교표 + 발행

| 지표 | OpenVLA int4 (sim, v1.5) | SmolVLA zero-shot (real) | SmolVLA fine-tuned (real) |
|---|---|---|---|
| placed (N쌍, Wilson) | 0/98 [0, 3.8%] | | |
| reached / grasped / lifted | 0 / 0 / 0 | | |
| latency (동일 4070) | 300.3 ms (action 1개, int4) | 106.3 ms / chunk (action 50개 → action 당 2.13 ms, bfloat16, 2026-09-19) | |

- README 실측 절 갱신 + Measurements 디렉토리 1개 (environment/methodology/findings — 경량) + vla-lab 글 1편 (v1.5 의 real 후속편). **여기까지가 상한.**
