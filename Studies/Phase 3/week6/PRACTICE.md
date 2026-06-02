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


# TensorRT 버전 확인 (본 자료는 TRT 10.x 기준)
dpkg -l | grep libnvinfer
# 예: libnvinfer10 10.x.x (JetPack 6.x 또는 dGPU TRT 10)


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


### 구조 생성 스크립트

아래 스크립트를 실행하면 위 디렉터리와 빈 소스 파일이 한 번에 만들어지고, week5 산출물(ONNX 모델, 테스트 이미지)을 복사한다. 각 소스 파일의 내용은 Step 2-5에서 채운다. week5 산출물을 찾지 못하면 경고만 출력하고 계속 진행하므로, 그때는 직접 채워 넣으면 된다.


```bash
#!/bin/bash
# week6_tensorrt 프로젝트 구조를 생성하는 스크립트
set -e  # 명령 하나라도 실패하면 즉시 중단

ROOT=outputs/week6_tensorrt  # 프로젝트 루트 (Phase 3 관례: 산출물은 outputs/ 아래)

# 디렉터리 생성 (-p: 중간 경로까지, 이미 있어도 에러 안 남)
mkdir -p "$ROOT/include"  # 헤더 파일 디렉터리
mkdir -p "$ROOT/src"      # 구현 파일 디렉터리
mkdir -p "$ROOT/models"   # ONNX/TensorRT 엔진 디렉터리
mkdir -p "$ROOT/data"     # 테스트 이미지 디렉터리

# 빌드 설정 파일 (내용은 별도 작성)
touch "$ROOT/CMakeLists.txt"

# 헤더 파일 (Step 2, 3에서 작성)
touch "$ROOT/include/trt_engine.h"    # TensorRT 엔진 클래스
touch "$ROOT/include/yolo_detector.h" # YOLO 검출기
touch "$ROOT/include/nms.h"           # NMS 함수

# 구현 파일 (Step 2-5에서 작성)
touch "$ROOT/src/trt_engine.cpp"      # 엔진 로드/추론
touch "$ROOT/src/yolo_detector.cpp"   # 전처리/후처리
touch "$ROOT/src/nms.cpp"             # NMS 구현
touch "$ROOT/src/main.cpp"            # 메인 실행

# 생성된 구조 확인
echo "[INFO] 프로젝트 구조 생성 완료:"
find "$ROOT" -print | sort

# week5 산출물 복사 (경로는 환경에 맞게 수정 가능)
WEEK5_DIR=../week5                          # week5 디렉터리 (ONNX/이미지 출처)
ONNX_SRC="$WEEK5_DIR/yolo11n_static.onnx"   # week5에서 export한 정적 shape ONNX
IMG_SRC="$WEEK5_DIR/test.jpg"               # 테스트 이미지

# ONNX 모델 복사 (없으면 경고만 출력하고 계속 진행)
if [ -f "$ONNX_SRC" ]; then
    cp "$ONNX_SRC" "$ROOT/models/yolo11n.onnx"
    echo "[INFO] ONNX 복사: $ONNX_SRC -> $ROOT/models/yolo11n.onnx"
else
    echo "[WARN] ONNX를 찾지 못함: $ONNX_SRC"
    echo "       week5에서 ONNX를 먼저 export한 뒤 $ROOT/models/yolo11n.onnx로 복사하세요."
fi

# 테스트 이미지 복사 (없으면 경고만 출력하고 계속 진행)
if [ -f "$IMG_SRC" ]; then
    cp "$IMG_SRC" "$ROOT/data/test.jpg"
    echo "[INFO] 이미지 복사: $IMG_SRC -> $ROOT/data/test.jpg"
else
    echo "[WARN] 테스트 이미지를 찾지 못함: $IMG_SRC"
    echo "       임의의 테스트 이미지를 $ROOT/data/test.jpg로 복사하세요."
fi
```


실행:


```bash
chmod +x setup.sh  # 실행 권한 부여
./setup.sh
```


---


## Step 1: ONNX → TensorRT 변환 (trtexec)

week5에서 만든 ONNX를 이제 NVIDIA GPU 전용 엔진(`.trt`)으로 한 번 더 변환한다. `trtexec` 명령 한 줄로 ONNX를 TensorRT 엔진으로 빌드하고 속도를 잰다.

### 왜 TensorRT로 또 변환하나

