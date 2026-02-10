cmake_minimum_required(VERSION 3.10)
project(Phase3_Week6_KeyframeManagement)

set(CMAKE_CXX_STANDARD 14)

# OpenCV
find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Basic demo
add_executable(basic basic.cpp)
target_link_libraries(basic ${OpenCV_LIBS})

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic ${OpenCV_LIBS})

# Quizzes
add_executable(quiz_easy quiz_easy.cpp)
add_executable(quiz_medium quiz_medium.cpp)

message(STATUS "Week 6: Keyframe Management configured")
