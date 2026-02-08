cmake_minimum_required(VERSION 3.10)
project(Week9_DepthTRT LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

# ────────────────────────────────────────────
# 퀴즈 빌드 (TensorRT 의존성 없음, 단독 빌드 가능)
# ────────────────────────────────────────────
add_executable(quiz_easy quiz_easy.cpp)
add_executable(quiz_medium quiz_medium.cpp)
add_executable(easy_sol quiz_solutions/easy_sol.cpp)
add_executable(medium_sol quiz_solutions/medium_sol.cpp)

# ────────────────────────────────────────────
# TensorRT 추론 빌드 (Jetson에서만 빌드)
# ────────────────────────────────────────────
option(BUILD_TRT_INFERENCE "TensorRT 추론 코드 빌드 (Jetson 전용)" OFF)

if(BUILD_TRT_INFERENCE)
    # OpenCV
    find_package(OpenCV 4 REQUIRED)

    # CUDA
    find_package(CUDA REQUIRED)

    # TensorRT
    find_library(NVINFER nvinfer)
    find_library(NVONNXPARSER nvonnxparser)

    if(NOT NVINFER)
        message(FATAL_ERROR "TensorRT(nvinfer)를 찾을 수 없습니다. Jetson/TensorRT가 설치되어 있는지 확인하세요.")
    endif()

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
endif()
