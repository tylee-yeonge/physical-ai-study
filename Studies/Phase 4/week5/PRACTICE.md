# Week 5 실습: OpenX-Embodiment 데이터 + LoRA 흐름


> **실습 목표**: OpenX-Embodiment 의 한 dataset 을 직접 로드해 RLDS schema 를 손에 익히고, LoRA config 를 작성해 학습 가능 파라미터를 확인한다.
> **예상 시간**: 6-8시간


---


## 환경 설정


```bash
# 내부에서 .venv-week5 생성 후 requirements.txt 설치
# (datasets, h5py, peft (LoRA 라이브러리) 포함)
bash pip_install.sh
source .venv-week5/bin/activate
```


---


## 실습 1: HuggingFace datasets 로 OpenX subset 로드


**파일명**: `practice_openx_subset.py`


```python
"""
실습 1: OpenX-Embodiment 의 한 subset (예: bridge) 일부를 로드
"""
from datasets import load_dataset
import numpy as np


print("=" * 60)
print("실습 1: OpenX-Embodiment subset 로드")
print("=" * 60)


# 주의: OpenX 전체는 수 TB. 일부만 streaming 으로 로드
# 또는 robotics 공식 site 의 작은 dataset
# 예시: bridge_v2 일부


# 방법 1: HuggingFace datasets (작은 subset 만)
try:
    ds = load_dataset(
        "lerobot/bridge_orig", # 또는 다른 RLDS 호환 dataset
        split="train",
        streaming=True,
    )
    print("\n[1-1] dataset 로드 성공")


    # 첫 3 episode 확인
    for i, ep in enumerate(ds):
        if i >= 3:
            break
        print(f"\n Episode {i}:")
        # observation 의 첫 step
        if 'observation' in ep:
            obs = ep['observation']
            print(f"obs keys: {list(obs.keys()) if hasattr(obs, 'keys') else 'N/A'}")
        if 'action' in ep:
            print(f"action shape: {np.asarray(ep['action']).shape if ep['action'] is not None else 'None'}")


except Exception as e:
    print(f"[warn] HuggingFace 로드 실패: {e}")
    print("-> RT-X 공식 페이지 또는 OpenVLA repo 의 작은 sample 사용")


# 방법 2: OpenVLA repo 의 작은 demo dataset (대안)
print("\n[1-2] 대안: OpenVLA repo 에서 sample trajectory 확인")
print("$ git clone https://github.com/openvla/openvla")
print("$ ls openvla/data/ # 또는 OpenVLA 의 demo dataset 위치")


print("\n 실습 1 완료!")
```


> OpenX 전체 (4 TB+) 는 본 로드맵에서 다 다운로드 안 함. WidowX / Bridge 의 작은 subset (~ 10GB) 으로 충분.


---


## 실습 2: RLDS schema 분석


**파일명**: `practice_rlds_schema.py`


```python
"""
실습 2: RLDS schema 의 표준 필드 시뮬레이션
목적: 자작 팔 데이터를 OpenX-Embodiment 와 호환되게 만들기 위한 준비
"""
import numpy as np


print("=" * 60)
print("실습 2: RLDS schema")
print("=" * 60)




# -- 2-1. 자작 팔 한 episode 시뮬레이션 --
def simulate_episode(n_steps=50, dof=6):
    steps = []
    for t in range(n_steps):
        steps.append({
            'observation': {
                'image': np.random.randint(0, 255, (224, 224, 3), dtype=np.uint8),
                'natural_language_instruction': "pick up the can",
                'joint_state': np.random.randn(dof).astype(np.float32),
            },
            'action': np.random.uniform(-0.05, 0.05, 7).astype(np.float32),
            'reward': 1.0 if t == n_steps - 1 else 0.0,
            'is_first': t == 0,
            'is_last': t == n_steps - 1,
            'is_terminal': t == n_steps - 1,
        })
    return {'steps': steps}




print("\n[2-1] 자작 팔 episode 시뮬레이션")
ep = simulate_episode(n_steps=50, dof=6)
print(f"Episode step 수 : {len(ep['steps'])}")
print(f"Step 0 의 obs.keys() : {list(ep['steps'][0]['observation'].keys())}")
print(f"Step 0 의 action : {ep['steps'][0]['action']}")
print(f"Step 0 의 is_first : {ep['steps'][0]['is_first']}")


# -- 2-2. OpenX schema 와의 호환성 점검 --
print("\n[2-2] OpenX 호환성 점검")
required_obs = ['image', 'natural_language_instruction']
required_step = ['observation', 'action', 'is_first', 'is_last']


obs_keys = set(ep['steps'][0]['observation'].keys())
step_keys = set(ep['steps'][0].keys())


obs_missing = set(required_obs) - obs_keys
step_missing = set(required_step) - step_keys


print(f"obs 누락 : {obs_missing or '없음'}")
print(f"step 누락: {step_missing or '없음'}")


if not obs_missing and not step_missing:
    print("\n OpenX schema 호환!")


print("\n 실습 2 완료!")
```


