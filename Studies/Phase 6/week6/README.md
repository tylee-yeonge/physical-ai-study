# Week 6: 성능 분석 및 개선 - 3D Detection 오류 진단과 개선 전략

> **이번 주 목표**: FCOS3D의 학습 결과를 분석하여 오류 유형을 파악하고, 성능 개선 전략을 적용하여 Car Moderate AP3D > 15%를 달성한다.
> **예상 시간**: 12-15시간
> **핵심 질문**: "3D Detection 모델이 왜 틀리는지 분석하고, 어떻게 개선할 수 있는가?"

---

## 시작하기 전에

### Week 5에서 배운 것

**MMDetection3D 실습 결과:**
```
Week 5 결과 (FCOS3D, KITTI):
  Car AP3D (Easy):     ~18%
  Car AP3D (Moderate): ~13%
  Car AP3D (Hard):     ~11%

질문:
  - 왜 AP3D가 이렇게 낮을까?
  - 어떤 케이스에서 실패하는 걸까?
  - 어떻게 하면 성능을 올릴 수 있을까?
```

**이번 주에 답합니다!**

> **포트폴리오 관점**: "모델 성능이 낮습니다"에서 끝나면 안 됩니다. "왜 낮은지 분석하고, 어떻게 개선했는지" 보여주는 것이 핵심입니다. 면접에서 "성능이 안 나올 때 어떻게 했나요?" 라는 질문에 체계적으로 답할 수 있어야 합니다.

---

## 핵심 개념 자세히 알아보기

### 1. 오류 분석 프레임워크

3D Detection의 오류는 크게 4가지 카테고리로 분류할 수 있습니다.

```
3D Detection 오류 분류:
┌────────────────────────────────────────────────────┐
│                 오류 유형                            │
├──────────────┬──────────────┬──────────────────────┤
│  Depth 오류   │ Orientation │ Occlusion / Truncation│
│              │   오류       │       오류            │
├──────────────┼──────────────┼──────────────────────┤
│ 깊이 추정 부정 │ 회전각도 추정│ 가려짐/잘림으로       │
│ 확 (z 오차)   │ 부정확 (ry)  │ 인한 검출 실패        │
├──────────────┼──────────────┼──────────────────────┤
│ 영향: 3D IoU │ 영향: 3D IoU│ 영향: Recall         │
│ 급격 감소     │ 감소         │ 감소                 │
└──────────────┴──────────────┴──────────────────────┘
```

#### 1.1 Depth 오류 분석

```
Depth 오류가 3D IoU에 미치는 영향:

  Depth 오차    예상 3D IoU (Car, IoU threshold=0.7)
  ────────     ──────────────────────────────────
  0.5m          ~0.75  (TP)
  1.0m          ~0.55  (FP!)
  2.0m          ~0.25  (FP!)
  5.0m          ~0.05  (FP!)

핵심 관찰:
  - Depth 1m 오차만으로 IoU 0.7 기준을 넘지 못함
  - Monocular에서 Depth 오차는 보통 2~5m
  - 이것이 AP3D가 낮은 가장 큰 원인!

거리별 Depth 오차 경향:
  근거리 (0-20m):  평균 오차 ~1.5m  → 상대적으로 정확
  중거리 (20-40m): 평균 오차 ~3.0m  → 성능 급감
  원거리 (40m+):   평균 오차 ~5.0m+ → 거의 실패
```

#### 1.2 Orientation 오류 분석

```
Orientation(ry) 오류의 영향:

  회전 오차      예상 3D IoU 감소
  ────────      ──────────────
  5도            ~2% 감소
  15도           ~8% 감소
  30도           ~20% 감소
  90도           ~50% 감소

흔한 Orientation 오류 패턴:
  1. 전후 혼동 (0도 vs 180도)
    → 차량의 앞뒤를 구분하지 못함
  2. 대칭 모호성
    → 좌우 대칭인 차량에서 ry가 180도 반전
  3. 원거리 각도 추정 어려움
    → 작은 객체에서는 시각적 단서 부족
```

#### 1.3 Occlusion / Truncation 오류

