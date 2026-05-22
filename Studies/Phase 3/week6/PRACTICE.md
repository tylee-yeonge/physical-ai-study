# Week 6 실습: TensorRT C++ 추론 파이프라인


> **목표**: ONNX 모델을 TensorRT로 변환하고 C++로 실시간 추론 구현하기
> **언어**: C++ (TensorRT, CUDA, OpenCV)
> **예상 시간**: 12시간


---


## 실습 개요


Week 6은 Week 5에서 생성한 ONNX 모델을 **TensorRT 엔진으로 변환**하고, **C++로 추론 파이프라인**을 구현합니다. NMS 후처리까지 포함하여 640x480 @ 30 FPS를 목표로 합니다.


---


## 환경 설정


### Jetson 환경 (권장)


```bash
# JetPack SDK 확인
cat /etc/nv_tegra_release
dpkg -l | grep TensorRT


# TensorRT 버전 확인
dpkg -l | grep libnvinfer
# 예: libnvinfer8 8.5.x


# OpenCV 확인
pkg-config --modversion opencv4
```


### 데스크톱 환경 (대안)


```bash
# TensorRT 설치 확인
dpkg -l | grep libnvinfer
# 또는
pip show tensorrt


# CUDA 확인
nvcc --version


# OpenCV 설치
sudo apt install libopencv-dev
```


---


## 프로젝트 구조


```
week6_tensorrt/
+-- CMakeLists.txt
+-- include/
| +-- trt_engine.h # TensorRT 엔진 클래스
| +-- yolo_detector.h # YOLO 검출기
| +-- nms.h # NMS 함수
+-- src/
| +-- trt_engine.cpp # 엔진 로드/추론
| +-- yolo_detector.cpp # 전처리/후처리
| +-- nms.cpp # NMS 구현
| +-- main.cpp # 메인 실행
+-- models/
| +-- yolo11n.onnx # Week 5에서 변환한 모델
+-- data/
    +-- test.jpg # 테스트 이미지
```


---


## Step 1: ONNX → TensorRT 변환 (trtexec)


### FP32 엔진 빌드


```bash
# 기본 FP32 변환
trtexec --onnx=models/yolo11n.onnx \
        --saveEngine=models/yolo11n_fp32.trt \
        --verbose


# 빌드 시간: 약 5-10분
```


### FP16 엔진 빌드 (권장)


```bash
# FP16 변환 (속도 2배 향상)
trtexec --onnx=models/yolo11n.onnx \
        --saveEngine=models/yolo11n_fp16.trt \
        --fp16 \
        --verbose


# 빌드 시간: 약 8-15분
```


### 벤치마크 실행


```bash
# FP32 벤치마크
trtexec --loadEngine=models/yolo11n_fp32.trt \
        --batch=1 --warmUp=500 --avgRuns=100


# FP16 벤치마크
trtexec --loadEngine=models/yolo11n_fp16.trt \
        --batch=1 --warmUp=500 --avgRuns=100
```


### 예상 출력 (Jetson Orin Nano)


```
[FP32]
GPU Compute Time: min = 28.5 ms, max = 32.1 ms, mean = 30.2 ms
Throughput: 33.1 qps


[FP16]
GPU Compute Time: min = 7.2 ms, max = 9.8 ms, mean = 8.1 ms
Throughput: 123.5 qps
```


---


## Step 2: TensorRT 엔진 로더 (C++)


### include/trt_engine.h


