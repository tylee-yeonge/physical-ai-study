# Week 7 실습: DINOv2 정독 + Architecture

> [time] **예상 시간**: 5시간

---

## [note] 실습 1: DINOv2 reading note

`~/phase5_notes/week7/dinov2_reading_note.md`:

```markdown
# DINOv2 Reading Note

## 0. Meta
- Title: DINOv2: Learning Robust Visual Features without Supervision
- Authors: Meta AI Research (2023)

## 1. One-liner
______________________________________________________________

## 2. 핵심 contribution
1. ______________________________________________
2. ______________________________________________
3. ______________________________________________

## 3. Architecture
ViT-S/B/L/G + patch 14
(직접 그림 또는 ASCII)

## 4. Self-distillation
- student/teacher mechanism
- EMA of teacher

## 5. iBOT (Masked Image Modeling)
- mask 비율: ___
- 효과: ___

## 6. 학습 데이터
- LVD-142M
- curation 방식: ___

## 7. CLIP 와의 비교
- DINOv2: spatial
- CLIP: semantic

## 8. OpenVLA 와의 연결
- ___
```

---

## [note] 실습 2: DINOv2 inference

```python
"""
practice_dinov2_inference.py
"""
import torch
from PIL import Image
from transformers import AutoImageProcessor, AutoModel

proc = AutoImageProcessor.from_pretrained('facebook/dinov2-large')
model = AutoModel.from_pretrained('facebook/dinov2-large').eval()

print(f"Hidden dim: {model.config.hidden_size}")  # 1024 (ViT-L)
print(f"Layers: {model.config.num_hidden_layers}")  # 24

import numpy as np
img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
inputs = proc(images=img, return_tensors='pt')

with torch.no_grad():
    out = model(**inputs)

# last_hidden_state: (1, n_patches+1, 1024)
print(f"Hidden state shape: {out.last_hidden_state.shape}")
print(f"CLS token  : {out.last_hidden_state[0, 0, :5]}")
print(f"Patch[0]  : {out.last_hidden_state[0, 1, :5]}")
```

---

## [note] 실습 3: one-pager

`~/phase5_notes/week7/dinov2_one_pager.md`:

```markdown
# DINOv2 한 페이지

## 1. One-liner
Text label 없는 self-supervised vision encoder, spatial 정보 강함.

## 2. Architecture
ViT 같음 (patch 14)
ViT-S/B/L/G variant

## 3. 학습
- Self-distillation (DINO)
- iBOT (masked image modeling)
- LVD-142M

## 4. CLIP / SigLIP vs DINOv2
- DINOv2: spatial (어디)
- CLIP / SigLIP: semantic (무엇)

## 5. OpenVLA
DINOv2 ViT-L (300M) + SigLIP ViT-L (300M) = vision encoder
```

---

## [O] 체크리스트
- [ ] reading note
- [ ] inference 동작
- [ ] one-pager
- [ ] quiz
