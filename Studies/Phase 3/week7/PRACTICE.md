# Week 7 실습: Monocular Depth Estimation 이론 및 모델 탐구

> [goal] **목표**: 깊이 추정의 원리를 이해하고, 주요 모델의 구조와 성능을 비교 분석하기
> [code] **언어**: Python (PyTorch, transformers)
> [time] **예상 시간**: 12시간

---

## [list] 실습 개요

Week 7은 이론 중심의 주차입니다. Monocular Depth Estimation의 핵심 개념을 학습하고, MiDaS/DPT/Depth Anything 모델을 Python으로 간단히 불러보며 구조를 파악합니다.

---

## [tool] 환경 설정

```bash
# 가상환경 활성화
conda activate phase5

# 패키지 설치
pip install torch torchvision
pip install transformers
pip install timm
pip install numpy opencv-python matplotlib
pip install Pillow

# 설치 확인
python -c "from transformers import pipeline; print('transformers OK')"
python -c "import timm; print(f'timm models: {len(timm.list_models())}')"
python -c "import torch; print(f'PyTorch: {torch.__version__}')"
```

---

## 프로젝트 구조

```
week7_depth_theory/
+-- explore_midas.py          # MiDaS 모델 탐구
+-- explore_dpt.py            # DPT 모델 탐구
+-- explore_depth_anything.py # Depth Anything 모델 탐구
+-- compare_models.py         # 모델 비교 분석
+-- depth_basics.py           # 깊이 추정 기초 개념
```

---

## Step 1: 깊이 추정 기초 개념 탐구

### depth_basics.py

```python
"""
Monocular Depth Estimation 기초 개념 탐구
- 깊이맵이란?
- Relative vs Metric Depth
- 깊이맵 시각화
"""
import numpy as np
import matplotlib.pyplot as plt

def explore_depth_map():
    """깊이맵의 기본 구조 이해"""
    print("=" * 40)
    print("깊이맵(Depth Map) 기초")
    print("=" * 40)

    # 가상의 깊이맵 생성
    h, w = 480, 640

    # 1. 상대 깊이맵 (0~1)
    # 위쪽은 멀리(하늘), 아래쪽은 가까이(바닥)
    relative_depth = np.zeros((h, w), dtype=np.float32)
    for y in range(h):
        relative_depth[y, :] = y / h  # 아래쪽이 가까움 (값이 큼)

    # 가상의 장애물 추가 (가까운 물체)
    relative_depth[200:350, 250:400] = 0.9  # 가까운 박스

    print(f"  깊이맵 크기: {relative_depth.shape}")
    print(f"  값 범위: [{relative_depth.min():.2f}, {relative_depth.max():.2f}]")
    print(f"  가까운 물체 (높은 값): {relative_depth[275, 325]:.2f}")
    print(f"  먼 배경 (낮은 값): {relative_depth[50, 320]:.2f}")

    # 2. 절대 깊이로 변환 (예시)
    scale = 10.0  # 최대 10m
    metric_depth = relative_depth * scale

    print(f"\n  절대 깊이 변환 (scale={scale}m):")
    print(f"  가까운 물체: {metric_depth[275, 325]:.1f}m")
    print(f"  먼 배경: {metric_depth[50, 320]:.1f}m")

    # 시각화
    fig, axes = plt.subplots(1, 3, figsize=(15, 4))

    axes[0].imshow(relative_depth, cmap='magma')
    axes[0].set_title('상대 깊이맵 (Relative)')
    axes[0].colorbar = plt.colorbar(axes[0].images[0], ax=axes[0])

    axes[1].imshow(metric_depth, cmap='magma')
    axes[1].set_title('절대 깊이맵 (Metric, m)')

    # 깊이별 컬러맵 비교
    colormaps = ['magma', 'inferno', 'plasma']
    for ax, cmap in zip([axes[2]], ['turbo']):
        ax.imshow(relative_depth, cmap=cmap)
        ax.set_title(f'컬러맵: {cmap}')

    plt.tight_layout()
    plt.savefig('depth_basics.png', dpi=100)
    print("\n  시각화 저장: depth_basics.png")

def compare_depth_representations():
    """Relative vs Metric Depth 비교"""
    print("\n" + "=" * 40)
    print("Relative vs Metric Depth 비교")
    print("=" * 40)

    # 시나리오: 같은 장면, 다른 표현
    print("""
    장면: 사람(2.5m), 자동차(8m), 건물(25m)

    Relative Depth:
      사람:  0.92  (가장 가까움)
      자동차: 0.68
      건물:  0.12  (가장 멀리)
      → 순서 정보만 제공

    Metric Depth:
      사람:  2.5m  (정확한 거리)
      자동차: 8.0m
      건물:  25.0m
      → 실제 거리 정보 제공

    용도별 선택:
      장애물 유무 판단 → Relative (충분)
      정확한 정지 거리 → Metric (필요)
      SLAM 스케일 복구 → Metric (필수)
    """)

if __name__ == "__main__":
    explore_depth_map()
    compare_depth_representations()
```

