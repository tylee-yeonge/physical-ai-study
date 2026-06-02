# Week 8 실습: Depth Anything 모델 사용 및 시각화


> **목표**: Depth Anything으로 실제 이미지 깊이 추론, 시각화, Metric 변환 실험하기
> **언어**: Python (PyTorch, transformers, OpenCV)
> **예상 시간**: 12시간


---


## 실습 개요


Week 8은 Depth Anything 모델을 실제로 사용합니다. HuggingFace에서 모델을 불러오고, 단일 이미지/비디오에서 깊이맵을 추론한 후 다양한 방법으로 시각화합니다. Metric Depth 변환과 YOLO+Depth 결합까지 실습합니다.


---


## 환경 설정


```bash
cd Studies/Phase\ 3/week8

# 가상환경 생성 및 의존성 설치 (apt 패키지 설치 포함, root 권한 필요)
# 내부에서 .venv-week8 생성 후 requirements.txt 설치
./pip_install.sh

# 가상환경 활성화 (sh 종료 후 현재 shell에서 직접 활성화 필요)
source .venv-week8/bin/activate


# 설치 확인
python -c "
from transformers import pipeline
pipe = pipeline('depth-estimation', model='depth-anything/Depth-Anything-V2-Small-hf')
print('Depth Anything 로드 성공!')
"
```


---


## 프로젝트 구조


```
week8_depth_anything/
+-- infer_pipeline.py # Pipeline API 추론
+-- infer_manual.py # 직접 모델 로드 추론
+-- visualize_depth.py # 깊이맵 시각화 (PNG)
+-- visualize_depth_3d.py # depth -> 3D point cloud (옵션, Rerun)
+-- metric_depth.py # Metric Depth 변환
+-- realtime_depth.py # 실시간 카메라 깊이
+-- yolo_depth_combine.py # YOLO + Depth 결합 (미리보기)
+-- data/
    +-- indoor.jpg # 실내 테스트 이미지
    +-- outdoor.jpg # 야외 테스트 이미지
```


---


## Step 1: Pipeline API로 간단 추론

week7에서 개념과 모델을 살펴봤다. week8은 Depth Anything으로 실제 작업을 한다. 첫 스텝은 HuggingFace pipeline으로 가장 적은 코드로 깊이를 추정해보는 것이다.

### 왜 pipeline부터 하나

pipeline API는 모델 로드 -> 전처리 -> 추론 -> 후처리를 몇 줄로 끝낸다. 내부적으로 다 알아서 해주므로 빠른 프로토타이핑에 좋다. 되는지 먼저 확인하는 용도다. 이 편리함의 대가가 Step 2의 주제다. 세밀한 제어가 안 된다. 먼저 쉬운 길을 보고, 그다음 직접 제어하는 길을 본다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| pipeline 생성 | `pipeline("depth-estimation", model=...)` | 한 줄로 모델 준비 |
| 추론 | `depth_pipe(image)` | 이미지 -> 깊이 결과 |
| 결과 확인 | `output["depth"]` | 깊이맵 꺼내기 |

### 핵심 포인트

- pipeline이 전처리/후처리를 숨긴다. 편하지만 무슨 일이 일어나는지 보이지 않는다. Step 2에서 그것을 펼친다.
- 첫 실행은 모델 다운로드로 느리다(트러블슈팅).


### infer_pipeline.py


```python
"""
HuggingFace Pipeline으로 Depth Anything 추론
가장 간단한 방법!
"""
import os
import time
import numpy as np
from PIL import Image
from transformers import pipeline


def infer_with_pipeline(image_path, model_name="depth-anything/Depth-Anything-V2-Small-hf"):
    """Pipeline API로 깊이 추론"""
    print("=" * 40)
    print("Pipeline API 깊이 추론")
    print("=" * 40)


    # 파이프라인 생성
    depth_pipe = pipeline( # 깊이 추정 파이프라인 (전처리+추론+후처리 일괄 수행)
        task="depth-estimation",
        model=model_name
    )
    print(f"모델: {model_name}")


    # 추론
    result = depth_pipe(image_path) # 이미지 -> 깊이 추정 결과 dict


    # 결과 분석
    depth_image = result["depth"] # PIL Image
    depth_array = np.array(depth_image) # PIL Image -> numpy 배열


    print(f"입력: {image_path}")
    print(f"출력 크기: {depth_array.shape}")
    print(f"값 범위: [{depth_array.min()}, {depth_array.max()}]")
    print(f"데이터 타입: {depth_array.dtype}")


    # 저장
    os.makedirs("outputs", exist_ok=True) # 결과물 폴더 (수업 자료와 분리)
    depth_image.save("outputs/depth_pipeline_result.png") # 깊이맵 이미지 파일로 저장
    print(f"저장: outputs/depth_pipeline_result.png")


    return depth_array


def compare_models(image_path):
    """모델 크기별 비교"""
    print("\n" + "=" * 40)
    print("모델 크기별 비교")
    print("=" * 40)


    models = [ # 비교할 모델 2종 (경로, 표시 이름)
        ("depth-anything/Depth-Anything-V2-Small-hf", "ViT-S"),
        ("depth-anything/Depth-Anything-V2-Base-hf", "ViT-B"),
    ]


    for model_name, desc in models: # 모델별로 추론 속도 측정
        pipe = pipeline("depth-estimation", model=model_name)


        # Warm-up
        pipe(image_path) # 측정 전 1회 실행 (첫 추론은 느림)


        # 속도 측정 (5회 평균)
        times = []
        for _ in range(5):
            start = time.time()
            result = pipe(image_path)
            times.append(time.time() - start)


        avg_ms = np.mean(times) * 1000 # 평균 추론 시간 (ms)
        print(f"{desc}: {avg_ms:.1f} ms/frame ({1000/avg_ms:.1f} FPS)")


if __name__ == "__main__":
    # 테스트 이미지 경로 (실제 이미지로 교체)
    image_path = "data/indoor.jpg"


    # Pipeline 추론
    depth = infer_with_pipeline(image_path)


    # 모델 비교 (선택)
    # compare_models(image_path)
```


