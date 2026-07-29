# Week 5 실습: 하네스 통합 -> N 결정 -> 방법 확정 -> 두 측정


> **실습 목표**: 하나의 스크립트로 두 모델을 각각 N회 돌려 seed 단위 원시 결과를 남긴다.
> **예상 시간**: 6-8시간 (대부분 실행 대기)
> **원칙**: 실습 3 (통계 방법 확정) 을 실습 4-5 (실행) 보다 먼저 한다. 결과를 본 뒤에 방법을 고르면 방법이 결과에 맞춰진다.


---


## 환경 설정


week4 와 같은 환경이다. **버전을 바꾸지 않는다.**


```bash
source "/workspace/study/physical-ai-study/Studies/Phase 4/.venv-vla/bin/activate"
cd "/workspace/study/physical-ai-study/Studies/Phase 4.5/week5"
mkdir -p outputs
```


sim 패키지가 이 venv 에 없으면 week0 실습 1-6 의 판단(합칠지 두 프로세스로 나눌지)을 그대로 따른다 — `week0/outputs/env_decision.md` 에 기록해 둔 결정이다.


---


## 실습 1: 하네스 통합


**파일명**: `eval_harness.py`


**스크립트를 복사해 두 개 만들지 않는다.** 모델을 인자로 받아 하나로 돌린다 — 그것이 변인 통제의 유일한 보장이다.


