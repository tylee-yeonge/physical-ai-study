# week3 probe 실측 기록

> 확인일: 2026-08-12
> 용도: 본 LoRA 사이클(실습 4)의 `max_steps` 와 예상 비용을 역산하는 근거. 그리고 Section 0 의 "OpenVLA 7B LoRA 1사이클이 RTX 4090 에서 가능한지" 판정
> 대상: RTX 4090 24GB pod (`runpod_setup.md` §3), `openvla-train:v2`, `maniskill_pickcube_only`
> 결과: **게이트 통과.** VRAM 피크 18,470MiB / 24,564MiB, 스텝당 2.67초

## 1. 실행 조건

| 항목 | 값 |
|---|---|
| GPU | RTX 4090 24,564MiB (`runpod_setup.md` §3) |
| `batch_size` / `grad_accumulation_steps` | 1 / 16 (유효 배치 16) |
| `lora_rank` | 32 |
| `max_steps` | 20 |
| `save_steps` | 1000 (= probe 구간에 저장이 걸리지 않게) |
| 데이터 / 통계 | `/workspace/data`, 기존 통계 캐시 재사용 |
| wandb | `WANDB_MODE=offline` |

`exp_id` 는 `openvla-7b+maniskill_pickcube_only+b16+lr-0.0005+lora-r32+dropout-0.0--image_aug` 로 생성됐다. `b16` 은 `batch_size x grad_accumulation_steps` 이고, 학습률 5e-4 와 image augmentation 은 스크립트 기본값이다.

## 2. 실측치

| 항목 | 값 | 출처 |
|---|---|---|
| 스텝당 시간 | **2.67초** | tqdm `20/20 [00:58<00:00, 2.67s/it]` |
| 20스텝 학습 구간 | 58초 | 같은 줄 |
| 전체 소요 | **2분 31초** | `time` 의 real |
| 모델 로딩 + 데이터셋 준비 | 약 93초 | 전체 151초 - 학습 58초 |
| VRAM 피크 | **18,470MiB** | 2초 간격 기록의 최댓값 |
| VRAM 여유 | 약 6,090MiB (사용률 75%) | 24,564 - 18,470 |
| trainable params | 110,828,288 / 7,652,065,472 (1.4483%) | 스크립트 출력 |

`Max step 20 reached! Stopping training...` 가 출력됐다 — `--max_steps` 가 무시되지 않고 인식됐다는 확인이다 (기본값은 200,000).

**로딩 93초는 스텝 수에 비례하지 않는 고정 비용**이다. 총 시간을 스텝 수로 나누면 스텝당 시간이 과대평가되므로, 역산에는 tqdm 이 보고한 2.67초를 쓴다.

## 3. Section 0 게이트 판정

| 항목 | 판정 | 근거 |
|---|---|---|
| OpenVLA 7B LoRA 1사이클이 RTX 4090 에서 가능한가 | **통과** | 유효 배치 16 조합에서 OOM 없이 20스텝 완주. VRAM 18.5GB 로 여유 6GB |

README §1 이 예고한 27GB 하한을 `batch_size 1 + grad_accumulation 16` 조합으로 24GB 안에 넣었다. 여유가 6GB 남았으므로 배치를 더 낮출 필요는 없다.

## 4. `max_steps` 역산

코드 확인 결과 **`max_steps` 는 옵티마이저 갱신 횟수**다. `tqdm` 의 진행 단위가 optimizer step 블록 안에서 갱신되므로(`finetune.py:315-318`), 실측 2.67초는 **gradient step 하나당** 시간이고 micro-batch 하나는 그 1/16 인 약 0.167초다.

데이터셋이 3,760 프레임이고 유효 배치가 16 이므로 **1 epoch = 235 gradient step** 이다. GPU 요금 시간당 $0.75 (`runpod_setup.md` §3) 기준으로 계산한다.

