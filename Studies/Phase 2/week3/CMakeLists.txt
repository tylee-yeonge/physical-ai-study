cmake_minimum_required(VERSION 3.10)
project(Phase2_Week3_FeatureDetection)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

# OpenCV 4 필수
find_package(OpenCV 4 REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "Found OpenCV ${OpenCV_VERSION}")
    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "OpenCV not found!")
endif()

# 기본 데모
add_executable(basic
    basic.cpp
)
target_link_libraries(basic ${OpenCV_LIBS})

# 기초 퀴즈
add_executable(quiz_easy
    quiz_easy.cpp
)
target_link_libraries(quiz_easy ${OpenCV_LIBS})

# 중급 퀴즈
add_executable(quiz_medium
    quiz_medium.cpp
)
target_link_libraries(quiz_medium ${OpenCV_LIBS})

# Google Test
find_package(GTest QUIET)
if(GTest_FOUND)
    enable_testing()
    add_executable(test_basic test/test_basic.cpp)
    target_link_libraries(test_basic ${OpenCV_LIBS} GTest::GTest GTest::Main)
    add_test(NAME Week3Tests COMMAND test_basic)
endif()

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 2 Week 3: Feature Detection")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
