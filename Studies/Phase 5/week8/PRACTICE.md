# Week 8 실습: DINOv2 inference + Patch Feature PCA 시각화

> [time] **예상 시간**: 4시간

---

## [note] 실습 1: DINOv2 latency benchmark

```python
"""
practice_dinov2_bench.py
"""
import time
import torch
import numpy as np
from PIL import Image
from transformers import AutoImageProcessor, AutoModel

proc = AutoImageProcessor.from_pretrained('facebook/dinov2-large')
model = AutoModel.from_pretrained('facebook/dinov2-large').to('cuda').half().eval()

img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
inputs = proc(images=img, return_tensors='pt').to('cuda')
inputs = {k: v.half() if v.dtype == torch.float32 else v for k, v in inputs.items()}

# warm-up
for _ in range(5):
    with torch.no_grad():
        _ = model(**inputs)
torch.cuda.synchronize()

# Measure
lats = []
for _ in range(100):
    torch.cuda.synchronize()
    t0 = time.time()
    with torch.no_grad():
        _ = model(**inputs)
    torch.cuda.synchronize()
    lats.append((time.time() - t0) * 1000)

arr = np.array(lats)
print(f"DINOv2 ViT-L mean: {arr.mean():.2f} ms")
print(f"VRAM peak: {torch.cuda.max_memory_allocated()/1e9:.2f} GB")
```

---

## [note] 실습 2: PCA 시각화

```python
"""
practice_dinov2_pca.py
- patch feature 의 PCA 3 dim 시각화
"""
import torch
import numpy as np
from PIL import Image
from transformers import AutoImageProcessor, AutoModel
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

proc = AutoImageProcessor.from_pretrained('facebook/dinov2-base')  # 가벼움
model = AutoModel.from_pretrained('facebook/dinov2-base').eval()

img = Image.open('/path/to/scene.jpg').convert('RGB') if False else Image.fromarray(
    (np.random.rand(224, 224, 3) * 255).astype(np.uint8))

inputs = proc(images=img, return_tensors='pt')
with torch.no_grad():
    out = model(**inputs)

feat = out.last_hidden_state[0, 1:].numpy()  # (N, D)
n_patches = feat.shape[0]
grid = int(np.sqrt(n_patches))

# PCA
mean = feat.mean(0, keepdims=True)
centered = feat - mean
U, S, Vt = np.linalg.svd(centered, full_matrices=False)
pca3 = centered @ Vt[:3].T  # (N, 3)
pca3 = (pca3 - pca3.min(0)) / (pca3.max(0) - pca3.min(0) + 1e-9)
pca3 = pca3.reshape(grid, grid, 3)

fig, ax = plt.subplots(1, 2, figsize=(8, 4))
ax[0].imshow(img); ax[0].set_title('RGB'); ax[0].axis('off')
ax[1].imshow(pca3); ax[1].set_title(f'DINOv2 patch PCA ({grid}x{grid})'); ax[1].axis('off')
plt.tight_layout()
plt.savefig('dinov2_pca.png', dpi=100)
print("저장: dinov2_pca.png")
```

기대: 같은 객체 / 같은 영역이 같은 색으로 묶임 (segmentation 비슷).

---

## [note] 실습 3: CLIP / DINOv2 / SigLIP 비교

```python
"""
같은 image 의 세 encoder 의 PCA 결과 비교
"""
# 동일 흐름으로 세 모델 모두 실행
# - CLIP (openai/clip-vit-large-patch14): semantic cluster
# - DINOv2 (facebook/dinov2-large): spatial cluster
# - SigLIP (google/siglip-large-patch16-256): semantic cluster (CLIP 와 비슷)

# fig, ax = plt.subplots(1, 4, figsize=(16, 4))
# 4 패널: RGB, CLIP PCA, DINOv2 PCA, SigLIP PCA
```

기대:
- RGB: 원본
- CLIP / SigLIP: 같은 object 가 같은 색 (semantic)
- DINOv2: spatial 경계가 명확 (segmentation 처럼)

---

## [O] 체크리스트
- [ ] DINOv2 latency benchmark
- [ ] PCA 시각화 동작
- [ ] CLIP / SigLIP 비교
- [ ] quiz
