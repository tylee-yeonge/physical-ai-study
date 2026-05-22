cmake_minimum_required(VERSION 3.10)
project(Phase3_Week6_TensorRT)
set(CMAKE_CXX_STANDARD 17)

# CUDA
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# TensorRT
find_path(TENSORRT_INCLUDE_DIR NvInfer.h
    HINTS /usr/include/x86_64-linux-gnu
          /usr/local/include
          /usr/include
)
find_library(TENSORRT_LIBRARY nvinfer
    HINTS /usr/lib/x86_64-linux-gnu
          /usr/local/lib
          /usr/lib
)
find_library(TENSORRT_PARSERS nvparsers
    HINTS /usr/lib/x86_64-linux-gnu
          /usr/local/lib
          /usr/lib
)

include_directories(${TENSORRT_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

# 소스 파일
set(SOURCES
    src/main.cpp
    src/trt_engine.cpp
    src/nms.cpp
    src/yolo_detector.cpp
)

# 실행 파일
add_executable(phase3_week6_tensorrt ${SOURCES})

target_link_libraries(phase3_week6_tensorrt
    ${OpenCV_LIBS}
    ${CUDA_LIBRARIES}
    ${TENSORRT_LIBRARY}
    ${TENSORRT_PARSERS}
    cudart
)

# 설치
install(TARGETS phase3_week6_tensorrt DESTINATION bin)