---


## Step 2: 직접 모델 로드 추론

pipeline이 감춘 단계를 직접 펼친다. 모델과 전처리기(processor)를 따로 불러 전처리 -> 추론 -> 후처리를 손으로 한다.

### 왜 직접 로드하나

pipeline은 편하지만 배포용으로는 부족하다. 입력 크기, 출력 후처리, 디바이스 배치 등을 제어하려면 모델과 processor를 분리해 다뤄야 한다. week5에서 ONNX Runtime을 직접 다룬 것과 같은 동기다. 편한 추상화에서 제어 가능한 저수준으로. 이 방식이 실제 시스템에 넣을 때의 기본형이다. pipeline은 데모, 이것이 실전이다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 분리 로드 | `AutoModel` + `AutoImageProcessor` | 모델/전처리기를 따로 |
| 전처리 | `processor(image)` | 입력 텐서 만들기 |
| 추론/후처리 | `model(**inputs)`, `interpolate` | 출력을 원본 크기로 복원 |

### 핵심 포인트

- 출력 깊이맵을 원본 이미지 크기로 다시 보간(interpolate)해야 한다. 모델은 고정 크기로 처리하기 때문이다.
- Step 1과 결과가 같아야 정상이다. 직접 한 것이 pipeline과 일치하는지 확인하는 것이 sanity check다.


### infer_manual.py


```python
"""
직접 모델 로드로 상세한 깊이 추론
배치 처리, GPU 제어, 중간 출력 접근 가능
"""
import torch
import numpy as np
import cv2
import time
from transformers import AutoModelForDepthEstimation, AutoImageProcessor
from PIL import Image


class DepthAnythingInference:
    def __init__(self, model_name="depth-anything/Depth-Anything-V2-Small-hf", device=None):
        """Depth Anything 추론기 초기화"""
        if device is None: # device 미지정 시 자동 선택
            self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        else:
            self.device = torch.device(device)


        print(f"장치: {self.device}")


        # 모델 로드
        self.model = AutoModelForDepthEstimation.from_pretrained(model_name) # HuggingFace에서 모델 로드
        self.model.to(self.device) # 디바이스로 이동
        self.model.eval() # 평가 모드


        # 이미지 프로세서 로드
        self.processor = AutoImageProcessor.from_pretrained(model_name) # 모델에 맞는 전처리기


        # 파라미터 수
        total_params = sum(p.numel() for p in self.model.parameters())
        print(f"파라미터: {total_params / 1e6:.1f}M")


    def infer(self, image):
        """단일 이미지 깊이 추론


        Args:
            image: PIL Image 또는 numpy array (BGR)
        Returns:
            depth_map: numpy array [H, W] (float32)
        """
        # numpy → PIL 변환
        if isinstance(image, np.ndarray): # numpy(BGR)면 PIL(RGB)로 변환
            image_pil = Image.fromarray(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))
        else:
            image_pil = image


        orig_size = image_pil.size # (W, H)


        # 전처리
        inputs = self.processor(images=image_pil, return_tensors="pt") # 모델 입력 텐서로 변환
        inputs = {k: v.to(self.device) for k, v in inputs.items()} # 입력을 디바이스로 이동


        # 추론
        with torch.no_grad(): # 추론이므로 gradient 계산 끔
            outputs = self.model(**inputs)


        # 깊이맵 추출 및 리사이즈
        depth = outputs.predicted_depth.squeeze() # [H_model, W_model]


        # 원본 크기로 리사이즈
        depth = torch.nn.functional.interpolate( # 모델 출력 크기 -> 원본 크기로 보간
            depth.unsqueeze(0).unsqueeze(0), # interpolate는 4차원 입력 필요
            size=(orig_size[1], orig_size[0]), # (H, W)
            mode="bicubic",
            align_corners=False
        ).squeeze()


        depth_numpy = depth.cpu().numpy() # 텐서 -> numpy 배열


        return depth_numpy


    def infer_batch(self, images):
        """배치 추론 (여러 이미지 동시)


        Args:
            images: list of PIL Images
        Returns:
            list of depth maps
        """
        inputs = self.processor(images=images, return_tensors="pt") # 여러 이미지를 한 배치로 묶어 전처리
        inputs = {k: v.to(self.device) for k, v in inputs.items()}


        with torch.no_grad():
            outputs = self.model(**inputs)


        depths = outputs.predicted_depth # [B, H, W]


        results = []
        for i, img in enumerate(images): # 이미지별로 원본 크기 복원
            d = depths[i].unsqueeze(0).unsqueeze(0)
            d = torch.nn.functional.interpolate(
                d, size=img.size[::-1], mode="bicubic", align_corners=False
            ).squeeze()
            results.append(d.cpu().numpy())


        return results


    def benchmark(self, image, num_runs=20):
        """추론 속도 벤치마크"""
        # Warm-up
        for _ in range(3): # 측정 전 워밍업 (첫 추론은 느림)
            self.infer(image)


        # 측정
        times = []
        for _ in range(num_runs):
            start = time.time()
            self.infer(image)
            times.append(time.time() - start)


        avg_ms = np.mean(times) * 1000 # 평균 추론 시간 (ms)
        std_ms = np.std(times) * 1000 # 표준편차 (ms)
        fps = 1000 / avg_ms # 초당 프레임 수


        print(f"\n 벤치마크 결과 ({num_runs}회):")
        print(f"평균: {avg_ms:.1f} ms (+/- {std_ms:.1f})")
        print(f"FPS: {fps:.1f}")


        return avg_ms


if __name__ == "__main__":
    print("=" * 40)
    print("Depth Anything 직접 추론")
    print("=" * 40)


    # 추론기 초기화
    inferencer = DepthAnythingInference() # 기본 모델로 추론기 생성


    # 이미지 로드
    image = Image.open("data/indoor.jpg")
    print(f"이미지 크기: {image.size}")


    # 추론
    depth_map = inferencer.infer(image) # 깊이맵 추론
    print(f"깊이맵 크기: {depth_map.shape}")
    print(f"깊이 범위: [{depth_map.min():.3f}, {depth_map.max():.3f}]")


    # 배치 추론 (같은 이미지를 2장 묶어 한 번에 처리)
    depth_maps = inferencer.infer_batch([image, image]) # 결과는 이미지별 깊이맵 리스트
    print(f"배치 결과: {len(depth_maps)}장, 각 shape={depth_maps[0].shape}")


    # 벤치마크
    inferencer.benchmark(image)
```


