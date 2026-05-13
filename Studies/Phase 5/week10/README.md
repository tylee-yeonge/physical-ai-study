# Week 10: SigLIP 논문 정독 + Sigmoid Loss


> **이번 주 목표**: SigLIP 의 sigmoid loss 가 CLIP 의 softmax 와 어떻게 다른지 이해. OpenVLA semantic encoder 의 절반.
> **예상 시간**: 8시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 | `requirements.txt` | torch, transformers |
| 2 | SigLIP 논문 정독 | https://arxiv.org/abs/2303.15343 | 22 페이지 |
| 3 | Sigmoid vs Softmax 이해 | `README.md` 3장 | loss 비교 |
| 4 | SigLIP inference | `PRACTICE.md` 1 | HF model 사용 |
| 5 | CLIP vs SigLIP 비교 | `PRACTICE.md` 2 | 같은 image 의 similarity |
| 6 | 퀴즈 + 노트 | one-pager | |


---


## 핵심 개념


### 1. SigLIP 한 줄


> "CLIP 의 sigmoid 변종. softmax 대신 sigmoid pairwise loss 사용 -> batch 의존성 적음, 학습 효율 좋음."


### 2. Softmax (CLIP) vs Sigmoid (SigLIP) Loss


CLIP InfoNCE (softmax):
```
L = -log( exp(sim(i,j)) / sum_k exp(sim(i,k)) )


특성: batch 의 negative 들이 서로 영향 (denominator)
요구: large batch (4K+) 효과적
```


SigLIP (sigmoid pairwise):
```
For each pair (i, j):
  if i == j: target = 1 (positive pair)
  else: target = 0 (negative pair)


L = sum_{i,j} BCE( sigmoid(sim(i,j)), target_{i,j} )


특성: 각 pair 가 독립
요구: small batch (1K) 도 OK
```


### 3. SigLIP 의 가능성


```
batch 1024 에서:
  CLIP: 학습 효과 약함 (4K+ 권장)
  SigLIP: 학습 효과 충분


-> SigLIP 가 더 작은 batch 로 학습 가능
-> compute 효율적 -> 더 큰 모델 학습 가능
```


### 4. Architecture (CLIP 와 거의 같음)


```
Image Encoder (ViT) -> projection -> image_vec
Text Encoder -> projection -> text_vec
Loss: sigmoid pairwise (CLIP 와 차이)
```


차이는 **loss function 만**.


### 5. SigLIP variant


| 모델 | Patch | Image size | Params |
|---|---|---|---|
| siglip-base-patch16-224 | 16 | 224 | 200M |
| siglip-large-patch16-256 | 16 | 256 | 643M |
| **siglip-so400m-patch14-384** | 14 | 384 | 877M |


마지막이 OpenVLA 의 vision encoder (SigLIP 의 가장 큰 버전).
OpenVLA 의 SigLIP 은 정확히 google/siglip-large-patch16-256 또는 siglip-so400m.


### 6. CLIP -> SigLIP 의 정확도 비교


| Task | CLIP | SigLIP |
|---|---|---|
| ImageNet zero-shot | 75% | 76% |
| COCO retrieval | 비슷 | 약간 우위 |
| 학습 비용 | high | lower |
| 작은 batch | 약함 | 잘 동작 |


성능 비슷, 효율 차이.


### 7. OpenVLA 에서의 SigLIP 역할


```
SigLIP 의 patch tokens (196) -> OpenVLA 의 semantic 정보
+ DINOv2 의 spatial tokens (256) -> spatial 정보
-> 함께 Llama 에 입력
```


DINOv2 와 SigLIP 의 **보완 관계**:
- 같은 ViT-L 구조이지만 학습 데이터 / 방식이 달라 강점 다름.
- 두 정보 합치면 robot manipulation 의 모든 정보 cover.


### 8. SigLIP 의 한계


CLIP 와 비슷:
1. spatial reasoning 약함
2. fine-grained 약함
3. OCR 약함
4. dynamic scene 약함


이 한계가 OpenVLA 가 DINOv2 와 함께 hybrid 사용하는 이유.


---


## 자체 점검


**Q1. SigLIP 의 CLIP 대비 핵심 차이?**
> Loss function. softmax InfoNCE (CLIP) vs sigmoid pairwise (SigLIP).


**Q2. Sigmoid loss 의 장점?**
> Batch 의존성 적음 -> 작은 batch (1K) 도 학습 가능 -> compute 효율.


**Q3. OpenVLA 가 사용하는 SigLIP variant?**
> SigLIP-L (또는 SigLIP-SO400M). ViT-L/16 또는 patch 14, 224/256/384 input.


**Q4. SigLIP 의 zero-shot 정확도?**
> CLIP 와 비슷 (76% on ImageNet). 효율 차이가 더 큰 이득.


**Q5. SigLIP 의 robot 한계?**
> CLIP 와 같음. Spatial / fine-grained / OCR / dynamic 약함. -> DINOv2 와 hybrid.


---


## 실습 + 다음 주


### 이번 주
- SigLIP 논문 정독
- HF SigLIP inference
- CLIP vs SigLIP 비교
- one-pager


### 다음 주 (week 11)
- OpenVLA 의 DINOv2 + SigLIP 통합 종합 이해


---


## 핵심 요약


1. **SigLIP = sigmoid pairwise loss** (CLIP 의 softmax InfoNCE 와 차이)
2. **작은 batch 로 학습 가능** -> compute 효율
3. **정확도 CLIP 와 비슷**, 효율 더 좋음
4. **OpenVLA semantic encoder** 로 SigLIP 사용
5. **DINOv2 와 hybrid** = spatial + semantic 모두 cover


- 이전: [Week 9](../week9/README.md) | 다음: [Week 11](../week11/README.md)
