# Week 5 실습: ONNX 변환 및 Runtime 추론


> **목표**: PyTorch 모델을 ONNX로 변환하고 Runtime으로 추론하기
> **언어**: Python (PyTorch, ONNX, ONNX Runtime)
> **예상 시간**: 12시간


---


## 실습 개요


Week 5는 학습된 PyTorch 모델을 **ONNX 포맷으로 변환**하고, **ONNX Runtime으로 추론**한 후 속도를 비교합니다. 양자화(FP16/INT8)까지 실험합니다.


---


## 환경 설정


```bash
# 가상환경 활성화
conda activate phase5


# 패키지 설치
pip install torch torchvision
pip install onnx onnxruntime-gpu
pip install ultralytics
pip install numpy opencv-python matplotlib


# 설치 확인
python -c "import onnx; print(f'ONNX: {onnx.__version__}')"
python -c "import onnxruntime as ort; print(f'ORT: {ort.__version__}')"
python -c "import torch; print(f'PyTorch: {torch.__version__}')"
```


---


## 프로젝트 구조


```
week5_onnx/
+-- export_onnx.py # ONNX 변환
+-- validate_onnx.py # ONNX 검증
+-- infer_onnx.py # ONNX Runtime 추론
+-- benchmark.py # 속도 비교
+-- quantize_model.py # 양자화
+-- utils.py # 전처리/후처리 유틸리티
```


---


## Step 1: PyTorch → ONNX 변환


### export_onnx.py


```python
"""
PyTorch YOLO11 → ONNX 변환
"""
import torch
from ultralytics import YOLO


def export_static():
    """Static shape 변환 (Jetson 배포용)"""
    model = YOLO('yolo11n.pt')


    # ONNX 변환
    model.export(
        format='onnx',
        imgsz=640,
        opset=12,
        simplify=True,
        half=False, # FP32
    )
    print("Static ONNX 변환 완료: yolo11n.onnx")


def export_dynamic():
    """Dynamic shape 변환 (서버 배포용)"""
    model = YOLO('yolo11n.pt')


    model.export(
        format='onnx',
        imgsz=640,
        opset=12,
        simplify=True,
        dynamic=True, # 동적 배치
    )
    print("Dynamic ONNX 변환 완료")


def export_manual():
    """수동 ONNX 변환 (커스텀 모델용)"""
    import torchvision.models as models


    # 예시: ResNet18
    model = models.resnet18(pretrained=True)
    model.eval()


    # 더미 입력
    dummy_input = torch.randn(1, 3, 224, 224)


    # ONNX 변환
    torch.onnx.export(
        model,
        dummy_input,
        "resnet18.onnx",
        opset_version=12,
        input_names=['input'],
        output_names=['output'],
        dynamic_axes={
            'input': {0: 'batch_size'},
            'output': {0: 'batch_size'}
        }
    )
    print("ResNet18 ONNX 변환 완료: resnet18.onnx")


if __name__ == "__main__":
    export_static()
    export_dynamic()
    export_manual()
```


---


## Step 2: ONNX 모델 검증


### validate_onnx.py


```python
"""
ONNX 모델 검증: 구조 확인 + 출력 비교
"""
import onnx
import numpy as np
import torch
from ultralytics import YOLO


def check_model(onnx_path):
    """ONNX 모델 구조 검증"""
    model = onnx.load(onnx_path)


    # 기본 검증
    onnx.checker.check_model(model)
    print(f"모델 검증 통과: {onnx_path}")


    # 모델 정보 출력
    print(f"\n모델 정보:")
    print(f"Opset version: {model.opset_import[0].version}")
    print(f"IR version: {model.ir_version}")
    print(f"Producer: {model.producer_name}")


    # 입력 정보
    print(f"\n[in] 입력:")
    for inp in model.graph.input:
        shape = [d.dim_value for d in inp.type.tensor_type.shape.dim]
        print(f"{inp.name}: {shape}")


    # 출력 정보
    print(f"\n[out] 출력:")
    for out in model.graph.output:
        shape = [d.dim_value for d in out.type.tensor_type.shape.dim]
        print(f"{out.name}: {shape}")


    # 노드 수
    print(f"\n 노드 수: {len(model.graph.node)}")


def compare_outputs(pt_path, onnx_path):
    """PyTorch vs ONNX 출력 비교"""
    import onnxruntime as ort


    # 더미 입력
    dummy = np.random.randn(1, 3, 640, 640).astype(np.float32)


    # ONNX Runtime 추론
    session = ort.InferenceSession(onnx_path)
    input_name = session.get_inputs()[0].name
    onnx_out = session.run(None, {input_name: dummy})


    print(f"ONNX 출력 shape: {onnx_out[0].shape}")
    print(f"출력 범위: [{onnx_out[0].min():.4f}, {onnx_out[0].max():.4f}]")


if __name__ == "__main__":
    check_model("yolo11n.onnx")
    compare_outputs("yolo11n.pt", "yolo11n.onnx")
```


