# Week 2: Co-fine-tuning + Action tokenization 정밀 분석


> **이번 주 목표**: "Action 도 token 이다" 의 의미를 손으로 계산할 수 있는 수준까지 내려간다. Co-fine-tuning 의 데이터 비율 / loss / 학습 step 을 한 페이지에 정리.
> **예상 시간**: 10~12시간 (Sec 3.2~3.3 재정독 4h + 토크나이저 실습 3h + 노트 정리 3h)
> **핵심 질문**: "VLM 의 vocab 의 마지막 256 개를 action 으로 쓰는 게 정말로 가능한가? 학습 시 web data 의 마지막 token 들과 충돌하지 않는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | 환경 (week1 그대로) | `requirements.txt` | numpy / sentencepiece 추가 |
| 2 | RT-2 Sec 3.2 재정독 | RT-2 PDF | Action Tokenization |
| 3 | RT-2 Sec 3.3 재정독 | RT-2 PDF | Co-fine-tuning |
| 4 | 토크나이저 실습 | `PRACTICE.md` 1~3 | SentencePiece + action token 시뮬레이션 |
| 5 | 퀴즈 (개념) | `quiz_easy.py` | tokenization / data mixture 개념 |
| 6 | 퀴즈 (코드/수학) | `quiz_medium.py` | loss 비율 계산 / vocab overlap 분석 |
| 7 | 노트 정리 | `PRACTICE.md` 4 | "한 페이지 Action Tokenization" 노트 |


---


## 시작하기 전에 — 이번 주의 한 가지 핵심 질문


> "VLM 의 vocab 의 마지막 256 개 token 을 action 으로 재사용하면, 원래 그 token 들이 web data 에서 가지던 의미는 어떻게 되는가?"


답을 미리 말하면: **원래 의미를 덮어쓴다**. 그 256 개 token 은 RT-2 fine-tune 이후 "action discrete bin" 만 의미하게 된다. 단, 학습 시:
- Web data sample 에서는 그 256 개 token 이 거의 등장하지 않도록 vocab 의 빈도가 낮은 끝부분을 선택 (low-frequency token 을 재사용)
- Robot data sample 에서만 그 256 개 token 이 등장하도록 sequence 구조 보장


이 한 줄을 이해하는 게 이번 주의 핵심.


---


## 핵심 개념 자세히 알아보기


### 1. Tokenization 의 정확한 정의 복습


VLM 의 입력/출력은 모두 **token sequence** 다. Token 은 vocabulary 안의 한 entry. 예시:


```
Vocabulary (SentencePiece, PaLI-X):
  Token ID 0 : <pad>
  Token ID 1 : <bos>
  Token ID 2 : <eos>
  Token ID 3 : "the"
  Token ID 4 : "_"
  Token ID 5 : "a"
  ...
  Token ID 255743 : "_supersonic"
  Token ID 255744 : <action_bin_0> <-- RT-2 가 재해석한 token
  ...
  Token ID 255999 : <action_bin_255> <-- RT-2 가 재해석한 token
```


**핵심**: VLM 입장에서 `<action_bin_5>` 는 그냥 ID 가 255749 인 token 일 뿐이다. "action" 이라는 특별한 의미를 모른다. 학습 데이터에서 그 token 이 어떤 context 에서 나오는지에 따라 그 token 의 임베딩 벡터가 적응된다.


### 2. SentencePiece / BPE 토크나이저의 동작


```
원문 : "pick up the can"
       v
SentencePiece:
       v
Token : ["_pick", "_up", "_the", "_can"]
       v
Token ID: [3421, 891, 3, 1247]
```


이 token ID 들이 VLM 의 입력. 출력도 token ID sequence.


RT-2 는 학습 시 **robot data sample** 의 sequence 를 다음과 같이 구성한다:


```
Input (image + text):
  <image>...</image> "pick up the can. action: "


Output target:
  "<action_bin_135> <action_bin_201> <action_bin_128> <action_bin_64> <action_bin_128> <action_bin_99> <action_bin_255> <eos>"
   ^---- dx ^---- dy ^---- dz ^---- rx ^---- ry ^---- rz ^---- gripper
```


이때 출력 target 의 token ID 는 모두 [255744, 255999] 범위. Cross-entropy loss 는 이 256 개 token 위에서만 계산되도록 mask 처리 가능 (또는 standard next-token-pred loss 그대로 사용 - 어느 쪽이든 동작).


### 3. Action Tokenization 의 정확한 수식


연속 action $a_i \in [a_{i,\min}, a_{i,\max}]$ 에 대해:


