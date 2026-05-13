# Week 5 실습: HuggingFace CLIP inference + zero-shot

> [time] **예상 시간**: 4시간

---

## [note] 실습 1: CLIP zero-shot classification

```python
"""
practice_clip_zeroshot.py
"""
import torch
from PIL import Image
from transformers import CLIPProcessor, CLIPModel
import numpy as np

model = CLIPModel.from_pretrained("openai/clip-vit-large-patch14")
proc = CLIPProcessor.from_pretrained("openai/clip-vit-large-patch14")

# ImageNet 의 일부 class
candidate_labels = ['cat', 'dog', 'banana', 'car', 'robot arm', 'red cup', 'mouse']
prompts = [f"a photo of a {l}" for l in candidate_labels]

img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))

inputs = proc(text=prompts, images=img, return_tensors="pt", padding=True)
with torch.no_grad():
    out = model(**inputs)

probs = out.logits_per_image.softmax(dim=-1).squeeze().numpy()
for label, p in sorted(zip(candidate_labels, probs), key=lambda x: -x[1]):
    print(f"  {label:15s}: {p:.4f}")
```

---

## [note] 실습 2: Prompt engineering 실험

```python
"""
practice_clip_prompts.py
- 같은 image 에 대해 다양한 prompt 의 차이
"""
prompts_v1 = ["cat", "dog", "banana"]
prompts_v2 = ["a photo of a cat", "a photo of a dog", "a photo of a banana"]
prompts_v3 = ["a high-quality photograph of a cat", ...]

# 각 prompt set 에 대해 softmax 확률 비교
# 결론: v2 가 표준 선호
```

---

## [note] 실습 3: 자작 환경 image 테스트

자작 책상 + 컵 + 마우스 환경 사진 (스마트폰 또는 ELP) 로:

```python
"""
자작 팔 환경 zero-shot
"""
from PIL import Image

img = Image.open("/path/to/desktop_photo.jpg").convert("RGB")
candidate = ['cup', 'mouse', 'keyboard', 'robot arm', 'monitor', 'pen']
prompts = [f"a photo of a {l}" for l in candidate]

# CLIP inference -> top-3 class
# Phase 7 의 OpenVLA + 자작 팔 환경의 zero-shot 사전 확인
```

기대 결과: 컵 / 마우스 / 키보드 등 일반 객체는 정확. 자작 팔 자체는 unusual 형태라 confidence 낮을 수 있음.

---

## [O] 체크리스트
- [ ] CLIP zero-shot classification 동작
- [ ] Prompt engineering 비교
- [ ] 자작 환경 image 테스트
- [ ] quiz
