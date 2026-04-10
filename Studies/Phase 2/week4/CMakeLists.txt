cmake_minimum_required(VERSION 3.10)
project(phase2_week4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 컴파일 경고
add_compile_options(-Wall -Wextra)

# OpenCV 찾기
find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 실행 타겟
add_executable(basic basic.cpp)
target_link_libraries(basic PRIVATE ${OpenCV_LIBS})

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic PRIVATE ${OpenCV_LIBS})

add_executable(quiz_easy quiz_easy.cpp)
target_link_libraries(quiz_easy PRIVATE ${OpenCV_LIBS})

add_executable(quiz_medium quiz_medium.cpp)
target_link_libraries(quiz_medium PRIVATE ${OpenCV_LIBS})
