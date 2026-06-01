# Week 1: OpenVLA fork + 자작 팔 fine-tuning 데이터 준비


> **이번 주 목표**: OpenVLA fork + teleop demonstrations 를 OpenX format 으로. LoRA fine-tune 사전.
> **예상 시간**: 10시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | OpenVLA fork | `PRACTICE.md` 1 | GitHub fork |
| 2 | Teleop 데이터 수집 | `PRACTICE.md` 2 | Hardware-Arm Stage 2 |
| 3 | OpenX format 변환 | `PRACTICE.md` 3 | RLDS |
| 4 | LoRA config | `PRACTICE.md` 4 | Phase 4 학습 |
| 5 | 퀴즈 | | |


---


## Phase 7 = 결정타 산출물 #4


```
이전 Phase 자산:
- Phase 4: OpenVLA + ROS2 minimal demo
- Phase 5: Foundation Model 이해
- Phase 6: Isaac Sim + 디지털 트윈
- Hardware-Arm Stage 2: 자작 6DOF 팔


산출물 #4 (2027.08~):
- Real-to-Sim-to-Real 영상 1~3분
- ROS2 패키지
- latency / Sim/Real gap 보고서
```


---


## 핵심 개념


### 1. OpenVLA fork


```bash
gh repo fork openvla/openvla --clone
git checkout -b my-arm-finetune
```


이유: 자작 팔 custom adapter, LoRA script 수정.


### 2. Teleop 데이터


```
Task: pick-and-place 3 종류
Demos: 100~200 per task (총 300~600 episodes)
Length: ~ 50 steps @ 5Hz
방법: Leader-follower teleop 또는 PS4 패드
```


### 3. OpenX RLDS schema


```python
{
    'steps': [{
        'observation': {'image': <224x224>, 'natural_language_instruction': str, 'state': <6dof>},
        'action': <7dof>,
        'reward': <0 or 1>,
        'is_first/is_last/is_terminal': bool,
    }, ...]
}
```


### 4. LoRA Config


```python
LoraConfig(r=32, lora_alpha=64, lora_dropout=0.0,
           target_modules=['q_proj','k_proj','v_proj','o_proj',
                            'gate_proj','up_proj','down_proj'],
           task_type='CAUSAL_LM')
```


### 5. 학습 hyper


```
batch_size 1, grad_accum 8
lr 5e-4, warmup 100
epochs 1~3, ~5K~20K steps
RTX 4070: 5-10 hr
```


### 6. 평가 plan


- Baseline (zero-shot): ~ 0-20%
- LoRA fine-tuned 목표: > 70%
- 50 trial * 3 task


---


## 자체 점검


**Q1. Fork 의 이유?** > Custom adapter + LoRA script.
**Q2. Demos 권장?** > 300-600 episodes.
**Q3. Data format?** > OpenX RLDS.
**Q4. LoRA rank?** > 32.
**Q5. 학습 시간?** > 5-10 hr (RTX 4070).


---


## 실습 + 다음


### 이번 주: fork + 데이터 + format + quiz
### 다음 주 (week 2): Inference 노드 통합


---


## 핵심 요약


1. **OpenVLA fork**
2. **300-600 demos**
3. **OpenX RLDS format**
4. **LoRA rank 32**
5. **5-10 hr 학습**


- [Phase 6](../../../Roadmap/Phase%206.md) | [Week 2](../week2/README.md)
