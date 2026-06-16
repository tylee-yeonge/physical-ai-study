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
| 4 | action representation 비교 정독 | README §3.5 (+ week4 §5) | 관절각/EE-delta/token, tokenization |
| 5 | datasets 라이브러리 실습 | `PRACTICE.md` 1-2 | HuggingFace로 OpenX 일부 로드 |
| 6 | LoRA 코드 흐름 분석 | `PRACTICE.md` 3 | OpenVLA repo 의 LoRA config |
| 7 | 퀴즈 | quiz_easy / quiz_medium | 데이터 구조 / action 표현 / LoRA 설정 |
| 8 | 노트 정리 | `PRACTICE.md` 4 | "자작 팔 데이터 호환성 분석" 노트 |


---


## 시작하기 전에 — 본 주의 실용적 가치


이번 주 학습이 가장 직접적으로 **Phase 7 산출물 v3 의 fine-tuning 단계** 와 연결된다:


```
Phase 7 산출물 v3 의 fine-tuning 단계 (2027.06):
  1. 자작 6DOF 팔로 teleop demonstration ~100개 수집
  2. OpenX-Embodiment format 으로 변환
  3. OpenVLA + LoRA fine-tune
  4. evaluation + 영상
```


이번 주에 1-3 의 *방법* 을 미리 익혀둔다. 자작 팔의 데이터 format 까지 미리 결정하면 Phase 7 진입 시 1-2주 절약 가능.


---


## 핵심 개념


### 0. embodiment 란 (용어 정의)


**embodiment**(임바디먼트)는 직역하면 "신체화", 즉 지능이 깃든 물리적 몸체를 뜻한다. 로봇 학습 맥락에서는 더 구체적으로 **"하나의 구체적인 로봇 하드웨어 구성"**을 가리킨다. 아래 중 하나라도 다르면 다른 embodiment 다:


| 결정 요소 | 예시 |
|---|---|
| 로봇 종류/형태 | 7-DoF 팔, 6-DoF 팔, 이동 로봇, 휴머노이드 |
| 자유도 (DoF) | Franka 7-DoF vs WidowX 6-DoF |
| 그리퍼 | Franka 2-finger vs WidowX gripper |
| 기구학 (관절 구조/링크 길이) | 같은 6-DoF 라도 구조가 다르면 다른 embodiment |
| 센서 배치 | 카메라 시점(머리/손목), depth 유무 |


즉 §2 표의 Franka / WidowX / UR5 / xArm / Sawyer 가 각각 **서로 다른 embodiment** 다. OpenX-Embodiment 데이터셋은 이런 서로 다른 로봇 22종(22 embodiments)의 데이터를 모은 것이다.


**왜 이 개념이 핵심인가**: embodiment 가 다르면 action space 가 다르다 (§3). Franka 는 action 이 "7-DoF 관절 속도", WidowX 는 "6-DoF end-effector delta + gripper" 다. 몸체가 다르니 "움직여라"를 표현하는 방식 자체가 다르고, 그대로는 22종을 한 모델에 학습시킬 수 없다. OpenVLA 가 이를 공통 표현으로 표준화하는 것이 §3 의 내용이다.


관련 용어:


| 용어 | 뜻 |
|---|---|
| cross-embodiment learning | 여러 로봇 데이터를 함께 학습해 한 로봇에서 배운 prior 를 다른 로봇에 전이시키는 학습 방식 (OpenX/OpenVLA 의 핵심 아이디어) |
| embodiment gap | 학습한 로봇과 실제 배포할 로봇의 몸체 차이. 자작 팔(Dynamixel 6-DoF)과 가장 가까운 게 WidowX 인 것(§2 주석), 이동 로봇이면 action 이 base 속도가 되는 것(§3.5 주석)이 이 gap 을 다루는 사례 |


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
| WidowX 250 | 6-DoF arm | 6 | WidowX gripper | ~ 80K | 매우 높음 (가장 가까움) |
| Franka Emika Panda | 7-DoF arm | 7 | Franka 2-finger | ~ 200K | 중간 (7DOF, 다른 액추에이터) |
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


**action** 은 매 순간 로봇에게 내리는 명령 한 개다. VLA 는 이미지 + 명령("캔을 집어라")을 받아 "지금 이렇게 움직여"라는 action 을 출력하고, 이를 0.1초 단위로 반복해 로봇을 움직인다. "이렇게 움직여"를 숫자로 어떻게 적느냐가 로봇마다 다른 게 표준화가 필요한 이유다.