```python
"""
실습 1: zero-shot / fine-tuned 를 같은 조건으로 N회 평가하는 단일 하네스

실행:
  python eval_harness.py --model openvla/openvla-7b --unnorm-key bridge_orig \
      --out outputs/eval_zeroshot.jsonl
  python eval_harness.py --model /root/models/openvla-maniskill-ft --unnorm-key maniskill_pickcube \
      --out outputs/eval_finetuned.jsonl
"""
import argparse                                    # 모델만 인자로 받기 위해
import json
import subprocess                                  # 스크립트 커밋 해시 기록용
import numpy as np
import torch
import gymnasium as gym
import mani_skill.envs
from PIL import Image
from transformers import AutoModelForVision2Seq, AutoProcessor, BitsAndBytesConfig


# ===== 고정 상수: 두 측정에서 절대 바뀌지 않는다 (week0/week4 확정값) =====
ENV_ID = "PickCube-v1"                             # week0 sim_facts.md
STEP_CAP = 100                                     # week0 sim_facts.md
INSTRUCTION = "pick up the cube"                   # week0-1 동일 문구
CAMERA_KEY = ("sensor_data", "base_camera", "rgb")  # week0 확정 경로
IMAGE_SIZE = 224                                   # OpenVLA 입력 크기
N_EPISODES = 100                                   # 실습 2 에서 확정한 값


# ===== 인자: 이 두 개만 바뀐다 =====
parser = argparse.ArgumentParser()
parser.add_argument("--model", required=True)      # 모델 경로 또는 허브 이름
parser.add_argument("--unnorm-key", required=True)  # 역정규화에 쓸 통계 키
parser.add_argument("--out", required=True)        # 결과 jsonl 경로
args = parser.parse_args()


# ===== 모델 적재: week4 실습 2 와 동일 설정 =====
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,                             # 두 모델 모두 4-bit (변인 통제)
    bnb_4bit_quant_type="nf4",
    bnb_4bit_use_double_quant=True,
    bnb_4bit_compute_dtype=torch.float16,
)
processor = AutoProcessor.from_pretrained(args.model, trust_remote_code=True)
vla = AutoModelForVision2Seq.from_pretrained(
    args.model,
    attn_implementation="eager",
    torch_dtype=torch.float16,
    low_cpu_mem_usage=True,
    trust_remote_code=True,
    quantization_config=bnb_config,
)
prompt = f"In: What action should the robot take to {INSTRUCTION}?\nOut:"


# ===== eval seed 목록: week0 산출물에서 읽는다 (손으로 옮겨 적지 않는다) =====
with open("../week0/outputs/zeroshot_baseline.json") as f:
    base_seeds = json.load(f)["seeds"]             # week0 이 쓴 목록
# N 을 늘렸으므로 목록을 확장한다. 규칙을 코드로 고정해 두 측정이 같은 목록을 쓰게 한다.
eval_seeds = list(range(N_EPISODES))               # <- 실습 2 에서 정한 확장 규칙으로 교체
assert set(base_seeds) <= set(eval_seeds), "week0 목록이 새 목록에 포함되어야 한다"
# 학습 seed 와의 겹침 검사 (week1 collect_meta.json 의 train_seeds)
with open("../week1/outputs/dataset/collect_meta.json") as f:
    train_seeds = set(json.load(f)["train_seeds"])
assert not (set(eval_seeds) & train_seeds), "eval seed 에 학습 seed 가 섞였다"


# ===== 실행 메타 (결과 파일에 조건을 박아 둔다) =====
commit = subprocess.run(["git", "rev-parse", "--short", "HEAD"],
                        capture_output=True, text=True).stdout.strip()
meta = {"model": args.model, "unnorm_key": args.unnorm_key, "env_id": ENV_ID,
        "step_cap": STEP_CAP, "instruction": INSTRUCTION, "n_episodes": N_EPISODES,
        "quant": "nf4+dq+fp16", "commit": commit}


env = gym.make(ENV_ID, obs_mode="rgb", control_mode="pd_ee_delta_pose")


# ===== episode 루프: 결과를 seed 단위로 즉시 append (중단 대비 + 짝지은 비교) =====
with open(args.out, "w") as out_file:
    out_file.write(json.dumps({"_meta": meta}) + "\n")   # 첫 줄에 조건 기록
    for seed in eval_seeds:
        obs, info = env.reset(seed=seed)
        stages = {"reached": False, "grasped": False, "lifted": False, "placed": False}
        reason = "step_cap"                        # 종료 사유 기본값
        for step in range(STEP_CAP):
            frame = np.asarray(obs[CAMERA_KEY[0]][CAMERA_KEY[1]][CAMERA_KEY[2]])
            if frame.ndim == 4:                    # 배치 차원 제거
                frame = frame[0]
            image = Image.fromarray(frame.astype(np.uint8)).resize((IMAGE_SIZE, IMAGE_SIZE))
            model_inputs = processor(prompt, image).to("cuda:0", dtype=torch.float16)
            with torch.no_grad():
                raw_action = vla.predict_action(
                    input_ids=model_inputs["input_ids"],
                    pixel_values=model_inputs["pixel_values"],
                    unnorm_key=args.unnorm_key,    # 인자로 받은 키
                    do_sample=False,               # 결정적 출력
                )
            action = raw_action                    # <- week0 정변환 적용으로 교체
            obs, reward, terminated, truncated, info = env.step(action)

            # 부분 도달률: week0 실습 6 에서 정한 판정식과 임계값을 그대로 쓴다
            # stages["reached"] |= <week0 판정식>
            # stages["grasped"] |= <week0 판정식>
            # stages["lifted"]  |= <week0 판정식>
            stages["placed"] |= bool(info.get("success", False))

            if stages["placed"]:
                reason = "success"
                break
            if terminated or truncated:
                reason = "env_end"
                break

        record = {"seed": seed, "steps": step + 1, "reason": reason, **stages}
        out_file.write(json.dumps(record) + "\n")  # 즉시 기록 (버퍼에 쌓아 두지 않는다)
        out_file.flush()
        print(f"   seed{seed:03d}: {record}")


env.close()
print(f"\n완료: {args.out}")
```


**확인 포인트**

- 상수 블록의 값이 week0/week4 기록과 하나도 다르지 않은가 (표로 대조한다)
- `_meta` 첫 줄에 조건이 다 들어갔는가
- 결과 파일이 실행 중에도 계속 자라는가 (즉시 기록 확인)


---


## 실습 2: N 결정 + 비용 산정


**산출물**: `outputs/eval_plan.md`


실측치로 계산해 N 을 정한다. 감으로 정하지 않는다.


```python
"""
실습 2: 실행 시간을 계산해 N 을 결정 (계산만 -- 실행 없음)
"""
LATENCY_MS = 300          # Measurements/openvla-rtx4070-int4 Block 1 실측 (본인 수치로 교체)
STEP_CAP = 100            # week0 확정값
OVERHEAD_MS = 20          # sim step + 이미지 변환 몫 (probe 로 확인해 교체)


per_step_s = (LATENCY_MS + OVERHEAD_MS) / 1000       # 스텝 1회 소요
per_episode_s = per_step_s * STEP_CAP                # 최악(조기 종료 없음) 기준
print(f"스텝당 {per_step_s:.3f}s / episode 최대 {per_episode_s:.1f}s")


for n in [20, 50, 100, 200]:                         # 후보 N 별 시간
    one_model_min = per_episode_s * n / 60
    print(f"  N={n:3d}: 모델당 약 {one_model_min:.0f}분, 두 모델 약 {one_model_min * 2:.0f}분")
```