```
Occlusion 수준에 따른 성능 변화:

  Occlusion 수준     검출 성공률
  ──────────────     ──────────
  가려짐 없음 (0%)    ~85%
  부분 가려짐 (25%)   ~60%
  절반 가려짐 (50%)   ~35%
  대부분 가려짐 (75%) ~10%

Truncation (이미지 경계 잘림):
  잘리지 않음 (0%)    ~80%
  부분 잘림 (25%)     ~55%
  절반 잘림 (50%)     ~25%
```

---

### 2. 체계적 오류 분석 방법

#### 2.1 예측 결과 분류

```python
# 오류 분석 코드 개요
def analyze_errors(predictions, ground_truths):
    """
    예측 결과를 카테고리별로 분류하는 분석 프레임워크

    분류 기준:
    ┌─────────────────────────────────────────────────┐
    │ True Positive:  3D IoU >= 0.7                   │
    │ Depth Error:    2D IoU >= 0.7 but 3D IoU < 0.7  │
    │ Orientation Error: Depth OK but angle 오류       │
    │ False Positive: 대응하는 GT 없음                 │
    │ False Negative: 검출하지 못한 GT                 │
    └─────────────────────────────────────────────────┘
    """
    results = {
        'TP': [],           # 정확한 검출
        'depth_error': [],  # 깊이 오류
        'orient_error': [], # 방향 오류
        'FP': [],           # 오검출
        'FN': [],           # 미검출
    }
    return results
```

#### 2.2 거리별 성능 분석

```
거리별 AP3D 분석 (Car Moderate):

  거리 구간     AP3D    주요 오류 원인
  ────────     ─────   ──────────────
  0-10m        ~35%    Truncation (가까워서 잘림)
  10-20m       ~25%    가장 성능 좋은 구간
  20-30m       ~12%    Depth 오차 증가
  30-40m       ~5%     Depth + 작은 크기
  40m+         ~1%     거의 검출 불가

핵심 인사이트:
  - 최적 거리 구간: 10-20m
  - 성능 향상 여지가 가장 큰 구간: 20-30m
```

---

### 3. 성능 개선 전략

#### 3.1 Multi-scale Feature 활용

```
문제: 원거리 객체는 이미지에서 매우 작음
  → 하나의 Feature Map으로는 부족

해결: FPN (Feature Pyramid Network) 활용

┌────────────────────────────────────────┐
│  입력 이미지 (1242 x 375)              │
│       ↓                                │
│  ResNet Backbone                       │
│       ↓                                │
│  FPN (Feature Pyramid Network)         │
│  ├── P3 (1/8 scale)  → 근거리 검출     │
│  ├── P4 (1/16 scale) → 중거리 검출     │
│  ├── P5 (1/32 scale) → 원거리 검출     │
│  └── P6 (1/64 scale) → 매우 원거리     │
│                                        │
│  각 스케일에서 독립적으로 3D bbox 예측   │
└────────────────────────────────────────┘

개선 효과:
  - 원거리 객체 검출률 향상
  - 다양한 크기의 객체에 대한 성능 균형
```

#### 3.2 Data Augmentation

```
3D Detection에 효과적인 Augmentation:

1. 기본 Augmentation:
   - Random Flip (좌우 반전)
     → rotation_y도 함께 반전!
     → ry_new = -ry, x_new = -x
   - Random Crop / Resize
   - Color Jittering (밝기, 대비)

2. 3D 특화 Augmentation:
   - Copy-Paste (다른 이미지에서 객체 복사)
   - Random Depth Perturbation
   - Camera Intrinsic Augmentation

3. 주의사항:
   ┌──────────────────────────────────────┐
   │ 2D Augmentation과 달리:              │
   │  - Flip 시 ry(회전각)도 변환 필요!    │
   │  - Crop 시 카메라 파라미터 재계산!     │
   │  - 캘리브레이션 정합성 유지 필수!      │
   └──────────────────────────────────────┘
```

#### 3.3 3D NMS (Non-Maximum Suppression)