---


## 실습 3: PEFT 의 LoRA config


**파일명**: `practice_lora_config.py`


```python
"""
실습 3: PEFT 의 LoRA 를 작은 mock 모델에 적용해 학습 가능 파라미터 확인
"""
import torch
import torch.nn as nn
from peft import LoraConfig, get_peft_model


print("=" * 60)
print("실습 3: LoRA config 와 학습 가능 파라미터 수")
print("=" * 60)




# -- 3-1. Mock LM (Llama 7B 의 작은 버전) --
class MockTransformerBlock(nn.Module):
    def __init__(self, hidden=4096):
        super().__init__()
        self.q_proj = nn.Linear(hidden, hidden, bias=False)
        self.k_proj = nn.Linear(hidden, hidden, bias=False)
        self.v_proj = nn.Linear(hidden, hidden, bias=False)
        self.o_proj = nn.Linear(hidden, hidden, bias=False)
        self.gate_proj = nn.Linear(hidden, hidden * 4, bias=False)
        self.up_proj = nn.Linear(hidden, hidden * 4, bias=False)
        self.down_proj = nn.Linear(hidden * 4, hidden, bias=False)




class MockLM(nn.Module):
    def __init__(self, n_layers=32, hidden=4096):
        super().__init__()
        self.layers = nn.ModuleList([MockTransformerBlock(hidden) for _ in range(n_layers)])




# -- 3-2. LoRA config --
def count_params(model, trainable_only=False):
    if trainable_only:
        return sum(p.numel() for p in model.parameters() if p.requires_grad)
    return sum(p.numel() for p in model.parameters())




print("\n[3-1] Mock LM 생성 (32 layer, hidden 4096)")
# 실제 메모리에 다 안 띄우고 layer 4 개로 축소
model = MockLM(n_layers=4, hidden=4096)
total_params = count_params(model)
print(f"전체 파라미터 (4 layer 축약): {total_params:,}")
print(f"추정 32 layer 전체 : {total_params * 32 // 4:,}")


# -- 3-3. PEFT LoRA 적용 --
# PEFT 가 Mock 모델에 그대로 적용은 어려우므로 LoraConfig 만 확인
lora_config = LoraConfig(
    r=32,
    lora_alpha=64,
    target_modules=['q_proj', 'k_proj', 'v_proj', 'o_proj',
                    'gate_proj', 'up_proj', 'down_proj'],
    lora_dropout=0.0,
    bias='none',
)
print(f"\n[3-2] LoRA config")
print(f"r : {lora_config.r}")
print(f"alpha : {lora_config.lora_alpha}")
print(f"target : {lora_config.target_modules}")


# -- 3-4. LoRA 파라미터 수 직접 계산 --
print("\n[3-3] LoRA 학습 파라미터 수 계산")
hidden = 4096
n_layers = 32 # Llama 7B 의 실제 layer 수
rank = 32


# attention: q,k,v,o 4 개 모두 hidden -> hidden
attn_params = 4 * rank * (hidden + hidden) * n_layers
# FFN: gate, up (hidden -> 4*hidden), down (4*hidden -> hidden)
ffn_params = (
    2 * rank * (hidden + 4 * hidden) * n_layers # gate, up
    + rank * (4 * hidden + hidden) * n_layers # down
)


lora_total = attn_params + ffn_params
base_total = 7e9
ratio = lora_total / base_total * 100


print(f"attention LoRA params : {attn_params:,}")
print(f"FFN LoRA params : {ffn_params:,}")
print(f"총 LoRA params : {lora_total:,}")
print(f"Base 7B 모델 대비 : {ratio:.2f}%")


print("\n 실습 3 완료!")
```