핵심 질문은 "왜 ONNX Runtime이 아니라 TensorRT인가"다. Jetson 같은 임베디드 보드에서는 ONNX Runtime만으로 30 FPS를 내기 어렵다. TensorRT는 NVIDIA GPU에 특화돼 Layer Fusion(레이어 합치기), Kernel Auto-Tuning(GPU별 최적 커널 선택), FP16/INT8 최적화를 적용해 같은 모델을 훨씬 빠르게 돌린다. 중요한 제약이 있다. TensorRT 엔진은 빌드한 GPU에서만 동작한다. Kernel Auto-Tuning이 그 GPU 아키텍처에 맞춰 튜닝하기 때문이다. 그래서 `.trt`는 배포 대상 장비에서 직접 빌드해야 한다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| FP32 빌드 | `trtexec --onnx --saveEngine` | 기본 엔진 생성 |
| FP16 빌드 | `--fp16` | 속도 약 2배 (권장) |
| 벤치마크 | trtexec로 100회 평균 | 엔진 추론 속도 측정 |

### 핵심 포인트

- FP16 플래그는 `--fp16`이다. `--half`나 `--float16`은 존재하지 않는다(README 명시).
- 빌드는 수 분 걸린다. 이것은 추론이 아니라 GPU별 최적화 탐색 시간이다. 한 번 빌드하면 `.trt`를 재사용한다.


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

빌드된 `.trt` 엔진을 C++에서 로드해 추론할 수 있게 감싸는 클래스를 만든다. Jetson 실시간 추론을 C++로 짜는 첫 부품이다.

### 왜 C++로 짜나

실시간 추론은 보통 C++로 짠다(속도, 하드웨어 제어). TensorRT C++ API의 세 핵심 객체를 다룬다. `IRuntime`(엔진 파일 로드), `ICudaEngine`(최적화된 모델 그 자체), `IExecutionContext`(실제 추론 수행). 직렬화된 `.trt`를 메모리로 읽어 deserialize하고, GPU 입출력 버퍼를 잡는 것까지가 이 로더의 일이다. Python 한 줄(`InferenceSession`)이 C++에서는 여러 단계로 펼쳐진다. 메모리 할당과 해제를 직접 관리해야 하는 것이 차이다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 엔진 로드 | `IRuntime::deserializeCudaEngine` | `.trt` 파일을 엔진 객체로 |
| 실행 컨텍스트 | `createExecutionContext` | 추론 수행 객체 생성 |
| 버퍼 관리 | `cudaMalloc` 입출력 | GPU 메모리 직접 할당 |

### 핵심 포인트

- Python과 달리 GPU 버퍼를 직접 `cudaMalloc`/`Free` 한다. 이것을 빠뜨리면 메모리 누수가 생긴다(트러블슈팅의 CUDA 메모리 부족).
- 헤더(`.h`)와 구현(`.cpp`)을 나눈 구조는 C++ 관례다. 인터페이스와 구현을 분리한다.


### include/trt_engine.h


```cpp
#ifndef PHASE3_WEEK6_TRT_ENGINE_H_
#define PHASE3_WEEK6_TRT_ENGINE_H_

#include <NvInfer.h>       // TensorRT 핵심 헤더
#include <cuda_runtime.h>  // CUDA 런타임 (GPU 메모리 할당 등)
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief TensorRT 로거 (TensorRT 내부 메시지를 받아 콘솔에 출력)
 */
class TrtLogger : public nvinfer1::ILogger
{
public:
    /**
     * @brief TensorRT 내부에서 호출하는 로그 콜백
     * @param severity 메시지 심각도
     * @param msg 로그 메시지 문자열
     */
    void log(Severity severity, const char* msg) noexcept override
    {
        if (severity <= Severity::kWARNING)  // WARNING 이상 심각도만 출력
        {
            std::cout << "[TRT] " << msg << std::endl;
        }
    }
};

/**
 * @brief 직렬화된 .trt 엔진을 로드해 추론을 수행하는 래퍼 클래스
 */
class TrtEngine
{
public:
    /**
     * @brief 엔진 파일을 로드하며 초기화
     * @param engine_path .trt 엔진 파일 경로
     */
    explicit TrtEngine(const std::string& engine_path);
    ~TrtEngine();

    /**
     * @brief 추론 실행 (입력 -> GPU 추론 -> 출력)
     * @param input_data 모델 입력 (host, 읽기 전용, get_input_size() 개의 float)
     * @param output_data 모델 출력 (host, get_output_size() 개의 float가 기록됨)
     * @return 추론 성공 여부
     */
    bool infer(const float* input_data, float* output_data);

    /**
     * @brief 입력 텐서 원소 개수 반환
     * @return float 원소 개수
     */
    int get_input_size() const
    {
        return input_size_;
    }

    /**
     * @brief 출력 텐서 원소 개수 반환
     * @return float 원소 개수
     */
    int get_output_size() const
    {
        return output_size_;
    }

private:
    /**
     * @brief 엔진 파일을 읽어 역직렬화
     * @param path .trt 엔진 파일 경로
     * @return 로드 성공 여부
     */
    bool load_engine(const std::string& path);

    /**
     * @brief 입출력용 GPU 메모리를 할당하고 텐서 주소를 컨텍스트에 등록
     */
    void allocate_buffers();

    TrtLogger logger_;
    nvinfer1::IRuntime* runtime_ = nullptr;           // 엔진 역직렬화용 런타임
    nvinfer1::ICudaEngine* engine_ = nullptr;         // 역직렬화된 추론 엔진
    nvinfer1::IExecutionContext* context_ = nullptr;  // 실제 추론을 실행하는 컨텍스트

    void* buffers_[2];         // [input, output] GPU 버퍼
    int input_size_ = 0;       // 입력 크기 (float 원소 개수)
    int output_size_ = 0;      // 출력 크기 (float 원소 개수)
    std::string input_name_;   // 입력 텐서 이름 (TRT 10 API: setTensorAddress에 사용)
    std::string output_name_;  // 출력 텐서 이름
};

#endif  // PHASE3_WEEK6_TRT_ENGINE_H_
```