---


## Step 3: 깊이맵 시각화

추정한 깊이맵을 사람이 보기 좋게 컬러맵으로 시각화한다. 단일 채널 회색조 깊이값에 색을 입혀 거리를 직관적으로 본다.

### 왜 컬러맵을 씌우나

깊이맵은 픽셀마다 깊이값 하나인 단일 채널 데이터라 그대로는 회색조라 읽기 어렵다. magma 같은 컬러맵으로 가까움/멂을 색 차이로 만들면 한눈에 들어온다. week4의 라벨 시각화, week6의 박스 시각화와 같은 결과를 눈으로 확인하는 단계다. 시각화는 디버깅 도구이기도 하다. 깊이맵이 이상하면(트러블슈팅) 색으로 바로 드러난다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 정규화 | min-max로 0-1 | 컬러맵 적용 전 스케일 맞추기 |
| 컬러맵 | `plt.get_cmap("magma")` | 깊이를 색으로 |
| 저장/표시 | 원본과 나란히 | 입력과 깊이 대조 |

### 핵심 포인트

- 컬러맵 적용 전 깊이값을 0-1로 정규화해야 색이 제대로 분포한다.
- 상대 깊이라 색은 이 장면 안에서의 상대 거리일 뿐이다. 절대 미터가 아니다(Step 4에서 변환).


### visualize_depth.py


