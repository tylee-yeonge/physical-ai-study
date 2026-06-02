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
