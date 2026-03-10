cmake_minimum_required(VERSION 3.10)
project(mini_vo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(include)

set(SOURCES
    src/camera.cpp
    src/feature_detector.cpp
    src/descriptor.cpp
    src/feature_matcher.cpp
    src/epipolar.cpp
    src/pose_recovery.cpp
    src/triangulator.cpp
    src/tracker.cpp
)

add_executable(mini_vo main.cpp ${SOURCES})
target_link_libraries(mini_vo ${OpenCV_LIBS})

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "mini_vo: Phase 2 프로젝트")
message(STATUS "  W3: FeatureDetector  (FAST 직접 구현)")
message(STATUS "  W3: Descriptor       (BRIEF 직접 구현)")
message(STATUS "  W4: FeatureMatcher   (stub)")
message(STATUS "  W5: Epipolar         (stub)")
message(STATUS "  W6: PoseRecovery     (stub)")
message(STATUS "  W7: Triangulator     (stub)")
message(STATUS "  W8: Tracker / LK     (stub)")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