```
2D NMS vs 3D NMS:

2D NMS:
  - 2D IoU 기준으로 중복 제거
  - 문제: 다른 깊이의 겹치는 bbox를 잘못 제거

3D NMS:
  - BEV(상공뷰) 또는 3D IoU 기준 중복 제거
  - 같은 2D 영역이라도 깊이가 다르면 유지

예시:
  ┌──────────────────────────┐
  │  이미지에서:              │
  │  ┌───────┐               │
  │  │ Car A │ (z=10m)       │
  │  │ Car B │ (z=25m)       │
  │  └───────┘               │
  │                          │
  │  2D NMS: B를 제거할 수 있음 (2D 겹침)│
  │  3D NMS: 깊이가 다르므로 둘 다 유지  │
  └──────────────────────────┘

MMDetection3D에서 3D NMS 설정:
  test_cfg = dict(
      use_rotate_nms=True,     # 회전 고려 NMS
      nms_thr=0.25,            # BEV IoU threshold
      score_thr=0.05,          # 최소 신뢰도
      max_per_img=200,         # 최대 검출 수
  )
```

#### 3.4 Depth 추정 개선 전략

```
전략 1: Depth 보조 Loss
  - Depth 예측에 추가적인 Loss 부여
  - Log-space Depth Loss: L = |log(d_pred) - log(d_gt)|
    → 원거리 오차에 덜 민감하게 학습

전략 2: Depth Bin Classification
  - 연속 값 회귀 대신, 구간별 분류
  - 예: [0-5m], [5-10m], ..., [45-50m] → 10개 bin
  - 분류 + 오프셋 회귀 → 더 안정적

전략 3: 기하학적 제약 활용
  - 차량 크기 prior: 승용차 ~4.5m, 트럭 ~12m
  - 2D bbox 크기와 Depth의 관계:
    z = f * H_real / h_pixel
    (f: 초점거리, H_real: 실제 높이, h_pixel: 픽셀 높이)
```

---

### 4. Ablation Study 수행 방법

```
성능 개선의 과학적 접근:

1. Baseline 확립
   → 변경 없이 기본 설정으로 학습
   → Car Moderate AP3D: ~13%

2. 한 번에 하나씩 변경 (Ablation)
   ┌──────────────────────────────────────┐
   │ 실험      │ 변경 사항    │ AP3D 변화  │
   ├──────────────────────────────────────┤
   │ Exp1      │ Multi-scale │ +1.5%      │
   │ Exp2      │ Augmentation│ +2.0%      │
   │ Exp3      │ 3D NMS      │ +0.8%      │
   │ Exp4      │ Depth Loss  │ +1.2%      │
   │ Exp1+2+3+4│ All         │ +4.5%      │
   └──────────────────────────────────────┘

3. 결과 분석 및 보고
   → "어떤 변경이 얼마나 기여했는지" 정량적으로 증명
   → 이것이 포트폴리오의 핵심!
```

---

### 5. 목표: Car Moderate AP3D > 15%

```
달성 전략 로드맵:

Baseline:          ~13% AP3D
  ↓
+ Multi-scale FPN: ~14.5% (+1.5%)
  ↓
+ Data Augment:    ~15.5% (+1.0%)
  ↓
+ 3D NMS 튜닝:     ~16.0% (+0.5%)
  ↓
+ Depth Loss:      ~17.0% (+1.0%)

최종 목표: 15% 이상 달성!

실제 FCOS3D 논문 성능:
  - nuScenes: NDS 0.372, mAP 0.295
  - KITTI Car Moderate: ~12-15% AP3D
  → 15% 이상이면 논문 수준을 달성한 것!
```

---

## 꼭 이해해야 할 핵심 개념

### 1. 오류 분석의 체계적 접근

```
단순히 "AP3D가 낮다"가 아니라:
  1. 어떤 유형의 오류가 가장 많은지 분류
  2. 어떤 거리에서 성능이 떨어지는지 분석
  3. 어떤 객체 특성(크기, 가려짐)에서 실패하는지 파악
  4. 각 오류에 대한 구체적 개선 전략 수립

이것이 "연구자/엔지니어"와 "사용자"의 차이!
```

### 2. Augmentation에서의 3D 일관성

```
핵심 원칙:
  이미지 변환 시 3D 레이블도 함께 변환해야 함!

Random Flip 예시:
  이미지: 좌우 반전
  2D bbox: x_new = img_width - x
  3D location: x_new = -x
  rotation_y: ry_new = -ry (반전!)

이것을 빠뜨리면 학습이 완전히 잘못됨!
```

### 3. NMS의 3D 확장

```
2D NMS:
  같은 객체에 대한 중복 검출 제거
  기준: 2D bbox IoU

3D NMS:
  BEV 평면에서의 IoU로 중복 제거
  장점: 서로 다른 깊이의 객체를 보존

BEV NMS가 3D Detection에 더 적합한 이유:
  - 다른 깊이에 있는 객체들이 2D에서는 겹칠 수 있음
  - 2D NMS는 이런 경우 올바른 검출을 제거할 수 있음
```

