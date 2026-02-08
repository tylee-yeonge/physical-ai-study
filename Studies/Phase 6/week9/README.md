# Week 9: BEVFormer 이해 - 카메라로 Bird's Eye View 만들기

> 🎯 **이번 주 목표**: BEVFormer의 전체 구조를 이해하고, Spatial Cross-Attention과 Temporal Self-Attention의 원리를 파악한다.
> ⏰ **예상 시간**: 12-15시간
> 💡 **핵심 질문**: "Multi-view 카메라 이미지로부터 어떻게 Bird's Eye View 표현을 생성하고, 시간 정보를 활용하는가?"

---

## 🌟 시작하기 전에

### Week 8에서 배운 것

**BEV 개념 복습:**
```
Side View (Camera):       BEV (Top-down):
    │  🚗 │                  ┌─────────┐
    │     │                  │    ▲    │
    ├─────┤   ⇒              │    │    │
  Road                       │  🚗    │
                             └─────────┘
```

**BEV 생성 방법 분류:**
```
1. IPM (Inverse Perspective Mapping)
   → 단순 기하 변환, 평면 가정 필요

2. MLP 기반 (Lift-Splat-Shoot)
   → 학습으로 Camera → BEV, Depth 예측 필요

3. Transformer 기반 (BEVFormer) ← 이번 주!
   → Query 기반, Attention으로 BEV 생성
   → 현재 SOTA 방법론
```

**이번 주 핵심:**
```
BEVFormer는 "Transformer의 Attention 메커니즘"을 활용하여
Multi-view 카메라 이미지에서 BEV 표현을 생성하는 모델이다.

기존 방법 대비 장점:
  - Depth 명시적 예측 불필요
  - Temporal 정보 자연스럽게 활용
  - End-to-end 학습 가능
```

---

## 📚 핵심 개념 자세히 알아보기

### 1. BEVFormer 전체 구조

#### 파이프라인 개요

```
┌───────────────────────────────────────────────────────────┐
│                    BEVFormer Pipeline                      │
│                                                           │
│  Multi-view Images (6대 카메라)                            │
│       ↓                                                   │
│  Backbone (ResNet-101 / VoVNet)                           │
│       ↓                                                   │
│  FPN (Feature Pyramid Network)                            │
│       ↓                                                   │
│  Multi-scale Image Features                               │
│       ↓                                                   │
│  ┌─────────────────────────────────────────────────┐      │
│  │          BEV Encoder (6 layers)                 │      │
│  │                                                 │      │
│  │  BEV Queries (200×200)                         │      │
│  │       ↓                                         │      │
│  │  Temporal Self-Attention ← 이전 프레임 BEV      │      │
│  │       ↓                                         │      │
│  │  Spatial Cross-Attention ← Image Features       │      │
│  │       ↓                                         │      │
│  │  Feed Forward Network                           │      │
│  │                                                 │      │
│  └─────────────────────────────────────────────────┘      │
│       ↓                                                   │
│  BEV Feature Map (200×200×256)                            │
│       ↓                                                   │
│  Detection Head (3D bbox + Velocity)                      │
│       ↓                                                   │
│  Output: [x, y, z, l, w, h, θ, vx, vy]                  │
└───────────────────────────────────────────────────────────┘
```

#### 논문 정보

```
"BEVFormer: Learning Bird's-Eye-View Representation
 from Multi-Camera Images via Spatiotemporal Transformers"

저자: Zhiqi Li et al.
학회: ECCV 2022
핵심 기여:
  1. Spatiotemporal Transformer로 BEV 생성
  2. 시간 정보를 활용한 3D Detection
  3. nuScenes SOTA 달성 (발표 시점)
```

---

### 2. BEV Queries - 학습 가능한 BEV 그리드

#### BEV Query란?

```
BEV Queries:
  - 형태: H_bev × W_bev × C = 200 × 200 × 256
  - 물리적 의미: BEV 공간의 각 셀을 대표하는 벡터
  - 셀 크기: 0.5m × 0.5m
  - 커버 범위: 100m × 100m (200 × 0.5m)
  - 종류: Learnable Embedding (학습으로 최적화)

┌─────────────────────────────────────────┐
│  BEV Query Grid (200 × 200)             │
│                                         │
│  ┌───┬───┬───┬───┬─ ─ ─┬───┐           │
│  │q₀₀│q₀₁│q₀₂│q₀₃│     │q₀,₁₉₉│      │
│  ├───┼───┼───┼───┤     ├───┤           │
│  │q₁₀│q₁₁│   │   │     │   │           │
│  ├───┼───┤   │   │     │   │           │
│  │   │   │   │   │     │   │           │
│  │   │   │   │   │     │   │           │
│  ├───┼───┼───┼───┤     ├───┤           │
│  │q₁₉₉,₀│  │   │     │q₁₉₉,₁₉₉│     │
│  └───┴───┴───┴───┴─ ─ ─┴───┘           │
│                                         │
│  각 qᵢⱼ ∈ ℝ²⁵⁶ (256차원 벡터)          │
│  각 셀 = 0.5m × 0.5m 물리 공간          │
└─────────────────────────────────────────┘
```

