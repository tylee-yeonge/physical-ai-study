# Week 2: Inference 노드 통합 (Phase 4 demo 확장)


> **이번 주 목표**: LoRA fine-tuned OpenVLA 의 ROS2 inference 노드. Phase 4 의 vla_node 를 자작 팔용으로 확장.
> **예상 시간**: 8시간


---


## 학습 순서


| # | 단계 | 파일 |
|---|---|---|
| 1 | LoRA fine-tune 실행 | `PRACTICE.md` 1 |
| 2 | Phase 4 vla_node 확장 | `PRACTICE.md` 2 |
| 3 | 자작 팔 joint action 변환 | `PRACTICE.md` 3 |
| 4 | 퀴즈 | |


---


## 핵심 개념


### LoRA fine-tune 실행


```bash
python train.py --base_model openvla/openvla-7b \
    --data_dir ~/teleop_data \
    --lora_r 32 --lr 5e-4 \
    --epochs 2 --batch_size 1 --grad_accum 8
```


### vla_node 확장 (Phase 4 의 vla_inference_node.py)


```python
# 기존: zero-shot OpenVLA
# 확장: LoRA fine-tuned


vla = AutoModelForVision2Seq.from_pretrained(
    "my-finetuned-openvla", # 본인 LoRA fine-tuned
    ...
)


# action -> joint command 변환 추가
def action_to_joint_command(ee_delta):
    # IK 또는 직접 joint delta
    ...
```


### 자작 팔의 action format


OpenVLA: end-effector delta (7-DoF)
자작 팔: joint angle (6-DoF) + gripper


변환:
```
ee_delta (xyz, rpy) -> IK -> joint angle delta
gripper -> dynamixel position
```


### Phase 4 와의 차이


| 항목 | Phase 4 | Phase 7 week 2 |
|---|---|---|
| 모델 | OpenVLA zero-shot | LoRA fine-tuned |
| Action | bridge_orig (WidowX) | 자작 팔 자체 통계 |
| 출력 | /vla/action (Twist) | /joint_command (JointState) |
| Target | Sim or recorded image | 실 자작 팔 |


---


## 자체 점검


**Q1.** 학습 후 unnormalize_key? > 자작 팔 자체 통계 (custom).
**Q2.** ee_delta -> joint 변환? > IK (PyKDL 또는 ikpy).
**Q3.** Phase 4 와 차이? > Fine-tuned model + joint output.
**Q4.** ROS2 토픽? > /joint_command + /joint_states.
**Q5.** 학습 결과 검증? > success rate (recorded test data).


---


## 핵심 요약


1. **LoRA fine-tuned** OpenVLA inference
2. **Phase 4 vla_node 확장** (joint action output)
3. **IK 변환** ee_delta -> joint
4. **자작 팔 unnormalize_key**
5. **다음: Sim dry-run**


- [Week 1](../week1/README.md) | [Week 3](../week3/README.md)
