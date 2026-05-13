# Week 1 실습: Fork + 데이터 + 변환

> [time] **예상 시간**: 5시간

---

## 실습 1: OpenVLA fork

```bash
gh repo fork openvla/openvla --clone
cd openvla
git checkout -b my-arm-finetune
```

## 실습 2: Teleop 데이터 수집

자작 6DOF 팔 (Hardware-Arm Stage 2) 에서:
```python
# teleop_collect.py - leader-follower 또는 PS4 패드
# 매 step 마다 image + joint_state + action 기록
# 50 steps per episode, 300 episodes
```

## 실습 3: OpenX 변환

```python
# convert_to_rlds.py
def episode_to_rlds(episode):
    return {'steps': [
        {'observation': {'image': step.img, 'natural_language_instruction': step.task,
                          'state': step.joint_state},
         'action': step.action,
         'reward': 1.0 if step.is_success else 0.0,
         'is_first': step.t == 0, 'is_last': step.t == n-1, 'is_terminal': step.is_terminal}
        for step in episode
    ]}

# HDF5 또는 TFRecord 저장
```

## 실습 4: LoRA config + 학습 시작

```python
from peft import LoraConfig, get_peft_model
config = LoraConfig(r=32, lora_alpha=64, ...)
model = get_peft_model(vla, config)
model.print_trainable_parameters()  # ~95M
```

## [O] 체크리스트
- [ ] OpenVLA fork
- [ ] Teleop 300 demos 수집
- [ ] OpenX format 변환
- [ ] LoRA config 설정