### src/trt_engine.cpp


```cpp
#include "trt_engine.h"


TrtEngine::TrtEngine(const std::string& engine_path)
{
    if (!load_engine(engine_path))  // 엔진 로드 실패 시 예외 발생
    {
        throw std::runtime_error("엔진 로드 실패: " + engine_path);
    }
    allocate_buffers();  // 입출력 GPU 버퍼 준비
    std::cout << "[INFO] TensorRT 엔진 로드 완료" << std::endl;
}


TrtEngine::~TrtEngine()
{
    // 소멸 시 GPU 메모리 해제 후 TensorRT 객체를 delete (TRT 10에서 destroy() 메서드 제거됨)
    cudaFree(buffers_[0]);
    cudaFree(buffers_[1]);
    delete context_;
    delete engine_;
    delete runtime_;
    std::cout << "[INFO] TensorRT 리소스 해제 완료" << std::endl;
}


bool TrtEngine::load_engine(const std::string& path)
{
    // 엔진 파일 읽기
    std::ifstream file(path, std::ios::binary);  // 바이너리 모드로 열기
    if (!file.good())
    {
        std::cerr << "[ERROR] 파일 열기 실패: " << path << std::endl;
        return false;
    }


    file.seekg(0, std::ios::end);  // 파일 끝으로 이동
    size_t size = file.tellg();    // 현재 위치 = 파일 크기
    file.seekg(0, std::ios::beg);  // 다시 파일 처음으로


    std::vector<char> data(size);  // 파일 크기만큼 버퍼 확보
    file.read(data.data(), size);  // 엔진 데이터 전체 읽기
    file.close();


    // 런타임 생성 및 엔진 역직렬화
    runtime_ = nvinfer1::createInferRuntime(logger_);
    engine_ = runtime_->deserializeCudaEngine(data.data(), size);  // 바이트 -> 엔진 객체
    context_ = engine_->createExecutionContext();  // 추론 실행 컨텍스트 생성


    if (!engine_ || !context_)
    {
        std::cerr << "[ERROR] 엔진 역직렬화 실패" << std::endl;
        return false;
    }


    return true;
}


void TrtEngine::allocate_buffers()
{
    // TRT 10 API: getBindingIndex/getBindingDimensions 대신
    // getNbIOTensors + getIOTensorName + getTensorIOMode + getTensorShape 사용
    int n_io = engine_->getNbIOTensors();  // 입출력 텐서 총 개수
    for (int i = 0; i < n_io; i++)
    {
        const char* name = engine_->getIOTensorName(i);  // i번째 텐서 이름
        auto mode = engine_->getTensorIOMode(name);      // 입력인지 출력인지
        auto dims = engine_->getTensorShape(name);       // 텐서 차원

        // 모든 차원을 곱해 전체 원소 수 계산
        int volume = 1;
        for (int d = 0; d < dims.nbDims; d++)
        {
            volume *= dims.d[d];
        }

        if (mode == nvinfer1::TensorIOMode::kINPUT)  // 입력 텐서
        {
            input_size_ = volume;
            input_name_ = name;
        }
        else  // 출력 텐서
        {
            output_size_ = volume;
            output_name_ = name;
        }
    }


    // GPU 메모리 할당 (원소 수 * float 크기)
    cudaMalloc(&buffers_[0], input_size_ * sizeof(float));
    cudaMalloc(&buffers_[1], output_size_ * sizeof(float));


    // TRT 10: 텐서 이름 → 디바이스 주소를 컨텍스트에 등록 (이후 enqueueV3가 이 주소를 사용)
    context_->setTensorAddress(input_name_.c_str(), buffers_[0]);
    context_->setTensorAddress(output_name_.c_str(), buffers_[1]);


    std::cout << "[INFO] 입력 (" << input_name_ << "): " << input_size_ << " floats" << std::endl;
    std::cout << "[INFO] 출력 (" << output_name_ << "): " << output_size_ << " floats" << std::endl;
}


bool TrtEngine::infer(const float* input_data, float* output_data)
{
    // CPU → GPU 복사
    cudaMemcpy(buffers_[0], input_data,
               input_size_ * sizeof(float), cudaMemcpyHostToDevice);  // 입력을 GPU로 전송


    // 추론 실행 (TRT 10: executeV2 대신 enqueueV3 + stream 동기화)
    // setTensorAddress는 allocate_buffers에서 이미 호출됨
    bool success = context_->enqueueV3(0);  // default stream(0)에 추론 작업 등록
    cudaStreamSynchronize(0);               // 추론 완료 대기 (동기 실행)


    // GPU → CPU 복사
    cudaMemcpy(output_data, buffers_[1],
               output_size_ * sizeof(float), cudaMemcpyDeviceToHost);  // 결과를 CPU로 회수


    return success;
}
```


