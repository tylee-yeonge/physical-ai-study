# Week 12 실습: 최종 데모 - 최적화 & 포트폴리오 (Python)

> 🎯 **목표**: 전체 파이프라인 최적화, 성능 목표 달성, 포트폴리오 정리
> 💻 **언어**: Python (PyTorch, OpenCV, NumPy)
> ⏰ **예상 시간**: 12시간

---

## 📋 실습 개요

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | 전체 파이프라인 벤치마크 | 필수 | 2시간 |
| 2 | 병목 분석 및 최적화 | 필수 | 3시간 |
| 3 | 최종 성능 측정 | 필수 | 2시간 |
| 4 | 포트폴리오 정리 | 필수 | 3시간 |
| 5 | Phase 5 복습 | 필수 | 2시간 |

---

## 🔧 환경 설정

```bash
# 가상환경 활성화
conda activate phase5

# 패키지 설치
pip install -r requirements.txt
```

---

## Step 1: 전체 파이프라인 벤치마크

### 1.1 단계별 시간 측정

```python
# benchmark_pipeline.py
"""전체 파이프라인 벤치마크: 각 단계별 시간 측정"""
import time
import numpy as np
import cv2

class PipelineBenchmark:
    """파이프라인 각 단계의 시간을 측정하는 벤치마크 도구"""

    def __init__(self):
        self.timings = {}

    def time_stage(self, stage_name, func, *args, **kwargs):
        """한 단계의 실행 시간 측정"""
        start = time.perf_counter()
        result = func(*args, **kwargs)
        elapsed_ms = (time.perf_counter() - start) * 1000

        if stage_name not in self.timings:
            self.timings[stage_name] = []
        self.timings[stage_name].append(elapsed_ms)

        return result

    def report(self):
        """벤치마크 결과 리포트 출력"""
        print("\n" + "=" * 60)
        print("  파이프라인 벤치마크 결과")
        print("=" * 60)

        total_avg = 0
        for stage, times in self.timings.items():
            avg = np.mean(times)
            std = np.std(times)
            total_avg += avg
            bar = "█" * int(avg / 2)
            print(f"  {stage:>20}: {avg:7.2f} ms (±{std:.2f}) {bar}")

        fps = 1000.0 / total_avg if total_avg > 0 else 0
        print(f"  {'─' * 55}")
        print(f"  {'합계':>20}: {total_avg:7.2f} ms")
        print(f"  {'FPS':>20}: {fps:7.1f}")
        print()

        # 목표 달성 여부
        if fps >= 10:
            print(f"  목표 달성! (>= 10 FPS)")
        else:
            print(f"  목표 미달 (< 10 FPS). 최적화 필요.")

        print("=" * 60)

        return {
            'total_ms': total_avg,
            'fps': fps,
            'stages': {k: np.mean(v) for k, v in self.timings.items()}
        }


def simulate_pipeline_benchmark(num_frames=50):
    """파이프라인 시뮬레이션 벤치마크

    실제 모델 대신 시뮬레이션 데이터를 사용합니다.
    실제 환경에서는 YOLO, Depth 모델을 로드하여 사용하세요.
    """
    benchmark = PipelineBenchmark()

    print(f"벤치마크 시작: {num_frames}프레임")
    print(f"{'─' * 40}")

    for i in range(num_frames):
        # 이미지 캡처 시뮬레이션
        def capture_image():
            return np.random.randint(0, 255, (480, 640, 3), dtype=np.uint8)
        image = benchmark.time_stage("이미지 캡처", capture_image)

        # YOLO 전처리
        def yolo_preprocess(img):
            return cv2.resize(img, (640, 640))
        yolo_input = benchmark.time_stage("YOLO 전처리", yolo_preprocess, image)

        # YOLO 추론 시뮬레이션
        def yolo_inference(inp):
            time.sleep(0.028)  # ~28ms
            return [{'bbox': (100, 100, 300, 300), 'class': 'car', 'conf': 0.9}]
        detections = benchmark.time_stage("YOLO 추론", yolo_inference, yolo_input)

        # Depth 전처리
        def depth_preprocess(img):
            return cv2.resize(img, (512, 384))
        depth_input = benchmark.time_stage("Depth 전처리", depth_preprocess, image)

        # Depth 추론 시뮬레이션
        def depth_inference(inp):
            time.sleep(0.050)  # ~50ms
            return np.random.uniform(1, 50, (384, 512))
        depth_map = benchmark.time_stage("Depth 추론", depth_inference, depth_input)

        # 융합 + 역투영
        def fusion(dets, depth):
            results = []
            for det in dets:
                x1, y1, x2, y2 = det['bbox']
                cx, cy = (x1+x2)//2, (y1+y2)//2
                z = depth[min(cy, depth.shape[0]-1), min(cx, depth.shape[1]-1)]
                results.append({'class': det['class'], '3d': (0, 0, z)})
            return results
        results_3d = benchmark.time_stage("융합+역투영", fusion, detections, depth_map)

        if (i + 1) % 10 == 0:
            print(f"  {i+1}/{num_frames} 완료")

    # 결과 출력
    return benchmark.report()


if __name__ == "__main__":
    simulate_pipeline_benchmark(50)
```

