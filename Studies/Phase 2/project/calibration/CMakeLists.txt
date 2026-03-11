cmake_minimum_required(VERSION 3.10)
project(camera_calibration)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -Wall")

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(include)

# 단일 카메라 캘리브레이션
add_executable(mono_calib 
    src/camera_calibration.cpp
    src/mono_calib.cpp
)
target_link_libraries(mono_calib ${OpenCV_LIBS})

# 스테레오 캘리브레이션 (다음 실습)
# add_executable(stereo_calib 
#     src/camera_calibration.cpp
#     src/stereo_calib.cpp
# )
# target_link_libraries(stereo_calib ${OpenCV_LIBS})