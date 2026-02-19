cmake_minimum_required(VERSION 3.10)
project(Phase3_Week11_Ceres_Practice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Ceres
find_package(Ceres REQUIRED)

# basic / my_basic (Ceres 필요)
add_executable(basic basic.cpp)
target_link_libraries(basic Eigen3::Eigen Ceres::ceres)

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic Eigen3::Eigen Ceres::ceres)

# Quiz (Ceres 불필요 — 개념 문제)
add_executable(quiz_easy quiz_easy.cpp)
target_link_libraries(quiz_easy Eigen3::Eigen)

add_executable(quiz_medium quiz_medium.cpp)
target_link_libraries(quiz_medium Eigen3::Eigen)

# Solution 빌드
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/quiz_solutions/easy_sol.cpp)
    add_executable(easy_sol quiz_solutions/easy_sol.cpp)
    target_link_libraries(easy_sol Eigen3::Eigen)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/quiz_solutions/medium_sol.cpp)
    add_executable(medium_sol quiz_solutions/medium_sol.cpp)
    target_link_libraries(medium_sol Eigen3::Eigen)
endif()

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 11: Ceres 실습")
message(STATUS "⚠️  빌드 환경: Ubuntu + Ceres 필요")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
