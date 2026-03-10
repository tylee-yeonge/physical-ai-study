cmake_minimum_required(VERSION 3.10)
project(mini_slam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Phase 3 mini_slam 헤더
include_directories(include)

# Phase 2 mini_vo 헤더 (재사용)
set(MINI_VO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Phase 2/mini_vo")
include_directories("${MINI_VO_DIR}/include")

# Phase 2 mini_vo 소스 (재사용)
set(MINI_VO_SOURCES
    "${MINI_VO_DIR}/src/camera.cpp"
    "${MINI_VO_DIR}/src/feature_detector.cpp"
    "${MINI_VO_DIR}/src/descriptor.cpp"
    "${MINI_VO_DIR}/src/feature_matcher.cpp"
    "${MINI_VO_DIR}/src/epipolar.cpp"
    "${MINI_VO_DIR}/src/pose_recovery.cpp"
    "${MINI_VO_DIR}/src/triangulator.cpp"
    "${MINI_VO_DIR}/src/tracker.cpp"
)

# Phase 3 mini_slam 소스
set(MINI_SLAM_SOURCES
    src/keyframe.cpp
    src/map.cpp
    src/local_ba_ceres.cpp
    src/local_ba_g2o.cpp
)

add_executable(mini_slam main.cpp ${MINI_VO_SOURCES} ${MINI_SLAM_SOURCES})
target_link_libraries(mini_slam ${OpenCV_LIBS})

# TODO (W8):  find_package(Ceres REQUIRED) 추가
# TODO (W9):  find_package(g2o REQUIRED)   추가

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "mini_slam: Phase 3 프로젝트")
message(STATUS "  Phase 2 mini_vo 재사용: ${MINI_VO_DIR}")
message(STATUS "  W1:  VO 파이프라인 검증")
message(STATUS "  W6:  Keyframe + Map 관리")
message(STATUS "  W8:  Ceres Local BA (stub)")
message(STATUS "  W9:  g2o Local BA   (stub)")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
