# Week 9 실습: ONNX & TensorRT 변환 - Depth 모델 (C++)

> 🎯 **목표**: Depth Anything 모델을 TensorRT로 변환하고 Jetson에서 실시간 추론 달성
> 💻 **언어**: C++ (TensorRT) + Python (변환용)
> ⏰ **예상 시간**: 12시간

---

## 📋 실습 개요

| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | Depth Anything → ONNX 변환 (PC/Python) | 필수 | 2시간 |
| 2 | ONNX → TensorRT FP16 변환 (Jetson) | 필수 | 3시간 |
| 3 | C++ TensorRT 추론 코드 작성 | 필수 | 4시간 |
| 4 | FPS 측정 및 메모리 프로파일링 | 필수 | 3시간 |

---

## 🔧 환경 설정

### PC (ONNX 변환용)

```bash
# Python 환경 설정
pip install torch torchvision transformers onnx onnxruntime
```

### Jetson Orin Nano (TensorRT 추론용)

```bash
# JetPack SDK 확인 (5.x 이상)
cat /etc/nv_tegra_release

# TensorRT 확인
dpkg -l | grep TensorRT

# OpenCV 확인
pkg-config --modversion opencv4

# jtop 설치 (모니터링)
sudo pip3 install jetson-stats
```

---

## Step 1: Depth Anything → ONNX 변환 (PC)

### 1.1 목표

HuggingFace의 Depth Anything Small 모델을 384x512 고정 입력으로 ONNX 변환합니다.

### 1.2 변환 코드

```python
# export_depth_to_onnx.py (PC에서 실행)
import torch
from transformers import AutoModelForDepthEstimation, AutoImageProcessor
import onnx
import numpy as np

# 1. 모델 로드
print("모델 로드 중...")
model = AutoModelForDepthEstimation.from_pretrained(
    "LiheYoung/depth-anything-small-hf"
)
model.eval()

# 2. 모델 파라미터 수 확인
total_params = sum(p.numel() for p in model.parameters())
print(f"총 파라미터: {total_params:,} ({total_params/1e6:.1f}M)")

# 3. 고정 입력 크기 설정
H, W = 384, 512
dummy_input = torch.randn(1, 3, H, W)

# 4. ONNX 변환
print(f"ONNX 변환 중... (입력: 1x3x{H}x{W})")
torch.onnx.export(
    model,
    dummy_input,
    "depth_anything_small_384x512.onnx",
    opset_version=17,
    input_names=["pixel_values"],
    output_names=["predicted_depth"],
    dynamic_axes=None  # 고정 크기
)

# 5. ONNX 검증
print("ONNX 모델 검증 중...")
onnx_model = onnx.load("depth_anything_small_384x512.onnx")
onnx.checker.check_model(onnx_model)
print("ONNX 모델 검증 완료!")

# 6. 파일 크기 확인
import os
file_size = os.path.getsize("depth_anything_small_384x512.onnx")
print(f"ONNX 파일 크기: {file_size / (1024*1024):.1f} MB")
```

### 1.3 ONNX Runtime 검증

```python
# verify_onnx.py (PC에서 실행)
import onnxruntime as ort
import numpy as np
import cv2
import time

# 1. ONNX 세션 생성
session = ort.InferenceSession("depth_anything_small_384x512.onnx")

# 2. 입출력 정보 확인
for inp in session.get_inputs():
    print(f"입력: {inp.name}, shape={inp.shape}, dtype={inp.type}")
for out in session.get_outputs():
    print(f"출력: {out.name}, shape={out.shape}, dtype={out.type}")

# 3. 더미 입력으로 추론
dummy = np.random.randn(1, 3, 384, 512).astype(np.float32)

# Warm-up
for _ in range(5):
    session.run(None, {"pixel_values": dummy})

# 속도 측정
times = []
for _ in range(50):
    start = time.time()
    outputs = session.run(None, {"pixel_values": dummy})
    times.append(time.time() - start)

print(f"\nONNX Runtime 추론:")
print(f"  평균: {np.mean(times)*1000:.1f} ms")
print(f"  출력 shape: {outputs[0].shape}")
print(f"  출력 범위: [{outputs[0].min():.3f}, {outputs[0].max():.3f}]")
```