---


## Step 3: NMS 구현

week3에서 Python으로 짠 NMS를 이번엔 C++로 다시 구현한다. TensorRT 추론 결과(원시 박스 텐서)를 사람이 읽을 검출 결과로 정리하는 후처리다.

### 왜 NMS를 또 짜나

TensorRT는 모델의 forward만 한다. 중복 박스 제거(NMS)는 엔진 밖에서 직접 해야 한다(week5 Step 3에서 본 것과 같은 구조). week3에서 알고리즘을 이미 이해했으니, 여기서는 그것을 C++로 옮기는 것이 핵심이다. 알고리즘은 같다(confidence 정렬 -> IoU로 억제 -> 반복). C++로 짜는 이유는 실시간 파이프라인의 일부라 Python 오버헤드를 피해야 하기 때문이다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| IoU 계산 | C++ 함수 | week3 IoU의 C++판 |
| 정렬/억제 | confidence 내림차순 + 임계값 | NMS 핵심 루프 |
| 결과 구조체 | `Detection` struct | 박스/클래스/점수 묶기 |

### 핵심 포인트

- 알고리즘은 week3와 동일하다. 새로 배우는 것은 C++ 자료구조(`std::vector`, `sort`)로 같은 로직을 표현하는 것이다.
- NMS 결과가 이상하면(트러블슈팅) 보통 좌표 포맷이나 IoU 임계값 문제다. week3 NMS의 디버깅 감각이 그대로 쓰인다.


### include/nms.h


```cpp
#ifndef PHASE3_WEEK6_NMS_H_
#define PHASE3_WEEK6_NMS_H_

#include <algorithm>
#include <vector>

/**
 * @brief 검출 결과 한 건 (바운딩 박스 + 클래스 + 점수)
 */
struct Detection
{
    float x1, y1, x2, y2;  // 바운딩 박스 (xyxy)
    float confidence;      // 신뢰도
    int class_id;          // 클래스 ID
};

/**
 * @brief 두 박스의 IoU 계산 (겹치는 정도)
 * @param a 첫 번째 박스
 * @param b 두 번째 박스
 * @return IoU 값 (0.0 ~ 1.0)
 */
float compute_iou(const Detection& a, const Detection& b);

/**
 * @brief NMS 실행 (중복 검출 박스 제거)
 * @param detections 후보 검출 목록 (confidence 기준으로 제자리 정렬됨)
 * @param iou_threshold 이 값을 초과해 겹치면 중복으로 보고 억제
 * @return 살아남은 검출 목록
 */
std::vector<Detection> nms(std::vector<Detection>& detections,
                           float iou_threshold = 0.45f);

#endif  // PHASE3_WEEK6_NMS_H_
```


