# Week 7: DINOv2 논문 정독 + Self-Distillation


> **이번 주 목표**: DINOv2 의 self-supervised learning (self-distillation, masked image modeling) 을 이해. OpenVLA 의 vision backbone 의 절반.
> **예상 시간**: 8시간
> **핵심 질문**: "Text label 없이 어떻게 vision encoder 를 학습하는가? DINOv2 의 spatial 정보가 robot 에 왜 필요한가?"


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 | `requirements.txt` | torch, transformers |
| 2 | DINOv2 논문 1회독 | https://arxiv.org/abs/2304.07193 | 22 페이지 |
| 3 | Self-distillation 이해 | `README.md` 3장 | Teacher-Student |
| 4 | Architecture 분석 | `PRACTICE.md` 1 | encoder + losses |
| 5 | inference + feature 시각화 | `PRACTICE.md` 2 | patch level |
| 6 | 퀴즈 + 노트 | one-pager | |


---


## 핵심 개념


### 1. DINOv2 한 줄


> "Text label 없이 image 만으로 학습된 vision encoder (self-supervised). Spatial / geometric 정보가 강함."


### 2. Self-Supervised Learning (SSL) 의 큰 그림


```
일반 학습 (supervised) : (image, label) pair 필요
DINOv2 (self-supervised) : image 만, label 없음
                           pretext task 로 학습
```


DINOv2 의 pretext task:
- Self-distillation (DINO 의 핵심)
- Masked image modeling
- iBOT-style global + local view


### 3. Self-Distillation (DINO 의 핵심)


```
한 image 의 두 augmentation:
  view_1 (global crop)
  view_2 (global crop)
  view_local_1, _2 (local crop)


Teacher network: 한 view 의 representation
Student network: 다른 view 의 representation


Loss: student 가 teacher 의 representation 예측
      (cross-entropy between distributions)


Teacher 는 student 의 EMA (exponential moving average)
```


EMA = teacher 가 student 의 느린 trajectory 따라감. 학습 collapse 방지.


### 4. iBOT-style masked image modeling


```
한 view 의 patch 일부를 mask
-> student 가 mask 된 patch 를 예측
-> teacher 의 unmasked feature 와 비교
```


이 trick 이 spatial feature 의 quality 를 크게 향상.


### 5. Architecture (DINOv2 ViT-L)


```
Image (224, patch 14) -> ViT-L (24 layer, 1024 hidden, 16 heads, 300M params)
  -> CLS token + patch token sequence
  -> Loss 적용 (self-distillation + iBOT)
```


본질적으로 ViT 와 동일. 차이는 **학습 방식** (SSL).


### 6. DINOv2 의 spatial feature 강점


```
DINOv2 patch token 시각화:
  - 비슷한 object 가 같은 color cluster
  - 같은 background 가 같은 cluster
  - segmentation 의 직접 토대
```


CLIP / SigLIP 의 token: semantic 정보 중심 (object identity).
DINOv2 의 token: spatial 정보 중심 (객체 위치, geometric 관계).


### 7. DINOv2 의 학습 데이터


LVD-142M: 142M 이미지 (label 없음, web scraping + curated).
ImageNet-1k 의 100x 크기.


### 8. OpenVLA 에서의 DINOv2 역할


```
RGB image
  |
  v
DINOv2 (patch 14) -> 256 spatial tokens
  |
  +-- concat with SigLIP semantic tokens
  |
  v
projector -> Llama hidden
```


"객체가 어디 있는가" 의 정보를 LLM 에 전달.


### 9. DINOv2 의 한계


1. 학습이 supervised 보다 느림 (data 더 필요)
2. CLIP / SigLIP 보다 semantic 정보 약함
3. 영상 / 시간 정보 없음
4. Multi-modal 자체로는 약함 (text 와 결합 시 추가 layer 필요)


### 10. DINOv2 variant


| 모델 | Params | 용도 |
|---|---|---|
| ViT-S/14 (DINOv2) | 22M | 가벼운 inference |
| ViT-B/14 | 86M | balance |
| ViT-L/14 | 300M | OpenVLA backbone |
| ViT-G/14 | 1.1B | 최대 |


---


## 자체 점검


**Q1. Self-distillation 의 원리?**
> 같은 image 의 두 view 의 representation 이 같아지도록 student 학습. Teacher 는 student 의 EMA.


**Q2. DINOv2 vs CLIP 의 차이?**
> DINOv2: text label 없이 self-supervised, spatial 정보 강함. CLIP: image-text pair, semantic 정보 강함.


**Q3. DINOv2 학습 데이터?**
> LVD-142M (142M 이미지, label 없음). 자동 curation.


**Q4. DINOv2 의 patch_size?**
> 14 (ViT 표준 16 과 다름). spatial resolution 약간 더 fine.


**Q5. OpenVLA 에서 DINOv2 의 역할?**
> Vision encoder 중 하나 (SigLIP 와 hybrid). 객체의 spatial 정보 (어디 있는가) 를 LLM 에 전달.


---


## 실습 + 다음 주


### 이번 주
- 논문 정독
- inference + feature 시각화
- one-pager
- quiz


### 다음 주 (week 8)
- HF DINOv2 feature 시각화 더 자세히
- patch attention map


---


## 핵심 요약


1. **Self-supervised SSL**: text label 없이 image 만으로
2. **Self-distillation + iBOT**: DINOv2 의 핵심 trick
3. **Spatial 정보** 강점 (CLIP 의 semantic 과 보완)
4. **ViT-L/14 (300M)** OpenVLA backbone
5. **LVD-142M** 학습 데이터


- 이전: [Week 6](../week6/README.md) | 다음: [Week 8 - DINOv2 inference + viz](../week8/README.md)