```
1. Normalize: n = (a_i - a_min) / (a_max - a_min) in [0, 1]
2. Discretize: b = min( floor(n * 256), 255 ) in [0, 255]
3. Token map: t = ACTION_TOKEN_START + b in [255744, 255999]
```


de-tokenize (inference 시):


```
1. Token map^-1: b = t - ACTION_TOKEN_START
2. De-normalize: a_i = a_min + (b + 0.5) / 256 * (a_max - a_min)
```


`+0.5` 가 들어가는 이유: bin 의 **중심값** 을 사용해 quantization 오차를 평균적으로 0 으로 만들기 위함.


### 4. Co-fine-tuning 의 데이터 mixture 표


RT-2 논문 Table 4 (또는 비슷한 표) 의 데이터 비율 (논문에 따라 약간씩 다름, 일반적 패턴):


| 데이터 종류 | 비율 (대략) | 예시 |
|---|---|---|
| WebLI (image-caption) | 50% | "A red apple on a wooden table." |
| OCR / VQA | 30% | Q: "What color is the apple?" A: "Red" |
| Robot trajectory | 20% | <image> "pick up the can. action: <action_bin_...> ..." |


학습 step:
- 매 batch 마다 위 비율로 sample 추출
- Loss = standard cross-entropy on next-token-prediction
- 다른 학습 trick 거의 없음 (이게 핵심: 표준 LM 학습 그대로)


### 5. Catastrophic Forgetting 의 정확한 메커니즘


만약 robot data 만으로 fine-tune 하면:


```
초기 weight (web 학습 완료):
  word embedding "red" 의 의미 -> [..., 0.43, -0.12, 0.71, ...]
  (web data 의 수많은 'red' context 에서 학습된 분포)


Robot data 만으로 fine-tune (10000 step):
  word embedding "red" 의 의미 -> [..., 0.41, -0.10, 0.73, ...]
  거의 안 변함 (robot data 에 'red' 거의 없음)


  하지만 다른 일반 단어는?
  word embedding "physics" 의 의미 -> [..., random drift, ...]
  (robot data 에 'physics' 가 한 번도 안 나옴, gradient 가 작아도 drift 누적)
```


Web data 가 mix 되면:
- 일반 단어들의 embedding 이 web sample 에서 계속 reinforce
- robot-specific token (action bin 등) 만 새로 학습
- 양쪽 능력이 공존


### 6. Loss 계산 시 실수하기 쉬운 점


학습 loss 는 next-token-prediction:


```
L = -sum_{t} log p(token_t | token_<t)
```


이 loss 가 web sample 과 robot sample 에 **모두 같은 형태로** 적용된다는 게 중요.


- Web sample: caption 의 모든 token 에 대해 prediction
- Robot sample: instruction 뒤의 action token sequence 에 대해 prediction


이게 가능한 이유: 두 sample 모두 "이전 token 으로 다음 token 예측" 이라는 동일한 task 로 환원 가능.


### 7. Inference 시 action 추출


inference 시 흐름:


```
입력 image + "pick up the can. action: "
                                       ^ 여기까지 prompt
모델이 next token 생성 시작:
  step 1: token ID 255744 + b_dx
  step 2: token ID 255744 + b_dy
  ...
  step 7: token ID 255744 + b_grip
  step 8: <eos>


후처리:
  각 token ID - 255744 = bin index
  de-normalize -> continuous action
```


특별한 decoding 알고리즘 (greedy / beam search / temperature) 은 표준 LM 과 동일. RT-2 는 보통 **greedy** 또는 **temperature ~ 0** 으로 deterministic action 생성.


### 8. 다른 action representation 과의 비교


| 방식 | 장점 | 단점 | 사용 모델 |
|---|---|---|---|
| Discrete token (RT-2) | LM 표준 흐름 그대로, 다양한 action 분포 표현 가능 | quantization 오차 | RT-2, OpenVLA |
| Continuous regression head | quantization 없음 | LM 흐름과 단절, multi-modal 분포 어려움 | RT-1, BC-Z |
| Diffusion head | multi-modal 분포 표현, smooth | inference 느림 (multi-step denoising) | Diffusion Policy, π0 |
| Flow matching | diffusion 보다 빠름 | 비교적 새로움 | Helix, π0 |


> 본 로드맵의 Phase 7 산출물 #4 에서 OpenVLA (discrete token) 를 사용. Phase 4 의 분기 재평가 (2026.11) 에서 π0 / Helix 등으로 갱신될 수 있음.


---


## 한 페이지 요약


### 핵심 수식 3개


```
1. Discretize: b = min( floor((a - a_min) / (a_max - a_min) * 256), 255 )
2. Token map: t = ACTION_TOKEN_START + b
3. De-tokenize: a = a_min + (b + 0.5) / 256 * (a_max - a_min)
```


