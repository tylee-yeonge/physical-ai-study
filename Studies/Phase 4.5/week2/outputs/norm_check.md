# Norm Check — 정규화 계약 확인

> 목적: 학습 전에 (1) 통계 파일이 어디에 어떤 기준으로 만들어지는가, (2) 정규화 마스크가 gripper 를 실제로 빼는가, (3) 캐시가 낡지 않았는가, (4) week1 gripper 부호 규약이 그대로 학습되는가를 확정한다.
> 작성일: 2026-08-10
> 확인 대상: `~/openvla/prismatic/vla/datasets/` (코드), `~/tensorflow_datasets/maniskill_pickcube/1.0.0/` (빌드된 데이터), `week1/outputs/dataset/ep*.npz` (라벨 원본)
> 상태: 계약과 판정 기준은 코드 사실로 확정. 통계 파일 실측값은 §5 에 기록. 코드 읽기로 세운 예측이 실측과 전부 일치했다.
> 검증 스크립트: `practice_norm_check.py` (통계 파일), `practice_load_check.py` (배치)

## 확정 요약

| 항목 | 확정 내용 | 출처 (파일:줄) |
|---|---|---|
| 통계 파일 경로 | `~/tensorflow_datasets/maniskill_pickcube/1.0.0/dataset_statistics_<sha256>.json`. 쓰기 실패 시 `~/.cache/orca/` 로 폴백 | `rlds/dataset.py:220`, `rlds/utils/data_utils.py:200-204` |
| 통계량 | action / proprio 각각 `mean, std, max, min, q01, q99` + `num_transitions`, `num_trajectories` | `data_utils.py:241-260` |
| 정규화에 실제로 쓰이는 통계량 | **q01 / q99** — OpenVLA 파인튜닝 경로는 `BOUNDS_Q99` 로 고정 | `datasets.py:99`, `data_utils.py:80-83` |
| 정규화 마스크 | `action_normalization_mask = [True]*6 + [False]` (EEF_POS 에서 자동 파생) | `oxe/materialize.py:35-39` |
| 마스크가 gripper 를 빼는가 | 뺀다. `tf.where(mask, 정규화식, x)` 이므로 마지막 차원은 원값 통과 | `data_utils.py:84-92` |
| 마스크가 통계 json 에 있는가 | **없다.** 캐시 파일을 쓴 뒤 메모리 dict 에 주입된다 | `dataset.py:227-231` |
| 캐시 키 | `sha256(str(builder.info) + str(state_obs_keys) + 변환함수 소스)` | `dataset.py:213-221` |
| week1 gripper 규약 | `0 = 닫힘, 1 = 열림`. 로더 규약(`0 = close, 1 = open`)과 일치 | `week1/action_transform.py:72`, `experiments/robot/robot_utils.py:99` |
| 실측 판정 | 위 예측이 전부 일치. 마스크 `[T]*6+[F]`, gripper 두 갈래, 이중 정규화 없음, 캐시 신선 | §5 |

## 1. 통계 파일 경로와 계산 기준

### 1.1 경로

통계는 로더가 처음 데이터셋을 열 때 계산되고 파일로 캐시된다. 저장 위치는 `save_dir=builder.data_dir` 로 넘어가고(`dataset.py:220`), `builder.data_dir` 은 이름과 버전까지 포함한 경로다.

```
/root/tensorflow_datasets/maniskill_pickcube/1.0.0/dataset_statistics_<sha256>.json
```

이 경로에 쓰지 못하면(`PermissionDeniedError`) `~/.cache/orca/dataset_statistics_<sha256>.json` 로 폴백한다(`data_utils.py:262-271`). 읽을 때도 두 경로를 순서대로 본다(`data_utils.py:206-217`) — 즉 **폴백 경로에 낡은 파일이 남아 있으면 데이터 디렉터리를 아무리 지워도 그것이 쓰인다.** 캐시를 지울 때 두 곳을 모두 지워야 하는 이유다.

실제로 만들어진 파일은 데이터 디렉터리 쪽 하나다. 폴백 경로 `~/.cache/orca/` 는 생성되지 않았다 — 기본 경로 쓰기가 성공했다는 뜻이다 (§5.1).

### 1.2 계산 기준 — 무엇을 재는가

`get_dataset_statistics` 는 데이터셋 전체를 한 번 훑어 action 과 proprio 를 이어 붙인 뒤 차원별로 다음을 남긴다(`data_utils.py:241-260`).

