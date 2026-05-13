# Week 1 실습: Patch Embedding 구현 + ViT inference + Attention 시각화


> **실습 목표**: ViT 의 patch embedding 을 직접 구현하고, HuggingFace ViT 로 inference + attention map 을 시각화한다.
> **예상 시간**: 5-7시간


---


## 환경 설정


```bash
conda create -n phase5 python=3.10 -y
conda activate phase5
pip install -r requirements.txt
```


---


## 실습 1: Patch Embedding 직접 구현


**파일명**: `practice_patch_embed.py`


```python
"""
실습 1: ViT 의 Patch Embedding 을 직접 구현
"""
import torch
import torch.nn as nn
import numpy as np
from PIL import Image
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


print("=" * 60)
print("실습 1: ViT Patch Embedding 직접 구현")
print("=" * 60)




class PatchEmbed(nn.Module):
    """ViT 의 patch embedding (Conv2d 로 구현)"""


    def __init__(self, img_size=224, patch_size=16, in_channels=3, embed_dim=768):
        super().__init__()
        self.img_size = img_size
        self.patch_size = patch_size
        self.n_patches = (img_size // patch_size) ** 2 # 196


        # 16x16 patch 추출 + linear projection 을 한 번에 (Conv2d 사용)
        self.proj = nn.Conv2d(
            in_channels, embed_dim,
            kernel_size=patch_size, stride=patch_size,
        )


        # CLS token + position embedding
        self.cls_token = nn.Parameter(torch.zeros(1, 1, embed_dim))
        self.pos_embed = nn.Parameter(torch.zeros(1, self.n_patches + 1, embed_dim))


    def forward(self, x):
        # x: (B, 3, 224, 224)
        B = x.shape[0]


        # patch projection
        x = self.proj(x) # (B, 768, 14, 14)
        x = x.flatten(2).transpose(1, 2) # (B, 196, 768)


        # CLS token prepend
        cls_tokens = self.cls_token.expand(B, -1, -1) # (B, 1, 768)
        x = torch.cat([cls_tokens, x], dim=1) # (B, 197, 768)


        # position embedding
        x = x + self.pos_embed


        return x




# -- 1-1. PatchEmbed 사용 예시 --
print("\n[1-1] PatchEmbed 동작 확인")
patch_embed = PatchEmbed()
dummy_img = torch.randn(2, 3, 224, 224)
out = patch_embed(dummy_img)
print(f"입력 shape : {dummy_img.shape}")
print(f"출력 shape : {out.shape} (B, 197, 768)")
assert out.shape == (2, 197, 768)


# -- 1-2. 196 = 14x14 패치 확인 --
print("\n[1-2] patch 개수 확인")
print(f"224 / 16 = {224 // 16}")
print(f"14 * 14 = {14*14}")
print(f"+ CLS = 197")


# -- 1-3. 시각화: 실제 이미지를 16x16 patch 로 나누어 시각화 --
print("\n[1-3] 실제 이미지 patch 시각화")
# 가짜 image
fake = (np.random.rand(224, 224, 3) * 255).astype(np.uint8)
patches = []
for i in range(14):
    for j in range(14):
        patches.append(fake[i*16:(i+1)*16, j*16:(j+1)*16])


fig, axes = plt.subplots(14, 14, figsize=(10, 10))
for idx, ax in enumerate(axes.flat):
    ax.imshow(patches[idx])
    ax.axis('off')
plt.suptitle("ViT: 224x224 image -> 196 patches of 16x16")
plt.tight_layout()
plt.savefig("vit_patches.png", dpi=80)
print("저장: vit_patches.png")


print("\n 실습 1 완료!")
```


---


## 실습 2: HuggingFace ViT inference


**파일명**: `practice_vit_inference.py`