**결정에 쓸 근거**

| 항목 | 값 |
|---|---|
| 스텝당 실측 시간 | |
| episode 최대 시간 | |
| 후보 N 별 총 실행 시간 | |
| 확정 N | |
| 확정 근거 | (구간 폭 vs 실행 시간의 절충) |
| **zero-shot 재측정 필요 여부** | N 을 바꿨으면 필요 |
| seed 목록 확장 규칙 | week0 목록을 포함하도록 |


> N 을 올리면 week0 의 20회 결과를 그대로 쓸 수 없다. 같은 N·같은 목록이어야 짝지은 비교가 성립한다. 확장 규칙은 **week0 목록을 부분집합으로 포함**하도록 정하면 옛 결과와의 대조도 가능해진다.


---


## 실습 3: 통계 방법 확정 (실행 전에 작성)


**산출물**: `outputs/stat_method.md`


**실습 4-5 를 시작하기 전에 이 문서를 완성한다.** 결과를 본 뒤에 방법을 고르면 방법이 결과에 맞춰진다.


### 3-1. 구간 추정 방법 고르기


경계(0/N, N/N)에서 성립하는 방법이 필요하다. 후보를 조사해 하나를 고르고 근거를 적는다.


```python
"""
실습 3-1: 정규 근사가 경계에서 무너지는 것을 직접 확인
"""
import math


N = 100
for success in [0, 1, 5, 25, 50]:                    # 관측값을 바꿔 본다
    p = success / N
    wald_half = 1.96 * math.sqrt(p * (1 - p) / N)     # 정규 근사 반폭
    print(f"  {success:3d}/{N}: p={p:.2f}, 정규 근사 반폭={wald_half:.4f}")
# 0/100 에서 반폭이 0 이 되는 것을 확인한다 -- 이 값을 보고할 수 없다.
```


고를 것: 경계에서 폭이 0 이 되지 않는 이항 비율 구간 추정 방법. 근사를 보정하는 계열과 근사를 쓰지 않는 계열이 있다. **어느 것을 왜 골랐는지**를 적고, 계산에 쓸 함수(직접 구현 또는 라이브러리)를 확정한다.


### 3-2. 짝지은 비교 방법 고르기


같은 seed 목록을 쓰므로 관측이 짝지어져 있다 (README §4). 판단의 기반은 **불일치 쌍**이다.


```
        fine-tuned 성공   fine-tuned 실패
zero  성공      a                b
zero  실패      c                d
```


- `a`, `d` 는 두 모델이 같은 결과를 낸 seed — 차이에 대한 정보가 없다
- `b`, `c` 가 불일치 쌍 — **`c` 가 `b` 보다 뚜렷이 많으면 개선의 근거**가 된다


적을 것: 이 2x2 표를 어떤 방법으로 판정할지, 표본이 작을 때(불일치 쌍이 몇 개뿐일 때)의 대응, 그리고 유의 수준.


### 3-3. 보고 문장의 형태를 미리 정하기


결과를 어떤 문장으로 쓸지 틀을 먼저 만든다. 숫자만 나중에 채운다.


```
zero-shot   {성공}/{N} (95% 구간 {하한}-{상한}%)
fine-tuned  {성공}/{N} (95% 구간 {하한}-{상한}%)
불일치 쌍   개선 {c}건 / 악화 {b}건 -> 판정 {...}
단계별      reached {..} -> {..} / grasped {..} -> {..} / lifted {..} -> {..}
결론        {구간 겹침 여부와 짝지은 판정에 근거한 한 문장}
```


> 이 틀을 미리 만드는 것이 §7 의 "방법을 결과보다 먼저" 를 실행하는 방법이다.


---


## 실습 4: zero-shot N회


**실행**