| 통계량 | 값 | 정규화에 쓰이나 |
|---|---|---|
| `mean`, `std` | 차원별 평균·표준편차 | `NormalizationType.NORMAL` 일 때만 |
| `q01`, `q99` | 차원별 1% / 99% 분위수 | **쓰인다** — 파인튜닝 경로가 `BOUNDS_Q99` |
| `min`, `max` | 차원별 최소·최대 | 정규화식에는 안 쓰인다. `min == max` 인 차원을 0 으로 덮는 판정에만 쓰인다 (§2.3) |
| `num_transitions` | 전체 step 수 | 아니오 (검증용) |
| `num_trajectories` | episode 수 | 아니오 (검증용) |

`BOUNDS_Q99` 의 변환식은 다음과 같다(`data_utils.py:88-91`).

```
clip( 2 * (x - q01) / (q99 - q01 + 1e-8) - 1,  -1,  +1 )
```

분위수를 쓰므로 튀는 값 하나가 범위를 망가뜨리지 않고, 대신 상하위 1% 는 `-1` / `+1` 로 잘린다.

두 가지를 덧붙인다.

- 통계는 **표준화 변환을 거친 뒤의 action 으로 계산된다.** `restructure` 를 적용한 데이터셋을 훑는다(`dataset.py:210-212`). 즉 통계가 보는 것은 내가 RLDS 에 저장한 원시 필드가 아니라 `transforms.py` 의 변환 함수가 내놓은 7차원 벡터다.
- proprio 는 `load_proprio=False` 라 관측에 없고, 없으면 `zeros_like(action)` 으로 대체되어 통계가 잡힌다(`data_utils.py:219-226`). proprio 통계가 전부 0 으로 나오는 것은 오류가 아니다. **길이도 action 을 따라 7 이 된다** — `configs.py` 에 적은 `state_obs_keys` 길이 3 과 무관하다 (§5.3).

기대 규모는 week1 라벨 기준으로 `num_trajectories = 100`, `num_transitions = 3760` 이다 (`week1/outputs/dataset/ep*.npz` 의 `openvla_actions` 합계).

## 2. 마스크 값과 gripper 제외 여부

### 2.1 값

마스크는 설정에 손으로 적지 않는다. `configs.py` 에 선언한 `action_encoding` 이 `EEF_POS` 면 아래 두 줄이 자동으로 붙는다(`materialize.py:35-39`).

```python
# [Contract] For EEF_POS & EEF_R6 actions, only the last action dimension (gripper) is absolute!
dataset_kwargs["absolute_action_mask"] = [False] * 6 + [True]
dataset_kwargs["action_normalization_mask"] = [True] * 6 + [False]
```

| 마스크 | 값 | 의미 |
|---|---|---|
| `action_normalization_mask` | `[T, T, T, T, T, T, F]` | 앞 6차원만 정규화 대상 |
| `absolute_action_mask` | `[F, F, F, F, F, F, T]` | 마지막 차원만 절대값(델타가 아님) |

### 2.2 제외가 실제로 일어나는 지점

정규화는 마스크를 조건으로 하는 `tf.where` 다(`data_utils.py:84-92`).

```python
map_fn=lambda x: tf.where(
    mask,
    tf.clip_by_value(2 * (x - low) / (high - low + 1e-8) - 1, -1, 1),
    x,          # <- mask 가 False 인 차원은 원값 그대로
)
```

마지막 차원은 `mask=False` 이므로 `x` 가 그대로 통과한다. gripper 제외는 확인된 사실이다.

한 가지 함정: **통계 json 파일에는 `mask` 키가 없다.** 마스크는 `get_dataset_statistics` 가 파일을 쓴 뒤에 메모리 dict 로 주입되기 때문이다(`dataset.py:227-231`). 따라서 캐시 json 을 열어 `stats["action"]["mask"]` 를 찾으면 `None` 이 나오는 것이 정상이고, 그것으로 마스크를 판정할 수 없다. 마스크는 다음 두 곳에서 확인한다.

| 확인처 | 무엇을 보나 |
|---|---|
| `configs.py` 의 내 데이터셋 항목 | `action_encoding` 이 `ActionEncoding.EEF_POS` 인가 (여기서 마스크가 파생된다) |
| **배치의 `absolute_action_mask`** | 로더가 배치에 실어 보내는 `(7,)` bool 배열. 실측으로 확인할 수 있는 유일한 지점 (§5.4) |
| 학습 `run_dir/dataset_statistics.json` | `save_dataset_statistics` 가 저장하는 사본. 이쪽에는 주입된 `mask` 가 들어 있다 (`data_utils.py:274-293`) |