### 1.2 체크포인트

```
 각 단계별 시간 측정 완료
 병목 단계 식별
 현재 FPS 확인
```

---

## Step 2: 병목 분석 및 최적화

### 2.1 최적화 적용 코드

```python
# optimization_strategies.py
"""최적화 전략 적용 및 비교"""
import numpy as np
import time
from concurrent.futures import ThreadPoolExecutor

def sequential_pipeline(yolo_fn, depth_fn, fusion_fn, image):
    """순차 실행 파이프라인"""
    start = time.perf_counter()

    detections = yolo_fn(image)
    depth_map = depth_fn(image)
    results = fusion_fn(detections, depth_map)

    elapsed = (time.perf_counter() - start) * 1000
    return results, elapsed

def parallel_pipeline(yolo_fn, depth_fn, fusion_fn, image):
    """병렬 실행 파이프라인 (YOLO + Depth 동시)"""
    start = time.perf_counter()

    with ThreadPoolExecutor(max_workers=2) as executor:
        yolo_future = executor.submit(yolo_fn, image)
        depth_future = executor.submit(depth_fn, image)

        detections = yolo_future.result()
        depth_map = depth_future.result()

    results = fusion_fn(detections, depth_map)

    elapsed = (time.perf_counter() - start) * 1000
    return results, elapsed

def compare_optimization(num_trials=20):
    """순차 vs 병렬 비교"""
    print("=" * 50)
    print("  순차 vs 병렬 파이프라인 비교")
    print("=" * 50)

    # 시뮬레이션 함수
    def yolo_sim(img):
        time.sleep(0.030)
        return [{'bbox': (100, 100, 300, 300), 'class': 'car'}]

    def depth_sim(img):
        time.sleep(0.055)
        return np.random.uniform(1, 50, (384, 512))

    def fusion_sim(dets, depth):
        return [{'class': d['class'], 'depth': 10.0} for d in dets]

    image = np.random.randint(0, 255, (480, 640, 3), dtype=np.uint8)

    # 순차 실행
    seq_times = []
    for _ in range(num_trials):
        _, t = sequential_pipeline(yolo_sim, depth_sim, fusion_sim, image)
        seq_times.append(t)

    # 병렬 실행
    par_times = []
    for _ in range(num_trials):
        _, t = parallel_pipeline(yolo_sim, depth_sim, fusion_sim, image)
        par_times.append(t)

    seq_avg = np.mean(seq_times)
    par_avg = np.mean(par_times)
    speedup = seq_avg / par_avg

    print(f"\n  순차 실행: {seq_avg:.1f} ms ({1000/seq_avg:.1f} FPS)")
    print(f"  병렬 실행: {par_avg:.1f} ms ({1000/par_avg:.1f} FPS)")
    print(f"  속도 향상: {speedup:.2f}x")
    print()

    if 1000/par_avg >= 10:
        print(f"  병렬 실행으로 10 FPS 목표 달성!")
    else:
        print(f"  추가 최적화 필요 (모델 경량화, 해상도 축소 등)")

if __name__ == "__main__":
    compare_optimization()
```

### 2.2 입력 해상도 최적화