---

## Step 2: MiDaS 모델 탐구

### explore_midas.py

```python
"""
MiDaS 모델 구조 탐구
- 모델 로드
- 구조 확인
- 파라미터 수 계산
"""
import torch

def explore_midas():
    """MiDaS 모델 구조 탐구"""
    print("=" * 40)
    print("MiDaS 모델 탐구")
    print("=" * 40)

    # MiDaS 모델 로드 (torch.hub)
    model_types = {
        "MiDaS_small": "경량 모델 (모바일용)",
        "DPT_Hybrid": "DPT 하이브리드 (CNN + ViT)",
        "DPT_Large": "DPT 대형 (ViT-Large)",
    }

    for model_type, desc in model_types.items():
        print(f"\n  모델: {model_type} - {desc}")
        try:
            model = torch.hub.load("intel-isl/MiDaS", model_type, trust_repo=True)
            model.eval()

            # 파라미터 수 계산
            total_params = sum(p.numel() for p in model.parameters())
            trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad)

            print(f"  총 파라미터: {total_params / 1e6:.1f}M")
            print(f"  학습 가능 파라미터: {trainable_params / 1e6:.1f}M")

            # 입출력 테스트
            dummy = torch.randn(1, 3, 384, 384)
            with torch.no_grad():
                output = model(dummy)
            print(f"  입력: {list(dummy.shape)}")
            print(f"  출력: {list(output.shape)}")

        except Exception as e:
            print(f"  로드 실패: {e}")
            print("  (인터넷 연결 필요)")

if __name__ == "__main__":
    explore_midas()
```

---

## Step 3: Depth Anything 모델 탐구

### explore_depth_anything.py

```python
"""
Depth Anything 모델 구조 탐구
- HuggingFace에서 로드
- 모델 구조 확인
- 추론 테스트
"""
from transformers import AutoModelForDepthEstimation, AutoImageProcessor
import torch

def explore_depth_anything():
    """Depth Anything 모델 구조 탐구"""
    print("=" * 40)
    print("Depth Anything 모델 탐구")
    print("=" * 40)

    models = {
        "LiheYoung/depth-anything-small-hf": "ViT-S (Small)",
        "LiheYoung/depth-anything-base-hf": "ViT-B (Base)",
    }

    for model_name, desc in models.items():
        print(f"\n  모델: {desc}")
        print(f"  경로: {model_name}")

        try:
            # 모델 로드
            model = AutoModelForDepthEstimation.from_pretrained(model_name)
            model.eval()

            # 파라미터 분석
            total_params = sum(p.numel() for p in model.parameters())
            print(f"  총 파라미터: {total_params / 1e6:.1f}M")

            # 모델 구조 요약
            print(f"  구조:")
            for name, module in model.named_children():
                num_params = sum(p.numel() for p in module.parameters())
                print(f"    {name}: {num_params / 1e6:.1f}M 파라미터")

            # 추론 테스트
            dummy = torch.randn(1, 3, 518, 518)
            with torch.no_grad():
                output = model(dummy)

            depth = output.predicted_depth
            print(f"  입력: {list(dummy.shape)}")
            print(f"  출력: {list(depth.shape)}")
            print(f"  깊이 범위: [{depth.min():.3f}, {depth.max():.3f}]")

        except Exception as e:
            print(f"  로드 실패: {e}")
            print("  (인터넷 연결 및 transformers 설치 필요)")

def analyze_vit_structure():
    """ViT 구조 상세 분석"""
    print("\n" + "=" * 40)
    print("ViT 구조 분석")
    print("=" * 40)

    print("""
    Depth Anything의 ViT 구조:

    1. Patch Embedding
       이미지 (3, 518, 518) → 패치 (N, 768)
       패치 크기: 14x14
       패치 수: (518/14)^2 = 37^2 = 1369

    2. Transformer Encoder
       Multi-Head Self-Attention × 12 (Small)
       → 각 패치가 다른 모든 패치와 관계 학습
       → 전역 컨텍스트 이해

    3. Decoder (DPT Head)
       Multi-scale Feature 결합
       → 1/4, 1/8, 1/16, 1/32 해상도 특징
       → 점진적 업샘플링으로 밀집 깊이맵 생성

    4. 출력
       (1, H, W) 깊이맵
       → 입력과 동일한 해상도
    """)

if __name__ == "__main__":
    explore_depth_anything()
    analyze_vit_structure()
```

---

## Step 4: 모델 비교 분석

### compare_models.py

