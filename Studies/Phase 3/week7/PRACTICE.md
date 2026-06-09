# Week 7 실습: Monocular Depth Estimation 이론 및 모델 탐구


> **목표**: 깊이 추정의 원리를 이해하고, 주요 모델의 구조와 성능을 비교 분석하기
> **언어**: Python (PyTorch, transformers)
> **예상 시간**: 12시간


---


## 실습 개요


Week 7은 이론 중심의 주차입니다. Monocular Depth Estimation의 핵심 개념을 학습하고, MiDaS/DPT/Depth Anything 모델을 Python으로 간단히 불러보며 구조를 파악합니다.


---


## 환경 설정


```bash
cd Studies/Phase\ 3/week7

# 가상환경 생성 및 의존성 설치 (apt 패키지 설치 포함, root 권한 필요)
# 내부에서 .venv-week7 생성 후 requirements.txt 설치
./pip_install.sh

# 가상환경 활성화 (sh 종료 후 현재 shell에서 직접 활성화 필요)
source .venv-week7/bin/activate


# 설치 확인
python -c "from transformers import pipeline; print('transformers OK')"
python -c "import timm; print(f'timm models: {len(timm.list_models())}')"
python -c "import torch; print(f'PyTorch: {torch.__version__}')"
```


---


## 프로젝트 구조


```
week7_depth_theory/
+-- explore_midas.py # MiDaS 모델 탐구
+-- explore_dpt.py # DPT 모델 탐구
+-- explore_depth_anything.py # Depth Anything 모델 탐구
+-- compare_models.py # 모델 비교 분석
+-- depth_basics.py # 깊이 추정 기초 개념
```


---


## Step 1: 깊이 추정 기초 개념 탐구

week1-6은 무엇이 어디에 있는가(검출)를 다뤘다. week7부터는 얼마나 멀리 있는가(깊이)로 넘어간다. 이 스텝은 깊이맵이 무엇이고, 상대 깊이와 절대 깊이가 어떻게 다른지 개념을 잡는다.

### 왜 깊이를 따로 추정하나

객체 검출만으로는 거리를 알 수 없다. BBox 크기로 짐작하려 해도 가까운 작은 사람과 먼 큰 사람을 구별하지 못한다. 그래서 깊이를 따로 추정한다. 사람은 양안 시차로, AI는 단일 이미지에서 학습으로 추론한다(단안 깊이 추정). 가장 중요한 개념이 relative와 metric depth의 차이다. 단안 추정은 보통 상대 깊이만 준다(어느 쪽이 더 가까운지는 알지만 몇 미터인지는 모른다). 이 한계가 week8의 metric depth 변환 실습으로 이어진다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 깊이맵 개념 | depth map 시각화 | 픽셀별 거리값의 2D 맵 |
| relative vs metric | 두 표현 비교 | 상대 깊이 vs 절대 거리(m) |
| 시각화 | colormap | 가까움/멂을 색으로 |

### 핵심 포인트

- 단안 깊이 추정의 본질적 한계는 절대 스케일을 모른다는 것이다(scale ambiguity). 사진 한 장만으로는 같은 장면이 실제로 작은 모형인지 큰 실물인지 구분할 수 없다.
- 이 스텝은 코드보다 개념이 핵심이다. 이후 모델 탐구의 토대다.


### depth_basics.py


