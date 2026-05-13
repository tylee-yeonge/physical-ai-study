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
+-- visualize_depth.py # 깊이맵 시각화
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
    depth_pipe = pipeline(
        task="depth-estimation",
        model=model_name
    )
    print(f"모델: {model_name}")


    # 추론
    result = depth_pipe(image_path)


    # 결과 분석
    depth_image = result["depth"] # PIL Image
    depth_array = np.array(depth_image)


    print(f"입력: {image_path}")
    print(f"출력 크기: {depth_array.shape}")
    print(f"값 범위: [{depth_array.min()}, {depth_array.max()}]")
    print(f"데이터 타입: {depth_array.dtype}")


    # 저장
    depth_image.save("depth_pipeline_result.png")
    print(f"저장: depth_pipeline_result.png")


    return depth_array


def compare_models(image_path):
    """모델 크기별 비교"""
    print("\n" + "=" * 40)
    print("모델 크기별 비교")
    print("=" * 40)


    import time


    models = [
        ("LiheYoung/depth-anything-small-hf", "ViT-S"),
        ("LiheYoung/depth-anything-base-hf", "ViT-B"),
    ]


    for model_name, desc in models:
        pipe = pipeline("depth-estimation", model=model_name)


        # Warm-up
        pipe(image_path)


        # 속도 측정 (5회 평균)
        times = []
        for _ in range(5):
            start = time.time()
            result = pipe(image_path)
            times.append(time.time() - start)


        avg_ms = np.mean(times) * 1000
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
        if device is None:
            self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        else:
            self.device = torch.device(device)


        print(f"장치: {self.device}")


        # 모델 로드
        self.model = AutoModelForDepthEstimation.from_pretrained(model_name)
        self.model.to(self.device)
        self.model.eval()


        # 이미지 프로세서 로드
        self.processor = AutoImageProcessor.from_pretrained(model_name)


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
        if isinstance(image, np.ndarray):
            image_pil = Image.fromarray(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))
        else:
            image_pil = image


        orig_size = image_pil.size # (W, H)


        # 전처리
        inputs = self.processor(images=image_pil, return_tensors="pt")
        inputs = {k: v.to(self.device) for k, v in inputs.items()}


        # 추론
        with torch.no_grad():
            outputs = self.model(**inputs)


        # 깊이맵 추출 및 리사이즈
        depth = outputs.predicted_depth.squeeze() # [H_model, W_model]


        # 원본 크기로 리사이즈
        depth = torch.nn.functional.interpolate(
            depth.unsqueeze(0).unsqueeze(0),
            size=(orig_size[1], orig_size[0]), # (H, W)
            mode="bicubic",
            align_corners=False
        ).squeeze()


        depth_numpy = depth.cpu().numpy()


        return depth_numpy


    def infer_batch(self, images):
        """배치 추론 (여러 이미지 동시)


        Args:
            images: list of PIL Images
        Returns:
            list of depth maps
        """
        inputs = self.processor(images=images, return_tensors="pt")
        inputs = {k: v.to(self.device) for k, v in inputs.items()}


        with torch.no_grad():
            outputs = self.model(**inputs)


        depths = outputs.predicted_depth # [B, H, W]


        results = []
        for i, img in enumerate(images):
            d = depths[i].unsqueeze(0).unsqueeze(0)
            d = torch.nn.functional.interpolate(
                d, size=img.size[::-1], mode="bicubic", align_corners=False
            ).squeeze()
            results.append(d.cpu().numpy())


        return results


    def benchmark(self, image, num_runs=20):
        """추론 속도 벤치마크"""
        # Warm-up
        for _ in range(3):
            self.infer(image)


        # 측정
        times = []
        for _ in range(num_runs):
            start = time.time()
            self.infer(image)
            times.append(time.time() - start)


        avg_ms = np.mean(times) * 1000
        std_ms = np.std(times) * 1000
        fps = 1000 / avg_ms


        print(f"\n 벤치마크 결과 ({num_runs}회):")
        print(f"평균: {avg_ms:.1f} ms (+/- {std_ms:.1f})")
        print(f"FPS: {fps:.1f}")


        return avg_ms