#### BEV Query의 직관적 이해

```
비유: "질문하는 그리드"

각 BEV Query는 다음과 같은 질문을 한다:
  "나는 BEV 공간의 (i, j) 위치를 담당하는데,
   6대 카메라 이미지 중 어디를 봐야
   나의 위치에 무엇이 있는지 알 수 있을까?"

→ 이 질문에 대한 답을 Spatial Cross-Attention이 해준다!
```

#### 구현 관점

```python
# BEV Queries 초기화 (학습 파라미터)
import torch
import torch.nn as nn

class BEVFormerEncoder(nn.Module):
    def __init__(self, bev_h=200, bev_w=200, embed_dim=256):
        super().__init__()
        # Learnable BEV Queries
        self.bev_queries = nn.Embedding(bev_h * bev_w, embed_dim)
        # 각 query의 BEV 공간 위치 인코딩
        self.bev_pos = nn.Embedding(bev_h * bev_w, embed_dim)

    def forward(self):
        bev_q = self.bev_queries.weight  # [40000, 256]
        bev_p = self.bev_pos.weight      # [40000, 256]
        return bev_q + bev_p
```

---

### 3. Spatial Cross-Attention - 이미지를 BEV로 변환

#### 핵심 아이디어

```
문제: BEV Query가 이미지의 "어디"를 봐야 하는가?

해결: 3D Reference Points → 2D 투영

Step 1: BEV Query (i, j) → 3D 공간 좌표 (x, y, z_ref) 매핑
        z_ref = 미리 정한 높이 (예: [-5m, -3m, -1m, 1m])

Step 2: 3D 좌표를 각 카메라에 투영 → 2D 점 (u, v) 획득
        [u, v, 1]^T = K × [R|t] × [x, y, z_ref, 1]^T

Step 3: 투영된 (u, v) 위치의 Image Feature를 가져옴

Step 4: Deformable Attention으로 주변 정보까지 수집
```

#### Reference Points 생성

```
BEV Query (i, j) 하나에 대해:

              카메라 1    카메라 2    카메라 3
z = 1m:    (u₁¹, v₁¹)  (u₁², v₁²)  (u₁³, v₁³)
z = -1m:   (u₂¹, v₂¹)  (u₂², v₂²)  (u₂³, v₂³)
z = -3m:   (u₃¹, v₃¹)  (u₃², v₃²)  (u₃³, v₃³)
z = -5m:   (u₄¹, v₄¹)  (u₄², v₄²)  (u₄³, v₄³)

→ 4개 높이 × 6대 카메라 = 24개 reference point
→ 이 중 이미지 범위 내에 있는 점만 사용
```

#### Deformable Attention

```
기존 Attention:
  모든 이미지 pixel에 대해 attention 계산
  → 너무 비용이 큼! (O(N²))

Deformable Attention:
  Reference Point 주변 K개 점에 대해서만 attention
  → 효율적! (O(K), K ≈ 4~8)

┌────────────────────────────────┐
│  Image Feature Map             │
│                                │
│           ○  ○                 │
│         ○  ●  ○               │
│           ○  ○                 │
│                                │
│  ● = Reference Point           │
│  ○ = Learnable Offset Points  │
│  (학습으로 최적 위치 결정)      │
└────────────────────────────────┘
```

#### 수식

```
SpatialCrossAttention(Q_p, F_cam):

  1. reference_points = project(BEV_to_3D(p), cam_params)

  2. for each camera c:
       for each height z_ref:
         ref_2d = project_3D_to_2D(x_p, y_p, z_ref, K_c, [R|t]_c)
         if ref_2d in image_bounds:
           feature = DeformAttn(Q_p, ref_2d, F_c)

  3. output = weighted_sum(features)

여기서:
  Q_p: BEV Query at position p
  F_cam: 각 카메라의 Feature Map
  K_c: 카메라 c의 내부 파라미터
  [R|t]_c: 카메라 c의 외부 파라미터
```

