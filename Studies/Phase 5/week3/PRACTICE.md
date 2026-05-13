# Week 3 실습: OpenVLA Vision Latency Breakdown


> **예상 시간**: 4시간


---


## 실습 1: Component-level latency 측정


**파일명**: `practice_openvla_breakdown.py`


```python
"""
OpenVLA inference 의 component-level latency.
Phase 4 week 6 의 단순 latency 가 아닌 vision / LM / 기타 분해.
"""
import time
import torch
import numpy as np
from PIL import Image
from transformers import AutoModelForVision2Seq, AutoProcessor, BitsAndBytesConfig


bnb = BitsAndBytesConfig(
    load_in_4bit=True, bnb_4bit_quant_type='nf4',
    bnb_4bit_use_double_quant=True, bnb_4bit_compute_dtype=torch.float16,
)
proc = AutoProcessor.from_pretrained("openvla/openvla-7b", trust_remote_code=True)
vla = AutoModelForVision2Seq.from_pretrained(
    "openvla/openvla-7b", attn_implementation='eager',
    torch_dtype=torch.float16, low_cpu_mem_usage=True,
    trust_remote_code=True, quantization_config=bnb,
)


img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
inputs = proc("In: pick up the can\nOut:", img).to('cuda:0', dtype=torch.float16)


# warm-up
for _ in range(5):
    with torch.no_grad():
        _ = vla.predict_action(**inputs, unnormalize_key='bridge_orig', do_sample=False)


# Component 별 측정 (간단 hooks 또는 prof. 사용)
n_iter = 50
breakdowns = {'preprocess': [], 'vision': [], 'lm_generate': [], 'total': []}


for _ in range(n_iter):
    torch.cuda.synchronize()
    t0 = time.time()
    # 여기서 OpenVLA 내부에 hook 을 걸어 component 별 측정
    # 또는 torch.profiler 사용
    with torch.no_grad():
        action = vla.predict_action(**inputs, unnormalize_key='bridge_orig', do_sample=False)
    torch.cuda.synchronize()
    breakdowns['total'].append((time.time() - t0) * 1000)


print("=" * 50)
print("OpenVLA latency total:")
arr = np.array(breakdowns['total'])
print(f"mean: {arr.mean():.2f} ms")
print(f"p95 : {np.percentile(arr, 95):.2f} ms")


# Component 별 측정은 torch.profiler 사용 권장:
# with torch.profiler.profile() as prof:
# vla.predict_action(...)
# print(prof.key_averages().table())
```


---


## 실습 2: torch.profiler 로 자세한 분해


```python
"""
실습 2: torch.profiler 로 OpenVLA 의 vision encoder 별 분해
"""
import torch.profiler as profiler


with profiler.profile(
    activities=[profiler.ProfilerActivity.CPU, profiler.ProfilerActivity.CUDA],
    record_shapes=True,
) as prof:
    with torch.no_grad():
        _ = vla.predict_action(**inputs, unnormalize_key='bridge_orig', do_sample=False)


# top 20 operation by CUDA time
print(prof.key_averages().table(sort_by="cuda_time_total", row_limit=20))
```


기대 출력 (예시):
```
Operation CUDA total %
- linear (LM matmul) 50%
- linear (vision matmul) 25%
- softmax (attention) 8%
- layer_norm 5%
- 기타 12%
```


---


## 실습 3: 정리 노트


`~/phase5_notes/week3/openvla_breakdown.md`:


```markdown
# OpenVLA Latency Breakdown (RTX 4070, 4-bit nf4)


## 총 165 ms
| Component | ms | % |
|---|---|---|
| Image preprocess | 5 | 3% |
| DINOv2 forward | 28 | 17% |
| SigLIP forward | 28 | 17% |
| Projector | 2 | 1% |
| LM decoder (autoregressive) | 95 | 58% |
| Action de-tokenize | 1 | <1% |
| 기타 | 6 | 3% |


## 주요 시사점
1. LM decoder 가 최대 비중 (58%)
2. Vision encoder 2개 = 35%
3. 두 vision encoder 병렬화 시 ~ 30 ms 절약 가능 (이론)
4. RTX 4070 SM 부족으로 실제 ~ 5~10 ms


## Phase 4 산출물 #2 와 연결
- 영상에서 보여준 165 ms 의 정확한 구성
- LM 이 최대 -> speculative decoding 등 LM 최적화 시 큰 이득
- Vision 은 이미 작은 비중 -> 추가 최적화 효과 한계
```


---


## 체크리스트
- [ ] OpenVLA 실측 (50회)
- [ ] torch.profiler 로 component 분해
- [ ] 노트 작성
- [ ] quiz