---


## Step 3: ONNX Runtime 추론


### infer_onnx.py


```python
"""
ONNX Runtime으로 YOLO11 추론
"""
import onnxruntime as ort
import numpy as np
import cv2
import time


class ONNXDetector:
    def __init__(self, model_path, conf_thresh=0.25, iou_thresh=0.45):
        # 세션 생성 (GPU 우선)
        providers = ['CUDAExecutionProvider', 'CPUExecutionProvider']
        self.session = ort.InferenceSession(model_path, providers=providers)


        self.input_name = self.session.get_inputs()[0].name
        self.input_shape = self.session.get_inputs()[0].shape
        self.conf_thresh = conf_thresh
        self.iou_thresh = iou_thresh


        # 사용 중인 Provider 확인
        print(f"Provider: {self.session.get_providers()}")
        print(f"입력 shape: {self.input_shape}")


    def preprocess(self, image):
        """이미지 전처리"""
        h, w = self.input_shape[2], self.input_shape[3]


        # Resize
        resized = cv2.resize(image, (w, h))


        # BGR → RGB
        rgb = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)


        # Normalize [0, 1]
        normalized = rgb.astype(np.float32) / 255.0


        # HWC → CHW → NCHW
        transposed = np.transpose(normalized, (2, 0, 1))
        batched = np.expand_dims(transposed, axis=0)


        return batched


    def postprocess(self, outputs, orig_shape):
        """후처리: NMS + bbox 변환"""
        predictions = outputs[0] # [1, 84, 8400] for YOLO11
        predictions = np.transpose(predictions, (0, 2, 1)) # [1, 8400, 84]


        boxes = []
        scores = []
        class_ids = []


        for pred in predictions[0]:
            # pred: [x, y, w, h, cls0, cls1, ..., cls79]
            x, y, w, h = pred[:4]
            class_scores = pred[4:]


            max_score = np.max(class_scores)
            if max_score > self.conf_thresh:
                class_id = np.argmax(class_scores)


                # xywh → xyxy
                x1 = x - w / 2
                y1 = y - h / 2
                x2 = x + w / 2
                y2 = y + h / 2


                boxes.append([x1, y1, x2, y2])
                scores.append(float(max_score))
                class_ids.append(int(class_id))


        if len(boxes) == 0:
            return [], [], []


        # NMS
        indices = cv2.dnn.NMSBoxes(
            [[b[0], b[1], b[2]-b[0], b[3]-b[1]] for b in boxes],
            scores, self.conf_thresh, self.iou_thresh
        )


        if len(indices) > 0:
            indices = indices.flatten()
            boxes = [boxes[i] for i in indices]
            scores = [scores[i] for i in indices]
            class_ids = [class_ids[i] for i in indices]


        # 원본 크기로 스케일링
        h_orig, w_orig = orig_shape[:2]
        h_input, w_input = self.input_shape[2], self.input_shape[3]


        scale_x = w_orig / w_input
        scale_y = h_orig / h_input


        scaled_boxes = []
        for b in boxes:
            scaled_boxes.append([
                b[0] * scale_x, b[1] * scale_y,
                b[2] * scale_x, b[3] * scale_y
            ])


        return scaled_boxes, scores, class_ids


    def detect(self, image):
        """이미지에서 객체 검출"""
        input_tensor = self.preprocess(image)
        outputs = self.session.run(None, {self.input_name: input_tensor})
        boxes, scores, class_ids = self.postprocess(outputs, image.shape)
        return boxes, scores, class_ids


def draw_detections(image, boxes, scores, class_ids):
    """검출 결과 시각화"""
    COCO_NAMES = ['person', 'bicycle', 'car', 'motorcycle', 'airplane',
                  'bus', 'train', 'truck', 'boat', 'traffic light', ...]


    for box, score, cls_id in zip(boxes, scores, class_ids):
        x1, y1, x2, y2 = map(int, box)


        # 바운딩 박스
        cv2.rectangle(image, (x1, y1), (x2, y2), (0, 255, 0), 2)


        # 라벨
        label = f"{cls_id}: {score:.2f}"
        cv2.putText(image, label, (x1, y1-10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)


    return image


if __name__ == "__main__":
    # 검출기 초기화
    detector = ONNXDetector("yolo11n.onnx")


    # 이미지 추론
    image = cv2.imread("test.jpg")
    boxes, scores, class_ids = detector.detect(image)


    print(f"\n검출 결과: {len(boxes)}개 객체")
    for box, score, cls_id in zip(boxes, scores, class_ids):
        print(f"클래스 {cls_id}: score={score:.3f}, "
              f"bbox=({box[0]:.0f},{box[1]:.0f},{box[2]:.0f},{box[3]:.0f})")


    # 시각화
    result = draw_detections(image.copy(), boxes, scores, class_ids)
    cv2.imwrite("result_onnx.jpg", result)
    print("결과 저장: result_onnx.jpg")
```