```python
"""
Monocular Depth Estimation 기초 개념 탐구
- 깊이맵이란?
- Relative vs Metric Depth
- 깊이맵 시각화
"""
import os
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
    relative_depth = np.zeros((h, w), dtype=np.float32) # 깊이맵 (h x w 2차원 배열)
    for y in range(h): # 각 행을 위에서 아래로 채움
        relative_depth[y, :] = y / h # 아래쪽이 가까움 (값이 큼)


    # 가상의 장애물 추가 (가까운 물체)
    relative_depth[200:350, 250:400] = 0.9 # 특정 사각 영역을 가까운 박스로 설정


    print(f"깊이맵 크기: {relative_depth.shape}")
    print(f"값 범위: [{relative_depth.min():.2f}, {relative_depth.max():.2f}]")
    print(f"가까운 물체 (높은 값): {relative_depth[275, 325]:.2f}")
    print(f"먼 배경 (낮은 값): {relative_depth[50, 320]:.2f}")


    # 2. 절대 깊이로 변환 (예시)
    scale = 10.0 # 최대 10m
    metric_depth = relative_depth * scale # 0~1 상대값에 거리 스케일을 곱해 실제 미터로


    print(f"\n 절대 깊이 변환 (scale={scale}m):")
    print(f"가까운 물체: {metric_depth[275, 325]:.1f}m")
    print(f"먼 배경: {metric_depth[50, 320]:.1f}m")


    # 시각화
    fig, axes = plt.subplots(1, 3, figsize=(15, 4)) # 그래프 3개를 가로로 배치


    axes[0].imshow(relative_depth, cmap='magma') # 상대 깊이맵 표시
    axes[0].set_title('Relative Depth Map')
    axes[0].colorbar = plt.colorbar(axes[0].images[0], ax=axes[0]) # 색상-깊이 대응 막대


    axes[1].imshow(metric_depth, cmap='magma') # 절대 깊이맵 표시
    axes[1].set_title('Metric Depth Map (m)')


    # 깊이별 컬러맵 비교
    for ax, cmap in zip([axes[2]], ['turbo']): # 세 번째 칸에 turbo 컬러맵 적용
        ax.imshow(relative_depth, cmap=cmap)
        ax.set_title(f'Colormap: {cmap}')


    plt.tight_layout()
    os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (시각화를 수업 자료와 분리)
    plt.savefig('outputs/depth_basics.png', dpi=100) # 결과 이미지 저장
    print("\n 시각화 저장: outputs/depth_basics.png")


def compare_depth_representations():
    """Relative vs Metric Depth 비교"""
    print("\n" + "=" * 40)
    print("Relative vs Metric Depth 비교")
    print("=" * 40)


    # 시나리오: 같은 장면, 다른 표현
    print("""
    장면: 사람(2.5m), 자동차(8m), 건물(25m)


    Relative Depth:
      사람: 0.92 (가장 가까움)
      자동차: 0.68
      건물: 0.12 (가장 멀리)
      → 순서 정보만 제공


    Metric Depth:
      사람: 2.5m (정확한 거리)
      자동차: 8.0m
      건물: 25.0m
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

단안 깊이 추정의 대표 1세대 모델 MiDaS를 직접 불러 추론해보고, 그 구조와 출력을 확인한다.

### 왜 MiDaS부터 보나

MiDaS의 핵심 기여는 서로 다른 데이터셋(야외 0-80m, 실내 0-10m 등)을 섞어 학습하는 방법을 제시한 것이다. 그냥 섞으면 손실 함수가 큰 숫자(야외)에 휘둘려 실내 학습이 무시된다. MiDaS는 스케일에 무관한 손실로 이를 풀어 환경에 강건한(zero-shot) 깊이 모델을 만들었다. 이 스텝에서는 `torch.hub`로 모델을 불러 임의 이미지에 추론하는 흐름을 익힌다. backbone이 CNN(v1-2)에서 ViT(v3, DPT)로 진화한 점도 확인한다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 모델 로드 | `torch.hub.load` | MiDaS 불러오기 |
| 추론 | 이미지 -> 깊이맵 | 단안 깊이 추정 실행 |
| 출력 해석 | 상대 깊이맵 | MiDaS는 relative depth 출력 |

### 핵심 포인트

- MiDaS 출력은 상대 깊이다(inverse depth 형태일 수 있음). 절대 미터가 아니다.
- `torch.hub` 로드가 실패하면(트러블슈팅) 네트워크/캐시 문제다. week8에서 쓰는 HuggingFace 방식과는 다른 경로다.


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
    model_types = { # 탐구할 MiDaS 계열 모델 3종
        "MiDaS_small": "경량 모델 (모바일용)",
        "DPT_Hybrid": "DPT 하이브리드 (CNN + ViT)",
        "DPT_Large": "DPT 대형 (ViT-Large)",
    }


    for model_type, desc in model_types.items(): # 모델을 하나씩 로드해 분석
        print(f"\n 모델: {model_type} - {desc}")
        try:
            model = torch.hub.load("intel-isl/MiDaS", model_type, trust_repo=True) # torch.hub에서 다운로드
            model.eval() # 평가 모드


            # 파라미터 수 계산
            total_params = sum(p.numel() for p in model.parameters()) # 전체 가중치 수
            trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad) # 그중 학습되는 것


            print(f"총 파라미터: {total_params / 1e6:.1f}M")
            print(f"학습 가능 파라미터: {trainable_params / 1e6:.1f}M")


            # 입출력 테스트
            dummy = torch.randn(1, 3, 384, 384) # 가짜 입력 (384x384 RGB)
            with torch.no_grad(): # 추론이므로 gradient 계산 끔
                output = model(dummy)
            print(f"입력: {list(dummy.shape)}")
            print(f"출력: {list(output.shape)}")


        except Exception as e: # 로드 실패 시 (예: 인터넷 연결 없음)
            print(f"로드 실패: {e}")
            print("(인터넷 연결 필요)")


if __name__ == "__main__":
    explore_midas()
```


