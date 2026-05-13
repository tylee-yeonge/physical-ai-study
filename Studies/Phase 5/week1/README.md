# Week 1: ViT (Vision Transformer) 논문 정독 + Architecture

> [goal] **이번 주 목표**: ViT 의 patch embedding + self-attention 구조를 한 페이지로 설명할 수 있는 수준에 도달. 모든 vision Transformer 의 토대.
> [time] **예상 시간**: 10시간 (논문 정독 5h + 시각화 3h + 노트 2h)
> [tip] **핵심 질문**: "이미지를 어떻게 token 으로 만드는가? Token 으로 만들면 무엇이 가능해지는가?"

---

## [list] 학습 순서

| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | 환경 | `requirements.txt` | torch, transformers |
| 2 | 사전 지식 점검 | `README.md` 2장 | Transformer / CNN 의 차이 |
| 3 | ViT 논문 1회독 | https://arxiv.org/abs/2010.11929 | 13 페이지 |
| 4 | Patch embedding 실습 | `PRACTICE.md` 1~2 | 직접 patch -> token |
| 5 | Self-attention 시각화 | `PRACTICE.md` 3 | attention map |
| 6 | 퀴즈 | quiz_easy / quiz_medium | 구조 / 수치 |
| 7 | 한 페이지 노트 | `PRACTICE.md` 4 | "ViT one-pager" |

---

## [*] Phase 5 의 위치

Phase 5 (Foundation Model 기초) 는 Phase 4 의 **사전 지식** 채우기:

```
Phase 4 (이전): OpenVLA 통합  - DINOv2 + SigLIP + Llama 7B 의 결합
       |
       +-- 이게 어떻게 동작하는지 깊이 모름
       |
Phase 5 (이번): 각 컴포넌트의 동작 원리
       |
       +-- ViT (모든 vision Transformer 의 토대)
       +-- CLIP (vision-language alignment)
       +-- DINOv2 (self-supervised, spatial)
       +-- SigLIP (CLIP 의 sigmoid 변종, semantic)
       |
Phase 6, 7 (다음): 이 지식으로 Real-to-Sim-to-Real 통합
```

**깊이 주의**: 본 phase 의 "동작 원리 수준" = 아키텍처 다이어그램 + 학습 방식 + 입출력 인터페이스 설명 가능. **직접 학습 / fine-tune 은 안 함**.

---

## [ref] 핵심 개념

### 1. ViT 가 가져온 혁명: "Image -> Token sequence"

```
이전 (CNN):
  Image -> conv -> conv -> pool -> conv ... -> feature map
  공간 정보 손실, receptive field 확장 어려움

ViT (Transformer):
  Image -> 16x16 patch -> linear projection -> token sequence
  ↓
  Transformer (self-attention)
  ↓
  Output token (CLS) for classification
```

핵심: 이미지를 **token sequence** 로 바꾸면, 자연어처럼 Transformer 에 입력 가능.

### 2. Patch Embedding 의 정확한 과정

```
Input: RGB image, shape (3, 224, 224)
       ↓
Step 1: 16x16 patch 로 분할
       224 / 16 = 14 -> 14x14 = 196 개 patch
       각 patch: (3, 16, 16) = 768 dim 벡터
       ↓
Step 2: Linear projection
       (768) -> (D=768) (또는 다른 hidden dim)
       ↓
Step 3: position embedding 더하기
       각 patch 의 위치 정보 (1, 2, ..., 196)
       ↓
Step 4: [CLS] token prepend
       전체 sequence: (197, D) 의 token sequence
```

이게 ViT 의 모든 입력. CNN 의 stem (첫 conv layer) 대신 patch + linear.

### 3. Architecture Diagram

```
Image (3, 224, 224)
       |
       v
+----------------+
| Patch Embed    | (Conv2d 16x16 stride 16)
+----------------+
       |
       v
[CLS] + 196 patch tokens (총 197, dim=768)
       |
       + position embedding
       |
       v
+----------------+
| Transformer    | x N layers (보통 12)
| - Multi-head   |
|   self-attn    |
| - MLP          |
| - LayerNorm    |
+----------------+
       |
       v
[CLS] token (1, 768) -- 분류용
patch tokens (196, 768) -- spatial features
       |
       v
+----------------+
| Classifier MLP | -> logits (num_classes)
+----------------+
```

### 4. Self-Attention 의 한 줄 요약

```
Self-attention: 모든 token 쌍의 관계를 한 번에 학습.
- CNN 의 local receptive field 제약 없음
- 임의 두 patch 의 직접 관계 모델링 가능
- 단점: 계산량 O(N^2) (N = token 수)
```

수식 핵심:

```
Q = X * W_q  (query)
K = X * W_k  (key)
V = X * W_v  (value)

Attention = softmax(Q * K^T / sqrt(d)) * V
```

Multi-head: 여러 sub-space 에서 attention 동시 계산.

### 5. ViT 의 크기 별 variant