### 2.3 마스크보다 우선하는 예외

`min == max` 인 차원은 마스크와 무관하게 0 으로 덮인다(`data_utils.py:96-99`).

```python
zeros_mask = metadata[key]["min"] == metadata[key]["max"]
... map_fn=lambda x: tf.where(zeros_mask, 0.0, x)
```

즉 gripper 가 데이터 전 구간에서 상수면 absolute 계약에도 불구하고 0 이 된다. 우리 데이터는 0 과 1 이 섞여 있어 해당하지 않는다 (§4.2 라벨 실측, §5.5 통계 실측).

## 3. 캐시 신선도 판정

### 3.1 캐시 키에 무엇이 들어가는가

파일명의 해시는 세 가지로 만든다(`dataset.py:213-221`).

| 해시 입력 | 내용 |
|---|---|
| `str(builder.info)` | 데이터셋 이름, 버전, split 별 example 수, 데이터 크기 등 `dataset_info.json` 이 담는 메타데이터 |
| `str(state_obs_keys)` | 설정에 적은 state 키 목록 |
| 변환 함수 소스 | `transforms.py` 에 등록한 내 함수의 소스 코드 문자열 |

여기 **들어가지 않는 것**이 판정의 핵심이다.

- tfrecord 의 내용 자체 (값이 바뀌어도 해시는 모른다)
- 파일 수정 시각

따라서 **episode 수와 데이터 크기가 그대로인 채 라벨 값만 고쳐 재생성하면 같은 해시가 나올 수 있고, 낡은 통계가 조용히 재사용된다.** 라벨을 고친 직후가 가장 위험한 순간이라는 README §6 의 함정이 여기서 나온다.

### 3.2 판정 절차

| 순서 | 확인 | 낡았다고 보는 조건 |
|---|---|---|
| 1 | 통계 json 과 tfrecord 의 `mtime` 비교 | tfrecord 가 통계보다 새로우면 낡음 |
| 2 | 폴백 경로 확인 | `~/.cache/orca/` 에 파일이 남아 있으면 그것이 먼저 읽힐 수 있다 |
| 3 | 변환 함수 / `state_obs_keys` 를 고쳤는가 | 고쳤으면 해시가 갈리므로 자동 재계산 (이 경우는 안전) |

데이터를 재생성할 때는 빌더의 `VERSION` 을 올린다. 버전이 `builder.info` 에 들어가므로 해시가 확실히 갈리고, 데이터도 새 디렉터리에 굽히므로 옛 통계와 섞이지 않는다.

낡았다고 판정되면 두 경로를 함께 지운다.

```bash
rm -f ~/tensorflow_datasets/maniskill_pickcube/1.0.0/dataset_statistics_*.json
rm -f ~/.cache/orca/dataset_statistics_*.json
```

### 3.3 현재 판정: 신선함

| 대상 | 시각 |
|---|---|
| `maniskill_pickcube-train.tfrecord-0000{0,1}-of-00002` | 2026-08-10 11:15:16 |
| `dataset_info.json` (train split 100 examples, 261,596,792 bytes) | 2026-08-10 11:15:16 |
| `dataset_statistics_76ad416b...cbe.json` | 2026-08-10 16:26:48 |

통계가 데이터보다 약 5시간 11분 나중이다. 이 캐시는 현재 데이터로 계산됐다.

### 3.4 mtime 비교의 한계

`practice_norm_check.py` 는 mtime 을 비교하지만, 그것만으로는 부족하다.

**통계 파일이 여러 개 남을 수 있다.** 해시가 갈리면 새 파일이 생기고 옛 파일은 지워지지 않는다. `glob` 의 반환 순서는 파일시스템 디렉터리 순서라 알파벳순도 시간순도 아니므로, 여러 개가 있으면 `candidates[0]` 이 옛것을 고를 수 있다. 그리고 그 옛 파일의 mtime 역시 데이터보다 나중일 수 있어 **mtime 비교가 낡은 캐시를 통과시킨다.**

따라서 판정 순서는 이렇다.

1. 통계 파일이 **몇 개** 잡히는지 먼저 본다. 2개 이상이면 그 자체가 경고 신호다
2. 해시가 현재 설정과 맞는지 본다 — 이것이 진짜 보증 수단이다 (§3.1)
3. mtime 비교는 보조 지표다

현재는 파일이 하나뿐이므로 이 함정에 걸리지 않았다. 라벨을 고쳐 재빌드하는 순간부터 유효해진다.

## 4. gripper 부호 규약 — 정규화가 보정하지 않는다

