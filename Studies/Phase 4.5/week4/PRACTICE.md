# Week 4 실습: 전송 -> 4-bit 적재 -> 통계 전환 -> 검증


> **실습 목표**: fine-tuned 모델을 로컬 4070 에 4-bit 로 올리고, 4층 검증을 통과시켜 week0 하네스에 꽂는다.
> **예상 시간**: 6-8시간
> **원칙**: "로드됐다" 에서 멈추지 않는다. 실습 3 의 값 대역 검사를 통과해야 모델이 쓸 수 있는 상태다.


---


## 환경 설정


추론은 Phase 4 의 공용 venv 를 그대로 쓴다. **버전을 바꾸지 않는다** — 이 조합 위에서 Block 1-3 실측이 재현되고, week0 baseline 도 이 환경에서 측정됐다.


```bash
source "/workspace/study/physical-ai-study/Studies/Phase 4/.venv-vla/bin/activate"
cd "/workspace/study/physical-ai-study/Studies/Phase 4.5/week4"
mkdir -p outputs
pip list | grep -iE "transformers|tokenizers|timm|accelerate|bitsandbytes|torch " > outputs/local_versions.txt
cat outputs/local_versions.txt      # 실습 4 의 대조 기준
```


---


## 실습 1: 회수·전송 + 무결성 검증


**산출물**: `outputs/transfer_check.md`


week3 이 남긴 4항목을 받는다. **통계 파일은 작아서 빠뜨리기 쉽다** — 목록으로 확인한다.


```bash
# 1-1. 원격에서 회수 대상 확인 (pod 또는 volume)
ssh <pod> "ls -la /workspace/volume/runs/*/"       # <- 접속 주소로 교체
#   확인할 4항목:
#     (a) 머지된 가중치 (*.safetensors 등, 15GB 급)
#     (b) LoRA 어댑터 원본 (작다)
#     (c) 데이터셋 통계 파일 (작다 -- 빠뜨리면 실습 3 에서 막힌다)
#     (d) processor / config 파일 + 학습 로그


# 1-2. 재개 가능한 전송 (끊겨도 이어받는다)
mkdir -p ~/models/openvla-maniskill-ft
rsync -avP <pod>:/workspace/volume/runs/<run-name>/ ~/models/openvla-maniskill-ft/


# 1-3. 무결성 검증 -- 파일 수와 총 바이트를 양쪽에서 비교
ssh <pod> "find /workspace/volume/runs/<run-name> -type f | wc -l; du -sb /workspace/volume/runs/<run-name>"
find ~/models/openvla-maniskill-ft -type f | wc -l
du -sb ~/models/openvla-maniskill-ft


# 1-4. 4항목 존재 확인 (눈으로 훑지 말고 목록으로)
ls -la ~/models/openvla-maniskill-ft/
find ~/models/openvla-maniskill-ft -name "*.json" | xargs ls -la    # 통계·config 확인
```


**기록할 것**

| 항목 | 값 |
|---|---|
| 전송 용량 / 소요 시간 | |
| 파일 수 (원격 / 로컬) | |
| 총 바이트 (원격 / 로컬) | |
| 4항목 존재 확인 | |
| 통계 파일 경로 | (실습 3 에서 쓴다) |


---


## 실습 2: 4-bit 적재 + VRAM 실측


**파일명**: `practice_load_4bit.py`


절차는 Phase 4 week6 과 같고 **모델 경로만 다르다.** 적재 메모리를 baseline 과 대조하는 것이 이 실습의 판정이다.