### src/nms.cpp


```cpp
#include "nms.h"


float compute_iou(const Detection& a, const Detection& b)
{
    // 교집합 계산 (겹치는 영역의 좌표)
    float x1 = std::max(a.x1, b.x1);
    float y1 = std::max(a.y1, b.y1);
    float x2 = std::min(a.x2, b.x2);
    float y2 = std::min(a.y2, b.y2);


    float intersection = std::max(0.0f, x2 - x1) * std::max(0.0f, y2 - y1);  // 교집합 넓이


    // 합집합 계산
    float area_a = (a.x2 - a.x1) * (a.y2 - a.y1);       // a 박스 넓이
    float area_b = (b.x2 - b.x1) * (b.y2 - b.y1);       // b 박스 넓이
    float union_area = area_a + area_b - intersection;  // 합집합 넓이


    if (union_area <= 0.0f)
    {
        return 0.0f;  // 0 나눗셈 방지
    }
    return intersection / union_area;  // IoU = 교집합 / 합집합
}


std::vector<Detection> nms(std::vector<Detection>& detections,
                           float iou_threshold)
{
    // confidence 기준 내림차순 정렬 (점수 높은 박스가 앞으로)
    std::sort(detections.begin(), detections.end(),
              [](const Detection& a, const Detection& b)
              {
                  return a.confidence > b.confidence;
              });


    std::vector<bool> suppressed(detections.size(), false);  // 박스별 제거 여부
    std::vector<Detection> result;


    for (size_t i = 0; i < detections.size(); i++)
    {
        if (suppressed[i])
        {
            continue;  // 이미 제거된 박스는 건너뜀
        }


        result.push_back(detections[i]);  // 점수가 가장 높은 박스는 유지


        // 현재 박스와 IoU가 높은 박스 제거
        for (size_t j = i + 1; j < detections.size(); j++)
        {
            if (suppressed[j])
            {
                continue;
            }
            if (detections[i].class_id != detections[j].class_id)
            {
                continue;  // 다른 클래스는 비교 안 함
            }


            float iou = compute_iou(detections[i], detections[j]);
            if (iou > iou_threshold)  // 많이 겹치면 중복으로 보고 제거
            {
                suppressed[j] = true;
            }
        }
    }


    return result;
}
```


---


## Step 4: YOLO 전처리/후처리

카메라 프레임을 모델 입력으로 바꾸고(전처리), 모델 출력을 박스로 푸는(후처리) 핵심 로직을 C++로 구현한다. 엔진 로더(Step 2)와 NMS(Step 3)를 `YoloDetector` 클래스 하나로 묶어, Step 5의 `main.cpp`가 호출할 검출기를 완성하는 부분이다.

### 왜 전처리/후처리를 직접 하나

모델은 정해진 크기와 형식의 텐서만 받는다. 임의 크기의 카메라 이미지를 letterbox(비율 유지 리사이즈 + 패딩)로 모델 입력 크기에 맞추고, BGR -> RGB, 0-1 정규화, HWC -> CHW 변환을 한다. week1 실습 1의 이미지 텐서 변환을 C++로 하는 셈이다. 후처리는 모델의 원시 출력(그리드별 박스 좌표와 점수)을 원본 이미지 좌표로 되돌리고 NMS를 적용한다. letterbox로 넣었으니 그 패딩과 스케일을 역산해야 박스가 원본에 맞게 그려진다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 전처리 | letterbox + 정규화 + CHW | 카메라 이미지를 모델 입력 텐서로 |
| 후처리 | 출력 디코딩 + 좌표 역변환 | 모델 출력을 원본 좌표 박스로 |
| NMS 연동 | Step 3 NMS 호출 | 중복 제거까지 |

### 핵심 포인트

- letterbox의 스케일과 패딩을 후처리에서 정확히 역산하지 않으면 박스가 어긋난다. 전처리와 후처리가 짝을 이뤄야 한다.
- week1의 HWC/CHW, BGR/RGB 변환 지식이 여기서 C++로 실전 적용된다.
- Step 2의 `TrtEngine`처럼 헤더(선언)와 구현을 나눈다. 헤더로 `YoloDetector`를 노출해야 Step 5의 `main.cpp`가 이 검출기를 호출할 수 있다.


### include/yolo_detector.h