| `max_steps` | epoch | 학습 시간 | `save_steps` | 저장 횟수 | 저장 시간 | 총 시간 | GPU 비용 |
|---|---|---|---|---|---|---|---|
| 1,000 | 4.3 | 44분 | 500 | 2 | 18분 | 62분 | 약 $0.78 |
| **2,000** | **8.5** | **89분** | **1,000** | **2** | **18분** | **107분** | **약 $1.34** |
| 2,000 | 8.5 | 89분 | 500 | 4 | 36분 | 125분 | 약 $1.56 |
| 3,000 | 12.8 | 133분 | 1,000 | 3 | 27분 | 160분 | 약 $2.00 |

여기에 로딩 고정 비용 약 2분을 더한다. 저장 시간은 §5 의 실측 추정(이벤트당 약 9분)을 곱한 값이다.

**선택: `max_steps 2000` + `save_steps 1000`.** 소규모 데이터에 8.5 epoch 은 LoRA 파인튜닝의 통상 범위이고, 저장이 총 시간의 17% 로 억제된다. pod 이 Secure Cloud 라 인스턴스 회수 위험이 낮아 자주 저장할 이유가 약하다.

**`max_steps` 는 `save_steps` 의 배수여야 한다.** 저장 조건(`finetune.py:321`)이 `gradient_step_idx % save_steps == 0` 이고 종료 조건(`367`)이 그 뒤에 평가되므로, 배수가 아니면 **마지막 학습 상태가 저장되지 않고 끝난다.**

20스텝 probe 는 갱신 20회 = 데이터셋 0.085 epoch 이다. **학습이 진행된 실행이 아니라 파이프라인이 도는 것만 확인한 실행**으로 취급한다.

## 5. 첫 실행에서 드러난 것 — 저장 비용

`--save_steps 10` 으로 돌린 첫 probe 는 20스텝인데 **10분 이상 걸렸다** (07:37 학습 시작 -> 07:47 가중치 파일 완료). 학습 자체는 1분이 안 되므로 나머지가 전부 저장 시간이다.

로그에 `Saving Model Checkpoint for Step 10` 이 **동일한 스텝에 대해 반복**됐고, 매번 `Loading checkpoint shards` 가 함께 떴다.

원인은 코드에 있다. 저장 조건이 배치 루프 안에 있고(`finetune.py:321`) `gradient_step_idx = batch_idx // grad_accumulation_steps`(`294`) 이므로, **같은 `gradient_step_idx` 가 유지되는 동안 조건이 계속 참이 되어 `grad_accumulation_steps` 횟수만큼 저장이 반복된다.** 지금 조합에서는 16회다.

한 번의 저장은 base 3 shard 를 다시 읽어 어댑터를 머지하고 15GB 를 network volume 에 쓴다. 그것이 16회 반복되므로 **저장 이벤트 1회 = 약 9분**이다 (실측 10분에서 학습 1분을 뺀 값).

| 항목 | 값 |
|---|---|
| 저장 이벤트 1회 | 약 10분 (내부적으로 15GB 쓰기 x 16) |
| 반복 횟수 | `grad_accumulation_steps` 와 같다 — 지금 조합에서 16회 |
| 1회당 | 약 37초 (base 3 shard 읽기 + 머지 + 15GB 쓰기) |
| 저장 방식 | `save_latest_checkpoint_only = True` 가 기본(`finetune.py:94`) — run 디렉터리에 **덮어쓰기** |

본 사이클(2,000 스텝) 실행에서 스텝 1000 저장이 약 10분간 이어지는 것을 확인했다. 진행 중인지는 반복 횟수로 센다.

```
grep -c "Saved Model Checkpoint for Step 1000" /workspace/train.log
```

이 값이 `grad_accumulation_steps` 에 도달하면 저장이 끝난다. 진행률 표시가 멈춘 것처럼 보이는 구간이라 **죽은 것과 구별하는 유일한 방법**이다.

덮어쓰기가 기본이라는 점은 볼륨 용량 측면에서 유리하다. 체크포인트가 시점별로 쌓이지 않으므로 15GB 하나만 유지된다. 반대로 **되돌릴 지점이 없다**는 뜻이기도 하다 — loss 가 발산했을 때 이전 상태로 돌아갈 수 없다 (README §5 의 트레이드오프).

