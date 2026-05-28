# Week 1 실습: RT-2 논문 정독 + Architecture Diagram 분해


> **실습 목표**: 논문을 닫고도 RT-2 의 입출력 / Architecture / 학습 흐름 / Emergent capability 를 한 페이지로 그릴 수 있게 한다.
> **예상 시간**: 6-8시간


---


## 환경 설정


Phase 4 전체 환경 구축 (Colab + 로컬 분업, 버전 매칭, ROS2 등) 은 [`../SETUP.md`](../SETUP.md) 에서 1회 수행한다. 본 week 1 에서 추가로 할 일은 아래 두 가지 뿐 — 코드 실행이 거의 없고 논문 정독 + 다이어그램이 중심이다.


```bash
# 1) week 1 venv (Phase 3 패턴 그대로; 가벼운 reading note 도구만)
bash pip_install.sh

# 2) 실습 노트용 작업 디렉토리
mkdir -p ~/phase4_notes/week1
cd ~/phase4_notes/week1
```


> week 1-7 은 로컬 GPU 가 거의 필요 없다. SETUP.md 의 GPU/Colab 셋업은 진입 시 한 번 점검만 해두고, 본격적인 활용은 week 8 (양자화 추론) 부터 시작된다.


---


## 실습 1: 논문 1회독 + Reading Note 템플릿 채우기


**파일명**: `rt2_reading_note.md` (직접 작성)


### 1-1. 논문 다운로드


```bash
# arxiv 에서 RT-2 논문 PDF 다운로드
mkdir -p ~/phase4_notes/papers
wget -O ~/phase4_notes/papers/rt2.pdf "https://arxiv.org/pdf/2307.15818.pdf"
```


### 1-2. Reading Note 템플릿 (직접 채우기)


`~/phase4_notes/week1/rt2_reading_note.md`:


```markdown
# RT-2 Reading Note


## 0. Meta
- 제목: RT-2: Vision-Language-Action Models Transfer Web Knowledge to Robotic Control
- 저자: Google DeepMind (2023)
- 분야: VLA / Robot Manipulation
- 읽은 날짜: 2026-09-XX
- 1회독 소요: __h __m


## 1. One-liner
> _____________________________________________________________


## 2. 핵심 contribution 3가지
1. _____________________________________________________________
2. _____________________________________________________________
3. _____________________________________________________________


## 3. Architecture (Figure 1 손그림)
[종이에 그린 그림을 사진으로 또는 ASCII 로 옮기기]


## 4. Action Tokenization (Section 3.2)
- Action 차원: ___
- Discretize bin 수: ___
- VLM vocab 의 어디를 재사용: ___
- 한 줄 설명: _______________________________________________


## 5. Co-fine-tuning (Section 3.3)
- 데이터 비율 (Web : Robot): ___ : ___
- 왜 robot only 안 되는가: __________________________________
- 학습 step 수 (대략): ___


## 6. Experiments (Section 4)
- Real-world 평가 대수: ___
- Baseline (RT-1, BC-Z 등) 대비 성공률 차이: ___%
- 가장 인상적인 사례: _________________________________________


## 7. Emergent Capability (Section 5)
사례 4가지를 한 줄씩:
1. _____________________________________________________________
2. _____________________________________________________________
3. _____________________________________________________________
4. _____________________________________________________________


## 8. Limitations (Section 6)
- Latency: ___ ms (RTX 4070 추정?: ___)
- closed-source 의 영향: ____________________________________
- Single-arm 한정: ___________________________________________


## 9. 본 로드맵 관점에서의 시사점
- 산출물 #4 (Real-to-Sim-to-Real) 에 어떤 영향: ______________
- 양산 SW 엔지니어 면접에서 강조할 포인트: _____________________


## 10. 추가로 읽어야 할 것
- _____________________________________________________________
```


### 1-3. 정독 페이스 가이드 (총 6시간)


