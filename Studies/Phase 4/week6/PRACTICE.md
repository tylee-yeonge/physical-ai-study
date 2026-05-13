# Week 6 실습: OpenVLA HuggingFace + 4-bit Inference + Latency 측정


> **실습 목표**: RTX 4070 12GB 에서 OpenVLA 4-bit inference 가 동작하고, 첫 latency 데이터를 확보한다.
> **예상 시간**: 6-8시간


---


## 환경 설정


```bash
conda activate phase4
pip install -r requirements.txt
```


```bash
# OpenVLA repo clone (코드 참고용)
cd ~
git clone https://github.com/openvla/openvla
cd openvla
```


---


## 실습 1: 첫 OpenVLA Inference


**파일명**: `practice_first_inference.py`


```python
"""
실습 1: OpenVLA 의 첫 inference 실행 (4-bit nf4)
"""
import torch
import numpy as np
from PIL import Image
from transformers import AutoModelForVision2Seq, AutoProcessor, BitsAndBytesConfig


print("=" * 60)
print("실습 1: OpenVLA 첫 inference")
print("=" * 60)


# -- 1-1. 4-bit quantization config --
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,
    bnb_4bit_quant_type='nf4',
    bnb_4bit_use_double_quant=True,
    bnb_4bit_compute_dtype=torch.float16,
)


# -- 1-2. processor + model 로드 --
print("\n[1-1] Processor 로드")
processor = AutoProcessor.from_pretrained(
    "openvla/openvla-7b",
    trust_remote_code=True,
)
print("Processor type:", type(processor).__name__)


print("\n[1-2] Model 로드 (4-bit, 시간 ~ 5분)")
vla = AutoModelForVision2Seq.from_pretrained(
    "openvla/openvla-7b",
    attn_implementation="eager",
    torch_dtype=torch.float16,
    low_cpu_mem_usage=True,
    trust_remote_code=True,
    quantization_config=bnb_config,
)
print("Model 로드 완료")
print(f"GPU 메모리 사용: {torch.cuda.memory_allocated()/1e9:.2f} GB")


# -- 1-3. Mock 입력 생성 --
print("\n[1-3] Mock 입력 생성")
image = Image.fromarray(
    (np.random.rand(224, 224, 3) * 255).astype(np.uint8)
)
instruction = "pick up the can"
prompt = f"In: What action should the robot take to {instruction}?\nOut:"


inputs = processor(prompt, image).to("cuda:0", dtype=torch.float16)
print(f"inputs.keys(): {list(inputs.keys())}")


# -- 1-4. 첫 inference --
print("\n[1-4] 첫 inference (warm-up)")
with torch.no_grad():
    action = vla.predict_action(
        **inputs,
        unnormalize_key="bridge_orig",
        do_sample=False,
    )
print(f"Action shape: {action.shape}")
print(f"Action : {action}")
print(f"GPU 메모리 : {torch.cuda.memory_allocated()/1e9:.2f} GB")


print("\n 실습 1 완료!")
```


**실행**:
```bash
python practice_first_inference.py
```


**기대 출력**:
- Action shape: (7,)
- Action: 7-DoF action vector
- GPU memory: ~ 5-7 GB


> 첫 실행 시 모델 다운로드에 ~ 15GB 디스크 사용 + ~ 5-10분 시간.


---


## 실습 2: Latency 측정 (산출물 #4 의 기초 데이터)


**파일명**: `practice_latency_measure.py`