### 핵심 데이터 mixture


```
Mini-batch 비율:
  WebLI (image-caption) : 50%
  OCR / VQA / etc. : 30%
  Robot trajectory (RT-1 ds) : 20%
```


### 핵심 학습 흐름


```
모든 sample 을 "image+text -> output text" 의 표준 next-token-prediction 으로 환원.
Robot sample 에서 output text 의 처음 7~11 token 이 action 으로 해석됨.
별도 loss / 별도 head 없음.
```


### 핵심 한계


- Quantization step ~ 0.78 mm (dx), 1.4 deg (rx)
- Inference latency ~ 200ms (5Hz)
- Vocab 의 마지막 256 개 token 의 원 의미는 덮어써짐 (단 low-frequency token 이라 영향 최소)


---


## 자체 점검


**Q1. "Action 도 token 이다" 의 정확한 의미는?**
> VLM 의 vocabulary (예: 256000 개) 중 마지막 256 개 token 을 action discrete bin (0~255) 으로 재해석. 즉 별도의 action head 없이 VLM 의 표준 next-token-prediction 으로 action 도 생성 가능. 학습 시에도 표준 cross-entropy loss 그대로 사용.


**Q2. 왜 vocab 의 마지막 256 개를 선택하는가?**
> 그 token 들이 web data 에서 가장 빈도가 낮은 token (rare token) 이라, 원래 의미를 덮어써도 web 성능에 거의 영향이 없기 때문. 또한 low-frequency token 은 web pretraining 시 충분히 학습되지 못한 token 이라 새로 의미 할당이 용이.


**Q3. Co-fine-tuning 의 비율이 web 8 : robot 2 인 이유 정확히는?**
> Robot data 의 절대량이 web data 대비 압도적으로 적기 때문에 (130k episodes vs 수십억 image-text pair), 그 차이를 학습 시 sample 비율로 보정. 만약 1:1 로 잡으면 robot data 가 너무 자주 반복되어 overfit. 8:2 가 catastrophic forgetting 방지와 action 학습의 균형점.


**Q4. Inference 시 quantization 오차의 영향은?**
> dx step ~ 0.78mm 으로 cm 단위 manipulation 에는 충분. 단 sub-mm 정밀 조립 / 미세 force control 은 불가. 본 로드맵 Phase 7 의 산출물 #4 에서 이 quantization 오차를 직접 측정해 "양산 시점 비용" 의 증거로 사용 가능.


**Q5. RT-2 의 action representation 이 Diffusion Policy 대비 가지는 차이는?**
> RT-2 는 discrete categorical distribution (256 bin) 으로 action 표현 → multi-modal 분포 어느 정도 가능, 단 quantization 오차 있음. Diffusion Policy 는 continuous distribution → quantization 없음, multi-modal 표현 우수, 단 inference 가 multi-step denoising 으로 느림. π0 / Helix 같은 최근 모델은 flow matching 으로 둘의 단점 보완.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. RT-2 Section 3.2 (Action Tokenization) 정독 + 메모
2. RT-2 Section 3.3 (Co-fine-tuning) 정독 + 데이터 mixture 표 정리
3. `practice_sentencepiece.py` 실행 - SentencePiece 의 토큰화 직접 확인
4. `practice_action_token_simulation.py` 실행 - action token 추출/복원 시뮬레이션
5. `practice_loss_simulation.py` 실행 - mock data 로 cross-entropy 계산
6. quiz_easy / quiz_medium 풀기
7. "한 페이지 Action Tokenization" 노트 산출 (week 3 블로그의 절반)


### 다음 주 (week 3) 준비
- 블로그 플랫폼 선정 (Velog / Medium / 본 레포의 `Studies/Phase 4/blog/`)
- week 1, 2 의 reading note 와 한 페이지 노트 통합
- 블로그 1편 (RT-2) 의 초안 작성 시작 (week 3 의 중심 작업)


---


## 이번 주 핵심 요약


1. **Tokenization 의 본질**: VLM 의 vocab 마지막 256 개 token 을 action discrete bin 으로 재사용.
2. **Quantization 수식 3개**: discretize / token map / de-tokenize.
3. **Co-fine-tuning mixture**: WebLI 50% + OCR/VQA 30% + Robot 20%.
4. **Loss 의 일관성**: 모든 sample 이 표준 next-token-prediction 으로 환원.
5. **Diffusion 등 대안 비교**: discrete (RT-2) / continuous (RT-1) / diffusion (DP) / flow matching (π0).


---


- 이전: [Week 1 - RT-2 1회독 + Architecture](../week1/README.md)


다음: [Week 3 - RT-2 블로그 1편 작성](../week3/README.md)