gripper 는 마스크에서 제외되므로 **week1 에서 저장한 값이 손대지 않은 채 그대로 학습된다.** 부호가 뒤집혀 있어도 정규화가 고쳐 주지 않고, 학습은 정상으로 돌고 loss 도 내려간다. 증상은 week5 eval 에서야 보인다. 그래서 여기가 부호를 값싸게 점검할 수 있는 마지막 지점이다.

### 4.1 양쪽 규약 대조

| 쪽 | 규약 | 근거 |
|---|---|---|
| OpenVLA RLDS 로더 | `0 = close, 1 = open` | `experiments/robot/robot_utils.py:99` 주석, `data_utils.py:143` (`rel2abs_gripper_actions` docstring) |
| ManiSkill 원본 명령 | `-1 = 닫힘, +1 = 열림` | `pd_ee_delta_pose` 제어 모드 |
| week1 라벨 | `0 = 닫힘, 1 = 열림` | `week1/action_transform.py:72` — `gripper = (commands[:, 6:7] + 1.0) / 2.0` |

### 4.2 실측 (week1/outputs/dataset/ep*.npz, `openvla_actions[:, 6]`)

| 항목 | 값 |
|---|---|
| episode 수 / transition 수 | 100 / 3760 |
| unique 값 | `{0.0, 1.0}` |
| min / max | 0.0 / 1.0 |
| 0 의 비율 / 1 의 비율 | 0.568 / 0.432 |

### 4.3 판정

- 부호 일치: week1 의 `0 = 닫힘` 이 로더의 `0 = close` 와 같다. 뒤집을 필요 없음
- 값이 두 갈래로만 존재하므로 로더의 gripper 이진화 가정과도 어긋나지 않는다
- `min != max` 이므로 §2.3 의 `zeros_mask` 조항에 걸리지 않는다
- 남은 조건: 표준화 변환 함수가 이 차원을 건드리지 않고 그대로 7번째에 두는 것. `transforms.py` 에 등록할 함수에서 gripper 를 반전하거나 재이진화하지 않는다

## 5. 실측

### 5.1 통계 파일 실경로

```
/root/tensorflow_datasets/maniskill_pickcube/1.0.0/
  dataset_statistics_76ad416b202a90c7673380085dd6dae3d67656594376b317d5bf12cee4fb1cbe.json
```

| 항목 | 값 |
|---|---|
| 파일 개수 (데이터 디렉터리) | 1 |
| 폴백 경로 `~/.cache/orca/` | 디렉터리 자체가 없음 |
| 최상위 키 | `action`, `proprio`, `num_transitions`, `num_trajectories` |
| `num_transitions` / `num_trajectories` | 3760 / 100 |

규모가 week1 라벨 기준 기대값(§1.2)과 정확히 일치한다. 평균 37.6 스텝/episode.

### 5.2 action 통계 실측

| 통계량 | dim0 (dX) | dim1 (dY) | dim2 (dZ) | dim3 (dRoll) | dim4 (dPitch) | dim5 (dYaw) | dim6 (gripper) |
|---|---|---|---|---|---|---|---|
| mean | -0.0000779 | -0.0000036 | -0.000391 | 0.0000365 | 0.000192 | -0.0000058 | 0.431915 |
| std | 0.005424 | 0.005435 | 0.008947 | 0.000445 | 0.002966 | 0.002342 | 0.495332 |
| min | -0.014826 | -0.012453 | -0.012251 | -0.003326 | -0.010437 | -0.011084 | 0.0 |
| max | 0.015074 | 0.012489 | 0.012415 | 0.002951 | 0.011650 | 0.011055 | 1.0 |
| q01 | -0.012085 | -0.012027 | -0.012173 | -0.001523 | -0.009729 | -0.009596 | 0.0 |
| q99 | 0.013165 | 0.012103 | 0.012311 | 0.001638 | 0.010497 | 0.009930 | 1.0 |

**자릿수 판정: 정상.** 위치 3축은 ±0.015 (m 단위, 스텝당 최대 1.5cm), 회전 3축은 ±0.011 (rad 단위, 최대 0.63도). week1 이 라벨을 물리 단위로 저장했고 **미리 정규화하지 않았다**는 확인이다. 미리 정규화됐다면 이 값들이 ±1 근처였을 것이고, 그 위에 로더가 한 번 더 정규화해 이중 정규화가 됐을 것이다.

**dim3 의 범위가 유독 좁다.** `q99 - q01` 을 비교하면 드러난다.

