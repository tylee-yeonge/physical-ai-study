# Week 8: HuggingFace DINOv2 inference + Patch Feature 시각화


> **이번 주 목표**: DINOv2 의 patch-level feature 를 시각화하여 spatial 정보의 본질을 본다. CLIP / SigLIP 와 직접 비교.
> **예상 시간**: 6시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 | `requirements.txt` | torch, sklearn (PCA) |
| 2 | DINOv2 inference | `PRACTICE.md` 1 | ViT-L 사용 |
| 3 | PCA 시각화 | `PRACTICE.md` 2 | patch feature 의 3 channel |
| 4 | CLIP vs DINOv2 비교 | `PRACTICE.md` 3 | 같은 image 의 feature 차이 |
| 5 | 노트 + 퀴즈 | one-pager | |


---


## 핵심 개념


### 1. Patch feature 의 시각화 방법


```
DINOv2 output: (1, N+1, 1024) # CLS + N patch tokens
- N = 256 (224x224, patch 14)


PCA 로 3 dim 축소:
  patch_feat (256, 1024) -> PCA -> (256, 3)
  -> (16, 16, 3) reshape
  -> RGB 이미지 처럼 시각화
```


결과: object 구분, segmentation 비슷한 효과.


### 2. CLIP / DINOv2 / SigLIP 의 직접 비교


같은 image 입력 시:


| Encoder | Patch feature 시각화 결과 |
|---|---|
| CLIP | semantic cluster (cat=같은 색, dog=다른 색) |
| DINOv2 | spatial cluster (object boundary 명확) |
| SigLIP | CLIP 와 비슷 (semantic 강조) |


본 주 권장: 세 모델 모두 시각화 후 비교.


### 3. DINOv2 feature 의 robot 응용


```
DINOv2 patch token (1024 dim) 활용:
1. Robot 환경의 객체 segmentation (without label)
2. 객체 pose estimation (geometric 정보)
3. Grasp affordance (어디를 잡을 수 있는가)
4. SLAM / 3D reconstruction 의 feature
```


OpenVLA 안에서: spatial reasoning 의 토대.


### 4. DINOv2 latency / VRAM (RTX 4070)


| Model | Params | Latency | VRAM |
|---|---|---|---|
| ViT-S/14 | 22M | ~ 5 ms | 0.2 GB |
| ViT-B/14 | 86M | ~ 12 ms | 0.5 GB |
| **ViT-L/14** | **300M** | **~ 28 ms** | **1.2 GB** |
| ViT-G/14 | 1.1B | OOM (fp16) | - |


OpenVLA 는 ViT-L 사용 (3번째). Latency 28 ms.


### 5. DINOv2 의 fine-tune (선택)


본 phase 에서는 fine-tune 안 함. 단:
- LoRA fine-tune: 새 robot 환경 적응
- Linear probe: 위에 작은 classifier
- DPT (depth) / DINOv2-segment 등 응용 모델 있음


### 6. DINOv2 vs SAM (Segment Anything Model)


| 모델 | 학습 방식 | 출력 |
|---|---|---|
| DINOv2 | self-supervised | patch feature (general) |
| SAM | label-rich segmentation 학습 | segmentation mask |


DINOv2 는 일반 feature, SAM 은 segmentation 특화.


OpenVLA 는 DINOv2 사용 (일반 feature 가 LLM 과 결합 자연스러움).


---


## 자체 점검


**Q1. Patch feature 의 PCA 시각화 결과?**
> DINOv2: spatial cluster (object 경계 명확). CLIP: semantic cluster (object identity).


**Q2. DINOv2 ViT-L latency?**
> ~ 28 ms (RTX 4070 fp16). OpenVLA vision encoder 의 절반.


**Q3. DINOv2 의 robot 응용 4 가지?**
> Segmentation / pose estimation / grasp affordance / SLAM feature.


**Q4. DINOv2 vs SAM 의 차이?**
> DINOv2 = self-supervised general feature. SAM = segmentation 특화. OpenVLA 는 DINOv2 선택 (general).


**Q5. ViT-G/14 가 OpenVLA 에 안 쓰이는 이유?**
> 1.1B params -> VRAM 부족 + latency 큼. ViT-L (300M) 이 balance.


---


## 실습 + 다음


### 이번 주
- DINOv2 inference + PCA 시각화
- CLIP / SigLIP / DINOv2 비교
- one-pager


### 다음 주 (week 9)
- OpenVLA 의 vision backbone 으로서의 종합 분석
- Phase 4 와의 직접 연결


---


## 핵심 요약


1. **DINOv2 patch feature = spatial cluster** (시각화)
2. **CLIP/SigLIP = semantic cluster** (보완적)
3. **ViT-L latency ~ 28ms** OpenVLA 의 절반
4. **Robot 응용**: segmentation / pose / grasp / SLAM
5. **DINOv2 vs SAM**: general vs segmentation 특화


- 이전: [Week 7](../week7/README.md) | 다음: [Week 9](../week9/README.md)
