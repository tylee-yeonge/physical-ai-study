cmake_minimum_required(VERSION 3.10)
project(Phase3_Week5_MiniVO)

set(CMAKE_CXX_STANDARD 14)

# OpenCV
find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# GTest (optional)
find_package(GTest)

# Basic demo executable
add_executable(basic basic.cpp)
target_link_libraries(basic ${OpenCV_LIBS})

# Quiz executables
add_executable(quiz_easy quiz_easy.cpp)
target_link_libraries(quiz_easy ${OpenCV_LIBS})

add_executable(quiz_medium quiz_medium.cpp)
target_link_libraries(quiz_medium ${OpenCV_LIBS})

# Tests (if GTest found)
if(GTEST_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
    add_executable(test_mini_vo test/test_basic.cpp)
    target_link_libraries(test_mini_vo ${OpenCV_LIBS} ${GTEST_LIBRARIES} pthread)
endif()

message(STATUS "Week 5: Mini VO Project configured")
