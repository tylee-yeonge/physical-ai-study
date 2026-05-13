# Week 9 실습: OpenVLA Backbone 종합 분석

> [time] **예상 시간**: 4시간

---

## [note] 실습 1: DINOv2 + SigLIP 통합 inference

```python
"""
practice_openvla_vision_only.py
OpenVLA 의 vision encoder 부분만 직접 실행
"""
import torch
from PIL import Image
from transformers import AutoModel, AutoImageProcessor
import numpy as np

# DINOv2
dino_proc = AutoImageProcessor.from_pretrained('facebook/dinov2-large')
dino_model = AutoModel.from_pretrained('facebook/dinov2-large').eval()

# SigLIP
sig_proc = AutoImageProcessor.from_pretrained('google/siglip-large-patch16-256')
sig_model = AutoModel.from_pretrained('google/siglip-large-patch16-256').eval()

img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))

# DINOv2 forward
dino_inputs = dino_proc(images=img, return_tensors='pt')
with torch.no_grad():
    dino_out = dino_model(**dino_inputs)
dino_tokens = dino_out.last_hidden_state[0, 1:]  # CLS 제외
print(f"DINOv2 tokens: {dino_tokens.shape}")  # (256, 1024) for 224 input

# SigLIP forward
sig_inputs = sig_proc(images=img, return_tensors='pt')
with torch.no_grad():
    sig_out = sig_model.vision_model(**sig_inputs)
sig_tokens = sig_out.last_hidden_state[0]  # (196, 1024)
print(f"SigLIP tokens: {sig_tokens.shape}")

# Concat
combined = torch.cat([dino_tokens, sig_tokens], dim=0)
print(f"Combined : {combined.shape}")  # (~452, 1024)
```

---

## [note] 실습 2: Projector MLP 분석

```python
"""
practice_projector.py
OpenVLA 의 projector (MLP) 구조 추정
"""
import torch.nn as nn

# OpenVLA 의 projector (대략)
projector = nn.Sequential(
    nn.Linear(1024, 4096),  # vision -> Llama hidden
    nn.GELU(),
    nn.Linear(4096, 4096),
)

# 파라미터 수
params = sum(p.numel() for p in projector.parameters())
print(f"Projector params: {params:,}")
print(f"-> 약 {params/1e6:.1f} M")

# 학습 시 -> LoRA 와 함께 학습
# 추론 시 -> 한 번의 forward (latency ~ 2 ms)
```

---

## [note] 실습 3: OpenVLA Architecture 다이어그램 갱신

`~/phase5_notes/week9/openvla_arch_v2.md`:

```markdown
# OpenVLA Architecture v2 (Phase 4 week 4 보강)

## Vision Backbone

\`\`\`mermaid
flowchart LR
    RGB --> DINO[DINOv2 ViT-L/14<br/>300M params<br/>spatial features]
    RGB --> SIG[SigLIP ViT-L/16<br/>300M params<br/>semantic features]
    DINO --> T1[256 spatial tokens]
    SIG --> T2[196 semantic tokens]
    T1 --> CC[Concat]
    T2 --> CC
    CC --> P[Projector MLP<br/>~ 30M params]
    P --> LP[Llama 2 7B prompt]
    INSTR[Instruction text] --> LP
    LP --> LM[Llama 2 7B<br/>frozen + LoRA]
    LM --> AT[Action tokens]
    AT --> DETOK[De-tokenize]
    DETOK --> ACTION[7-DoF Action]
\`\`\`

## Component 별 weights / training

| Component | params | weights | Phase 7 학습 |
|---|---|---|---|
| DINOv2 | 300M | frozen | X |
| SigLIP | 300M | frozen | X |
| Projector | ~30M | 학습 가능 | O |
| Llama 2 7B | 7B | LoRA rank 32 | O (~70M) |

## Phase 4 week 4 와의 차이

- Vision tokens 의 정확한 수 (256 + 196 = 452)
- Projector 의 크기 (~30M)
- 각 component 의 학습 여부
- LoRA fine-tune 시 학습되는 부분 명확
```

---

## [O] 체크리스트
- [ ] DINOv2 + SigLIP 통합 inference
- [ ] Projector 분석
- [ ] Architecture v2 다이어그램
- [ ] quiz
