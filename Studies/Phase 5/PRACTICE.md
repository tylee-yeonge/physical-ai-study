# Phase 5: Detection + Depth 실습 (Python + Jetson 배포)

> 🎯 **목표**: YOLO 객체 검출 + Depth Estimation을 Jetson에서 TensorRT로 배포  
> 💻 **언어**: **Python** (학습) + **TensorRT/C++** (배포)  
> 🛠️ **하드웨어**: **Jetson Orin Nano** 필수  
> ⏰ **예상 시간**: ~90시간 (3개월)
> 📌 각 주차별 상세 실습은 `week{N}/PRACTICE.md`를 참고하세요. 이 문서는 Phase 전체 개요입니다.

---

## 📋 실습 개요

Phase 5은 **2D Perception의 핵심**으로, 딥러닝 모델을 학습하고 **Jetson에서 실시간 추론**하는 전 과정을 경험합니다.

### 언어 전략

| 단계 | 언어 | 이유 |
|------|------|------|
| **모델 학습** | Python (PyTorch) | 딥러닝 생태계 표준 |
| **모델 변환** | Python → ONNX | 중간 포맷 |
| **TensorRT 변환** | Python/C++ | 최적화 |
| **배포 (Inference)** | C++ (TensorRT) | Jetson 최고 성능 |

**핵심**: Python으로 빠르게 학습 → Jetson에서 C++/TensorRT로 배포

---

## 🔧 실습 1: YOLO 객체 검출 (4주)

### Week 1-2: YOLOv8 학습 (Python)

**환경 세팅**:
```bash
# PC (학습용)
conda create -n phase5 python=3.10
conda activate phase5

pip install torch torchvision
pip install ultralytics
```

**커스텀 데이터셋 학습**:

```python
# train_yolo.py
from ultralytics import YOLO

# 사전학습 모델 로드
model = YOLO('yolov8n.pt')  # nano (가볍고 빠름)

# 커스텀 데이터셋 학습
results = model.train(
    data='coco128.yaml',    # 또는 custom.yaml
    epochs=100,
    imgsz=640,
    batch=16,
    device=0,               # GPU
    project='yolo_runs',
    name='exp_v8n',
)

# 모델 저장
model.save('yolov8n_custom.pt')
```

**커스텀 데이터셋 포맷** (`custom.yaml`):
```yaml
path: /path/to/dataset
train: images/train
val: images/val

nc: 3  # number of classes
names: ['person', 'car', 'box']
```

### Week 3: ONNX 변환 및 검증

```python
# export_onnx.py
from ultralytics import YOLO

model = YOLO('yolov8n_custom.pt')

# ONNX 변환
model.export(format='onnx', 
            imgsz=640,
            opset=12,
            simplify=True)

# onnx 파일 생성: yolov8n_custom.onnx
```

**ONNX 검증**:
```python
import onnxruntime as ort
import numpy as np
import cv2

# ONNX Runtime 추론
session = ort.InferenceSession('yolov8n_custom.onnx')

img = cv2.imread('test.jpg')
img_resized = cv2.resize(img, (640, 640))
img_normalized = img_resized.astype(np.float32) / 255.0
img_transposed = np.transpose(img_normalized, (2, 0, 1))
img_batch = np.expand_dims(img_transposed, axis=0)

# 추론
outputs = session.run(None, {session.get_inputs()[0].name: img_batch})
print(f"Output shape: {outputs[0].shape}")
```

### Week 4: Jetson에서 TensorRT 배포

**Jetson에서**:
```bash
# 1. ONNX → TensorRT 변환
/usr/src/tensorrt/bin/trtexec \
    --onnx=yolov8n_custom.onnx \
    --saveEngine=yolov8n_custom.trt \
    --fp16  # FP16 정밀도 (속도 2배, 정확도 손실 미미)

# 2. Benchmark
/usr/src/tensorrt/bin/trtexec \
    --loadEngine=yolov8n_custom.trt \
    --batch=1
```

**C++ TensorRT Inference** (간략 버전):
```cpp
// yolo_trt_inference.cpp
#include <NvInfer.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <vector>

class YOLOTRTInference {
public:
    YOLOTRTInference(const std::string& engine_file) {
        // TensorRT 엔진 로드
        std::ifstream file(engine_file, std::ios::binary);
        // ... (TensorRT 초기화 코드 생략)
    }
    
    void infer(const cv::Mat& image, std::vector<Detection>& detections) {
        // 전처리
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(640, 640));
        
        // TensorRT 추론
        // ... (생략)
    }
};

int main() {
    YOLOTRTInference yolo("yolov8n_custom.trt");
    
    cv::VideoCapture cap(0);  // Jetson 카메라
    
    cv::Mat frame;
    while (true) {
        cap >> frame;
        
        std::vector<Detection> detections;
        yolo.infer(frame, detections);
        
        // 시각화
        for (const auto& det : detections) {
            cv::rectangle(frame, det.bbox, cv::Scalar(0, 255, 0), 2);
        }
        
        cv::imshow("YOLO TRT", frame);
        if (cv::waitKey(1) == 27) break;
    }
    
    return 0;
}
```