| 모델 | Layers | Hidden dim | Heads | Params |
|---|---|---|---|---|
| ViT-Ti (Tiny) | 12 | 192 | 3 | 5.7M |
| ViT-S (Small) | 12 | 384 | 6 | 22M |
| ViT-B (Base) | 12 | 768 | 12 | 86M |
| ViT-L (Large) | 24 | 1024 | 16 | 307M |
| ViT-H (Huge) | 32 | 1280 | 16 | 632M |
| ViT-22B | 48 | 6144 | 48 | 22B (PaLI-X 의 일부) |

OpenVLA 의 DINOv2 / SigLIP 은 모두 **ViT-L (300M)** 기반.

### 6. ViT 학습의 trick

```
1. 대량 데이터 (ImageNet-21k or JFT-300M) 사전학습
2. ImageNet-1k 로 fine-tune
3. AdamW + warmup + cosine schedule
4. mixup / cutmix augmentation
5. label smoothing
```

CNN 보다 데이터가 더 필요. 작은 데이터 (~100K) 로는 ResNet 보다 낮음. 대량 데이터에서만 advantage.

### 7. ViT 의 가장 중요한 implications

1. **다른 modality 와 자연스러운 결합**: text token + image token 그대로 concat 가능 (VLM 의 기반)
2. **scale 잘 됨**: 모델 / 데이터 키울수록 성능 ↑ (CNN 은 plateau)
3. **spatial feature 추출**: patch token 자체가 spatial 정보 보존 (segmentation / detection 에 응용)
4. **transfer learning**: 다양한 downstream task 에 잘 transfer

이 4 가지가 ViT 가 모든 modern foundation model 의 토대인 이유.

### 8. CNN vs ViT 비교 표

| 항목 | CNN | ViT |
|---|---|---|
| Inductive bias | translation invariance | 없음 |
| 데이터 효율 | 좋음 (100K 도 OK) | 나쁨 (수백만 필요) |
| Receptive field | local -> global (depth 필요) | 첫 layer 부터 global |
| Scale | plateau | linear 향상 |
| Vision modality 결합 | 추가 trick 필요 | 자연스러움 |
| Spatial feature | feature map | patch token |
| GPU 메모리 | conv 효율적 | sequence 길이 N^2 |

### 9. OpenVLA 와의 직접 연결

OpenVLA 의 vision encoder:
- DINOv2 (ViT-L, self-supervised, 300M)
- SigLIP (ViT-L, image-text contrastive, 300M)

둘 다 ViT-L 기반. 본 phase 의 week 7~12 에서 자세히. 본 주는 **ViT 자체** 만.

---

## [search] 자체 점검

**Q1. Patch Embedding 의 정확한 과정은?**
> 1) 이미지를 16x16 patch 로 분할 (224x224 -> 196개), 2) 각 patch 를 linear projection 으로 D 차원 벡터로, 3) position embedding 더함, 4) [CLS] token prepend -> total (197, D) sequence.

**Q2. Self-attention 의 시간 복잡도는?**
> O(N^2 * D). N=token 수, D=hidden dim. 이미지 크기가 커지면 N 폭증 (예: 1024x1024 image, 16x16 patch -> N=4096, sequence^2 = 16M).

**Q3. ViT 가 CNN 보다 데이터를 더 많이 필요로 하는 이유?**
> CNN 은 translation invariance 등 inductive bias 가 내장. ViT 는 그런 bias 없어 데이터로 학습해야 함. 작은 데이터셋에서는 ResNet 보다 낮음.

**Q4. ViT-L 의 parameter 수?**
> 약 300M (307M). 24 layer, hidden 1024, 16 heads. OpenVLA 의 DINOv2 / SigLIP 도 ViT-L.

**Q5. Multi-head attention 의 의미?**
> 같은 sequence 를 여러 sub-space (head) 에서 attention 동시 계산. 각 head 가 다른 패턴 학습 가능 (예: 한 head 는 spatial, 다른 head 는 semantic). ViT-B 의 12 heads = 768/12 = 64 dim/head.

---

## [note] 이번 주 실습 & 다음 주 준비

### 이번 주 실습 과제
1. ViT 논문 1회독 (13페이지, 가벼움)
2. `practice_patch_embed.py` - 직접 patch embedding 구현
3. `practice_vit_inference.py` - HuggingFace ViT-B 로 ImageNet 분류
4. `practice_attention_viz.py` - attention map 시각화
5. ViT one-pager 노트
6. quiz_easy / quiz_medium

### 다음 주 (week 2) 준비
- HuggingFace transformers 의 ViT 모델 카드 확인
- 다양한 image input 으로 inference test

---

## [goal] 이번 주 핵심 요약

1. **Image -> Token sequence**: ViT 의 핵심 아이디어.
2. **Patch embedding**: 16x16 patch + linear projection + position embed + CLS.
3. **Self-attention O(N^2)**: 메모리 / 속도 한계.
4. **데이터 의존**: 대량 데이터 필수, 작은 데이터셋에선 CNN 보다 낮음.
5. **OpenVLA 의 DINOv2 / SigLIP 은 모두 ViT-L (300M)**.

---

[O] 이전: [Phase 4 - VLA 논문 + ROS2 demo](../../../Roadmap/Phase%204.md)

다음: [Week 2 - HuggingFace ViT inference + ImageNet 분류](../week2/README.md)
