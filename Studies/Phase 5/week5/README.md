# Week 5: HuggingFace CLIP inference + image-text similarity

> [goal] **이번 주 목표**: HuggingFace CLIP 으로 다양한 image-text pair 의 similarity 측정. zero-shot classification 시연.
> [time] **예상 시간**: 6시간
> [tip] **핵심 질문**: "자작 팔 환경 image 에 대해 CLIP 의 zero-shot 정확도는?"

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 | `requirements.txt` | week4 그대로 |
| 2 | HF CLIP inference | `PRACTICE.md` 1 | similarity |
| 3 | Zero-shot classification | `PRACTICE.md` 2 | ImageNet 일부 |
| 4 | 자작 환경 test | `PRACTICE.md` 3 | 책상 + 컵 prompt |
| 5 | 퀴즈 + 노트 | quiz | |

---

## [ref] 핵심 개념

### 1. CLIP inference 표준 패턴

```python
from transformers import CLIPProcessor, CLIPModel

processor = CLIPProcessor.from_pretrained("openai/clip-vit-large-patch14")
model = CLIPModel.from_pretrained("openai/clip-vit-large-patch14")

texts = ["a photo of a cat", "a photo of a dog"]
inputs = processor(text=texts, images=image, return_tensors="pt", padding=True)
outputs = model(**inputs)
logits = outputs.logits_per_image  # (B_img, B_txt)
probs = logits.softmax(dim=-1)
```

### 2. Prompt engineering

| Prompt | 효과 |
|---|---|
| "cat" | 낮은 정확도 |
| "a photo of a cat" | 표준 |
| "a photo of a {breed} cat" | fine-grained |
| "a high-quality photo of a cat" | 약간 향상 |

"a photo of a X" 표준.

### 3. Zero-shot 한계

| 분야 | 정확도 |
|---|---|
| ImageNet | 70~80% |
| Fashion | 80%+ |
| 추상 / 그래픽 | 50~60% |
| Fine-grained | 40~50% |
| OCR | 매우 약함 |

### 4. CLIP 의 자작 팔 환경 활용

```
"a photo of a red cup" -> 빨간 컵 인식
"a photo of a robot arm" -> 자작 팔 인식
```

OpenVLA 의 "pick up the red can" 명령에서 'red can' 인식이 비슷한 원리.

### 5. CLIP vs SigLIP (week 10 예고편)

| 항목 | CLIP | SigLIP |
|---|---|---|
| Loss | softmax InfoNCE | sigmoid |
| Batch 의존 | 강함 (4K+) | 약함 (1K OK) |
| OpenVLA | 안 씀 | 사용 |

---

## [search] 자체 점검

**Q1. CLIP 표준 prompt 패턴?**
> "a photo of a X". 학습 데이터 분포에 가까움.

**Q2. Zero-shot 정확도 일반 객체?**
> 70~80%.

**Q3. Detection 가능?**
> CLIP 단독으로 X. GroundingDINO / OWL-ViT 등 별도.

**Q4. 배치 사이즈 권장?**
> 4096+ (InfoNCE hard negative).

**Q5. SigLIP 와 차이?**
> sigmoid loss, batch 의존성 약함, OpenVLA 사용.

---

## [note] 실습 + 다음

### 이번 주
- CLIP inference 실습 1, 2, 3
- 자작 환경 image 테스트
- quiz

### 다음 주 (week 6)
- Open-vocab classification mini-demo

---

## [goal] 핵심 요약

1. **HuggingFace CLIPProcessor + CLIPModel**
2. **"a photo of a X"** 표준 prompt
3. **Zero-shot 70~80%** 일반 객체
4. **Detection 별도 모델** (CLIP feature 활용)
5. **SigLIP 가 CLIP 의 후속**, OpenVLA backbone

[O] 이전: [Week 4](../week4/README.md) | 다음: [Week 6](../week6/README.md)