---

### 4. Temporal Self-Attention - 시간 정보 활용

#### 왜 Temporal 정보가 중요한가?

```
문제 상황:

Frame t:                      Frame t-1:
┌──────────────┐              ┌──────────────┐
│    🚗        │              │  🚗          │
│  (가려짐!)   │              │ (잘 보임!)   │
│    🚛       │              │   🚛         │
└──────────────┘              └──────────────┘

→ 현재 프레임에서 가려진 차량도
  이전 프레임 정보로 검출 가능!

추가 장점:
  - 속도(Velocity) 추정 가능
  - 움직이는 물체 추적 용이
  - 검출 안정성 향상
```

#### Temporal Self-Attention 동작 방식

```
Step 1: 이전 프레임의 BEV Feature를 가져옴
        B_{t-1} (200 × 200 × 256)

Step 2: Ego-motion으로 좌표 정렬
        B_{t-1}' = warp(B_{t-1}, ego_motion_{t-1→t})

Step 3: 현재 BEV Query와 정렬된 이전 BEV를 합침
        output = SelfAttention(Q_t, concat(Q_t, B_{t-1}'))

┌─────────────────────────────────────────────┐
│                                             │
│  Q_t (현재 BEV Query)                       │
│       ↓                                     │
│  Self-Attention                             │
│       ↑                                     │
│  [Q_t, warp(B_{t-1})] (concat)             │
│                                             │
│  warp: ego-motion으로 좌표 정렬              │
│    - 차량이 앞으로 1m 이동했으면              │
│    - 이전 BEV를 1m 만큼 shift               │
│                                             │
└─────────────────────────────────────────────┘
```

#### Ego-motion 보상

```
왜 필요한가?

t-1 시점:                 t 시점:
  ┌─────────┐              ┌─────────┐
  │  A       │              │         │
  │    ego→  │              │    ego  │
  │  B       │     →        │  A      │
  └─────────┘              │  B      │
                           └─────────┘

ego-motion 없이 합치면 → A, B 위치가 어긋남!
ego-motion으로 보정 후 합치면 → 정확한 정렬!

보정 수식:
  p_{t-1→t} = R_ego × p_{t-1} + t_ego
  여기서 R_ego, t_ego = t-1에서 t로의 차량 이동
```

---

### 5. Detection Head - 최종 검출

#### 출력 형식

```
BEV Feature (200 × 200 × 256)
       ↓
Detection Head (Deformable DETR 기반)
       ↓
예측 결과:
  - 3D Bounding Box: [x, y, z, l, w, h, θ]
  - 속도 (Velocity): [vx, vy]
  - 클래스: Car, Truck, Pedestrian, ... (10 classes)
  - 신뢰도: confidence score

nuScenes 기준 출력:
  [cx, cy, cz, w, l, h, rot, vx, vy, class, score]
```

#### Detection Head 구조

```
┌───────────────────────────────────────┐
│  Detection Head (DETR style)          │
│                                       │
│  Object Queries (900개)               │
│       ↓                               │
│  Decoder (6 layers)                   │
│       ↓                               │
│  ├── Classification Head → 클래스     │
│  ├── Regression Head → 3D bbox       │
│  └── Velocity Head → vx, vy         │
│                                       │
│  Loss:                                │
│  - Classification: Focal Loss        │
│  - Regression: L1 Loss               │
│  - Hungarian Matching (GT 매칭)      │
└───────────────────────────────────────┘
```

---

### 6. 논문 핵심 Figure 분석

#### Figure 2: 전체 아키텍처

```
논문 Figure 2에서 확인할 것:

1. 입력: 6대 카메라 이미지 (FRONT, FRONT_LEFT, FRONT_RIGHT,
         BACK, BACK_LEFT, BACK_RIGHT)

2. Backbone → FPN → Multi-scale Features
   - 보통 4개 스케일 (1/8, 1/16, 1/32, 1/64)

3. BEV Encoder:
   - BEV Queries가 핵심
   - Temporal + Spatial Attention 순서 확인
   - 6개 Layer 반복

4. Detection Head:
   - DETR 스타일의 Object Query + Decoder
```

#### Figure 3: Spatial Cross-Attention 상세