```python
"""
깊이맵 시각화: 다양한 컬러맵 및 표현 방법
"""
import os
import numpy as np
import cv2
import matplotlib.pyplot as plt
from PIL import Image
from transformers import pipeline


def normalize_depth(depth_map):
    """깊이맵 정규화 (0~1)"""
    d_min = depth_map.min()
    d_max = depth_map.max()
    if d_max - d_min < 1e-6: # 전부 같은 값이면 0으로 (0 나눗셈 방지)
        return np.zeros_like(depth_map)
    return (depth_map - d_min) / (d_max - d_min) # 최소~최대를 0~1 범위로 선형 변환


def depth_to_colormap(depth_map, colormap_name='magma'):
    """깊이맵에 컬러맵 적용"""
    depth_norm = normalize_depth(depth_map) # 0~1로 정규화
    cmap = plt.get_cmap(colormap_name) # 컬러맵 함수 가져오기
    colored = cmap(depth_norm)[:, :, :3] # alpha 제거 (RGBA -> RGB)
    colored = (colored * 255).astype(np.uint8) # 0~1 실수 -> 0~255 정수
    return colored


def visualize_side_by_side(image_path, depth_map, save_path="outputs/depth_comparison.png"):
    """원본 이미지와 깊이맵 나란히 시각화"""
    fig, axes = plt.subplots(1, 2, figsize=(14, 5)) # 그래프 2개 가로 배치


    # 원본 이미지
    image = Image.open(image_path)
    axes[0].imshow(image)
    axes[0].set_title('원본 이미지', fontsize=14)
    axes[0].axis('off')


    # 깊이맵
    im = axes[1].imshow(depth_map, cmap='magma')
    axes[1].set_title('깊이맵 (Depth Anything)', fontsize=14)
    axes[1].axis('off')
    plt.colorbar(im, ax=axes[1], fraction=0.046, pad=0.04) # 색상-깊이 대응 막대


    plt.tight_layout()
    plt.savefig(save_path, dpi=150, bbox_inches='tight') # 결과 이미지 저장
    print(f"저장: {save_path}")
    plt.close()


def visualize_colormaps(depth_map, save_path="outputs/depth_colormaps.png"):
    """다양한 컬러맵 비교"""
    colormaps = ['magma', 'inferno', 'turbo', 'viridis', 'plasma', 'gray'] # 비교할 컬러맵 6종


    fig, axes = plt.subplots(2, 3, figsize=(15, 8)) # 2x3 그리드
    axes = axes.flatten() # 2차원 배열 -> 1차원으로 펴서 순회 편하게


    for ax, cmap_name in zip(axes, colormaps): # 컬러맵별로 같은 깊이맵 표시
        ax.imshow(depth_map, cmap=cmap_name)
        ax.set_title(f'컬러맵: {cmap_name}', fontsize=12)
        ax.axis('off')


    plt.suptitle('깊이맵 컬러맵 비교', fontsize=16)
    plt.tight_layout()
    plt.savefig(save_path, dpi=150, bbox_inches='tight')
    print(f"저장: {save_path}")
    plt.close()


def visualize_depth_histogram(depth_map, save_path="outputs/depth_histogram.png"):
    """깊이값 분포 히스토그램"""
    fig, axes = plt.subplots(1, 2, figsize=(12, 4))


    # 깊이맵
    axes[0].imshow(depth_map, cmap='magma')
    axes[0].set_title('깊이맵')
    axes[0].axis('off')


    # 히스토그램
    axes[1].hist(depth_map.flatten(), bins=100, color='steelblue', alpha=0.7) # 모든 픽셀의 깊이값 분포
    axes[1].set_xlabel('깊이 값')
    axes[1].set_ylabel('픽셀 수')
    axes[1].set_title('깊이값 분포')
    axes[1].axvline(depth_map.mean(), color='red', linestyle='--', # 평균값 위치에 세로선
                     label=f'평균: {depth_map.mean():.2f}')
    axes[1].legend()


    plt.tight_layout()
    plt.savefig(save_path, dpi=150, bbox_inches='tight')
    print(f"저장: {save_path}")
    plt.close()


def create_overlay(image_path, depth_map, alpha=0.5, save_path="outputs/depth_overlay.png"):
    """원본 이미지 위에 깊이맵 오버레이"""
    # 원본 이미지 로드
    image = cv2.imread(image_path)
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB) # BGR -> RGB


    # 깊이맵 컬러맵 적용
    depth_colored = depth_to_colormap(depth_map, 'turbo') # 깊이맵을 컬러 이미지로


    # 크기 맞추기
    if depth_colored.shape[:2] != image.shape[:2]: # 크기 다르면 깊이맵을 이미지 크기에 맞춤
        depth_colored = cv2.resize(depth_colored,
                                    (image.shape[1], image.shape[0]))


    # 오버레이
    overlay = cv2.addWeighted(image, 1 - alpha, depth_colored, alpha, 0) # 두 이미지를 alpha 비율로 합성


    # 저장
    plt.figure(figsize=(10, 6))
    plt.imshow(overlay)
    plt.title(f'깊이 오버레이 (alpha={alpha})')
    plt.axis('off')
    plt.savefig(save_path, dpi=150, bbox_inches='tight')
    print(f"저장: {save_path}")
    plt.close()


if __name__ == "__main__":
    print("=" * 40)
    print("깊이맵 시각화")
    print("=" * 40)


    image_path = "data/indoor.jpg"


    # 깊이 추론
    pipe = pipeline("depth-estimation", model="depth-anything/Depth-Anything-V2-Small-hf")
    result = pipe(image_path)
    depth_map = np.array(result["depth"]).astype(np.float32) # 깊이맵을 numpy float32로 변환
    print(f"깊이맵 크기: {depth_map.shape}")


    # 시각화
    os.makedirs("outputs", exist_ok=True) # 결과물 폴더 (시각화를 수업 자료와 분리)
    visualize_side_by_side(image_path, depth_map) # 원본+깊이맵 나란히
    visualize_colormaps(depth_map) # 컬러맵 6종 비교
    visualize_depth_histogram(depth_map) # 깊이값 히스토그램
    create_overlay(image_path, depth_map, alpha=0.4) # 원본 위에 깊이맵 오버레이


    print("\n 모든 시각화 완료!")
```


---


## Step 3-B (옵션): Rerun 으로 3D point cloud 시각화


> **요구사항**: 로컬 (맥북 등) 에 `rerun-sdk` 설치. 자세한 설치/사용은 [`ENVIRONMENT.md` Section 4-1](../../../ENVIRONMENT.md) 참조.
> **학습 효과**: depth map 의 "공간감" 을 직접 확인. depth -> camera intrinsics 로 unproject 해서 3D point cloud 로 펼치는 패턴은 Phase 4 / Phase 6 에서 반복 등장한다.
> **건너뛰기 OK**: 환경 준비 안 됐으면 Step 3 의 PNG 시각화로 충분히 학습 검증 가능.

깊이맵을 2D 색 그림이 아니라 3D 점구름(point cloud)으로 띄운다. 각 픽셀을 깊이에 따라 공간 좌표로 펼쳐 Rerun 뷰어로 본다.

### 왜 3D로 펼쳐 보나

2D 깊이맵은 이 픽셀이 얼마나 먼지를 색으로만 보여준다. 3D point cloud는 그 깊이로 픽셀을 실제 공간에 흩뿌려, 장면의 입체 구조를 회전하며 볼 수 있다. 깊이 추정이 진짜 3D 정보를 담고 있음을 체감하는 단계다. 옵션인 이유는 Rerun 설치와 뷰어가 필요하고 핵심 학습에는 필수가 아니기 때문이다. 깊이를 공간으로 이해하고 싶을 때 보는 보너스다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 역투영 | 깊이 + 카메라 내부파라미터 | 픽셀을 3D 좌표로 |
| point cloud | (x, y, z) + 색 | 3D 점 집합 생성 |
| Rerun 시각화 | `rr.log` | 인터랙티브 3D 뷰 |

### 핵심 포인트

- 픽셀을 3D로 펼치려면 카메라 내부 파라미터(초점거리 등)가 필요하다. 없으면 가정값을 쓴다(형태는 보이되 절대 스케일은 부정확).
- 상대 깊이 기반이라 point cloud의 절대 크기는 의미가 약하다. 구조를 보는 용도다. unproject 패턴은 Phase 4/6에서 다시 등장한다.


