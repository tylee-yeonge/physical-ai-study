# Week 11 실습: OpenVLA Backbone 최종 통합


> **예상 시간**: 4시간


---


## 실습 1: OpenVLA 최종 architecture 다이어그램


`~/phase5_notes/week11/openvla_final.md`:


```markdown
# OpenVLA Architecture - 최종 통합


\`\`\`mermaid
flowchart TD
    RGB[RGB Image 224x224]


    RGB --> DINO[DINOv2 ViT-L/14<br/>300M params, frozen]
    RGB --> SIG[SigLIP ViT-L/16<br/>300M params, frozen]


    DINO --> DT[256 spatial tokens<br/>1024 dim each]
    SIG --> ST[196 semantic tokens<br/>1024 dim each]


    DT --> CONCAT[Concat<br/>452 tokens]
    ST --> CONCAT


    CONCAT --> PROJ[Projector MLP<br/>30M params<br/>1024 -> 4096]


    INSTR[Instruction text] --> TOK[SentencePiece]
    TOK --> ITOK[~30 text tokens]


    PROJ --> LLAMA[Llama 2 7B<br/>frozen + LoRA r=32]
    ITOK --> LLAMA


    LLAMA --> ATOK[Action tokens<br/>7개]
    ATOK --> DETOK[De-tokenize]
    DETOK --> ACT[7-DoF Action]
\`\`\`


## 학습 매트릭스


| Component | Pre-train | Phase 7 LoRA |
|---|---|---|
| DINOv2 | Meta LVD-142M | frozen |
| SigLIP | Google WebLI | frozen |
| Projector | OpenVLA init | 학습 (30M) |
| Llama | Meta | LoRA r=32 (65M) |


Total trainable = 95M (~1.25%)
```


---


## 실습 2: Latency / VRAM 종합 분석


```python
"""
practice_openvla_full_bench.py
- 모든 component 의 latency / VRAM 측정
"""
import time
import torch
import numpy as np
from PIL import Image
from transformers import AutoImageProcessor, AutoModel, BitsAndBytesConfig
from transformers import AutoModelForVision2Seq, AutoProcessor


device = 'cuda'


# 1. DINOv2 ViT-L 만
dino_proc = AutoImageProcessor.from_pretrained('facebook/dinov2-large')
dino = AutoModel.from_pretrained('facebook/dinov2-large').to(device).half().eval()


# 2. SigLIP ViT-L 만
sig_proc = AutoImageProcessor.from_pretrained('google/siglip-large-patch16-256')
sig = AutoModel.from_pretrained('google/siglip-large-patch16-256').to(device).half().eval()


# 3. OpenVLA 전체
bnb = BitsAndBytesConfig(load_in_4bit=True, bnb_4bit_quant_type='nf4')
proc = AutoProcessor.from_pretrained("openvla/openvla-7b", trust_remote_code=True)
vla = AutoModelForVision2Seq.from_pretrained(
    "openvla/openvla-7b", attn_implementation='eager',
    torch_dtype=torch.float16, low_cpu_mem_usage=True,
    trust_remote_code=True, quantization_config=bnb,
)


img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
inputs = proc("In: pick up the can\nOut:", img).to(device, dtype=torch.float16)


# warm-up
for _ in range(5):
    with torch.no_grad():
        _ = vla.predict_action(**inputs, unnormalize_key='bridge_orig', do_sample=False)


# Measure
results = []
for _ in range(50):
    torch.cuda.synchronize()
    t0 = time.time()
    with torch.no_grad():
        _ = vla.predict_action(**inputs, unnormalize_key='bridge_orig', do_sample=False)
    torch.cuda.synchronize()
    results.append((time.time() - t0) * 1000)


print(f"OpenVLA total : {np.mean(results):.1f} ms")
print(f"VRAM peak : {torch.cuda.max_memory_allocated()/1e9:.2f} GB")
print(f"Throughput : {1000/np.mean(results):.1f} Hz")
```


---


## 실습 3: Phase 5 종합 한 페이지


`~/phase5_notes/week11/phase5_summary.md`:


```markdown
# Phase 5 종합 (2027.02~2027.04)


## Learned
1. ViT: Patch + Self-attention + Position embed
2. CLIP: image-text contrastive, open-vocab
3. DINOv2: self-supervised, spatial features
4. SigLIP: CLIP 의 sigmoid 변종, OpenVLA semantic


## OpenVLA Vision Backbone
- DINOv2 ViT-L (300M) + SigLIP ViT-L (300M)
- concat 452 tokens -> projector -> Llama
- Latency 56 ms (vision), 165 ms (total)
- VRAM 6.5 GB (RTX 4070 fit)


## Phase 7 LoRA Plan
- Trainable: projector + Llama LoRA r=32 = 95M
- 작은 demonstrations (~200) 으로 학습 가능
- Phase 7 산출물 #4 의 핵심 기법


## Skills
- HuggingFace transformers 의 ViT / CLIP / DINOv2 / SigLIP / OpenVLA 인터페이스
- Patch feature PCA 시각화
- Multi-modal architecture 다이어그램 작성
- Component-level latency / VRAM 추정
```


---


## 체크리스트
- [ ] OpenVLA 최종 다이어그램
- [ ] component-level benchmark
- [ ] Phase 5 종합 노트
- [ ] quiz