```python
# resolution_optimization.py
"""입력 해상도별 속도-정확도 트레이드오프 분석"""
import numpy as np
import time

def analyze_resolution_tradeoff():
    """다양한 입력 해상도에서의 속도 시뮬레이션"""
    print("=" * 55)
    print("  입력 해상도별 속도-정확도 트레이드오프")
    print("=" * 55)

    # (해상도, 예상 추론 시간, 예상 AbsRel)
    configs = [
        ("256x320",   256, 320, 35,  0.18),
        ("288x384",   288, 384, 42,  0.14),
        ("384x512",   384, 512, 55,  0.09),
        ("480x640",   480, 640, 80,  0.07),
        ("518x518",   518, 518, 95,  0.06),
    ]

    print(f"\n  {'해상도':>10} | {'추론시간':>8} | {'FPS':>5} | {'AbsRel':>8} | 적합성")
    print(f"  {'─' * 55}")

    for name, h, w, ms, absrel in configs:
        fps = 1000.0 / ms
        if fps >= 15 and absrel < 0.15:
            suit = "최적"
        elif fps >= 10:
            suit = "양호"
        else:
            suit = "느림"

        print(f"  {name:>10} | {ms:>6}ms | {fps:>5.1f} | {absrel:>8.2f} | {suit}")

    print(f"\n  권장: 384x512 (속도와 정확도의 균형)")

if __name__ == "__main__":
    analyze_resolution_tradeoff()
```

### 2.3 체크포인트

```
 병렬 실행 구현 및 비교
 해상도별 트레이드오프 분석
 최적 설정 결정
```

---

## Step 3: 최종 성능 측정

### 3.1 최종 결과 정리

```python
# final_performance.py
"""최종 성능 측정 및 결과 정리"""
import numpy as np

def generate_final_report():
    """Phase 5 최종 성능 리포트 생성"""
    print("=" * 60)
    print("  Phase 5 최종 성능 리포트")
    print("=" * 60)

    # 성능 결과 (실제 측정값으로 교체)
    results = {
        '전체 FPS': {'value': 15.4, 'target': 10.0, 'unit': 'FPS'},
        'YOLO 추론': {'value': 30.0, 'target': 40.0, 'unit': 'ms'},
        'Depth 추론': {'value': 55.0, 'target': 70.0, 'unit': 'ms'},
        '융합+역투영': {'value': 2.0, 'target': 5.0, 'unit': 'ms'},
        'GPU 메모리': {'value': 3.8, 'target': 6.0, 'unit': 'GB'},
        'Depth AbsRel': {'value': 0.09, 'target': 0.15, 'unit': ''},
        'YOLO mAP@50': {'value': 0.52, 'target': 0.45, 'unit': ''},
    }

    print(f"\n  {'항목':>15} | {'결과':>8} | {'목표':>8} | 달성")
    print(f"  {'─' * 50}")

    all_passed = True
    for name, data in results.items():
        val = data['value']
        target = data['target']
        unit = data['unit']

        # 달성 여부 판단
        if name in ['YOLO 추론', 'Depth 추론', '융합+역투영', 'GPU 메모리', 'Depth AbsRel']:
            passed = val <= target  # 낮을수록 좋음
        else:
            passed = val >= target  # 높을수록 좋음

        status = "PASS" if passed else "FAIL"
        if not passed:
            all_passed = False

        print(f"  {name:>15} | {val:>6.2f}{unit:>2} | {target:>6.2f}{unit:>2} | {status}")

    print(f"  {'─' * 50}")
    if all_passed:
        print(f"  전체 결과: 모든 목표 달성!")
    else:
        print(f"  전체 결과: 일부 목표 미달성. 추가 최적화 필요.")

    print("=" * 60)

    return results

def generate_phase5_summary():
    """Phase 5 전체 학습 내용 요약"""
    print("\n" + "=" * 60)
    print("  Phase 5 학습 요약")
    print("=" * 60)

    topics = [
        ("Week 1", "PyTorch 기초", "Tensor, autograd, DataLoader, CNN"),
        ("Week 2", "CV 라이브러리", "Albumentations, W&B, timm"),
        ("Week 3", "YOLO 이론", "Backbone, Neck, Head, Anchor-free"),
        ("Week 4", "YOLO 학습", "커스텀 데이터셋, Augmentation, mAP"),
        ("Week 5", "YOLO ONNX", "torch.onnx.export, ONNX Runtime"),
        ("Week 6", "YOLO TensorRT", "FP16, C++ 추론, Jetson 배포"),
        ("Week 7", "Depth 이론", "Monocular Depth, ViT 인코더"),
        ("Week 8", "Depth Anything", "HuggingFace, 추론, 시각화"),
        ("Week 9", "Depth TensorRT", "ONNX 변환, FP16, C++ 추론"),
        ("Week 10", "Depth 검증", "AbsRel, RMSE, delta, 취약점"),
        ("Week 11", "Detection+Depth", "3D 역투영, ROS2 통합"),
        ("Week 12", "최종 데모", "최적화, 벤치마크, 포트폴리오"),
    ]

    for week, title, details in topics:
        print(f"  {week:>7} | {title:<15} | {details}")

    print("=" * 60)

if __name__ == "__main__":
    generate_final_report()
    generate_phase5_summary()
```