### 1.4 체크포인트

```
✅ ONNX 파일 생성 확인
✅ ONNX 검증 통과
✅ ONNX Runtime 추론 성공
✅ 출력 shape 확인 (1, 384, 512) 또는 (1, 1, 384, 512)
```

---

## Step 2: ONNX → TensorRT FP16 변환 (Jetson)

### 2.1 ONNX 파일 전송

```bash
# PC → Jetson 파일 전송
scp depth_anything_small_384x512.onnx jetson@<JETSON_IP>:~/models/
```

### 2.2 TensorRT 변환

```bash
# Jetson에서 실행

# FP16 변환 (권장)
/usr/src/tensorrt/bin/trtexec \
    --onnx=/home/jetson/models/depth_anything_small_384x512.onnx \
    --saveEngine=/home/jetson/models/depth_anything_small_fp16.trt \
    --fp16 \
    --workspace=4096 \
    --verbose 2>&1 | tee trt_conversion_log.txt

# 변환 완료 후 벤치마크
/usr/src/tensorrt/bin/trtexec \
    --loadEngine=/home/jetson/models/depth_anything_small_fp16.trt \
    --batch=1 \
    --warmUp=500 \
    --duration=10

# FP32 변환 (비교용)
/usr/src/tensorrt/bin/trtexec \
    --onnx=/home/jetson/models/depth_anything_small_384x512.onnx \
    --saveEngine=/home/jetson/models/depth_anything_small_fp32.trt \
    --workspace=4096
```

### 2.3 변환 결과 확인

```bash
# 파일 크기 비교
ls -lh /home/jetson/models/depth_anything_small_*.trt

# 예상 출력:
# depth_anything_small_fp16.trt  ~50MB
# depth_anything_small_fp32.trt  ~95MB
```

### 2.4 체크포인트

```
✅ TRT FP16 엔진 파일 생성
✅ trtexec 벤치마크에서 15+ FPS 확인
✅ FP32 vs FP16 속도 비교 완료
```

---

## Step 3: C++ TensorRT 추론 코드 작성

### 3.1 프로젝트 구조

```
depth_trt_inference/
├── CMakeLists.txt
├── include/
│   └── depth_trt_engine.h
├── src/
│   ├── depth_trt_engine.cpp
│   └── main.cpp
└── models/
    └── depth_anything_small_fp16.trt
```

### 3.2 헤더 파일: depth_trt_engine.h

```cpp
// include/depth_trt_engine.h
#pragma once

#include <NvInfer.h>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

// TensorRT Logger
class TRTLogger : public nvinfer1::ILogger {
public:
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kWARNING) {
            std::cout << "[TRT] " << msg << std::endl;
        }
    }
};

class DepthTRTEngine {
public:
    DepthTRTEngine(const std::string& engine_file);
    ~DepthTRTEngine();

    // 핵심 추론 함수
    cv::Mat infer(const cv::Mat& image);

    // 성능 측정
    float getLastInferenceTime() const { return last_inference_ms_; }
    void printMemoryUsage() const;

private:
    // 전처리
    void preprocess(const cv::Mat& image, float* input_buffer);

    // 후처리
    cv::Mat postprocess(float* output_buffer);

    TRTLogger logger_;
    std::unique_ptr<nvinfer1::IRuntime> runtime_;
    std::unique_ptr<nvinfer1::ICudaEngine> engine_;
    std::unique_ptr<nvinfer1::IExecutionContext> context_;

    void* buffers_[2];       // GPU 버퍼 [input, output]
    cudaStream_t stream_;

    int input_h_ = 384;
    int input_w_ = 512;
    int input_size_;         // 1 * 3 * H * W
    int output_size_;        // 1 * 1 * H * W

    float last_inference_ms_ = 0.0f;

    // 정규화 파라미터 (ImageNet)
    const float mean_[3] = {0.485f, 0.456f, 0.406f};
    const float std_[3]  = {0.229f, 0.224f, 0.225f};
};
```