```python
"""
실습 2: 100 회 inference 의 latency 통계
이게 산출물 #4 (Real-to-Sim-to-Real) 의 'VLA latency 측정' 의 baseline.
"""
import time
import torch
import numpy as np
from PIL import Image
from transformers import AutoModelForVision2Seq, AutoProcessor, BitsAndBytesConfig


print("=" * 60)
print("실습 2: OpenVLA Latency 측정")
print("=" * 60)


# -- 2-1. 모델 로드 (실습 1 그대로) --
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,
    bnb_4bit_quant_type='nf4',
    bnb_4bit_use_double_quant=True,
    bnb_4bit_compute_dtype=torch.float16,
)
processor = AutoProcessor.from_pretrained("openvla/openvla-7b", trust_remote_code=True)
vla = AutoModelForVision2Seq.from_pretrained(
    "openvla/openvla-7b",
    attn_implementation="eager",
    torch_dtype=torch.float16,
    low_cpu_mem_usage=True,
    trust_remote_code=True,
    quantization_config=bnb_config,
)
print("\n[2-1] 모델 로드 완료")


# -- 2-2. Warm-up (첫 5번은 측정 제외) --
print("\n[2-2] Warm-up (5 inference)")
image = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
instruction = "pick up the can"
prompt = f"In: What action should the robot take to {instruction}?\nOut:"
inputs = processor(prompt, image).to("cuda:0", dtype=torch.float16)


for i in range(5):
    with torch.no_grad():
        action = vla.predict_action(**inputs, unnormalize_key="bridge_orig", do_sample=False)
print("warm-up 완료")


# -- 2-3. Latency 측정 (100 회) --
print("\n[2-3] 100 회 inference latency 측정")
latencies = []


for i in range(100):
    # 새 이미지 (cache 효과 방지)
    image = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
    inputs = processor(prompt, image).to("cuda:0", dtype=torch.float16)


    torch.cuda.synchronize()
    start = time.time()
    with torch.no_grad():
        action = vla.predict_action(**inputs, unnormalize_key="bridge_orig", do_sample=False)
    torch.cuda.synchronize()
    elapsed_ms = (time.time() - start) * 1000
    latencies.append(elapsed_ms)


    if (i + 1) % 10 == 0:
        print(f"{i+1}/100: latest = {elapsed_ms:.1f} ms")


# -- 2-4. 통계 --
arr = np.array(latencies)
print("\n[2-4] Latency 통계")
print(f"mean : {arr.mean():.1f} ms")
print(f"median : {np.median(arr):.1f} ms")
print(f"std : {arr.std():.1f} ms")
print(f"min : {arr.min():.1f} ms")
print(f"max : {arr.max():.1f} ms")
print(f"p95 : {np.percentile(arr, 95):.1f} ms")
print(f"p99 : {np.percentile(arr, 99):.1f} ms")
print()
print(f"Throughput : {1000 / arr.mean():.2f} Hz")


# -- 2-5. 결과 저장 (산출물 #4 의 baseline) --
np.save("openvla_latency_4070_int4.npy", arr)
print("\n 결과 저장: openvla_latency_4070_int4.npy")
print("-> Phase 7 의 산출물 #4 에서 비교 baseline 으로 사용")


print("\n 실습 2 완료!")
```


**실행**:
```bash
python practice_latency_measure.py
```


**기대 통계** (RTX 4070, 4-bit nf4):
- mean: 120-180 ms
- p95: 200-250 ms
- Throughput: 5-7 Hz


> 이 latency 데이터를 Phase 7 의 산출물 #4 영상에서 직접 인용. "RTX 4070 + 4-bit OpenVLA = 6Hz, 5Hz robot control 가능".


---


## 실습 3: 에러 + 해결 기록


**파일명**: `~/phase4_notes/week6/errors_log.md`


```markdown
# Week 6 inference 에러 기록


## 시도한 환경
- GPU: RTX 4070 12GB
- CUDA: ___
- Python: ___
- bitsandbytes: ___


## 발생한 에러


### 에러 1: ___
- 증상: ___
- 원인: ___
- 해결: ___


### 에러 2: ___
- ...


## 안정 동작 확인
- [ ] First inference 성공
- [ ] 100회 latency 측정 완료
- [ ] GPU 메모리 < 12 GB
- [ ] inference 결과가 reasonable 한 action range (-0.1~0.1 m, -pi~pi rad)
```


---


## 실습 체크리스트


- [ ] OpenVLA HuggingFace 모델 카드 정독
- [ ] `practice_first_inference.py` 성공
- [ ] `practice_latency_measure.py` 실행 + 통계 확보
- [ ] `errors_log.md` 작성
- [ ] quiz_easy / quiz_medium 풀기
- [ ] `openvla_latency_4070_int4.npy` git commit (산출물 #4 의 입력 데이터)


---


## 참고 자료


- [OpenVLA HuggingFace](https://huggingface.co/openvla/openvla-7b)
- [bitsandbytes 라이브러리](https://github.com/TimDettmers/bitsandbytes)
- [OpenVLA inference 예시 (GitHub)](https://github.com/openvla/openvla)
- [HuggingFace Quantization guide](https://huggingface.co/docs/transformers/main/en/quantization)


---


## 트러블슈팅 (자주 발생)


| 에러 | 해결 |
|---|---|
| `RuntimeError: CUDA out of memory` | 다른 GPU 프로세스 종료, 4-bit 확인 |
| `ImportError: bitsandbytes` | `pip install bitsandbytes>=0.43.0` |
| `flash_attn not installed` | `attn_implementation="eager"` 옵션 추가 |
| download 중단 | `--resume-download` 옵션 |
| inference 가 0 action | unnormalize_key 누락 or 잘못 |