```cpp
#pragma once
#include <NvInfer.h> // TensorRT 핵심 헤더
#include <cuda_runtime.h> // CUDA 런타임 (GPU 메모리 할당 등)
#include <string>
#include <vector>
#include <fstream>
#include <iostream>


// TensorRT 로거 (TensorRT 내부 메시지를 받아 콘솔에 출력)
class TrtLogger : public nvinfer1::ILogger {
public:
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kWARNING) { // WARNING 이상 심각도만 출력
            std::cout << "[TRT] " << msg << std::endl;
        }
    }
};


class TrtEngine {
public:
    TrtEngine(const std::string& engine_path); // 엔진 파일을 로드하며 초기화
    ~TrtEngine();


    // 추론 실행 (입력 -> GPU 추론 -> 출력)
    bool infer(float* input_data, float* output_data);


    // Getter
    int getInputSize() const { return input_size_; }
    int getOutputSize() const { return output_size_; }


private:
    TrtLogger logger_;
    nvinfer1::IRuntime* runtime_ = nullptr; // 엔진 역직렬화용 런타임
    nvinfer1::ICudaEngine* engine_ = nullptr; // 역직렬화된 추론 엔진
    nvinfer1::IExecutionContext* context_ = nullptr; // 실제 추론을 실행하는 컨텍스트


    void* buffers_[2]; // [input, output] GPU 버퍼
    int input_size_ = 0; // 입력 크기 (float 원소 개수)
    int output_size_ = 0; // 출력 크기 (float 원소 개수)


    bool loadEngine(const std::string& path); // 엔진 파일을 읽어 역직렬화
    void allocateBuffers(); // 입출력용 GPU 메모리 할당
};
```


### src/trt_engine.cpp


```cpp
#include "trt_engine.h"


TrtEngine::TrtEngine(const std::string& engine_path) {
    if (!loadEngine(engine_path)) { // 엔진 로드 실패 시 예외 발생
        throw std::runtime_error("엔진 로드 실패: " + engine_path);
    }
    allocateBuffers(); // 입출력 GPU 버퍼 준비
    std::cout << "[INFO] TensorRT 엔진 로드 완료" << std::endl;
}


TrtEngine::~TrtEngine() {
    // 소멸 시 GPU 메모리와 TensorRT 객체를 모두 해제
    cudaFree(buffers_[0]);
    cudaFree(buffers_[1]);
    if (context_) context_->destroy();
    if (engine_) engine_->destroy();
    if (runtime_) runtime_->destroy();
    std::cout << "[INFO] TensorRT 리소스 해제 완료" << std::endl;
}


bool TrtEngine::loadEngine(const std::string& path) {
    // 엔진 파일 읽기
    std::ifstream file(path, std::ios::binary); // 바이너리 모드로 열기
    if (!file.good()) {
        std::cerr << "[ERROR] 파일 열기 실패: " << path << std::endl;
        return false;
    }


    file.seekg(0, std::ios::end); // 파일 끝으로 이동
    size_t size = file.tellg(); // 현재 위치 = 파일 크기
    file.seekg(0, std::ios::beg); // 다시 파일 처음으로


    std::vector<char> data(size); // 파일 크기만큼 버퍼 확보
    file.read(data.data(), size); // 엔진 데이터 전체 읽기
    file.close();


    // 런타임 생성 및 엔진 역직렬화
    runtime_ = nvinfer1::createInferRuntime(logger_);
    engine_ = runtime_->deserializeCudaEngine(data.data(), size); // 바이트 -> 엔진 객체
    context_ = engine_->createExecutionContext(); // 추론 실행 컨텍스트 생성


    if (!engine_ || !context_) {
        std::cerr << "[ERROR] 엔진 역직렬화 실패" << std::endl;
        return false;
    }


    return true;
}


void TrtEngine::allocateBuffers() {
    // 입력/출력 바인딩 정보
    int input_idx = engine_->getBindingIndex("images"); // 입력 텐서 인덱스
    int output_idx = engine_->getBindingIndex("output0"); // 출력 텐서 인덱스


    auto input_dims = engine_->getBindingDimensions(input_idx);
    auto output_dims = engine_->getBindingDimensions(output_idx);


    // 크기 계산 (float 기준) - 모든 차원을 곱해 전체 원소 수를 구함
    input_size_ = 1;
    for (int i = 0; i < input_dims.nbDims; i++) {
        input_size_ *= input_dims.d[i];
    }


    output_size_ = 1;
    for (int i = 0; i < output_dims.nbDims; i++) {
        output_size_ *= output_dims.d[i];
    }


    // GPU 메모리 할당 (원소 수 * float 크기)
    cudaMalloc(&buffers_[0], input_size_ * sizeof(float));
    cudaMalloc(&buffers_[1], output_size_ * sizeof(float));


    std::cout << "[INFO] 입력 크기: " << input_size_ << " floats" << std::endl;
    std::cout << "[INFO] 출력 크기: " << output_size_ << " floats" << std::endl;
}


bool TrtEngine::infer(float* input_data, float* output_data) {
    // CPU → GPU 복사
    cudaMemcpy(buffers_[0], input_data,
               input_size_ * sizeof(float), cudaMemcpyHostToDevice); // 입력을 GPU로 전송


    // 추론 실행
    bool success = context_->executeV2(buffers_); // GPU에서 추론 수행


    // GPU → CPU 복사
    cudaMemcpy(output_data, buffers_[1],
               output_size_ * sizeof(float), cudaMemcpyDeviceToHost); // 결과를 CPU로 회수


    return success;
}
```