- **관절각**(joint velocity): 팔의 각 모터를 초당 몇 도 돌릴지 직접 지정. 로봇엔 가장 직접적이지만 모터 개수/배치가 로봇마다 달라(Franka 7개 vs WidowX 6개) 다른 로봇엔 그대로 안 통한다. `DoF`(Degrees of Freedom, 자유도)는 독립적으로 움직이는 축의 개수.
- **EE-delta**(end-effector delta): 모터 대신 팔 맨 끝(손끝, end-effector)을 "공간에서 어디로 옮길지"만 지정. `[dx, dy, dz, rx, ry, rz, gripper]` = 손끝 위치 변화 3 + 자세(회전) 변화 3 + 그리퍼 1.
  - 핵심은 **델타**(delta), 즉 "변화량"이다. "좌표 (0.5, 0.2, 0.3)으로 가라"(절대값)가 아니라 "지금 위치에서 x 로 +2cm 더 가라"(상대 변화량). 매 step 작은 변화량을 계속 내보내며 움직인다.
  - 손끝이 공간에서 움직이는 건 모든 팔에 공통이라 모터 개수가 달라도 말이 된다. 그래서 OpenVLA 가 이걸 공통 표준으로 골랐다.
  - 표기상 7개라 "7-DoF action" 이라 부르지만 엄밀히는 위치 3 + 회전 3 = 6 자유도에 gripper 1 을 더한 것.
- **normalization**(정규화): 로봇마다 한 step 이동 스케일이 다르다(한쪽 5cm, 한쪽 5mm). 학습 전 각 로봇 데이터를 보통 -1 에서 +1 범위로 맞추고(normalize), 실제 로봇에 줄 때 원래 스케일로 되돌린다(de-normalize). 이 "되돌리는 통계"가 로봇마다 다르며, week6 의 `unnorm_key` 가 "어느 로봇 기준으로 되돌릴지" 고르는 인자다.


**"OpenVLA 표준화" 두 줄 상세 (2단계)**


코드 블록의 `OpenVLA 표준화:` 부분은 다음 2단계를 뜻한다.


1단계 — **표현 통일**: 각 embodiment 의 원본 action(관절각이든 EE-delta 든)을 전부 동일한 7-DoF end-effector delta 로 맞춰 모델의 출력 형식을 하나로 고정한다. 7개 차원의 의미:


| 차원 | 기호 | 의미 |
|---|---|---|
| 1-3 | dx, dy, dz | 손끝 위치 변화량 (x/y/z 이동) |
| 4-6 | rx, ry, rz | 손끝 자세 변화량 (roll/pitch/yaw 회전) |
| 7 | gripper | 그리퍼 열림/닫힘 |


2단계 — **스케일 통일**: 위 정규화를 차원별로 적용해 값 범위를 -1 에서 +1 로 맞춘다. 이 정규화 통계가 코드의 "각 embodiment 의 자체 normalization", 즉 로봇(dataset)마다 따로 저장되는 값이다. 정규화가 왜 필요하고 구체적으로 어떻게 -1 에서 +1 로 맞추는지는 아래 "정규화 상세" 참고.


정규화까지 끝난 -1 에서 +1 의 연속값이, 다음 절(§3.5)에서 256 bin 토큰으로 이산화된다.


**정규화 상세 — 왜 로봇마다 다르고, 어떻게 -1 에서 +1 로 맞추나**


같은 `dx`(손끝 x 이동량)라도 dataset 마다 숫자 분포가 다르다. 원인:


- **기록 주기**(Hz): action 은 "한 step 동안의 변화량" 이라 step 시간이 다르면 크기가 달라진다. 같은 0.1 m/s 라도 5 Hz(0.2초/step)는 dx 약 0.02, 30 Hz(0.033초/step)는 dx 약 0.0033 으로 기록 — 같은 동작인데 6배 차이.
- **로봇/작업공간 크기**: 대형 산업팔은 한 step 수십 cm, 소형 데스크탑 팔은 수 cm.
- **단위·좌표 규약**: 위치 m vs cm, 회전 라디안, gripper 0/1 vs -1/+1 등 dataset 마다 제각각.
- **teleop 스타일**: 빠른 시연 vs 느린 시연.


정규화를 안 하면 (1) 값이 큰 차원이 학습 loss 를 지배해 작은 차원을 모델이 무시하고, (2) §3.5 의 256 bin 격자를 모든 차원·dataset 에 공통으로 깔 수 없다.


정규화는 차원별로 데이터에서 구한 `q01`(하위 1% 값) / `q99`(상위 1% 값, 즉 99 분위수)를 각각 -1 / +1 에 대응시킨다. raw min/max 가 아니라 분위수를 쓰는 건 이상치 한두 개가 스케일을 망치지 않게 하기 위함이고, 범위 밖 값은 잘라낸다(clip):


```
정규화   : x_norm = clip( 2 * (x - q01) / (q99 - q01) - 1,  -1, +1 )
역정규화 : x      = (x_norm + 1) / 2 * (q99 - q01) + q01
```


