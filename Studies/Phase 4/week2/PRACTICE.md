# Week 2 실습: SentencePiece + Action Tokenization + Loss 시뮬레이션


> **실습 목표**: VLM 의 토크나이저를 직접 다루고, action token 의 추출/복원 사이클을 코드로 검증한다.
> **예상 시간**: 6-8시간


---


## 환경 설정


```bash
conda activate phase4
pip install -r requirements.txt
# 추가: sentencepiece, transformers (tokenizer 만), torch (loss 계산용)
```


---


## 실습 1: SentencePiece 토크나이저 사용


**파일명**: `practice_sentencepiece.py`


```python
"""
실습 1: SentencePiece 토크나이저로 직접 토큰화 / 복원
목적: VLM 의 입력/출력이 token ID sequence 임을 손에 익힌다.
"""
from transformers import AutoTokenizer


print("=" * 60)
print("실습 1: SentencePiece 토크나이저 사용")
print("=" * 60)


# -- 1-1. 토크나이저 로드 --
# PaLI-X 의 정확한 vocab 은 공개되지 않았으므로 비슷한 다국어 모델로 대체
tokenizer = AutoTokenizer.from_pretrained("google/flan-t5-base")
print(f"\n[1-1] Tokenizer info")
print(f"Vocab size: {tokenizer.vocab_size}")
print(f"Special tokens: {tokenizer.special_tokens_map}")


# -- 1-2. 텍스트 -> token ID --
text = "pick up the can"
ids = tokenizer.encode(text)
tokens = tokenizer.convert_ids_to_tokens(ids)
print(f"\n[1-2] Encode")
print(f"Text : '{text}'")
print(f"IDs : {ids}")
print(f"Tokens : {tokens}")


# -- 1-3. token ID -> 텍스트 --
recovered = tokenizer.decode(ids)
print(f"\n[1-3] Decode")
print(f"Recovered: '{recovered}'")


# -- 1-4. 한 단어가 여러 토큰으로 나뉘는 사례 --
for word in ["supersonic", "anthropomorphic", "tokenization", "Pyongyang"]:
    ids = tokenizer.encode(word, add_special_tokens=False)
    tokens = tokenizer.convert_ids_to_tokens(ids)
    print(f"'{word}' -> {tokens} (n_tokens={len(ids)})")


# -- 1-5. 빈도 낮은 token 의 ID 분포 확인 (RT-2 가 마지막 256개를 쓰는 이유) --
print("\n[1-5] Vocab 의 마지막 부분 token 확인 (RT-2 가 재사용한 범위)")
last_n = 10
last_tokens = []
for tid in range(tokenizer.vocab_size - last_n, tokenizer.vocab_size):
    t = tokenizer.convert_ids_to_tokens(tid)
    last_tokens.append((tid, t))
    print(f"ID {tid:>6}: '{t}'")


print("\n -> 마지막 token 들은 사용 빈도가 낮은 sub-word.")
print("RT-2 는 이런 'rare token' 들을 action bin 으로 재사용한다.")


print("\n 실습 1 완료!")
```


**실행**:
```bash
python practice_sentencepiece.py
```


**확인 포인트**:
- "pick up the can" 이 4-6 개 토큰으로 분해됨
- 흔하지 않은 단어는 더 많은 sub-word 로 분해
- vocab 의 마지막 token 들은 의미 없는 부분 문자열


---


## 실습 2: Action Token 추출/복원 사이클


**파일명**: `practice_action_token_simulation.py`