if __name__ == "__main__":
    print("=" * 40)
    print("Depth Anything 직접 추론")
    print("=" * 40)


    # 추론기 초기화
    inferencer = DepthAnythingInference()


    # 이미지 로드
    image = Image.open("data/indoor.jpg")
    print(f"이미지 크기: {image.size}")


    # 추론
    depth_map = inferencer.infer(image)
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
    if d_max - d_min < 1e-6:
        return np.zeros_like(depth_map)
    return (depth_map - d_min) / (d_max - d_min)


def depth_to_colormap(depth_map, colormap_name='magma'):
    """깊이맵에 컬러맵 적용"""
    depth_norm = normalize_depth(depth_map)
    cmap = plt.get_cmap(colormap_name)
    colored = cmap(depth_norm)[:, :, :3] # alpha 제거
    colored = (colored * 255).astype(np.uint8)
    return colored


def visualize_side_by_side(image_path, depth_map, save_path="depth_comparison.png"):
    """원본 이미지와 깊이맵 나란히 시각화"""
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))


    # 원본 이미지
    image = Image.open(image_path)
    axes[0].imshow(image)
    axes[0].set_title('원본 이미지', fontsize=14)
    axes[0].axis('off')


    # 깊이맵
    im = axes[1].imshow(depth_map, cmap='magma')
    axes[1].set_title('깊이맵 (Depth Anything)', fontsize=14)
    axes[1].axis('off')
    plt.colorbar(im, ax=axes[1], fraction=0.046, pad=0.04)


    plt.tight_layout()
    plt.savefig(save_path, dpi=150, bbox_inches='tight')
    print(f"저장: {save_path}")
    plt.close()


def visualize_colormaps(depth_map, save_path="depth_colormaps.png"):
    """다양한 컬러맵 비교"""
    colormaps = ['magma', 'inferno', 'turbo', 'viridis', 'plasma', 'gray']


    fig, axes = plt.subplots(2, 3, figsize=(15, 8))
    axes = axes.flatten()


    for ax, cmap_name in zip(axes, colormaps):
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
    axes[1].hist(depth_map.flatten(), bins=100, color='steelblue', alpha=0.7)
    axes[1].set_xlabel('깊이 값')
    axes[1].set_ylabel('픽셀 수')
    axes[1].set_title('깊이값 분포')
    axes[1].axvline(depth_map.mean(), color='red', linestyle='--',
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
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)


    # 깊이맵 컬러맵 적용
    depth_colored = depth_to_colormap(depth_map, 'turbo')


    # 크기 맞추기
    if depth_colored.shape[:2] != image.shape[:2]:
        depth_colored = cv2.resize(depth_colored,
                                    (image.shape[1], image.shape[0]))


    # 오버레이
    overlay = cv2.addWeighted(image, 1 - alpha, depth_colored, alpha, 0)


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
    depth_map = np.array(result["depth"]).astype(np.float32)
    print(f"깊이맵 크기: {depth_map.shape}")


    # 시각화
    visualize_side_by_side(image_path, depth_map)
    visualize_colormaps(depth_map)
    visualize_depth_histogram(depth_map)
    create_overlay(image_path, depth_map, alpha=0.4)


    print("\n 모든 시각화 완료!")
