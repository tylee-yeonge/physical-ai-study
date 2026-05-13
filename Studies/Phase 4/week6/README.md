# Week 6: OpenVLA HuggingFace 모델 카드 + 환경 셋업


> **이번 주 목표**: OpenVLA 의 HuggingFace 모델 카드를 정독하고, RTX 4070 12GB 환경에서 4-bit quantization 으로 inference 실행까지 가져간다. (week 7 블로그 + week 8~ ROS2 demo 의 사전 준비)
> **예상 시간**: 8-10시간
> **핵심 질문**: "OpenVLA inference 가 내 환경에서 동작하는가? 한 frame 의 latency 는 몇 ms 인가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | 환경 | `requirements.txt` | bitsandbytes, accelerate 추가 |
| 2 | HuggingFace 모델 카드 정독 | https://huggingface.co/openvla/openvla-7b | usage / VRAM / license |
| 3 | 4-bit quantization 셋업 | `PRACTICE.md` 1 | bitsandbytes nf4 |
| 4 | 첫 inference 실행 | `PRACTICE.md` 2 | mock image + instruction |
| 5 | latency 측정 | `PRACTICE.md` 3 | 첫 latency 데이터 (이게 산출물 #4 의 기초) |
| 6 | 퀴즈 | quiz_easy / quiz_medium | quantization / inference 흐름 |


---


## 시작하기 전에 — 환경 점검


```bash
# 사전 확인
nvidia-smi # RTX 4070, VRAM 11~12 GB available 확인
nvcc --version # CUDA 11.8 이상
python --version # 3.10+
```


```bash
# conda 환경 (week 1 그대로)
conda activate phase4


# bitsandbytes 설치 (4-bit quantization 핵심)
pip install bitsandbytes>=0.43.0 accelerate>=0.30.0
```


---


## 핵심 개념


### 1. HuggingFace 모델 카드의 핵심 정보


OpenVLA 의 HuggingFace 페이지에서 반드시 확인할 5 가지:


| 항목 | OpenVLA 의 값 | 본 로드맵에서의 의미 |
|---|---|---|
| Model size | 7B parameter | RTX 4070 4-bit 가능 |
| License | MIT (코드) / Llama 2 (weights) | 상업적 사용 시 Llama 2 라이선스 확인 |
| Required VRAM | fp16 ~ 14GB, int4 ~ 5GB | 4070 12GB → 4-bit 필수 |
| Inference latency | ~ 100ms (RTX 4090 fp16) | 4070 + 4-bit 시 ~ 150ms |
| Usage 예시 | `transformers.AutoModelForVision2Seq` | inference 코드의 진입점 |


### 2. 4-bit Quantization (bitsandbytes nf4)


```
fp16 weight -> int4 (4-bit normalized float, nf4)
4 byte 가중치 -> 0.5 byte 가중치
                    ↓
                메모리 1/8
                속도 ~ 1.5~2x 빠름
                정확도 ~ 1~2%p 손실
```


핵심 사용 코드:


```python
from transformers import BitsAndBytesConfig


bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,
    bnb_4bit_quant_type='nf4',
    bnb_4bit_use_double_quant=True,
    bnb_4bit_compute_dtype=torch.float16,
)


model = AutoModelForVision2Seq.from_pretrained(
    'openvla/openvla-7b',
    quantization_config=bnb_config,
    device_map='auto',
    torch_dtype=torch.float16,
)
```


### 3. inference 흐름


```
1. Image preprocess (PIL → tensor, normalize, resize)
2. Text instruction tokenize
3. processor 가 image + text 를 model 입력 형식으로 결합
4. model.predict_action() 호출 (OpenVLA 의 wrapper)
5. action ndarray 반환 [dx, dy, dz, rx, ry, rz, gripper]
```


`predict_action()` 은 OpenVLA 의 custom method. 내부적으로:
- vision encoder forward
- LM decoder generate (greedy or sampled)
- action token de-tokenize


### 4. Latency 측정의 중요성


본 로드맵의 **산출물 #4 결정타** 의 핵심 측정 지표가 latency 다. 이번 주에 첫 데이터를 확보:


```
1 frame inference 의 component:
  - image preprocess : ~ 10~30 ms (CPU)
  - vision encoder forward : ~ 20~40 ms
  - LM decoder generate : ~ 60~120 ms (가장 큼)
  - action de-tokenize : ~ 1 ms
  -----------------------------------------
  total : ~ 100~200 ms
```


> LM decoder 의 시간이 압도적. 이게 줄어들면 (speculative decoding 등) latency 전체가 좋아짐.


### 5. unnormalize_key 의 의미


OpenVLA `predict_action()` 호출 시 `unnormalize_key` 인자가 있다:


```python
action = model.predict_action(
    image=...,
    instruction=...,
    unnormalize_key="bridge_orig", # <- 이게 핵심
)
```


이 key 는 OpenX-Embodiment 의 어떤 dataset 의 action normalization 통계로 de-normalize 할지 지정. 자작 팔의 경우 가까운 embodiment (예: "bridge_orig") 를 사용하거나, fine-tune 후 자체 통계 사용.


| key 예시 | 의미 |
|---|---|
| `bridge_orig` | WidowX (Bridge dataset) 의 normalization |
| `fractal20220817_data` | Google RT-1 의 normalization |
| `dlr_sara_pour_converted_externally_to_rlds` | DLR pour task |
| (custom) | LoRA fine-tune 후 본인 통계 사용 |


### 6. inference 시 자주 발생하는 에러


| 에러 | 원인 | 해결 |
|---|---|---|
| OOM (Out of Memory) | 4-bit 안 쓰거나 batch 큼 | bitsandbytes 적용 / batch=1 |
| missing flash_attn | flash attention 미설치 | `pip install flash-attn` (선택) |
| Image input shape mismatch | preprocess 누락 | processor 사용 권장 |
| TypeError on predict_action | OpenVLA 의 custom method | repo 의 inference 스크립트 참고 |


### 7. inference 코드의 표준 형태


```python
import torch
from PIL import Image
from transformers import AutoModelForVision2Seq, AutoProcessor


# 모델 로드 (4-bit)
processor = AutoProcessor.from_pretrained("openvla/openvla-7b", trust_remote_code=True)
vla = AutoModelForVision2Seq.from_pretrained(
    "openvla/openvla-7b",
    attn_implementation="eager", # flash_attn 없을 때
    torch_dtype=torch.float16,
    low_cpu_mem_usage=True,
    trust_remote_code=True,
    quantization_config=bnb_config,
)


# 입력
image = Image.open("test.jpg")
instruction = "pick up the can"
prompt = f"In: What action should the robot take to {instruction}?\nOut:"


# 추론
inputs = processor(prompt, image).to("cuda:0", dtype=torch.float16)
action = vla.predict_action(**inputs, unnormalize_key="bridge_orig", do_sample=False)
```


### 8. 본 로드맵의 다음 5주 흐름 (week 8-12)


이번 주의 inference 셋업이 끝나면:


- **week 7**: OpenVLA 블로그 1편 작성
- **week 8**: HuggingFace inference 셋업 (이번 주 결과 + 안정화)
- **week 9**: inference 입출력 인터페이스 정리
- **week 10**: ROS2 패키지 골격
- **week 11**: image subscribe → inference → action publish
- **week 12**: Rerun 시각화 + 1분 영상


---


## 자체 점검


**Q1. OpenVLA 를 RTX 4070 12GB 에서 띄우려면 어떤 quantization?**
> 4-bit nf4 (bitsandbytes). 7B fp16 은 14GB, int4 는 ~ 5GB.


**Q2. `predict_action()` 의 `unnormalize_key` 의 역할은?**
> OpenX-Embodiment 의 어떤 dataset 의 action normalization 통계로 de-normalize 할지 지정. 자작 팔에는 "bridge_orig" (WidowX) 가 가장 가까움.


**Q3. inference 의 가장 큰 latency component 는?**
> LM decoder generate (~60-120ms). 다음이 vision encoder (~20-40ms). preprocess / de-tokenize 는 미미.


**Q4. 첫 inference 가 두번째 inference 보다 느린 이유는?**
> Warm-up. CUDA kernel JIT, KV cache 초기화, model weights GPU 로드 등. 첫 1-2번 inference 는 측정에서 제외하고 평균.


**Q5. OpenVLA inference 시 do_sample=False 의 의미?**
> Greedy decoding 사용 (deterministic action). robot 제어에서는 일관성이 중요하므로 보통 greedy. do_sample=True 는 다양성 필요 시.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. OpenVLA HuggingFace 모델 카드 정독 + 5가지 항목 확인
2. `practice_first_inference.py` - 첫 inference 실행 (4-bit)
3. `practice_latency_measure.py` - 100 회 inference latency 통계
4. 발생 에러 + 해결 기록 (`~/phase4_notes/week6/errors_log.md`)
5. quiz_easy / quiz_medium 풀기


### 다음 주 (week 7) 준비
- week 6 의 latency 측정 결과 + RT-2 블로그 (week 3) 와 비교
- OpenVLA 블로그 outline 작성 시작


---


## 이번 주 핵심 요약


1. **4-bit quantization (bitsandbytes nf4) 필수** RTX 4070 12GB 환경.
2. **`predict_action()` + `unnormalize_key`**: OpenVLA 의 custom inference API.
3. **Latency ~ 100-200ms**: LM decoder 가 대부분.
4. **첫 데이터 확보**: 산출물 #4 의 latency 측정의 baseline.
5. **다음 주부터 블로그 + ROS2 demo**: 본격적 통합 시작.


---


- 이전: [Week 5 - OpenX-Embodiment + Fine-tuning](../week5/README.md)


다음: [Week 7 - OpenVLA 블로그 1편 작성](../week7/README.md)