---


## Step 3: NMS 구현


### include/nms.h


```cpp
#pragma once
#include <vector>
#include <algorithm>


struct Detection {
    float x1, y1, x2, y2; // 바운딩 박스 (xyxy)
    float confidence; // 신뢰도
    int class_id; // 클래스 ID
};


// IoU 계산 (두 박스가 겹치는 정도)
float computeIoU(const Detection& a, const Detection& b);


// NMS 실행 (중복 검출 박스 제거)
std::vector<Detection> nms(std::vector<Detection>& detections,
                           float iou_threshold = 0.45f);
```


### src/nms.cpp


```cpp
#include "nms.h"


float computeIoU(const Detection& a, const Detection& b) {
    // 교집합 계산 (겹치는 영역의 좌표)
    float x1 = std::max(a.x1, b.x1);
    float y1 = std::max(a.y1, b.y1);
    float x2 = std::min(a.x2, b.x2);
    float y2 = std::min(a.y2, b.y2);


    float intersection = std::max(0.0f, x2 - x1) * std::max(0.0f, y2 - y1); // 교집합 넓이


    // 합집합 계산
    float area_a = (a.x2 - a.x1) * (a.y2 - a.y1); // a 박스 넓이
    float area_b = (b.x2 - b.x1) * (b.y2 - b.y1); // b 박스 넓이
    float union_area = area_a + area_b - intersection; // 합집합 넓이


    if (union_area <= 0.0f) return 0.0f; // 0 나눗셈 방지
    return intersection / union_area; // IoU = 교집합 / 합집합
}


std::vector<Detection> nms(std::vector<Detection>& detections,
                           float iou_threshold) {
    // confidence 기준 내림차순 정렬 (점수 높은 박스가 앞으로)
    std::sort(detections.begin(), detections.end(),
              [](const Detection& a, const Detection& b) {
                  return a.confidence > b.confidence;
              });


    std::vector<bool> suppressed(detections.size(), false); // 박스별 제거 여부
    std::vector<Detection> result;


    for (size_t i = 0; i < detections.size(); i++) {
        if (suppressed[i]) continue; // 이미 제거된 박스는 건너뜀


        result.push_back(detections[i]); // 점수가 가장 높은 박스는 유지


        // 현재 박스와 IoU가 높은 박스 제거
        for (size_t j = i + 1; j < detections.size(); j++) {
            if (suppressed[j]) continue;
            if (detections[i].class_id != detections[j].class_id) continue; // 다른 클래스는 비교 안 함


            float iou = computeIoU(detections[i], detections[j]);
            if (iou > iou_threshold) { // 많이 겹치면 중복으로 보고 제거
                suppressed[j] = true;
            }
        }
    }


    return result;
}
```


---


## Step 4: YOLO 전처리/후처리


### src/yolo_detector.cpp (핵심 부분)


