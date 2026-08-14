# week3 본 LoRA 사이클 기록

> 확인일: 2026-08-13
> 용도: Section 0 의 "OpenVLA 7B LoRA 1사이클이 RTX 4090 에서 가능한지" 를 완주로 확정한 기록. week4 가 쓸 산출물의 소재와 검증 상태
> 대상: RTX 4090 24GB pod (`runpod_setup.md` §3), `openvla-train:v2`, `maniskill_pickcube_only`
> 결과: **2,000 스텝 완주.** 총 94분, `train_loss` 10.75 -> 0.40, action token 정확도 0.12 -> 0.87

## 1. 실행 조건

| 항목 | 값 |
|---|---|
| `batch_size` / `grad_accumulation_steps` | 1 / 16 (유효 배치 16) |
| `max_steps` / `save_steps` | 2,000 / 1,000 |
| `lora_rank` / `lora_alpha` / `lora_dropout` | 32 / 16 / 0.0 |
| `learning_rate` | 5e-4 (스크립트 기본값) |
| `image_aug` | True (기본값) |
| `run_root_dir` / `adapter_tmp_dir` | `/workspace/runs` / `/workspace/adapter-tmp` |
| wandb | `WANDB_MODE=offline`, `WANDB_DIR=/workspace/wandb` |
| 실행 환경 | tmux 세션 안, `set -eo pipefail` |

`exp_id` = `openvla-7b+maniskill_pickcube_only+b16+lr-0.0005+lora-r32+dropout-0.0--image_aug`

데이터셋 3,760 프레임 / 유효 배치 16 이므로 2,000 스텝은 **약 8.5 epoch** 이다. 학습 스크립트는 `train[:95%]` 만 사용한다.

## 2. 실측 — 시간

| 항목 | 값 |
|---|---|
| gradient step 당 | **2.51-2.58초** (전 구간 안정) |
| 총 소요 | **94.3분** (wandb `_runtime` 기준) |
| 스텝 1000 저장 | 약 7분 (16회 반복) |
| 스텝 2000 저장 | 약 30초 (1회) |
| GPU 비용 | 94.3분 x $0.75/hr = **약 $1.18** |

probe 실측 2.67초와 6% 차이로, `probe_measure.md` §4 의 역산 근거가 유효했음이 확인됐다.

### 2.1 저장 비용은 실행마다 크게 변동한다

같은 조합(`grad_accumulation_steps 16`, 15GB 머지 저장 x 16회)에서 저장 이벤트 1회가 **7-25분 범위**로 관측됐다. network volume 이 MooseFS 기반 공유 파일시스템이라 **쓰기 속도가 그때의 클러스터 상태에 좌우되기 때문**으로 보인다. pod 개체와 볼륨이 달라지면 재현되지 않는다.

계획에는 보수적인 값(회당 25분)을 쓰고, 실측은 매 실행 기록한다.

### 2.2 마지막 저장은 반복되지 않는다

| 스텝 | `Saved Model Checkpoint` 출력 |
|---|---|
| 1000 | 16회 |
| 2000 | **1회** |

`max_steps` 에 도달한 저장은 1회로 끝난다. 저장 조건(`finetune.py:321`) 직후 종료 조건(`367`)이 평가되어 첫 저장 뒤 곧바로 `break` 하기 때문이다.

**계획 규칙**: 저장 총비용 = (저장 횟수 - 1) x 저장 1회 + 마지막 1회분.

## 3. 학습 지표

원본: `loss_history.csv` (wandb offline 기록에서 추출, 201개 표본, 10 스텝 간격)

| 스텝 | `train_loss` | action 정확도 | `l1_loss` |
|---|---|---|---|
| 0 | 10.7541 | 0.1161 | 0.41169 |
| 200 | 1.5870 | 0.5536 | 0.09769 |
| 400 | 1.5177 | 0.5536 | 0.07087 |
| 600 | 1.1750 | 0.6696 | 0.05714 |
| 800 | 0.9453 | 0.6964 | 0.05028 |
| 1000 | 0.5840 | 0.8304 | 0.02458 |
| 1200 | 0.5867 | 0.8304 | 0.02920 |
| 1400 | 0.4669 | 0.8571 | 0.01877 |
| 1600 | 0.4309 | 0.8214 | 0.01078 |
| 1800 | 0.3796 | 0.8482 | 0.02801 |
| 2000 | 0.3968 | 0.8661 | 0.02570 |