---


## Step 4: 속도 벤치마크


### benchmark.py


```python
"""
PyTorch vs ONNX Runtime 속도 비교
"""
import time
import numpy as np
import torch
import onnxruntime as ort
from ultralytics import YOLO


def benchmark_pytorch(model_path, num_runs=100):
    """PyTorch 추론 속도 측정"""
    model = YOLO(model_path)
    dummy = np.random.randint(0, 255, (640, 640, 3), dtype=np.uint8)


    # Warm-up (5회)
    for _ in range(5):
        model(dummy, verbose=False)


    # 벤치마크
    times = []
    for _ in range(num_runs):
        start = time.time()
        model(dummy, verbose=False)
        times.append(time.time() - start)


    avg_ms = np.mean(times) * 1000
    fps = 1000 / avg_ms
    print(f"PyTorch: {avg_ms:.1f} ms / {fps:.1f} FPS")
    return avg_ms


def benchmark_onnx(model_path, provider='CUDAExecutionProvider', num_runs=100):
    """ONNX Runtime 추론 속도 측정"""
    session = ort.InferenceSession(model_path, providers=[provider])
    input_name = session.get_inputs()[0].name
    shape = session.get_inputs()[0].shape


    dummy = np.random.randn(*shape).astype(np.float32)


    # Warm-up (5회)
    for _ in range(5):
        session.run(None, {input_name: dummy})


    # 벤치마크
    times = []
    for _ in range(num_runs):
        start = time.time()
        session.run(None, {input_name: dummy})
        times.append(time.time() - start)


    avg_ms = np.mean(times) * 1000
    fps = 1000 / avg_ms
    print(f"ONNX ({provider[:4]}): {avg_ms:.1f} ms / {fps:.1f} FPS")
    return avg_ms


if __name__ == "__main__":
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("속도 벤치마크: PyTorch vs ONNX")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")


    pt_time = benchmark_pytorch("yolo11n.pt")
    onnx_cpu = benchmark_onnx("yolo11n.onnx", "CPUExecutionProvider")
    onnx_gpu = benchmark_onnx("yolo11n.onnx", "CUDAExecutionProvider")


    print(f"\n 결과 요약:")
    print(f"ONNX GPU vs PyTorch: {pt_time/onnx_gpu:.1f}x 빠름")
    print(f"ONNX CPU vs PyTorch: {pt_time/onnx_cpu:.1f}x")
```