```python
"""
실습 2: continuous action -> token ID -> 다시 continuous 까지의 사이클
목적: RT-2 의 inference / 학습 시 action 의 표현 방식을 코드로 본다.
"""
import numpy as np


print("=" * 60)
print("실습 2: Action Token 추출/복원 사이클")
print("=" * 60)


# -- 2-1. Vocab / Action 설정 --
VOCAB_SIZE = 256000
N_BIN = 256
ACTION_TOKEN_START = VOCAB_SIZE - N_BIN


action_dims = ['dx', 'dy', 'dz', 'rx', 'ry', 'rz', 'gripper']
a_min = np.array([-0.10, -0.10, -0.10, -np.pi, -np.pi, -np.pi, 0.0])
a_max = np.array([ 0.10, 0.10, 0.10, np.pi, np.pi, np.pi, 1.0])


print(f"\n[2-1] 설정")
print(f"VOCAB_SIZE : {VOCAB_SIZE}")
print(f"ACTION_TOKEN_START : {ACTION_TOKEN_START}")
print(f"Action dims : {action_dims}")




# -- 2-2. discretize / token_map / de-tokenize 함수 --
def discretize(action, low, high, n_bins):
    normalized = (np.clip(action, low, high) - low) / (high - low)
    return np.minimum((normalized * n_bins).astype(int), n_bins - 1)




def to_token_ids(bin_idx, start):
    return bin_idx + start




def from_token_ids(token_ids, start):
    return np.asarray(token_ids) - start




def de_discretize(bin_idx, low, high, n_bins):
    return low + (bin_idx + 0.5) / n_bins * (high - low)




# -- 2-3. 학습 시: continuous action -> token sequence --
print("\n[2-3] 학습 시: continuous action -> token sequence")
original_action = np.array([0.05, -0.03, 0.02, 0.5, -1.2, 0.0, 0.9])
bin_idx = discretize(original_action, a_min, a_max, N_BIN)
token_ids = to_token_ids(bin_idx, ACTION_TOKEN_START)


print(f"원본 action : {original_action}")
print(f"bin index : {bin_idx}")
print(f"output token ID : {token_ids.tolist()}")




# -- 2-4. inference 시: token sequence -> continuous action --
print("\n[2-4] inference 시: token sequence -> continuous action")
recovered_bin = from_token_ids(token_ids, ACTION_TOKEN_START)
recovered_action = de_discretize(recovered_bin, a_min, a_max, N_BIN)


print(f"output token ID : {token_ids.tolist()}")
print(f"복원된 bin index : {recovered_bin.tolist()}")
print(f"복원된 action : {np.round(recovered_action, 5)}")


err = recovered_action - original_action
print(f"\n Quantization 오차 :")
for i, name in enumerate(action_dims):
    unit = "m" if i < 3 else ("rad" if i < 6 else "")
    print(f"{name:7s}: {err[i]:+.5f} {unit}")




# -- 2-5. 통계적 quantization 오차 분석 --
print("\n[2-5] 통계적 quantization 오차 (10000 random action)")
np.random.seed(42)
n_samples = 10000
errors = []
for _ in range(n_samples):
    a = np.random.uniform(a_min, a_max)
    b = discretize(a, a_min, a_max, N_BIN)
    a_recovered = de_discretize(b, a_min, a_max, N_BIN)
    errors.append(a_recovered - a)
errors = np.array(errors)


print(f"Mean error (per dim) : {np.round(errors.mean(axis=0), 6)}")
print(f"Max error (per dim) : {np.round(np.abs(errors).max(axis=0), 5)}")
print(f"RMS error (per dim) : {np.round(np.sqrt((errors**2).mean(axis=0)), 5)}")


print("\n -> mean 은 0 근처 (bin 중심값을 쓰는 이유)")
print("-> max 는 step/2 근처 (bin 의 가장자리 case)")
print("-> RMS 는 step / sqrt(12) (uniform 분포의 표준편차 공식)")


print("\n 실습 2 완료!")
```


**실행**:
```bash
python practice_action_token_simulation.py
```


**확인 포인트**:
- Mean quantization error 가 거의 0
- Max error 가 약 step/2
- RMS error 가 약 step/sqrt(12) (이론값)


---


## 실습 3: Co-fine-tuning Loss 시뮬레이션


**파일명**: `practice_loss_simulation.py`


```python
"""
실습 3: web sample 과 robot sample 의 loss 가 어떻게 합산되는지 시뮬레이션
목적: co-fine-tuning 시 두 종류 데이터가 같은 loss 로 학습된다는 것을 확인.
"""
import torch
import torch.nn.functional as F
import numpy as np


print("=" * 60)
print("실습 3: Co-fine-tuning Loss 시뮬레이션")
print("=" * 60)


torch.manual_seed(42)


# -- 3-1. Mock VLM 의 출력 (logits) 생성 --
VOCAB_SIZE = 1000 # 실제는 256K, 여기선 축약
SEQ_LEN = 10
BATCH = 4


# Mock 모델 출력: [batch, seq_len, vocab]
fake_logits = torch.randn(BATCH, SEQ_LEN, VOCAB_SIZE)


# Target token sequence
fake_target = torch.randint(0, VOCAB_SIZE, (BATCH, SEQ_LEN))


print(f"\n[3-1] Mock 출력")
print(f"logits shape : {fake_logits.shape} [B, L, V]")
print(f"target shape : {fake_target.shape} [B, L]")




# -- 3-2. standard next-token-prediction loss --
print("\n[3-2] Standard cross-entropy loss")
loss_per_token = F.cross_entropy(
    fake_logits.reshape(-1, VOCAB_SIZE),
    fake_target.reshape(-1),
    reduction='none',
)
loss_per_token = loss_per_token.reshape(BATCH, SEQ_LEN)
print(f"Per-token loss shape : {loss_per_token.shape}")
print(f"Per-token loss (sample 0): {loss_per_token[0].numpy().round(3)}")




# -- 3-3. Web sample vs Robot sample loss --
# 가정: batch 4 개 중 처음 3 개는 web, 마지막 1 개는 robot
# (mixture ratio 75% web : 25% robot)
print("\n[3-3] Web vs Robot sample 의 loss")
web_loss = loss_per_token[:3].mean()
robot_loss = loss_per_token[3].mean()
combined_loss = loss_per_token.mean()


print(f"Web loss (3 sample) : {web_loss.item():.4f}")
print(f"Robot loss (1 sample) : {robot_loss.item():.4f}")
print(f"Combined loss : {combined_loss.item():.4f}")


print("\n -> 두 종류 sample 의 loss 가 동일한 cross-entropy.")
print("RT-2 는 batch 단위로 averaged 된 loss 로 backprop.")




# -- 3-4. Action token 만 loss 에 포함하려면 (선택적 mask) --
print("\n[3-4] Action token 만 loss 에 (mask 사용)")
# Robot sample 에서 마지막 7 token 이 action 이라고 가정
ACTION_LEN = 7
mask = torch.zeros_like(fake_target, dtype=torch.float)
mask[3, -ACTION_LEN:] = 1.0 # robot sample (idx 3) 의 마지막 7 token 만


masked_loss = (loss_per_token * mask).sum() / mask.sum().clamp(min=1)
print(f"Masked loss (robot action 만): {masked_loss.item():.4f}")


print("\n -> 이 방식이 'action 만 학습' 하고 싶을 때 쓸 수 있음.")
print("단 RT-2 는 mask 없이 standard loss 그대로 씀 (논문 Sec 3.3).")




# -- 3-5. Data mixture 비율의 효과 시뮬레이션 --
print("\n[3-5] Data mixture 비율 별 loss 비교")
ratios = [(1.0, 0.0), (0.8, 0.2), (0.5, 0.5), (0.0, 1.0)]
fake_web_loss = 2.5
fake_robot_loss = 4.0 # robot data 는 새로 배우는 거라 초기 loss 높다 가정


print("(Web sample 평균 loss = 2.5, Robot sample 평균 loss = 4.0 가정)")
print()
print(f"{'Web:Robot':<15} {'Combined loss':>20}")
print(f"{'-'*15:<15} {'-'*20:>20}")
for w, r in ratios:
    combined = w * fake_web_loss + r * fake_robot_loss
    print(f"{w:.1f} : {r:.1f} {combined:>20.4f}")


print("\n -> robot 비율이 클수록 combined loss 가 높아짐.")
print("하지만 robot 비율이 너무 작으면 robot task 학습이 안 됨.")
print("RT-2 는 ~ 20% robot 으로 균형을 맞춤.")


print("\n 실습 3 완료!")
```