```cpp
#ifndef PHASE3_WEEK6_YOLO_DETECTOR_H_
#define PHASE3_WEEK6_YOLO_DETECTOR_H_

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "nms.h"         // Detection 구조체, nms 함수 (Step 3)
#include "trt_engine.h"  // 엔진 로더 (Step 2)

/**
 * @brief YOLO11 검출기 (전처리 + TensorRT 추론 + 후처리 + NMS를 하나로 묶음)
 */
class YoloDetector
{
public:
    /**
     * @brief 엔진 경로와 임계값으로 검출기 초기화
     * @param engine_path .trt 엔진 파일 경로
     * @param conf_thresh confidence 임계값 (이 값 미만 박스는 버림)
     * @param iou_thresh NMS IoU 임계값
     */
    explicit YoloDetector(const std::string& engine_path,
                          float conf_thresh = 0.25f,
                          float iou_thresh = 0.45f);

    /**
     * @brief 이미지 한 장을 받아 검출 결과 목록을 반환
     * @param image 입력 이미지 (BGR, 임의 크기)
     * @return 검출 결과 목록 (전처리 -> 추론 -> 후처리 -> NMS)
     */
    std::vector<Detection> detect(const cv::Mat& image);

private:
    /**
     * @brief 이미지를 모델 입력 텐서(CHW, 정규화)로 변환
     * @param image 입력 이미지 (BGR)
     * @param blob 변환된 입력 텐서 (출력 인자)
     */
    void preprocess(const cv::Mat& image, cv::Mat& blob);

    /**
     * @brief 모델 원시 출력을 원본 좌표계의 검출 박스로 변환
     * @param output 모델 출력 (flatten된 float 벡터)
     * @param orig_size 원본 이미지 크기 (좌표 역변환에 사용)
     * @return NMS까지 적용된 검출 결과 목록
     */
    std::vector<Detection> postprocess(const std::vector<float>& output,
                                       cv::Size orig_size);

    TrtEngine engine_;   // 내부에 TensorRT 엔진을 소유 (생성자에서 로드)
    float conf_thresh_;  // confidence 임계값
    float iou_thresh_;   // NMS IoU 임계값
};

#endif  // PHASE3_WEEK6_YOLO_DETECTOR_H_
```


### src/yolo_detector.cpp


```cpp
#include "yolo_detector.h"


YoloDetector::YoloDetector(const std::string& engine_path,
                           float conf_thresh,
                           float iou_thresh)
    : engine_(engine_path),  // 멤버 초기화 리스트로 엔진 로드
      conf_thresh_(conf_thresh),
      iou_thresh_(iou_thresh)
{
}


std::vector<Detection> YoloDetector::detect(const cv::Mat& image)
{
    // 1. 전처리
    cv::Mat blob;
    preprocess(image, blob);


    // 2. 추론
    std::vector<float> output(engine_.get_output_size());
    engine_.infer(reinterpret_cast<const float*>(blob.data), output.data());


    // 3. 후처리
    return postprocess(output, image.size());
}


void YoloDetector::preprocess(const cv::Mat& image, cv::Mat& blob)
{
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(640, 640));     // 모델 입력 크기로 리사이즈
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);  // BGR -> RGB
    resized.convertTo(resized, CV_32F, 1.0 / 255.0);    // 0~255 -> 0~1 정규화


    // HWC → CHW 변환
    std::vector<cv::Mat> channels(3);
    cv::split(resized, channels);  // 채널 분리 (R, G, B)


    // CHW 연속 메모리 할당
    blob = cv::Mat(3 * 640, 640, CV_32F);
    for (int c = 0; c < 3; c++)  // 채널을 세로로 이어붙여 CHW 형태로 배치
    {
        channels[c].copyTo(blob(cv::Rect(0, c * 640, 640, 640)));
    }
}


std::vector<Detection> YoloDetector::postprocess(const std::vector<float>& output,
                                                 cv::Size orig_size)
{
    // YOLO11 출력: [1, 84, 8400] → 전치 → [8400, 84]
    const int kNumClasses = 80;
    const int kNumBoxes = 8400;


    std::vector<Detection> detections;


    for (int i = 0; i < kNumBoxes; i++)  // 후보 박스 8400개를 하나씩 검사
    {
        // xywh 추출 (박스 중심좌표와 크기)
        float cx = output[0 * kNumBoxes + i];
        float cy = output[1 * kNumBoxes + i];
        float w = output[2 * kNumBoxes + i];
        float h = output[3 * kNumBoxes + i];


        // 클래스 점수 최댓값 탐색
        float max_score = 0.0f;
        int max_class = 0;
        for (int c = 0; c < kNumClasses; c++)
        {
            float score = output[(4 + c) * kNumBoxes + i];
            if (score > max_score)  // 점수가 가장 높은 클래스 찾기
            {
                max_score = score;
                max_class = c;
            }
        }


        if (max_score < conf_thresh_)
        {
            continue;  // 임계값 미만은 버림
        }


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
```


