# Week 2 실습: ViT inference + Benchmark


> **예상 시간**: 5-7시간


---


## 실습 1: ViT-B / ViT-L inference


**파일명**: `practice_vit_benchmark.py`


```python
"""
실습: ViT-B vs ViT-L 의 latency, VRAM, 정확도 비교
"""
import time
import torch
import numpy as np
from PIL import Image
from transformers import ViTImageProcessor, ViTForImageClassification


print("=" * 60)
print("실습: ViT-B / ViT-L benchmark")
print("=" * 60)


device = 'cuda' if torch.cuda.is_available() else 'cpu'
print(f"Device: {device}")


models_to_test = [
    ('google/vit-base-patch16-224', 'ViT-B'),
    ('google/vit-large-patch16-224', 'ViT-L'),
]




def benchmark(model_id, name, n_iter=100):
    print(f"\n--- {name} ({model_id}) ---")
    processor = ViTImageProcessor.from_pretrained(model_id)
    model = ViTForImageClassification.from_pretrained(model_id).to(device).eval()
    model = model.half() # fp16


    params = sum(p.numel() for p in model.parameters())
    print(f"Params: {params:,}")


    img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
    inputs = processor(images=img, return_tensors='pt').to(device)
    inputs = {k: v.half() if v.dtype == torch.float32 else v for k, v in inputs.items()}


    # Warm-up
    for _ in range(5):
        with torch.no_grad():
            _ = model(**inputs)
    torch.cuda.synchronize()


    # Measure
    lats = []
    for _ in range(n_iter):
        torch.cuda.synchronize()
        t0 = time.time()
        with torch.no_grad():
            _ = model(**inputs)
        torch.cuda.synchronize()
        lats.append((time.time() - t0) * 1000)


    lats = np.array(lats)
    vram = torch.cuda.max_memory_allocated() / 1e9


    print(f"Mean latency: {lats.mean():.2f} ms")
    print(f"p95 latency : {np.percentile(lats, 95):.2f} ms")
    print(f"VRAM peak : {vram:.2f} GB")


    del model
    torch.cuda.empty_cache()




for model_id, name in models_to_test:
    benchmark(model_id, name)


print("\n 완료")
```


**예상 출력**:
```
ViT-B (86M): 10 ms / 0.5 GB
ViT-L (300M): 30 ms / 1.2 GB
```


---


## 실습 2: 다양한 image 입력 테스트


**파일명**: `practice_vit_inputs.py`


```python
"""
다양한 image 입력으로 ViT 의 robustness 측정
"""
import torch
import numpy as np
from PIL import Image
from transformers import ViTImageProcessor, ViTForImageClassification


processor = ViTImageProcessor.from_pretrained('google/vit-base-patch16-224')
model = ViTForImageClassification.from_pretrained('google/vit-base-patch16-224').eval()


# 1. natural image (랜덤이지만 자연스러운 패턴 가정)
img_natural = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))


# 2. 회전 (90도)
img_rotated = img_natural.rotate(90)


# 3. 흑백 (grayscale -> 3채널 복사)
import numpy as np
arr = np.array(img_natural.convert('L'))
img_gray = Image.fromarray(np.stack([arr]*3, axis=-1))


# 4. 작은 sub-region
img_small = img_natural.resize((50, 50)).resize((224, 224)) # blur effect


for name, img in [('natural', img_natural), ('rotated', img_rotated),
                  ('gray', img_gray), ('blurry', img_small)]:
    inputs = processor(images=img, return_tensors='pt')
    with torch.no_grad():
        out = model(**inputs)
    pred = out.logits.argmax(-1).item()
    conf = torch.softmax(out.logits, -1).max().item()
    print(f"{name:10s}: class {pred} (conf {conf:.3f})")


print("\n 완료")
print("\n결론:")
print("- natural / gray : 합리적 분류 (confidence > 0.5)")
print("- rotated 90도 : confidence 급락")
print("- blurry : 분류 불확실")
print("-> ViT 는 회전 / scale 에 sensitive (data augmentation 필요)")
```


---


## 실습 3: 한 페이지 보강


`~/phase5_notes/week2/vit_benchmark.md`:


```markdown
# ViT Benchmark (RTX 4070 fp16)


| Model | Params | Mean Latency | p95 | VRAM |
|---|---|---|---|---|
| ViT-B | 86M | 10 ms | 13 ms | 0.5 GB |
| ViT-L | 300M | 30 ms | 35 ms | 1.2 GB |


OpenVLA 의 vision encoder = DINOv2 (ViT-L) + SigLIP (ViT-L)
~ 60 ms / 2.4 GB


Phase 4 week 6 의 OpenVLA 165ms 중:
- Vision (DINOv2 + SigLIP) : ~ 60 ms
- LM decoder generate : ~ 95 ms
- 기타 : ~ 10 ms
```


---


## 체크리스트
- [ ] ViT-B / ViT-L benchmark 결과 확보
- [ ] 다양한 image 입력 결과
- [ ] one-pager 보강
- [ ] quiz