마지막 10개 표본(스텝 1910-2000) 평균: `train_loss` **0.4311**, 정확도 **0.8464**, `l1_loss` **0.01741**

세 지표가 무엇인지 정리한다.

| 지표 | 뜻 |
|---|---|
| `train_loss` | action 을 토큰으로 이산화해 예측하는 교차 엔트로피. 모델이 정답 토큰에 부여한 확률이 낮을수록 크다 |
| action 정확도 | 예측한 action 토큰이 정답 토큰과 일치한 비율 |
| `l1_loss` | 토큰을 되돌린 연속값과 정답의 평균 절대 오차 (정규화 공간) |

읽히는 것은 이렇다.

- **초기 200스텝에서 급감**(10.75 -> 1.59)한 뒤 완만해진다. 사전학습된 모델이 새 action 분포에 맞춰지는 단계가 앞쪽에서 끝난다
- **1000스텝 이후 개선 폭이 작다** (0.58 -> 0.40). 8.5 epoch 근처에서 수렴 조짐이 보이므로, 스텝을 더 늘려 얻을 이득은 크지 않을 가능성이 있다
- `l1_loss` 0.026 은 정규화 공간의 값이므로, 실제 미터·라디안 오차로 환산하려면 `dataset_statistics.json` 의 std 를 곱해야 한다

**그러나 이 수치는 통과 기준이 아니다.** README §8 이 지적한 대로, 라벨이 일관되게 틀려 있어도 모델은 그 틀린 라벨을 잘 맞추게 되고 loss 는 예쁘게 떨어진다. 라벨 자체의 정합성은 week2 의 조용한 실패 검증(`norm_check.md`)이 담당하며, 그 검증은 통과했다.

## 4. 산출물과 회수

| 산출물 | 크기 | 회수 |
|---|---|---|
| LoRA 어댑터 `adapter_model.safetensors` | **462MB** | 완료 |
| `adapter_config.json` | 988B | 완료 |
| `dataset_statistics.json` | 2.3KB | 완료 |
| processor / tokenizer 설정 (8개 파일) | 약 2.4MB | 완료 |
| wandb offline 기록 | 755KB | 완료 |
| `train.log` | 149KB | 완료 |
| 머지 가중치 `model-0000N-of-00004.safetensors` | 약 15GB | 미회수 (§4.3) |

로컬 회수 위치: `outputs/recovered/`. 추출한 지표는 `outputs/loss_history.csv`.

### 4.1 어댑터 검증

```
r = 32, lora_alpha = 16, lora_dropout = 0.0
base_model_name_or_path = openvla/openvla-7b
base 리비전 = 47a0ec7fc4ec123775a391911046cf33cf9ed83f
target_modules = fc1 fc2 fc3 q k_proj kv proj qkv q_proj v_proj o_proj
                 gate_proj up_proj down_proj lm_head
```

`target_modules` 에 **`lm_head` 가 포함**된 것이 어댑터가 462MB 인 이유다. 어휘 크기 x 임베딩 차원 행렬이 함께 저장되며, 저장 시 `save_embedding_layers=True` 경고가 그 신호다. rank 32 어댑터만이라면 수십 MB 규모다.

**로컬 HuggingFace 캐시의 스냅샷 해시가 위 리비전과 일치한다.** 재머지의 핵심 전제가 충족됐다.

### 4.2 통계 파일 검증

```
action.mean[6] = 0.4319   (gripper)
action.std[6]  = 0.4953
```

dim6 평균이 **0.4319** 로 week1 실측의 "1 의 비율 0.432" 와 일치한다. 라벨이 빌드 -> 로더 -> 학습을 통과하며 변형되지 않았다는 뜻이다. 이 파일이 추론에서 `unnorm_key` 의 근거가 된다 (README §6).