```python
"""
실습 2: HuggingFace 의 사전학습 ViT-B 로 ImageNet 분류
"""
import torch
from PIL import Image
from transformers import ViTImageProcessor, ViTForImageClassification


print("=" * 60)
print("실습 2: ViT inference")
print("=" * 60)


# -- 2-1. 모델 로드 --
processor = ViTImageProcessor.from_pretrained('google/vit-base-patch16-224')
model = ViTForImageClassification.from_pretrained('google/vit-base-patch16-224')
model.eval()
print(f"모델: google/vit-base-patch16-224")
print(f"파라미터 수: {sum(p.numel() for p in model.parameters()):,}")


# -- 2-2. 추론 --
# 실제 이미지 또는 mock
img = Image.open('/path/to/your/image.jpg') if False else Image.fromarray(
    (torch.rand(224, 224, 3) * 255).numpy().astype('uint8')
)


inputs = processor(images=img, return_tensors="pt")
with torch.no_grad():
    outputs = model(**inputs)


logits = outputs.logits
pred = logits.argmax(-1).item()
print(f"\n 예측 class id: {pred}")
print(f"예측 label : {model.config.id2label[pred]}")
print(f"확률 top-5 : {torch.softmax(logits, dim=-1)[0].topk(5)}")
print("\n 실습 2 완료!")
```


---


## 실습 3: Attention 시각화


**파일명**: `practice_attention_viz.py`


```python
"""
실습 3: ViT 의 attention map 시각화
- [CLS] token 이 어떤 patch 에 attention 하는지 보여줌
"""
import torch
import numpy as np
from PIL import Image
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


from transformers import ViTImageProcessor, ViTModel


print("=" * 60)
print("실습 3: Attention 시각화")
print("=" * 60)


processor = ViTImageProcessor.from_pretrained('google/vit-base-patch16-224')
model = ViTModel.from_pretrained('google/vit-base-patch16-224', output_attentions=True)
model.eval()


img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
inputs = processor(images=img, return_tensors="pt")


with torch.no_grad():
    out = model(**inputs)


# attention: tuple of (B, H, N, N) per layer
last_attn = out.attentions[-1] # (1, 12, 197, 197)
print(f"attention shape: {last_attn.shape}")


# [CLS] token (idx 0) 이 patch 들에 어떻게 attention 하는가
cls_attn = last_attn[0, :, 0, 1:] # (12 heads, 196 patches)
cls_attn_mean = cls_attn.mean(0) # (196,)
cls_attn_2d = cls_attn_mean.reshape(14, 14).numpy()


fig, ax = plt.subplots(1, 2, figsize=(10, 5))
ax[0].imshow(img); ax[0].set_title("RGB"); ax[0].axis('off')
ax[1].imshow(cls_attn_2d, cmap='hot'); ax[1].set_title("CLS attention (last layer, mean over heads)")
ax[1].axis('off')
plt.tight_layout()
plt.savefig("vit_attention.png", dpi=100)
print("\n 저장: vit_attention.png")
print("-> 밝은 부분이 CLS 가 'class 결정에 중요' 라고 본 patch")


print("\n 실습 3 완료!")
```


---


## 실습 4: ViT one-pager


**파일명**: `~/phase5_notes/week1/vit_one_pager.md`


```markdown
# ViT 한 페이지


## 1. One-liner
Image -> 16x16 patch sequence -> Transformer.


## 2. Architecture
[Patch Embed -> [CLS] + 196 tokens + pos -> Transformer x N -> Classifier]


## 3. 수치
- Image: 224x224 -> 14x14 = 196 patches
- Patch dim: 16*16*3 = 768
- Hidden dim D: 768 (ViT-B), 1024 (ViT-L)
- Total seq len: 197 (CLS + 196)


## 4. 주요 variant
- ViT-B: 86M, 12 layer
- ViT-L: 300M, 24 layer (OpenVLA backbone)


## 5. CNN 대비 차이
- 데이터 더 필요
- scale 잘 됨
- 다른 modality 결합 자연스러움


## 6. OpenVLA 연결
- DINOv2 (ViT-L) + SigLIP (ViT-L) hybrid
- 둘 다 ViT 기반
```


---


## 실습 체크리스트


- [ ] PatchEmbed 직접 구현 + 동작 확인
- [ ] HuggingFace ViT inference 성공
- [ ] Attention map 시각화
- [ ] ViT one-pager 작성
- [ ] quiz_easy / quiz_medium


---


## 참고 자료


- [ViT paper](https://arxiv.org/abs/2010.11929)
- [HuggingFace ViT docs](https://huggingface.co/docs/transformers/model_doc/vit)
- [The Illustrated Transformer](https://jalammar.github.io/illustrated-transformer/)
