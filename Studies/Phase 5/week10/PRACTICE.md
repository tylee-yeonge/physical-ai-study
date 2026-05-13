# Week 10 실습: SigLIP inference + CLIP 비교


> **예상 시간**: 4시간


---


## 실습 1: SigLIP inference


```python
"""
practice_siglip_inference.py
"""
import torch
from PIL import Image
from transformers import AutoProcessor, AutoModel
import numpy as np


proc = AutoProcessor.from_pretrained('google/siglip-large-patch16-256')
model = AutoModel.from_pretrained('google/siglip-large-patch16-256').eval()


img = Image.fromarray((np.random.rand(256, 256, 3) * 255).astype(np.uint8))
texts = ["a photo of a cat", "a photo of a dog", "a photo of a robot arm"]


inputs = proc(text=texts, images=img, return_tensors='pt', padding='max_length')


with torch.no_grad():
    out = model(**inputs)


# logits_per_image: sigmoid 적용 후
print(f"logits_per_image: {out.logits_per_image}")
# sigmoid (CLIP 는 softmax)
import torch.nn.functional as F
probs = torch.sigmoid(out.logits_per_image)
for t, p in zip(texts, probs[0]):
    print(f"'{t}': {p.item():.4f}")
print()
print("NOTE: CLIP 와 달리 SigLIP 는 각 pair 가 독립 (sum != 1)")
```


---


## 실습 2: CLIP vs SigLIP 직접 비교


```python
"""
practice_clip_vs_siglip.py
"""
import torch
import numpy as np
from PIL import Image
from transformers import (
    CLIPProcessor, CLIPModel,
    AutoProcessor, AutoModel,
)


clip_proc = CLIPProcessor.from_pretrained("openai/clip-vit-large-patch14")
clip_model = CLIPModel.from_pretrained("openai/clip-vit-large-patch14").eval()


sig_proc = AutoProcessor.from_pretrained("google/siglip-large-patch16-256")
sig_model = AutoModel.from_pretrained("google/siglip-large-patch16-256").eval()


img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
texts = ["red cup", "blue mouse", "robot arm", "keyboard"]


# CLIP
clip_inputs = clip_proc(text=[f"a photo of a {t}" for t in texts],
                         images=img, return_tensors='pt', padding=True)
with torch.no_grad():
    clip_out = clip_model(**clip_inputs)
clip_probs = clip_out.logits_per_image.softmax(-1).squeeze().tolist()


# SigLIP
sig_inputs = sig_proc(text=[f"a photo of a {t}" for t in texts],
                        images=img, return_tensors='pt', padding='max_length')
with torch.no_grad():
    sig_out = sig_model(**sig_inputs)
sig_probs = torch.sigmoid(sig_out.logits_per_image).squeeze().tolist()


# 비교
print(f"{'Label':<15}{'CLIP softmax':<15}{'SigLIP sigmoid'}")
print("-" * 50)
for t, c, s in zip(texts, clip_probs, sig_probs):
    print(f"{t:<15}{c:<15.4f}{s:.4f}")


# 차이:
# CLIP probs sum = 1 (softmax)
# SigLIP probs 가각 독립 (sigmoid), sum 다를 수 있음
print(f"\nCLIP sum: {sum(clip_probs):.4f}")
print(f"SigLIP sum: {sum(sig_probs):.4f}")
```


---


## 실습 3: one-pager


`~/phase5_notes/week10/siglip_one_pager.md`:


```markdown
# SigLIP 한 페이지


## 1. One-liner
CLIP 의 sigmoid 변종. softmax 대신 pairwise sigmoid loss.


## 2. CLIP vs SigLIP
| 항목 | CLIP | SigLIP |
|---|---|---|
| Loss | softmax InfoNCE | sigmoid pairwise |
| Batch 의존 | 4K+ | 1K OK |
| Compute | high | lower |
| ImageNet zero-shot | 75% | 76% |


## 3. Architecture
ViT (image) + Transformer (text) - CLIP 와 같음
차이는 loss function 만


## 4. OpenVLA 에서
- SigLIP-L 또는 SigLIP-SO400M
- semantic features
- DINOv2 (spatial) 과 hybrid


## 5. 한계
CLIP 와 비슷: spatial / fine-grained / OCR / dynamic 약함
-> hybrid 가 필요한 이유
```


---


## 체크리스트
- [ ] SigLIP inference
- [ ] CLIP vs SigLIP 비교
- [ ] one-pager
- [ ] quiz