---


## Step 5: 메인 실행 및 FPS 측정

앞 부품(엔진 로더, 전처리/후처리, NMS)을 하나로 묶어 카메라 -> 추론 -> 시각화 루프를 돌리고 FPS를 측정한다. week6의 최종 결과물이다.

### 왜 FPS를 측정하나

목표는 30 FPS, 즉 프레임당 33.3ms 안에 전처리, 추론, 후처리, 시각화를 끝내는 것이다. 단계별 시간을 재서 병목을 찾는다(전처리 약 2ms, 추론 약 8ms, NMS 약 1ms). 실시간 시스템에서 충분히 빠른가는 평균이 아니라 프레임 예산 안에 드는가로 판단한다. 이것이 Phase 3 비전 파트(week1-6)의 종착점이다. 학습(week1-4) -> 변환(week5) -> 엣지 배포(week6)의 전체 사슬을 C++ 실시간 추론으로 닫는다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 파이프라인 통합 | 로더 + 전처리 + NMS + 시각화 | 부품을 하나로 |
| FPS 측정 | 프레임당 시간 누적 | 실시간성 판단 |
| 병목 분석 | 단계별 타이밍 | 어디가 느린지 |

### 핵심 포인트

- 30 FPS는 평균이 아니라 프레임 예산(33.3ms) 안에 드는지로 본다. 한 프레임이라도 넘으면 끊긴다.
- 멀티스레딩(캡처와 추론 분리)으로 더 짜낼 수 있다(README 7절). 단계별 타이밍으로 병목부터 잡는 것이 먼저다.


### src/main.cpp


```cpp
#include <iostream>
#include <chrono> // 시간 측정용
#include <opencv2/opencv.hpp>
#include "yolo_detector.h" // 전처리 + 추론 + 후처리 + NMS를 묶은 검출기 (Step 4)


int main(int argc, char** argv)
{
    std::string engine_path = "models/yolo11n_fp16.trt";  // 기본 엔진 경로
    std::string image_path = "data/test.jpg";             // 기본 이미지 경로


    if (argc >= 2)
    {
        engine_path = argv[1];  // 첫 번째 인자로 엔진 경로 받기
    }
    if (argc >= 3)
    {
        image_path = argv[2];  // 두 번째 인자로 이미지 경로 받기
    }


    std::cout << "=============================" << std::endl;
    std::cout << "TensorRT YOLO11 추론 테스트" << std::endl;
    std::cout << "=============================" << std::endl;


    // 검출기 생성 (생성자에서 엔진 로드까지 수행)
    YoloDetector detector(engine_path);


    // 테스트 이미지 로드
    cv::Mat image = cv::imread(image_path);
    if (image.empty())  // 이미지 로드 실패 체크
    {
        std::cerr << "[ERROR] 이미지 로드 실패: " << image_path << std::endl;
        return -1;
    }


    // Warm-up (10회): 첫 추론은 느리므로 측정에서 제외
    for (int i = 0; i < 10; i++)
    {
        detector.detect(image);
    }


    // FPS 측정 (100회): 전처리 + 추론 + 후처리 + NMS 전체를 잰다
    int num_runs = 100;
    std::vector<Detection> detections;  // 마지막 프레임의 검출 결과 (시각화에 사용)
    auto start = std::chrono::high_resolution_clock::now();  // 측정 시작 시각


    for (int i = 0; i < num_runs; i++)
    {
        // detect() 내부의 infer()가 cudaStreamSynchronize로 추론 완료까지 대기하므로
        // 별도 cudaDeviceSynchronize 없이도 반환 시점에 결과가 준비된다
        detections = detector.detect(image);
    }


    auto end = std::chrono::high_resolution_clock::now();  // 측정 종료 시각
    double total_ms = std::chrono::duration<double, std::milli>(end - start).count();
    double avg_ms = total_ms / num_runs;  // 프레임당 평균 처리 시간
    double fps = 1000.0 / avg_ms;         // 초당 프레임 수


    // 검출 결과 시각화 (마지막 프레임 기준)
    for (const auto& det : detections)
    {
        // 박스 그리기 (초록색)
        cv::rectangle(image,
                      cv::Point(static_cast<int>(det.x1), static_cast<int>(det.y1)),
                      cv::Point(static_cast<int>(det.x2), static_cast<int>(det.y2)),
                      cv::Scalar(0, 255, 0), 2);
        // 클래스 ID 라벨 (박스 좌상단 위쪽)
        std::string label = "cls " + std::to_string(det.class_id);
        cv::putText(image, label,
                    cv::Point(static_cast<int>(det.x1), static_cast<int>(det.y1) - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }
    cv::imwrite("data/result.jpg", image);  // 박스를 그린 결과 저장


    std::cout << "\n[결과]" << std::endl;
    std::cout << "평균 처리 시간 (전처리+추론+후처리): " << avg_ms << " ms" << std::endl;
    std::cout << "FPS: " << fps << std::endl;
    std::cout << "검출된 객체: " << detections.size() << " 개" << std::endl;
    std::cout << "결과 저장: data/result.jpg" << std::endl;
    std::cout << "목표 달성: " << (fps >= 30 ? "성공!" : "미달") << std::endl;


    return 0;
}
```