### 3.3 구현 파일: depth_trt_engine.cpp

```cpp
// src/depth_trt_engine.cpp
#include "depth_trt_engine.h"
#include <chrono>
#include <cassert>

DepthTRTEngine::DepthTRTEngine(const std::string& engine_file) {
    // 1. 엔진 파일 로드
    std::ifstream file(engine_file, std::ios::binary);
    assert(file.good() && "엔진 파일을 찾을 수 없습니다!");

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> engine_data(size);
    file.read(engine_data.data(), size);
    file.close();

    printf("엔진 파일 로드 완료: %.1f MB\n", size / (1024.0f * 1024.0f));

    // 2. TensorRT Runtime 생성
    runtime_.reset(nvinfer1::createInferRuntime(logger_));
    engine_.reset(runtime_->deserializeCudaEngine(engine_data.data(), size));
    context_.reset(engine_->createExecutionContext());

    // 3. 입출력 크기 계산
    input_size_ = 1 * 3 * input_h_ * input_w_;
    output_size_ = 1 * 1 * input_h_ * input_w_;

    // 4. GPU 메모리 할당
    cudaMalloc(&buffers_[0], input_size_ * sizeof(float));
    cudaMalloc(&buffers_[1], output_size_ * sizeof(float));

    // 5. CUDA 스트림 생성
    cudaStreamCreate(&stream_);

    printf("TensorRT 엔진 초기화 완료\n");
    printf("  입력: (1, 3, %d, %d) = %d floats\n", input_h_, input_w_, input_size_);
    printf("  출력: (1, 1, %d, %d) = %d floats\n", input_h_, input_w_, output_size_);

    printMemoryUsage();
}

DepthTRTEngine::~DepthTRTEngine() {
    cudaFree(buffers_[0]);
    cudaFree(buffers_[1]);
    cudaStreamDestroy(stream_);
}

void DepthTRTEngine::preprocess(const cv::Mat& image, float* input_buffer) {
    // 1. Resize
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(input_w_, input_h_));

    // 2. BGR → RGB
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);

    // 3. float 변환 + 정규화
    rgb.convertTo(rgb, CV_32FC3, 1.0f / 255.0f);

    // 4. NHWC → NCHW + ImageNet 정규화
    std::vector<cv::Mat> channels(3);
    cv::split(rgb, channels);

    for (int c = 0; c < 3; c++) {
        channels[c] = (channels[c] - mean_[c]) / std_[c];

        // NCHW 순서로 복사
        memcpy(input_buffer + c * input_h_ * input_w_,
               channels[c].data,
               input_h_ * input_w_ * sizeof(float));
    }
}

cv::Mat DepthTRTEngine::postprocess(float* output_buffer) {
    // 출력을 cv::Mat으로 변환
    cv::Mat depth(input_h_, input_w_, CV_32FC1, output_buffer);

    // 0~1 정규화
    double min_val, max_val;
    cv::minMaxLoc(depth, &min_val, &max_val);

    cv::Mat normalized;
    depth.convertTo(normalized, CV_32FC1, 1.0 / (max_val - min_val), -min_val / (max_val - min_val));

    return normalized.clone();
}

cv::Mat DepthTRTEngine::infer(const cv::Mat& image) {
    auto start = std::chrono::high_resolution_clock::now();

    // 1. 전처리 (CPU)
    std::vector<float> input_host(input_size_);
    preprocess(image, input_host.data());

    // 2. Host → Device
    cudaMemcpyAsync(buffers_[0], input_host.data(),
                    input_size_ * sizeof(float),
                    cudaMemcpyHostToDevice, stream_);

    // 3. TensorRT 추론
    context_->enqueueV2(buffers_, stream_, nullptr);

    // 4. Device → Host
    std::vector<float> output_host(output_size_);
    cudaMemcpyAsync(output_host.data(), buffers_[1],
                    output_size_ * sizeof(float),
                    cudaMemcpyDeviceToHost, stream_);

    cudaStreamSynchronize(stream_);

    // 5. 후처리
    cv::Mat depth = postprocess(output_host.data());

    auto end = std::chrono::high_resolution_clock::now();
    last_inference_ms_ = std::chrono::duration<float, std::milli>(end - start).count();

    return depth;
}

void DepthTRTEngine::printMemoryUsage() const {
    size_t free_mem, total_mem;
    cudaMemGetInfo(&free_mem, &total_mem);

    printf("GPU 메모리: %.2f GB / %.2f GB (사용: %.2f GB)\n",
           free_mem / (1024.0f * 1024.0f * 1024.0f),
           total_mem / (1024.0f * 1024.0f * 1024.0f),
           (total_mem - free_mem) / (1024.0f * 1024.0f * 1024.0f));
}
```

