# Week 5: OpenX-Embodiment 데이터 + Fine-tuning 흐름


> **이번 주 목표**: OpenX-Embodiment 의 22 embodiments 와 970K episodes 의 구조를 파악하고, fine-tuning (특히 LoRA) 의 실제 흐름을 코드 수준에서 이해한다.
> **예상 시간**: 10시간 (논문 Sec 4 재정독 3h + datasets 라이브러리 실습 4h + 노트 3h)
> **핵심 질문**: "내 자작 6DOF 팔 (Phase 7) 의 데이터는 OpenX-Embodiment 의 어떤 embodiment 와 가장 가까운가? 그 데이터의 trajectory format 은 어떻게 생겼는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | 환경 | `requirements.txt` | datasets, h5py 추가 |
| 2 | OpenX 페이지 정독 | https://robotics-transformer-x.github.io/ | dataset list / format |
| 3 | OpenVLA 논문 Sec 4 재정독 | OpenVLA PDF | fine-tuning details |
| 4 | datasets 라이브러리 실습 | `PRACTICE.md` 1-2 | HuggingFace로 OpenX 일부 로드 |
| 5 | LoRA 코드 흐름 분석 | `PRACTICE.md` 3 | OpenVLA repo 의 LoRA config |
| 6 | 퀴즈 | quiz_easy / quiz_medium | 데이터 구조 / LoRA 설정 |
| 7 | 노트 정리 | `PRACTICE.md` 4 | "자작 팔 데이터 호환성 분석" 노트 |


---


## 시작하기 전에 — 본 주의 실용적 가치


이번 주 학습이 가장 직접적으로 **Phase 7 산출물 #4 의 fine-tuning 단계** 와 연결된다:


```
Phase 7 산출물 #4 의 fine-tuning 단계 (2027.06):
  1. 자작 6DOF 팔로 teleop demonstration ~100개 수집
  2. OpenX-Embodiment format 으로 변환
  3. OpenVLA + LoRA fine-tune
  4. evaluation + 영상
```


이번 주에 1-3 의 *방법* 을 미리 익혀둔다. 자작 팔의 데이터 format 까지 미리 결정하면 Phase 7 진입 시 1-2주 절약 가능.


---


## 핵심 개념


### 1. OpenX-Embodiment 의 구조


OpenX-Embodiment 는 **60개의 dataset 을 표준 schema 로 통합** 한 collection:


```
OpenX-Embodiment
  ├── RT-1 (Google) — Franka, ~130K episodes
  ├── Bridge Data V2 — WidowX, ~50K episodes
  ├── Berkeley Cable Routing — UR5, ~1K episodes
  ├── DROID — Franka, ~75K episodes
  ├── Bridge — WidowX, ~25K episodes
  ├── Stanford Robocook — Franka, ~3K episodes
  ... (총 60개 dataset)
  └── (총 970K episodes, 22 embodiments)
```


표준 schema (RLDS 형식 - Reinforcement Learning Dataset Schema):


```python
{
    'steps': [
        {
            'observation': {
                'image': <RGB image>, # 224x224 또는 256x256
                'natural_language_instruction': <str>, # 'pick up the can'
                # (선택) 'wrist_image', 'depth', 'state' 등
            },
            'action': <7-DoF or N-DoF>, # [dx, dy, dz, rx, ry, rz, gripper]
            'reward': <float>,
            'is_first': <bool>, 'is_last': <bool>, 'is_terminal': <bool>,
        },
        ... (한 episode 의 모든 step)
    ]
}
```


### 2. 22 embodiments 별 특징 (대표 5개)


| Embodiment | Robot type | DoF | Gripper | Episode 수 | 본 로드맵 자작 팔과의 유사도 |
|---|---|---|---|---|---|
| Franka Emika Panda | 7-DoF arm | 7 | Franka 2-finger | ~ 200K | 매우 높음 (가장 가까움) |
| WidowX 250 | 6-DoF arm | 6 | WidowX gripper | ~ 80K | 높음 (6DOF) |
| UR5 / UR5e | 6-DoF arm | 6 | 다양 | ~ 30K | 높음 |
| xArm 7 | 7-DoF arm | 7 | xArm gripper | ~ 20K | 중간 |
| Sawyer | 7-DoF arm | 7 | Rethink gripper | ~ 5K | 중간 |


> 본 로드맵 자작 팔 (Dynamixel XM430 6DOF) 와 가장 가까운 것은 **WidowX 250** (6-DoF Dynamixel 기반). Phase 7 의 LoRA fine-tune 시 WidowX subset 의 trajectory 가 가장 유용한 학습 prior.


### 3. Action space 표준화


OpenX-Embodiment 는 embodiment 별로 action space 가 다르다. OpenVLA 는 **표준화** 하여 학습:


```
Embodiment 별 action:
  Franka : 7-DoF joint velocity
  WidowX : 6-DoF end-effector delta pose + gripper
  UR5 : 6-DoF end-effector delta pose + gripper
  ...


OpenVLA 표준화:
  [dx, dy, dz, rx, ry, rz, gripper] (7-DoF end-effector delta)
  + 각 embodiment 의 자체 normalization
```


이게 22 embodiments 를 한 모델에 학습 가능한 핵심.


### 4. Fine-tuning 흐름 (OpenVLA GitHub repo 기준)


