# Week 11: OpenVLA DINOv2 + SigLIP 통합 - 완전한 이해


> **이번 주 목표**: Phase 5 의 4 모델 (ViT/CLIP/DINOv2/SigLIP) 통합. OpenVLA vision backbone 모든 component 설명 가능.
> **예상 시간**: 6시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 4 모델 종합 | `README.md` 2장 | 비교 |
| 2 | OpenVLA backbone 최종 | `PRACTICE.md` 1 | 다이어그램 |
| 3 | Latency / VRAM | `PRACTICE.md` 2 | 모든 수치 |
| 4 | Phase 5 종합 노트 | `PRACTICE.md` 3 | one-pager |
| 5 | 퀴즈 | | |


---


## 핵심 개념 - Phase 5 종합


### 1. 4 모델 비교


| 모델 | 학습 | 강점 | OpenVLA |
|---|---|---|---|
| ViT (2020) | supervised | scale | 토대 |
| CLIP (2021) | image-text contrastive | semantic + open-vocab | 안 씀 (직접) |
| DINOv2 (2023) | self-supervised | spatial | spatial encoder |
| SigLIP (2023) | image-text sigmoid | semantic + 효율 | semantic encoder |


### 2. OpenVLA Vision Backbone spec


```
DINOv2 ViT-L/14 (300M):
  patch 14, image 224 -> 256 patches
  spatial features


SigLIP ViT-L/16 (300M):
  patch 16, image 224 -> 196 patches
  semantic features


Fusion:
  452 patch tokens -> projector MLP -> 4096 dim
  + Llama prompt
```


### 3. Latency 종합 (RTX 4070)


| Component | Params | Latency | VRAM |
|---|---|---|---|
| DINOv2 ViT-L | 300M | 28 ms | 1.2 GB |
| SigLIP ViT-L | 300M | 28 ms | 1.2 GB |
| Projector | 30M | 2 ms | 0.1 GB |
| Llama 7B int4 | 7B | 95 ms | 4 GB |
| Total | ~7.6B | ~165 ms | ~6.5 GB |


### 4. Training 매트릭스


| Component | Pre-train | OpenVLA fine-tune | Phase 7 LoRA |
|---|---|---|---|
| DINOv2 | Meta LVD-142M | frozen | frozen |
| SigLIP | Google WebLI | frozen | frozen |
| Projector | random init | 학습 (~30M) | 학습 |
| Llama | Meta | 부분 학습 | LoRA r=32 (~65M) |


Phase 7 trainable: ~95M (~ 1.25%).


### 5. Phase 6, 7 진입 준비


본 주가 Phase 5 의 사실상 종합. week 12 는 Phase 4 demo 보강 + 회고.


Phase 6 (Isaac Sim) / Phase 7 (Real-to-Sim-to-Real) 의 vision 토대 = 본 phase.


---


## 자체 점검


**Q1. OpenVLA vision encoder 총 params?**
> ~630M (DINOv2 300 + SigLIP 300 + projector 30).


**Q2. LoRA trainable 비중?**
> ~95M / 7.6B = ~1.25%.


**Q3. Inference latency breakdown?**
> Vision 56 + Projector 2 + LM 95 + 기타 12 = 165 ms.


**Q4. 4 vision foundation 모델 중 OpenVLA 가 직접 안 쓰는 것?**
> CLIP. SigLIP 가 후속.


**Q5. ViT 의 역할?**
> 모든 vision foundation model 의 토대. patch embed + self-attention.


---


## 실습 + 다음 주


### 이번 주
- 4 모델 비교 표
- OpenVLA backbone 종합 다이어그램
- 한 페이지 통합 노트
- quiz


### 다음 주 (week 12 - 마지막)
- Phase 4 demo 보강
- Phase 6 진입 준비
- Phase 5 회고


---


## 핵심 요약


1. **4 vision foundation 모델** 의 정리
2. **OpenVLA vision encoder ~630M**
3. **Latency 165 ms** breakdown
4. **LoRA trainable ~95M (1.25%)**
5. **Phase 6/7 vision 토대 완성**


- 이전: [Week 10](../week10/README.md) | 다음: [Week 12](../week12/README.md)