```


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
    if len(ref_points) < 2:
        print("경고: 최소 2개의 참조점 필요!")
        return None


    # 참조점에서 상대 깊이값과 실제 깊이 수집
    rel_values = []
    metric_values = []


    for y, x, real_depth in ref_points:
        # 주변 5x5 영역 평균 (노이즈 감소)
        region = depth_map[max(0, y-2):y+3, max(0, x-2):x+3]
        rel_values.append(region.mean())
        metric_values.append(real_depth)


    rel_values = np.array(rel_values)
    metric_values = np.array(metric_values)


    # 선형 회귀: metric = alpha * relative + beta
    # 최소제곱법 (2개 이상의 참조점)
    A = np.column_stack([rel_values, np.ones_like(rel_values)])
    result = np.linalg.lstsq(A, metric_values, rcond=None)
    alpha, beta = result[0]


    print(f"Scale (alpha): {alpha:.4f}")
    print(f"Shift (beta): {beta:.4f}")


    # 전체 깊이맵 변환
    metric_depth = alpha * depth_map + beta


    # 음수 깊이 클리핑
    metric_depth = np.clip(metric_depth, 0.0, None)


    return metric_depth


def analyze_depth_regions(depth_map, image_path):
    """깊이맵 영역별 분석"""
    print("\n" + "=" * 40)
    print("깊이맵 영역별 분석")
    print("=" * 40)


    h, w = depth_map.shape


    # 3등분 (상단/중단/하단)
    top = depth_map[:h//3, :]
    mid = depth_map[h//3:2*h//3, :]
    bot = depth_map[2*h//3:, :]


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
    axes[0, 1].axhline(h//3, color='cyan', linewidth=1, linestyle='--')
    axes[0, 1].axhline(2*h//3, color='cyan', linewidth=1, linestyle='--')


    # 영역별 히스토그램
    axes[1, 0].hist(top.flatten(), bins=50, alpha=0.5, label='상단', color='skyblue')
    axes[1, 0].hist(mid.flatten(), bins=50, alpha=0.5, label='중단', color='orange')
    axes[1, 0].hist(bot.flatten(), bins=50, alpha=0.5, label='하단', color='green')
    axes[1, 0].legend()
    axes[1, 0].set_title('영역별 깊이 분포')


    # 가로 프로파일 (중앙 라인)
    center_line = depth_map[h//2, :]
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


    ref_points = [
        (240, 320, 2.0), # 이미지 중앙, 실제 2m
        (100, 320, 5.0), # 이미지 상단, 실제 5m
    ]


    metric_depth = relative_to_metric(depth_map, ref_points)
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
    h_img, w_img = image.shape[:2]
    h_dep, w_dep = depth_map.shape


    # 깊이맵 크기를 이미지에 맞춤
    if (h_dep, w_dep) != (h_img, w_img):
        depth_resized = cv2.resize(depth_map, (w_img, h_img))
    else:
        depth_resized = depth_map


    print(f"\n 검출 결과:")
    for box in yolo_boxes:
        x1, y1, x2, y2, cls_name, conf = box


        # 바운딩 박스 내 깊이 추출
        roi_depth = depth_resized[int(y1):int(y2), int(x1):int(x2)]


        if roi_depth.size == 0:
            continue


        mean_depth = roi_depth.mean()
        min_depth = roi_depth.min()
        max_depth = roi_depth.max()


        # 깊이 기반 거리 판단 (상대 깊이)
        if mean_depth > 200:
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
        color = (0, 255, 0) if mean_depth < 150 else (0, 0, 255)
        cv2.rectangle(image, (int(x1), int(y1)), (int(x2), int(y2)), color, 2)
        label = f"{cls_name}: depth={mean_depth:.0f}"
        cv2.putText(image, label, (int(x1), int(y1) - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)


    cv2.imwrite("yolo_depth_result.jpg", image)
    print(f"\n 저장: yolo_depth_result.jpg")


if __name__ == "__main__":
    # 가상의 YOLO 검출 결과 (실제 사용 시 YOLO 모델로 대체)
    yolo_boxes = [
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
    depth_map = np.array(result["depth"]).astype(np.float32)


    # 결합
    combine_yolo_depth(image_path, yolo_boxes, depth_map)
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