### 3.2 체크포인트

```
 최종 성능 수치 기록
 목표 대비 달성 여부 확인
 Phase 5 학습 요약 완료
```

---

## Step 4: 포트폴리오 정리

### 4.1 포트폴리오 체크리스트

```python
# portfolio_checklist.py
"""포트폴리오 준비 체크리스트"""

def portfolio_checklist():
    """포트폴리오에 포함해야 할 항목 체크리스트"""
    print("=" * 50)
    print("  포트폴리오 체크리스트")
    print("=" * 50)

    items = {
        "프로젝트 개요": [
            "프로젝트 제목 및 한 줄 설명",
            "문제 정의 (왜 만들었는가?)",
            "기술 스택 목록",
        ],
        "시스템 설계": [
            "아키텍처 다이어그램 (PNG/SVG)",
            "데이터 흐름도",
            "ROS2 토픽 구조도",
        ],
        "핵심 코드": [
            "YOLO TensorRT 추론 코드",
            "Depth TensorRT 추론 코드",
            "Detection + Depth 융합 코드",
            "3D 역투영 코드",
        ],
        "성능 결과": [
            "FPS 벤치마크 결과",
            "Depth 정확도 (AbsRel, RMSE)",
            "YOLO mAP 결과",
            "GPU 메모리 사용량",
        ],
        "데모": [
            "실행 GIF 또는 동영상",
            "스크린샷 (2D 검출 + Depth + 3D)",
            "Bird's Eye View 시각화",
        ],
        "문서": [
            "README.md (프로젝트 설명)",
            "설치/실행 방법",
            "개선 방향 (Future Work)",
        ],
    }

    for category, checklist in items.items():
        print(f"\n  [{category}]")
        for item in checklist:
            print(f"    [ ] {item}")

    print("\n" + "=" * 50)

if __name__ == "__main__":
    portfolio_checklist()
```

### 4.2 성능 결과 시각화

```python
# performance_visualization.py
"""성능 결과를 차트로 시각화"""
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def create_performance_charts():
    """성능 결과 차트 생성"""

    fig, axes = plt.subplots(1, 3, figsize=(18, 5))

    # 1. 단계별 시간 분포 (Pie Chart)
    stages = ['YOLO 추론', 'Depth 추론', '전처리', '후처리', '융합']
    times = [30, 55, 6, 4, 2]
    colors = ['#ff9999', '#ff4444', '#66b3ff', '#99ff99', '#ffcc99']

    axes[0].pie(times, labels=stages, colors=colors, autopct='%1.0f%%',
                startangle=90)
    axes[0].set_title('단계별 시간 분포')

    # 2. 최적화 전후 비교 (Bar Chart)
    categories = ['순차\n(최적화 전)', '병렬\n(최적화 후)']
    fps_values = [9.5, 15.4]
    colors_bar = ['#ff6666', '#66cc66']

    bars = axes[1].bar(categories, fps_values, color=colors_bar, width=0.5)
    axes[1].axhline(y=10, color='red', linestyle='--', label='목표 (10 FPS)')
    axes[1].set_ylabel('FPS')
    axes[1].set_title('최적화 전후 FPS 비교')
    axes[1].legend()

    for bar, val in zip(bars, fps_values):
        axes[1].text(bar.get_x() + bar.get_width()/2., bar.get_height() + 0.3,
                     f'{val:.1f}', ha='center', va='bottom', fontweight='bold')

    # 3. 거리별 3D 위치 정확도 (Line Chart)
    distances = [5, 10, 15, 20, 30, 40, 50]
    errors = [0.3, 0.5, 0.8, 1.2, 2.0, 3.5, 5.0]

    axes[2].plot(distances, errors, 'bo-', linewidth=2, markersize=8)
    axes[2].fill_between(distances, errors, alpha=0.2)
    axes[2].set_xlabel('GT 거리 (m)')
    axes[2].set_ylabel('3D 위치 오차 (m)')
    axes[2].set_title('거리별 3D 위치 추정 오차')
    axes[2].grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig('performance_charts.png', dpi=150, bbox_inches='tight')
    print("성능 차트 저장: performance_charts.png")

if __name__ == "__main__":
    create_performance_charts()
```

### 4.3 체크포인트

```
 포트폴리오 체크리스트 확인
 성능 차트 생성
 README.md 초안 작성
```

---