### visualize_depth_3d.py


```python
"""
Step 3-B (옵션): depth map -> 3D point cloud + Rerun 시각화

depth 의 각 픽셀을 카메라 intrinsics 로 unproject 해 3D 점으로 만들고,
원본 이미지 색을 그대로 입혀 색칠된 point cloud 로 띄운다.

PNG 의 colored depth 가 보여주지 못하는 "공간감" 을 회전/줌으로 직접 확인.
"""
import os
import numpy as np
import rerun as rr # 3D 시각화 도구
from PIL import Image
from transformers import pipeline


def depth_to_pointcloud(depth_map, image_rgb, fx, fy, cx, cy):
    """depth (H, W) + RGB (H, W, 3) -> points (N, 3), colors (N, 3).

    Args:
        depth_map: relative depth 또는 metric depth. (H, W).
        image_rgb: 원본 RGB. (H, W, 3) uint8.
        fx, fy, cx, cy: 카메라 intrinsics (픽셀 단위).

    Returns:
        points: (N, 3) [X, Y, Z]
        colors: (N, 3) uint8

    Note:
        Depth Anything 의 출력은 relative depth (스케일 미상) 라서
        절대 거리 (미터) 가 필요하면 Step 4 의 metric_depth 변환 적용 후 사용.
    """
    H, W = depth_map.shape
    xs, ys = np.meshgrid(np.arange(W), np.arange(H)) # 각 픽셀의 (x, y) 좌표 격자

    # 핀홀 unproject: (u, v, Z) -> (X, Y, Z)
    Z = depth_map.astype(np.float32) # 깊이값이 곧 Z 좌표
    X = (xs - cx) * Z / fx # 핀홀 카메라 역투영으로 X 계산
    Y = (ys - cy) * Z / fy # 핀홀 카메라 역투영으로 Y 계산

    points = np.stack([X, Y, Z], axis=-1).reshape(-1, 3) # (H,W,3) -> (N,3) 점 목록
    colors = image_rgb.reshape(-1, 3) # 각 점에 입힐 색 (원본 RGB)
    return points, colors


def main():
    image_path = "data/indoor.jpg"

    # 1) 깊이 추론
    pipe = pipeline("depth-estimation", model="depth-anything/Depth-Anything-V2-Small-hf")
    result = pipe(image_path)
    depth_map = np.array(result["depth"]).astype(np.float32)

    image_pil = Image.open(image_path).convert("RGB").resize( # 원본을 깊이맵 크기에 맞춰 리사이즈
        (depth_map.shape[1], depth_map.shape[0])
    )
    image_rgb = np.array(image_pil)

    # 2) 카메라 intrinsics 추정 (실측 K 없을 때 대략값)
    #    fx = fy = W (FOV 약 53도 가정). cx, cy 는 이미지 중앙
    H, W = depth_map.shape
    fx = fy = float(W) # 초점거리 (픽셀 단위)
    cx, cy = W / 2.0, H / 2.0 # 주점 (이미지 중앙)

    # 3) point cloud 구성
    points, colors = depth_to_pointcloud(depth_map, image_rgb, fx, fy, cx, cy)
    print(f"point count: {points.shape[0]}")

    # 4) Rerun 로깅
    rr.init("depth_anything", spawn=False) # Rerun 세션 시작

    # 2D: 원본 이미지 + colored depth
    rr.log("image/rgb", rr.Image(image_rgb)) # 원본 이미지 기록
    rr.log("image/depth", rr.DepthImage(depth_map)) # 깊이맵 기록

    # 3D: point cloud + 카메라
    rr.log(
        "world/cam",
        rr.Pinhole(image_from_camera=np.array([[fx, 0, cx], [0, fy, cy], [0, 0, 1]]), # 카메라 내부 파라미터 행렬
                   width=W, height=H),
    )
    rr.log("world/points", rr.Points3D(points, colors=colors, radii=0.005)) # 3D 점 구름 기록

    # 5) RRD 저장 (로컬 viewer 에서 열기)
    os.makedirs("outputs", exist_ok=True) # 결과물 폴더 (수업 자료와 분리)
    rr.save("outputs/depth_pointcloud.rrd") # 결과를 .rrd 파일로 저장
    print("저장: outputs/depth_pointcloud.rrd")
    print("로컬에서 확인: rerun outputs/depth_pointcloud.rrd")


if __name__ == "__main__":
    main()
```


**실행 + 로컬 확인 절차** (ENVIRONMENT.md 의 방법 A):


1. 원격에서 `python visualize_depth_3d.py` -> `outputs/depth_pointcloud.rrd` 생성
2. VSCode 파일트리에서 우클릭 -> Download -> 로컬로 받기
3. 로컬 (맥북) 에서:
   ```bash
   conda activate rerun
   rerun ~/Downloads/depth_pointcloud.rrd
   ```


**확인 포인트**:
- 3D 패널에서 회전/줌. 가까운 물체 vs 먼 벽이 공간적으로 분리되어 보이는가
- 2D 패널의 colored depth 와 3D 의 point cloud 가 같은 장면임을 비교
- `image/rgb` 만 켜면 원본, `image/depth` 만 켜면 depth — 엔티티 토글로 비교 가능


**한 단계 더**:
- Step 4 의 `metric_depth` 변환 결과를 `depth_map` 대신 넣으면 미터 단위 절대 거리로 point cloud 가 펼쳐진다
- relative depth 와 metric depth 의 차이를 3D 에서 시각적으로 비교 가능