```
1. 데이터 준비:
   - HuggingFace datasets 로 OpenX subset 로드 (또는 직접 trajectory)
   - RLDS schema 변환
   - image preprocess (224x224, normalize)
   - action normalize per dataset


2. 모델 준비:
   - OpenVLA base 7B 로드 (4-bit 양자화 권장)
   - LoRA config 추가 (rank 16~32)


3. 학습 루프:
   - batch_size 8~16 (gradient accumulation 8)
   - lr 5e-4 (LoRA), warmup 100 step
   - 1~5 epoch (5K~50K step)
   - GPU 1대 RTX 4090 기준 1~10 시간


4. 평가:
   - 평가 trajectory 의 success rate
   - real-world (또는 sim) 에서 50 trial 정도
```


### 5. LoRA Config 상세


OpenVLA repo 의 LoRA config 예시:


```python
from peft import LoraConfig


config = LoraConfig(
    r=32, # rank
    lora_alpha=64, # scaling
    target_modules=[
        'q_proj', 'k_proj', 'v_proj', 'o_proj',
        'gate_proj', 'up_proj', 'down_proj',
    ], # 모든 Llama attention + FFN linear
    lora_dropout=0.0,
    bias='none',
    task_type='CAUSAL_LM',
)
```


설정 별 영향:
- **rank (r)**: 클수록 표현력 강함, 작을수록 메모리 절약. 16-32 권장.
- **alpha**: scaling factor. 일반적으로 alpha = 2 * rank.
- **target_modules**: 어디에 LoRA 적용. attention 만 vs FFN 까지 vs all linear.
- **dropout**: regularization. 보통 0 ~ 0.1.


### 6. LoRA 학습 시 GPU 메모리 예상


OpenVLA 7B + LoRA on RTX 4070 12GB:


```
- Base model (int4 nf4) : ~ 4 GB
- LoRA weights (fp16) : ~ 0.2 GB
- Activation memory (batch=1) : ~ 4 GB
- Gradient (LoRA only, fp16) : ~ 0.2 GB
- Optimizer state (Adam, bf16): ~ 0.4 GB
- KV cache + buffer : ~ 1 GB
-----------------------------------------
Total : ~ 10 GB (fit!)
```


> batch_size=1 + gradient_accumulation=8 이 본 로드맵 RTX 4070 환경의 표준.


### 7. 자작 팔 데이터 수집 (Phase 7 의 예고편)


자작 6DOF 팔 (Dynamixel XM430) 의 teleop 데이터 수집 흐름:


```
1. teleop 입력 장치 (leader-follower 또는 PS4 패드)
2. 1~10 Hz 로 RGB 카메라 + joint state + action 기록
3. 한 task 당 50~100 demonstrations
4. RLDS format 으로 저장 (HDF5 또는 TFRecord)
5. OpenVLA + LoRA fine-tune
```


### 8. 평가 지표


OpenVLA 의 평가 지표 (논문 Table):


| 지표 | 의미 | 산출물 #4 에서 측정 |
|---|---|---|
| Success rate | 50 trial 중 성공 비율 | O 핵심 |
| Recovery rate | 실패 후 복구 비율 | O (선택) |
| Generalization | unseen object 성공률 | O 보조 |
| Latency | inference time / step | O (Phase 7 의 핵심) |


---


## 자체 점검


**Q1. OpenX-Embodiment 의 RLDS schema 의 핵심 5 필드는?**
> observation.image, observation.natural_language_instruction, action, reward, is_first/is_last/is_terminal flag. 가장 중요한 건 image / instruction / action 3 가지.


**Q2. 자작 6DOF 팔과 가장 가까운 OpenX embodiment 는?**
> WidowX 250 (6-DoF + Dynamixel 기반 + 2-finger gripper). Bridge / Bridge V2 dataset 이 가장 유용한 LoRA 사전 분포.


**Q3. OpenVLA + LoRA 학습 시 학습 가능 파라미터는 전체 모델의 몇 %인가?**
> rank 16-32 기준 약 1% 미만 (~50-100M out of 7B). 이게 빠른 fine-tuning 의 핵심.


**Q4. RTX 4070 12GB 에서 OpenVLA + LoRA 학습 가능한가?**
> 가능. base model 4-bit nf4 + LoRA fp16 + batch_size 1 + gradient_accumulation 8 조합으로 ~ 10GB 사용.


**Q5. 자작 팔 데이터 수집 시 episode 당 평균 step 수는?**
> 보통 30-100 step (5-15초). 1 task 당 50-100 episode 면 ~ 5000-10000 step. LoRA 학습에 충분.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. OpenX-Embodiment 페이지 정독 + dataset list 확인
2. `practice_openx_subset.py` - HuggingFace datasets 로 일부 로드
3. `practice_lora_config.py` - PEFT 의 LoRA config 작성 + 파라미터 수 확인
4. "자작 팔 데이터 호환성 분석" 노트
5. quiz_easy / quiz_medium 풀기


### 다음 주 (week 6) 준비
- OpenVLA HuggingFace 모델 카드 한 번 더: https://huggingface.co/openvla/openvla-7b
- bitsandbytes 라이브러리 설치 시도 (4-bit quantization, week 6 의 핵심)


---


## 이번 주 핵심 요약


1. **OpenX-Embodiment 는 22 embodiments 통합** 데이터셋, RLDS schema.
2. **자작 6DOF 팔 = WidowX 250 유사** → Bridge dataset 의 trajectory 가 prior 로 유용.
3. **LoRA + 4-bit base** 가 RTX 4070 12GB 환경의 표준 fine-tune 구성.
4. **학습 파라미터 ~ 1% 미만** → 새 robot 빠른 적응.
5. **자작 팔 데이터 수집**: 50-100 demonstrations × 30-100 step = ~5K~10K step.


---


- 이전: [Week 4 - OpenVLA 1회독 + Architecture](../week4/README.md)


다음: [Week 6 - HuggingFace 모델 카드 + 환경 셋업](../week6/README.md)