| 시간 | 범위 | 목표 |
|---|---|---|
| 0:00 ~ 0:30 | Abstract + Sec 1 | One-liner + contribution 3 적기 |
| 0:30 ~ 1:30 | Sec 2 (Related Work) | RT-1 / Gato / Flamingo 와의 관계 표 1줄씩 |
| 1:30 ~ 3:00 | Sec 3 (Approach) | Architecture / Action Tokenization / Co-fine-tuning |
| 3:00 ~ 4:30 | Sec 4 (Experiments) | 결과 표를 한 장으로 요약 |
| 4:30 ~ 5:30 | Sec 5 (Emergent Cap.) | 4가지 사례 한 줄씩 |
| 5:30 ~ 6:00 | Sec 6 (Limitations) | 5가지 한계 노트 |


> 6시간 안에 다 안 끝나도 OK. 우선 Sec 1+3+5 만 끝내도 80% 도달. Sec 2 (Related Work) 는 마지막에 봐도 된다.


---


## 실습 2: Architecture Diagram 손으로 다시 그리기


**파일명**: `rt2_arch_diagram.txt` (ASCII 또는 사진)


### 2-1. 단계별 그리기


논문 Figure 1 을 보지 말고 (이미 본 후) 다음 흐름으로 직접 그려본다:


1. 왼쪽 위: RGB image 입력
2. 그 아래: Vision Encoder (ViT) → image token 196 개
3. 왼쪽 아래: Text instruction 입력
4. 그 아래: Text Tokenizer → text token N 개
5. 가운데: Concat 모듈
6. 오른쪽 위: Transformer Decoder
7. 오른쪽 아래: Output token sequence
8. 오른쪽 끝: De-tokenize → 7-DoF action


### 2-2. ASCII 버전 예시 (이미 README에 있음, 자기 손으로 다시 그릴 것)


```
            +---------+
            | RGB image| -> ViT -> 196 image tokens \
            +---------+ \
                                                      ----> [concat]
                                                     /
+-------------------------+ +-------------+ /
| "pick up the can" | -----> | SentPiece |
+-------------------------+ | text tokens |
                                   +-------------+
                                          |
                                          v
                            +----------------------+
                            | Transformer Decoder |
                            | (PaLI-X / PaLM core)|
                            +----------------------+
                                          |
                                          v
                            +----------------------+
                            | output token sequence|
                            | "1 128 91 ... <eos>" |
                            +----------------------+
                                          |
                                          v (de-tokenize)
                            +----------------------+
                            | 7-DoF action vector |
                            | [dx,dy,dz,rx,ry,rz,g]|
                            +----------------------+
```


### 2-3. 그림 위에 직접 메모 (필수)


자기가 그린 다이어그램 위에 다음을 적는다:


- Vision Encoder 옆: "ViT, 224x224 input, 14x14 patch, 196 tokens"
- Text Tokenizer 옆: "SentencePiece, vocab=256K"
- Decoder 옆: "PaLI-X 5B 또는 55B"
- Output token 옆: "256 bin per dim, 7 dims = 7 tokens"
- 화살표 옆: "input 60 token + output 11 token (action 7 + EOS 등)"


---


## 실습 3: Action Tokenization 의 수학 계산


**파일명**: `practice_action_tokenization.py`