---


## Step 3: Depth Anything 모델 탐구

최신 단안 깊이 모델 Depth Anything을 불러 MiDaS와 같은 방식으로 추론해본다. week8에서 본격적으로 쓸 모델을 미리 만나는 단계다.

### 왜 Depth Anything인가

Depth Anything은 대규모 비라벨 데이터를 Teacher-Student(지식 증류)로 활용해 MiDaS보다 더 강건하고 정밀한 깊이를 낸다. zero-shot 계열의 현재 대표 모델이다. HuggingFace transformers의 pipeline이나 모델로 쉽게 불러온다. week8 전체가 이 모델 기반이므로, 여기서 로드와 추론 방식, 출력 형태를 먼저 익혀둔다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 모델 로드 | HuggingFace transformers | Depth Anything 불러오기 |
| 추론 | 이미지 -> 깊이맵 | 추론 실행 |
| MiDaS와 비교 | 같은 입력 출력 대조 | 품질 차이 감 잡기 |

### 핵심 포인트

- 로드 경로가 MiDaS(`torch.hub`)와 다르다(HuggingFace). 두 생태계를 모두 경험한다.
- 여전히 기본은 상대 깊이다. metric으로 쓰려면 변환이 필요하다(week8).


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


    models = { # 탐구할 Depth Anything 모델 2종
        "depth-anything/Depth-Anything-V2-Small-hf": "ViT-S (Small)",
        "depth-anything/Depth-Anything-V2-Base-hf": "ViT-B (Base)",
    }


    for model_name, desc in models.items(): # 모델을 하나씩 로드해 분석
        print(f"\n 모델: {desc}")
        print(f"경로: {model_name}")


        try:
            # 모델 로드
            model = AutoModelForDepthEstimation.from_pretrained(model_name) # HuggingFace에서 다운로드
            model.eval() # 평가 모드


            # 파라미터 분석
            total_params = sum(p.numel() for p in model.parameters()) # 전체 가중치 수
            print(f"총 파라미터: {total_params / 1e6:.1f}M")


            # 모델 구조 요약
            print(f"구조:")
            for name, module in model.named_children(): # 최상위 하위 모듈별 파라미터 수 출력
                num_params = sum(p.numel() for p in module.parameters())
                print(f"{name}: {num_params / 1e6:.1f}M 파라미터")


            # 추론 테스트
            dummy = torch.randn(1, 3, 518, 518) # 가짜 입력 (518x518 RGB)
            with torch.no_grad(): # 추론이므로 gradient 계산 끔
                output = model(dummy)


            depth = output.predicted_depth # 예측된 깊이맵
            print(f"입력: {list(dummy.shape)}")
            print(f"출력: {list(depth.shape)}")
            print(f"깊이 범위: [{depth.min():.3f}, {depth.max():.3f}]")


        except Exception as e: # 로드 실패 시 (예: 인터넷/패키지 없음)
            print(f"로드 실패: {e}")
            print("(인터넷 연결 및 transformers 설치 필요)")


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