실용적 결론: **`save_steps` 를 작게 두면 저장이 학습 시간을 압도한다.** 2,000 스텝에 `save_steps 500` 이면 저장 4회 = 36분이 학습 89분에 더해진다.

## 6. 남은 최적화 여지 (미확인)

VRAM 여유가 6GB 다. `batch_size` 를 2 로 올리고 `grad_accumulation_steps` 를 8 로 낮추면 유효 배치 16 을 유지하면서 **두 곳에서 동시에 시간이 줄어든다.**

| 조합 | 유효 배치 | gradient step 당 micro-batch | 저장 반복 횟수 | 저장 이벤트 1회 |
|---|---|---|---|---|
| batch 1 / accum 16 | 16 | 16 | 16 | 약 10분 |
| batch 2 / accum 8 | 16 | 8 | **8** | **약 5분** |

- **학습 쪽**: micro-batch 하나의 시간이 배치 크기에 정비례하지는 않는다 (배치가 작으면 GPU 가 놀고 있다). 절반까지는 아니어도 gradient step 당 시간이 줄어든다
- **저장 쪽**: `grad_accumulation_steps` 가 곧 저장 반복 횟수다 (§5). 8 로 낮추면 저장 이벤트 비용도 절반이 된다

두 번째 줄이 §5 를 실측하고 나서 드러난 이득이다. 저장 한 번에 10분이 드는 상황에서 이것은 학습 시간 단축과 같은 무게를 갖는다.

확인 비용은 20스텝 probe 하나(저장 없음, 약 2-3분)다. 다만 활성값이 늘어 VRAM 이 얼마나 오르는지는 재 봐야 알고, **용량의 90% 를 넘으면 채택하지 않는다** — 몇 시간짜리 학습에서 경계에 붙은 조합은 시퀀스 길이 변동만으로 OOM 이 된다.

## 7. 통계 파일 관찰

| 실행 | 통계 파일 |
|---|---|
| 실습 2-5 로드 검증 | `dataset_statistics_76ad416b...cbe.json` 재사용 |
| probe (학습 경로) | `dataset_statistics_4037d689...872b.json` — 첫 실행이 계산, 두 번째가 재사용 |
| probe 결과물 | `/workspace/runs/<exp_id>/dataset_statistics.json` (2,298 바이트) 저장 |

캐시 파일명의 해시가 갈린 것은 **데이터셋 kwargs 조합이 달라 캐시 키가 다르기 때문**이다. 로드 검증은 확인용 최소 설정으로 로더를 띄웠고 학습 경로는 image augmentation 을 포함한 실제 설정을 쓴다. 통계 값 자체는 같은 데이터에서 계산되므로 동일할 것으로 보이지만, **두 파일의 값을 직접 대조하지는 않았다.**

추론에서 `unnorm_key` 의 근거가 되는 것은 세 번째 줄, 즉 **run 디렉터리에 저장된 `dataset_statistics.json`** 이다 (README §6). 회수 목록의 필수 항목이다.

## 8. 이 기록이 보장하지 않는 것

- **학습이 유효한지는 확인하지 않았다.** 20스텝은 갱신 1회 남짓이고, loss 추이는 실습 4 가 처음 본다. loss 가 내려가는 것조차 통과 기준이 아니다 (README §8)
- **저장 동작을 검증하지 않았다.** 이 probe 는 저장을 의도적으로 끄고 측정했다. 체크포인트가 재개에 쓸 수 있는 상태인지는 실습 5 가 확인한다
- **LoRA 어댑터 원본이 volume 밖에 있었다.** 첫 실행은 `adapter_tmp_dir` 기본값(컨테이너 안 상대 경로)으로 돌아 `/opt/openvla/adapter-tmp/` 에 떨어졌다. pod 를 멈추면 사라지는 위치다. 두 번째 실행부터 `/workspace/adapter-tmp` 를 명시했다
- **volume 잔량을 `df` 로 볼 수 없다.** 공유 파일시스템이라 클러스터 전체 용량이 표시된다. base 캐시 15GB + 머지 체크포인트 15GB 로 50GB 볼륨의 60% 가 이미 찼고, `du -sh` 로 직접 재야 한다