## Step 5: Phase 5 복습

### 5.1 핵심 수식 복습

```python
# review_formulas.py
"""Phase 5 핵심 수식 복습"""
import numpy as np

def review_all_formulas():
    """Phase 5에서 배운 모든 핵심 수식 복습"""
    print("=" * 55)
    print("  Phase 5 핵심 수식 복습")
    print("=" * 55)

    # 1. 역투영
    print("\n  [1] 역투영 (Unprojection)")
    fx, fy, cx, cy = 500.0, 500.0, 320.0, 240.0
    u, v, Z = 400.0, 200.0, 10.0
    X = (u - cx) * Z / fx
    Y = (v - cy) * Z / fy
    print(f"    X = (u-cx)*Z/fx = ({u}-{cx})*{Z}/{fx} = {X:.2f}m")
    print(f"    Y = (v-cy)*Z/fy = ({v}-{cy})*{Z}/{fy} = {Y:.2f}m")
    print(f"    Z = {Z:.2f}m")

    # 2. 스테레오 깊이
    print("\n  [2] 스테레오 깊이")
    f, b, d = 721.5, 0.54, 50.0
    Z_stereo = f * b / d
    print(f"    Z = f*b/d = {f}*{b}/{d} = {Z_stereo:.2f}m")

    # 3. AbsRel
    print("\n  [3] AbsRel")
    gt = np.array([10.0, 20.0, 30.0])
    pred = np.array([9.5, 22.0, 28.0])
    absrel = np.mean(np.abs(pred - gt) / gt)
    print(f"    GT:   {gt}")
    print(f"    Pred: {pred}")
    print(f"    AbsRel = mean(|pred-gt|/gt) = {absrel:.4f}")

    # 4. delta < 1.25
    print("\n  [4] delta < 1.25")
    ratio = np.maximum(pred / gt, gt / pred)
    delta1 = np.mean(ratio < 1.25)
    print(f"    ratio = max(pred/gt, gt/pred) = {ratio}")
    print(f"    delta<1.25 = {delta1:.4f}")

    # 5. FPS
    print("\n  [5] FPS 계산")
    avg_ms = 65.0
    fps = 1000.0 / avg_ms
    print(f"    평균 추론 시간: {avg_ms}ms")
    print(f"    FPS = 1000 / {avg_ms} = {fps:.1f}")

    print("\n" + "=" * 55)

if __name__ == "__main__":
    review_all_formulas()
```

---

## ✅ 실습 체크리스트

### Step 1: 벤치마크
- [ ] 전체 파이프라인 시간 측정 완료
- [ ] 각 단계별 시간 비율 파악
- [ ] 병목 단계 식별

### Step 2: 최적화
- [ ] 병렬 실행 적용
- [ ] 해상도 트레이드오프 분석
- [ ] 최적 설정 결정

### Step 3: 최종 성능
- [ ] 최종 FPS 측정
- [ ] 목표 달성 여부 확인
- [ ] 결과 기록

### Step 4: 포트폴리오
- [ ] 포트폴리오 체크리스트 확인
- [ ] 성능 차트 생성
- [ ] README.md 작성

### Step 5: 복습
- [ ] 핵심 수식 복습
- [ ] Phase 5 전체 내용 정리
- [ ] Phase 6 준비 사항 확인

---

## 💡 트러블슈팅

### 병렬 실행이 빨라지지 않을 때

```
문제: ThreadPoolExecutor 사용 시 속도 향상 없음
원인: Python GIL(Global Interpreter Lock)
해결:
  1. CUDA 스트림 분리 (C++ 레벨)
  2. multiprocessing 사용 (프로세스 분리)
  3. asyncio + 비동기 CUDA 호출
```

### GPU 메모리 부족 시

```
문제: 두 모델 동시 로드 시 OOM
해결:
  1. FP16 사용 (메모리 ~40% 절약)
  2. 작은 모델 선택 (YOLOv8n, Depth-S)
  3. torch.cuda.empty_cache() 호출
  4. batch_size=1 유지
```

---

## 🔗 참고 자료

- [TensorRT 최적화 가이드](https://developer.nvidia.com/tensorrt)
- [Jetson 성능 튜닝](https://developer.nvidia.com/embedded/jetson-orin-nano)
- [CUDA 스트림 프로그래밍](https://developer.nvidia.com/blog/gpu-pro-tip-cuda-7-streams-simplify-concurrency/)

---

**다음**: [Phase 6 - LiDAR 3D Perception](../../../Roadmap/Phase%206.md)