---


## Step 4: Metric Depth 변환 실험

상대 깊이를 실제 거리(미터)로 바꾼다. week7에서 배운 단안 추정의 한계(절대 스케일을 모름)를 참조점으로 보정하는 실습이다.

### 왜 metric으로 변환하나

Depth Anything 기본 출력은 상대 깊이라 더 가깝다/멀다만 안다. 로봇이 1.5m 앞에 장애물을 알려면 미터로 변환해야 한다. 기본 발상은 선형 변환 `d_metric = alpha * d_relative + beta`다. 미지수 alpha/beta가 두 개라 참조점 두 개(상대 깊이와 실제 거리를 둘 다 아는 픽셀)가 있으면 풀린다. 한계도 분명하다. 실제 관계가 비선형일 수 있고, 참조점이 부정확하면 전체가 틀어지며, 카메라가 움직이면 보정이 무효가 된다. 이 한계를 아는 것이 변환을 쓰는 것만큼 중요하다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 선형 변환 | `alpha * d_rel + beta` | 상대를 절대로 |
| 참조점 2개 | 연립방정식 풀이 | alpha, beta 계산 |
| 검증 | 알려진 거리와 대조 | 변환 정확도 확인 |

### 핵심 포인트

- alpha가 음수가 될 수 있다. 상대 깊이는 큰 값이 가까움인데 미터는 작은 값이 가까움이라 방향이 반대이기 때문이다(README 명시).
- 참조점 두 개로 맞춘 보정은 그 장면과 카메라에서만 유효하다. 일반 해법이 아니라 그때그때 보정이다.


### metric_depth.py


```python
"""
Relative Depth → Metric Depth 변환 실험
참조점 기반 스케일 보정
"""
import os
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
from transformers import pipeline


def relative_to_metric(depth_map, ref_points):
    """참조점 기반 Metric Depth 변환


    Args:
        depth_map: numpy array [H, W] 상대 깊이맵
        ref_points: list of (y, x, real_depth_m)


    Returns:
        metric_depth: numpy array [H, W] 절대 깊이 (미터)
    """
    if len(ref_points) < 2: # 선형 회귀에는 참조점 2개 이상 필요
        print("경고: 최소 2개의 참조점 필요!")
        return None


    # 참조점에서 상대 깊이값과 실제 깊이 수집
    rel_values = []
    metric_values = []


    for y, x, real_depth in ref_points:
        # 주변 5x5 영역 평균 (노이즈 감소)
        region = depth_map[max(0, y-2):y+3, max(0, x-2):x+3]
        rel_values.append(region.mean()) # 참조점의 상대 깊이값
        metric_values.append(real_depth) # 참조점의 실제 거리(m)


    rel_values = np.array(rel_values)
    metric_values = np.array(metric_values)


    # 선형 회귀: metric = alpha * relative + beta
    # 최소제곱법 (2개 이상의 참조점)
    A = np.column_stack([rel_values, np.ones_like(rel_values)]) # [상대값, 1] 형태의 행렬
    result = np.linalg.lstsq(A, metric_values, rcond=None) # 최소제곱법으로 alpha, beta 추정
    alpha, beta = result[0]


    print(f"Scale (alpha): {alpha:.4f}")
    print(f"Shift (beta): {beta:.4f}")


    # 전체 깊이맵 변환
    metric_depth = alpha * depth_map + beta # 구한 식을 전체 픽셀에 적용


    # 음수 깊이 클리핑
    metric_depth = np.clip(metric_depth, 0.0, None) # 음수 거리는 0으로 잘라냄


    return metric_depth


def analyze_depth_regions(depth_map, image_path):
    """깊이맵 영역별 분석"""
    print("\n" + "=" * 40)
    print("깊이맵 영역별 분석")
    print("=" * 40)


    h, w = depth_map.shape


    # 3등분 (상단/중단/하단)
    top = depth_map[:h//3, :] # 상단 1/3 영역
    mid = depth_map[h//3:2*h//3, :] # 중단 1/3 영역
    bot = depth_map[2*h//3:, :] # 하단 1/3 영역


    print(f"상단 (하늘/천장): 평균={top.mean():.3f}, 범위=[{top.min():.3f}, {top.max():.3f}]")
    print(f"중단 (물체): 평균={mid.mean():.3f}, 범위=[{mid.min():.3f}, {mid.max():.3f}]")
    print(f"하단 (바닥): 평균={bot.mean():.3f}, 범위=[{bot.min():.3f}, {bot.max():.3f}]")


    # 시각화
    fig, axes = plt.subplots(2, 2, figsize=(12, 8))


    image = Image.open(image_path)
    axes[0, 0].imshow(image)
    axes[0, 0].set_title('원본 이미지')


    axes[0, 1].imshow(depth_map, cmap='magma')
    axes[0, 1].set_title('깊이맵')
    axes[0, 1].axhline(h//3, color='cyan', linewidth=1, linestyle='--') # 상/중 경계선
    axes[0, 1].axhline(2*h//3, color='cyan', linewidth=1, linestyle='--') # 중/하 경계선


    # 영역별 히스토그램
    axes[1, 0].hist(top.flatten(), bins=50, alpha=0.5, label='상단', color='skyblue')
    axes[1, 0].hist(mid.flatten(), bins=50, alpha=0.5, label='중단', color='orange')
    axes[1, 0].hist(bot.flatten(), bins=50, alpha=0.5, label='하단', color='green')
    axes[1, 0].legend()
    axes[1, 0].set_title('영역별 깊이 분포')


    # 가로 프로파일 (중앙 라인)
    center_line = depth_map[h//2, :] # 이미지 중앙 가로줄의 깊이값
    axes[1, 1].plot(center_line)
    axes[1, 1].set_title('중앙 수평선 깊이 프로파일')
    axes[1, 1].set_xlabel('X 좌표')
    axes[1, 1].set_ylabel('깊이 값')


    for ax in axes.flatten():
        ax.axis('off') if hasattr(ax, 'images') and ax.images else None


    plt.tight_layout()
    os.makedirs("outputs", exist_ok=True) # 결과물 폴더 (분석 그래프를 수업 자료와 분리)
    plt.savefig("outputs/depth_analysis.png", dpi=150, bbox_inches='tight')
    print(f"저장: outputs/depth_analysis.png")
    plt.close()


if __name__ == "__main__":
    print("=" * 40)
    print("Metric Depth 변환 실험")
    print("=" * 40)


    image_path = "data/indoor.jpg"


    # 깊이 추론
    pipe = pipeline("depth-estimation", model="depth-anything/Depth-Anything-V2-Small-hf")
    result = pipe(image_path)
    depth_map = np.array(result["depth"]).astype(np.float32)


    # 영역 분석
    analyze_depth_regions(depth_map, image_path)


    # Metric 변환 (가상 참조점 예시)
    # 실제 사용 시 이미지에서 실제 거리를 아는 지점을 지정
    print("\n Metric 변환 예시:")
    print("(실제 사용 시 실제 거리를 아는 참조점 2개 이상 필요)")


    ref_points = [ # 참조점 목록: (y좌표, x좌표, 실제 거리 m)
        (240, 320, 2.0), # 이미지 중앙, 실제 2m
        (100, 320, 5.0), # 이미지 상단, 실제 5m
    ]


    metric_depth = relative_to_metric(depth_map, ref_points) # 상대 깊이 -> 절대 깊이 변환
    if metric_depth is not None:
        print(f"Metric 깊이 범위: [{metric_depth.min():.2f}m, {metric_depth.max():.2f}m]")
```