```python
"""
실습 3: RT-2 의 Action Tokenization 정확히 계산하기


목적: "7-DoF continuous action 을 256 bin token 으로 양자화" 의 의미를
      수치로 직접 확인한다.
"""
import numpy as np


print("=" * 60)
print("실습 3: RT-2 Action Tokenization")
print("=" * 60)


# -- 3-1. Action 의 차원과 범위 가정 --
# RT-2 의 실제 action space (논문 Sec 3.2 + RT-1 paper 참고)
# 예시: end-effector delta pose (x,y,z) + axis-angle rotation + gripper
print("\n[3-1] Action space 정의")
action_names = ['dx', 'dy', 'dz', 'rx', 'ry', 'rz', 'gripper']
# 단위: meter (x,y,z), radian (r), [0,1] (gripper)
action_min = np.array([-0.1, -0.1, -0.1, -np.pi, -np.pi, -np.pi, 0.0])
action_max = np.array([ 0.1, 0.1, 0.1, np.pi, np.pi, np.pi, 1.0])
N_BIN = 256


print(f"Action dim: {len(action_names)}")
print(f"Dims: {action_names}")
print(f"Min : {action_min}")
print(f"Max : {action_max}")
print(f"Bin per dim: {N_BIN}")


# -- 3-2. Discretize (continuous -> bin index) --
def discretize(action, low, high, n_bins):
    """연속 action 을 [0, n_bins-1] 범위의 정수로 양자화"""
    clipped = np.clip(action, low, high)
    normalized = (clipped - low) / (high - low)
    return np.minimum((normalized * n_bins).astype(int), n_bins - 1)


def de_discretize(bin_idx, low, high, n_bins):
    """bin index 를 다시 continuous action 으로 복원 (bin 중심값 사용)"""
    return low + (bin_idx + 0.5) / n_bins * (high - low)


print("\n[3-2] 양자화 예시")
example_action = np.array([0.05, -0.03, 0.02, 0.5, -1.2, 0.0, 1.0])
bin_idx = discretize(example_action, action_min, action_max, N_BIN)
recovered = de_discretize(bin_idx, action_min, action_max, N_BIN)
err = recovered - example_action


print(f"원본 action : {example_action}")
print(f"bin index : {bin_idx}")
print(f"복원 action : {np.round(recovered, 4)}")
print(f"양자화 오차 : {np.round(err, 5)}")


# -- 3-3. Vocabulary 매핑 --
# RT-2 는 VLM vocab 의 마지막 256 개 토큰을 action 으로 재해석
VOCAB_SIZE = 256000 # PaLI-X 의 SentencePiece vocab (대략)
ACTION_TOKEN_START = VOCAB_SIZE - N_BIN # 마지막 256 개


print("\n[3-3] Vocabulary 매핑")
print(f"VLM vocab size : {VOCAB_SIZE}")
print(f"Action token start id : {ACTION_TOKEN_START}")
print(f"Action token id range : [{ACTION_TOKEN_START}, {VOCAB_SIZE - 1}]")
print(f"Example bin 128 -> token id {ACTION_TOKEN_START + 128}")


# -- 3-4. Output token sequence 구조 --
# 한 frame 의 RT-2 출력 token sequence
output_token_ids = [ACTION_TOKEN_START + b for b in bin_idx]
EOS_TOKEN_ID = 2 # 가정
output_token_ids.append(EOS_TOKEN_ID)


print("\n[3-4] 한 frame 의 output token sequence")
print(f"Sequence length : {len(output_token_ids)} (action 7 + EOS 1)")
print(f"Token IDs : {output_token_ids}")


# -- 3-5. 양자화 오차의 영향 --
print("\n[3-5] 양자화 오차의 직관")
print("dx 의 양자화 step :")
step_dx = (action_max[0] - action_min[0]) / N_BIN
print(f"range = {action_max[0] - action_min[0]:.4f} m, bin = {N_BIN}")
print(f"step = {step_dx*1000:.4f} mm")
print()
print("rx (rotation) 의 양자화 step :")
step_rx = (action_max[3] - action_min[3]) / N_BIN
print(f"range = {action_max[3] - action_min[3]:.4f} rad, bin = {N_BIN}")
print(f"step = {np.degrees(step_rx):.4f} deg")


print("\n[3-6] 질문: gripper (range 0~1) 의 256 bin 양자화")
print("quantization step 은 1/256 = {:.4f}".format(1/N_BIN))
print("=> gripper close/open 의 'fine motion' 이 어디서 한계인가?")


print("\n 실습 3 완료!")
```


**실행**:
```bash
python practice_action_tokenization.py
```


**예상 출력 핵심**:
- `dx` 의 step = 약 0.78 mm (cm 단위 manipulation 에 적당)
- `rx` (rotation) 의 step = 약 1.4 도 (대부분 OK, 정밀 조립은 어려움)
- gripper step = 약 0.004 (256 단계)


> 이 수치가 RT-2 의 "fine motion 한계" 의 정량적 근거. week 3 블로그에서 인용할 수 있다.