**목표 성능**:
- Jetson Orin Nano: **30+ FPS** @ 640×480
- 정확도: mAP **> 0.5**

---

## 🔧 실습 2: Monocular Depth Estimation (4주)

### Week 1-2: Depth Anything 학습 (Python)

```bash
git clone https://github.com/LiheYoung/Depth-Anything
cd Depth-Anything
pip install -r requirements.txt
```

**Fine-tuning** (선택):
```python
# fine_tune_depth.py
import torch
from depth_anything.models import DepthAnything

model = DepthAnything.from_pretrained('LiheYoung/depth_anything_vitb14')

# 추론
depth = model(image)  # [H, W]
```

### Week 3: ONNX & TensorRT 배포

**ONNX 변환**:
```python
import torch
from depth_anything.models import DepthAnything

model = DepthAnything.from_pretrained('LiheYoung/depth_anything_vitb14')
model.eval()

dummy_input = torch.randn(1, 3, 384, 512)

torch.onnx.export(
    model,
    dummy_input,
    "depth_anything.onnx",
    opset_version=12,
    input_names=['input'],
    output_names=['depth']
)
```

**Jetson TensorRT 변환**:
```bash
trtexec --onnx=depth_anything.onnx \
        --saveEngine=depth_anything.trt \
        --fp16
```

### Week 4: 실시간 Depth 맵 생성

**목표**: 스테레오 카메라 **없이** 단안 카메라로 Depth 추정

```python
# depth_realtime.py (Jetson)
import cv2
import numpy as np
import tensorrt as trt
import pycuda.driver as cuda

class DepthTRT:
    def __init__(self, engine_file):
        # TensorRT 초기화
        pass
    
    def infer(self, image):
        # Depth 추정
        return depth_map

cap = cv2.VideoCapture(0)

depth_model = DepthTRT('depth_anything.trt')

while True:
    ret, frame = cap.read()
    
    depth = depth_model.infer(frame)
    
    # 시각화
    depth_colored = cv2.applyColorMap(
        cv2.convertScaleAbs(depth, alpha=255/depth.max()), 
        cv2.COLORMAP_TURBO
    )
    
    cv2.imshow('RGB', frame)
    cv2.imshow('Depth', depth_colored)
    
    if cv2.waitKey(1) == 27:
        break
```

---

## 🔧 실습 3: 통합 시스템 (2주)

### YOLO + Depth 융합

**목표**: 검출된 객체의 3D 위치 추정

```python
# yolo_depth_fusion.py
class YOLODepthFusion:
    def __init__(self, yolo_trt, depth_trt, camera_K):
        self.yolo = yolo_trt
        self.depth = depth_trt
        self.K = camera_K  # 내부 파라미터 (Phase 2 캘리브레이션 결과)
    
    def process(self, image):
        # 1. YOLO 검출
        detections = self.yolo.infer(image)
        
        # 2. Depth 추정
        depth_map = self.depth.infer(image)
        
        # 3. 3D 위치 계산
        objects_3d = []
        
        for det in detections:
            x, y, w, h = det.bbox
            cx, cy = int(x + w/2), int(y + h/2)
            
            # Depth 값
            depth_value = depth_map[cy, cx]
            
            # 2D → 3D (역투영)
            fx, fy = self.K[0, 0], self.K[1, 1]
            cx_k, cy_k = self.K[0, 2], self.K[1, 2]
            
            X = (cx - cx_k) / fx * depth_value
            Y = (cy - cy_k) / fy * depth_value
            Z = depth_value
            
            objects_3d.append({
                'class': det.class_name,
                'position': [X, Y, Z],
                'bbox_2d': det.bbox
            })
        
        return objects_3d
```

**산출물**:
- Jetson에서 실시간 객체 검출 + 3D 위치 추정
- **30fps 유지**
- GUI 시각화 (OpenCV 또는 ROS RViz)

---

## ✅ 체크리스트

### YOLO
- [ ] YOLOv8 커스텀 데이터 학습
- [ ] ONNX 변환 및 검증
- [ ] Jetson TensorRT 배포
- [ ] 30+ FPS 달성

### Depth Estimation
- [ ] Depth Anything 추론
- [ ] ONNX & TensorRT 변환
- [ ] 실시간 Depth 맵 생성

### 통합
- [ ] YOLO + Depth 융합
- [ ] 3D 위치 추정 정확도 검증
- [ ] 데모 영상 제작

---

## 📚 참고 자료

- [Ultralytics YOLOv8](https://docs.ultralytics.com/)
- [Depth-Anything GitHub](https://github.com/LiheYoung/Depth-Anything)
- [TensorRT Developer Guide](https://docs.nvidia.com/deeplearning/tensorrt/developer-guide/)
- [Jetson Orin Optimization](https://developer.nvidia.com/embedded/jetson-orin)

---

**다음 단계**: Phase 6 - 3D Perception (nuScenes, BEV)