```python
"""
MiDaS vs DPT vs Depth Anything 비교 분석
"""

def compare_models():
    """모델별 특성 비교표 출력"""
    print("=" * 60)
    print("Monocular Depth 모델 비교 분석")
    print("=" * 60)

    print("""
    +--------------+--------------+--------------+--------------+
    |  항목         |  MiDaS v3.1  |  DPT-Large   | Depth Any-S  |
    +--------------+--------------+--------------+--------------+
    | 연도         |  2022        |  2021        |  2024        |
    | Backbone     |  BEiT/Swin   |  ViT-Large   |  ViT-Small   |
    | 파라미터     |  ~345M       |  ~343M       |  ~24.8M      |
    | 학습 데이터  |  라벨 12종   |  라벨 1.2M   |  라벨+비라벨  |
    |              |              |              |  63.5M       |
    | Zero-shot    |  좋음        |  좋음        |  매우 우수    |
    | 속도 (A100)  |  ~40ms       |  ~45ms       |  ~12ms       |
    | Jetson 추론  |  ~200ms      |  ~350ms      |  ~50ms       |
    | 출력 타입    |  Relative    |  Relative    |  Relative    |
    | 실시간 가능  |  어려움      |  불가능      |  가능 (FP16) |
    +--------------+--------------+--------------+--------------+
    """)

    print("\n  용도별 추천:")
    print("  +----------------------+------------------+")
    print("  | 용도                  | 추천 모델         |")
    print("  +----------------------+------------------+")
    print("  | Jetson 실시간 추론   | Depth Anything-S |")
    print("  | 서버 고정밀 추론     | Depth Anything-L |")
    print("  | 연구/실험용         | DPT-Large        |")
    print("  | 레거시 호환         | MiDaS v3.1       |")
    print("  +----------------------+------------------+")

def compare_learning_methods():
    """학습 방법론 비교"""
    print("\n" + "=" * 60)
    print("학습 방법론 비교")
    print("=" * 60)

    print("""
    +------------------+--------------+--------------+--------------+
    |  항목             | Supervised   |Self-supervised|  Zero-shot   |
    +------------------+--------------+--------------+--------------+
    | GT 깊이 필요     |  필요        |  불필요       |  혼합        |
    | 학습 데이터      |  적음 (~1M)  |  중간         |  대규모      |
    |                  |              |  (동영상)     |  (60M+)      |
    | 일반화 능력      |  약함        |  보통         |  매우 강함   |
    | 정확도           |  높음        |  보통         |  높음        |
    | 대표 모델        |  AdaBins     |  Monodepth2  | Depth Any.   |
    | 특징             | 환경 특화    | 비디오 활용   | 범용적       |
    +------------------+--------------+--------------+--------------+
    """)

if __name__ == "__main__":
    compare_models()
    compare_learning_methods()
```

---

## 실행 순서

```bash
# 1. 깊이 추정 기초 개념
python depth_basics.py

# 2. MiDaS 모델 탐구
python explore_midas.py

# 3. Depth Anything 모델 탐구
python explore_depth_anything.py

# 4. 모델 비교 분석
python compare_models.py
```

### 예상 출력

```
========================================
깊이맵(Depth Map) 기초
========================================
  깊이맵 크기: (480, 640)
  값 범위: [0.00, 0.92]
  가까운 물체 (높은 값): 0.90
  먼 배경 (낮은 값): 0.10

========================================
Depth Anything 모델 탐구
========================================
  모델: ViT-S (Small)
  총 파라미터: 24.8M
  입력: [1, 3, 518, 518]
  출력: [1, 518, 518]
```

---

## [O] 체크리스트

- [ ] 깊이맵의 기본 개념 이해 (Relative vs Metric)
- [ ] Supervised/Self-supervised/Zero-shot 학습 방법 이해
- [ ] MiDaS 모델 로드 및 구조 확인
- [ ] DPT (Dense Prediction Transformer) 개념 이해
- [ ] Depth Anything 모델 로드 및 구조 확인
- [ ] ViT (Vision Transformer) 기본 구조 이해
- [ ] Teacher-Student 학습 프레임워크 이해
- [ ] Scale-Invariant Loss 개념 이해
- [ ] 모델별 비교 분석표 작성 완료

---

## [tip] 트러블슈팅

### HuggingFace 모델 다운로드 실패
```
문제: Connection timeout / 403 Forbidden
해결:
1. HuggingFace 로그인: huggingface-cli login
2. 프록시 설정 확인
3. 오프라인 모드: 미리 다운로드 후 로컬 경로 지정
```

### MiDaS torch.hub 로드 실패
```
문제: torch.hub.load() 실패
해결:
1. 인터넷 연결 확인
2. trust_repo=True 추가
3. 또는 git clone 후 로컬 로드
```

### GPU 메모리 부족
```
문제: CUDA out of memory
해결:
1. torch.no_grad() 블록 안에서 추론
2. 작은 모델부터 시작 (Small → Base → Large)
3. 입력 해상도 줄이기 (518 → 384)
```

---

**이전**: [Week 6 - TensorRT C++ 추론](../week6/PRACTICE.md)
**다음**: [Week 8 - Depth Anything 사용](../week8/PRACTICE.md)