```cpp
#include <opencv2/opencv.hpp>
#include "trt_engine.h"
#include "nms.h"


class YoloDetector {
public:
    YoloDetector(const std::string& engine_path,
                 float conf_thresh = 0.25f,
                 float iou_thresh = 0.45f)
        : engine_(engine_path), // 멤버 초기화 리스트로 엔진 로드
          conf_thresh_(conf_thresh),
          iou_thresh_(iou_thresh) {}


    std::vector<Detection> detect(const cv::Mat& image) {
        // 1. 전처리
        cv::Mat blob;
        preprocess(image, blob);


        // 2. 추론
        std::vector<float> output(engine_.getOutputSize());
        engine_.infer(reinterpret_cast<float*>(blob.data), output.data());


        // 3. 후처리
        return postprocess(output, image.size());
    }


private:
    TrtEngine engine_;
    float conf_thresh_; // confidence 임계값
    float iou_thresh_; // NMS IoU 임계값


    void preprocess(const cv::Mat& image, cv::Mat& blob) {
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(640, 640)); // 모델 입력 크기로 리사이즈
        cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB); // BGR -> RGB
        resized.convertTo(resized, CV_32F, 1.0 / 255.0); // 0~255 -> 0~1 정규화


        // HWC → CHW 변환
        std::vector<cv::Mat> channels(3);
        cv::split(resized, channels); // 채널 분리 (R, G, B)


        // CHW 연속 메모리 할당
        blob = cv::Mat(3 * 640, 640, CV_32F);
        for (int c = 0; c < 3; c++) { // 채널을 세로로 이어붙여 CHW 형태로 배치
            channels[c].copyTo(blob(cv::Rect(0, c * 640, 640, 640)));
        }
    }


    std::vector<Detection> postprocess(const std::vector<float>& output,
                                        cv::Size orig_size) {
        // YOLO11 출력: [1, 84, 8400] → 전치 → [8400, 84]
        const int num_classes = 80;
        const int num_boxes = 8400;


        std::vector<Detection> detections;


        for (int i = 0; i < num_boxes; i++) { // 후보 박스 8400개를 하나씩 검사
            // xywh 추출 (박스 중심좌표와 크기)
            float cx = output[0 * num_boxes + i];
            float cy = output[1 * num_boxes + i];
            float w = output[2 * num_boxes + i];
            float h = output[3 * num_boxes + i];


            // 클래스 점수 최댓값 탐색
            float max_score = 0.0f;
            int max_class = 0;
            for (int c = 0; c < num_classes; c++) {
                float score = output[(4 + c) * num_boxes + i];
                if (score > max_score) { // 점수가 가장 높은 클래스 찾기
                    max_score = score;
                    max_class = c;
                }
            }


            if (max_score < conf_thresh_) continue; // 임계값 미만은 버림


            // xywh → xyxy (중심+크기 -> 좌상단/우하단, 원본 이미지 크기로 스케일링)
            Detection det;
            det.x1 = (cx - w / 2.0f) * orig_size.width / 640.0f;
            det.y1 = (cy - h / 2.0f) * orig_size.height / 640.0f;
            det.x2 = (cx + w / 2.0f) * orig_size.width / 640.0f;
            det.y2 = (cy + h / 2.0f) * orig_size.height / 640.0f;
            det.confidence = max_score;
            det.class_id = max_class;


            detections.push_back(det);
        }


        // NMS 적용 (중복 박스 제거)
        return nms(detections, iou_thresh_);
    }
};
```


---


## Step 5: 메인 실행 및 FPS 측정


### src/main.cpp