> 실제 OpenVLA repo 에서 학습 시 PEFT 가 자동으로 LoRA layer 를 wrap. `print_trainable_parameters()` 한 줄로 확인 가능.


---


## 실습 4: 자작 팔 데이터 호환성 분석 노트


**파일명**: `~/phase4_notes/week5/self_arm_data_compat.md`


```markdown
# 자작 6DOF 팔 데이터 호환성 분석


## 1. 자작 팔 hardware
- Robot: Dynamixel XM430 6DOF + 그리퍼
- Camera: ELP Stereo (보유)
- DoF: 6 + gripper = 7


## 2. OpenX-Embodiment 의 가장 가까운 embodiment
- WidowX 250 (6-DoF arm, Dynamixel 기반)
- Bridge / Bridge V2 dataset 활용 가능


## 3. RLDS schema 호환 작업
- [ ] observation.image: RGB 224x224 (resize 필요)
- [ ] observation.natural_language_instruction: 작업 명령
- [ ] action: [dx, dy, dz, rx, ry, rz, gripper] (end-effector delta)
- [ ] reward / is_first / is_last: episode 경계만 1


## 4. Action space 변환
- 자작 팔: joint position (6 dim)
- OpenVLA: end-effector delta pose (7 dim)
- 변환: forward kinematics 로 EE pose 계산 -> delta 계산
- 표현 3층위: 관절각(자작 팔 원본 6 dim) -> EE-delta(OpenX 표준 7 dim) -> action token(OpenVLA 가 256 bin 으로 이산화해 출력, README week5 §3.5)
- 주의: 내가 수집/저장하는 단위는 EE-delta 까지. token 화는 OpenVLA 내부에서 일어나고 de-tokenize 로 다시 연속값이 되므로, 데이터셋 포맷에는 token 이 아니라 연속 action 을 담는다


## 5. 데이터 수집 계획 (Phase 7 의 사전 설계)
- task: pick-and-place 1~3 종류
- demonstrations: 50~100 episode per task
- episode 길이: 30~100 step (5~15초)
- 수집 방법: leader-follower teleop 또는 PS4 패드


## 6. LoRA 설정 (예정)
- rank: 32
- alpha: 64
- target: q,k,v,o,gate,up,down (all linear)
- learning rate: 5e-4
- batch_size: 1 + grad_accum 8


## 7. 평가 계획
- success rate (50 trial)
- 비교: zero-shot OpenVLA vs LoRA fine-tuned
- latency 측정 (Phase 7 의 핵심)
```


---


## 실습 체크리스트


- [ ] `practice_openx_subset.py` 실행 (또는 OpenX 공식 페이지에서 dataset 구조 확인)
- [ ] `practice_rlds_schema.py` 실행
  - [ ] schema 호환성 출력 확인
- [ ] `practice_lora_config.py` 실행
  - [ ] LoRA 비율 ~ 1% 확인
- [ ] 자작 팔 호환성 노트 산출
- [ ] quiz_easy / quiz_medium 풀기
- [ ] git commit


---


## 참고 자료


- [OpenX-Embodiment paper](https://arxiv.org/abs/2310.08864)
- [OpenX project page](https://robotics-transformer-x.github.io/)
- [OpenVLA GitHub - LoRA training](https://github.com/openvla/openvla)
- [PEFT (HuggingFace LoRA)](https://github.com/huggingface/peft)
- [RLDS schema](https://github.com/google-research/rlds)
- [Bridge Data V2](https://rail-berkeley.github.io/bridgedata/)