---


## Step 5: 양자화


### quantize_model.py


```python
"""
ONNX 모델 양자화: FP32 → FP16, INT8
"""
import onnx
from onnxruntime.quantization import quantize_dynamic, QuantType
import numpy as np


def convert_fp16(input_path, output_path):
    """FP32 → FP16 변환"""
    from onnxconverter_common import float16


    model = onnx.load(input_path)
    model_fp16 = float16.convert_float_to_float16(model)
    onnx.save(model_fp16, output_path)


    # 크기 비교
    import os
    size_fp32 = os.path.getsize(input_path) / 1024 / 1024
    size_fp16 = os.path.getsize(output_path) / 1024 / 1024
    print(f"FP16 변환 완료")
    print(f"FP32: {size_fp32:.1f} MB")
    print(f"FP16: {size_fp16:.1f} MB ({size_fp16/size_fp32*100:.0f}%)")


def quantize_int8_dynamic(input_path, output_path):
    """Dynamic INT8 양자화 (캘리브레이션 불필요)"""
    quantize_dynamic(
        input_path,
        output_path,
        weight_type=QuantType.QInt8
    )


    import os
    size_orig = os.path.getsize(input_path) / 1024 / 1024
    size_int8 = os.path.getsize(output_path) / 1024 / 1024
    print(f"INT8 Dynamic 양자화 완료")
    print(f"원본: {size_orig:.1f} MB")
    print(f"INT8: {size_int8:.1f} MB ({size_int8/size_orig*100:.0f}%)")


if __name__ == "__main__":
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("ONNX 양자화")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")


    # FP16
    convert_fp16("yolo11n.onnx", "yolo11n_fp16.onnx")


    print()


    # INT8
    quantize_int8_dynamic("yolo11n.onnx", "yolo11n_int8.onnx")
```


---


## 실행 순서


```bash
# 1. ONNX 변환
python export_onnx.py


# 2. 모델 검증
python validate_onnx.py


# 3. 추론 테스트
python infer_onnx.py


# 4. 속도 벤치마크
python benchmark.py


# 5. 양자화
python quantize_model.py
```


### 예상 출력


```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  속도 벤치마크: PyTorch vs ONNX
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


  PyTorch: 18.2 ms / 54.9 FPS
  ONNX (CPU): 45.1 ms / 22.2 FPS
  ONNX (CUDA): 11.8 ms / 84.7 FPS


결과 요약:
  ONNX GPU vs PyTorch: 1.5x 빠름
  ONNX CPU vs PyTorch: 0.4x


━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
ONNX 양자화
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


- FP16 변환 완료
  FP32: 12.2 MB
  FP16: 6.1 MB (50%)


- INT8 Dynamic 양자화 완료
  원본: 12.2 MB
  INT8: 3.2 MB (26%)
```


---


## 체크리스트


- [ ] PyTorch → ONNX 변환 성공
- [ ] onnx.checker 검증 통과
- [ ] ONNX Runtime CPU 추론 성공
- [ ] ONNX Runtime GPU 추론 성공
- [ ] PyTorch vs ONNX 속도 비교 완료
- [ ] FP16 양자화 성공
- [ ] INT8 Dynamic 양자화 성공
- [ ] Netron에서 그래프 시각화 확인


---


## 트러블슈팅


### CUDA Provider 오류
```
문제: CUDAExecutionProvider not available
해결: pip install onnxruntime-gpu (CPU 버전 제거 후)
     또는 CUDA 버전 호환성 확인
```


### ONNX 변환 실패
```
문제: torch.onnx.export fails
해결:
1. opset_version 올리기 (12 → 13)
2. simplify=True 추가
3. 커스텀 레이어 제거 또는 대체
```


### INT8 정확도 하락
```
문제: INT8 양자화 후 정확도 크게 감소
해결:
1. Static 양자화 + 캘리브레이션 사용
2. 민감한 레이어 FP32 유지
3. 캘리브레이션 데이터 늘리기
```


---


**다음**: Week 6에서 ONNX → TensorRT 변환 및 C++ 추론!
