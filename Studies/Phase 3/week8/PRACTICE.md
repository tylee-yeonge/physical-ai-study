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
# 가상환경 활성화
conda activate phase5


# 패키지 설치
pip install torch torchvision
pip install transformers
pip install timm
pip install numpy opencv-python matplotlib
pip install Pillow


# 설치 확인
python -c "
from transformers import pipeline
pipe = pipeline('depth-estimation', model='LiheYoung/depth-anything-small-hf')
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


### infer_pipeline.py


```python
"""
HuggingFace Pipeline으로 Depth Anything 추론
가장 간단한 방법!
"""
from transformers import pipeline
from PIL import Image
import numpy as np


def infer_with_pipeline(image_path, model_name="LiheYoung/depth-anything-small-hf"):
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
    depth_image.save("depth_pipeline_result.png") # 깊이맵 이미지 파일로 저장
    print(f"저장: depth_pipeline_result.png")


    return depth_array


def compare_models(image_path):
    """모델 크기별 비교"""
    print("\n" + "=" * 40)
    print("모델 크기별 비교")
    print("=" * 40)


    import time


    models = [ # 비교할 모델 2종 (경로, 표시 이름)
        ("LiheYoung/depth-anything-small-hf", "ViT-S"),
        ("LiheYoung/depth-anything-base-hf", "ViT-B"),
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
    def __init__(self, model_name="LiheYoung/depth-anything-small-hf", device=None):
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


    # 벤치마크
    inferencer.benchmark(image)
```


---


## Step 3: 깊이맵 시각화


### visualize_depth.py


```python
"""
깊이맵 시각화: 다양한 컬러맵 및 표현 방법
"""
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


def visualize_side_by_side(image_path, depth_map, save_path="depth_comparison.png"):
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


def visualize_colormaps(depth_map, save_path="depth_colormaps.png"):
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


def visualize_depth_histogram(depth_map, save_path="depth_histogram.png"):
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


def create_overlay(image_path, depth_map, alpha=0.5, save_path="depth_overlay.png"):
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
    pipe = pipeline("depth-estimation", model="LiheYoung/depth-anything-small-hf")
    result = pipe(image_path)
    depth_map = np.array(result["depth"]).astype(np.float32) # 깊이맵을 numpy float32로 변환
    print(f"깊이맵 크기: {depth_map.shape}")


    # 시각화
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


### visualize_depth_3d.py


```python
"""
Step 3-B (옵션): depth map -> 3D point cloud + Rerun 시각화

depth 의 각 픽셀을 카메라 intrinsics 로 unproject 해 3D 점으로 만들고,
원본 이미지 색을 그대로 입혀 색칠된 point cloud 로 띄운다.

PNG 의 colored depth 가 보여주지 못하는 "공간감" 을 회전/줌으로 직접 확인.
"""
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
    pipe = pipeline("depth-estimation", model="LiheYoung/depth-anything-small-hf")
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
    rr.save("depth_pointcloud.rrd") # 결과를 .rrd 파일로 저장
    print("저장: depth_pointcloud.rrd")
    print("로컬에서 확인: rerun depth_pointcloud.rrd")


if __name__ == "__main__":
    main()
```


**실행 + 로컬 확인 절차** (ENVIRONMENT.md 의 방법 A):


1. 원격에서 `python visualize_depth_3d.py` -> `depth_pointcloud.rrd` 생성
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


### metric_depth.py


```python
"""
Relative Depth → Metric Depth 변환 실험
참조점 기반 스케일 보정
"""
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
    plt.savefig("depth_analysis.png", dpi=150, bbox_inches='tight')
    print(f"저장: depth_analysis.png")
    plt.close()


if __name__ == "__main__":
    print("=" * 40)
    print("Metric Depth 변환 실험")
    print("=" * 40)


    image_path = "data/indoor.jpg"


    # 깊이 추론
    pipe = pipeline("depth-estimation", model="LiheYoung/depth-anything-small-hf")
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


### yolo_depth_combine.py


```python
"""
YOLO 객체 검출 + Depth Anything 깊이 결합
(미리보기 - Week 11에서 본격 다룸)
"""
import numpy as np
import cv2
from PIL import Image


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


    cv2.imwrite("yolo_depth_result.jpg", image) # 결과 이미지 저장
    print(f"\n 저장: yolo_depth_result.jpg")


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
    from transformers import pipeline
    image_path = "data/indoor.jpg"


    depth_pipe = pipeline("depth-estimation",
                          model="LiheYoung/depth-anything-small-hf")
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
  모델: LiheYoung/depth-anything-small-hf
  입력: data/indoor.jpg
  출력 크기: (480, 640)
  값 범위: [0, 255]
  저장: depth_pipeline_result.png


========================================
깊이맵 시각화
========================================
  저장: depth_comparison.png
  저장: depth_colormaps.png
  저장: depth_histogram.png
  저장: depth_overlay.png
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
   huggingface-cli download LiheYoung/depth-anything-small-hf
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