```python
"""
실습 2: fine-tuned 머지 가중치를 4-bit 로 적재하고 VRAM 을 실측
"""
import torch
from transformers import AutoModelForVision2Seq, AutoProcessor, BitsAndBytesConfig


MODEL_PATH = "/root/models/openvla-maniskill-ft"   # <- 실습 1 의 로컬 경로
BASELINE_GB = 4.38                                 # week0/Block 1 실측 (4-bit OpenVLA 7B)


print("=" * 60)
print("실습 2: 4-bit 적재 + VRAM 실측")
print("=" * 60)


# -- 2-1. week0 과 동일한 양자화 설정 (변인 통제 -- README §5) --
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,                             # 4-bit 적재
    bnb_4bit_quant_type="nf4",                     # week0 과 같은 양자화 종류
    bnb_4bit_use_double_quant=True,                # week0 과 같은 double quant 여부
    bnb_4bit_compute_dtype=torch.float16,          # week0 과 같은 연산 dtype
)


# -- 2-2. 적재 전 메모리 기록 (증분을 보기 위해) --
torch.cuda.reset_peak_memory_stats()               # 피크 카운터 초기화
before_gb = torch.cuda.memory_allocated() / 1e9
print(f"\n[2-2] 적재 전: {before_gb:.2f} GB")


# -- 2-3. 로컬 경로에서 적재 --
processor = AutoProcessor.from_pretrained(MODEL_PATH, trust_remote_code=True)
vla = AutoModelForVision2Seq.from_pretrained(
    MODEL_PATH,                                    # 허브 이름이 아니라 로컬 디렉터리
    attn_implementation="eager",                   # week0 과 같은 attention 구현
    torch_dtype=torch.float16,
    low_cpu_mem_usage=True,
    trust_remote_code=True,                        # 자체 모델 코드 실행 허용
    quantization_config=bnb_config,
)
after_gb = torch.cuda.memory_allocated() / 1e9
peak_gb = torch.cuda.max_memory_allocated() / 1e9
print(f"[2-3] 적재 후: {after_gb:.2f} GB (피크 {peak_gb:.2f} GB)")


# -- 2-4. baseline 대조 (양자화가 실제로 걸렸는지 판정 -- README §3) --
diff = after_gb - BASELINE_GB
print(f"\n[2-4] baseline({BASELINE_GB} GB) 대비 차이: {diff:+.2f} GB")
if abs(diff) < 0.5:
    print("   판정: 근사 일치 -- 양자화 적용됨")
else:
    print("   판정: 벗어남 -- 양자화 설정 또는 적재 범위 확인 필요")
```


**확인 포인트**

- `trust_remote_code` 로드가 실패하면 체크포인트에 자체 모델 코드로 가는 연결이 보존됐는지 확인한다 (README §2)
- 차이가 크게 양수면 양자화 설정이 안 걸린 것, 크게 음수면 모델 일부만 로드된 것을 의심한다


---


## 실습 3: `unnorm_key` 전환 + 값 대역 검사


**파일명**: `practice_unnorm_switch.py`


이번 주의 기술 과제다. 남의 통계에서 내 통계로 바꾸고, **바뀌었다는 것을 값으로 확인**한다.


```python
"""
실습 3: 학습 통계를 모델에 연결하고 출력 대역으로 검증
"""
import json
import numpy as np
import torch
from PIL import Image


# 실습 2 의 processor / vla 가 로드된 상태를 전제한다 (같은 세션에서 이어 실행)


STATS_PATH = "/root/models/openvla-maniskill-ft/dataset_statistics.json"   # <- 실습 1 확인 경로
DATASET_KEY = "maniskill_pickcube"                 # week2 에서 등록한 이름
INSTRUCTION = "pick up the cube"                   # week0-1 과 같은 문구


print("=" * 60)
print("실습 3: unnorm_key 전환")
print("=" * 60)


# -- 3-1. 모델이 현재 아는 키 목록 확인 --
print("\n[3-1] 현재 norm_stats 키:", list(vla.norm_stats.keys()))
# 내 데이터셋 이름이 여기 있으면 그대로 쓴다.
# 없으면 3-2 로 주입한다.


# -- 3-2. 통계 주입 (키가 없을 때만) --
with open(STATS_PATH) as f:
    stats = json.load(f)                           # week3 학습이 저장한 통계
print("\n[3-2] 통계 파일 키:", list(stats.keys()))
# 구조가 모델의 norm_stats 형식과 같은지 비교한 뒤 주입한다.
# vla.norm_stats[DATASET_KEY] = stats[DATASET_KEY]   # <- 실제 구조에 맞게 교체
print("주입 후 키:", list(vla.norm_stats.keys()))


# -- 3-3. 두 키로 각각 추론해 대역을 비교 (전환이 실제로 효과가 있는지) --
image = Image.fromarray(                           # 고정 입력 (같은 입력에 두 키만 바꾼다)
    (np.random.RandomState(0).rand(224, 224, 3) * 255).astype(np.uint8)
)
prompt = f"In: What action should the robot take to {INSTRUCTION}?\nOut:"
inputs = processor(prompt, image).to("cuda:0", dtype=torch.float16)


for key in ["bridge_orig", DATASET_KEY]:           # 남의 통계 vs 내 통계
    with torch.no_grad():
        action = vla.predict_action(
            input_ids=inputs["input_ids"],
            pixel_values=inputs["pixel_values"],
            unnorm_key=key,
            do_sample=False,                       # 결정적 출력
        )
    print(f"\n[3-3] unnorm_key={key}")
    print("   action:", np.round(action, 4))
    print("   위치 3차원 크기:", np.round(np.abs(action[:3]), 4))


# -- 3-4. 학습 데이터 통계 대역과 대조 (README §7 의 3층) --
# 통계 파일의 분위수를 꺼내 위 출력이 그 범위 안에 있는지 본다.
# week1 실습 6 의 히스토그램 범위와도 비교한다.
action_stats = stats[DATASET_KEY]["action"]         # <- 실제 구조에 맞게 교체
for name, value in action_stats.items():
    print(f"   {name}: {np.round(np.asarray(value), 4)}")
```