---


## 실습 4: 한 페이지 RT-2 다이어그램 노트 산출


**파일명**: `~/phase4_notes/week1/rt2_one_page.md`


이번 주 학습의 최종 산출물은 **한 페이지짜리 RT-2 요약 노트** 다. 형식 자유, 단 다음 8 가지 항목이 반드시 들어가야 한다:


1. **One-liner**: RT-2 가 무엇인가 (한 줄)
2. **Architecture 다이어그램**: 손그림 또는 ASCII (실습 2 결과물)
3. **Action Tokenization**: 256 bin / VLM vocab 재사용 / quantization step 수치
4. **Co-fine-tuning**: web : robot 비율 / 이유
5. **Emergent Capability 4 사례**: 한 줄씩
6. **Limitations 5 가지**: 한 줄씩 (latency / closed / etc.)
7. **본 로드맵 관점**: 산출물 #4 와의 연결 / 면접 포인트
8. **다음 주 (week 2) 시작 질문**: 적어도 1 개


### 산출물 권장 포맷


- 형식: Markdown 또는 PDF (Markdown 추천 → 나중에 블로그로 재활용 가능)
- 분량: A4 1 페이지에 들어가게 (글자 크기 조절)
- 위치: `~/phase4_notes/week1/rt2_one_page.md`


> 이 한 페이지가 week 3 (RT-2 블로그) 의 골격이 된다. 정성껏 작성하자.


---


## 실습 체크리스트


- [ ] RT-2 논문 1회독 완료 (대략 12 페이지)
- [ ] `rt2_reading_note.md` 의 빈칸 모두 채움
- [ ] Architecture diagram 손으로 다시 그려서 사진 또는 ASCII 로 저장
- [ ] `practice_action_tokenization.py` 실행 + 출력 수치 확인
  - [ ] dx 의 step 이 약 0.78 mm 임을 확인
  - [ ] rx 의 step 이 약 1.4 deg 임을 확인
- [ ] `rt2_one_page.md` 한 페이지 요약 산출
- [ ] quiz_easy.py / quiz_medium.py 풀고 solutions 확인
- [ ] 이번 주 reading note 를 git 에 commit
  - `git add ~/phase4_notes/week1/*`
  - `git commit -m "phase4 w1: rt2 reading note"`


---


## 참고 자료


### 필수
- [RT-2 paper (arXiv)](https://arxiv.org/abs/2307.15818)
- [RT-2 project page](https://robotics-transformer2.github.io/)


### 보조 (선택, 시간 여유 시)
- [RT-1 paper (arXiv)](https://arxiv.org/abs/2212.06817) - RT-2 의 직전 모델
- [PaLI-X paper](https://arxiv.org/abs/2305.18565) - RT-2 backbone 중 하나
- [PaLM-E paper](https://arxiv.org/abs/2303.03378) - RT-2 backbone 중 하나
- [DeepMind RT-2 blog post](https://www.deepmind.com/blog/rt-2-new-model-translates-vision-and-language-into-action) - 가벼운 개요


### Tokenization 사전 지식 (있으면 좋음)
- [Andrej Karpathy: Let's build the GPT Tokenizer](https://www.youtube.com/watch?v=zduSFxRajkE) - 2h 강의
- [SentencePiece paper](https://arxiv.org/abs/1808.06226) - PaLI-X 의 tokenizer


---


## 트러블슈팅


| 증상 | 원인 | 해결 |
|---|---|---|
| 논문이 너무 길어 막힘 | 한 번에 다 보려 함 | Sec 1+3+5 만 우선, Sec 2 (Related Work) 는 마지막에 |
| Architecture 가 이해 안 됨 | VLM 사전 지식 부족 | Phase 5 의 ViT/CLIP 자료 미리 훑기 (1-2시간) |
| Action tokenization 이 이해 안 됨 | tokenizer 의 개념 자체가 모호 | Karpathy 강의 30분만 보기 |
| 시간 부족 | Sec 1+3+5+reading_note 만 끝내도 OK | Sec 2/4 는 week 2 에 흡수 |