```
논문 Figure 3에서 확인할 것:

1. BEV Query 하나가 여러 높이의 Reference Point 생성
2. 각 Reference Point를 6대 카메라에 투영
3. 투영된 위치에서 Deformable Attention 수행
4. 결과를 가중합하여 BEV Feature 업데이트

핵심 수식:
  SCA(z_p, {F_t^i}_{i=1}^{N_cam}) =
    1/|V_hit| Σ_{i∈V_hit} DeformAttn(z_p, P(p, i, j), F_t^i)

  V_hit: reference point가 이미지 범위 안에 있는 카메라 집합
```

---

### 7. Ablation Study 분석

#### 주요 Ablation 결과

```
표: BEVFormer Ablation Study (nuScenes val set)

| 설정                          | NDS   | mAP   |
|-------------------------------|-------|-------|
| Baseline (Spatial만)          | 0.478 | 0.370 |
| + Temporal (1 frame)          | 0.502 | 0.396 |
| + Temporal (4 frames)         | 0.517 | 0.416 |
| + Multi-scale Features        | 0.525 | 0.423 |
| BEVFormer-Base (최종)          | 0.517 | 0.416 |

핵심 관찰:
  1. Temporal Attention → NDS +2.4% 향상 (가장 큰 기여)
  2. Multi-frame 사용 시 추가 향상
  3. Spatial Cross-Attention 자체가 이미 강력

왜 Temporal이 효과적인가?
  - 가려진 객체 복구
  - 속도 추정 정확도 향상
  - 검출 일관성 증가
```

#### BEV Resolution의 영향

```
| BEV 해상도    | 셀 크기  | NDS   | FPS  |
|--------------|---------|-------|------|
| 50 × 50      | 2.0m    | 0.451 | 5.2  |
| 100 × 100    | 1.0m    | 0.489 | 3.8  |
| 200 × 200    | 0.5m    | 0.517 | 1.7  |

→ 해상도 높을수록 성능 UP, 속도 DOWN
→ 200×200이 성능/속도 균형점
```

---

### 8. BEVFormer vs 다른 BEV 방법론 비교

```
┌──────────────┬────────────┬──────────────┬────────────┐
│ 방법          │ BEV 생성    │ Temporal     │ NDS        │
├──────────────┼────────────┼──────────────┼────────────┤
│ DETR3D       │ Query→3D   │ ✗           │ 0.412      │
│ PETR         │ 3D PE      │ ✗           │ 0.455      │
│ BEVDet       │ Lift-Splat │ ✗           │ 0.392      │
│ BEVFormer    │ Query+Attn │ ✓           │ 0.517      │
│ BEVFormer v2 │ 개선 버전   │ ✓           │ 0.556      │
└──────────────┴────────────┴──────────────┴────────────┘

BEVFormer의 핵심 차별점:
  1. Explicit Depth 예측 불필요 (vs BEVDet)
  2. Temporal 정보 활용 (vs DETR3D, PETR)
  3. Deformable Attention으로 효율적 (vs naive Attention)
```

---

## 💡 꼭 이해해야 할 핵심 개념

### 1. BEV Query의 역할

```
BEV Query = "BEV 공간을 대표하는 학습 가능한 벡터"

일반 Transformer:
  - NLP: 단어 → Query
  - ViT: 이미지 패치 → Query

BEVFormer:
  - BEV 그리드 셀 → Query
  - 각 Query가 "내 위치에 뭐가 있어?" 라고 질문
  - Image Feature에서 답을 가져옴 (Cross-Attention)
```

### 2. Spatial Cross-Attention의 핵심

```
핵심: "어디를 봐야 하는지" 를 기하학으로 결정

3D Reference Points → Camera Projection → 2D Points
→ 해당 위치의 Feature를 가져옴

이것이 가능한 이유:
  - 카메라 내부/외부 파라미터를 알고 있음
  - BEV Query의 3D 위치를 알고 있음
  → 정확한 투영이 가능!
```

### 3. Temporal Self-Attention의 원리

```
핵심: "이전 프레임의 BEV를 현재에 활용"

1. 이전 BEV Feature를 ego-motion으로 정렬
2. 현재 BEV Query와 concat
3. Self-Attention으로 필요한 정보 선택

→ 가려진 객체, 속도 추정에 큰 도움
```

### 4. Deformable Attention이 필요한 이유

```
일반 Cross-Attention:
  Query × 모든 Image pixel = O(H×W×N_cam) → 너무 느림!

Deformable Cross-Attention:
  Query × K개 학습된 offset 점 = O(K) → 빠름!

K = 4~8: Reference Point 주변에서 가장 유용한 점을 학습
→ 정확도 유지 + 속도 대폭 향상
```