```cpp
#include <iostream>
#include <chrono> // 시간 측정용
#include <opencv2/opencv.hpp>
#include "trt_engine.h"
#include "nms.h"


int main(int argc, char** argv) {
    std::string engine_path = "models/yolo11n_fp16.trt"; // 기본 엔진 경로
    std::string image_path = "data/test.jpg"; // 기본 이미지 경로


    if (argc >= 2) engine_path = argv[1]; // 첫 번째 인자로 엔진 경로 받기
    if (argc >= 3) image_path = argv[2]; // 두 번째 인자로 이미지 경로 받기


    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "TensorRT YOLO11 추론 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;


    // 엔진 로드
    TrtEngine engine(engine_path);


    // 테스트 이미지 로드
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) { // 이미지 로드 실패 체크
        std::cerr << "[ERROR] 이미지 로드 실패: " << image_path << std::endl;
        return -1;
    }


    // Warm-up (10회)
    std::vector<float> input(engine.getInputSize());
    std::vector<float> output(engine.getOutputSize());


    for (int i = 0; i < 10; i++) { // 측정 전 워밍업 (첫 추론은 느림)
        engine.infer(input.data(), output.data());
    }
    cudaDeviceSynchronize(); // GPU 작업 완료 대기


    // FPS 측정 (100회)
    int num_runs = 100;
    auto start = std::chrono::high_resolution_clock::now(); // 측정 시작 시각


    for (int i = 0; i < num_runs; i++) {
        engine.infer(input.data(), output.data());
    }
    cudaDeviceSynchronize(); // 모든 추론 완료 대기


    auto end = std::chrono::high_resolution_clock::now(); // 측정 종료 시각
    double total_ms = std::chrono::duration<double, std::milli>(end - start).count();
    double avg_ms = total_ms / num_runs; // 평균 추론 시간
    double fps = 1000.0 / avg_ms; // 초당 프레임 수


    std::cout << "\n[결과]" << std::endl;
    std::cout << "평균 추론 시간: " << avg_ms << " ms" << std::endl;
    std::cout << "FPS: " << fps << std::endl;
    std::cout << "목표 달성: " << (fps >= 30 ? "성공!" : "미달") << std::endl;


    return 0;
}
```


---


## 빌드 및 실행


```bash
# 빌드
mkdir build && cd build
cmake ..
make -j$(nproc)


# 실행
./phase3_week6_tensorrt models/yolo11n_fp16.trt data/test.jpg


# 카메라 실시간 테스트 (선택)
./phase3_week6_tensorrt models/yolo11n_fp16.trt --camera 0
```


### 예상 출력


```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  TensorRT YOLO11 추론 테스트
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[TRT] Loaded engine: models/yolo11n_fp16.trt
[INFO] TensorRT 엔진 로드 완료
[INFO] 입력 크기: 1228800 floats
[INFO] 출력 크기: 705600 floats


[결과]
  평균 추론 시간: 8.1 ms
  FPS: 123.5
  목표 달성: 성공!
```


---


## 체크리스트


- [ ] trtexec로 FP32 엔진 빌드 성공
- [ ] trtexec로 FP16 엔진 빌드 성공
- [ ] FP32 vs FP16 벤치마크 비교 완료
- [ ] C++ TensorRT 엔진 로더 구현
- [ ] GPU 메모리 할당/해제 구현
- [ ] 전처리 (resize, normalize, HWC→CHW) 구현
- [ ] NMS (IoU + confidence) 구현
- [ ] 후처리 (bbox 변환, 스케일링) 구현
- [ ] FPS 측정 코드 구현
- [ ] 640x480 @ 30 FPS 목표 달성


---


## 트러블슈팅


### trtexec가 없을 때
```
문제: trtexec: command not found
해결: 경로를 명시적으로 지정
  /usr/src/tensorrt/bin/trtexec (Jetson)
  또는 TensorRT SDK 설치 후 bin/ 디렉토리 확인
```


### 엔진 로드 실패
```
문제: "엔진 역직렬화 실패"
해결:
1. GPU가 빌드 시 사용한 GPU와 동일한지 확인
2. TensorRT 버전 호환성 확인
3. 엔진 파일이 손상되지 않았는지 확인 (파일 크기 체크)
```


### CUDA 메모리 부족
```
문제: cudaMalloc failed: out of memory
해결:
1. 다른 GPU 프로세스 종료
2. --workspace 크기 줄이기
3. FP16 사용으로 메모리 절약
```


### NMS 결과가 이상할 때
```
문제: 박스가 너무 많거나 적음
해결:
1. conf_thresh 조절 (기본 0.25)
2. iou_thresh 조절 (기본 0.45)
3. YOLO 출력 포맷 확인 ([1, 84, 8400] 맞는지)
4. xywh → xyxy 변환 로직 확인
```


---


**이전**: [Week 5 - ONNX 변환](../week5/PRACTICE.md)
**다음**: [Week 7 - Monocular Depth 이론](../week7/README.md)
