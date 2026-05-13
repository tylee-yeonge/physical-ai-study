# Week 4: CLIP 논문 정독 + Contrastive Learning


> **이번 주 목표**: CLIP 의 architecture 와 contrastive learning 의 원리를 한 페이지로 설명할 수 있다.
> **예상 시간**: 8시간
> **핵심 질문**: "Image 와 text 를 같은 vector space 에 매핑하는 게 무슨 의미인가? 그게 'open-vocabulary' 인 이유는?"


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 | `requirements.txt` | torch, transformers, datasets |
| 2 | CLIP 논문 1회독 | https://arxiv.org/abs/2103.00020 | 26 페이지 |
| 3 | Contrastive learning 이해 | `README.md` 3장 | InfoNCE loss |
| 4 | Architecture 분해 | `PRACTICE.md` 1 | image / text encoder + projection |
| 5 | Mini-demo | `PRACTICE.md` 2 | image-text similarity |
| 6 | 퀴즈 + 노트 | quiz / one-pager | |


---


## 핵심 개념


### 1. CLIP 의 한 줄 요약


> "이미지와 텍스트를 **공통 vector space** 에 매핑한 model. Image-text contrastive learning (OpenAI, 2021)."


### 2. Architecture


```
Image Text
  | |
  v v
+-----------+ +-----------+
| ViT-L/14 | | Text |
| (image | | Transformer|
| encoder) | | (text |
| | | encoder) |
+-----------+ +-----------+
       | |
       v v
+-----------+ +-----------+
| MLP head | | MLP head |
+-----------+ +-----------+
       | |
       v v
   image vector text vector
   (512 dim) (512 dim)
       | |
       +-------+---------+
               |
               v
       cosine similarity
       (또는 InfoNCE loss 의 logit)
```


### 3. Contrastive Learning (InfoNCE Loss)


학습 시 한 batch 의 N 개 (image, text) pair:


```
correct pair: (image_i, text_i) -> 높은 similarity
wrong pair: (image_i, text_j, j != i) -> 낮은 similarity


InfoNCE:
  L = -log( exp(sim(I_i, T_i)) / sum_j exp(sim(I_i, T_j)) )
```


batch 안에서 cross-entropy. batch 가 커야 (4096+) 효과 큼.


### 4. CLIP 의 학습 데이터


- WIT (Web Image-Text): 400M pair (OpenAI scraping)
- 다양한 domain, 다양한 언어 (단 영어 위주)
- alt-text 자동 caption 사용


### 5. CLIP variant (OpenAI / OpenCLIP)


| 모델 | Image encoder | Text encoder | Params |
|---|---|---|---|
| CLIP ViT-B/32 | ViT-B (patch 32) | Transformer | 151M |
| CLIP ViT-B/16 | ViT-B (patch 16) | Transformer | 150M |
| CLIP ViT-L/14 | ViT-L (patch 14) | Transformer | 428M |
| CLIP ViT-H/14 | ViT-H (patch 14) | Transformer | 986M |


### 6. CLIP 의 "open-vocabulary" 의미


기존 분류 모델: 1000 classes (ImageNet) - 학습된 label 만 가능
CLIP: 임의 텍스트 prompt -> image 와 비교 가능


```python
prompts = ["a photo of a cat", "a photo of a dog", "a photo of a banana"]
image = ...


# CLIP image encoder -> image_vec
# CLIP text encoder -> 각 prompt 의 text_vec
# similarity 계산 -> 가장 높은 prompt 선택
```


새 class 추가 = 새 prompt 만들기 (재학습 불필요). 이게 zero-shot classification.


### 7. Robot 에서의 CLIP


VLA / OpenVLA 에서 CLIP 직접 사용은 안 하지만 영향:
- SigLIP 의 직접 조상 (다음 모델, week 10)
- "open-vocabulary" 개념 자체가 VLA 의 emergent capability 의 기반
- "pick up the red can" 명령에서 'red can' 의 visual 매칭에 비슷한 원리


### 8. CLIP 의 한계


1. Caption quality 의존 (alt-text 가 noisy)
2. 영어 위주
3. Fine-grained 인식 약함 (개의 품종 등)
4. Long text 처리 약함 (보통 77 token limit)
5. Spatial reasoning 약함 (객체 위치 등)


이 한계들이 SigLIP / DINOv2 등 후속 모델의 동기.


---


## 자체 점검


**Q1. CLIP 의 가장 큰 contribution?**
> Image-text contrastive learning 으로 "open-vocabulary" 가능. 임의 텍스트 prompt 로 zero-shot 분류 가능.


**Q2. CLIP-L/14 의 parameter?**
> 약 428M. Image encoder ViT-L (300M) + text encoder transformer (~ 128M).


**Q3. InfoNCE loss 의 핵심?**
> 한 batch 의 N pair 에서 correct 는 높은 sim, wrong (N-1 개) 는 낮은 sim. cross-entropy.


**Q4. CLIP 의 한계 3 가지?**
> Caption noise / 영어 위주 / fine-grained 약함 / long text 한계 / spatial reasoning 약함.


**Q5. CLIP 과 OpenVLA 의 관계?**
> 직접 사용은 안 하지만 SigLIP 의 조상이며 "open-vocabulary" 의 개념 자체가 VLA 의 emergent capability 의 기반.


---


## 실습 + 다음 주


### 이번 주
- CLIP 논문 정독
- 실습 1, 2
- one-pager
- quiz


### 다음 주
- HuggingFace CLIP 모델 사용
- image-text similarity 직접 측정


---


## 핵심 요약


1. **CLIP = image + text 를 공통 vector space 에 매핑**
2. **InfoNCE contrastive loss**: batch 안에서 cross-entropy
3. **WIT 400M pair** 학습 (OpenAI)
4. **Open-vocabulary**: 임의 prompt 로 zero-shot
5. **한계** caption / 영어 / fine-grained / long text / spatial


---


- 이전: [Week 3](../week3/README.md) | 다음: [Week 5 - CLIP inference](../week5/README.md)