**판정**

| 관측 | 해석 |
|---|---|
| 두 키의 출력 대역이 뚜렷이 다르고, 내 키 쪽이 학습 통계 범위 안 | 전환 성공 |
| 두 키의 출력이 같다 | 키가 실제로 적용되지 않았다 (주입 위치·구조 확인) |
| 내 키 출력이 학습 통계 범위를 크게 벗어난다 | 통계 구조 매핑 오류 |
| 내 키에서 예외 발생 | 키 이름 또는 통계 형식 불일치 |


> 두 번째 줄이 가장 위험하다 — 오류 없이 옛 키로 역정규화되면 **남의 로봇 스케일로 조용히 틀린다** (README §4).


---


## 실습 4: 버전 호환성 + 결정성 검증


**파일명**: `practice_compat_check.py`


```python
"""
실습 4: 학습 환경과 추론 환경의 조합을 대조하고 결정성을 확인
"""
import torch
import numpy as np


print("=" * 60)
print("실습 4: 호환성 + 결정성")
print("=" * 60)


# -- 4-1. 현재 추론 환경 버전 출력 (Phase 4 SETUP §7 매트릭스와 대조) --
import transformers, tokenizers, timm, accelerate, bitsandbytes
print("\n[4-1] 추론 환경")
for module in [torch, transformers, tokenizers, timm, accelerate, bitsandbytes]:
    print(f"   {module.__name__}: {module.__version__}")
# week3 컨테이너의 버전(outputs/image_build.md 기록)과 표로 대조한다.
# 다른 항목이 있어도 그 자체가 문제는 아니다 -- 로드와 출력이 정상이면 통과다.


# -- 4-2. 결정성 확인: 같은 입력에 같은 출력인가 --
# 실습 3 의 inputs 를 재사용한다 (같은 세션)
outputs = []
for trial in range(3):                             # 3회 반복
    with torch.no_grad():
        action = vla.predict_action(
            input_ids=inputs["input_ids"],
            pixel_values=inputs["pixel_values"],
            unnorm_key=DATASET_KEY,
            do_sample=False,                       # greedy -- 항상 같은 출력이어야 한다
        )
    outputs.append(np.asarray(action))
    print(f"   trial{trial}: {np.round(action, 5)}")


max_diff = max(np.abs(outputs[0] - other).max() for other in outputs[1:])
print(f"\n[4-2] 최대 편차: {max_diff:.2e}")
print("   판정:", "결정적" if max_diff < 1e-6 else "비결정 -- do_sample / dropout 설정 확인")
```


**기록할 것** (`outputs/compat_check.md`)

- 학습 환경 vs 추론 환경 버전 대조 표
- 로드 성공 여부와 경고 메시지 전문
- 결정성 판정
- **안 열렸을 경우**: 어느 선택지를 골랐는지와 그 대가 (README §6)


---


## 실습 5: 하네스 smoke test (1 episode)


**파일명**: `practice_smoke_test.py`


week0 실습 6 의 코드를 가져와 **모델 경로와 `unnorm_key` 만 바꾼다.** 다른 것을 바꾸면 week5 의 before/after 가 성립하지 않는다.


