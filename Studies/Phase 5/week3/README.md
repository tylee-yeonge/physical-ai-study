# Week 3: OpenVLA backbone 과의 관계 + Phase 4 latency 분석

> [goal] **이번 주 목표**: ViT 의 이해 (week 1, 2) 와 OpenVLA 의 architecture (Phase 4 week 4) 를 연결. Phase 4 의 ROS2 demo latency 를 vision encoder 관점으로 재분석.
> [time] **예상 시간**: 6시간
> [tip] **핵심 질문**: "OpenVLA 의 165 ms 중 vision (DINOv2 + SigLIP) 의 정확한 비중은?"

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | OpenVLA architecture 재정독 | Phase 4 week 4 | hybrid vision encoder |
| 2 | Vision encoder latency 측정 | `PRACTICE.md` 1 | DINOv2 + SigLIP 분리 |
| 3 | Latency breakdown | `PRACTICE.md` 2 | Vision vs LM vs 기타 |
| 4 | 정리 노트 | `PRACTICE.md` 3 | "ViT -> OpenVLA" |
| 5 | 퀴즈 | quiz_easy / quiz_medium | architecture |

---

## [ref] 핵심 개념

### 1. OpenVLA Vision Encoder 구조 복습

```
RGB Image (224 or 384)
   |        |
   v        v
DINOv2    SigLIP
(ViT-L,   (ViT-L,
 patch14) patch16)
   |        |
   v        v
spatial   semantic
features  features
   |        |
   +---+----+
       v
   concat + projector (MLP)
       v
   Llama 2 7B input
```

### 2. 두 encoder 의 정확한 spec

| 항목 | DINOv2 | SigLIP |
|---|---|---|
| Architecture | ViT-L | ViT-L |
| Patch size | 14 | 16 |
| Hidden dim | 1024 | 1024 |
| Layers | 24 | 24 |
| Params | ~ 300M | ~ 300M |
| Input | 224 또는 384 | 224 또는 384 |
| Output token | spatial (CLS 제외) | semantic (CLS 포함 가능) |

### 3. Latency breakdown (예상)

Phase 4 week 6 의 165 ms 분해:

| Component | Latency | 비중 |
|---|---|---|
| Image preprocess | ~ 5 ms | 3% |
| DINOv2 forward | ~ 28 ms | 17% |
| SigLIP forward | ~ 28 ms | 17% |
| Projector (MLP) | ~ 2 ms | 1% |
| LM decoder generate | ~ 95 ms | 58% |
| Action de-tokenize | ~ 1 ms | <1% |
| 기타 | ~ 6 ms | 3% |

Vision (~ 58 ms) + LM (~ 95 ms) = 93%.

### 4. 두 encoder 의 fusion 방식

```
DINOv2: (B, N_d, 1024)   # N_d = 257 (patch14, 224)
SigLIP: (B, N_s, 1024)   # N_s = 197 (patch16, 224)

방식 A: sequence concat
  -> (B, N_d + N_s, 1024) = (B, 454, 1024)
  -> projector (MLP) -> (B, 454, 4096)  # Llama dim

방식 B: token alignment (각 spatial 위치 정렬)
  -> 더 복잡, OpenVLA 는 안 씀
```

OpenVLA 는 **방식 A (단순 concat)**.

### 5. 병렬화 가능성

DINOv2 와 SigLIP 은 독립적. 이론상 병렬:

```
순차: DINOv2 (28) -> SigLIP (28) = 56 ms
병렬: max(28, 28) = 28 ms (이론)
```

RTX 4070 SM 부족으로 실제 절약 ~ 5~10 ms.

### 6. ViT 지식이 OpenVLA 에 그대로 적용

| ViT 지식 | OpenVLA 에서 |
|---|---|
| Patch embedding | DINOv2 / SigLIP 첫 layer |
| Self-attention | 양쪽 모든 layer |
| Position embedding | 양쪽 입력 |
| Multi-head | 각 encoder 16 head |
| Layer norm | 각 layer 마지막 |

### 7. 본 주 결과물

- OpenVLA architecture 다이어그램 재작성 (본인 손그림)
- Component breakdown 표 1 페이지
- "ViT 지식 -> OpenVLA" 노트

---

## [search] 자체 점검

**Q1. OpenVLA vision encoder 의 총 parameter?**
> ViT-L * 2 = 600M (Llama 2 7B 의 ~8.5%).

**Q2. Vision latency 비중?**
> 165 ms 중 ~ 58 ms (35%). LM generate (~ 95 ms, 58%) 다음.

**Q3. 두 encoder fusion 방식?**
> Sequence concat 후 projector MLP (단순). OpenVLA 표준.

**Q4. 병렬화 효과?**
> 이론 50% (56 -> 28 ms), 실제 4070 에서 5~10 ms 절약 (SM 부족).

**Q5. ViT 지식의 직접 응용?**
> patch embed / attention / pos embed 가 OpenVLA vision 의 모든 layer 에 그대로.

---

## [note] 이번 주 실습 & 다음 주 준비

### 실습
1. OpenVLA architecture 다이어그램 재작성
2. Latency breakdown 표
3. quiz

### 다음 주
- CLIP 논문 다운로드
- Contrastive learning 사전 지식

---

## [goal] 핵심 요약

1. **OpenVLA vision = 2 * ViT-L (600M)**
2. **Vision ~ 35%, LM ~ 58%** latency 비중
3. **단순 concat fusion** (OpenVLA)
4. **병렬화 효과 미미** (RTX 4070)
5. **ViT 지식 = OpenVLA vision 의 직접 이해 토대**

[O] 이전: [Week 2](../week2/README.md) | 다음: [Week 4 - CLIP 정독](../week4/README.md)