---


## Step 5: YOLO + Depth 결합 (미리보기)

Phase 3의 최종 그림이다. YOLO로 무엇이 어디에 있는지를 찾고(검출), Depth로 얼마나 멀리 있는지를 더해, 검출된 객체마다 거리를 붙인다.

### 왜 결합하나

검출(week1-6)과 깊이(week7-8)를 따로 배웠다. 이 둘을 합치면 1.5m 앞에 사람처럼 장애물 회피에 쓸 수 있는 정보가 된다. 방법은 단순하다. YOLO가 준 박스 영역의 깊이맵을 모아(보통 박스 중심이나 중앙값) 그 객체의 거리로 삼는다. 미리보기인 이유는 정밀한 결합과 센서 융합은 이 과정의 시작점일 뿐이기 때문이다. 여기서는 두 모델의 출력을 한 화면에 합치는 최소 형태를 본다. Phase 3가 그려온 큰 그림(검출 + 깊이 = 공간 인식)을 닫는 단계다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 두 모델 동시 | YOLO + Depth Anything | 한 이미지에 둘 다 추론 |
| 거리 산출 | 박스 영역 깊이 집계 | 객체별 대표 거리 |
| 통합 시각화 | 박스 + 거리 표시 | 무엇이 얼마나 멀리 |

### 핵심 포인트

- 박스 전체 평균보다 중앙값이나 중심 부근이 안정적이다. 박스에 배경 깊이가 섞이면 거리가 왜곡된다.
- 거리의 신뢰도는 깊이가 상대인지 metric인지에 달렸다. 실제 미터가 필요하면 Step 4 변환이 선행돼야 한다.


### yolo_depth_combine.py


