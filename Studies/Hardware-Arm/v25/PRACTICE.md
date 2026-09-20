# v2.5 PRACTICE — 명령 골격

> 명령어·설정 키는 LeRobot 버전에 따라 다르다 — **착수 시 https://huggingface.co/docs/lerobot 의 SmolVLA·SO-101 페이지로 재확인**하고 아래 골격을 맞춘다. 기준선은 스파이크에서 lerobot 0.6.2 로 검증한 명령이다 — 전체 인자 (포트 · `id` · 카메라 설정 · `--dataset.no_stamp=true` · `--display_data=false`) 는 [`../spike/week2/week2_guide.md`](../spike/week2/week2_guide.md) §2.2 (녹화) · §3 (정책 실행) 에 있다.
>
> **착수 전에 먼저 닫을 것 2개** (`../spike/week2/RESULT.md` §4 #8 · #11): ① `smolvla_base` 의 정규화 통계 키가 `so100.` 접두어라 0.6.2 에서 역정규화가 적용되지 않는다 — 통계 · 단위 규약 (관절은 도, 그리퍼는 0-100) 을 맞추기 전에는 zero-shot 출력이 영상 · 지시문과 무관하게 0 근처다 ② `--robot.max_relative_target` 은 속도 제한이면서 사실상 토크 제한이다 — 팔이 목표 자세를 중력에 맞서 유지할 수 있는 최소값을 먼저 잰다.

## 1. 본 수집 (Stage 1 완료 상태에서)

```bash
# 스파이크 must 2 와 같은 규약: 시작 자세 통일, 에피소드 길이 상한, 배치 번호 메모
# must 2 데이터셋의 결함 5개 (RESULT §1 행 2) 는 되풀이하지 않는다: 시범이 끝나면 → 로 에피소드를 끊고,
# 지시문의 물체 색 · 목표물을 실제와 맞추고, 손목 카메라 케이블을 화면 밖으로 정리한다.
# lerobot-record 는 키 입력을 기다리지 않고 타이머로 다음 에피소드를 시작한다 (RESULT §4 #9)
lerobot-record \
  --robot.type=so101_follower --teleop.type=so101_leader \
  --dataset.repo_id=<id>/so101-<task>-v25 --dataset.num_episodes=<실측 재산정> \
  --dataset.single_task="<task instruction>"
# 에피소드 메타에 배치 번호를 남긴다 (태그 또는 별도 CSV)
# 품질 게이트: 수집 중 실패 에피소드는 그 자리에서 재녹화 (expert 데이터만)
```

## 2. 실기 eval 루프 (zero-shot / fine-tuned 공용)

```bash
# lerobot 0.6.2 에서 정책 실행은 lerobot-rollout 이다 (lerobot-record 는 녹화 전용).
# 영상 · 관절값을 데이터셋으로 남기려면 --strategy.type=episodic. 카메라 이름은 camera1 (손목) · camera2 (전체 뷰).
# zero-shot: --policy.path=lerobot/smolvla_base
# fine-tuned: --policy.path=<로컬 체크포인트>
# 배치 마커 i 에 물체 배치 → 에피소드 1회 → 4단계 판정 기록 (reached/grasped/lifted/placed)
# ABBA 세션 교차, 판정 규칙은 README §2 에서 고정한 문서 기준
```
기록 형식: v1.5 `eval_*.jsonl` 스키마 재사용 (메타 1줄 + 에피소드 N줄) — 분석 스크립트 (`analyze_results.py`) 를 그대로 다시 쓴다.

## 3. SmolVLA 파인튜닝 (로컬 4070)

```bash
lerobot-train \
  --policy.path=lerobot/smolvla_base \
  --dataset.repo_id=<id>/so101-<task>-v25 \
  --output_dir=outputs/smolvla_v25 --batch_size=<VRAM 실측으로> --steps=<기본값에서 시작>
# 확인: loss 곡선 저장, 체크포인트 주기, VRAM 피크 기록 (Measurements environment 용)
# 4070 12GB 에서 OOM 시: batch 축소 → gradient accumulation (v1.5 에서 쓴 수법)
```

## 4. 비교표 + 발행

| 지표 | OpenVLA int4 (sim, v1.5) | SmolVLA zero-shot (real) | SmolVLA fine-tuned (real) |
|---|---|---|---|
| placed (N쌍, Wilson) | 0/98 [0, 3.8%] | | |
| reached / grasped / lifted | 0 / 0 / 0 | | |
| latency (동일 4070) | 300.3 ms (action 1개, int4) | 106.3 ms / chunk (action 50개 → action 당 2.13 ms, bfloat16, 2026-09-19) | |

- README 실측 절 갱신 + Measurements 디렉토리 1개 (environment/methodology/findings — 경량) + vla-lab 글 1편 (v1.5 의 real 후속편). **여기까지가 상한.**