### 3.4 메인 파일: main.cpp

```cpp
// src/main.cpp
#include "depth_trt_engine.h"
#include <numeric>

void runBenchmark(DepthTRTEngine& engine, int num_frames = 100) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("벤치마크: %d 프레임\n", num_frames);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    cv::Mat dummy(384, 512, CV_8UC3, cv::Scalar(128, 128, 128));
    std::vector<float> times;

    // Warm-up
    printf("Warm-up (10 프레임)...\n");
    for (int i = 0; i < 10; i++) {
        engine.infer(dummy);
    }

    // 측정
    printf("측정 중...\n");
    for (int i = 0; i < num_frames; i++) {
        engine.infer(dummy);
        times.push_back(engine.getLastInferenceTime());
    }

    // 통계
    float avg = std::accumulate(times.begin(), times.end(), 0.0f) / times.size();
    float min_t = *std::min_element(times.begin(), times.end());
    float max_t = *std::max_element(times.begin(), times.end());
    float fps = 1000.0f / avg;

    printf("\n결과:\n");
    printf("  평균: %.2f ms\n", avg);
    printf("  최소: %.2f ms\n", min_t);
    printf("  최대: %.2f ms\n", max_t);
    printf("  FPS:  %.1f\n\n", fps);

    // 목표 달성 여부
    if (fps >= 15.0f) {
        printf("✅ 목표 달성! (15 FPS 이상)\n");
    } else {
        printf("❌ 목표 미달. 최적화 필요.\n");
    }

    engine.printMemoryUsage();
}

void runRealtimeDemo(DepthTRTEngine& engine) {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        printf("카메라를 열 수 없습니다.\n");
        return;
    }

    printf("\n실시간 Depth 추정 시작 (ESC로 종료)\n");

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        // Depth 추론
        cv::Mat depth = engine.infer(frame);

        // 시각화
        cv::Mat depth_vis;
        depth.convertTo(depth_vis, CV_8UC1, 255.0);
        cv::applyColorMap(depth_vis, depth_vis, cv::COLORMAP_TURBO);

        // FPS 표시
        float fps = 1000.0f / engine.getLastInferenceTime();
        std::string fps_text = cv::format("FPS: %.1f", fps);
        cv::putText(depth_vis, fps_text, cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

        // 원본 + Depth 나란히 표시
        cv::Mat resized_frame;
        cv::resize(frame, resized_frame, depth_vis.size());

        cv::Mat combined;
        cv::hconcat(resized_frame, depth_vis, combined);

        cv::imshow("RGB | Depth", combined);
        if (cv::waitKey(1) == 27) break;  // ESC
    }
}

int main(int argc, char** argv) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Week 9: Depth TensorRT Inference\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    std::string engine_path = "models/depth_anything_small_fp16.trt";
    if (argc > 1) {
        engine_path = argv[1];
    }

    // 엔진 로드
    DepthTRTEngine engine(engine_path);

    // 벤치마크
    runBenchmark(engine, 100);

    // 실시간 데모 (카메라 연결 시)
    runRealtimeDemo(engine);

    return 0;
}
```