```python
"""
YOLO 객체 검출 + Depth Anything 깊이 결합
(미리보기 - Week 11에서 본격 다룸)
"""
import os
import numpy as np
import cv2
from PIL import Image
from transformers import pipeline


def combine_yolo_depth(image_path, yolo_boxes, depth_map):
    """YOLO 바운딩 박스 영역의 깊이 분석


    Args:
        image_path: 이미지 경로
        yolo_boxes: list of [x1, y1, x2, y2, class_name, confidence]
        depth_map: numpy array [H, W]
    """
    print("=" * 40)
    print("YOLO + Depth 결합 분석")
    print("=" * 40)


    image = cv2.imread(image_path)
    h_img, w_img = image.shape[:2] # 원본 이미지 크기
    h_dep, w_dep = depth_map.shape # 깊이맵 크기


    # 깊이맵 크기를 이미지에 맞춤
    if (h_dep, w_dep) != (h_img, w_img): # 크기 다르면 깊이맵을 이미지 크기로 리사이즈
        depth_resized = cv2.resize(depth_map, (w_img, h_img))
    else:
        depth_resized = depth_map


    print(f"\n 검출 결과:")
    for box in yolo_boxes: # 검출된 객체마다 깊이 분석
        x1, y1, x2, y2, cls_name, conf = box


        # 바운딩 박스 내 깊이 추출
        roi_depth = depth_resized[int(y1):int(y2), int(x1):int(x2)] # 박스 영역의 깊이값들


        if roi_depth.size == 0: # 빈 영역이면 건너뜀
            continue


        mean_depth = roi_depth.mean() # 박스 영역 평균 깊이
        min_depth = roi_depth.min()
        max_depth = roi_depth.max()


        # 깊이 기반 거리 판단 (상대 깊이)
        if mean_depth > 200: # 상대 깊이값이 클수록 가까움
            distance_desc = "매우 가까움"
        elif mean_depth > 150:
            distance_desc = "가까움"
        elif mean_depth > 100:
            distance_desc = "중간"
        else:
            distance_desc = "멀리"


        print(f"{cls_name} (conf={conf:.2f}):")
        print(f"bbox: ({x1:.0f},{y1:.0f})-({x2:.0f},{y2:.0f})")
        print(f"깊이: 평균={mean_depth:.1f}, "
              f"범위=[{min_depth:.1f}, {max_depth:.1f}]")
        print(f"거리: {distance_desc}")


        # 시각화
        color = (0, 255, 0) if mean_depth < 150 else (0, 0, 255) # 멀면 초록, 가까우면 빨강
        cv2.rectangle(image, (int(x1), int(y1)), (int(x2), int(y2)), color, 2)
        label = f"{cls_name}: depth={mean_depth:.0f}"
        cv2.putText(image, label, (int(x1), int(y1) - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)


    os.makedirs("outputs", exist_ok=True) # 결과물 폴더 (수업 자료와 분리)
    cv2.imwrite("outputs/yolo_depth_result.jpg", image) # 결과 이미지 저장
    print(f"\n 저장: outputs/yolo_depth_result.jpg")


if __name__ == "__main__":
    # 가상의 YOLO 검출 결과 (실제 사용 시 YOLO 모델로 대체)
    yolo_boxes = [ # 각 항목: [x1, y1, x2, y2, 클래스명, confidence]
        [100, 50, 300, 400, "person", 0.92],
        [400, 200, 550, 350, "chair", 0.85],
        [50, 300, 200, 450, "dog", 0.78],
    ]


    print("(가상의 YOLO 검출 결과로 테스트)")
    print("실제 사용 시 ultralytics YOLO 모델로 대체하세요.")


    # 깊이 추론
    image_path = "data/indoor.jpg"


    depth_pipe = pipeline("depth-estimation",
                          model="depth-anything/Depth-Anything-V2-Small-hf")
    result = depth_pipe(image_path)
    depth_map = np.array(result["depth"]).astype(np.float32) # 깊이맵을 numpy float32로


    # 결합
    combine_yolo_depth(image_path, yolo_boxes, depth_map) # 박스별 깊이 분석
```


---


## 실행 순서


```bash
# 0. 테스트 이미지 준비
mkdir -p data
# 실내/야외 이미지를 data/ 폴더에 저장


# 1. Pipeline 추론
python infer_pipeline.py


# 2. 직접 추론
python infer_manual.py


# 3. 시각화
python visualize_depth.py


# 4. Metric 변환
python metric_depth.py


# 5. YOLO+Depth 결합 (선택)
python yolo_depth_combine.py
```


### 예상 출력


```
========================================
Pipeline API 깊이 추론
========================================
  모델: depth-anything/Depth-Anything-V2-Small-hf
  입력: data/indoor.jpg
  출력 크기: (480, 640)
  값 범위: [0, 255]
  저장: outputs/depth_pipeline_result.png


========================================
깊이맵 시각화
========================================
  저장: outputs/depth_comparison.png
  저장: outputs/depth_colormaps.png
  저장: outputs/depth_histogram.png
  저장: outputs/depth_overlay.png
  모든 시각화 완료!
```


---


## 체크리스트


- [ ] HuggingFace Pipeline으로 깊이 추론 성공
- [ ] 직접 모델 로드 및 추론 성공
- [ ] GPU 추론 테스트 (가능한 경우)
- [ ] 깊이맵 시각화 (magma, turbo 컬러맵)
- [ ] 원본+깊이맵 나란히 시각화
- [ ] 깊이맵 오버레이 생성
- [ ] 깊이 히스토그램 분석
- [ ] Relative → Metric 변환 실험
- [ ] 깊이맵 영역별 분석 (상/중/하)
- [ ] YOLO + Depth 결합 미리보기 (선택)


---


## 트러블슈팅


### 모델 다운로드 느림
```
문제: HuggingFace 모델 다운로드가 매우 느림
해결:
1. 미러 사이트 사용: export HF_ENDPOINT=https://hf-mirror.com
2. 또는 미리 다운로드:
   huggingface-cli download depth-anything/Depth-Anything-V2-Small-hf
3. 프록시 설정 확인
```


### CUDA 메모리 오류
```
문제: RuntimeError: CUDA out of memory
해결:
1. torch.no_grad() 블록 사용 (필수!)
2. ViT-S (Small) 모델 사용
3. 입력 이미지 크기 줄이기
4. 다른 GPU 프로세스 종료: nvidia-smi로 확인
```


### 깊이맵이 이상할 때
```
문제: 깊이맵이 전체적으로 균일하거나 뒤집혀 보임
해결:
1. 전처리 확인: AutoImageProcessor 사용 권장
2. 모델 출력 해석 확인: 큰 값=가까움? 멀리?
3. 입력 이미지 형식 확인: RGB vs BGR
4. 정규화 범위 확인: 0~1 vs 0~255
```


### matplotlib 한글 깨짐
```
문제: 그래프 제목의 한글이 깨짐
해결:
  import matplotlib.pyplot as plt
  plt.rcParams['font.family'] = 'AppleGothic' # macOS
  # 또는
  plt.rcParams['font.family'] = 'NanumGothic' # Linux
  plt.rcParams['axes.unicode_minus'] = False
```


---


**이전**: [Week 7 - Monocular Depth 이론](../week7/PRACTICE.md)
**다음**: [Week 9 - ONNX & TensorRT 변환: Depth 모델](../week9/PRACTICE.md)