앞서 본 MiDaS와 Depth Anything을 같은 이미지들에 돌려 출력 품질과 속도를 나란히 비교한다. week7 이론 주차를 정리하는 단계다.

### 왜 비교하나

모델 선택은 정확도, 속도, 메모리의 트레이드오프다(week3의 모델 크기 선택, week5의 양자화와 같은 사고방식). 같은 입력에 두 모델을 돌려 깊이맵을 나란히 두고, 경계 선명도, 노이즈, 추론 시간을 비교해 어느 상황에 무엇을 쓸지 판단 기준을 세운다. 이것이 깊이 파트의 이론 마무리이고, week8에서 선택한 모델로 실전(시각화, metric 변환, YOLO 결합)을 한다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 동일 입력 비교 | 두 모델 같은 이미지 | 공정 비교 |
| 정성 평가 | 깊이맵 나란히 시각화 | 경계/노이즈 차이 |
| 정량 평가 | 추론 시간 | 속도 트레이드오프 |

### 핵심 포인트

- 더 좋은 모델은 없고 상황별 트레이드오프만 있다. 실시간이면 빠른 쪽, 품질 우선이면 정밀한 쪽이다.
- 비교는 반드시 같은 입력과 같은 전처리로 한다. 안 그러면 모델 차이가 아니라 조건 차이를 보게 된다(week5 벤치마크와 같은 원칙).


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
    | 항목 | MiDaS v3.1 | DPT-Large | Depth Any-S |
    +--------------+--------------+--------------+--------------+
    | 연도 | 2022 | 2021 | 2024 |
    | Backbone | BEiT/Swin | ViT-Large | ViT-Small |
    | 파라미터 | ~345M | ~343M | ~24.8M |
    | 학습 데이터 | 라벨 12종 | 라벨 1.2M | 라벨+비라벨 |
    | | | | 63.5M |
    | Zero-shot | 좋음 | 좋음 | 매우 우수 |
    | 속도 (A100) | ~40ms | ~45ms | ~12ms |
    | Jetson 추론 | ~200ms | ~350ms | ~50ms |
    | 출력 타입 | Relative | Relative | Relative |
    | 실시간 가능 | 어려움 | 불가능 | 가능 (FP16) |
    +--------------+--------------+--------------+--------------+
    """)


    print("\n 용도별 추천:")
    print("+----------------------+------------------+")
    print("| 용도 | 추천 모델 |")
    print("+----------------------+------------------+")
    print("| Jetson 실시간 추론 | Depth Anything-S |")
    print("| 서버 고정밀 추론 | Depth Anything-L |")
    print("| 연구/실험용 | DPT-Large |")
    print("| 레거시 호환 | MiDaS v3.1 |")
    print("+----------------------+------------------+")


def compare_learning_methods():
    """학습 방법론 비교"""
    print("\n" + "=" * 60)
    print("학습 방법론 비교")
    print("=" * 60)


    print("""
    +------------------+--------------+--------------+--------------+
    | 항목 | Supervised |Self-supervised| Zero-shot |
    +------------------+--------------+--------------+--------------+
    | GT 깊이 필요 | 필요 | 불필요 | 혼합 |
    | 학습 데이터 | 적음 (~1M) | 중간 | 대규모 |
    | | | (동영상) | (60M+) |
    | 일반화 능력 | 약함 | 보통 | 매우 강함 |
    | 정확도 | 높음 | 보통 | 높음 |
    | 대표 모델 | AdaBins | Monodepth2 | Depth Any. |
    | 특징 | 환경 특화 | 비디오 활용 | 범용적 |
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


## 체크리스트


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


## 트러블슈팅


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