```python
"""
실습 5: week0 하네스에 fine-tuned 모델을 꽂아 1 episode 돌린다 (성공률 판정 안 함)
"""
import numpy as np
import torch
import gymnasium as gym
import mani_skill.envs
from PIL import Image


# week0 실습 6 에서 고정한 값 -- 그대로 쓴다
ENV_ID = "PickCube-v1"                             # <- week0 확정값
STEP_CAP = 100                                     # <- week0 확정값
SMOKE_SEED = 500                                   # eval seed(week0 목록)와 겹치지 않는 값
INSTRUCTION = "pick up the cube"                   # 같은 문구


# 바뀌는 것은 이 두 개뿐이다
MODEL_PATH = "/root/models/openvla-maniskill-ft"   # zero-shot 은 "openvla/openvla-7b" 였다
UNNORM_KEY = "maniskill_pickcube"                  # zero-shot 은 "bridge_orig" 였다


print("=" * 60)
print("실습 5: smoke test (1 episode)")
print("=" * 60)


# 모델 로드는 실습 2 와 동일 (같은 세션이면 재사용)
env = gym.make(ENV_ID, obs_mode="rgb", control_mode="pd_ee_delta_pose")
obs, info = env.reset(seed=SMOKE_SEED)
prompt = f"In: What action should the robot take to {INSTRUCTION}?\nOut:"


for step in range(STEP_CAP):
    frame = np.asarray(obs["sensor_data"]["base_camera"]["rgb"])   # <- week0 확정 경로
    if frame.ndim == 4:
        frame = frame[0]
    image = Image.fromarray(frame.astype(np.uint8)).resize((224, 224))
    model_inputs = processor(prompt, image).to("cuda:0", dtype=torch.float16)
    with torch.no_grad():
        raw_action = vla.predict_action(
            input_ids=model_inputs["input_ids"],
            pixel_values=model_inputs["pixel_values"],
            unnorm_key=UNNORM_KEY,
            do_sample=False,
        )
    # 변환은 week0 계약 표의 정변환을 그대로 쓴다 (한 곳에 둔 코드를 부른다)
    action = raw_action                            # <- week0 정변환 적용으로 교체
    obs, reward, terminated, truncated, info = env.step(action)
    if terminated or truncated or info.get("success", False):
        break


env.close()
print(f"\n루프 완주: {step + 1} 스텝. 예외 없음")
print("성공/실패는 판정하지 않는다 -- week5 의 N회 측정에서 다룬다")
```


**통과 판정** (README §7 의 4층)

| 층 | 확인 |
|---|---|
| 1 | OOM 없이 적재 (실습 2) |
| 2 | VRAM 이 baseline 과 근사 일치 (실습 2) |
| 3 | 출력 대역이 학습 통계 범위 안 (실습 3) |
| 4 | 같은 입력에 같은 출력 (실습 4) |
| + | 하네스에서 예외 없이 1 episode 완주 (실습 5) |


> 여기서 성공했는지 궁금하더라도 판정하지 않는다. 표본 1개의 인상이 week5 해석을 오염시킨다 (README §8).


---


## 마무리: week5 로 넘기는 것


| 고정해 넘기는 것 | 출처 |
|---|---|
| seed 목록 (eval 전용) | week0 `zeroshot_baseline.json` |
| step cap / 카메라 키 경로 / instruction | week0 `sim_facts.md` |
| 4-bit 설정 (nf4 / double quant / compute dtype) | 이번 주 실습 2 |
| 부분 도달률 판정식 | week0 실습 6 |
| 정변환 코드 | week0 계약 표 구현체 |
| **바뀌는 것: 모델 경로 + `unnorm_key`** | 이번 주 실습 5 |


검증 로그는 `Measurements/` 에 남긴다.


| 산출물 | 착지점 |
|---|---|
| `outputs/transfer_check.md`, `outputs/compat_check.md` | `Measurements/openvla-lora-runpod/environment.md` |
| VRAM 실측 + 4층 검증 결과 | 같은 디렉터리 `findings.md` |
| `practice_*.py` | 같은 디렉터리 `scripts/` |


> 모델 가중치는 커밋하지 않는다 (`*.safetensors` 는 gitignore 대상). 위치와 재현 절차만 남긴다.