### 3.5 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(DepthTRTInference CUDA CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

# OpenCV
find_package(OpenCV 4 REQUIRED)

# CUDA
find_package(CUDA REQUIRED)

# TensorRT (Jetson에서 자동 감지)
find_library(NVINFER nvinfer)
find_library(NVONNXPARSER nvonnxparser)

include_directories(
    include
    ${OpenCV_INCLUDE_DIRS}
    ${CUDA_INCLUDE_DIRS}
    /usr/include/aarch64-linux-gnu  # Jetson TRT 헤더
)

add_executable(depth_inference
    src/depth_trt_engine.cpp
    src/main.cpp
)

target_link_libraries(depth_inference
    ${OpenCV_LIBS}
    ${CUDA_LIBRARIES}
    ${NVINFER}
    ${NVONNXPARSER}
)
```

---

## Step 4: FPS 측정 및 메모리 프로파일링

### 4.1 빌드 및 실행

```bash
cd depth_trt_inference
mkdir build && cd build
cmake ..
make -j$(nproc)

# 벤치마크 실행
./depth_inference ../models/depth_anything_small_fp16.trt
```

### 4.2 jtop으로 실시간 모니터링

```bash
# 터미널 1: jtop 실행
jtop

# 터미널 2: 추론 실행
./depth_inference ../models/depth_anything_small_fp16.trt
```

### 4.3 결과 기록 템플릿

```
┌─────────────────────────────────────────┐
│         Depth TRT 성능 측정 결과         │
├──────────────────┬──────────────────────┤
│ FP32 추론 시간   │         ms           │
│ FP16 추론 시간   │         ms           │
│ FP32 FPS         │         fps          │
│ FP16 FPS         │         fps          │
│ FP16 속도 향상   │         x            │
│ GPU 메모리 사용   │         MB           │
│ GPU 사용률       │         %            │
│ 전력 소비        │         W            │
│ 온도             │         °C           │
└──────────────────┴──────────────────────┘
```

---

## ✅ 체크리스트

### Step 1: ONNX 변환
- [ ] Depth Anything Small 모델 로드
- [ ] 384x512 고정 입력으로 ONNX 변환
- [ ] ONNX 모델 검증 통과
- [ ] ONNX Runtime 추론 확인

### Step 2: TensorRT 변환
- [ ] ONNX 파일을 Jetson에 전송
- [ ] trtexec로 FP16 변환 완료
- [ ] trtexec 벤치마크 실행
- [ ] FP32 vs FP16 비교

### Step 3: C++ 추론 코드
- [ ] TRT 엔진 로드 성공
- [ ] 전처리 (resize, normalize) 구현
- [ ] 추론 + 후처리 동작 확인
- [ ] Depth map 시각화

### Step 4: 성능 측정
- [ ] 100프레임 벤치마크 실행
- [ ] 15-20 FPS 달성 확인
- [ ] GPU 메모리 < 4GB 확인
- [ ] jtop으로 모니터링 완료

---

## 💡 트러블슈팅

### ONNX 변환 실패 시

```
문제: "Unsupported op" 에러
해결: opset_version 변경 (11, 12, 13, 17 순서로 시도)

문제: 동적 크기 관련 에러
해결: dynamic_axes=None 확인, torch.onnx.export에서 고정 입력 사용
```

### TensorRT 변환 실패 시

```
문제: "Out of memory" 에러
해결: --workspace 줄이기 (2048, 1024)

문제: "Unsupported layer" 에러
해결: TensorRT/JetPack 버전 확인 (최신 권장)
```

### FPS가 목표 미달 시

```
시도 1: MAX 전력 모드
  sudo nvpmodel -m 0
  sudo jetson_clocks

시도 2: 입력 크기 줄이기
  288 × 384 또는 256 × 320

시도 3: INT8 양자화 시도
  trtexec --best --int8
```

---

**다음**: Quiz로 개념 점검!