---

## 자체 점검 - 이해했는지 확인!

**Q1. Monocular 3D Detection에서 AP3D가 낮은 가장 큰 원인은 무엇인가?**
> Depth(깊이) 추정의 부정확함이 가장 큰 원인이다. 단안 카메라에서는 깊이 정보가 근본적으로 모호(Scale Ambiguity)하며, 1m 이상의 Depth 오차가 발생하면 3D IoU가 0.7 기준을 넘지 못해 False Positive로 처리된다. 2D에서는 잘 맞지만(AP2D ~85%) 3D에서 실패하는(AP3D ~13%) 주요 원인이다.

**Q2. Data Augmentation에서 Random Flip 적용 시 주의할 점은?**
> 이미지를 좌우 반전할 때 3D 레이블도 함께 변환해야 한다. 구체적으로 (1) 3D 중심 좌표의 x를 반전 (x_new = -x), (2) rotation_y를 반전 (ry_new = -ry), (3) 2D bbox 좌표도 반전해야 한다. 이를 빠뜨리면 학습 데이터가 잘못되어 오히려 성능이 하락한다.

**Q3. 3D NMS가 2D NMS보다 3D Detection에 적합한 이유는?**
> 2D NMS는 이미지 평면에서의 겹침만 고려하므로, 서로 다른 깊이에 있는 객체들이 이미지에서 겹칠 때 올바른 검출을 제거할 수 있다. 3D NMS(또는 BEV NMS)는 BEV 공간에서의 IoU를 기준으로 판단하므로, 깊이가 다른 객체들을 올바르게 보존한다.

**Q4. Ablation Study가 중요한 이유는?**
> 한 번에 여러 변경을 적용하면 어떤 변경이 성능 향상에 기여했는지 알 수 없다. Ablation Study는 한 번에 하나의 요소만 변경하여 각 요소의 기여도를 정량적으로 측정한다. 이는 연구의 재현 가능성을 보장하고, 포트폴리오에서 "어떤 기법이 왜 효과적인지"를 설명할 수 있는 근거가 된다.

---

## 이번 주 실습 & 다음 주 준비

### 이번 주 실습 과제

1. **오류 분석**: Week 5 학습 결과에서 TP/FP/FN 분류 및 오류 유형 분석
2. **거리별 성능 분석**: 거리 구간별 AP3D 계산
3. **Augmentation 적용**: Random Flip, Color Jitter 적용 후 재학습
4. **3D NMS 튜닝**: NMS threshold 변경에 따른 성능 변화 실험
5. **Depth Loss 실험**: Depth 보조 Loss 추가 후 성능 비교
6. **Ablation Study 정리**: 각 개선 기법의 성능 기여도 표로 정리

자세한 내용은 [PRACTICE.md](./PRACTICE.md) 참고

### 다음 주 준비

- nuScenes 데이터셋 사이트 회원가입: https://www.nuscenes.org/
- nuScenes Mini (10GB) 다운로드 시작
- nuScenes-devkit 설치: `pip install nuscenes-devkit`

---

## 이번 주 핵심 요약

1. **오류 분석**은 성능 개선의 첫 걸음이며, Depth/Orientation/Occlusion/Truncation으로 분류하여 체계적으로 접근해야 한다.
2. **Depth 오류**가 가장 큰 성능 저하 원인이다. 1m 이상의 오차로 3D IoU가 급감하며, 특히 원거리에서 심각하다.
3. **Multi-scale FPN**, **Data Augmentation**, **3D NMS 튜닝**, **Depth Loss 개선** 등을 통해 AP3D를 체계적으로 향상시킬 수 있다.
4. **Ablation Study**로 각 개선 기법의 기여도를 정량적으로 측정하는 것이 과학적 접근이며 포트폴리오의 핵심이다.
5. **목표 Car Moderate AP3D > 15%** 달성은 논문 수준의 성능이며, 이 과정 자체가 실무 역량의 증거가 된다.

---

이전: [Week 5 - MMDetection3D 실습](../week5/README.md)

다음: [Week 7 - nuScenes 데이터셋](../week7/README.md)