| 차원 | q99 - q01 |
|---|---|
| dim0 / dim1 / dim2 | 0.02525 / 0.02413 / 0.02448 |
| dim3 | **0.00316** |
| dim4 / dim5 | 0.02023 / 0.01953 |

dim3 이 다른 회전 축의 약 1/6 이다. `BOUNDS_Q99` 는 차원별로 따로 늘리므로, dim3 의 0.18도 폭이 dim4/dim5 의 1.2도 폭과 **동일한 [-1, 1] 신호로 증폭된다.** 상대 분산(std / 반폭)은 dim3 0.281, dim4 0.293, dim5 0.240 으로 비슷하니 분포 모양 자체는 이상하지 않다. 남은 확인은 **PickCube 궤적에서 roll 이 pitch/yaw 보다 6배 덜 움직이는 것이 물리적으로 맞는가** 이며, `week1/action_transform.py` 의 회전 delta 계산과 대조해야 한다.

### 5.3 proprio 통계 실측

`mean`, `std`, `min`, `max`, `q01`, `q99` 가 모두 길이 7 의 0 벡터다.

§1.2 의 예측대로 `load_proprio=False` 라 `zeros_like(action)` 이 대체된 결과이므로 오류가 아니다. 여기서 확인된 추가 사실은 **길이가 7 이라는 것** — `configs.py` 에 적은 `state_obs_keys` 길이 3 과 무관하다. 그 값은 `materialize.py:62-63` 에서 제거되므로 이 경로에 관여하지 않는다. proprio 를 켜는 시점에는 그 길이가 유효해지므로 그때 다시 봐야 한다.

### 5.4 마스크 실측

통계 json 파일: `stats["action"]["mask"]` 는 **`None`** — §2.2 의 예측대로 파일에는 없다.

로더가 실제로 들고 있는 값은 두 곳에서 확인된다.

| 확인처 | 값 |
|---|---|
| 배치의 `absolute_action_mask` | `[False, False, False, False, False, False, True]` |
| `dataset.dataset_statistics["maniskill_pickcube"]["action"]["mask"]` | `[True, True, True, True, True, True, False]` |

`materialize.py:37-38` 이 만든 값과 정확히 같다. 메모리 dict 의 action 키가 `mask, max, mean, min, q01, q99, std` 7개인 것도 확인됐다 — 파일의 6개에 `mask` 가 주입된 형태다.

**마스크 계약 확정: gripper 는 정규화 대상에서 제외되고 절대값으로 취급된다.** 실습 3 에서 `action_encoding` 을 `EEF_POS` 로 적은 한 줄이 이 결과를 만들었다.

### 5.5 gripper 가 두 갈래임을 통계로 증명

두 값만 갖는 분포의 표준편차는 `sqrt(p(1-p))` 다.

| 항목 | 값 |
|---|---|
| mean (= 1 의 비율 p) | 0.431915 |
| `sqrt(p(1-p))` 계산값 | 0.495418 |
| 실측 std | 0.495332 |

소수 넷째 자리까지 일치한다. 중간값이 섞여 있으면 이 등식이 깨지므로, **dim6 이 0 과 1 두 값만 갖는다**는 증명이다. week1 실측(§4.2)의 `1 의 비율 0.432` 와도 일치한다 — 라벨이 빌드와 로더를 통과하며 변형되지 않았다는 뜻이다.

`min != max` 이므로 §2.3 의 `zeros_mask` 조항에도 걸리지 않는다.

### 5.6 아직 확인되지 않은 것

| 항목 | 왜 미확인인가 |
|---|---|
| 정규화 **후** action 이 실제로 `[-1, 1]` 대역인가 | `practice_load_check.py` 가 `next(iter(dataset))` 로 샘플 1개만 꺼낸다. `RLDSDataset` 은 transition 하나씩 내놓으므로 차원별 min 과 max 가 같은 값이 되어 범위를 판정할 수 없다. 여러 샘플을 누적해야 한다 |
| 정규화 후에도 gripper 가 0/1 로 남는가 | 같은 이유. 샘플 1개의 `np.unique` 는 값 1개다 |
| episode 경계 (실습 4-3 (f)) | 검사 코드가 없다. 재료는 배치의 `observation.timestep`, `observation.pad_mask` |

앞의 둘은 §5.2 의 통계 실측과 §5.4 의 마스크 실측으로 **계약이 확정된 상태**이므로, 남은 것은 그 계약이 실제 출력에서도 지켜지는지 보는 확인 사격이다. 다만 확인 없이 통과로 기록하지 않는다.