**실행**:
```bash
python practice_loss_simulation.py
```


---


## 실습 4: 한 페이지 노트 산출


**파일명**: `~/phase4_notes/week2/action_token_one_page.md`


이번 주의 산출물은 **week 3 블로그의 절반** 이 될 수 있는 한 페이지 노트:


```markdown
# RT-2 Action Tokenization & Co-fine-tuning - 한 페이지


## 1. 핵심 한 줄
> Action 도 token. VLM 의 vocab 마지막 256개를 action bin 으로 재사용.


## 2. 수식 3개
1. Discretize: b = min( floor((a - a_min) / (a_max - a_min) * 256), 255 )
2. Token map: t = ACTION_TOKEN_START + b
3. De-tokenize: a = a_min + (b + 0.5) / 256 * (a_max - a_min)


## 3. Quantization step 수치 (실습 결과)
- dx, dy, dz : ~ 0.78 mm
- rx, ry, rz : ~ 1.40 deg
- gripper : ~ 0.004 (256단계)


## 4. Co-fine-tuning mixture
| 데이터 | 비율 |
|---|---|
| WebLI (image-caption) | 50% |
| OCR / VQA | 30% |
| Robot (RT-1 dataset) | 20% |


## 5. Loss 의 일관성
모든 sample 이 standard next-token-prediction CE loss 로 학습.
Robot sample 에서 output 의 처음 7~11 token 이 action 으로 해석.


## 6. 학습 시 자주 놓치는 포인트
- vocab 마지막 256개를 쓰는 이유: rare token 이라 원 의미 손실 최소
- robot data 만 fine-tune 하면 catastrophic forgetting
- mixture 비율은 absolute size 가 아닌 'sample 등장 빈도' 임


## 7. 본 로드맵 관점
- Phase 7 산출물 v3 에서 OpenVLA (동일한 discrete action token) 사용
- Quantization step 수치를 면접에서 'fine motion 한계' 의 정량적 근거로 인용
```


---


## 실습 체크리스트


- [ ] `practice_sentencepiece.py` 실행, vocab 마지막 token 의 sparsity 확인
- [ ] `practice_action_token_simulation.py` 실행
  - [ ] discretize -> token_id -> de-tokenize 사이클 동작 확인
  - [ ] quantization 오차 통계 (mean ~ 0, RMS ~ step/sqrt(12)) 확인
- [ ] `practice_loss_simulation.py` 실행
  - [ ] web/robot loss 가 같은 CE 임을 확인
  - [ ] mixture ratio 별 combined loss 변화 확인
- [ ] `action_token_one_page.md` 노트 산출
- [ ] quiz_easy / quiz_medium 풀고 solutions 확인
- [ ] git commit


---


## 참고 자료


- [RT-2 paper Section 3.2 / 3.3](https://arxiv.org/abs/2307.15818)
- [SentencePiece paper](https://arxiv.org/abs/1808.06226)
- [HuggingFace Tokenizers tutorial](https://huggingface.co/docs/transformers/tokenizer_summary)
- [Andrej Karpathy: Tokenizer 강의](https://www.youtube.com/watch?v=zduSFxRajkE)