---

## 🔍 자체 점검 - 이해했는지 확인!

**Q1. BEV Queries의 크기가 200x200이고 셀 크기가 0.5m일 때, 커버하는 실제 범위는?**

> 200 x 0.5m = 100m, 즉 차량 중심으로 100m x 100m 영역을 커버한다. 이는 nuScenes 데이터셋의 검출 범위(50m 반경)와 일치하며, 도로 위 대부분의 객체를 포함하기에 충분한 범위이다.

**Q2. Spatial Cross-Attention에서 Reference Point를 여러 높이(z)로 만드는 이유는?**

> BEV 공간은 X-Y 평면이므로 높이 정보가 없다. 하지만 실제 3D 공간에서 객체는 다양한 높이에 있을 수 있다 (도로면의 차량 vs 교통 표지판). 여러 높이에 Reference Point를 만들어 각각을 카메라에 투영하면, 다양한 높이의 정보를 모두 수집할 수 있다. 네트워크가 학습을 통해 각 높이의 중요도를 결정한다.

**Q3. Temporal Self-Attention에서 ego-motion 보상이 없으면 어떤 문제가 발생하는가?**

> 차량이 이동하면 동일한 BEV 위치가 다른 실세계 좌표를 가리키게 된다. ego-motion 보상 없이 이전 프레임의 BEV Feature를 합치면, 정적 객체의 위치가 어긋나고 (이중으로 보임), 속도 추정이 부정확해진다. ego-motion으로 이전 BEV를 현재 좌표계에 맞춰 정렬해야 올바른 시간 정보 융합이 가능하다.

**Q4. BEVFormer가 BEVDet(Lift-Splat-Shoot 기반)보다 Depth 추정에 유리한 이유는?**

> BEVDet는 각 픽셀에 대해 명시적으로 Depth를 예측해야 하며, 이 Depth 예측의 정확도가 전체 성능에 직접적인 영향을 미친다. 반면 BEVFormer는 Attention 메커니즘으로 암묵적으로(implicitly) 3D 정보를 학습하므로, 명시적 Depth 예측의 오류에 덜 민감하다. 카메라 파라미터를 통한 기하학적 투영으로 Reference Point를 생성하고, Deformable Attention이 적절한 Feature를 선택한다.

---

## 📝 이번 주 실습 & 다음 주 준비

### 이번 주 실습 과제

1. **논문 정독**: BEVFormer 논문 (ECCV 2022) 읽기, Figure 2와 3에 집중
2. **BEV Query 구현 실습**: PRACTICE.md의 코드를 따라 BEV Query와 Reference Point 생성 실습
3. **Spatial Cross-Attention 이해**: 3D → 2D 투영 과정을 코드로 구현
4. **Temporal Self-Attention 이해**: ego-motion 보상 과정 시각화
5. **Ablation Study 분석**: 논문의 Table 결과를 자신의 말로 정리
6. **비교 분석**: BEVFormer vs BEVDet vs DETR3D 장단점 정리

### 다음 주 준비

- BEVFormer GitHub 저장소 클론: `git clone https://github.com/fundamentalvision/BEVFormer.git`
- MMDetection3D 환경 설정 확인
- nuScenes Mini 데이터셋 다운로드 (아직 안 했다면)
- Pretrained weight 다운로드 (용량이 크므로 미리)

---

## 🎯 이번 주 핵심 요약

1. **BEVFormer**는 Transformer 기반으로 Multi-view 이미지에서 BEV 표현을 생성하는 모델이며, ECCV 2022에서 발표되었다.
2. **BEV Queries**는 200x200 그리드의 learnable embedding으로, BEV 공간의 각 셀을 대표하며 0.5m 해상도로 100m x 100m 영역을 커버한다.
3. **Spatial Cross-Attention**은 BEV Query에서 3D Reference Point를 생성하고, 카메라에 투영하여 Image Feature를 가져오는 메커니즘이다.
4. **Temporal Self-Attention**은 이전 프레임의 BEV Feature를 ego-motion으로 정렬한 후 현재 정보와 합쳐, 가려진 객체 검출과 속도 추정을 돕는다.
5. **Deformable Attention**을 사용하여 계산 효율을 높이면서도 정확한 Feature 추출이 가능하다.

---

✅ 이전: [Week 8 - BEV 개념 이해](../week8/README.md)

다음: [Week 10 - BEVFormer 실습](../week10/README.md)