- `q01` / `q99` 는 고정 상수가 아니라 **각 dataset 데이터에서 한 번 계산해 저장**한 값이다 — "절대값"도 raw min/max 도 아닌 "이상치를 잘라낸 데이터 기반 범위". 추론 때 같은 값으로 역정규화한다.
- 어느 dataset 통계로 역정규화할지가 출력 스케일을 정한다 — week6 의 `unnorm_key`. 자작 팔이면 가장 가까운 embodiment 통계(예: `bridge_orig`)를 쓰거나, fine-tune 후 자기 데이터로 q01/q99 를 새로 구한다.
- **gripper 예외**: gripper 는 보통 이미 0/1 이산값이라 정규화에서 제외되기도 한다 (dataset 통계의 normalization mask 로 해당 차원 skip). 정확한 동작은 코드에서 확인.


이게 22 embodiments 를 한 모델에 학습 가능한 핵심.


### 3.5 Action representation 비교 축 (관절각 / EE-delta / token)


같은 "action" 이라도 표현 방식이 다르고, 이게 VLA 구조 비교의 핵심 축이다:


| 표현 | 예시 | 쓰는 곳 |
|---|---|---|
| 관절각 (joint) | Franka 7-DoF joint velocity | embodiment 원본 |
| EE-delta (연속) | `[dx, dy, dz, rx, ry, rz, gripper]` | OpenX 표준화 후 (§3) |
| action token (이산) | 각 차원을 256 bin 으로 discretize 한 정수 토큰 | OpenVLA 가 Llama 로 출력 |


OpenVLA 는 연속 EE-delta 를 차원별 **256 bin 으로 나눠 Llama vocabulary 의 토큰으로 치환**(자주 안 쓰는 256개 토큰을 덮어씀)해 출력한다 (아키텍처 다이어그램상 위치는 week4 §5). 즉 week2 의 "action 도 토큰" 이 여기서 구체화된다 — 연속 제어량을 언어 모델이 생성할 수 있는 이산 토큰으로 바꾼 것. 이 비교 축(관절각 -> EE-delta -> token)이 week7 OpenVLA 블로그의 "남이 안 짚는 각도" 재료가 된다.


§3 의 EE-delta 값(dx 등)은 +0.017 같은 연속 실수다. 그런데 OpenVLA 의 본체인 Llama 는 원래 단어(토큰)를 출력하는 언어 모델이라 소수점 숫자를 직접 뱉도록 만들어지지 않았다. 그래서 연속값을 정수 토큰으로 바꾸는 트릭을 쓴다.


- **이산화**(discretize): 각 차원의 값 범위를 256 칸(bin)으로 잘라 "몇 번째 칸"인지 정수로 표현. 연속적인 온도(23.48...도)를 "0-40도를 256칸으로 나눈 149번 칸"처럼 정수 하나로 바꾸는 것과 같다. 정밀도는 약간 잃지만 정수가 된다.
- **vocab trick**: 이 256 개 칸을 Llama 단어 사전에서 거의 안 쓰는 단어 256 개에 덮어씌운다. 그러면 Llama 입장에선 action 출력이 "특수 단어 7개를 말하는 것"과 같아져, 추가 구조 없이 기존 단어 생성 능력으로 action 을 낸다.
- **de-tokenize**: 추론 때는 거꾸로 토큰(정수 칸) -> 연속값(dx 등)으로 복원한다.


정리하면 세 표현은 경쟁이 아니라 변환 단계다: 제각각인 로봇 원본(관절각) -> EE-delta 로 통일(§3) -> 토큰으로 바꿔 Llama 가 출력(§3.5) -> 토큰을 연속값으로 복원해 로봇에 전달.


> **이식 관점 (열린 질문)**: 위 표는 전부 팔 기준이다. 이동 로봇이면 action 이 base 속도(선속도 + 각속도)가 되어 EE-delta 매핑이 통째로 달라진다. 이 갭은 Phase 4 demo (week9) 의 인터페이스 정리와 2026.11 재평가(부록 D)의 입력이다. 지금 결론 낼 질문이 아니라 열어두는 질문.


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


| 지표 | 의미 | 산출물 v3 에서 측정 |
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


**Q6. OpenVLA 가 출력하는 action 은 관절각/EE-delta/token 중 무엇이며 어떻게 만들어지나?**
> **action token**(이산). 연속 EE-delta `[dx,dy,dz,rx,ry,rz,gripper]` 를 차원별 256 bin 으로 discretize 한 뒤 Llama vocabulary 토큰으로 치환해 출력하고, 사용 시 de-tokenize 로 다시 연속값이 된다 (week2 RT-2 의 vocab trick 과 동일, 아키텍처 위치는 week4 §5). 관절각은 embodiment 원본, EE-delta 는 OpenX 표준화 단계.


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
6. **action 표현 3층위**: 관절각(원본) → EE-delta(OpenX 표준화) → action token(256 bin 이산, OpenVLA 출력). 이동 로봇이면 base 속도로 매핑이 달라짐(열린 질문, week9).


---


- 이전: [Week 4 - OpenVLA 1회독 + Architecture](../week4/README.md)


다음: [Week 6 - HuggingFace 모델 카드 + 환경 셋업](../week6/README.md)