---


## CMakeLists.txt

소스 파일(`src/*.cpp`)을 TensorRT, CUDA, OpenCV와 연결해 실행 파일 하나로 빌드하는 설정이다. 세 의존성을 찾는 방식이 서로 다르다는 점이 핵심이다. OpenCV는 `find_package`로 바로 찾지만, TensorRT는 공식 CMake 패키지 설정이 없어 `find_path`/`find_library`로 헤더와 라이브러리를 직접 찾아야 한다. CUDA는 `CUDAToolkit` 모듈이 `CUDA::cudart` 타깃을 제공한다.

```cmake
cmake_minimum_required(VERSION 3.18)  # CUDAToolkit 모듈 사용 위해 3.18 이상
project(phase3_week6_tensorrt LANGUAGES CXX)

# C++17 표준 강제
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 컴파일 경고 활성화
add_compile_options(-Wall -Wextra)

# OpenCV: 표준 find_package로 탐색 (전처리/시각화에 사용)
find_package(OpenCV REQUIRED)

# CUDA: cudaMalloc/cudaMemcpy 등 런타임 API 사용 (CUDA::cudart 타깃 제공)
find_package(CUDAToolkit REQUIRED)

# TensorRT: 공식 CMake config가 없어 헤더/라이브러리를 직접 탐색
#   TENSORRT_ROOT를 -D 옵션으로 넘기면 비표준 설치 경로도 지원
#   예) cmake .. -DTENSORRT_ROOT=/usr/src/tensorrt
find_path(TENSORRT_INCLUDE_DIR NvInfer.h
    HINTS ${TENSORRT_ROOT} /usr/include/x86_64-linux-gnu /usr/include/aarch64-linux-gnu)
find_library(TENSORRT_NVINFER nvinfer
    HINTS ${TENSORRT_ROOT} /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)

# 탐색 실패 시 명확한 에러로 중단
if(NOT TENSORRT_INCLUDE_DIR OR NOT TENSORRT_NVINFER)
    message(FATAL_ERROR "TensorRT를 찾을 수 없습니다. -DTENSORRT_ROOT=<경로>로 지정하세요.")
endif()

# 실행 파일: 모든 소스 파일을 하나로 빌드
add_executable(phase3_week6_tensorrt
    src/main.cpp
    src/trt_engine.cpp
    src/yolo_detector.cpp
    src/nms.cpp)

# 헤더 경로: 프로젝트 include/ + TensorRT 헤더
target_include_directories(phase3_week6_tensorrt PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${TENSORRT_INCLUDE_DIR})

# 라이브러리 링크: TensorRT + CUDA 런타임 + OpenCV
target_link_libraries(phase3_week6_tensorrt PRIVATE
    ${TENSORRT_NVINFER}
    CUDA::cudart
    ${OpenCV_LIBS})
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
=============================
TensorRT YOLO11 추론 테스트
=============================
[INFO] 입력 (images): 1228800 floats
[INFO] 출력 (output0): 705600 floats
[INFO] TensorRT 엔진 로드 완료


[결과]
평균 처리 시간 (전처리+추론+후처리): 9.4 ms
FPS: 106.4
검출된 객체: 3 개
결과 저장: data/result.jpg
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
