# Week 4 실습: CLIP Architecture + image-text similarity


> **예상 시간**: 4-6시간


---


## 실습 1: CLIP 의 image / text encoder 분해


**파일명**: `practice_clip_arch.py`


```python
"""
실습: CLIP 의 두 encoder 출력 vector 확인
"""
import torch
from PIL import Image
from transformers import CLIPProcessor, CLIPModel


print("CLIP Architecture 분해")
print("=" * 50)


device = 'cuda' if torch.cuda.is_available() else 'cpu'
processor = CLIPProcessor.from_pretrained("openai/clip-vit-base-patch32")
model = CLIPModel.from_pretrained("openai/clip-vit-base-patch32").to(device).eval()


print(f"\nImage encoder: {model.vision_model.__class__.__name__}")
print(f"Text encoder : {model.text_model.__class__.__name__}")
print(f"Image proj : {model.visual_projection}")
print(f"Text proj : {model.text_projection}")


# 분리 추론
import numpy as np
img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
texts = ["a photo of a cat", "a photo of a dog"]


inputs = processor(text=texts, images=img, return_tensors="pt", padding=True).to(device)


with torch.no_grad():
    image_features = model.get_image_features(pixel_values=inputs.pixel_values)
    text_features = model.get_text_features(input_ids=inputs.input_ids,
                                             attention_mask=inputs.attention_mask)


print(f"\nImage features shape: {image_features.shape}")
print(f"Text features shape: {text_features.shape}")


# Normalize
image_features = image_features / image_features.norm(dim=-1, keepdim=True)
text_features = text_features / text_features.norm(dim=-1, keepdim=True)


# Cosine similarity
sim = (image_features @ text_features.T)
print(f"\nSimilarities:")
for i, t in enumerate(texts):
    print(f"'{t}': {sim[0, i].item():.3f}")
```


---


## 실습 2: image-text similarity mini-demo


**파일명**: `practice_clip_similarity.py`


```python
"""
mini-demo: 다양한 prompt 와 image 의 similarity 매트릭스
"""
import torch
from PIL import Image
from transformers import CLIPProcessor, CLIPModel
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np


processor = CLIPProcessor.from_pretrained("openai/clip-vit-base-patch32")
model = CLIPModel.from_pretrained("openai/clip-vit-base-patch32").eval()


prompts = [
    "a photo of a cat",
    "a photo of a dog",
    "a photo of a banana",
    "a photo of a car",
    "a photo of a robot arm",
]
# 5 개의 random image (실제로는 실제 이미지 권장)
imgs = [Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8)) for _ in range(5)]


inputs = processor(text=prompts, images=imgs, return_tensors="pt", padding=True)
with torch.no_grad():
    out = model(**inputs)


# logits_per_image: (5 images, 5 prompts)
sim = out.logits_per_image.numpy()


plt.figure(figsize=(8, 6))
plt.imshow(sim, cmap='viridis')
plt.colorbar(label='similarity (logit)')
plt.xticks(range(len(prompts)), prompts, rotation=45)
plt.yticks(range(len(imgs)), [f"image {i}" for i in range(len(imgs))])
plt.title("CLIP Image-Text Similarity")
plt.tight_layout()
plt.savefig("clip_similarity.png", dpi=100)
print("저장: clip_similarity.png")
```


---


## 실습 3: one-pager 보강


`~/phase5_notes/week4/clip_one_pager.md`:


```markdown
# CLIP 한 페이지


## 1. One-liner
Image + text 를 공통 vector space 로 매핑 (contrastive).


## 2. Architecture
Image encoder (ViT) + Text encoder (Transformer)
-> 각자 MLP projection -> 512 dim shared vector space
-> cosine similarity


## 3. Training: InfoNCE
batch 안의 N 쌍에서 correct 는 높은 sim, wrong (N-1) 은 낮음.
data: WIT 400M image-text pair.


## 4. Variant
| Model | Image | Params |
|---|---|---|
| ViT-B/32 | ViT-B patch 32 | 151M |
| ViT-L/14 | ViT-L patch 14 | 428M |


## 5. Open-vocabulary
임의 prompt 로 zero-shot 분류. 새 class = 새 prompt.


## 6. 한계
- caption noise
- 영어 위주
- fine-grained 약함
- long text 한계
- spatial reasoning 약함


## 7. OpenVLA 와의 관계
SigLIP (CLIP 의 sigmoid 변종) 이 OpenVLA backbone 중 하나.
```


---


## 체크리스트
- [ ] CLIP architecture 분해
- [ ] similarity demo
- [ ] one-pager
- [ ] quiz