```bash
cd "/workspace/study/physical-ai-study/Studies/Phase 4.5/week5"
python eval_harness.py \
  --model openvla/openvla-7b \
  --unnorm-key bridge_orig \
  --out outputs/eval_zeroshot.jsonl \
  2>&1 | tee outputs/eval_zeroshot.log
```


**확인 포인트**

- `_meta` 의 `unnorm_key` 가 `bridge_orig` 인가 (week0 과 같은 조건)
- 결과 줄 수가 N+1 (메타 1줄 + episode N줄) 인가
- week0 의 20개 seed 에 해당하는 줄만 뽑아 week0 결과와 비교해 본다 — 크게 다르면 조건이 어긋난 것이다


> 마지막 확인이 유용하다. **week0 재현 검사**를 겸한다 — 같은 seed·같은 조건이면 결과도 같아야 한다 (결정적 추론이므로).


---


## 실습 5: fine-tuned N회


**실행** — 바뀌는 것은 두 인자뿐이다.


```bash
python eval_harness.py \
  --model /root/models/openvla-maniskill-ft \
  --unnorm-key maniskill_pickcube \
  --out outputs/eval_finetuned.jsonl \
  2>&1 | tee outputs/eval_finetuned.log
```


**실행 후 무결성 검사** (집계는 하지 않는다)


```python
"""
실습 5: 두 결과 파일의 조건 일치와 짝지음 성립만 확인 (해석 없음)
"""
import json


def load(path):
    """jsonl 을 읽어 메타와 레코드로 나눈다."""
    with open(path) as f:
        lines = [json.loads(line) for line in f]
    return lines[0]["_meta"], lines[1:]


zero_meta, zero_records = load("outputs/eval_zeroshot.jsonl")
ft_meta, ft_records = load("outputs/eval_finetuned.jsonl")


# 5-1. 조건 일치 검사: model/unnorm_key 만 달라야 한다
print("\n[5-1] 메타 차이")
for key in set(zero_meta) | set(ft_meta):
    if zero_meta.get(key) != ft_meta.get(key):
        print(f"   {key}: {zero_meta.get(key)} -> {ft_meta.get(key)}")
# 위 출력에 model, unnorm_key 외의 항목이 있으면 변인이 샜다


# 5-2. 짝지음 성립 검사: seed 목록이 정확히 같아야 한다
zero_seeds = [r["seed"] for r in zero_records]
ft_seeds = [r["seed"] for r in ft_records]
print(f"\n[5-2] seed 수: {len(zero_seeds)} / {len(ft_seeds)}")
print("   목록 동일:", zero_seeds == ft_seeds)


# 5-3. 결측 검사
print(f"\n[5-3] 종료 사유 분포")
for records, name in [(zero_records, "zero"), (ft_records, "ft")]:
    reasons = {}
    for record in records:
        reasons[record["reason"]] = reasons.get(record["reason"], 0) + 1
    print(f"   {name}: {reasons}")
# 여기까지가 이번 주다. 성공률 집계와 구간 계산은 week6.
```


**통과 판정**

| 검사 | 기대 |
|---|---|
| 메타 차이 | `model`, `unnorm_key` 두 항목만 |
| seed 목록 | 두 파일이 정확히 같은 순서·같은 값 |
| 레코드 수 | 양쪽 모두 N |
| week0 재현 (실습 4) | 겹치는 seed 의 결과가 일치 |


---


## 마무리: week6 로 넘기는 것


| 산출물 | week6 에서의 용도 |
|---|---|
| `outputs/eval_zeroshot.jsonl` | before 원시 결과 |
| `outputs/eval_finetuned.jsonl` | after 원시 결과 |
| `outputs/stat_method.md` | 적용할 방법 (미리 확정된 것) |
| `outputs/eval_plan.md` | N 과 그 근거 |


원시 결과와 방법 문서는 `Measurements/` 로 옮긴다.


```bash
mkdir -p "/workspace/study/physical-ai-study/Measurements/openvla-lora-eval/raw"
```


| 옮길 것 | 착지점 |
|---|---|
| 두 `jsonl` + 로그 | `raw/` |
| `stat_method.md`, `eval_plan.md` | `methodology.md` |
| `eval_harness.py` | `scripts/` |


> 집계 수치와 해석은 week6 이 `findings.md` 에 쓴다. 이번 주는 **결과를 보지 않고 넘긴다** — 그것이 방법을 지키는 방식이다.