### 4.3 머지 가중치는 재머지로 만든다

15GB 를 내리지 않았다. 머지 가중치 = base + 어댑터이고 base 15GB 가 로컬 캐시에 있으므로, 어댑터로 다시 합치면 같은 모델이 된다. **머지는 학습과 같은 버전 조합에서 수행한다.**

```bash
docker run --rm \
    -v <회수 경로>:/recover \
    -v ~/.cache/huggingface:/root/.cache/huggingface \
    openvla-train:v2 bash
```

`openvla-train:v2` 안에 peft 0.11.1 / transformers 4.40.1 / torch 2.2.0 이 들어 있다. 호스트의 기본 python 환경은 torch 2.12 / timm 1.0.27 로 openvla 핀과 다르므로 쓰지 않는다. 머지 연산 자체는 GPU 가 필요 없다 — bf16 가중치에 수정분을 더하는 CPU 작업이고, 4070 12GB 에는 15GB 가 올라가지 않는다.

### 4.4 회수 경로

`runpodctl` 은 pod -> 로컬 방향에서 쓸 수 없다. 로컬이 고포트로 아웃바운드 연결을 걸어야 하는데 그것이 막히면 **연결은 맺어지고 데이터가 한 바이트도 오지 않는다.** 반대 방향(로컬 -> pod)은 pod 쪽이 연결을 걸어 성공한다.

**network volume 의 S3 호환 API** 가 동작한 유일한 경로다. HTTPS 만 쓰고 멀티파트라 재개되며 pod 없이도 접근된다. 절차는 `PRACTICE.md` 4-4.

제약 하나: `aws s3 cp` 는 **HeadObject 403** 으로 실패하고 저수준 `aws s3api get-object` 를 써야 한다.

### 4.5 wandb 기록 파싱

loss 는 `train.log` 에 없다. `finetune.py:305` 가 `wandb.log` 로만 보내기 때문이다. offline 기록은 protobuf 스트림이므로 다음으로 읽는다.

```python
from wandb.sdk.internal.datastore import DataStore
from wandb.proto import wandb_internal_pb2 as pb

ds = DataStore()
ds.open_for_scan("run-<id>.wandb")
# rec.history.item 의 key 는 비어 있고 nested_key 에 들어 있다
key = item.key or ".".join(item.nested_key)
```

`item.key` 만 보면 **키가 전부 빈 문자열로 나와 값이 뒤섞인다.** `nested_key` 를 함께 봐야 한다.

## 5. Section 0 판정

| 항목 | 판정 | 근거 |
|---|---|---|
| OpenVLA 7B LoRA 1사이클이 RTX 4090 에서 가능한가 | **통과** | 2,000 스텝(8.5 epoch) 완주, VRAM 18.5GB/24.5GB, 94분, $1.18 |

`probe_measure.md` §3 의 추정 판정이 완주로 확정됐다. 롤백 옵션 B(경량 adaptation 축소)는 발동하지 않는다.

## 6. 이 기록이 보장하지 않는 것

- **held-out 평가가 없다.** 학습 스크립트는 `train[:95%]` 만 쓰고 나머지 5% 로 검증하지 않는다. 표의 정확도 0.87 은 **학습 데이터에 대한 값**이며, 3,760 프레임을 8.5 epoch 돌았으므로 암기의 가능성을 배제할 수 없다
- **loss 하락은 정답의 정합성을 보장하지 않는다** (README §8). 라벨이 일관되게 틀려도 같은 곡선이 나온다
- **정책이 실제로 동작하는지는 모른다.** 성공률 측정은 week4 이후다. "돌아간다" 와 "맞다" 는 다르다
- **재머지를 검증하지 않았다.** 리비전과 버전 조합은 맞춰져 있으나(§4.1, §4.3) 실제로 로드해 본 것은 아니다
- **머지 가중치를 한 번도 로드해 보지 않았다.** volume 에 15GB 가 저장된 것은 확인했다
- **batch 2 조합은 시도하지 않았다.** `probe_measure.md` §6 의 최적화 여지는 미확인으로 남는다
